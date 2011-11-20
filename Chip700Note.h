/*
 *  Chip700Note.h
 *  Chip700
 *
 *  Created by 開発用 on 06/09/06.
 *  Copyright 2006 Vermicelli Magic. All rights reserved.
 *
 */

#include "AUInstrumentBase.h"
#include "Chip700Version.h"

#ifndef __Chip700Note_h__
#define __Chip700Note_h__

#include "Chip700defines.h"
#include <list>

typedef enum
{
    ATTACK,
    DECAY,
    SUSTAIN,
    RELEASE,
	FASTRELEASE
} env_state_t32;


class EchoKernel
{
public:
	EchoKernel()
	{
		mDelayUnit=512;
		mEchoBuffer.AllocateClear(7680);
		mFIRLength=8;
		mFIRbuf.AllocateClear(mFIRLength);
		mFilterStride=1;
		mEchoIndex=0;
		mFIRIndex=0;
		
		m_echo_vol=0;
		m_input = 0;
	};
	
	void 	Input(int samp);
	int		GetOut();
	void	Reset();
	void	SetEchoVol( Float32 val )
	{
		m_echo_vol = val;
	}
	void	SetFBLevel( Float32 val )
	{
		m_fb_lev = val;
	}
	void	SetFIRTap( int index, Float32 val )
	{
		m_fir_taps[index] = val;
	}
	void	SetDelaySamples( Float32 val )
	{
		m_delay_samples = val * mDelayUnit;
	}
	
private:
	TAUBuffer<int>	mEchoBuffer;
	TAUBuffer<int>	mFIRbuf;
	int			mEchoIndex;
	int			mFIRIndex, mFIRLength;
	int			mDelayUnit;
	int			mFilterStride;
	
	int			m_echo_vol;
	int			m_fb_lev;
	int			m_fir_taps[8];
	int			m_delay_samples;
	
	int			m_input;
};

struct Chip700Note : public SynthNote
{
public:
	virtual				~Chip700Note() {}
	virtual void		Reset();
	
	virtual void		Attack(const MusicDeviceNoteParams &inParams);
	virtual void		Kill(UInt32 inFrame);
	virtual void		Release(UInt32 inFrame);
	virtual void		FastRelease(UInt32 inFrame);
	virtual Float32		Amplitude() { return mEnvx; }
	virtual OSStatus	Render(UInt32 inNumFrames, AudioBufferList& inBufferList);
	
private:
	typedef struct {
		bool	isOn;
		int		note;
		int		ch;
		int		velo;
		int		remain_samples;
	} NoteEvt;
	
	int				mInternalClock;
	int				mProcessFrac;
	int				mProcessbuf[2][16];
	int				mProcessbufPtr;
	EchoKernel		mEcho[2];
	
	std::list<NoteEvt>	mNoteEvt;
	
	
	unsigned char	*brrdata;
	int				mMemPtr;        /* Sample data memory pointer   */
	int             mEnd;            /* End or loop after block      */
	int             mEnvcnt;         /* Counts to envelope update    */
	env_state_t32   mEnvstate;       /* Current envelope state       */
	int             mEnvx;           /* Last env height (0-0x7FFF)   */
	int             mFilter;         /* Last header's filter         */
	int             mHalf;           /* Active nybble of BRR         */
	int             mHeaderCnt;     /* Bytes before new header (0-8)*/
	int             mMixfrac;        /* Fractional part of smpl pstn */	//サンプル間を4096分割した位置
	int				mPitch;          /* Sample pitch (4096->32000Hz) */
	int             mRange;          /* Last header's range          */
	int             mSampptr;        /* Where in sampbuf we are      */
	int				mSmp1;           /* Last sample (for BRR filter) */
	int				mSmp2;           /* Second-to-last sample decoded*/
	int				mSampbuf[4];   /* Buffer for Gaussian interp   */
	
	int				ar,dr,sl,sr,vol_l,vol_r;
	
	int				mVelo;
	unsigned int	mLoopPoint;
	bool			mLoop;
	
	bool			mFRFlag;
	
	float			mVibPhase;
	
	bool			mEchoOn;
	
	
	void KeyOn(NoteEvt *evt);
	float VibratoWave(float phase);
};


#endif
