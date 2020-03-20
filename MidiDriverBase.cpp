//
//  MidiDriverBase.cpp
//  C700
//
//  Created by osoumen on 2017/06/10.
//  MIDIの規格内の処理はなるべくこちらで処理する
//

#include "MidiDriverBase.h"

const float onepi = 3.14159265358979;

static const int	VOLUME_CURB[128]
= {
    0x000, 0x001, 0x001, 0x001, 0x002, 0x003, 0x005, 0x006, 0x008, 0x00a, 0x00d, 0x00f, 0x012, 0x015, 0x019, 0x01d,
    0x020, 0x025, 0x029, 0x02e, 0x033, 0x038, 0x03d, 0x043, 0x049, 0x04f, 0x056, 0x05d, 0x064, 0x06b, 0x072, 0x07a,
    0x082, 0x08a, 0x093, 0x09b, 0x0a4, 0x0ae, 0x0b7, 0x0c1, 0x0cb, 0x0d5, 0x0e0, 0x0eb, 0x0f6, 0x101, 0x10d, 0x118,
    0x124, 0x131, 0x13d, 0x14a, 0x157, 0x165, 0x172, 0x180, 0x18e, 0x19c, 0x1ab, 0x1ba, 0x1c9, 0x1d8, 0x1e8, 0x1f8,
    0x208, 0x218, 0x229, 0x23a, 0x24b, 0x25c, 0x26e, 0x280, 0x292, 0x2a4, 0x2b7, 0x2ca, 0x2dd, 0x2f0, 0x304, 0x318,
    0x32c, 0x341, 0x355, 0x36a, 0x380, 0x395, 0x3ab, 0x3c1, 0x3d7, 0x3ed, 0x404, 0x41b, 0x432, 0x44a, 0x461, 0x479,
    0x492, 0x4aa, 0x4c3, 0x4dc, 0x4f5, 0x50f, 0x528, 0x542, 0x55d, 0x577, 0x592, 0x5ad, 0x5c8, 0x5e4, 0x600, 0x61c,
    0x638, 0x655, 0x671, 0x68e, 0x6ac, 0x6c9, 0x6e7, 0x705, 0x724, 0x742, 0x761, 0x780, 0x79f, 0x7bf, 0x7df, 0x7ff
};

static const int PAN_CURB[129]={
    511, 511, 510, 510, 510, 510, 510, 509, 509, 508, 507, 507, 506, 505, 504, 503,
    502, 500, 499, 498, 496, 495, 493, 491, 490, 488, 486, 484, 482, 480, 477, 475,
    473, 470, 468, 465, 463, 460, 457, 454, 451, 448, 445, 442, 439, 436, 432, 429,
    425, 422, 418, 414, 411, 407, 403, 399, 395, 391, 387, 383, 378, 374, 370, 365,
    361, 356, 352, 347, 342, 338, 333, 328, 323, 318, 313, 308, 303, 298, 293, 287,
    282, 277, 271, 266, 260, 255, 249, 244, 238, 233, 227, 221, 215, 210, 204, 198,
    192, 186, 180, 174, 168, 162, 156, 150, 144, 138, 132, 126, 119, 113, 107, 101,
    95, 88, 82, 76, 69, 63, 57, 50, 44, 38, 31, 25, 19, 12, 6, 0, 0
};

//-----------------------------------------------------------------------------
void Lfo_Linear::SetUpdateRate(int rate)
{
    mUpdateRate = rate;
}

//-----------------------------------------------------------------------------
void Lfo_Linear::Reset()
{
    mVibsens = 0;
    mVibPhase = 0;
}

//-----------------------------------------------------------------------------
void Lfo_Linear::SetVibFreq( float value )
{
    mVibfreq = value * ((onepi*2) / mUpdateRate);
}

//-----------------------------------------------------------------------------
void Lfo_Linear::SetVibDepth( float value )
{
    mVibdepth = value / 2;
}

//-----------------------------------------------------------------------------
void Lfo_Linear::SetVibSens( int sens )
{
    mVibsens = sens;
}

//-----------------------------------------------------------------------------
int Lfo_Linear::Update(int pitch)
{
    mVibPhase += mVibfreq;
    if (mVibPhase > onepi) {
        mVibPhase -= onepi*2;
    }
    
    float vibwave = calcVibratoWave(mVibPhase);
    int pitchRatio = (vibwave*mVibdepth)*VOLUME_CURB[mVibsens];
    return pitchRatio;
}

//-----------------------------------------------------------------------------
void Lfo_Linear::ResetPhase()
{
    mVibPhase = .0f;
}

//-----------------------------------------------------------------------------
float Lfo_Linear::calcVibratoWave(float phase)
{
    float x2=phase*phase;
    float vibwave = 7.61e-03f;
    vibwave *= x2;
    vibwave -= 1.6605e-01f;
    vibwave *= x2;
    vibwave += 1.0f;
    vibwave *= phase;
    return vibwave;
}

//-----------------------------------------------------------------------------
float Portament_Linear::Update(float pitch)
{
    float   newPitch;

    if ( mTargetPitch > pitch) {
        newPitch = pitch * mTc;
        if (newPitch > mTargetPitch) {
            newPitch = mTargetPitch;
        }
    }
    else if ( mTargetPitch < pitch) {
        newPitch = pitch / mTc;
        if (newPitch < mTargetPitch) {
            newPitch = mTargetPitch;
        }
    }
    else {
        newPitch = pitch;
    }
    return newPitch;
}


//-----------------------------------------------------------------------------
MidiDriverBase::MidiDriverBase(int maxVoices)
{
    MutexInit(mMIDIEvtMtx);
    
    for (int i=0; i<16; i++) {
        mChStat[i].prog = 0;
        mChStat[i].pitchBend = 0;
        mChStat[i].vibDepth = 0;
        mChStat[i].pbRange = static_cast<float>(DEFAULT_PBRANGE);
        mChStat[i].portaOn = false;
        //mChStat[i].portaTc = 1.0f;
        mChStat[i].portaStartNote = 0;
        mChStat[i].volume = VOLUME_DEFAULT;
        mChStat[i].expression = EXPRESSION_DEFAULT;
        mChStat[i].pan = 64;
        mChStat[i].damper = false;
        
        mChStat[i].lastNote = 0;
        
        mChStat[i].dataEntryValue = 0;
        mChStat[i].rpn = 0x7f7f;
        mChStat[i].nrpn = 0x7f7f;
        mChStat[i].isSettingNRPN = false;
    }
    
    mNoteOffIntervalCycles = 0;
    
    mVoiceManager.Initialize(maxVoices);
    
    Reset();
}

//-----------------------------------------------------------------------------
MidiDriverBase::~MidiDriverBase()
{
    MutexDestroy(mMIDIEvtMtx);
}

//-----------------------------------------------------------------------------
void MidiDriverBase::Reset()
{
    mVoiceManager.Reset();
    handleAllNotesOff();    // コンストラクタ内ではサブクラスのは呼ばれない
}

//-----------------------------------------------------------------------------
void MidiDriverBase::AllSoundOff(int ch)
{
    mVoiceManager.ReleaseAllVoices(ch);
    handleAllSoundOff(ch);
}

//-----------------------------------------------------------------------------
void MidiDriverBase::ResetAllControllers(int ch)
{
    mChStat[ch].pitchBend = 0;
    mChStat[ch].vibDepth = 0;
    mChStat[ch].pbRange = static_cast<float>(DEFAULT_PBRANGE);
    mChStat[ch].expression = EXPRESSION_DEFAULT;
    //mChStat[ch].pan = 64;
    mChStat[ch].portaOn = false;
    //mChStat[ch].portaTc = 1.0f;
    mChStat[ch].portaStartNote = 0;
    mChStat[ch].damper = false;
    //mChStat[i].monoMode = false;

    handleResetAllControllers(ch);
}

//-----------------------------------------------------------------------------
void MidiDriverBase::AllNotesOff()
{
    mVoiceManager.Reset();
    handleAllNotesOff();
}

//-----------------------------------------------------------------------------
void MidiDriverBase::EnqueueMidiEvent(const MIDIEvt *evt)
{
    MutexLock(mMIDIEvtMtx);
    mMIDIEvt.push_back( *evt );
    MutexUnlock(mMIDIEvtMtx);
}

//-----------------------------------------------------------------------------
void MidiDriverBase::ProcessMidiEvents()
{
    
    //イベント処理
    MutexLock(mMIDIEvtMtx);
    if ( mMIDIEvt.size() != 0 ) {
        std::list<MIDIEvt>::iterator	it = mMIDIEvt.begin();
        while ( it != mMIDIEvt.end() ) {
            if ( it->toWaitCycles >= 0 ) {
                it->toWaitCycles--;
            }
            if ( it->toWaitCycles < 0 ) {
                if (doImmediateEvents(&(*it))) {
                    it = mMIDIEvt.erase( it );
                    continue;
                }
            }
            it++;
        }
    }
    MutexUnlock(mMIDIEvtMtx);
    if ( mDelayedEvt.size() != 0 ) {
        std::list<MIDIEvt>::iterator	it = mDelayedEvt.begin();
        while ( it != mDelayedEvt.end() ) {
            if ( it->toWaitCycles >= 0 ) {
                it->toWaitCycles--;
            }
            if ( it->toWaitCycles < 0 ) {
                if (doDelayedEvents(&(*it))) {
                    it = mDelayedEvt.erase( it );
                    continue;
                }
            }
            it++;
        }
    }
}

//-----------------------------------------------------------------------------
bool MidiDriverBase::doImmediateEvents( const MIDIEvt *evt )
{
    bool    handled = true;
    
    if (evt->type == NOTE_OFF) {
        if (mChStat[evt->ch].damper) {
            // ノートオフが保留される
            handled = false;
        }
        else {
            int	stops=0;
            
            // 再生中なら停止する
            // 再生中でも未再生でも同じ処理で止められるはず
            int             vo=-1;
            stops = mVoiceManager.ReleaseVoice(getReleasePriority(evt->ch, evt->data1), evt->ch, evt->uniqueID, &vo);
            if (stops > 0) {
                handleNoteOff( evt, vo );
                //mChStat[evt->ch].noteOns -= stops;
            }
        }
    }
    else {
        // ノートオフとレジスタログ以外のイベントは全て遅延実行する
        MIDIEvt dEvt = *evt;
        dEvt.toWaitCycles = GetNoteOffIntervalCycles();
        
        if (dEvt.type == NOTE_ON) {
            bool legato = false;
            //ボイスを確保して再生準備状態にする
            int	v = isMonoMode(dEvt.ch, dEvt.data1) ? (dEvt.ch & 0x07):-1;
            
            int releasedCh;
            v = mVoiceManager.AllocVoice(getKeyOnPriority(dEvt.ch, dEvt.data1), dEvt.ch, dEvt.uniqueID,
                                         v, &releasedCh, &legato);
            if (legato) {
                dEvt.setLegato();  // レガートフラグ
            }
            
            if (v != -1) {
                handleNoteOnFirst(v, dEvt.ch, dEvt.data1, dEvt.data2, legato, releasedCh);
                // 上位4bitに確保したボイス番号を入れる
                dEvt.setAllocedVo(v);
            }
        }
        mDelayedEvt.push_back(dEvt);
        
    }
    
    return handled;
}

//-----------------------------------------------------------------------------
bool MidiDriverBase::doDelayedEvents( const MIDIEvt *evt )
{
    bool    handled = true;
    
    switch (evt->type) {
        case NOTE_OFF:
            break;
            
        case NOTE_ON:
        {
            int     midiCh = evt->ch & 0x0f;
            int     note = evt->data1 & 0x7f;
            
            //ボイスを取得
            int v = (evt->ch >> 4) & 0x0f;
            
            //mPlayVo に v が含まれていなかったら鳴らさない
            if (
                (mVoiceManager.IsPlayingVoice(v) == false) ||     // doNoteOn1を経ていない
                (mVoiceManager.GetVoiceUniqueID(v) != evt->uniqueID) //|| // 発音前にかき消されて上書きされた
                ) {
                break;
            }
            
            //波形データが存在しない場合は、ここで中断
            if (!isPatchLoaded(midiCh, note)) {
                break;
            }
            
            mVoiceManager.SetKeyOn(v);

            if (handleNoteOnDelayed( v, midiCh, note, evt->data2, evt->isLegato() )) {
                // handleNoteOnDelayedはキーオンされた場合はtrueを返す
                mChStat[midiCh].lastNote = note;
            }
            break;
        }
            
        case AFTER_TOUCH_POLY:
            handleAfterTouchPoly(evt->ch, evt->data1, evt->data2);
            break;
            
        case CONTROL_CHANGE:
            handleControlChange(evt->ch, evt->data1, evt->data2);
            break;
            
        case PROGRAM_CHANGE:
            mChStat[evt->ch].prog = evt->data1;
            handleProgramChange(evt->ch, evt->data1);
            break;
            
        case AFTER_TOUCH_CHANNEL:
            mChStat[evt->ch].afterTouch = evt->data1;
            handleAfterTouchChannel(evt->ch, evt->data1);
            break;
            
        case PITCH_BEND:
        {
            int pitchBend = ((evt->data2 << 7) | evt->data1) - 8192;
            mChStat[evt->ch].pitchBend = pitchBend;
            handlePitchBend(evt->ch, pitchBend);
            break;
        }
            
        default:
            break;
    }
    return handled;
}

//-----------------------------------------------------------------------------
void MidiDriverBase::handleControlChange( int ch, int controlNum, int value )
{
    switch (controlNum) {
        case 1:
            // モジュレーションホイール
            mChStat[ch].vibDepth = value;
            handleModWheelChange(ch, value);
            break;
            
        case 5:
        {
            // ポルタメントタイム
            float centPerMilis = calcGM2PortamentCurve(value);
            centPerMilis *= 1000.0f / getPortamentFreq();
            handlePortaTimeChange( ch, value, centPerMilis );
            break;
        }
        case 7:
            // ボリューム
            mChStat[ch].volume = value & 0x7f;
            handleVolumeChange(ch, value);
            break;
            
        case 10:
            // パン
            mChStat[ch].pan = value & 0x7f;
            handlePanpotChange(ch, value);
            break;
            
        case 11:
            // エクスプレッション
            mChStat[ch].expression = value & 0x7f;
            handleExpressionChange(ch, value);
            break;
            
        case 55:
            // チャンネル リミット
            changeChLimit(ch, value);
            break;
            
        case 64:
            // ホールド１（ダンパー）
            mChStat[ch].damper = (value < 64)?false:true;
            handleDamperChange(ch, mChStat[ch].damper);
            break;
            
        case 65:
            // ポルタメント・オン・オフ
            mChStat[ch].portaOn = (value < 64)?false:true;
            handlePortamentOnChange(ch, mChStat[ch].portaOn);
            break;
            
        case 76:
            // ビブラート・レート
            //SetVibFreq(ch, (35.0f * value) / 127);
            break;
            
        case 77:
            // ビブラート・デプス
            //SetVibDepth(ch, (15.0f * value) / 127);
            break;
            
        case 84:
            // ポルタメント・コントロール
            mChStat[ch].portaStartNote = value;
            handlePortamentStartNoteChange(ch, value);
            break;
            
        case 6:
            //データ・エントリー(LSB)
            parseDataEntryLSB(ch, value);
            break;
            
        case 38:
            // データ・エントリー(MSB)
            parseDataEntryMSB(ch, value);
            break;
            
        case 98:
            // NRPN (LSB)
            parseNRPNLSB(ch, value);
            break;
            
        case 99:
            // NRPN (MSB)
            parseNRPNMSB(ch, value);
            break;
            
        case 100:
            // RPN (LSB)
            parseRPNLSB(ch, value);
            break;
            
        case 101:
            // RPN (MSB)
            parseRPNMSB(ch, value);
            break;
            
        case 120://Force off
            AllSoundOff(ch);
            break;
            
        case 121:
            ResetAllControllers(ch);
            break;
            
        case 123://All Note Off
            AllNotesOff();
            break;
            
        default:
            break;
    }
}

//-----------------------------------------------------------------------------
void MidiDriverBase::changeChLimit( int ch, int value )
{
    mVoiceManager.SetChLimit(ch, value);
}

//-----------------------------------------------------------------------------
float MidiDriverBase::calcGM2PortamentCurve(int value)
{
    float   logCentPerMilis;
    if (value < 16) {
        logCentPerMilis = (value * value)/192.0f - (3.0f/16.0f)*value + 3.0f;
    }
    else if (value < 112) {
        logCentPerMilis = -value/48.0f + 5.0f/3.0f;
    }
    else {
        logCentPerMilis = -(value * value)/256.0f + (41.0f/48.0f)*value - 142.0f/3.0f;
    }
    return powf(10.0f, logCentPerMilis);
}

//-----------------------------------------------------------------------------
void MidiDriverBase::parseRPNLSB(int ch, int value)
{
    mChStat[ch].rpn &= 0xff00;
    mChStat[ch].rpn |= value & 0x7f;
    mChStat[ch].isSettingNRPN = false;
}

//-----------------------------------------------------------------------------
void MidiDriverBase::parseRPNMSB(int ch, int value)
{
    mChStat[ch].rpn &= 0x00ff;
    mChStat[ch].rpn |= (value & 0x7f) << 8;
    mChStat[ch].isSettingNRPN = false;
}

//-----------------------------------------------------------------------------
void MidiDriverBase::parseNRPNLSB(int ch, int value)
{
    mChStat[ch].nrpn &= 0xff00;
    mChStat[ch].nrpn |= value & 0x7f;
    mChStat[ch].isSettingNRPN = true;
}

//-----------------------------------------------------------------------------
void MidiDriverBase::parseNRPNMSB(int ch, int value)
{
    mChStat[ch].nrpn &= 0x00ff;
    mChStat[ch].nrpn |= (value & 0x7f) << 8;
    mChStat[ch].isSettingNRPN = true;
}

//-----------------------------------------------------------------------------
void MidiDriverBase::parseDataEntryLSB(int ch, int value)
{
    mChStat[ch].dataEntryValue &= 0xff00;
    mChStat[ch].dataEntryValue |= value & 0x7f;
    handleDataEntryValueChange(ch, mChStat[ch].isSettingNRPN, mChStat[ch].isSettingNRPN?mChStat[ch].nrpn:mChStat[ch].rpn, mChStat[ch].dataEntryValue);
}

//-----------------------------------------------------------------------------
void MidiDriverBase::parseDataEntryMSB(int ch, int value)
{
    mChStat[ch].dataEntryValue &= 0x00ff;
    mChStat[ch].dataEntryValue |= (value & 0x7f) << 8;
}

//-----------------------------------------------------------------------------
void MidiDriverBase::handleDataEntryValueChange(int ch, bool isNRPN, int addr, int value)
{
    if (isNRPN) {
    }
    else  {
        switch (addr) {
            case 0x0000:
                mChStat[ch].pbRange = value;
                break;
        }
    }
}

//-----------------------------------------------------------------------------
void MidiDriverBase::calcPanVolume(int value, int *volL, int *volR)
{
    *volL = (*volL * PAN_CURB[value]) / 511;
    *volR = (*volR * PAN_CURB[128-value]) / 511;
}
