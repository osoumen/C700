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
//	FASTRELEASE
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
        int         volume;
        int         expression;
        int         pan;
        float       pbRange;
        //bool        portaOn;
        float       portaTc;
        float       portaStartPitch;
        int         lastNote;
        bool        damper;
        //bool        monoMode;
        
        //int         priority;
        int         limit;
        int         noteOns;
        //int         releasePriority;
        
        unsigned int changeFlg;
        InstParams  changedVP;
    } ChannelStatus;
    
	C700Generator();
	virtual				~C700Generator() {}
	
	virtual void		Reset();

	void		KeyOn( int ch, int note, int velo, unsigned int uniqueID, int inFrame );
	void		KeyOff( int ch, int note, int velo, unsigned int uniqueID, int inFrame );
	void		ProgramChange( int ch, int value, int inFrame );
	void		PitchBend( int ch, int value1, int value2, int inFrame );
    void        ControlChange( int ch, int controlNum, int value, int inFrame );
	void		AllNotesOff();
	void		AllSoundOff();
	void		ResetAllControllers();
	
    // channel params
	void		ModWheel( int ch, int value );
	void		Damper( int ch, bool on );
    void        Volume( int ch, int value );
    void        Expression( int ch, int value );
    void        Panpot( int ch, int value );
    void        ChangeChRate(int ch, double rate);
    void        ChangeChBasekey(int ch, int basekey);
    void        ChangeChLowkey(int ch, int lowkey);
    void        ChangeChHighkey(int ch, int highkey);
    void        ChangeChAR(int ch, int ar);
    void        ChangeChDR(int ch, int dr);
    void        ChangeChSL(int ch, int sl);
    void        ChangeChSR(int ch, int sr);
    void        ChangeChVolL(int ch, int voll);
    void        ChangeChVolR(int ch, int volr);
    void        ChangeChEcho(int ch, int echo);
    void        ChangeChBank(int ch, int bank);
    void        ChangeChSustainMode(int ch, int sustainMode);
    void        SetPortamentOn( int ch, bool on );
    void        SetPortamentTime( int ch, int value );
    void        UpdatePortamentoTime( int prog );
    void        SetPortamentControl( int ch, int note );
    void        SetChPriority( int ch, int value );
    void        SetChLimit( int ch, int value );
    void        SetReleasePriority( int ch, int value );
    void        SetMonoMode( int ch, bool on );

    // global params
	void		SetVoiceLimit( int value );
	void		SetPBRange( float value );
	void		SetPBRange( int ch, float value );
	void		SetADPCMMode( bool value );
	void		SetMultiMode( int bank, bool value );
	bool		GetMultiMode( int bank ) const;
	void		SetVelocityMode( velocity_mode value );
	void		SetVibFreq( int ch, float value );
	void		SetVibDepth( int ch, float value );
	void		SetMainVol_L( int value );
	void		SetMainVol_R( int value );
	void		SetEchoVol_L( int value );
	void		SetEchoVol_R( int value );
	void		SetFeedBackLevel( int value );
	void		SetDelayTime( int value );
	int			GetDelayTime();
	void		SetFIRTap( int tap, int value );
	
	void		SetSampleRate( double samplerate ) {
        mSampleRate = samplerate;
        mEventDelaySamples = calcEventDelaySamples();
    }
    
	void		Process( unsigned int frames, float *output[2] );
	int			GetKeyMap( int bank, int key ) const { return mKeyMap[bank][key]; }
	InstParams	*getVP(int pg) const { return &mVPset[pg]; }
	InstParams	*getMappedVP(int bank, int key) const { return &mVPset[mKeyMap[bank][key]]; }
	void		SetVPSet( InstParams *vp );
    
    void        SetEventDelayClocks(int clocks) {
        mEventDelayClocks = clocks;
        mEventDelaySamples = calcEventDelaySamples();
    }
    
    double      GetProcessDelayTime();
    int         GetNoteOnNotes(int ch) { return mChStat[ch].noteOns; }
	
	void		RefreshKeyMap(void);
	
private:
	static const int INTERNAL_CLOCK = 32000;
    static const int CYCLES_PER_SAMPLE = 21168;
    static const int PORTAMENT_CYCLE_SAMPLES = 32;  // ポルタメント処理を行うサンプル数(32kHz換算)
    static const int CLOCKS_PER_SAMPLE = 32;
    
    static const int VOLUME_DEFAULT = 100;
    static const int EXPRESSION_DEFAULT = 127;
    static const int DEFAULT_PBRANGE = 2;
	
	enum EvtType {
		NOTE_ON = 0,
		NOTE_OFF,
        PROGRAM_CHANGE,
        PITCH_BEND,
        CONTROL_CHANGE
	};
	
	typedef struct {
		EvtType         type;
		unsigned char	ch;
		unsigned char	note;
		unsigned char	velo;
		unsigned int	uniqueID;
		int				remain_samples;
	} MIDIEvt;
	
	struct VoiceState {
		int				midi_ch;
        int             priority;
		unsigned int	uniqueID;
        
        bool            isKeyOn;       // キーオンされているかどうか
        bool            legato;        // モノモードで２音目以降の音か
		
		int				pb;
		int				vibdepth;
		bool			reg_pmod;
		float			vibPhase;
        float           portaPitch;
		
		int				ar,dr,sl,sr,vol_l,vol_r;
		
		int				velo;
        int             volume;
        int             expression;
        int             pan;
		unsigned int	loopPoint;
		bool			loop;
        
		bool			echoOn;
		

        // 音源内部状態
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

		void Reset();
	};
	
	double			mSampleRate;
	
	int				mProcessFrac;
	int				mProcessbuf[2][16];		//リサンプリング用バッファ
	int				mProcessbufPtr;			//リサンプリング用バッファ書き込み位置
	EchoKernel		mEcho[2];
	
	std::list<MIDIEvt>	mMIDIEvt;			//受け取ったイベントのキュー
	std::list<MIDIEvt>	mDelayedEvt;		//遅延実行イベントのキュー
    bool            mClearEvent;            //次のRenderでFIFOをクリアするフラグ
	
	std::list<int>	mPlayVo;				//ノートオン状態のボイス
	std::list<int>	mWaitVo;				//ノートオフ状態のボイス
	
	VoiceState		mVoice[kMaximumVoices];		//ボイスの状況
	
	int				mVoiceLimit;
	int				mMainVolume_L;
	int				mMainVolume_R;
	float			mVibfreq;
	float			mVibdepth;
	bool			mNewADPCM;
	bool			mDrumMode[NUM_BANKS];
	velocity_mode	mVelocityMode;
    ChannelStatus   mChStat[16];
    int             mPortamentCount;        // DSP処理が1サンプル出力される毎にカウントされ、ポルタメント処理されるとPORTAMENT_CYCLE_SAMPLES 減らす
    int             mEventDelaySamples;     // 動作遅延サンプル(処理サンプリングレート)
    int             mEventDelayClocks;      // 動作遅延クロック
	
	int				mKeyMap[NUM_BANKS][128];	//各キーに対応するプログラムNo.
	InstParams		*mVPset;
	
	int		FindFreeVoice();
    int     StealVoice(int prio);
    int     StealVoice(int ch, int prio);
	int		StopPlayingVoice( const MIDIEvt *evt );
	void	doKeyOn(const MIDIEvt *evt);
	void	doProgramChange( int ch, int value );
	void	doPitchBend( int ch, int value1, int value2 );
    void    doControlChange( int ch, int controlNum, int value );
	float	VibratoWave(float phase);
	int		CalcPBValue(int ch, float pitchBend, int basePitch);
    InstParams getChannelVP(int ch, int note);
    void processPortament(int vo);
    void calcPanVolume(int value, int *volL, int *volR);
    bool doEvents1( const MIDIEvt *evt );
    bool doEvents2( const MIDIEvt *evt );
    int calcEventDelaySamples() { return ((mEventDelayClocks / CLOCKS_PER_SAMPLE) * mSampleRate) / INTERNAL_CLOCK; }
    float calcGM2PortamentCurve(int value);
};
