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

typedef enum
{
    ATTACK,
    DECAY,
    SUSTAIN,
    RELEASE,
	FASTRELEASE
} env_state_t32;


struct Chip700Note : public SynthNote
{
	virtual				~Chip700Note() {}
	virtual void		Reset();
	
	virtual void		Attack(const MusicDeviceNoteParams &inParams);
	virtual void		Kill(UInt32 inFrame);
	virtual void		Release(UInt32 inFrame);
	virtual void		FastRelease(UInt32 inFrame);
	virtual Float32		Amplitude() { return mEnvx; }
	virtual OSStatus	Render(UInt32 inNumFrames, AudioBufferList& inBufferList);
	
	void KeyOn(void);
	float VibratoWave(float phase);
	
	int				mInternalClock;
	int				mProcessFrac;
	int				mProcessbuf[16];
	int				mProcessbufPtr;
	
	SInt32			mVelo;
	UInt32			mLoopPoint;
	bool			mLoop;
	
	MusicDeviceNoteParams	mParam;
	unsigned char	*brrdata;
	bool			mFRFlag;
	
	float			mVibPhase;
	
	int				mMemPtr;        /* Sample data memory pointer   */
	int             mEnd;            /* End or loop after block      */
	int             mEnvcnt;         /* Counts to envelope update    */
	env_state_t32   mEnvstate;       /* Current envelope state       */
	int             mEnvx;           /* Last env height (0-0x7FFF)   */
	int             mFilter;         /* Last header's filter         */
	int             mHalf;           /* Active nybble of BRR         */
	int             mHeaderCnt;     /* Bytes before new header (0-8)*/
	int             mMixfrac;        /* Fractional part of smpl pstn */	//サンプル間を4096分割した位置
	int             mOnCnt;         /* Is it time to turn on yet?   */
	int				mOffCnt;
	int				mPitch;          /* Sample pitch (4096->32000Hz) */
	int             mRange;          /* Last header's range          */
//	unsigned long   mSamp_id;        /* Sample ID#                   */
	int             mSampptr;        /* Where in sampbuf we are      */
	int				mSmp1;           /* Last sample (for BRR filter) */
	int				mSmp2;           /* Second-to-last sample decoded*/
	int				mSampbuf[4];   /* Buffer for Gaussian interp   */
	
	int				ar,dr,sl,sr,vol_l,vol_r;
};

#endif
