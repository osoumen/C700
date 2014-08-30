/*
 *  C700Generator.h
 *  C700
 *
 *  Created by osoumen on 06/09/06.
 *  Copyright 2006 Vermicelli Magic. All rights reserved.
 *
 */

#pragma once

#include "C700defines.h"
#include "EchoKernel.h"
#include <list>

//-----------------------------------------------------------------------------
typedef enum
{
    ATTACK,
    DECAY,
    SUSTAIN,
    RELEASE,
	FASTRELEASE
} env_state_t32;

//-----------------------------------------------------------------------------
typedef enum
{
    kVelocityMode_Constant,
    kVelocityMode_Square,
    kVelocityMode_Linear
} velocity_mode;

//-----------------------------------------------------------------------------
class C700Generator
{
public:
    typedef struct {
        int         prog;
        float		pitchBend;
        int			vibDepth;
        int         expression;
        int         pan;
        float       pbRange;
        bool        portaOn;
        float       portaTc;
        float       portaStartPitch;
        int         lastNote;
        uint32_t    changeFlg;
        InstParams  changedVP;
    } ChannelStatus;
    
	C700Generator();
	virtual				~C700Generator() {}
	
	virtual void		Reset();

	void		KeyOn( unsigned char ch, unsigned char note, unsigned char velo, unsigned int uniqueID, int inFrame );
	void		KeyOff( unsigned char ch, unsigned char note, unsigned char velo, unsigned int uniqueID, int inFrame );
	void		AllNotesOff();
	void		AllSoundOff();
	void		ResetAllControllers();
	
	void		ProgramChange( int ch, int pgnum, int inFrame );
	void		PitchBend( int ch, int value, int inFrame );
	void		ModWheel( int ch, int value, int inFrame );
	void		Damper( int ch, int value, int inFrame );
    void        Expression( int ch, int value, int inFrame );
    void        Panpot( int ch, int value, int inFrame );
    void        ChangeChRate(int ch, double rate, int inFrame);
    void        ChangeChBasekey(int ch, int basekey, int inFrame);
    void        ChangeChLowkey(int ch, int lowkey, int inFrame);
    void        ChangeChHighkey(int ch, int highkey, int inFrame);
    void        ChangeChAR(int ch, int ar, int inFrame);
    void        ChangeChDR(int ch, int dr, int inFrame);
    void        ChangeChSL(int ch, int sl, int inFrame);
    void        ChangeChSR(int ch, int sr, int inFrame);
    void        ChangeChVolL(int ch, int voll, int inFrame);
    void        ChangeChVolR(int ch, int volr, int inFrame);
    void        ChangeChEcho(int ch, int echo, int inFrame);
    void        ChangeChBank(int ch, int bank, int inFrame);
    void        ChangeChSustainMode(int ch, int sustainMode, int inFrame);

	void		SetVoiceLimit( int value );
	void		SetPBRange( float value );
	void		SetPBRange( int ch, float value );
	void		SetClipper( bool value );
	void		SetMultiMode( int bank, bool value );
	bool		GetMultiMode( int bank ) const;
	void		SetVelocityMode( velocity_mode value );
	void		SetVibFreq( float value );
	void		SetVibDepth( float value );
    void        SetPortamentOn( int ch, bool on );
    void        SetPortamentTime( int ch, float secs );
    void        SetPortamentControl( int ch, int note );
	
	void		SetMainVol_L( int value );
	void		SetMainVol_R( int value );
	void		SetEchoVol_L( int value );
	void		SetEchoVol_R( int value );
	void		SetFeedBackLevel( int value );
	void		SetDelayTime( int value );
	int			GetDelayTime();
	void		SetFIRTap( int tap, int value );
	
	void		SetSampleRate( double samplerate ) { mSampleRate = samplerate; }
    
	void		Process( unsigned int frames, float *output[2] );
	int			GetKeyMap( int bank, int key ) const { return mKeyMap[bank][key]; }
	InstParams	*getVP(int pg) const { return &mVPset[pg]; }
	InstParams	*getMappedVP(int bank, int key) const { return &mVPset[mKeyMap[bank][key]]; }
	void		SetVPSet( InstParams *vp );
	
	void		RefreshKeyMap(void);
	
private:
	static const int INTERNAL_CLOCK = 32000;
    static const int CYCLES_PER_SAMPLE = 21168;
    static const int PORTAMENT_CYCLE_SAMPLES = 32;  // ポルタメント処理を行うサンプル数(32kHz換算)
    static const int EXPRESSION_DEFAULT = 127;
    static const int DEFAULT_PBRANGE = 2;
	
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
        float           portaPitch;
		
		int				ar,dr,sl,sr,vol_l,vol_r;
		
		int				velo;
        int             expression;
        int             pan;
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
	
	std::list<int>	mPlayVo;				//ノートオン状態のボイス
	std::list<int>	mWaitVo;				//ノートオフ状態のボイス
	
	VoiceState		mVoice[kMaximumVoices];		//ボイスの状況
	
	int				mVoiceLimit;
	int				mMainVolume_L;
	int				mMainVolume_R;
	float			mVibfreq;
	float			mVibdepth;
	bool			mClipper;
	bool			mDrumMode[NUM_BANKS];
	velocity_mode	mVelocityMode;
    ChannelStatus   mChStat[16];
    int             mPortamentCount;        // DSP処理が1サンプル出力される毎にカウントされ、ポルタメント処理されるとPORTAMENT_CYCLE_SAMPLES 減らす
	
	int				mKeyMap[NUM_BANKS][128];	//各キーに対応するプログラムNo.
	InstParams		*mVPset;
	
	int		FindFreeVoice( const NoteEvt *evt );
	int		StopPlayingVoice( const NoteEvt *evt );
	void	DoKeyOn(NoteEvt *evt);
	float	VibratoWave(float phase);
	int		CalcPBValue(int ch, float pitchBend, int basePitch);
    InstParams getChannelVP(int ch, int note);
    void processPortament(int vo);
    void calcPanVolume(int value, int *volL, int *volR);
};
