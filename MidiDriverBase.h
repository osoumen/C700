//
//  MidiDriverBase.h
//  C700
//
//  Created by osoumen on 2017/06/10.
//
//

#ifndef MidiDriverBase_h
#define MidiDriverBase_h

#include "C700TimeThread.h"
#include "DynamicVoiceManager.h"

//-----------------------------------------------------------------------------
class Lfo_Linear {
public:
    Lfo_Linear(int updateRate=1000) : mUpdateRate(updateRate), mVibfreq(0.00137445), mVibdepth(0.5) {}
    ~Lfo_Linear() {}
    
    void        SetUpdateRate(int rate);
    void        Reset();
    void		SetVibFreq( float value );
    void		SetVibDepth( float value );
    void        SetVibSens( int sens );
    int         Update(int pitch);
    void        ResetPhase();
    float       calcVibratoWave(float phase);
    
private:
    int         mUpdateRate;
    int			mVibsens;
    float		mVibPhase;
    float       mVibfreq;
    float       mVibdepth;
};

//-----------------------------------------------------------------------------
class Portament_Linear {
public:
    Portament_Linear() : mTargetPitch(0), mTc(1.0f) {}
    void    Reset()
    {
        mTargetPitch = 0;
        mTc = 1.0f;
    }
    float               processPortament(float pitch);
    void                setTargetPicth(int pitch) { mTargetPitch = pitch; }
    int                 getTargetPitch() { return mTargetPitch; }
    void                setTc(float tc) { mTc = tc; }
    
private:
    int                 mTargetPitch;
    float               mTc;
    
};

//-----------------------------------------------------------------------------
class MidiDriverBase {
public:
    enum EvtType {
        NOTE_ON = 0,
        NOTE_OFF,
        PROGRAM_CHANGE,
        PITCH_BEND,
        CONTROL_CHANGE,
        START_REGLOG,
        MARKLOOP_REGLOG,
        END_REGLOG
    };
    
    typedef struct {
        EvtType         type;
        unsigned char	ch;
        unsigned char	note;
        unsigned char	velo;
        unsigned int	uniqueID;
        int				remain_samples;
        void setLegato() { note |= 0x80; }
        bool isLegato() const { return (note&0x80)!=0 ? true:false; }
        void setAllocedVo(int vo) { ch = (ch & 0x0f) | ((vo & 0x0f) << 4); }
        int getAllocedVo() const { return (ch & 0xf0) >> 4; }
    } MIDIEvt;
    
    typedef struct {
        int         prog;
        int         pitchBend;
        int			vibDepth;
        int         volume;
        int         expression;
        int         pan;
        float       pbRange;
        bool        portaOn;
        //float       portaTc;
        int         portaStartNote;
        int         lastNote;
        bool        damper;
    } ChannelStatus;
    
    MidiDriverBase(int maxVoices=8);
    virtual ~MidiDriverBase();
    
    virtual void		Reset();
    void                AllSoundOff();
    void                ResetAllControllers();
    void                AllNotesOff();
    
    void                EnqueueMidiEvent(const MIDIEvt *evt);
    
    void                ProcessMidiEvents();
    virtual void        doPreMidiEvents() {}
    virtual void        doPostMidiEvents() {}

    void                SetNoteOffIntervalCycles(int cycles) { mNoteOffIntervalCycles = cycles; }
    int                 GetNoteOffIntervalCycles() { return mNoteOffIntervalCycles; }
    
    int                 GetNoteOnNotes(int ch) { return mVoiceManager.GetNoteOns(ch); }
    void                changeChLimit( int ch, int value );
    int                 GetVoiceMidiCh(int vo) { return mVoiceManager.GetVoiceMidiCh(vo); }
    bool                IsKeyOnVoice(int vo) { return mVoiceManager.IsKeyOn(vo); }
    int                 GetVoiceLimit() { return mVoiceManager.GetVoiceLimit(); }

protected:
    static const int    VOLUME_DEFAULT = 100;
    static const int    EXPRESSION_DEFAULT = 127;
    static const int    DEFAULT_PBRANGE = 2;
    
    // channel messages
    virtual void        handleProgramChange( int ch, int value ) = 0;
    virtual void        handlePitchBend( int ch, int pitchbend ) = 0;
    virtual void        handleControlChange( int ch, int controlNum, int value );
    virtual bool        handleNoteOnFirst( MIDIEvt *evt, int killedMidiCh ) = 0;
    virtual bool        handleNoteOnDelayed(unsigned char vo, unsigned char midiCh, unsigned char note, unsigned char velo, bool isLegato) = 0;
    virtual void        handleNoteOff( const MIDIEvt *evt, int vo ) = 0;
    
    
    // control changes
    // set=それ以外の一般的なコントロールチェンジ
    // CCでいじれるのはリアルタイムコントロールが必要なものが中心で、全プリセットパラメータではない
    // ノートオン毎に切り替われば良いものはKernelでVoiceParamを直接書き換えるようになっている
    virtual void		handleAllNotesOff() {}
    virtual void		handleAllSoundOff() {}
    virtual void		handleResetAllControllers() {}

    virtual void        handleModWheelChange( int ch, int value ) {}
    virtual void        handleDamperChange( int ch, bool on ) {}
    virtual void        handleVolumeChange( int ch, int value ) {}
    virtual void        handleExpressionChange( int ch, int value ) {}
    virtual void        handlePanpotChange( int ch, int value ) {}
    
    virtual void        handlePortamentOnChange( int ch, bool on ) {}
    virtual void        handlePortaTimeChange( int ch, int ccValue, float centPerMilis ) {}
    virtual void        handlePortamentStartNoteChange( int ch, int note );
    
    virtual void        handleDataEntryValueChange(int ch, bool isNRPN, int addr, int value);
    virtual void        setPBRange( int ch, float value );

    const ChannelStatus *getChannelStatus(int ch) const { return &mChStat[ch]; }
    virtual int         getKeyOnPriority(int ch, int note) = 0;
    virtual int         getReleasePriority(int ch, int note) = 0;
    virtual bool        isMonoMode(int ch, int note) = 0;
    virtual bool        isPatchLoaded(int ch, int note) = 0;
    virtual float       getPortamentFreq() = 0;    // ポルタメント処理の１秒間の解像度
    void                calcPanVolume(int value, int *volL, int *volR);
    float               calcGM2PortamentCurve(int value);
    
    DynamicVoiceManager mVoiceManager;

private:
    MutexObject         mMIDIEvtMtx;
    std::list<MIDIEvt>	mMIDIEvt;			//受け取ったイベントのキュー
    std::list<MIDIEvt>	mDelayedEvt;		//遅延実行イベントのキュー
    int                 mNoteOffIntervalCycles;
    ChannelStatus       mChStat[16];
    
    // RPN, NRPN
    int                 mDataEntryValue[16];
    int                 mIsSettingNRPN[16];
    int                 mRPN[16];
    int                 mNRPN[16];

    void                setRPNLSB(int ch, int value);
    void                setRPNMSB(int ch, int value);
    void                setNRPNLSB(int ch, int value);
    void                setNRPNMSB(int ch, int value);
    void                setDataEntryLSB(int ch, int value);
    void                setDataEntryMSB(int ch, int value);

    bool                doFirstEvents( const MIDIEvt *evt );
    bool                doDelayedEvents( const MIDIEvt *evt );
};

#endif /* MidiDriverBase_h */
