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
    pitch = .0f;
    pan = 64;
    non = false;
	
    porta.Reset();
    lfo.Reset();
}

//-----------------------------------------------------------------------------
C700Driver::C700Driver()
: MidiDriverBase(8),
  mSampleRate(44100.0),
  mVelocityMode( kVelocityMode_Square ),
  mVPset(NULL)
{
    MutexInit(mREGLOGEvtMtx);
    
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
    mIsAccurateMode = false;
    mFastReleaseAsKeyOff = true;

    mEventDelayClocks = 8192;   // 8ms
    SetNoteOffIntervalCycles(calcEventDelaySamples());
    
	for (int i=0; i<16; i++) {
        mCCChangeFlg[i] = 0;
        mPortaStartPitch[i] = 0;
        mChPortaTc[i] = 1.0f;
	}
    
    for (int v=0; v<kMaximumVoices; v++) {
        mVoiceStat[v].lfo.SetUpdateRate(INTERNAL_CLOCK / PITCH_CYCLE_SAMPLES);
    }
	Reset();
}

//-----------------------------------------------------------------------------
C700Driver::~C700Driver()
{
    MutexDestroy(mREGLOGEvtMtx);
}

//-----------------------------------------------------------------------------
void C700Driver::Reset()
{
    MidiDriverBase::Reset();
    
	for (int i=0; i<16; i++) {
		mProcessbuf[0][i]=0;
		mProcessbuf[1][i]=0;
	}
	mProcessFrac=0;
	mProcessbufPtr=0;
    mPortamentCount=0;
    for (int i=0; i<kMaximumVoices; i++) {
        mPitchCount[i] = 0;
    }
}

//-----------------------------------------------------------------------------
void C700Driver::StartRegisterLog( int inFrame )
{
    RegLogEvt	evt;
	evt.type = START_REGLOG;
	evt.toWaitCycles = inFrame;
    MutexLock(mREGLOGEvtMtx);
	mREGLOGEvt.push_back( evt );
    MutexUnlock(mREGLOGEvtMtx);
}

//-----------------------------------------------------------------------------
void C700Driver::MarkLoopRegisterLog( int inFrame )
{
    RegLogEvt	evt;
	evt.type = MARKLOOP_REGLOG;
	evt.toWaitCycles = inFrame;
    MutexLock(mREGLOGEvtMtx);
	mREGLOGEvt.push_back( evt );
    MutexUnlock(mREGLOGEvtMtx);
}

//-----------------------------------------------------------------------------
void C700Driver::EndRegisterLog( int inFrame )
{
    RegLogEvt	evt;
	evt.type = END_REGLOG;
	evt.toWaitCycles = inFrame;
    MutexLock(mREGLOGEvtMtx);
	mREGLOGEvt.push_back( evt );
    MutexUnlock(mREGLOGEvtMtx);
}

//-----------------------------------------------------------------------------
void C700Driver::handleAllNotesOff()
{
    //mClearEvent = true;
    mDSP.KeyOffAll();
	for ( int i=0; i<kMaximumVoices; i++ ) {
        mVoiceStat[i].Reset();
	}
    mKeyOnFlag = 0;
    mKeyOffFlag = 0;
    mEchoOnFlag = 0;
    mPMOnFlag = 0;
    mNoiseOnFlag = 0;
}

//-----------------------------------------------------------------------------
void C700Driver::handleAllSoundOff(int ch)
{
	mDSP.ResetEcho();
}

//-----------------------------------------------------------------------------
void C700Driver::handleResetAllControllers(int ch)
{
    mCCChangeFlg[ch] = 0;
    mChannnelInst[ch] = mVPset[mChStat[ch].prog];
}

//-----------------------------------------------------------------------------
void C700Driver::handleProgramChange( int ch, int value )
{
    if (mVPset) {
        mCCChangeFlg[ch] = 0;
        mChannnelInst[ch] = mVPset[value];
        
        handleControlChange(ch, 5, mVPset[value].portamentoRate);
    }
}

//-----------------------------------------------------------------------------
void C700Driver::handlePitchBend( int ch, sint16_t pitchbend )
{
	for ( int i=0; i<kMaximumVoices; i++ ) {
		if ( GetVoiceMidiCh(i) == ch ) {
			mVoiceStat[i].pb = calcPBValue( ch, pitchbend, mVoiceStat[i].porta.GetTargetPitch() );
		}
	}
}

//-----------------------------------------------------------------------------
void C700Driver::handleModWheelChange( int ch, int value )
{
	for ( int i=0; i<kMaximumVoices; i++ ) {
		if ( GetVoiceMidiCh(i) == ch ) {
			mVoiceStat[i].lfo.SetVibSens(value);
			mVoiceStat[i].reg_pmod = value > 0 ? true:false;
		}
	}
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
        mChStat[i].pbRange = (int)value;
    }
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
            mVoiceManager.SetVoiceAllocMode(DynamicVoiceAllocator::ALLOC_MODE_OLDEST);
            break;
        case kVoiceAllocMode_SameChannel:
            mVoiceManager.SetVoiceAllocMode(DynamicVoiceAllocator::ALLOC_MODE_SAMECH);
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
    for (int i=0; i<kMaximumVoices; i++) {
        mVoiceStat[i].lfo.SetVibFreq(value);
    }
}

//-----------------------------------------------------------------------------
void C700Driver::SetVibDepth( int ch, float value )
{
    for (int i=0; i<kMaximumVoices; i++) {
        mVoiceStat[i].lfo.SetVibDepth(value);
    }
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
    SetNoteOffIntervalCycles(calcEventDelaySamples());
}

//-----------------------------------------------------------------------------
void C700Driver::UpdatePortamentoTime( int prog )
{
    if (mVPset == NULL) {
        return;
    }
    
    for (int i=0; i<16; i++) {
        if (mChStat[i].prog == prog) {
            handleControlChange(i, 5, mVPset[prog].portamentoRate);
        }
    }
}

//-----------------------------------------------------------------------------
void C700Driver::changeChPriority( int ch, int value )
{
    mChannnelInst[ch].noteOnPriority = value;
    mCCChangeFlg[ch] |= HAS_NOTEONPRIORITY;
}

//-----------------------------------------------------------------------------
void C700Driver::changeReleasePriority( int ch, int value )
{
    mChannnelInst[ch].releasePriority = value;
    mCCChangeFlg[ch] |= HAS_RELEASEPRIORITY;
}

//-----------------------------------------------------------------------------
void C700Driver::changeMonoMode( int ch, bool on )
{
    mChannnelInst[ch].monoMode = on;
    mCCChangeFlg[ch] |= HAS_MONOMODE;
}

//-----------------------------------------------------------------------------
bool C700Driver::isPatchLoaded(int ch, int note)
{
    const InstParams  *pgVP = &mVPset[mChStat[ch].prog];
    if (mDrumMode[pgVP->bank]) {
        return getMappedVP(pgVP->bank, note)->hasBrrData();
    }
    else {
        return pgVP->hasBrrData();
    }
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
        if (mCCChangeFlg[ch] & HAS_RATE) mergedVP.rate = mChannnelInst[ch].rate;
        if (mCCChangeFlg[ch] & HAS_BASEKEY) mergedVP.basekey = mChannnelInst[ch].basekey;
        if (mCCChangeFlg[ch] & HAS_LOWKEY) mergedVP.lowkey = mChannnelInst[ch].lowkey;
        if (mCCChangeFlg[ch] & HAS_HIGHKEY) mergedVP.highkey = mChannnelInst[ch].highkey;
        if (mCCChangeFlg[ch] & HAS_AR) mergedVP.ar = mChannnelInst[ch].ar;
        if (mCCChangeFlg[ch] & HAS_DR) mergedVP.dr = mChannnelInst[ch].dr;
        if (mCCChangeFlg[ch] & HAS_SL) mergedVP.sl = mChannnelInst[ch].sl;
        if (mCCChangeFlg[ch] & HAS_SR1) mergedVP.sr1 = mChannnelInst[ch].sr1;
        if (mCCChangeFlg[ch] & HAS_SR2) mergedVP.sr2 = mChannnelInst[ch].sr2;
        if (mCCChangeFlg[ch] & HAS_VOLL) mergedVP.volL = mChannnelInst[ch].volL;
        if (mCCChangeFlg[ch] & HAS_VOLR) mergedVP.volR = mChannnelInst[ch].volR;
        if (mCCChangeFlg[ch] & HAS_ECHO) mergedVP.echo = mChannnelInst[ch].echo;
        if (mCCChangeFlg[ch] & HAS_BANK) mergedVP.bank = mChannnelInst[ch].bank;
        if (mCCChangeFlg[ch] & HAS_SUSTAINMODE) mergedVP.sustainMode = mChannnelInst[ch].sustainMode;
        if (mCCChangeFlg[ch] & HAS_MONOMODE) mergedVP.monoMode = mChannnelInst[ch].monoMode;
        if (mCCChangeFlg[ch] & HAS_PORTAMENTOON) mergedVP.portamentoOn = mChannnelInst[ch].portamentoOn;
        if (mCCChangeFlg[ch] & HAS_PORTAMENTORATE) mergedVP.portamentoRate = mChannnelInst[ch].portamentoRate;
        if (mCCChangeFlg[ch] & HAS_NOTEONPRIORITY) mergedVP.noteOnPriority = mChannnelInst[ch].noteOnPriority;
        if (mCCChangeFlg[ch] & HAS_RELEASEPRIORITY) mergedVP.releasePriority = mChannnelInst[ch].releasePriority;
        if (mCCChangeFlg[ch] & HAS_PMON) mergedVP.pmOn = mChannnelInst[ch].pmOn;
        if (mCCChangeFlg[ch] & HAS_NOISEON) mergedVP.noiseOn = mChannnelInst[ch].noiseOn;
        
        return mergedVP;
    }
}

//-----------------------------------------------------------------------------
void C700Driver::handleVolumeChange( int ch, int value )
{
    // 発音中のボイスに反映
    for (int i=0; i<kMaximumVoices; i++) {
        if (GetVoiceMidiCh(i) == ch) {
            mVoiceStat[i].volume = mChStat[ch].volume;
        }
    }
}

//-----------------------------------------------------------------------------
void C700Driver::handleExpressionChange( int ch, int value )
{
    // 発音中のボイスに反映
    for (int i=0; i<kMaximumVoices; i++) {
        if (GetVoiceMidiCh(i) == ch) {
            mVoiceStat[i].expression = mChStat[ch].expression;
        }
    }
}

//-----------------------------------------------------------------------------
void C700Driver::handlePanpotChange( int ch, int value )
{
    // 発音中のボイスに反映
    for (int i=0; i<kMaximumVoices; i++) {
        if (GetVoiceMidiCh(i) == ch) {
            mVoiceStat[i].pan = mChStat[ch].pan;
        }
    }
}

//-----------------------------------------------------------------------------
void C700Driver::handlePortamentStartNoteChange( int ch, int note )
{
    InstParams		vp = getChannelVP(ch, mChStat[ch].lastNote);
    mPortaStartPitch[ch] = pow(2., (note - vp.basekey) / 12.)/INTERNAL_CLOCK*vp.rate*4096 + 0.5;
}

//-----------------------------------------------------------------------------
void C700Driver::handlePortaTimeChange( int ch, int ccValue, float centPerMilis )
{
    mChannnelInst[ch].portamentoRate = ccValue;
    mCCChangeFlg[ch] |= HAS_PORTAMENTORATE;
    
    float portaTc = powf(2.0f, centPerMilis / 1200);
    mChPortaTc[ch] = portaTc;
    // 発音中のボイスに反映
    for (int i=0; i<kMaximumVoices; i++) {
        if (GetVoiceMidiCh(i) == ch) {
            mVoiceStat[i].porta.SetTc(portaTc);
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
    mChannnelInst[ch].ar = ar & 0x0f;
    mCCChangeFlg[ch] |= HAS_AR;
    // 発音中のボイスに反映
    for (int i=0; i<kMaximumVoices; i++) {
        if (GetVoiceMidiCh(i) == ch) {
            mDSP.SetAR(i, mChannnelInst[ch].ar);
        }
    }
}

//-----------------------------------------------------------------------------
void C700Driver::changeChDR(int ch, int dr)
{
    mChannnelInst[ch].dr = dr & 0x07;
    mCCChangeFlg[ch] |= HAS_DR;
    // 発音中のボイスに反映
    for (int i=0; i<kMaximumVoices; i++) {
        if (GetVoiceMidiCh(i) == ch) {
            mDSP.SetDR(i, mChannnelInst[ch].dr);
        }
    }
}

//-----------------------------------------------------------------------------
void C700Driver::changeChSL(int ch, int sl)
{
    mChannnelInst[ch].sl = sl & 0x07;
    mCCChangeFlg[ch] |= HAS_SL;
    // 発音中のボイスに反映
    for (int i=0; i<kMaximumVoices; i++) {
        if (GetVoiceMidiCh(i) == ch) {
            mDSP.SetSL(i, mChannnelInst[ch].sl);
        }
    }
}

//-----------------------------------------------------------------------------
void C700Driver::changeChSR1(int ch, int sr)
{
    mChannnelInst[ch].sr1 = sr & 0x1f;
    mCCChangeFlg[ch] |= HAS_SR1;
    // 発音中のボイスに反映
    for (int i=0; i<kMaximumVoices; i++) {
        if (IsKeyOnVoice(i)) {
            if (GetVoiceMidiCh(i) == ch) {
                mDSP.SetSR(i, mChannnelInst[ch].sr1);
            }
        }
    }
}

//-----------------------------------------------------------------------------
void C700Driver::changeChSR2(int ch, int sr)
{
    mChannnelInst[ch].sr2 = sr & 0x1f;
    mCCChangeFlg[ch] |= HAS_SR2;
    // 発音中のボイスに反映
    for (int i=0; i<kMaximumVoices; i++) {
        if (!IsKeyOnVoice(i)) {
            if (GetVoiceMidiCh(i) == ch) {
                mDSP.SetSR(i, mChannnelInst[ch].sr2);
            }
        }
    }
}

//-----------------------------------------------------------------------------
#if 0
void C700Driver::changeChVolL(int ch, int voll)
{
    mChannnelInst[ch].volL = voll;
    mCCChangeFlg[ch] |= HAS_VOLL;
    // 発音中のボイスに反映
    for (int i=0; i<kMaximumVoices; i++) {
        if (GetVoiceMidiCh(i) == ch) {
            mVoiceStat[i].vol_l = mChannnelInst[ch].volL;
        }
    }
}

//-----------------------------------------------------------------------------
void C700Driver::changeChVolR(int ch, int volr)
{
    mChannnelInst[ch].volR = volr;
    mCCChangeFlg[ch] |= HAS_VOLR;
    // 発音中のボイスに反映
    for (int i=0; i<kMaximumVoices; i++) {
        if (GetVoiceMidiCh(i) == ch) {
            mVoiceStat[i].vol_r = mChannnelInst[ch].volR;
        }
    }
}
#endif
//-----------------------------------------------------------------------------
void C700Driver::changeChEcho(int ch, int echo)
{
    mChannnelInst[ch].echo = echo ? true:false;
    mCCChangeFlg[ch] |= HAS_ECHO;
    // 発音中のボイスに反映
    for (int i=0; i<kMaximumVoices; i++) {
        if (GetVoiceMidiCh(i) == ch) {
            mDSP.SetEchoOn(i, mChannnelInst[ch].echo);
        }
    }
}

//-----------------------------------------------------------------------------
void C700Driver::changeChPMON(int ch, int pmon)
{
    mChannnelInst[ch].pmOn = pmon ? true:false;
    mCCChangeFlg[ch] |= HAS_PMON;
    // 発音中のボイスに反映
    for (int i=0; i<kMaximumVoices; i++) {
        if (GetVoiceMidiCh(i) == ch) {
            mDSP.SetPMOn(i, mChannnelInst[ch].pmOn);
        }
    }
}

//-----------------------------------------------------------------------------
void C700Driver::changeChNON(int ch, int non)
{
    mChannnelInst[ch].noiseOn = non ? true:false;
    mCCChangeFlg[ch] |= HAS_NOISEON;
    // 発音中のボイスに反映
    for (int i=0; i<kMaximumVoices; i++) {
        if (GetVoiceMidiCh(i) == ch) {
            mDSP.SetNoiseOn(i, mChannnelInst[ch].noiseOn);
        }
    }
}

//-----------------------------------------------------------------------------
void C700Driver::changeChBank(int ch, int bank)
{
    mChannnelInst[ch].bank = bank & 0x03;
    mCCChangeFlg[ch] |= HAS_BANK;
}

//-----------------------------------------------------------------------------
void C700Driver::changeChSustainMode(int ch, int sustainMode)
{
    mChannnelInst[ch].sustainMode = sustainMode ? true:false;
    mCCChangeFlg[ch] |= HAS_SUSTAINMODE;
}

//-----------------------------------------------------------------------------
int C700Driver::calcPBValue( int ch, int pitchBend, int basePitch )
{
    float pb_value = pitchBend / 8192.0;
    return (int)((pow(2., (pb_value * mChStat[ch].pbRange) / 12.) - 1.0)*basePitch);
}

//-----------------------------------------------------------------------------
double C700Driver::GetProcessDelayTime()
{
    return ((mEventDelayClocks / CLOCKS_PER_SAMPLE) + 8) / static_cast<double>(INTERNAL_CLOCK);    // 8ms + resample
}

//-----------------------------------------------------------------------------
void C700Driver::SetEventDelayClocks(int clocks)
{
    mEventDelayClocks = clocks;
    SetNoteOffIntervalCycles(calcEventDelaySamples());
}

//-----------------------------------------------------------------------------
int  C700Driver::getKeyOnPriority(int ch, int note)
{
    return getChannelVP(ch, note).noteOnPriority;
}

//-----------------------------------------------------------------------------
int  C700Driver::getReleasePriority(int ch, int note)
{
    return getChannelVP(ch, note).releasePriority;
}

//-----------------------------------------------------------------------------
bool C700Driver::isMonoMode(int ch, int note)
{
    return getChannelVP(ch, note).monoMode;
}

//-----------------------------------------------------------------------------
bool C700Driver::handleNoteOnFirst( uint8_t vo, uint8_t midiCh, uint8_t note, uint8_t velo, bool isLegato, int killedMidiCh )
{
    if (!isLegato) {
        if (killedMidiCh >= 0) {
            mKeyOffFlag |= 1 << vo;
        }
        
        InstParams		vp = getChannelVP(midiCh & 0x0f, note);
        mDSP.SetARDR(vo, vp.ar, vp.dr);
        mDSP.SetSLSR(vo, vp.sl, vp.sr1);
        
        if (vp.portamentoOn) {
            handleControlChange(midiCh, 65, 127);
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
bool C700Driver::handleNoteOnDelayed(uint8_t v, uint8_t midiCh, uint8_t note, uint8_t velo, bool isLegato)
{
    InstParams		vp = getChannelVP(midiCh, note);
    
	// 中心周波数の算出
    int targetPitch = pow(2., (note - vp.basekey) / 12.)/INTERNAL_CLOCK*vp.rate*4096 + 0.5;
    mVoiceStat[v].porta.SetTargetPicth(targetPitch);
    
    if ((mChStat[midiCh].portaOn == true) &&
        (mPortaStartPitch[midiCh] != 0)) {
        mVoiceStat[v].pitch = mPortaStartPitch[midiCh];
    }
    else {
        mVoiceStat[v].pitch = targetPitch;
    }
    mVoiceStat[v].porta.SetTc(mChPortaTc[midiCh]);
    
    if (!isLegato) {
        //ベロシティの取得
        if ( mVelocityMode == kVelocityMode_Square ) {
            mVoiceStat[v].velo = VOLUME_CURB[velo];
        }
        else if ( mVelocityMode == kVelocityMode_Linear ) {
            mVoiceStat[v].velo = velo << 4;
        }
        else {
            mVoiceStat[v].velo=VOLUME_CURB[127];
        }
        
        mVoiceStat[v].volume = mChStat[midiCh].volume;
        mVoiceStat[v].expression = mChStat[midiCh].expression;
        mVoiceStat[v].pan = mChStat[midiCh].pan;
        mVoiceStat[v].pb = calcPBValue( midiCh, mChStat[midiCh].pitchBend, targetPitch );
        mVoiceStat[v].lfo.SetVibSens(mChStat[midiCh].vibDepth);
        mVoiceStat[v].reg_pmod = mChStat[midiCh].vibDepth>0 ? true:false;
        mVoiceStat[v].lfo.ResetPhase();
        mVoiceStat[v].vol_l=vp.volL;
        mVoiceStat[v].vol_r=vp.volR;
        mVoiceStat[v].non=vp.noiseOn;

        // 波形番号を指定する
        int     srcn;
        if (mDrumMode[vp.bank]) {
            srcn = GetKeyMap(vp.bank, note);
        }
        else {
            srcn = mChStat[midiCh].prog;
        }
        mVoiceStat[v].srcn = srcn;
        mDSP.SetSrcn(v, srcn);
        if (!mIsAccurateMode) {
            mDSP.setBrr(v, vp.brrData(), vp.lp);
        }
        
        if (vp.echo) {
            mEchoOnFlag |= 1 << v;
        }
        
        // PMONを設定
        if (vp.pmOn) {
            mPMOnFlag |= 1 << v;
        }
        
        // NONを設定
        if (vp.noiseOn) {
            mNoiseOnFlag |= 1 << v;
        }
        
        mDSP.SetARDR(v, vp.ar, vp.dr);
        mDSP.SetSLSR(v, vp.sl, vp.sr1);
        
        // キーオン
        mKeyOnFlag |= 1 << v;
        
        // 強制ピッチが再計算
        mPitchCount[v] = 0;
        
        // キーオンされた場合はtrueを返す
        return true;
    }
    return false;
}


//-----------------------------------------------------------------------------
void C700Driver::handleNoteOff( const MIDIEvt *evt, int vo )
{
    InstParams		vp = getChannelVP(evt->ch, evt->data1);
    if (vp.sustainMode && (vp.sr2 != 31 || !mFastReleaseAsKeyOff)) {
        //キーオフさせずにsrを変更する
        mDSP.SetDR(vo, 7);
        mDSP.SetSR(vo, vp.sr2);
    }
    else {
        // キーオフ
        mKeyOffFlag |= 1 << vo;
    }
}

//-----------------------------------------------------------------------------
bool C700Driver::doRegLogEvents( const RegLogEvt *evt )
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
void C700Driver::handleControlChange( int ch, int controlNum, int value )
{
    switch (controlNum) {
        case 56:
            // チャンネル プライオリティ
            changeChPriority(ch, value);
            break;
            
        case 57:
            // リリース プライオリティ
            changeReleasePriority(ch, value);
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
            
        default:
            // サブクラスの処理を優先する
            MidiDriverBase::handleControlChange(ch, controlNum, value);
            break;
    }
}

//-----------------------------------------------------------------------------
void C700Driver::doPreMidiEvents()
{
    // REGLOGイベントの処理
    MutexLock(mREGLOGEvtMtx);
    if ( mREGLOGEvt.size() != 0 ) {
        std::list<RegLogEvt>::iterator	it = mREGLOGEvt.begin();
        while ( it != mREGLOGEvt.end() ) {
            if ( it->toWaitCycles >= 0 ) {
                it->toWaitCycles--;
            }
            if ( it->toWaitCycles < 0 ) {
                if (doRegLogEvents(&(*it))) {
                    it = mREGLOGEvt.erase( it );
                    continue;
                }
            }
            it++;
        }
    }
    MutexUnlock(mREGLOGEvtMtx);
}

//-----------------------------------------------------------------------------
void C700Driver::doPostMidiEvents()
{
    while (mPortamentCount >= 0) {
        // ポルタメント処理
        for (int v=0; v<GetVoiceLimit(); v++) {
            if (IsKeyOnVoice(v)) {
                mVoiceStat[v].pitch = mPortaStartPitch[GetVoiceMidiCh(v)] = mVoiceStat[v].porta.Update(mVoiceStat[v].pitch);
            }
        }
        mPortamentCount -= PORTAMENT_CYCLE_SAMPLES;
    }
    mPortamentCount++;
    
    if (mKeyOffFlag) {
        mDSP.KeyOffVoiceFlg(mKeyOffFlag);
        mKeyOffFlag = 0;
    }
    
    for ( int v=0; v<kMaximumVoices; v++ ) {
        //ピッチの算出
        if (mPitchCount[v] >= 0) {
            if (mVoiceStat[v].non) {
                int midiCh = GetVoiceMidiCh(v);
                int pb = (int)(mChStat[midiCh].pbRange * (mChStat[midiCh].pitchBend / 8192.0) + 0.5f);
                int noiseFreq = (mChStat[midiCh].lastNote+pb+4) % 32;  // 60=0に
                // ノイズ周波数を設定する
                mDSP.SetNoiseFreq(noiseFreq);
            }
            else {
                int voicePitch = static_cast<int>(mVoiceStat[v].pitch + 0.5f);
                
                int pitch = (voicePitch + mVoiceStat[v].pb) & 0x3fff;
                
                if (mVoiceStat[v].reg_pmod) {
                    int pitchRatio = mVoiceStat[v].lfo.Update(pitch);
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
        
        mPitchCount[v]++;
    }
    if (mKeyOnFlag) {
        mDSP.SetEchoOnFlg(mEchoOnFlag, mKeyOnFlag);
    }
    mEchoOnFlag = 0;
    if (mKeyOnFlag) {
        mDSP.SetPMOnFlg(mPMOnFlag, mKeyOnFlag);
    }
    mPMOnFlag = 0;
    if (mKeyOnFlag) {
        mDSP.SetNoiseOnFlg(mNoiseOnFlag, mKeyOnFlag);
    }
    mNoiseOnFlag = 0;
    if (mKeyOnFlag) {
        mDSP.KeyOnVoiceFlg(mKeyOnFlag);
    }
    mKeyOnFlag = 0;
    
    
    int outl=0,outr=0;
    mDSP.Process1Sample(outl, outr);
    
    mProcessbuf[0][mProcessbufPtr] = outl;
    mProcessbuf[1][mProcessbufPtr] = outr;
    mProcessbufPtr=(mProcessbufPtr+1)&0x0f;
}

//-----------------------------------------------------------------------------
void C700Driver::Process( unsigned int frames, float *output[2] )
{
	int		procstep = (INTERNAL_CLOCK*CYCLES_PER_SAMPLE) / mSampleRate;    // CYCLES_PER_SAMPLE=1.0 とした固定小数
	
    mDSP.BeginFrameProcess((double)frames / mSampleRate);
    
	for (unsigned int frame=0; frame<frames; ++frame) {
        doPreMidiEvents();
        
        ProcessMidiEvents();
        
		for ( ; mProcessFrac >= 0; mProcessFrac -= CYCLES_PER_SAMPLE ) {
            doPostMidiEvents();
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
        mCCChangeFlg[i] = 0;
        mChannnelInst[i] = vp[0];
    }
}

//-----------------------------------------------------------------------------
void C700Driver::handleDataEntryValueChange(int ch, bool isNRPN, int addr, int value)
{
    if (isNRPN) {
        if ((addr & 0xff00) == 0x7e00) {   // #98:rr #99:126
            mDSP.WriteReg(addr & 0x00ff, value);
        }
    }
    else  {
//        switch (addr) {
//            default:
                MidiDriverBase::handleDataEntryValueChange(ch, isNRPN, addr, value);
//                break;
//        }
    }
}
