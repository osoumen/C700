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
#include "DynamicVoiceAllocator.h"
#include <list>

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef short sint16_t;
typedef unsigned int uint32_t;

#define    VOLUME_DEFAULT 100
#define    EXPRESSION_DEFAULT 127
#define    DEFAULT_PBRANGE 2


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
        NOTE_OFF	        = 0x80,
        NOTE_ON             = 0x90,
        AFTER_TOUCH_POLY    = 0xa0,
        CONTROL_CHANGE      = 0xb0,
        PROGRAM_CHANGE      = 0xc0,
        AFTER_TOUCH_CHANNEL = 0xd0,
        PITCH_BEND          = 0xe0,
        SYSTEM_EXCLUSIVE    = 0xf0,
        MTC_QUARTER_FRAME   = 0xf1,
        SONG_POSITION       = 0xf2,
        SONG_SELECT         = 0xf3,
        TUNE_REQUEST        = 0xf6,
        MIDI_CLOCK          = 0xf8,
        MIDI_START          = 0xfa,
        MIDI_CONTINUE       = 0xfb,
        MIDI_STOP           = 0xfc,
        ACTIVE_SENSING      = 0xfe,
        MIDI_SYSTEM_RESET   = 0xff,
        INVALID_TYPE        = 0x00,
    };
    
    typedef struct MIDIEvt {
        EvtType     type;
        uint8_t     ch;
        uint8_t     data1;
        uint8_t     data2;
        uint32_t	uniqueID;
        int			toWaitCycles;
        void setLegato() { data1 |= 0x80; }
        bool isLegato() const { return (data1&0x80)!=0 ? true:false; }
        void setAllocedVo(int vo) { ch = (ch & 0x0f) | ((vo & 0x0f) << 4); }
        int getAllocedVo() const { return (ch & 0xf0) >> 4; }
    };
    
    struct ChannelStatus {
        int     prog;
        int     pitchBend;
        int     afterTouch;
        int     vibDepth;
        int     volume;
        int     expression;
        int     pan;
        int     pbRange;
        bool    portaOn;
        int     portaStartNote;
        int     lastNote;
        bool    damper;
        int     dataEntryValue;
        bool    isSettingNRPN;
        int     rpn;
        int     nrpn;
    };
    
    MidiDriverBase(int maxVoices=8);
    virtual ~MidiDriverBase();
    
    // 外部から呼び出すための関数
    virtual void		Reset();
    void                AllSoundOff(int ch);
    void                ResetAllControllers(int ch);
    void                AllNotesOff();
    virtual void        doPreMidiEvents() {}
    void                ProcessMidiEvents();
    virtual void        doPostMidiEvents() {}
    
    // MIDIイベントを入力する
    void                EnqueueMidiEvent(const MIDIEvt *evt);
    
    // 外部から呼び出して動作状態を変更する関数
    void                SetNoteOffIntervalCycles(int cycles) { mNoteOffIntervalCycles = cycles; }
    int                 GetNoteOffIntervalCycles() { return mNoteOffIntervalCycles; }
    void                changeChLimit( int ch, int value );
    
    // 外部から呼び出して動作状態を取得する関数
    int                 GetNoteOnNotes(int ch) { return mVoiceManager.GetNoteOns(ch); }
    int                 GetVoiceMidiCh(int vo) { return mVoiceManager.GetVoiceMidiCh(vo); }
    bool                IsKeyOnVoice(int vo) { return mVoiceManager.IsKeyOn(vo); }
    int                 GetVoiceLimit() { return mVoiceManager.GetVoiceLimit(); }

protected:
    // channel messagesに対する処理をサブクラスで実装する
    // controlChangeはサブクラス側で処理できなかった場合はMidiDriverBase側に渡すべし
    virtual void        handleNoteOff( const MIDIEvt *evt, int vo ) = 0;
    virtual bool        handleNoteOnFirst( uint8_t vo, uint8_t midiCh, uint8_t note, uint8_t velo, bool isLegato, int killedMidiCh ) = 0;
    virtual bool        handleNoteOnDelayed(uint8_t vo, uint8_t midiCh, uint8_t note, uint8_t velo, bool isLegato) = 0;
    virtual void        handleAfterTouchPoly( int ch, int note, int value ) = 0;
    virtual void        handleControlChange( int ch, int controlNum, int value );
    virtual void        handleProgramChange( int ch, int value ) = 0;
    virtual void        handleAfterTouchChannel( int ch, int value ) = 0;
    virtual void        handlePitchBend( int ch, sint16_t pitchbend ) = 0;
    
    
    // control changesに対する処理をサブクラスで実装する
    // CCでいじれるのはリアルタイムコントロールが必要なものが中心で、基本パッチの改変はさせない
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

    // MidiDriverBase内の処理で必要だけど値はサブクラスで持っていた方が良さそうなもの
    virtual int         getKeyOnPriority(int ch, int note) = 0;
    virtual int         getReleasePriority(int ch, int note) = 0;
    virtual bool        isMonoMode(int ch, int note) = 0;
    virtual bool        isPatchLoaded(int ch, int note) = 0;
    virtual float       getPortamentFreq() = 0;    // ポルタメント処理の１秒間の解像度
    
    // ユーティリティ関数
    void                calcPanVolume(int value, int *volL, int *volR);
    float               calcGM2PortamentCurve(int value);
    
    // おそらく使用頻度が高いのでサブクラスにも開放する
    ChannelStatus       mChStat[16];
    DynamicVoiceAllocator mVoiceManager;

private:
    MutexObject         mMIDIEvtMtx;
    std::list<MIDIEvt>	mMIDIEvt;			//受け取ったイベントのキュー
    std::list<MIDIEvt>	mDelayedEvt;		//遅延実行イベントのキュー
    int                 mNoteOffIntervalCycles;
    
    // RPN, NRPNなど内部的に呼び出される関数
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
