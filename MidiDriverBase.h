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
private:
    int         mUpdateRate;
    int			mVibsens;
    float		mVibPhase;
    float       mVibfreq;
    float       mVibdepth;

    float       calcVibratoWave(float phase);
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
    float               Update(float pitch);
    void                SetTargetPicth(int pitch) { mTargetPitch = pitch; }
    int                 GetTargetPitch() { return mTargetPitch; }
    void                SetTc(float tc) { mTc = tc; }
    
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
        CONTROL_CHANGE
    };
    
    typedef struct {
        EvtType         type;
        unsigned char	ch;
        unsigned char	data1;
        unsigned char	data2;
        unsigned int	uniqueID;
        int				remain_samples;
        void setLegato() { data1 |= 0x80; }
        bool isLegato() const { return (data1&0x80)!=0 ? true:false; }
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
        int         pbRange;
        bool        portaOn;
        int         portaStartNote;
        int         lastNote;
        bool        damper;
        int         dataEntryValue;
        int         isSettingNRPN;
        int         rpn;
        int         nrpn;
        
    } ChannelStatus;
    
    MidiDriverBase(int maxVoices=8);
    virtual ~MidiDriverBase();
    
    virtual void		Reset();
    void                AllSoundOff(int ch);
    void                ResetAllControllers(int ch);
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
    virtual bool        handleNoteOnFirst( unsigned char vo, unsigned char midiCh, unsigned char note, unsigned char velo, bool isLegato, int killedMidiCh ) = 0;
    virtual bool        handleNoteOnDelayed(unsigned char vo, unsigned char midiCh, unsigned char note, unsigned char velo, bool isLegato) = 0;
    virtual void        handleNoteOff( const MIDIEvt *evt, int vo ) = 0;
    
    
    // control changes
    // set=それ以外の一般的なコントロールチェンジ
    // CCでいじれるのはリアルタイムコントロールが必要なものが中心で、全プリセットパラメータではない
    // ノートオン毎に切り替われば良いものはKernelでVoiceParamを直接書き換えるようになっている
    virtual void		handleAllNotesOff() {}
    virtual void		handleAllSoundOff(int ch) {}
    virtual void		handleResetAllControllers(int ch) {}

    virtual void        handleModWheelChange( int ch, int value ) = 0;
    virtual void        handleDamperChange( int ch, bool on ) = 0;
    virtual void        handleVolumeChange( int ch, int value ) = 0;
    virtual void        handleExpressionChange( int ch, int value ) = 0;
    virtual void        handlePanpotChange( int ch, int value ) = 0;
    
    virtual void        handlePortamentOnChange( int ch, bool on ) = 0;
    virtual void        handlePortaTimeChange( int ch, int ccValue, float centPerMilis ) = 0;
    virtual void        handlePortamentStartNoteChange( int ch, int note ) = 0;
    
    virtual void        handleDataEntryValueChange(int ch, bool isNRPN, int addr, int value);

    virtual int         getKeyOnPriority(int ch, int note) = 0;
    virtual int         getReleasePriority(int ch, int note) = 0;
    virtual bool        isMonoMode(int ch, int note) = 0;
    virtual bool        isPatchLoaded(int ch, int note) = 0;
    virtual float       getPortamentFreq() = 0;    // ポルタメント処理の１秒間の解像度
    
    void                calcPanVolume(int value, int *volL, int *volR);
    float               calcGM2PortamentCurve(int value);
    
    ChannelStatus       mChStat[16];
    DynamicVoiceManager mVoiceManager;

private:
    MutexObject         mMIDIEvtMtx;
    std::list<MIDIEvt>	mMIDIEvt;			//受け取ったイベントのキュー
    std::list<MIDIEvt>	mDelayedEvt;		//遅延実行イベントのキュー
    int                 mNoteOffIntervalCycles;
    
    // RPN, NRPN
    void                parseRPNLSB(int ch, int value);
    void                parseRPNMSB(int ch, int value);
    void                parseNRPNLSB(int ch, int value);
    void                parseNRPNMSB(int ch, int value);
    void                parseDataEntryLSB(int ch, int value);
    void                parseDataEntryMSB(int ch, int value);

    bool                doImmediateEvents( const MIDIEvt *evt );
    bool                doDelayedEvents( const MIDIEvt *evt );
};

#endif /* MidiDriverBase_h */
