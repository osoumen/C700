/*
 *  C700Generator.cpp
 *  C700
 *
 *  Created by osoumen on 06/09/06.
 *  Copyright 2006 Vermicelli Magic. All rights reserved.
 *
 */

#include "C700defines.h"
#include "C700Generator.h"
#include <math.h>
#include "gauss.h"

#define filter1(a1)	(( a1 >> 1 ) + ( ( -a1 ) >> 5 ))
#define filter2(a1,a2)	(a1 + ( ( -( a1 + ( a1 >> 1 ) ) ) >> 5 ) - ( a2 >> 1 ) + ( a2 >> 5 ))
#define filter3(a1,a2)	(a1  + ( ( -( a1 + ( a1 << 2 ) + ( a1 << 3 ) ) ) >> 7 )  - ( a2 >> 1 )  + ( ( a2 + ( a2 >> 1 ) ) >> 4 ))

const float onepi = 3.14159265358979;

#define ANALOG_PORTAMENTO 0

static const int	*G1 = &gauss[256];
static const int	*G2 = &gauss[512];
static const int	*G3 = &gauss[255];
static const int	*G4 = &gauss[0];

static const int	CNT_INIT = 0x7800;
static const int	ENVCNT[32]
= {
    0x0000, 0x000F, 0x0014, 0x0018, 0x001E, 0x0028, 0x0030, 0x003C,
    0x0050, 0x0060, 0x0078, 0x00A0, 0x00C0, 0x00F0, 0x0140, 0x0180,
    0x01E0, 0x0280, 0x0300, 0x03C0, 0x0500, 0x0600, 0x0780, 0x0A00,
    0x0C00, 0x0F00, 0x1400, 0x1800, 0x1E00, 0x2800, 0x3C00, 0x7800
};

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

static const int sinctable[] = {
	-8,31,0,-462,1911,-4438,7057,32767,7057,-4438,1911,-462,0,31,-8,0,
	0,18,46,-535,1858,-3768,4443,32510,9863,-4978,1871,-347,-57,45,-9,0,
	0,7,80,-572,1726,-3016,2076,31745,12796,-5345,1725,-193,-125,61,-10,0,
	0,0,104,-574,1532,-2226,0,30497,15785,-5492,1464,0,-200,78,-11,0,
	0,-5,117,-548,1294,-1437,-1755,28803,18752,-5378,1087,227,-279,93,-11,0,
	0,-9,120,-500,1029,-685,-3169,26715,21615,-4969,596,481,-360,107,-11,0,
	0,-11,116,-435,753,0,-4239,24296,24296,-4239,0,753,-435,116,-11,0,
	0,-11,107,-360,481,596,-4969,21615,26715,-3169,-685,1029,-500,120,-9,0,
	0,-11,93,-279,227,1087,-5378,18752,28803,-1755,-1437,1294,-548,117,-5,0,
	0,-11,78,-200,0,1464,-5492,15785,30497,0,-2226,1532,-574,104,0,0,
	0,-10,61,-125,-193,1725,-5345,12796,31745,2076,-3016,1726,-572,80,7,0,
	0,-9,45,-57,-347,1871,-4978,9863,32510,4443,-3768,1858,-535,46,18,0,
	0,-8,31,0,-462,1911,-4438,7057,32767,7057,-4438,1911,-462,0,31,-8
};

static unsigned char silence_brr[] = {
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

//-----------------------------------------------------------------------------
C700Generator::C700Generator()
: mSampleRate(44100.0),
  mNewADPCM( false ),
  mVelocityMode( kVelocityMode_Square ),
  mVPset(NULL)
{
	for ( int i=0; i<NUM_BANKS; i++ ) {
		mDrumMode[i] = false;
	}
	for ( int bnk=0; bnk<NUM_BANKS; bnk++ ) {
		for ( int i=0; i<128; i++ ) {
			mKeyMap[bnk][i] = 0;
		}
	}
	//Initialize
	mVoiceLimit = 8;
	mMainVolume_L = 127;
	mMainVolume_R = 127;
	mVibfreq = 0.00137445;
	mVibdepth = 0.5;
    mEventDelayClocks = 8192;   // 8ms
    mEventDelaySamples = calcEventDelaySamples();
    
	for (int i=0; i<16; i++) {
        mChStat[i].changeFlg = 0;
        mChStat[i].prog = 0;
        mChStat[i].pitchBend = 0;
        mChStat[i].vibDepth = 0;
        mChStat[i].pbRange = static_cast<float>(DEFAULT_PBRANGE);
        //mChStat[i].portaOn = false;
        mChStat[i].portaTc = 1.0f;
        mChStat[i].portaStartPitch = 0;
        mChStat[i].volume = VOLUME_DEFAULT;
        mChStat[i].expression = EXPRESSION_DEFAULT;
        mChStat[i].pan = 64;
        //mChStat[i].priority = 64;
        mChStat[i].limit = 127;
        //mChStat[i].releasePriority = 0;
        mChStat[i].damper = false;
        //mChStat[i].monoMode = false;
        
        mChStat[i].lastNote = 0;
        mChStat[i].noteOns = 0;
	}
	Reset();
}

//-----------------------------------------------------------------------------
void C700Generator::VoiceState::Reset()
{
	midi_ch = 0;
	uniqueID = 0;
    priority = 0;
    
    isKeyOn = false;
    legato = false;
	
	smp1=0;
	smp2=0;
	sampbuf[0]=0;
	sampbuf[1]=0;
	sampbuf[2]=0;
	sampbuf[3]=0;
	
	pb = 0;
	reg_pmod = 0;
	vibdepth = 0;
	vibPhase = 0.0f;
    portaPitch = .0f;
    pan = 64;
	
	brrdata = silence_brr;
	loopPoint = 0;
	loop = false;
	pitch = 0;
	memPtr = 0;
	headerCnt = 0;
	half = 0;
	envx = 0;
	end = 0;
	sampptr = 0;
	mixfrac=0;
	envcnt = CNT_INIT;
	envstate = RELEASE;	
}

//-----------------------------------------------------------------------------
void C700Generator::Reset()
{
	for (int i=0; i<16; i++) {
		mProcessbuf[0][i]=0;
		mProcessbuf[1][i]=0;
	}
	mProcessFrac=0;
	mProcessbufPtr=0;
    mPortamentCount=0;
	
	mPlayVo.clear();
	mWaitVo.clear();
	for(int i=0;i<mVoiceLimit;i++){
		mWaitVo.push_back(i);
	}
    
    AllSoundOff();
}

//-----------------------------------------------------------------------------
void C700Generator::KeyOn( int ch, int note, int velo, unsigned int uniqueID, int inFrame )
{
	MIDIEvt			evt;
	evt.type = NOTE_ON;
	evt.ch = ch;
	evt.note = note;
	evt.velo = velo;
	evt.uniqueID = uniqueID;
	evt.remain_samples = inFrame;
	mMIDIEvt.push_back( evt );
}

//-----------------------------------------------------------------------------
void C700Generator::KeyOff( int ch, int note, int velo, unsigned int uniqueID, int inFrame )
{
	MIDIEvt			evt;
	evt.type = NOTE_OFF;
	evt.ch = ch;
	evt.note = note;
	evt.velo = velo;
	evt.uniqueID = uniqueID;
	evt.remain_samples = inFrame;
	mMIDIEvt.push_back( evt );
}


//-----------------------------------------------------------------------------
void C700Generator::ControlChange( int ch, int controlNum, int value, int inFrame )
{
    MIDIEvt			evt;
	evt.type = CONTROL_CHANGE;
	evt.ch = ch;
	evt.note = controlNum;
	evt.velo = value;
	evt.uniqueID = 0;
	evt.remain_samples = inFrame;
	mMIDIEvt.push_back( evt );
}

//-----------------------------------------------------------------------------
void C700Generator::AllNotesOff()
{
    //mClearEvent = true;
	for ( int i=0; i<kMaximumVoices; i++ ) {
		mVoice[i].Reset();
	}
    for (int i=0; i<16; i++) {
        mChStat[i].noteOns = 0;
    }
}

//-----------------------------------------------------------------------------
void C700Generator::AllSoundOff()
{
    AllNotesOff();
	mEcho[0].Reset();
	mEcho[1].Reset();	
}

//-----------------------------------------------------------------------------
void C700Generator::ResetAllControllers()
{
	for (int i=0; i<16; i++) {
        mChStat[i].changeFlg = 0;
        mChStat[i].changedVP = mVPset[mChStat[i].prog];
        mChStat[i].pitchBend = 0;
        mChStat[i].vibDepth = 0;
        mChStat[i].pbRange = static_cast<float>(DEFAULT_PBRANGE);
        mChStat[i].expression = EXPRESSION_DEFAULT;
        //mChStat[i].pan = 64;
        //mChStat[i].portaOn = false;
        //mChStat[i].portaTc = 1.0f;
        mChStat[i].portaStartPitch = 0;
        //mChStat[i].priority = 64;
        //mChStat[i].limit = 127;
        //mChStat[i].releasePriority = 0;
        mChStat[i].damper = false;
        //mChStat[i].monoMode = false;
	}
}

//-----------------------------------------------------------------------------
void C700Generator::ProgramChange( int ch, int value, int inFrame )
{
    MIDIEvt			evt;
	evt.type = PROGRAM_CHANGE;
	evt.ch = ch;
	evt.note = value;
	evt.velo = 0;
	evt.uniqueID = 0;
	evt.remain_samples = inFrame;
	mMIDIEvt.push_back( evt );
}
//-----------------------------------------------------------------------------
void C700Generator::doProgramChange( int ch, int value )
{
	mChStat[ch].prog = value;
    if (mVPset) {
        mChStat[ch].changeFlg = 0;
        mChStat[ch].changedVP = mVPset[mChStat[ch].prog];
        
        SetPortamentTime(ch, value);
    }
}

//-----------------------------------------------------------------------------
int C700Generator::CalcPBValue( int ch, float pitchBend, int basePitch )
{
	return (int)((pow(2., (pitchBend * mChStat[ch].pbRange) / 12.) - 1.0)*basePitch);
}

//-----------------------------------------------------------------------------
void C700Generator::PitchBend( int ch, int value1, int value2, int inFrame )
{
    MIDIEvt			evt;
	evt.type = PITCH_BEND;
	evt.ch = ch;
	evt.note = value1;
	evt.velo = value2;
	evt.uniqueID = 0;
	evt.remain_samples = inFrame;
	mMIDIEvt.push_back( evt );
}

//-----------------------------------------------------------------------------
void C700Generator::doPitchBend( int ch, int value1, int value2 )
{
    int pitchBend = ((value2 << 7) | value1) - 8192;
	float pb_value = pitchBend / 8192.0;
	
    mChStat[ch].pitchBend = pb_value;
	for ( int i=0; i<kMaximumVoices; i++ ) {
		if ( mVoice[i].midi_ch == ch ) {
			mVoice[i].pb = CalcPBValue( ch, pb_value, mVoice[i].pitch );
		}
	}
}

//-----------------------------------------------------------------------------
void C700Generator::ModWheel( int ch, int value )
{
    mChStat[ch].vibDepth = value;
	for ( int i=0; i<kMaximumVoices; i++ ) {
		if ( mVoice[i].midi_ch == ch ) {
			mVoice[i].vibdepth = value;
			mVoice[i].reg_pmod = value > 0 ? true:false;
		}
	}
}

//-----------------------------------------------------------------------------
void C700Generator::Damper( int ch, bool on )
{
    mChStat[ch].damper = on;
}

//-----------------------------------------------------------------------------
void C700Generator::SetVoiceLimit( int value )
{
	if ( value < mVoiceLimit ) {
		//空きボイスリストから削除する
		for ( int i=value; i<mVoiceLimit; i++ ) {
			mWaitVo.remove(i);
		}
	}
	if ( value > mVoiceLimit ) {
		//空きボイスを追加する
		for ( int i=mVoiceLimit; i<value; i++ ) {
			mWaitVo.push_back(i);
		}
	}
	mVoiceLimit = value;
}

//-----------------------------------------------------------------------------
void C700Generator::SetPBRange( float value )
{
    for (int i=0; i<16; i++) {
        mChStat[i].pbRange = value;
    }
}
//-----------------------------------------------------------------------------
void C700Generator::SetPBRange( int ch, float value )
{
	mChStat[ch].pbRange = value;
}

//-----------------------------------------------------------------------------
void C700Generator::SetADPCMMode( bool value )
{
	mNewADPCM = value;
}

//-----------------------------------------------------------------------------
void C700Generator::SetMultiMode( int bank, bool value )
{
	mDrumMode[bank] = value;
}

//-----------------------------------------------------------------------------
bool C700Generator::GetMultiMode( int bank ) const
{
	return mDrumMode[bank];
}

//-----------------------------------------------------------------------------
void C700Generator::SetVelocityMode( velocity_mode value )
{
	mVelocityMode = value;
}

//-----------------------------------------------------------------------------
void C700Generator::SetVibFreq( int ch, float value )
{
	mVibfreq = value*((onepi*2)/INTERNAL_CLOCK);
}

//-----------------------------------------------------------------------------
void C700Generator::SetVibDepth( int ch, float value )
{
	mVibdepth = value / 2;
}

//-----------------------------------------------------------------------------
void C700Generator::SetMainVol_L( int value )
{
	mMainVolume_L = value;
}

//-----------------------------------------------------------------------------
void C700Generator::SetMainVol_R( int value )
{
	mMainVolume_R = value;
}

//-----------------------------------------------------------------------------
void C700Generator::SetEchoVol_L( int value )
{
	mEcho[0].SetEchoVol( value );
}

//-----------------------------------------------------------------------------
void C700Generator::SetEchoVol_R( int value )
{
	mEcho[1].SetEchoVol( value );
}

//-----------------------------------------------------------------------------
void C700Generator::SetFeedBackLevel( int value )
{
	mEcho[0].SetFBLevel( value );
	mEcho[1].SetFBLevel( value );
}

//-----------------------------------------------------------------------------
void C700Generator::SetDelayTime( int value )
{
	mEcho[0].SetDelayTime( value );
	mEcho[1].SetDelayTime( value );
}

//-----------------------------------------------------------------------------
void C700Generator::SetFIRTap( int tap, int value )
{
	mEcho[0].SetFIRTap(tap, value);
	mEcho[1].SetFIRTap(tap, value);
}

//-----------------------------------------------------------------------------
void C700Generator::SetPortamentOn( int ch, bool on )
{
    //mChStat[ch].portaOn = on;
    mChStat[ch].changedVP.portamentoOn = on;
    mChStat[ch].changeFlg |= HAS_PORTAMENTOON;
}

//-----------------------------------------------------------------------------
float C700Generator::calcGM2PortamentCurve(int value)
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
void C700Generator::SetPortamentTime( int ch, int value )
{
#if ANALOG_PORTAMENTO
    float secs = value / 100.0f;
    mChStat[ch].portaTc = expf(-2.0f / ((INTERNAL_CLOCK / PORTAMENT_CYCLE_SAMPLES) * secs));
#else
    float centPerMilis = calcGM2PortamentCurve(value);
    centPerMilis *= 1000.0f / (static_cast<float>(INTERNAL_CLOCK) / PORTAMENT_CYCLE_SAMPLES);
    mChStat[ch].portaTc = powf(2.0f, centPerMilis / 1200);
#endif
}

//-----------------------------------------------------------------------------
void C700Generator::UpdatePortamentoTime( int prog )
{
    if (mVPset == NULL) {
        return;
    }
    
    for (int i=0; i<16; i++) {
        if (mChStat[i].prog == prog) {
            SetPortamentTime(i, mVPset[prog].portamentoRate);
        }
    }
}

//-----------------------------------------------------------------------------
void C700Generator::SetPortamentControl( int ch, int note )
{
    InstParams		vp = getChannelVP(ch, mChStat[ch].lastNote);
    mChStat[ch].portaStartPitch = pow(2., (note - vp.basekey) / 12.)/INTERNAL_CLOCK*vp.rate*4096 + 0.5;
}

//-----------------------------------------------------------------------------
void C700Generator::SetChPriority( int ch, int value )
{
    //mChStat[ch].priority = value;
    mChStat[ch].changedVP.noteOnPriority = value;
    mChStat[ch].changeFlg |= HAS_NOTEONPRIORITY;
}

//-----------------------------------------------------------------------------
void C700Generator::SetChLimit( int ch, int value )
{
    mChStat[ch].limit = value;
}

//-----------------------------------------------------------------------------
void C700Generator::SetReleasePriority( int ch, int value )
{
    //mChStat[ch].releasePriority = value;
    mChStat[ch].changedVP.releasePriority = value;
    mChStat[ch].changeFlg |= HAS_RELEASEPRIORITY;
}

//-----------------------------------------------------------------------------
void C700Generator::SetMonoMode( int ch, bool on )
{
    //mChStat[ch].monoMode = on;
    mChStat[ch].changedVP.monoMode = on;
    mChStat[ch].changeFlg |= HAS_MONOMODE;
}

//-----------------------------------------------------------------------------
int C700Generator::FindFreeVoice()
{
	int	v=-1;

	//空きボイスを探す
    if ( mWaitVo.size() > 0 ) {
		v = mWaitVo.front();
		mWaitVo.pop_front();
	}
    return v;
}

//-----------------------------------------------------------------------------
int C700Generator::StealVoice(int prio)
{
    // MIDIch無視 低優先度
    
    int v=-1;
    int prio_min = 0x7fff;
    
    std::list<int>::reverse_iterator  it = mPlayVo.rbegin();
    while (it != mPlayVo.rend()) {
        int vo = *it;
        if (mVoice[vo].priority <= prio_min) {
            prio_min = mVoice[vo].priority;
            v = vo;
        }
        it++;
    }
    // 鳴らす音より高い優先度の音しか無かったら鳴らさない
    if (prio_min > prio) {
        v = -1;
    }
    
    return v;
}

//-----------------------------------------------------------------------------
int C700Generator::StealVoice(int ch, int prio)
{
    int v=-1;
    int prio_min = 0x7fff;
    
    std::list<int>::reverse_iterator  it = mPlayVo.rbegin();
    while (it != mPlayVo.rend()) {
        int vo = *it;
        if ( (mVoice[vo].priority <= prio_min) && (mVoice[vo].midi_ch == ch) ) {
            prio_min = mVoice[vo].priority;
            v = vo;
        }
        it++;
    }
    // 鳴らす音より高い優先度の音しか無かったら鳴らさない
    if (prio_min > prio) {
        v = -1;
    }
    
    return v;
}

//-----------------------------------------------------------------------------
int C700Generator::FindVoice(int ch)
{
    int v=-1;
    int prio_min = 0x7fff;
    
    std::list<int>::reverse_iterator  it = mPlayVo.rbegin();
    while (it != mPlayVo.rend()) {
        int vo = *it;
        bool    chMatch = (mVoice[vo].midi_ch == ch) ? true:false;
        if (ch == -1) {
            chMatch = true;
        }
        if ( (mVoice[vo].priority <= prio_min) && chMatch ) {
            prio_min = mVoice[vo].priority;
            v = vo + kMaximumVoices;
        }
        it++;
    }
    it = mWaitVo.rbegin();
    while (it != mWaitVo.rend()) {
        int vo = *it;
        if (mVoice[vo].priority <= prio_min) {
            prio_min = mVoice[vo].priority;
            v = vo;
        }
        it++;
    }
    
    return v;
}

//-----------------------------------------------------------------------------
int C700Generator::StopPlayingVoice( const MIDIEvt *evt )
{
	int	stops=0;

    // 再生中なら停止する
    // 再生中でも未再生でも同じ処理で止められるはず
    std::list<int>::iterator	it = mPlayVo.begin();
    while (it != mPlayVo.end()) {
        int	vo = *it;
        
        if ( mVoice[vo].uniqueID == evt->uniqueID ) {
            InstParams		vp = getChannelVP(evt->ch, evt->note);
            if (mVoice[vo].isKeyOn) {
                if (vp.sustainMode) {
                    //キーオフさせずにsrを変更する
                    mVoice[vo].dr = 7;
                    mVoice[vo].sr = vp.sr;
                }
                else {
                    // キーオフ
                    mVoice[vo].envstate = RELEASE;
                }
                mVoice[vo].uniqueID = 0;
                mVoice[vo].priority = vp.releasePriority;
                mVoice[vo].isKeyOn = false;
            }
            if ( vo < mVoiceLimit ) {
                mWaitVo.push_back(vo);
            }
            it = mPlayVo.erase(it);
            stops++;
            continue;
        }
        it++;
    }

	return stops;
}

//-----------------------------------------------------------------------------
void C700Generator::doKeyOn(const MIDIEvt *evt)
{
    int     midiCh = evt->ch & 0x0f;
    
	InstParams		vp = getChannelVP(midiCh, evt->note);
	
	//波形データが存在しない場合は、ここで中断
	if (vp.brr.data == NULL) {
		return;
	}
	
	//ボイスを取得
    int v = (evt->ch >> 4) & 0x0f;
    if (
        (mPlayVo.size() == 0) ||
        (mVoice[v].isKeyOn == true) ||
        (mVoice[v].uniqueID != evt->uniqueID) ||
        (mVoice[v].midi_ch != midiCh)
        ) {
        v = -1;
    }
    if (v == -1) {
        return;
    }
	
	// 中心周波数の算出
	mVoice[v].pitch = pow(2., (evt->note - vp.basekey) / 12.)/INTERNAL_CLOCK*vp.rate*4096 + 0.5;
    
    if (vp.portamentoOn) {
        if (mChStat[midiCh].portaStartPitch == 0) {
            mChStat[midiCh].portaStartPitch = mVoice[v].pitch;
        }
        mVoice[v].portaPitch = mChStat[midiCh].portaStartPitch;
    }
    else {
        mVoice[v].portaPitch = mVoice[v].pitch;
    }
    
    mVoice[v].isKeyOn = true;

    if (!mVoice[v].legato) {
        //ベロシティの取得
        if ( mVelocityMode == kVelocityMode_Square ) {
            mVoice[v].velo = VOLUME_CURB[evt->velo];
        }
        else if ( mVelocityMode == kVelocityMode_Linear ) {
            mVoice[v].velo = evt->velo << 4;
        }
        else {
            mVoice[v].velo=VOLUME_CURB[127];
        }
        
        mVoice[v].volume = mChStat[midiCh].volume;
        mVoice[v].expression = mChStat[midiCh].expression;
        
        mVoice[v].brrdata = vp.brr.data;
        mVoice[v].loopPoint = vp.lp;
        mVoice[v].loop = vp.loop;
        mVoice[v].echoOn = vp.echo;
        
        mVoice[v].pb = CalcPBValue( midiCh, mChStat[midiCh].pitchBend, mVoice[v].pitch );
        mVoice[v].vibdepth = mChStat[midiCh].vibDepth;

        mVoice[v].reg_pmod = mVoice[v].vibdepth>0 ? true:false;
        mVoice[v].vibPhase = 0.0f;
        
        mVoice[v].vol_l=vp.volL;
        mVoice[v].vol_r=vp.volR;
        mVoice[v].ar=vp.ar;
        mVoice[v].dr=vp.dr;
        mVoice[v].sl=vp.sl;
        if (vp.sustainMode) {
            mVoice[v].sr=0;		//ノートオフ時に設定値になる
        }
        else {
            mVoice[v].sr=vp.sr;
        }
        
        // キーオン
        mVoice[v].memPtr = 0;
        mVoice[v].headerCnt = 0;
        mVoice[v].half = 0;
        mVoice[v].envx = 0;
        mVoice[v].end = 0;
        mVoice[v].sampptr = 0;
        mVoice[v].mixfrac = 3 * 4096;
        mVoice[v].envcnt = CNT_INIT;
        mVoice[v].envstate = ATTACK;
        
        // 最後に発音したノート番号を保存
        mChStat[midiCh].lastNote = evt->note;
    }
}

//-----------------------------------------------------------------------------
InstParams C700Generator::getChannelVP(int ch, int note)
{
    InstParams  *pgVP = &mVPset[mChStat[ch].prog];
    if (mDrumMode[pgVP->bank]) {
        return *(getMappedVP(pgVP->bank, note));
    }
    else {
        InstParams  mergedVP;
        mergedVP = *pgVP;
        //if (mChStat[ch].changeFlg & HAS_PGNAME) mergedVP.pgname = mChStat[ch].changedVP.pgname;
        if (mChStat[ch].changeFlg & HAS_RATE) mergedVP.rate = mChStat[ch].changedVP.rate;
        if (mChStat[ch].changeFlg & HAS_BASEKEY) mergedVP.basekey = mChStat[ch].changedVP.basekey;
        if (mChStat[ch].changeFlg & HAS_LOWKEY) mergedVP.lowkey = mChStat[ch].changedVP.lowkey;
        if (mChStat[ch].changeFlg & HAS_HIGHKEY) mergedVP.highkey = mChStat[ch].changedVP.highkey;
        if (mChStat[ch].changeFlg & HAS_AR) mergedVP.ar = mChStat[ch].changedVP.ar;
        if (mChStat[ch].changeFlg & HAS_DR) mergedVP.dr = mChStat[ch].changedVP.dr;
        if (mChStat[ch].changeFlg & HAS_SL) mergedVP.sl = mChStat[ch].changedVP.sl;
        if (mChStat[ch].changeFlg & HAS_SR) mergedVP.sr = mChStat[ch].changedVP.sr;
        if (mChStat[ch].changeFlg & HAS_VOLL) mergedVP.volL = mChStat[ch].changedVP.volL;
        if (mChStat[ch].changeFlg & HAS_VOLR) mergedVP.volR = mChStat[ch].changedVP.volR;
        if (mChStat[ch].changeFlg & HAS_ECHO) mergedVP.echo = mChStat[ch].changedVP.echo;
        if (mChStat[ch].changeFlg & HAS_BANK) mergedVP.bank = mChStat[ch].changedVP.bank;
//        if (mChStat[ch].changeFlg & HAS_ISEMPHASIZED) mergedVP.isEmphasized = mChStat[ch].changedVP.isEmphasized;
//        if (mChStat[ch].changeFlg & HAS_SOURCEFILE) mergedVP.sourceFile = mChStat[ch].changedVP.sourceFile;
        if (mChStat[ch].changeFlg & HAS_SUSTAINMODE) mergedVP.sustainMode = mChStat[ch].changedVP.sustainMode;
        if (mChStat[ch].changeFlg & HAS_MONOMODE) mergedVP.monoMode = mChStat[ch].changedVP.monoMode;
        if (mChStat[ch].changeFlg & HAS_PORTAMENTOON) mergedVP.portamentoOn = mChStat[ch].changedVP.portamentoOn;
        if (mChStat[ch].changeFlg & HAS_PORTAMENTORATE) mergedVP.portamentoRate = mChStat[ch].changedVP.portamentoRate;
        if (mChStat[ch].changeFlg & HAS_NOTEONPRIORITY) mergedVP.noteOnPriority = mChStat[ch].changedVP.noteOnPriority;
        if (mChStat[ch].changeFlg & HAS_RELEASEPRIORITY) mergedVP.releasePriority = mChStat[ch].changedVP.releasePriority;
        return mergedVP;
    }
}

//-----------------------------------------------------------------------------
void C700Generator::Volume( int ch, int value )
{
    mChStat[ch].volume = value & 0x7f;
    // 発音中のボイスに反映
    for (int i=0; i<kMaximumVoices; i++) {
        if (mVoice[i].midi_ch == ch) {
            mVoice[i].volume = mChStat[ch].volume;
        }
    }
}

//-----------------------------------------------------------------------------
void C700Generator::Expression( int ch, int value )
{
    mChStat[ch].expression = value & 0x7f;
    // 発音中のボイスに反映
    for (int i=0; i<kMaximumVoices; i++) {
        if (mVoice[i].midi_ch == ch) {
            mVoice[i].expression = mChStat[ch].expression;
        }
    }
}

//-----------------------------------------------------------------------------
void C700Generator::Panpot( int ch, int value )
{
    mChStat[ch].pan = value & 0x7f;
    // 発音中のボイスに反映
    for (int i=0; i<kMaximumVoices; i++) {
        if (mVoice[i].midi_ch == ch) {
            mVoice[i].pan = mChStat[ch].pan;
        }
    }
}

//-----------------------------------------------------------------------------
void C700Generator::ChangeChRate(int ch, double rate)
{
    mChStat[ch].changedVP.rate = rate;
    mChStat[ch].changeFlg |= HAS_RATE;
}

//-----------------------------------------------------------------------------
void C700Generator::ChangeChBasekey(int ch, int basekey)
{
    mChStat[ch].changedVP.basekey = basekey;
    mChStat[ch].changeFlg |= HAS_BASEKEY;
}

//-----------------------------------------------------------------------------
void C700Generator::ChangeChLowkey(int ch, int lowkey)
{
    mChStat[ch].changedVP.lowkey = lowkey;
    mChStat[ch].changeFlg |= HAS_LOWKEY;
}

//-----------------------------------------------------------------------------
void C700Generator::ChangeChHighkey(int ch, int highkey)
{
    mChStat[ch].changedVP.highkey = highkey;
    mChStat[ch].changeFlg |= HAS_HIGHKEY;
}

//-----------------------------------------------------------------------------
void C700Generator::ChangeChAR(int ch, int ar)
{
    mChStat[ch].changedVP.ar = ar & 0x0f;
    mChStat[ch].changeFlg |= HAS_AR;
    // 発音中のボイスに反映
    for (int i=0; i<kMaximumVoices; i++) {
        if (mVoice[i].midi_ch == ch) {
            mVoice[i].ar = mChStat[ch].changedVP.ar;
        }
    }
}

//-----------------------------------------------------------------------------
void C700Generator::ChangeChDR(int ch, int dr)
{
    mChStat[ch].changedVP.dr = dr & 0x07;
    mChStat[ch].changeFlg |= HAS_DR;
    // 発音中のボイスに反映
    for (int i=0; i<kMaximumVoices; i++) {
        if (mVoice[i].midi_ch == ch) {
            mVoice[i].dr = mChStat[ch].changedVP.dr;
        }
    }
}

//-----------------------------------------------------------------------------
void C700Generator::ChangeChSL(int ch, int sl)
{
    mChStat[ch].changedVP.sl = sl & 0x07;
    mChStat[ch].changeFlg |= HAS_SL;
    // 発音中のボイスに反映
    for (int i=0; i<kMaximumVoices; i++) {
        if (mVoice[i].midi_ch == ch) {
            mVoice[i].sl = mChStat[ch].changedVP.sl;
        }
    }
}

//-----------------------------------------------------------------------------
void C700Generator::ChangeChSR(int ch, int sr)
{
    mChStat[ch].changedVP.sr = sr & 0x1f;
    mChStat[ch].changeFlg |= HAS_SR;
    // 発音中のボイスに反映
    for (int i=0; i<kMaximumVoices; i++) {
        if (mVoice[i].midi_ch == ch) {
            mVoice[i].sr = mChStat[ch].changedVP.sr;
        }
    }
}

//-----------------------------------------------------------------------------
void C700Generator::ChangeChVolL(int ch, int voll)
{
    mChStat[ch].changedVP.volL = voll;
    mChStat[ch].changeFlg |= HAS_VOLL;
    // 発音中のボイスに反映
    for (int i=0; i<kMaximumVoices; i++) {
        if (mVoice[i].midi_ch == ch) {
            mVoice[i].vol_l = mChStat[ch].changedVP.volL;
        }
    }
}

//-----------------------------------------------------------------------------
void C700Generator::ChangeChVolR(int ch, int volr)
{
    mChStat[ch].changedVP.volR = volr;
    mChStat[ch].changeFlg |= HAS_VOLR;
    // 発音中のボイスに反映
    for (int i=0; i<kMaximumVoices; i++) {
        if (mVoice[i].midi_ch == ch) {
            mVoice[i].vol_r = mChStat[ch].changedVP.volR;
        }
    }
}

//-----------------------------------------------------------------------------
void C700Generator::ChangeChEcho(int ch, int echo)
{
    mChStat[ch].changedVP.echo = echo ? true:false;
    mChStat[ch].changeFlg |= HAS_ECHO;
    // 発音中のボイスに反映
    for (int i=0; i<kMaximumVoices; i++) {
        if (mVoice[i].midi_ch == ch) {
            mVoice[i].echoOn = mChStat[ch].changedVP.echo;
        }
    }
}

//-----------------------------------------------------------------------------
void C700Generator::ChangeChBank(int ch, int bank)
{
    mChStat[ch].changedVP.bank = bank & 0x03;
    mChStat[ch].changeFlg |= HAS_BANK;
}

//-----------------------------------------------------------------------------
void C700Generator::ChangeChSustainMode(int ch, int sustainMode)
{
    mChStat[ch].changedVP.sustainMode = sustainMode ? true:false;
    mChStat[ch].changeFlg |= HAS_SUSTAINMODE;
}

//-----------------------------------------------------------------------------
float C700Generator::VibratoWave(float phase)
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
void C700Generator::processPortament(int vo)
{
    float   newPitch;
#if ANALOG_PORTAMENTO
    float   tc = mChStat[ mVoice[vo].midi_ch ].portaTc;
    float   tcInv = 1.0f - tc;
    newPitch = mVoice[vo].pitch * tcInv + mVoice[vo].portaPitch * tc;
    mVoice[vo].portaPitch = newPitch;
#else
    if ( mVoice[vo].pitch > mVoice[vo].portaPitch) {
        newPitch = mVoice[vo].portaPitch * mChStat[ mVoice[vo].midi_ch ].portaTc;
        if (newPitch > mVoice[vo].pitch) {
            newPitch = mVoice[vo].pitch;
        }
        mVoice[vo].portaPitch = newPitch;
    }
    else if ( mVoice[vo].pitch < mVoice[vo].portaPitch) {
        newPitch = mVoice[vo].portaPitch / mChStat[ mVoice[vo].midi_ch ].portaTc;
        if (newPitch < mVoice[vo].pitch) {
            newPitch = mVoice[vo].pitch;
        }
        mVoice[vo].portaPitch = newPitch;
    }
#endif
    mChStat[ mVoice[vo].midi_ch ].portaStartPitch = mVoice[vo].portaPitch;
}

//-----------------------------------------------------------------------------
bool C700Generator::doEvents1( const MIDIEvt *evt )
{
    bool    handled = true;
    
    if (evt->type == NOTE_OFF) {
        if (mChStat[evt->ch].damper) {
            handled = false;
        }
        else {
            int stops = StopPlayingVoice( evt );
            mChStat[evt->ch].noteOns -= stops;
        }
    }
    else {
        // ノートオフ以外のイベントは全て遅延実行する
        MIDIEvt dEvt = *evt;
        dEvt.remain_samples = mEventDelaySamples;
        
        if (evt->type == NOTE_ON) {
            //ボイスを確保して再生準備状態にする
            InstParams		vp = getChannelVP(evt->ch, evt->note);
            int	v = -1;
            int limit = vp.monoMode ? 1:mChStat[evt->ch].limit;
            
            if (mChStat[evt->ch].noteOns >= limit) {
                // ch発音数を超えてたら、そのchの音を一つ止めて次の音を鳴らす
                v = StealVoice(evt->ch, 9999);
                if (v != -1) {
                    if ((mVoice[v].isKeyOn == false) || (!vp.monoMode)) {
                        mPlayVo.remove(v);
                        mChStat[ mVoice[v].midi_ch ].noteOns--;
                        mVoice[v].legato = false;
                    }
                    else {
                        mVoice[v].legato = true;
                    }
                }
            }
            else {
                // 超えてない場合は、後着優先で優先度の低い音を消す
                v = FindVoice();
                if (v >= kMaximumVoices) {  //空きがなくてどこかを止めた
                    v -= kMaximumVoices;
                    mPlayVo.remove(v);
                    mChStat[ mVoice[v].midi_ch ].noteOns--;
                    mVoice[v].legato = false;
                }
                else if (v >= 0) {
                    mWaitVo.remove(v);
                    mVoice[v].legato = false;
                }
                else {
                    mVoice[v].legato = false;
                }
            }
            
            if (v != -1) {
                // 上位4bitにボイス番号を入れる
                dEvt.ch += v << 4;
                mVoice[v].isKeyOn = false;
                mVoice[v].midi_ch = evt->ch;
                mVoice[v].uniqueID = evt->uniqueID;
                mVoice[v].priority = vp.noteOnPriority;
                if (mVoice[v].legato == false) {
                    mPlayVo.push_back(v);
                    mVoice[v].envstate = RELEASE;
                    mChStat[evt->ch].noteOns++;
                }
            }
        }
        mDelayedEvt.push_back(dEvt);
    }
    
    return handled;
}

void C700Generator::doControlChange( int ch, int controlNum, int value )
{
    switch (controlNum) {
        case 1:
            // モジュレーションホイール
            ModWheel(ch, value);
            break;
            
        case 5:
            // ポルタメントタイム
            SetPortamentTime(ch, value);
            mChStat[ch].changedVP.portamentoRate = value;
            mChStat[ch].changeFlg |= HAS_PORTAMENTORATE;
            break;
            
        case 7:
            // ボリューム
            Volume(ch, value);
            break;
            
        case 10:
            // パン
            Panpot(ch, value);
            break;
            
        case 11:
            // エクスプレッション
            Expression(ch, value);
            break;
            
        case 55:
            // チャンネル リミット
            SetChLimit(ch, value);
            break;
            
        case 56:
            // チャンネル プライオリティ
            SetChPriority(ch, value);
            break;
            
        case 57:
            // リリース プライオリティ
            SetReleasePriority(ch, value);
            break;
            
        case 64:
            // ホールド１（ダンパー）
            Damper(ch, (value < 64)?false:true);
            break;
            
        case 65:
            // ポルタメント・オン・オフ
            SetPortamentOn(ch, (value < 64)?false:true);
            break;
            
        case 72:
            // SR
            ChangeChSR(ch, value >> 2);
            break;
            
        case 73:
            // AR
            ChangeChAR(ch, value >> 3);
            break;
            
        case 80:
            // SL
            ChangeChSL(ch, value >> 4);
            break;
            
        case 75:
            // DR
            ChangeChDR(ch, value >> 4);
            break;
            
        case 76:
            // ビブラート・レート
            SetVibFreq(ch, (35.0f * value) / 127);
            break;
            
        case 77:
            // ビブラート・デプス
            SetVibDepth(ch, (15.0f * value) / 127);
            break;
            
        case 84:
            // ポルタメント・コントロール
            SetPortamentControl(ch, value);
            break;
            
        case 91:
            // ECEN ON/OFF
            ChangeChEcho(ch, (value < 64)?0:127);
            break;
            
        case 126:
            // Mono Mode
            SetMonoMode(ch, (value < 64)?0:127);
            break;
            
        case 127:
            // Poly Mode
            SetMonoMode(ch, (value < 64)?127:0);
            break;
            
        default:
            break;
    }
}

bool C700Generator::doEvents2( const MIDIEvt *evt )
{
    bool    handled = true;
    
    switch (evt->type) {
        case NOTE_ON:
            doKeyOn( evt );
            break;
            
        case NOTE_OFF:
            //StopPlayingVoice( evt );
            break;
            
        case PROGRAM_CHANGE:
            doProgramChange(evt->ch, evt->note);
            break;
            
        case PITCH_BEND:
            doPitchBend(evt->ch, evt->note, evt->velo);
            break;
            
        case CONTROL_CHANGE:
            doControlChange(evt->ch, evt->note, evt->velo);
            break;
            
        default:
            //handled = false;
            break;
    }
    return handled;
}

//-----------------------------------------------------------------------------
void C700Generator::Process( unsigned int frames, float *output[2] )
{
	int		outx;
	int		pitch;
	int		procstep = (INTERNAL_CLOCK*CYCLES_PER_SAMPLE) / mSampleRate;    // CYCLES_PER_SAMPLE=1.0 とした固定小数
	
	//メイン処理
    
    if (mClearEvent) {
        mMIDIEvt.clear();
        mDelayedEvt.clear();
        mClearEvent = false;
    }
    
	for (unsigned int frame=0; frame<frames; ++frame) {
		//イベント処理
		if ( mMIDIEvt.size() != 0 ) {
			std::list<MIDIEvt>::iterator	it = mMIDIEvt.begin();
			while ( it != mMIDIEvt.end() ) {
				if ( it->remain_samples >= 0 ) {
					it->remain_samples--;
                }
                if ( it->remain_samples < 0 ) {
                    if (doEvents1(&(*it))) {
                        it = mMIDIEvt.erase( it );
                        continue;
                    }
                }
				it++;
			}
		}
        if ( mDelayedEvt.size() != 0 ) {
			std::list<MIDIEvt>::iterator	it = mDelayedEvt.begin();
			while ( it != mDelayedEvt.end() ) {
				if ( it->remain_samples >= 0 ) {
					it->remain_samples--;
                }
                if ( it->remain_samples < 0 ) {
                    if (doEvents2(&(*it))) {
                        it = mDelayedEvt.erase( it );
                        continue;
                    }
                }
				it++;
			}
		}
        
        while (mPortamentCount >= 0) {
            // ポルタメント処理
            std::list<int>::iterator	it = mPlayVo.begin();
            while (it != mPlayVo.end()) {
                int	vo = *it;
                if (mVoice[vo].isKeyOn) {
                    processPortament(vo);
                }
                it++;
            }
            mPortamentCount -= PORTAMENT_CYCLE_SAMPLES;
        }
		
		for ( ; mProcessFrac >= 0; mProcessFrac -= CYCLES_PER_SAMPLE ) {
			int outl=0,outr=0;
			for ( int v=0; v<kMaximumVoices; v++ ) {
				outx = 0;
				//--
				{
					switch( mVoice[v].envstate ) {
						case ATTACK:
							if ( mVoice[v].ar == 15 ) {
								mVoice[v].envx += 0x400;
							}
							else {
								mVoice[v].envcnt -= ENVCNT[ ( mVoice[v].ar << 1 ) + 1 ];
								if ( mVoice[v].envcnt > 0 ) {
									break;
								}
								mVoice[v].envx += 0x20;       /* 0x020 / 0x800 = 1/64         */
								mVoice[v].envcnt = CNT_INIT;
							}
							
							if ( mVoice[v].envx > 0x7FF ) {
								mVoice[v].envx = 0x7FF;
								mVoice[v].envstate = DECAY;
							}
							break;
							
						case DECAY:
							mVoice[v].envcnt -= ENVCNT[ mVoice[v].dr*2 + 0x10 ];
							if ( mVoice[v].envcnt <= 0 ) {
								mVoice[v].envcnt = CNT_INIT;
								mVoice[v].envx -= ( ( mVoice[v].envx - 1 ) >> 8 ) + 1;
							}
							
							if ( mVoice[v].envx <= 0x100 * ( mVoice[v].sl + 1 ) ) {
								mVoice[v].envstate = SUSTAIN;
							}
							break;
							
						case SUSTAIN:
							mVoice[v].envcnt -= ENVCNT[ mVoice[v].sr ];
							if ( mVoice[v].envcnt > 0 ) {
								break;
							}
							mVoice[v].envcnt = CNT_INIT;
							mVoice[v].envx -= ( ( mVoice[v].envx - 1 ) >> 8 ) + 1;
							break;
							
						case RELEASE:
							mVoice[v].envx -= 0x8;
							if ( mVoice[v].envx <= 0 ) {
								mVoice[v].envx = -1;
							}
							break;
							
//						case FASTRELEASE:
//							mVoice[v].envx -= 0x40;
//							if ( mVoice[v].envx <= 0 ) {
//								mVoice[v].envx = -1;
//							}
//							break;
					}
				}
				
				if ( mVoice[v].envx < 0 ) {
					outx = 0;
					continue;
				}
				
				//ピッチの算出
                int voicePitch = static_cast<int>(mVoice[v].portaPitch + 0.5f);
//                int voicePitch = mChStat[mVoice[v].midi_ch].portaOn ?
//                static_cast<int>(mVoice[v].portaPitch + 0.5f):mVoice[v].pitch;

				pitch = (voicePitch + mVoice[v].pb) & 0x3fff;
				
				if (mVoice[v].reg_pmod) {
					mVoice[v].vibPhase += mVibfreq;
					if (mVoice[v].vibPhase > onepi) {
						mVoice[v].vibPhase -= onepi*2;
					}
					
					float vibwave = VibratoWave(mVoice[v].vibPhase);
					int pitchRatio = (vibwave*mVibdepth)*VOLUME_CURB[mVoice[v].vibdepth];
					
					pitch = ( pitch * ( pitchRatio + 32768 ) ) >> 15;
					if (pitch <= 0) {
						pitch=1;
					}
				}
				
				for( ; mVoice[v].mixfrac >= 0; mVoice[v].mixfrac -= 4096 ) {
					if( !mVoice[v].headerCnt ) {	//ブロックの始まり
						if( mVoice[v].end & 1 ) {	//データ終了フラグあり
							if( mVoice[v].loop ) {
								mVoice[v].memPtr = mVoice[v].loopPoint;	//読み出し位置をループポイントまで戻す
							}
							else {	//ループなし
								mVoice[v].envx = 0;
								while( mVoice[v].mixfrac >= 0 ) {
									mVoice[v].sampbuf[mVoice[v].sampptr] = 0;
									outx = 0;
									mVoice[v].sampptr  = ( mVoice[v].sampptr + 1 ) & 3;
									mVoice[v].mixfrac -= 4096;
								}
								break;
							}
						}
						
						//開始バイトの情報を取得
						mVoice[v].headerCnt = 8;
						int headbyte = ( unsigned char )mVoice[v].brrdata[mVoice[v].memPtr++];
						mVoice[v].range = headbyte >> 4;
						mVoice[v].end = headbyte & 3;
						mVoice[v].filter = ( headbyte & 12 ) >> 2;
					}
					
					if ( mVoice[v].half == 0 ) {
						mVoice[v].half = 1;
						outx = ( ( signed char )mVoice[v].brrdata[ mVoice[v].memPtr ] ) >> 4;
					}
					else {
						mVoice[v].half = 0;
						outx = ( signed char )( mVoice[v].brrdata[ mVoice[v].memPtr++ ] << 4 );
						outx >>= 4;
						mVoice[v].headerCnt--;
					}
					//outx:4bitデータ
					
					if ( mVoice[v].range <= 0xC ) {
						outx = ( outx << mVoice[v].range ) >> 1;
					}
					else {
						outx &= ~0x7FF;
					}
					//outx:4bitデータ*Range
					
					switch( mVoice[v].filter ) {
						case 0:
							break;
							
						case 1:
							outx += filter1(mVoice[v].smp1);
							break;
							
						case 2:
							outx += filter2(mVoice[v].smp1,mVoice[v].smp2);
							break;
							
						case 3:
							outx += filter3(mVoice[v].smp1,mVoice[v].smp2);
							break;
					}
					
                    // フィルタ後にクリップ
					if ( outx < -32768 ) {
						outx = -32768;
					}
					else if ( outx > 32767 ) {
						outx = 32767;
					}
                    // y[-1]へ送る際に２倍されたらクランプ
					if (mNewADPCM) {
						mVoice[v].smp2 = mVoice[v].smp1;
						mVoice[v].smp1 = ( signed short )( outx << 1 );
					}
					else {
                        // 古いエミュレータの一部にはクランプを行わないものもある
                        // 音は実機と異なる
						mVoice[v].smp2 = mVoice[v].smp1;
						mVoice[v].smp1 = outx << 1;
					}
                    mVoice[v].sampbuf[mVoice[v].sampptr] = mVoice[v].smp1;
					mVoice[v].sampptr = ( mVoice[v].sampptr + 1 ) & 3;
				}
				
				int fracPos = mVoice[v].mixfrac >> 4;
				int smpl = ( ( G4[ -fracPos - 1 ] * mVoice[v].sampbuf[ mVoice[v].sampptr ] ) >> 11 ) & ~1;
				smpl += ( ( G3[ -fracPos ]
						 * mVoice[v].sampbuf[ ( mVoice[v].sampptr + 1 ) & 3 ] ) >> 11 ) & ~1;
				smpl += ( ( G2[ fracPos ]
						 * mVoice[v].sampbuf[ ( mVoice[v].sampptr + 2 ) & 3 ] ) >> 11 ) & ~1;
				// openspcではなぜかここでもクランプさせていた
                // ここも無いと実機と違ってしまう
				if (mNewADPCM) {
					smpl = ( signed short )smpl;
				}
				smpl += ( ( G1[ fracPos ]
						 * mVoice[v].sampbuf[ ( mVoice[v].sampptr + 3 ) & 3 ] ) >> 11 ) & ~1;
				
                // ガウス補間後にクリップ
				if ( smpl > 32767 ) {
					smpl = 32767;
				}
				else if ( smpl < -32768 ) {
					smpl = -32768;
				}
				outx = smpl;
				
				mVoice[v].mixfrac += pitch;
				
				outx = ( ( outx * mVoice[v].envx ) >> 11 ) & ~1;
				outx = ( outx * mVoice[v].velo ) >> 11;
				
                //パンのボリューム値への反映
                int volL = mVoice[v].vol_l;
                int volR = mVoice[v].vol_r;
                calcPanVolume(mVoice[v].pan, &volL, &volR);
                
				//ボリューム値の反映
				volL = ( volL * VOLUME_CURB[ mVoice[v].volume ] ) / 0x7ff;
				volR = ( volR * VOLUME_CURB[ mVoice[v].volume ] ) / 0x7ff;
                
                // エクスプレッションの反映
				volL = ( volL * VOLUME_CURB[ mVoice[v].expression ] ) / 0x7ff;
				volR = ( volR * VOLUME_CURB[ mVoice[v].expression ] ) / 0x7ff;
				
				//ゲイン値の反映
				int vl = ( volL * outx ) >> 7;
				int vr = ( volR * outx ) >> 7;
                
				//エコー処理
				if ( mVoice[v].echoOn ) {
					mEcho[0].Input(vl);
					mEcho[1].Input(vr);
				}
				//メインボリュームの反映
				outl += ( vl * mMainVolume_L ) >> 7;
				outr += ( vr * mMainVolume_R ) >> 7;
			}
			outl += mEcho[0].GetFxOut();
			outr += mEcho[1].GetFxOut();
			mProcessbuf[0][mProcessbufPtr] = outl;
			mProcessbuf[1][mProcessbufPtr] = outr;
			mProcessbufPtr=(mProcessbufPtr+1)&0x0f;
            
            mPortamentCount++;
		}
		//--
		//16pointSinc補間
		for ( int ch=0; ch<2; ch++ ) {
			int inputFrac = mProcessFrac+CYCLES_PER_SAMPLE;
			int tabidx1 = ( inputFrac/1764 ) << 4;
			int tabidx2 = tabidx1 + 16;
			int a1 = 0, a2 = 0;
			for (int i=0; i<4; i++) {
				a1 += sinctable[tabidx1++] * mProcessbuf[ch][mProcessbufPtr] >> 15;
				a2 += sinctable[tabidx2++] * mProcessbuf[ch][mProcessbufPtr] >> 15;
				mProcessbufPtr=(mProcessbufPtr+1)&0x0f;
				a1 += sinctable[tabidx1++] * mProcessbuf[ch][mProcessbufPtr] >> 15;
				a2 += sinctable[tabidx2++] * mProcessbuf[ch][mProcessbufPtr] >> 15;
				mProcessbufPtr=(mProcessbufPtr+1)&0x0f;
				a1 += sinctable[tabidx1++] * mProcessbuf[ch][mProcessbufPtr] >> 15;
				a2 += sinctable[tabidx2++] * mProcessbuf[ch][mProcessbufPtr] >> 15;
				mProcessbufPtr=(mProcessbufPtr+1)&0x0f;
				a1 += sinctable[tabidx1++] * mProcessbuf[ch][mProcessbufPtr] >> 15;
				a2 += sinctable[tabidx2++] * mProcessbuf[ch][mProcessbufPtr] >> 15;
				mProcessbufPtr=(mProcessbufPtr+1)&0x0f;
			}
			if ( output[ch] ) {
				output[ch][frame] += ( a1 + ( (( a2 - a1 ) * ( inputFrac % 1764 )) / 1764 ) ) / 32768.0f;
			}
		}
		//--
		
		mProcessFrac += procstep;
	}
}

//-----------------------------------------------------------------------------
void C700Generator::calcPanVolume(int value, int *volL, int *volR)
{
#if 0
    int     absL = (*volL > 0)?(*volL):-(*volL);
    int     absR = (*volR > 0)?(*volR):-(*volR);
    int     center =(absR + absL) / 2;
    int     diffRL = absR - absL;
    diffRL += value * 2 - 128;
    absL = center - diffRL;
    absR = center + diffRL;
    if (absL > 127) {
        absL = 127;
    }
    if (absL < 0) {
        absL = 0;
    }
    if (absR > 127) {
        absR = 127;
    }
    if (absR < 0) {
        absR = 0;
    }
    *volL = (*volL)>0 ? absL:-absL;
    *volR = (*volR)>0 ? absR:-absR;
#else
    *volL = (*volL * PAN_CURB[value]) / 511;
    *volR = (*volR * PAN_CURB[128-value]) / 511;
#endif
}

//-----------------------------------------------------------------------------
void C700Generator::RefreshKeyMap(void)
{
	if ( mVPset ) {
		bool	initialized[NUM_BANKS];
		for (int i=0; i<NUM_BANKS; i++ ) {
			initialized[i] = false;
		}
		
		for (int prg=0; prg<128; prg++) {
			if (mVPset[prg].brr.data) {
				if ( !initialized[mVPset[prg].bank] ) {
					// 一番最初のプログラムで初期化することで、未使用パッチが0にならないようにする
					for (int i=0; i<128; i++) {
						mKeyMap[mVPset[prg].bank][i]=prg;
					}
					initialized[mVPset[prg].bank] = true;
				}
				for (int i=mVPset[prg].lowkey; i<=mVPset[prg].highkey; i++) {
					mKeyMap[mVPset[prg].bank][i]=prg;
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void C700Generator::SetVPSet( InstParams *vp )
{
    mVPset = vp;
    for (int i=0; i<16; i++) {
        mChStat[i].changeFlg = 0;
        mChStat[i].changedVP = vp[0];
    }
}

//-----------------------------------------------------------------------------
double C700Generator::GetProcessDelayTime()
{
    return ((mEventDelayClocks / CLOCKS_PER_SAMPLE) + 8) / static_cast<double>(INTERNAL_CLOCK);    // 8ms + resample
}