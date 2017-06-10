/*
 *  C700Driver.cpp
 *  C700
 *
 *  Created by osoumen on 06/09/06.
 *  Copyright 2006 Vermicelli Magic. All rights reserved.
 *
 */

#include "C700defines.h"
#include "C700Driver.h"
#include <math.h>

const float onepi = 3.14159265358979;

#define ANALOG_PORTAMENTO 0

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

//-----------------------------------------------------------------------------
void C700Driver::VoiceStatus::Reset()
{
	pb = 0;
	reg_pmod = 0;
	vibdepth = 0;
	vibPhase = 0.0f;
    portaPitch = .0f;
    pan = 64;
    non = false;
	
    targetPitch = 0;
	//loopPoint = 0;
	//loop = false;
}

//-----------------------------------------------------------------------------
C700Driver::C700Driver()
: mSampleRate(44100.0),
  mVelocityMode( kVelocityMode_Square ),
  mVPset(NULL)
{
    MutexInit(mREGLOGEvtMtx);
    MutexInit(mMIDIEvtMtx);
    
	for ( int i=0; i<NUM_BANKS; i++ ) {
		mDrumMode[i] = false;
	}
	for ( int bnk=0; bnk<NUM_BANKS; bnk++ ) {
		for ( int i=0; i<128; i++ ) {
			mKeyMap[bnk][i] = 0;
		}
	}
	//Initialize
    mVibfreq = 0.00137445;
	mVibdepth = 0.5;
    
    mVoiceLimit = 8;
    mIsAccurateMode = false;
    mFastReleaseAsKeyOff = true;

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
        //mChStat[i].releasePriority = 0;
        mChStat[i].damper = false;
        
        mChStat[i].lastNote = 0;
	}
    mVoiceManager.Initialize(8);
    
	Reset();
}

//-----------------------------------------------------------------------------
C700Driver::~C700Driver()
{
    MutexDestroy(mMIDIEvtMtx);
    MutexDestroy(mREGLOGEvtMtx);
}
//-----------------------------------------------------------------------------
void C700Driver::Reset()
{
	for (int i=0; i<16; i++) {
        mDataEntryValue[i] = 0;
        mRPN[i] = 0x7f7f;
        mNRPN[i] = 0x7f7f;
        mIsSettingNRPN[i] = false;
        
		mProcessbuf[0][i]=0;
		mProcessbuf[1][i]=0;
	}
	mProcessFrac=0;
	mProcessbufPtr=0;
    mPortamentCount=0;
    for (int i=0; i<kMaximumVoices; i++) {
        mPitchCount[i] = 0;
    }
	
    mVoiceManager.Reset();
    
    AllSoundOff();
}

//-----------------------------------------------------------------------------
void C700Driver::EnqueueMidiEvent(const MIDIEvt *evt)
{
    MutexLock(mMIDIEvtMtx);
    mMIDIEvt.push_back( *evt );
    MutexUnlock(mMIDIEvtMtx);
}

//-----------------------------------------------------------------------------
void C700Driver::StartRegisterLog( int inFrame )
{
    MIDIEvt			evt;
	evt.type = START_REGLOG;
	evt.ch = 0;
	evt.note = 0;
	evt.velo = 0;
	evt.uniqueID = 0;
	evt.remain_samples = inFrame;
    MutexLock(mREGLOGEvtMtx);
	mREGLOGEvt.push_back( evt );
    MutexUnlock(mREGLOGEvtMtx);
}

//-----------------------------------------------------------------------------
void C700Driver::MarkLoopRegisterLog( int inFrame )
{
    MIDIEvt			evt;
	evt.type = MARKLOOP_REGLOG;
	evt.ch = 0;
	evt.note = 0;
	evt.velo = 0;
	evt.uniqueID = 0;
	evt.remain_samples = inFrame;
    MutexLock(mREGLOGEvtMtx);
	mREGLOGEvt.push_back( evt );
    MutexUnlock(mREGLOGEvtMtx);
}

//-----------------------------------------------------------------------------
void C700Driver::EndRegisterLog( int inFrame )
{
    MIDIEvt			evt;
	evt.type = END_REGLOG;
	evt.ch = 0;
	evt.note = 0;
	evt.velo = 0;
	evt.uniqueID = 0;
	evt.remain_samples = inFrame;
    MutexLock(mREGLOGEvtMtx);
	mREGLOGEvt.push_back( evt );
    MutexUnlock(mREGLOGEvtMtx);
}

//-----------------------------------------------------------------------------
void C700Driver::AllNotesOff()
{
    //mClearEvent = true;
    mDSP.KeyOffAll();
	for ( int i=0; i<kMaximumVoices; i++ ) {
        mVoiceStat[i].Reset();
        mKeyOnFlag[i] = false;
        mKeyOffFlag[i] = false;
        mEchoOnMask[i] = false;
        mPMOnMask[i] = false;
        mNoiseOnMask[i] = false;
	}
    mEchoOnFlag = 0;
    mPMOnFlag = 0;
    mNoiseOnFlag = 0;
    mVoiceManager.Reset();
}

//-----------------------------------------------------------------------------
void C700Driver::AllSoundOff()
{
    AllNotesOff();
	mDSP.ResetEcho();
}

//-----------------------------------------------------------------------------
void C700Driver::ResetAllControllers()
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
void C700Driver::doProgramChange( int ch, int value )
{
	mChStat[ch].prog = value;
    if (mVPset) {
        mChStat[ch].changeFlg = 0;
        mChStat[ch].changedVP = mVPset[mChStat[ch].prog];
        
        setPortamentTime(ch, mVPset[mChStat[ch].prog].portamentoRate);
    }
}

//-----------------------------------------------------------------------------
int C700Driver::calcPBValue( int ch, float pitchBend, int basePitch )
{
	return (int)((pow(2., (pitchBend * mChStat[ch].pbRange) / 12.) - 1.0)*basePitch);
}

//-----------------------------------------------------------------------------
void C700Driver::doPitchBend( int ch, int value1, int value2 )
{
    int pitchBend = ((value2 << 7) | value1) - 8192;
	float pb_value = pitchBend / 8192.0;
	
    mChStat[ch].pitchBend = pb_value;
	for ( int i=0; i<kMaximumVoices; i++ ) {
		if ( mVoiceManager.GetVoiceMidiCh(i) == ch ) {
			mVoiceStat[i].pb = calcPBValue( ch, pb_value, mVoiceStat[i].targetPitch );
		}
	}
}

//-----------------------------------------------------------------------------
void C700Driver::setModWheel( int ch, int value )
{
    mChStat[ch].vibDepth = value;
	for ( int i=0; i<kMaximumVoices; i++ ) {
		if ( mVoiceManager.GetVoiceMidiCh(i) == ch ) {
			mVoiceStat[i].vibdepth = value;
			mVoiceStat[i].reg_pmod = value > 0 ? true:false;
		}
	}
}

//-----------------------------------------------------------------------------
void C700Driver::setDamper( int ch, bool on )
{
    mChStat[ch].damper = on;
}

//-----------------------------------------------------------------------------
void C700Driver::SetVoiceLimit( int value )
{
    mVoiceLimit = value;
    if (!mIsAccurateMode) {
        mVoiceManager.ChangeVoiceLimit(value);
        mDSP.SetVoiceLimit(value);
    }
}

//-----------------------------------------------------------------------------
void C700Driver::SetPBRange( float value )
{
    for (int i=0; i<16; i++) {
        mChStat[i].pbRange = value;
    }
}
//-----------------------------------------------------------------------------
void C700Driver::setPBRange( int ch, float value )
{
	mChStat[ch].pbRange = value;
}

//-----------------------------------------------------------------------------
void C700Driver::SetEngineType( engine_type type )
{
    switch (type) {
        case kEngineType_Old:
            mDSP.SetNewADPCM(false);
            mDSP.SetRealEmulation(false);
            mIsAccurateMode = false;
            mVoiceManager.ChangeVoiceLimit(mVoiceLimit);
            mDSP.SetVoiceLimit(mVoiceLimit);
            break;
        case kEngineType_Relaxed:
            mDSP.SetNewADPCM(true);
            mDSP.SetRealEmulation(false);
            mIsAccurateMode = false;
            mVoiceManager.ChangeVoiceLimit(mVoiceLimit);
            mDSP.SetVoiceLimit(mVoiceLimit);
            break;
        case kEngineType_Accurate:
            mDSP.SetNewADPCM(true);
            mDSP.SetRealEmulation(true);
            mIsAccurateMode = true;
            mVoiceManager.ChangeVoiceLimit(8);
            mDSP.SetVoiceLimit(8);
            break;
    }
}

//-----------------------------------------------------------------------------
void C700Driver::SetVoiceAllocMode( voicealloc_mode mode )
{
    switch (mode) {
        case kVoiceAllocMode_Oldest:
            mVoiceManager.SetVoiceAllocMode(DynamicVoiceManager::ALLOC_MODE_OLDEST);
            break;
        case kVoiceAllocMode_SameChannel:
            mVoiceManager.SetVoiceAllocMode(DynamicVoiceManager::ALLOC_MODE_SAMECH);
            break;
        default:
            break;
    }
}

//-----------------------------------------------------------------------------
void C700Driver::SetFastReleaseAsKeyOff( bool value )
{
    mFastReleaseAsKeyOff = value;
}

//-----------------------------------------------------------------------------
void C700Driver::SetMultiMode( int bank, bool value )
{
	mDrumMode[bank] = value;
}

//-----------------------------------------------------------------------------
bool C700Driver::GetMultiMode( int bank ) const
{
	return mDrumMode[bank];
}

//-----------------------------------------------------------------------------
void C700Driver::SetVelocityMode( velocity_mode value )
{
	mVelocityMode = value;
}

//-----------------------------------------------------------------------------
void C700Driver::SetVibFreq( int ch, float value )
{
	mVibfreq = value*((onepi*2)/(INTERNAL_CLOCK / PITCH_CYCLE_SAMPLES));
}

//-----------------------------------------------------------------------------
void C700Driver::SetVibDepth( int ch, float value )
{
	mVibdepth = value / 2;
}

//-----------------------------------------------------------------------------
void C700Driver::SetMainVol_L( int value )
{
    mDSP.SetMainVolumeL(value);
}

//-----------------------------------------------------------------------------
void C700Driver::SetMainVol_R( int value )
{
    mDSP.SetMainVolumeR(value);
}

//-----------------------------------------------------------------------------
void C700Driver::SetEchoVol_L( int value )
{
    mDSP.SetEchoVol_L(value);
}

//-----------------------------------------------------------------------------
void C700Driver::SetEchoVol_R( int value )
{
    mDSP.SetEchoVol_R(value);
}

//-----------------------------------------------------------------------------
void C700Driver::SetFeedBackLevel( int value )
{
    mDSP.SetFeedBackLevel(value);
}

//-----------------------------------------------------------------------------
void C700Driver::SetDelayTime( int value )
{
    mDSP.SetDelayTime(value);
}

//-----------------------------------------------------------------------------
void C700Driver::SetFIRTap( int tap, int value )
{
    mDSP.SetFIRTap(tap, value);
}

//-----------------------------------------------------------------------------
void C700Driver::SetBrrSample( int srcn, const unsigned char *data, int size, int loopPoint)
{
    mDSP.SetDir(mMemManager.GetDirAddr() >> 8);
    mMemManager.WriteData(srcn, data, size, loopPoint);
    mMemManager.UpdateMem(&mDSP);
}

//-----------------------------------------------------------------------------
void C700Driver::DelBrrSample( int srcn )
{
    mDSP.SetDir(mMemManager.GetDirAddr() >> 8);
    mMemManager.DeleteData(srcn);
    mMemManager.UpdateMem(&mDSP);
}

//-----------------------------------------------------------------------------
void C700Driver::UpdateLoopPoint( int prog )
{
    mMemManager.ChangeLoopPoint(prog, mVPset[prog].lp, &mDSP);
}

//-----------------------------------------------------------------------------
void C700Driver::UpdateLoopFlag( int prog )
{
    if (mMemManager.HasData(prog)) {
        // 最後のバイトだけ書き換える
        mMemManager.ChangeLoopFlag(prog, mVPset[prog].isLoop(), &mDSP);
    }
    else {
        mDSP.SetDir(mMemManager.GetDirAddr() >> 8);
        mMemManager.WriteData(prog, mVPset[prog].brrData(), mVPset[prog].brrSize(), mVPset[prog].lp);
        mMemManager.UpdateMem(&mDSP);
    }
}

//-----------------------------------------------------------------------------
void C700Driver::SetSampleRate( double samplerate )
{
    mSampleRate = samplerate;
    mEventDelaySamples = calcEventDelaySamples();
}

//-----------------------------------------------------------------------------
void C700Driver::SetEventDelayClocks(int clocks)
{
    mEventDelayClocks = clocks;
    mEventDelaySamples = calcEventDelaySamples();
}

//-----------------------------------------------------------------------------
void C700Driver::changePortamentOn( int ch, bool on )
{
    //mChStat[ch].portaOn = on;
    mChStat[ch].changedVP.portamentoOn = on;
    mChStat[ch].changeFlg |= HAS_PORTAMENTOON;
}

//-----------------------------------------------------------------------------
float C700Driver::calcGM2PortamentCurve(int value)
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
void C700Driver::setPortamentTime( int ch, int value )
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
void C700Driver::UpdatePortamentoTime( int prog )
{
    if (mVPset == NULL) {
        return;
    }
    
    for (int i=0; i<16; i++) {
        if (mChStat[i].prog == prog) {
            setPortamentTime(i, mVPset[prog].portamentoRate);
        }
    }
}

//-----------------------------------------------------------------------------
void C700Driver::setPortamentControl( int ch, int note )
{
    InstParams		vp = getChannelVP(ch, mChStat[ch].lastNote);
    mChStat[ch].portaStartPitch = pow(2., (note - vp.basekey) / 12.)/INTERNAL_CLOCK*vp.rate*4096 + 0.5;
}

//-----------------------------------------------------------------------------
void C700Driver::changeChPriority( int ch, int value )
{
    //mChStat[ch].priority = value;
    mChStat[ch].changedVP.noteOnPriority = value;
    mChStat[ch].changeFlg |= HAS_NOTEONPRIORITY;
}

//-----------------------------------------------------------------------------
void C700Driver::changeChLimit( int ch, int value )
{
    mVoiceManager.SetChLimit(ch, value);
}

//-----------------------------------------------------------------------------
void C700Driver::changeReleasePriority( int ch, int value )
{
    //mChStat[ch].releasePriority = value;
    mChStat[ch].changedVP.releasePriority = value;
    mChStat[ch].changeFlg |= HAS_RELEASEPRIORITY;
}

//-----------------------------------------------------------------------------
void C700Driver::changeMonoMode( int ch, bool on )
{
    //mChStat[ch].monoMode = on;
    mChStat[ch].changedVP.monoMode = on;
    mChStat[ch].changeFlg |= HAS_MONOMODE;
}

//-----------------------------------------------------------------------------
InstParams C700Driver::getChannelVP(int ch, int note)
{
    const InstParams  *pgVP = &mVPset[mChStat[ch].prog];
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
        if (mChStat[ch].changeFlg & HAS_SR1) mergedVP.sr1 = mChStat[ch].changedVP.sr1;
        if (mChStat[ch].changeFlg & HAS_SR2) mergedVP.sr2 = mChStat[ch].changedVP.sr2;
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
        if (mChStat[ch].changeFlg & HAS_PMON) mergedVP.pmOn = mChStat[ch].changedVP.pmOn;
        if (mChStat[ch].changeFlg & HAS_NOISEON) mergedVP.noiseOn = mChStat[ch].changedVP.noiseOn;
        
        return mergedVP;
    }
}

//-----------------------------------------------------------------------------
void C700Driver::setVolume( int ch, int value )
{
    mChStat[ch].volume = value & 0x7f;
    // 発音中のボイスに反映
    for (int i=0; i<kMaximumVoices; i++) {
        if (mVoiceManager.GetVoiceMidiCh(i) == ch) {
            mVoiceStat[i].volume = mChStat[ch].volume;
        }
    }
}

//-----------------------------------------------------------------------------
void C700Driver::setExpression( int ch, int value )
{
    mChStat[ch].expression = value & 0x7f;
    // 発音中のボイスに反映
    for (int i=0; i<kMaximumVoices; i++) {
        if (mVoiceManager.GetVoiceMidiCh(i) == ch) {
            mVoiceStat[i].expression = mChStat[ch].expression;
        }
    }
}

//-----------------------------------------------------------------------------
void C700Driver::setPanpot( int ch, int value )
{
    mChStat[ch].pan = value & 0x7f;
    // 発音中のボイスに反映
    for (int i=0; i<kMaximumVoices; i++) {
        if (mVoiceManager.GetVoiceMidiCh(i) == ch) {
            mVoiceStat[i].pan = mChStat[ch].pan;
        }
    }
}
#if 0
//-----------------------------------------------------------------------------
void C700Driver::ChangeChRate(int ch, double rate)
{
    mChStat[ch].changedVP.rate = rate;
    mChStat[ch].changeFlg |= HAS_RATE;
}

//-----------------------------------------------------------------------------
void C700Driver::ChangeChBasekey(int ch, int basekey)
{
    mChStat[ch].changedVP.basekey = basekey;
    mChStat[ch].changeFlg |= HAS_BASEKEY;
}

//-----------------------------------------------------------------------------
void C700Driver::ChangeChLowkey(int ch, int lowkey)
{
    mChStat[ch].changedVP.lowkey = lowkey;
    mChStat[ch].changeFlg |= HAS_LOWKEY;
}

//-----------------------------------------------------------------------------
void C700Driver::ChangeChHighkey(int ch, int highkey)
{
    mChStat[ch].changedVP.highkey = highkey;
    mChStat[ch].changeFlg |= HAS_HIGHKEY;
}
#endif
//-----------------------------------------------------------------------------
void C700Driver::changeChAR(int ch, int ar)
{
    mChStat[ch].changedVP.ar = ar & 0x0f;
    mChStat[ch].changeFlg |= HAS_AR;
    // 発音中のボイスに反映
    for (int i=0; i<kMaximumVoices; i++) {
        if (mVoiceManager.GetVoiceMidiCh(i) == ch) {
            mDSP.SetAR(i, mChStat[ch].changedVP.ar);
        }
    }
}

//-----------------------------------------------------------------------------
void C700Driver::changeChDR(int ch, int dr)
{
    mChStat[ch].changedVP.dr = dr & 0x07;
    mChStat[ch].changeFlg |= HAS_DR;
    // 発音中のボイスに反映
    for (int i=0; i<kMaximumVoices; i++) {
        if (mVoiceManager.GetVoiceMidiCh(i) == ch) {
            mDSP.SetDR(i, mChStat[ch].changedVP.dr);
        }
    }
}

//-----------------------------------------------------------------------------
void C700Driver::changeChSL(int ch, int sl)
{
    mChStat[ch].changedVP.sl = sl & 0x07;
    mChStat[ch].changeFlg |= HAS_SL;
    // 発音中のボイスに反映
    for (int i=0; i<kMaximumVoices; i++) {
        if (mVoiceManager.GetVoiceMidiCh(i) == ch) {
            mDSP.SetSL(i, mChStat[ch].changedVP.sl);
        }
    }
}

//-----------------------------------------------------------------------------
void C700Driver::changeChSR1(int ch, int sr)
{
    mChStat[ch].changedVP.sr1 = sr & 0x1f;
    mChStat[ch].changeFlg |= HAS_SR1;
    // 発音中のボイスに反映
    for (int i=0; i<kMaximumVoices; i++) {
        if (mVoiceManager.IsKeyOn(i)) {
            if (mVoiceManager.GetVoiceMidiCh(i) == ch) {
                mDSP.SetSR(i, mChStat[ch].changedVP.sr1);
            }
        }
    }
}

//-----------------------------------------------------------------------------
void C700Driver::changeChSR2(int ch, int sr)
{
    mChStat[ch].changedVP.sr2 = sr & 0x1f;
    mChStat[ch].changeFlg |= HAS_SR2;
    // 発音中のボイスに反映
    for (int i=0; i<kMaximumVoices; i++) {
        if (!mVoiceManager.IsKeyOn(i)) {
            if (mVoiceManager.GetVoiceMidiCh(i) == ch) {
                mDSP.SetSR(i, mChStat[ch].changedVP.sr2);
            }
        }
    }
}

//-----------------------------------------------------------------------------
#if 0
void C700Driver::changeChVolL(int ch, int voll)
{
    mChStat[ch].changedVP.volL = voll;
    mChStat[ch].changeFlg |= HAS_VOLL;
    // 発音中のボイスに反映
    for (int i=0; i<kMaximumVoices; i++) {
        if (mVoiceManager.GetVoiceMidiCh(i) == ch) {
            mVoiceStat[i].vol_l = mChStat[ch].changedVP.volL;
        }
    }
}

//-----------------------------------------------------------------------------
void C700Driver::changeChVolR(int ch, int volr)
{
    mChStat[ch].changedVP.volR = volr;
    mChStat[ch].changeFlg |= HAS_VOLR;
    // 発音中のボイスに反映
    for (int i=0; i<kMaximumVoices; i++) {
        if (mVoiceManager.GetVoiceMidiCh(i) == ch) {
            mVoiceStat[i].vol_r = mChStat[ch].changedVP.volR;
        }
    }
}
#endif
//-----------------------------------------------------------------------------
void C700Driver::changeChEcho(int ch, int echo)
{
    mChStat[ch].changedVP.echo = echo ? true:false;
    mChStat[ch].changeFlg |= HAS_ECHO;
    // 発音中のボイスに反映
    for (int i=0; i<kMaximumVoices; i++) {
        if (mVoiceManager.GetVoiceMidiCh(i) == ch) {
            mDSP.SetEchoOn(i, mChStat[ch].changedVP.echo);
        }
    }
}

//-----------------------------------------------------------------------------
void C700Driver::changeChPMON(int ch, int pmon)
{
    mChStat[ch].changedVP.pmOn = pmon ? true:false;
    mChStat[ch].changeFlg |= HAS_PMON;
    // 発音中のボイスに反映
    for (int i=0; i<kMaximumVoices; i++) {
        if (mVoiceManager.GetVoiceMidiCh(i) == ch) {
            mDSP.SetPMOn(i, mChStat[ch].changedVP.pmOn);
        }
    }
}

//-----------------------------------------------------------------------------
void C700Driver::changeChNON(int ch, int non)
{
    mChStat[ch].changedVP.noiseOn = non ? true:false;
    mChStat[ch].changeFlg |= HAS_NOISEON;
    // 発音中のボイスに反映
    for (int i=0; i<kMaximumVoices; i++) {
        if (mVoiceManager.GetVoiceMidiCh(i) == ch) {
            mDSP.SetNoiseOnFlg(i, mChStat[ch].changedVP.noiseOn);
        }
    }
}

//-----------------------------------------------------------------------------
void C700Driver::changeChBank(int ch, int bank)
{
    mChStat[ch].changedVP.bank = bank & 0x03;
    mChStat[ch].changeFlg |= HAS_BANK;
}

//-----------------------------------------------------------------------------
void C700Driver::changeChSustainMode(int ch, int sustainMode)
{
    mChStat[ch].changedVP.sustainMode = sustainMode ? true:false;
    mChStat[ch].changeFlg |= HAS_SUSTAINMODE;
}

//-----------------------------------------------------------------------------
float C700Driver::vibratoWave(float phase)
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
void C700Driver::processPortament(int vo)
{
    float   newPitch;
#if ANALOG_PORTAMENTO
    float   tc = mChStat[ mVoice[vo].midi_ch ].portaTc;
    float   tcInv = 1.0f - tc;
    newPitch = mVoice[vo].pitch * tcInv + mVoice[vo].portaPitch * tc;
    mVoice[vo].portaPitch = newPitch;
#else
    if ( mVoiceStat[vo].targetPitch > mVoiceStat[vo].portaPitch) {
        newPitch = mVoiceStat[vo].portaPitch * mChStat[ mVoiceManager.GetVoiceMidiCh(vo) ].portaTc;
        if (newPitch > mVoiceStat[vo].targetPitch) {
            newPitch = mVoiceStat[vo].targetPitch;
        }
        mVoiceStat[vo].portaPitch = newPitch;
    }
    else if ( mVoiceStat[vo].targetPitch < mVoiceStat[vo].portaPitch) {
        newPitch = mVoiceStat[vo].portaPitch / mChStat[ mVoiceManager.GetVoiceMidiCh(vo) ].portaTc;
        if (newPitch < mVoiceStat[vo].targetPitch) {
            newPitch = mVoiceStat[vo].targetPitch;
        }
        mVoiceStat[vo].portaPitch = newPitch;
    }
#endif
    mChStat[ mVoiceManager.GetVoiceMidiCh(vo) ].portaStartPitch = mVoiceStat[vo].portaPitch;
}

//-----------------------------------------------------------------------------
bool C700Driver::doNoteOnFirst( MIDIEvt dEvt )
{
    bool legato = false;
    dEvt.remain_samples = mEventDelaySamples;
    
    if (dEvt.type == NOTE_ON) {
        //ボイスを確保して再生準備状態にする
        InstParams		vp = getChannelVP(dEvt.ch, dEvt.note);
        int	v = -1;

        int releasedCh;
        v = mVoiceManager.AllocVoice(vp.noteOnPriority, dEvt.ch, dEvt.uniqueID,
                               vp.monoMode, &releasedCh, &legato);
        if (legato) {
            dEvt.note |= 0x80;  // レガートフラグ
        }
        
        if (v != -1) {
            if (legato == false) {
                //mDSP.KeyOffVoice(v);
                mKeyOffFlag[v] = true;
                
                mDSP.SetARDR(v, vp.ar, vp.dr);
                mDSP.SetSLSR(v, vp.sl, vp.sr1);
            }
            // 上位4bitに確保したボイス番号を入れる
            dEvt.ch += v << 4;
        }
    }
    mDelayedEvt.push_back(dEvt);
    return true;
}

//-----------------------------------------------------------------------------
void C700Driver::doNoteOnDelayed(const MIDIEvt *evt)
{
    int     midiCh = evt->ch & 0x0f;
    int     note = evt->note & 0x7f;
    
	InstParams		vp = getChannelVP(midiCh, note);
	
	//波形データが存在しない場合は、ここで中断
	if (!vp.hasBrrData()) {
		return;
	}
	
	//ボイスを取得
    int v = (evt->ch >> 4) & 0x0f;
    
    //mPlayVo に v が含まれていなかったら鳴らさない
    if (
        (mVoiceManager.IsPlayingVoice(v) == false) ||     // doNoteOn1を経ていない
        (mVoiceManager.GetVoiceUniqueID(v) != evt->uniqueID) //|| // 発音前にかき消されて上書きされた
        ) {
        return;
    }
	
	// 中心周波数の算出
	mVoiceStat[v].targetPitch = pow(2., (note - vp.basekey) / 12.)/INTERNAL_CLOCK*vp.rate*4096 + 0.5;
    
    if (vp.portamentoOn) {
        if (mChStat[midiCh].portaStartPitch == 0) {
            mChStat[midiCh].portaStartPitch = mVoiceStat[v].targetPitch;
        }
        mVoiceStat[v].portaPitch = mChStat[midiCh].portaStartPitch;
    }
    else {
        mVoiceStat[v].portaPitch = mVoiceStat[v].targetPitch;
    }
    
    mVoiceManager.SetKeyOn(v);
    
    if ((evt->note & 0x80) == 0) {
        //ベロシティの取得
        if ( mVelocityMode == kVelocityMode_Square ) {
            mVoiceStat[v].velo = VOLUME_CURB[evt->velo];
        }
        else if ( mVelocityMode == kVelocityMode_Linear ) {
            mVoiceStat[v].velo = evt->velo << 4;
        }
        else {
            mVoiceStat[v].velo=VOLUME_CURB[127];
        }
        
        mVoiceStat[v].volume = mChStat[midiCh].volume;
        mVoiceStat[v].expression = mChStat[midiCh].expression;
        mVoiceStat[v].pan = mChStat[midiCh].pan;
        mVoiceStat[v].pb = calcPBValue( midiCh, mChStat[midiCh].pitchBend, mVoiceStat[v].targetPitch );
        mVoiceStat[v].vibdepth = mChStat[midiCh].vibDepth;
        mVoiceStat[v].reg_pmod = mVoiceStat[v].vibdepth>0 ? true:false;
        mVoiceStat[v].vibPhase = 0.0f;
        mVoiceStat[v].vol_l=vp.volL;
        mVoiceStat[v].vol_r=vp.volR;
        mVoiceStat[v].non=vp.noiseOn;

        // 波形番号を指定する
        int     srcn;
        if (mDrumMode[vp.bank]) {
            srcn = GetKeyMap(vp.bank, evt->note);
        }
        else {
            srcn = mChStat[midiCh].prog;
        }
        mVoiceStat[v].srcn = srcn;
        mDSP.SetSrcn(v, srcn);
        if (!mIsAccurateMode) {
            mDSP.setBrr(v, vp.brrData(), vp.lp);
        }
        
        mEchoOnFlag &= ~(1 << v);
        if (vp.echo) {
            mEchoOnFlag |= 1 << v;
        }
        mEchoOnMask[v] = true;
        
        // PMONを設定
        mPMOnFlag &= ~(1 << v);
        if (vp.pmOn) {
            mPMOnFlag |= 1 << v;
        }
        mPMOnMask[v] = true;
        
        // NONを設定
        mNoiseOnFlag &= ~(1 << v);
        if (vp.noiseOn) {
            mNoiseOnFlag |= 1 << v;
        }
        mNoiseOnMask[v] = true;
        
        mDSP.SetARDR(v, vp.ar, vp.dr);
        mDSP.SetSLSR(v, vp.sl, vp.sr1);
        
        // キーオン
        mKeyOnFlag[v] = true;
        
        // 強制ピッチが再計算
        mPitchCount[v] = 0;
        
        // 最後に発音したノート番号を保存
        mChStat[midiCh].lastNote = note;
    }
}


//-----------------------------------------------------------------------------
int C700Driver::doNoteOff( const MIDIEvt *evt )
{
	int	stops=0;
    
    // 再生中なら停止する
    // 再生中でも未再生でも同じ処理で止められるはず
    InstParams		vp = getChannelVP(evt->ch, evt->note);
    int             vo=-1;
    stops = mVoiceManager.ReleaseVoice(vp.releasePriority, evt->ch, evt->uniqueID, &vo);
    if (stops > 0) {
        if (vp.sustainMode && (vp.sr2 != 31 || !mFastReleaseAsKeyOff)) {
            //キーオフさせずにsrを変更する
            mDSP.SetDR(vo, 7);
            mDSP.SetSR(vo, vp.sr2);
        }
        else {
            // キーオフ
            //mDSP.KeyOffVoice(vo);
            mKeyOffFlag[vo] = true;
        }
    }
	return stops;
}

//-----------------------------------------------------------------------------
bool C700Driver::doRegLogEvents( const MIDIEvt *evt )
{
    bool    handled = true;
    
    if (evt->type == START_REGLOG) {
        mDSP.BeginRegisterLog();
    }
    else if (evt->type == MARKLOOP_REGLOG) {
        mDSP.MarkRegisterLogLoop();
    }
    else if (evt->type == END_REGLOG) {
        mDSP.EndRegisterLog();
    }
    else {
        handled = false;
    }
    return handled;
}

//-----------------------------------------------------------------------------
bool C700Driver::doFirstEvents( const MIDIEvt *evt )
{
    bool    handled = true;
    
    if (evt->type == NOTE_OFF) {
        if (mChStat[evt->ch].damper) {
            handled = false;
        }
        else {
            doNoteOff( evt );
            //mChStat[evt->ch].noteOns -= stops;
        }
    }
    else {
        // ノートオフとレジスタログ以外のイベントは全て遅延実行する
        doNoteOnFirst(*evt);
    }
    
    return handled;
}

//-----------------------------------------------------------------------------
bool C700Driver::doDelayedEvents( const MIDIEvt *evt )
{
    bool    handled = true;
    
    switch (evt->type) {
        case NOTE_ON:
            doNoteOnDelayed( evt );
            break;
            
        case NOTE_OFF:
            //doNoteOff( evt );
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
void C700Driver::doControlChange( int ch, int controlNum, int value )
{
    switch (controlNum) {
        case 1:
            // モジュレーションホイール
            setModWheel(ch, value);
            break;
            
        case 5:
            // ポルタメントタイム
            setPortamentTime(ch, value);
            mChStat[ch].changedVP.portamentoRate = value;
            mChStat[ch].changeFlg |= HAS_PORTAMENTORATE;
            break;
            
        case 7:
            // ボリューム
            setVolume(ch, value);
            break;
            
        case 10:
            // パン
            setPanpot(ch, value);
            break;
            
        case 11:
            // エクスプレッション
            setExpression(ch, value);
            break;
            
        case 55:
            // チャンネル リミット
            changeChLimit(ch, value);
            break;
            
        case 56:
            // チャンネル プライオリティ
            changeChPriority(ch, value);
            break;
            
        case 57:
            // リリース プライオリティ
            changeReleasePriority(ch, value);
            break;
            
        case 64:
            // ホールド１（ダンパー）
            setDamper(ch, (value < 64)?false:true);
            break;
            
        case 65:
            // ポルタメント・オン・オフ
            changePortamentOn(ch, (value < 64)?false:true);
            break;
            
        case 72:
            // SR2
            changeChSR2(ch, value >> 2);
            break;
            
        case 82:
            // SR1
            changeChSR1(ch, value >> 2);
            break;

        case 73:
            // AR
            changeChAR(ch, value >> 3);
            break;
            
        case 80:
            // SL
            changeChSL(ch, value >> 4);
            break;
            
        case 75:
            // DR
            changeChDR(ch, value >> 4);
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
            setPortamentControl(ch, value);
            break;
            
        case 91:
            // ECEN ON/OFF
            changeChEcho(ch, (value < 64)?0:127);
            break;
            
        case 92:
            // PMON ON/OFF
            changeChPMON(ch, (value < 64)?0:127);
            break;
            
        case 93:
            // NON ON/OFF
            changeChNON(ch, (value < 64)?0:127);
            break;
            
        case 126:
            // Mono Mode
			changeMonoMode(ch, (value < 64)?false:true);
            break;
            
        case 127:
            // Poly Mode
            changeMonoMode(ch, (value < 64)?true:false);
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
            
        default:
            break;
    }
}

//-----------------------------------------------------------------------------
void C700Driver::Process( unsigned int frames, float *output[2] )
{
	int		procstep = (INTERNAL_CLOCK*CYCLES_PER_SAMPLE) / mSampleRate;    // CYCLES_PER_SAMPLE=1.0 とした固定小数
	
    mDSP.BeginFrameProcess((double)frames / mSampleRate);
    
	for (unsigned int frame=0; frame<frames; ++frame) {
        // REGLOGイベントの処理
        MutexLock(mREGLOGEvtMtx);
		if ( mREGLOGEvt.size() != 0 ) {
			std::list<MIDIEvt>::iterator	it = mREGLOGEvt.begin();
			while ( it != mREGLOGEvt.end() ) {
				if ( it->remain_samples >= 0 ) {
					it->remain_samples--;
                }
                if ( it->remain_samples < 0 ) {
                    if (doRegLogEvents(&(*it))) {
                        it = mREGLOGEvt.erase( it );
                        continue;
                    }
                }
				it++;
			}
		}
        MutexUnlock(mREGLOGEvtMtx);
        
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
        
        
		for ( ; mProcessFrac >= 0; mProcessFrac -= CYCLES_PER_SAMPLE ) {
            int kon = 0;
            int ecen = 0;
            int pmon = 0;
            int non = 0;
            
            while (mPortamentCount >= 0) {
                // ポルタメント処理
#if 0
                std::list<int>::iterator	it = mPlayVo.begin();
                while (it != mPlayVo.end()) {
                    int	vo = *it;
                    if (mVoiceManager.IsKeyOn(vo)) {
                        processPortament(vo);
                    }
                    it++;
                }
#else
                for (int i=0; i<mVoiceManager.GetVoiceLimit(); i++) {
                    if (mVoiceManager.IsKeyOn(i)) {
                        processPortament(i);
                    }
                }
#endif
                mPortamentCount -= PORTAMENT_CYCLE_SAMPLES;
            }
            
            {
                int koff = 0;
                for ( int v=0; v<kMaximumVoices; v++ ) {
                    if (mKeyOffFlag[v]) {
                        koff |= 1 << v;
                        mKeyOffFlag[v] = false;
                    }
                }
                if (koff) {
                    mDSP.KeyOffVoiceFlg(koff);
                }
            }
            
            
            for ( int v=0; v<kMaximumVoices; v++ ) {
				//ピッチの算出
                if (mPitchCount[v] >= 0) {
                    if (mVoiceStat[v].non) {
                        int midiCh = mVoiceManager.GetVoiceMidiCh(v);
                        int pb = (int)(mChStat[midiCh].pbRange * mChStat[midiCh].pitchBend + 0.5f);
                        int noiseFreq = (mChStat[midiCh].lastNote+pb+4) % 32;  // 60=0に
                        // ノイズ周波数を設定する
                        mDSP.SetNoiseFreq(noiseFreq);
                    }
                    else {
                        int voicePitch = static_cast<int>(mVoiceStat[v].portaPitch + 0.5f);
                        
                        int pitch = (voicePitch + mVoiceStat[v].pb) & 0x3fff;
                        
                        if (mVoiceStat[v].reg_pmod) {
                            mVoiceStat[v].vibPhase += mVibfreq;
                            if (mVoiceStat[v].vibPhase > onepi) {
                                mVoiceStat[v].vibPhase -= onepi*2;
                            }
                            
                            float vibwave = vibratoWave(mVoiceStat[v].vibPhase);
                            int pitchRatio = (vibwave*mVibdepth)*VOLUME_CURB[mVoiceStat[v].vibdepth];
                            
                            pitch = ( pitch * ( pitchRatio + 32768 ) ) >> 15;
                            if (pitch <= 0) {
                                pitch=1;
                            }
                        }
                        mDSP.SetPitch(v, pitch);
                    }
                    mPitchCount[v] = -PITCH_CYCLE_SAMPLES;
                }
                
                // パンのボリューム値への反映
                int volL = mVoiceStat[v].vol_l;
                int volR = mVoiceStat[v].vol_r;
                calcPanVolume(mVoiceStat[v].pan, &volL, &volR);
                
                // ベロシティの反映
                volL = ( volL * mVoiceStat[v].velo ) >> 11;
                volR = ( volR * mVoiceStat[v].velo ) >> 11;
                
                // ボリューム値の反映
                volL = ( volL * VOLUME_CURB[ mVoiceStat[v].volume ] ) / 0x7ff;
                volR = ( volR * VOLUME_CURB[ mVoiceStat[v].volume ] ) / 0x7ff;
                
                // エクスプレッションの反映
                volL = ( volL * VOLUME_CURB[ mVoiceStat[v].expression ] ) / 0x7ff;
                volR = ( volR * VOLUME_CURB[ mVoiceStat[v].expression ] ) / 0x7ff;
                
                mDSP.SetVol_L(v, volL);
                mDSP.SetVol_R(v, volR);
                
                if (mEchoOnMask[v]) {
                    ecen |= 1 << v;
                    mEchoOnMask[v] = false;
                }
                if (mPMOnMask[v]) {
                    pmon |= 1 << v;
                    mPMOnMask[v] = false;
                }
                if (mNoiseOnMask[v]) {
                    non |= 1 << v;
                    mNoiseOnMask[v] = false;
                }
                if (mKeyOnFlag[v]) {
                    kon |= 1 << v;
                    mKeyOnFlag[v] = false;
                }
                
                mPitchCount[v]++;
            }
            if (ecen) {
                mDSP.SetEchoOnFlg(mEchoOnFlag, ecen);
            }
            if (pmon) {
                mDSP.SetPMOnFlg(mPMOnFlag, pmon);
            }
            if (non) {
                mDSP.SetNoiseOnFlg(mNoiseOnFlag, non);
            }
            if (kon) {
                mDSP.KeyOnVoiceFlg(kon);
            }
            
            mPortamentCount++;

            int outl=0,outr=0;
            mDSP.Process1Sample(outl, outr);
			
            mProcessbuf[0][mProcessbufPtr] = outl;
			mProcessbuf[1][mProcessbufPtr] = outr;
			mProcessbufPtr=(mProcessbufPtr+1)&0x0f;
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
void C700Driver::calcPanVolume(int value, int *volL, int *volR)
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
void C700Driver::RefreshKeyMap(void)
{
	if ( mVPset ) {
		bool	initialized[NUM_BANKS];
		for (int i=0; i<NUM_BANKS; i++ ) {
			initialized[i] = false;
		}
		
		for (int prg=0; prg<128; prg++) {
			if (mVPset[prg].hasBrrData()) {
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
void C700Driver::SetVPSet( InstParams *vp )
{
    mVPset = vp;
    for (int i=0; i<16; i++) {
        mChStat[i].changeFlg = 0;
        mChStat[i].changedVP = vp[0];
    }
}

//-----------------------------------------------------------------------------
double C700Driver::GetProcessDelayTime()
{
    return ((mEventDelayClocks / CLOCKS_PER_SAMPLE) + 8) / static_cast<double>(INTERNAL_CLOCK);    // 8ms + resample
}

//-----------------------------------------------------------------------------
void C700Driver::setRPNLSB(int ch, int value)
{
    mRPN[ch] &= 0xff00;
    mRPN[ch] |= value & 0x7f;
    mIsSettingNRPN[ch] = false;
}

//-----------------------------------------------------------------------------
void C700Driver::setRPNMSB(int ch, int value)
{
    mRPN[ch] &= 0x00ff;
    mRPN[ch] |= (value & 0x7f) << 8;
    mIsSettingNRPN[ch] = false;
}

//-----------------------------------------------------------------------------
void C700Driver::setNRPNLSB(int ch, int value)
{
    mNRPN[ch] &= 0xff00;
    mNRPN[ch] |= value & 0x7f;
    mIsSettingNRPN[ch] = true;
}

//-----------------------------------------------------------------------------
void C700Driver::setNRPNMSB(int ch, int value)
{
    mNRPN[ch] &= 0x00ff;
    mNRPN[ch] |= (value & 0x7f) << 8;
    mIsSettingNRPN[ch] = true;
}

//-----------------------------------------------------------------------------
void C700Driver::setDataEntryLSB(int ch, int value)
{
    mDataEntryValue[ch] &= 0xff00;
    mDataEntryValue[ch] |= value & 0x7f;
    sendDataEntryValue(ch);
}

//-----------------------------------------------------------------------------
void C700Driver::setDataEntryMSB(int ch, int value)
{
    mDataEntryValue[ch] &= 0x00ff;
    mDataEntryValue[ch] |= (value & 0x7f) << 8;
}

//-----------------------------------------------------------------------------
void C700Driver::sendDataEntryValue(int ch)
{
    if (mIsSettingNRPN) {
        if ((mNRPN[ch] & 0xff00) == 0x7e00) {   // #98:rr #99:126
            mDSP.WriteReg(mNRPN[ch] & 0x00ff, mDataEntryValue[ch]);
        }
    }
    else  {
        switch (mRPN[ch]) {
            case 0x0000:
                setPBRange(ch, mDataEntryValue[ch]);
                break;
                
            default:
                break;
        }
    }
}
