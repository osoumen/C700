/*
 *  C700Driver.h
 *  C700
 *
 *  Created by osoumen on 06/09/06.
 *  Copyright 2006 Vermicelli Magic. All rights reserved.
 *
 */

#pragma once

#include "C700defines.h"
#include "C700DSP.h"
#include "DynamicVoiceManager.h"

//-----------------------------------------------------------------------------
typedef enum
{
    kVelocityMode_Constant,
    kVelocityMode_Square,
    kVelocityMode_Linear
} velocity_mode;

//-----------------------------------------------------------------------------
class C700Driver
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
        
        unsigned int changeFlg;
        InstParams  changedVP;
    } ChannelStatus;
    
    typedef struct VoiceStatus {
		int				pb;
		int				vibdepth;
		bool			reg_pmod;
		float			vibPhase;
        float           portaPitch;
		
		//int				ar,dr,sl,sr,vol_l,vol_r;    // ミラー
        int             vol_l,vol_r;
		
		int				velo;
        int             volume;
        int             expression;
        int             pan;
        int             srcn;
        //unsigned char	*brrdata;    // ミラー
		//unsigned int	loopPoint;    // ミラー
		//bool			loop;    // ミラー
        
		//bool			echoOn;    // ミラー
        
        int				pitch;    // ミラー
        
        VoiceStatus() : pb(0), vibdepth(0), reg_pmod(0), vibPhase(0), portaPitch(0),
                        vol_l(0), vol_r(0), velo(0), volume(0), expression(0), pan(0), srcn(0),
                        pitch(0) {}
        void Reset();
	} VoiceStatus;
    
	C700Driver();
	virtual				~C700Driver() {}
	
	virtual void		Reset();

	void		NoteOn( int ch, int note, int velo, unsigned int uniqueID, int inFrame );
	void		NoteOff( int ch, int note, int velo, unsigned int uniqueID, int inFrame );
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
    //void        ChangeChVolL(int ch, int voll);
    //void        ChangeChVolR(int ch, int volr);
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
    int         GetNoteOnNotes(int ch) { return mVoiceManager.GetNoteOns(ch); }
	
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
	
	double			mSampleRate;
	
	int				mProcessFrac;
	int				mProcessbuf[2][16];		//リサンプリング用バッファ
	int				mProcessbufPtr;			//リサンプリング用バッファ書き込み位置
	
	std::list<MIDIEvt>	mMIDIEvt;			//受け取ったイベントのキュー
	std::list<MIDIEvt>	mDelayedEvt;		//遅延実行イベントのキュー
    //bool            mClearEvent;            //次のRenderでFIFOをクリアするフラグ
	
    DynamicVoiceManager mVoiceManager;
	    
	bool			mDrumMode[NUM_BANKS];
	velocity_mode	mVelocityMode;
    ChannelStatus   mChStat[16];
    float			mVibfreq;
	float			mVibdepth;

    int             mPortamentCount;        // DSP処理が1サンプル出力される毎にカウントされ、ポルタメント処理されるとPORTAMENT_CYCLE_SAMPLES 減らす
    int             mEventDelaySamples;     // 動作遅延サンプル(処理サンプリングレート)
    int             mEventDelayClocks;      // 動作遅延クロック
	
	int				mKeyMap[NUM_BANKS][128];	//各キーに対応するプログラムNo.
	InstParams		*mVPset;
    
    C700DSP         mDSP;
    VoiceStatus		mVoiceStat[kMaximumVoices];
	
    InstParams getChannelVP(int ch, int note);
    void processPortament(int vo);
    void calcPanVolume(int value, int *volL, int *volR);
    void    doProgramChange( int ch, int value );
	void	doPitchBend( int ch, int value1, int value2 );
    void    doControlChange( int ch, int controlNum, int value );
	float	VibratoWave(float phase);
	int		CalcPBValue(int ch, float pitchBend, int basePitch);
    bool    doNoteOn1( MIDIEvt dEvt );
	void	doNoteOn2(const MIDIEvt *evt);
	int		doNoteOff( const MIDIEvt *evt );
    bool doEvents1( const MIDIEvt *evt );
    bool doEvents2( const MIDIEvt *evt );
    int calcEventDelaySamples() { return ((mEventDelayClocks / CLOCKS_PER_SAMPLE) * mSampleRate) / INTERNAL_CLOCK; }
    float calcGM2PortamentCurve(int value);
};
