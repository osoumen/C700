/*
 *  Chip700Generator.h
 *  Chip700
 *
 *  Created by 開発用 on 06/09/06.
 *  Copyright 2006 Vermicelli Magic. All rights reserved.
 *
 */

#ifndef __Chip700Generator_h__
#define __Chip700Generator_h__

#include "Chip700defines.h"
#include <AUBuffer.h>
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
private:
	static const int DELAY_UNIT = 512;
	static const int ECHO_BUFFER_SIZE = 7680;
	static const int FIR_LENGTH = 8;
	static const int FILTER_STLIDE = 1;
	
public:
	EchoKernel()
	{
		mEchoBuffer.AllocateClear(ECHO_BUFFER_SIZE);
		mFIRbuf.AllocateClear(FIR_LENGTH);
		mEchoIndex=0;
		mFIRIndex=0;
		
		m_echo_vol=0;
		m_input = 0;
	};
	
	void 	Input(int samp);
	int		GetFxOut();
	void	Reset();
	void	SetEchoVol( int val )
	{
		m_echo_vol = val;
	}
	void	SetFBLevel( int val )
	{
		m_fb_lev = val;
	}
	void	SetFIRTap( int index, int val )
	{
		m_fir_taps[index] = val;
	}
	void	SetDelayTime( int val )
	{
		m_delay_samples = val * DELAY_UNIT;
	}
	
private:
	TAUBuffer<int>	mEchoBuffer;
	TAUBuffer<int>	mFIRbuf;
	int			mEchoIndex;
	int			mFIRIndex;
	
	int			m_echo_vol;
	int			m_fb_lev;
	int			m_fir_taps[8];
	int			m_delay_samples;
	
	int			m_input;
};

class Chip700Generator
{
public:
	Chip700Generator();
	virtual				~Chip700Generator() {}
	virtual void		Reset();

	void KeyOn( unsigned char ch, unsigned char note, unsigned char velo, unsigned int uniqueID, int inFrame );
	void KeyOff( unsigned char ch, unsigned char note, unsigned char velo, unsigned int uniqueID, int inFrame );
	void AllNotesOff();
	void AllSoundOff();
	void ResetAllControllers();
	
	void ProgramChange( int ch, int pgnum, int inFrame );
	void PitchBend( int ch, int value, int inFrame );
	void ModWheel( int ch, int value, int inFrame );
	void Damper( int ch, int value, int inFrame );
	
	void SetVoiceLimit( int value );
	void SetPBRange( float value );
	void SetClipper( bool value );
	void SetMultiMode( int bank, bool value );
	void SetVelocitySens( bool value );
	void SetVibFreq( float value );
	void SetVibDepth( float value );
	
	void SetMainVol_L( int value );
	void SetMainVol_R( int value );
	void SetEchoVol_L( int value );
	void SetEchoVol_R( int value );
	void SetFeedBackLevel( int value );
	void SetDelayTime( int value );
	void SetFIRTap( int tap, int value );
	
	void SetSampleRate( double samplerate ) { mSampleRate = samplerate; }
	
	void Process( unsigned int frames, float *output[2] );
	int GetKeyMap( int bank, int key ) { return mKeyMap[bank][key]; }
	VoiceParams getVP(int pg) {return mVPset[pg];};
	VoiceParams getMappedVP(int bank, int key) {return mVPset[mKeyMap[bank][key]];};
	void SetVPSet( VoiceParams *vp ) { mVPset = vp; }
	
	void RefreshKeyMap(void);
	
private:
	static const int INTERNAL_CLOCK = 32000;
	
	enum EvtType {
		NOTE_ON = 0,
		NOTE_OFF
	};
	
	typedef struct {
		unsigned char	type;
		unsigned char	ch;
		unsigned char	note;
		unsigned char	velo;
		unsigned int	uniqueID;
		int				remain_samples;
	} NoteEvt;
	
	struct VoiceState {
		int				midi_ch;
		unsigned int	uniqueID;
		
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
		
		
		int				pb;
		int				vibdepth;
		bool			reg_pmod;
		float			vibPhase;
		
		int				ar,dr,sl,sr,vol_l,vol_r;
		
		int				velo;
		unsigned int	loopPoint;
		bool			loop;
	
		bool			echoOn;
		
		void Reset();
	};
	
	double			mSampleRate;
	
	int				mProcessFrac;
	int				mProcessbuf[2][16];		//リサンプリング用バッファ
	int				mProcessbufPtr;			//リサンプリング用バッファ書き込み位置
	EchoKernel		mEcho[2];
	
	std::list<NoteEvt>	mNoteEvt;			//受け取ったイベントのキュー
	
	VoiceState		mVoice[kMaximumVoices];		//ボイスの状況
	
	int				mVoiceLimit;
	int				mMainVolume_L;
	int				mMainVolume_R;
	float			mVibfreq;
	float			mVibdepth;
	float			mPbrange;
	bool			mClipper;
	bool			mDrumMode[NUM_BANKS];
	bool			mVelocitySens;
	int				mChProgram[16];
	float			mChPitchBend[16];
	int				mChVibDepth[16];
	
	int				mKeyMap[NUM_BANKS][128];	//各キーに対応するプログラムNo.
	VoiceParams		*mVPset;
	
	int FindFreeVoice( const NoteEvt *evt );
	int StopPlayingVoice( const NoteEvt *evt );
	void DoKeyOn(NoteEvt *evt);
	float VibratoWave(float phase);
	int CalcPBValue(float pitchBend, int basePitch);
};


#endif
