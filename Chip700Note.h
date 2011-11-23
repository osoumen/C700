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
	virtual Float32		Amplitude();
	virtual OSStatus	Render(UInt32 inNumFrames, AudioBufferList& inBufferList);
	
	void ScheduleKeyOn( unsigned char ch, unsigned char note, unsigned char velo, int inFrame );
	void ScheduleKeyOff( unsigned char ch, unsigned char note, unsigned char velo, int inFrame );

private:
	static const int MAX_VOICES = 16;
	enum EvtType {
		NOTE_ON = 0,
		NOTE_OFF
	};
	
	typedef struct {
		unsigned char	type;
		unsigned char	ch;
		unsigned char	note;
		unsigned char	velo;
		int		remain_samples;
	} NoteEvt;
	
	struct VoiceState {
		unsigned char	*brrdata;
		int				memPtr;        /* Sample data memory pointer   */
		int             end;            /* End or loop after block      */
		int             envcnt;         /* Counts to envelope update    */
		env_state_t32   envstate;       /* Current envelope state       */
		int             envx;           /* Last env height (0-0x7FFF)   */
		int             filter;         /* Last header's filter         */
		int             half;           /* Active nybble of BRR         */
		int             headerCnt;     /* Bytes before new header (0-8)*/
		int             mixfrac;        /* Fractional part of smpl pstn */	//サンプル間を4096分割した位置
		int				pitch;          /* Sample pitch (4096->32000Hz) */
		int             range;          /* Last header's range          */
		int             sampptr;        /* Where in sampbuf we are      */
		int				smp1;           /* Last sample (for BRR filter) */
		int				smp2;           /* Second-to-last sample decoded*/
		int				sampbuf[4];   /* Buffer for Gaussian interp   */
		
		int				ar,dr,sl,sr,vol_l,vol_r;
		
		int				velo;
		unsigned int	loopPoint;
		bool			loop;
		
//		bool			FRFlag;
		
		float			vibPhase;
		
		bool			echoOn;
		
		void Reset();
	};
	
	int				mInternalClock;
	int				mProcessFrac;
	int				mProcessbuf[2][16];
	int				mProcessbufPtr;
	EchoKernel		mEcho[2];
	
	std::list<NoteEvt>	mNoteEvt;
	
	
	VoiceState		mVoice[MAX_VOICES];
	/*
	unsigned char	*brrdata;
	int				mMemPtr;     
	int             mEnd;        
	int             mEnvcnt;     
	env_state_t32   mEnvstate;   
	int             mEnvx;       
	int             mFilter;     
	int             mHalf;       
	int             mHeaderCnt;  
	int             mMixfrac;    
	int				mPitch;      
	int             mRange;      
	int             mSampptr;    
	int				mSmp1;       
	int				mSmp2;       
	int				mSampbuf[4]; 
	
	int				ar,dr,sl,sr,vol_l,vol_r;
	
	int				mVelo;
	unsigned int	mLoopPoint;
	bool			mLoop;
	
	bool			mFRFlag;
	
	float			mVibPhase;
	
	bool			mEchoOn;
	*/
	
	int		_note;
	
	int FindVoice( const NoteEvt *evt );
	void KeyOn(NoteEvt *evt);
	float VibratoWave(float phase);
};


#endif
