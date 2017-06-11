//
//  MidiDriverBase.cpp
//  C700
//
//  Created by osoumen on 2017/06/10.
//  MIDIの規格内の処理はなるべくこちらで処理する
//

#include "MidiDriverBase.h"

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
float Portament_Linear::processPortament(float pitch)
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
    for (int i=0; i<16; i++) {
        mDataEntryValue[i] = 0;
        mRPN[i] = 0x7f7f;
        mNRPN[i] = 0x7f7f;
        mIsSettingNRPN[i] = false;
    }
    mVoiceManager.Reset();
    handleAllSoundOff();
}

//-----------------------------------------------------------------------------
void MidiDriverBase::AllSoundOff()
{
    mVoiceManager.Reset();
    handleAllSoundOff();
}

//-----------------------------------------------------------------------------
void MidiDriverBase::ResetAllControllers()
{
    for (int i=0; i<16; i++) {
        mChStat[i].pitchBend = 0;
        mChStat[i].vibDepth = 0;
        mChStat[i].pbRange = static_cast<float>(DEFAULT_PBRANGE);
        mChStat[i].expression = EXPRESSION_DEFAULT;
        //mChStat[i].pan = 64;
        mChStat[i].portaOn = false;
        //mChStat[i].portaTc = 1.0f;
        mChStat[i].portaStartNote = 0;
        mChStat[i].damper = false;
        //mChStat[i].monoMode = false;
    }
    handleResetAllControllers();
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
            if ( it->remain_samples >= 0 ) {
                it->remain_samples--;
            }
            if ( it->remain_samples < 0 ) {
                if (doFirstEvents(&(*it))) {
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
            if ( it->remain_samples >= 0 ) {
                it->remain_samples--;
            }
            if ( it->remain_samples < 0 ) {
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
bool MidiDriverBase::doFirstEvents( const MIDIEvt *evt )
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
            stops = mVoiceManager.ReleaseVoice(getReleasePriority(evt->ch, evt->note), evt->ch, evt->uniqueID, &vo);
            if (stops > 0) {
                handleNoteOff( evt, vo );
                //mChStat[evt->ch].noteOns -= stops;
            }
        }
    }
    else {
        // ノートオフとレジスタログ以外のイベントは全て遅延実行する
        MIDIEvt dEvt = *evt;
        dEvt.remain_samples = GetNoteOffIntervalCycles();
        
        if (dEvt.type == NOTE_ON) {
            bool legato = false;
            //ボイスを確保して再生準備状態にする
            int	v = -1;
            
            int releasedCh;
            v = mVoiceManager.AllocVoice(getKeyOnPriority(dEvt.ch, dEvt.note), dEvt.ch, dEvt.uniqueID,
                                         isMonoMode(dEvt.ch, dEvt.note), &releasedCh, &legato);
            if (legato) {
                dEvt.setLegato();  // レガートフラグ
            }
            
            if (v != -1) {
                // 上位4bitに確保したボイス番号を入れる
                dEvt.setAllocedVo(v);
                handleNoteOnFirst(&dEvt, releasedCh);
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
        case NOTE_ON:
        {
            int     midiCh = evt->ch & 0x0f;
            int     note = evt->note & 0x7f;
            
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

            if (handleNoteOnDelayed( v, midiCh, note, evt->velo, evt->isLegato() )) {
                // handleNoteOnDelayedはキーオンされた場合はtrueを返す
                mChStat[midiCh].lastNote = note;
            }
            break;
        }
            
        case NOTE_OFF:
            //handleNoteOff( evt );
            break;
            
        case PROGRAM_CHANGE:
            mChStat[evt->ch].prog = evt->note;
            handleProgramChange(evt->ch, evt->note);
            break;
            
        case PITCH_BEND:
        {
            int pitchBend = ((evt->velo << 7) | evt->note) - 8192;
            mChStat[evt->ch].pitchBend = pitchBend;
            handlePitchBend(evt->ch, pitchBend);
            break;
        }
            
        case CONTROL_CHANGE:
            handleControlChange(evt->ch, evt->note, evt->velo);
            break;
            
        default:
            //handled = false;
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
            setDataEntryLSB(ch, value);
            break;
            
        case 38:
            // データ・エントリー(MSB)
            setDataEntryMSB(ch, value);
            break;
            
        case 98:
            // NRPN (LSB)
            setNRPNLSB(ch, value);
            break;
            
        case 99:
            // NRPN (MSB)
            setNRPNMSB(ch, value);
            break;
            
        case 100:
            // RPN (LSB)
            setRPNLSB(ch, value);
            break;
            
        case 101:
            // RPN (MSB)
            setRPNMSB(ch, value);
            break;
            
        case 120://Force off
            AllSoundOff();
            break;
            
        case 121:
            ResetAllControllers();
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
void MidiDriverBase::setPBRange( int ch, float value )
{
    mChStat[ch].pbRange = value;
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
void MidiDriverBase::handlePortamentStartNoteChange( int ch, int note )
{
}

//-----------------------------------------------------------------------------
void MidiDriverBase::setRPNLSB(int ch, int value)
{
    mRPN[ch] &= 0xff00;
    mRPN[ch] |= value & 0x7f;
    mIsSettingNRPN[ch] = false;
}

//-----------------------------------------------------------------------------
void MidiDriverBase::setRPNMSB(int ch, int value)
{
    mRPN[ch] &= 0x00ff;
    mRPN[ch] |= (value & 0x7f) << 8;
    mIsSettingNRPN[ch] = false;
}

//-----------------------------------------------------------------------------
void MidiDriverBase::setNRPNLSB(int ch, int value)
{
    mNRPN[ch] &= 0xff00;
    mNRPN[ch] |= value & 0x7f;
    mIsSettingNRPN[ch] = true;
}

//-----------------------------------------------------------------------------
void MidiDriverBase::setNRPNMSB(int ch, int value)
{
    mNRPN[ch] &= 0x00ff;
    mNRPN[ch] |= (value & 0x7f) << 8;
    mIsSettingNRPN[ch] = true;
}

//-----------------------------------------------------------------------------
void MidiDriverBase::setDataEntryLSB(int ch, int value)
{
    mDataEntryValue[ch] &= 0xff00;
    mDataEntryValue[ch] |= value & 0x7f;
    handleDataEntryValueChange(ch, mIsSettingNRPN[ch], mIsSettingNRPN[ch]?mNRPN[ch]:mRPN[ch], mDataEntryValue[ch]);
}

//-----------------------------------------------------------------------------
void MidiDriverBase::setDataEntryMSB(int ch, int value)
{
    mDataEntryValue[ch] &= 0x00ff;
    mDataEntryValue[ch] |= (value & 0x7f) << 8;
}

//-----------------------------------------------------------------------------
void MidiDriverBase::handleDataEntryValueChange(int ch, bool isNRPN, int addr, int value)
{
    if (isNRPN) {
    }
    else  {
        switch (addr) {
            case 0x0000:
                setPBRange(ch, value);
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
