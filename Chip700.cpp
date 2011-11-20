#include "Chip700.h"
#include "samplebrr.h"
#include "brrcodec.h"
#include <AudioToolbox/AudioToolbox.h>


static CFStringRef kParam_poly_Name = CFSTR("Voices");
static const float kDefaultValue_poly = 8;

static CFStringRef kParameter_mainvol_L_Name = CFSTR("MainVol(L)");
static const float kDefaultValue_mainvol_L = 127;

static CFStringRef kParameter_mainvol_R_Name = CFSTR("MainVol(R)");
static const float kDefaultValue_mainvol_R = 127;

static CFStringRef kParam_vibdepth_Name = CFSTR("Vibrato Depth");
static const float kDefaultValue_vibdepth = 0;

static CFStringRef kParam_vibrate_Name = CFSTR("Vibrato Rate");
static const float kDefaultValue_vibrate = 7;
static const float kMinimumValue_vibrate = 0.01;
static const float kMaximumValue_vibrate = 35;

static CFStringRef kParam_vibdepth2_Name = CFSTR("Vibrato Depth2");
static const float kDefaultValue_vibdepth2 = 1;

static CFStringRef kParam_velocity_Name = CFSTR("Velocity");
static const float kDefaultValue_velocity = 1;

static CFStringRef kParam_clipnoise_Name = CFSTR("Cliping Noise");
static const float kDefaultValue_clipnoise = 0;

static CFStringRef kParam_bendrange_Name = CFSTR("Bend Range");
static const float kDefaultValue_bendrange = 2;

static CFStringRef kParam_program_Name = CFSTR("Program");
static const float kDefaultValue_program = 0;

static CFStringRef kParam_drummode_Name = CFSTR("Drum Mode");
static const float kDefaultValue_drummode = 0;

//エコー部
static CFStringRef kParameter_echovol_L_Name = CFSTR("Wet(L)");
static const float kDefaultValue_echovol_L = 50;

static CFStringRef kParameter_echovol_R_Name = CFSTR("Wet(R)");
static const float kDefaultValue_echovol_R = -50;

static CFStringRef kParameter_echoFB_Name = CFSTR("FeedBack");
static const float kDefaultValue_echoFB = -70;

static CFStringRef kParameter_echodelay_Name = CFSTR("Delay");
static const float kDefaultValue_echodelay = 6;

static CFStringRef kParameter_firC0_Name = CFSTR("Filter a0");
static const float kDefaultValue_fir0 = 127;

static CFStringRef kParameter_firC1_Name = CFSTR("Filter a1");
static const float kDefaultValue_fir1 = 0;

static CFStringRef kParameter_firC2_Name = CFSTR("Filter a2");
static const float kDefaultValue_fir2 = 0;

static CFStringRef kParameter_firC3_Name = CFSTR("Filter a3");
static const float kDefaultValue_fir3 = 0;

static CFStringRef kParameter_firC4_Name = CFSTR("Filter a4");
static const float kDefaultValue_fir4 = 0;

static CFStringRef kParameter_firC5_Name = CFSTR("Filter a5");
static const float kDefaultValue_fir5 = 0;

static CFStringRef kParameter_firC6_Name = CFSTR("Filter a6");
static const float kDefaultValue_fir6 = 0;

static CFStringRef kParameter_firC7_Name = CFSTR("Filter a7");
static const float kDefaultValue_fir7 = 0;


static const int kNumberPresets = 2;
static AUPreset kPresets[kNumberPresets] = 
{
	{ 0, CFSTR("Empty") },		
	{ 1, CFSTR("Testtones") }		
};

static const int kDefaultValue_AR = 15;
static const int kDefaultValue_DR = 7;
static const int kDefaultValue_SL = 7;
static const int kDefaultValue_SR = 0;


static CFStringRef kSaveKey_ProgName = CFSTR("progname");
static CFStringRef kSaveKey_EditProg = CFSTR("editprog");
static CFStringRef kSaveKey_brrdata = CFSTR("brrdata");
static CFStringRef kSaveKey_looppoint = CFSTR("looppoint");
static CFStringRef kSaveKey_samplerate = CFSTR("samplerate");
static CFStringRef kSaveKey_basekey = CFSTR("key");
static CFStringRef kSaveKey_lowkey = CFSTR("lowkey");
static CFStringRef kSaveKey_highkey = CFSTR("highkey");
static CFStringRef kSaveKey_ar = CFSTR("ar");
static CFStringRef kSaveKey_dr = CFSTR("dr");
static CFStringRef kSaveKey_sl = CFSTR("sl");
static CFStringRef kSaveKey_sr = CFSTR("sr");
static CFStringRef kSaveKey_volL = CFSTR("volL");
static CFStringRef kSaveKey_volR = CFSTR("volR");
static CFStringRef kSaveKey_echo = CFSTR("echo");

int GetARTicks( int ar, Float64 tempo );
int GetDRTicks( int dr, Float64 tempo );
int GetSRTicks( int sr, Float64 tempo );

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

COMPONENT_ENTRY(Chip700)


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	Chip700::Chip700
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Chip700::Chip700(AudioUnit component)
: AUMonotimbralInstrumentBase(component, 0, 1)
{
	CreateElements();
	Globals()->UseIndexedParameters(kNumberOfParameters);
	
	Globals()->SetParameter(kParam_poly, kDefaultValue_poly);
	Globals()->SetParameter(kParam_mainvol_L, kDefaultValue_mainvol_L );
	Globals()->SetParameter(kParam_mainvol_R, kDefaultValue_mainvol_R );
	Globals()->SetParameter(kParam_vibdepth, kDefaultValue_vibdepth);
	Globals()->SetParameter(kParam_vibrate, kDefaultValue_vibrate);
	Globals()->SetParameter(kParam_vibdepth2, kDefaultValue_vibdepth2);
	Globals()->SetParameter(kParam_velocity, kDefaultValue_velocity);
	Globals()->SetParameter(kParam_clipnoise, kDefaultValue_clipnoise);
	Globals()->SetParameter(kParam_bendrange, kDefaultValue_bendrange);
	Globals()->SetParameter(kParam_program, kDefaultValue_program);
	Globals()->SetParameter(kParam_drummode, kDefaultValue_drummode);
	for ( int i=0; i<15; i++ ) {
		Globals()->SetParameter(kParam_program_2+i, kDefaultValue_program);
		Globals()->SetParameter(kParam_vibdepth_2+i, kDefaultValue_vibdepth);
	}
	
	//エコー
	Globals()->SetParameter(kParam_echovol_L, kDefaultValue_echovol_L );
	Globals()->SetParameter(kParam_echovol_R, kDefaultValue_echovol_R );
	Globals()->SetParameter(kParam_echoFB, kDefaultValue_echoFB );
	Globals()->SetParameter(kParam_echodelay, kDefaultValue_echodelay );
	Globals()->SetParameter(kParam_fir0, kDefaultValue_fir0 );
	Globals()->SetParameter(kParam_fir1, kDefaultValue_fir1 );
	Globals()->SetParameter(kParam_fir2, kDefaultValue_fir2 );
	Globals()->SetParameter(kParam_fir3, kDefaultValue_fir3 );
	Globals()->SetParameter(kParam_fir4, kDefaultValue_fir4 );
	Globals()->SetParameter(kParam_fir5, kDefaultValue_fir5 );
	Globals()->SetParameter(kParam_fir6, kDefaultValue_fir6 );
	Globals()->SetParameter(kParam_fir7, kDefaultValue_fir7 );
	for (int i=0; i<5; i++) {
		mFilterBand[i] = 1.0f;
	}
	
	mEditProg = 0;
	//プログラムの初期化
	for (int i=0; i<128; i++) {
		mVPset[i].pgname = NULL;
		mVPset[i].brr.size=0;
		mVPset[i].brr.data=NULL;
		mVPset[i].basekey=0;
		mVPset[i].lowkey=0;
		mVPset[i].highkey=0;
		mVPset[i].loop=false;
		mVPset[i].echo=false;
		mVPset[i].lp=0;
		mVPset[i].rate=0;
		mVPset[i].volL=100;
		mVPset[i].volR=100;
		
		mVPset[i].ar=kDefaultValue_AR;
		mVPset[i].dr=kDefaultValue_DR;
		mVPset[i].sl=kDefaultValue_SL;
		mVPset[i].sr=kDefaultValue_SR;
		
		mKeyMap[i] = 0;
	}
#if AU_DEBUG_DISPATCHER
	mDebugDispatcher = new AUDebugDispatcher(this);
#endif
}

ComponentResult Chip700::Initialize()
{	
	AUMonotimbralInstrumentBase::Initialize();
	SetNotes(kMaximumVoices, Globals()->GetParameter(kParam_poly), mChip700Notes, sizeof(Chip700Note));

	return noErr;
}

void Chip700::Cleanup()
{
	for (int i=0; i<128; i++) {
		if (mVPset[i].pgname)
			CFRelease(mVPset[i].pgname);
		mVPset[i].pgname = NULL;
	}
}

ComponentResult Chip700::StartNote(MusicDeviceInstrumentID		inInstrument, 
						  MusicDeviceGroupID 			inGroupID, 
						  NoteInstanceID 				&outNoteInstanceID, 
						  UInt32 						inOffsetSampleFrame, 
						  const MusicDeviceNoteParams &inParams)
{
	ChangeMaxActiveNotes(Globals()->GetParameter(kParam_poly));
	return AUMonotimbralInstrumentBase::StartNote(inInstrument,inGroupID,outNoteInstanceID,inOffsetSampleFrame,inParams);
}

ComponentResult	Chip700::Render(   AudioUnitRenderActionFlags &	ioActionFlags,
												const AudioTimeStamp &			inTimeStamp,
												UInt32							inNumberFrames)
{
	CallHostBeatAndTempo(NULL, &mTempo);
	return AUMonotimbralInstrumentBase::Render(ioActionFlags, inTimeStamp, inNumberFrames);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	Chip700::GetParameterInfo
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ComponentResult		Chip700::GetParameterInfo(AudioUnitScope		inScope,
											  AudioUnitParameterID	inParameterID,
											  AudioUnitParameterInfo	&outParameterInfo )
{
	ComponentResult result = noErr;
	
	outParameterInfo.flags = 	kAudioUnitParameterFlag_IsWritable
		|		kAudioUnitParameterFlag_IsReadable;
    
    if (inScope == kAudioUnitScope_Global) {
        switch(inParameterID)
        {
			case kParam_poly:
                AUBase::FillInParameterName (outParameterInfo, kParam_poly_Name, false);
                outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
                outParameterInfo.minValue = kMinimumValue_1;
                outParameterInfo.maxValue = kMaximumVoices;
                outParameterInfo.defaultValue = kDefaultValue_poly;
				break;
			case kParam_vibdepth:
			case kParam_vibdepth_2:
			case kParam_vibdepth_3:
			case kParam_vibdepth_4:
			case kParam_vibdepth_5:
			case kParam_vibdepth_6:
			case kParam_vibdepth_7:
			case kParam_vibdepth_8:
			case kParam_vibdepth_9:
			case kParam_vibdepth_10:
			case kParam_vibdepth_11:
			case kParam_vibdepth_12:
			case kParam_vibdepth_13:
			case kParam_vibdepth_14:
			case kParam_vibdepth_15:
			case kParam_vibdepth_16:
				AUBase::FillInParameterName (outParameterInfo, kParam_vibdepth_Name, false);
				outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
				outParameterInfo.minValue = kMinimumValue_0;
				outParameterInfo.maxValue = kMaximumValue_127;
				outParameterInfo.defaultValue = kDefaultValue_vibdepth;
				break;
			case kParam_vibrate:
				AUBase::FillInParameterName (outParameterInfo, kParam_vibrate_Name, false);
				outParameterInfo.unit = kAudioUnitParameterUnit_Hertz;
				outParameterInfo.minValue = kMinimumValue_vibrate;
				outParameterInfo.maxValue = kMaximumValue_vibrate;
				outParameterInfo.defaultValue = kDefaultValue_vibrate;
				break;
			case kParam_vibdepth2:
				AUBase::FillInParameterName (outParameterInfo, kParam_vibdepth2_Name, false);
				outParameterInfo.unit = kAudioUnitParameterUnit_Generic;
				outParameterInfo.minValue = kMinimumValue_1;
				outParameterInfo.maxValue = kMaximumValue_15;
				outParameterInfo.defaultValue = kDefaultValue_vibdepth2;
				break;
			case kParam_velocity:
				AUBase::FillInParameterName (outParameterInfo, kParam_velocity_Name, false);
				outParameterInfo.unit = kAudioUnitParameterUnit_Boolean;
				outParameterInfo.minValue = kMinimumValue_0;
				outParameterInfo.maxValue = 1;
				outParameterInfo.defaultValue = kDefaultValue_velocity;
				break;
			case kParam_clipnoise:
				AUBase::FillInParameterName (outParameterInfo, kParam_clipnoise_Name, false);
				outParameterInfo.unit = kAudioUnitParameterUnit_Boolean;
				outParameterInfo.minValue = kMinimumValue_0;
				outParameterInfo.maxValue = 1;
				outParameterInfo.defaultValue = kDefaultValue_clipnoise;
				break;
			case kParam_bendrange:
				AUBase::FillInParameterName (outParameterInfo, kParam_bendrange_Name, false);
				outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
				outParameterInfo.minValue = 1;
				outParameterInfo.maxValue = 24;
				outParameterInfo.defaultValue = kDefaultValue_bendrange;
				break;
			case kParam_program:
			case kParam_program_2:
			case kParam_program_3:
			case kParam_program_4:
			case kParam_program_5:
			case kParam_program_6:
			case kParam_program_7:
			case kParam_program_8:
			case kParam_program_9:
			case kParam_program_10:
			case kParam_program_11:
			case kParam_program_12:
			case kParam_program_13:
			case kParam_program_14:
			case kParam_program_15:
			case kParam_program_16:
				AUBase::FillInParameterName (outParameterInfo, kParam_program_Name, false);
				outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
				outParameterInfo.minValue = kMinimumValue_0;
				outParameterInfo.maxValue = kMaximumValue_127;
				outParameterInfo.defaultValue = kDefaultValue_program;
				break;
			case kParam_drummode:
				AUBase::FillInParameterName (outParameterInfo, kParam_drummode_Name, false);
				outParameterInfo.unit = kAudioUnitParameterUnit_Boolean;
				outParameterInfo.minValue = kMinimumValue_0;
				outParameterInfo.maxValue = 1;
				outParameterInfo.defaultValue = kDefaultValue_drummode;
				break;
			
			//エコー
			case kParam_mainvol_L:
                AUBase::FillInParameterName (outParameterInfo, kParameter_mainvol_L_Name, false);
                outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
                outParameterInfo.minValue = kMinimumValue_n128;
                outParameterInfo.maxValue = kMaximumValue_127;
                outParameterInfo.defaultValue = kDefaultValue_mainvol_L;
                break;
            case kParam_mainvol_R:
                AUBase::FillInParameterName (outParameterInfo, kParameter_mainvol_R_Name, false);
                outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
                outParameterInfo.minValue = kMinimumValue_n128;
                outParameterInfo.maxValue = kMaximumValue_127;
                outParameterInfo.defaultValue = kDefaultValue_mainvol_R;
                break;
            case kParam_echovol_L:
                AUBase::FillInParameterName (outParameterInfo, kParameter_echovol_L_Name, false);
                outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
                outParameterInfo.minValue = kMinimumValue_n128;
                outParameterInfo.maxValue = kMaximumValue_127;
                outParameterInfo.defaultValue = kDefaultValue_echovol_L;
                break;
            case kParam_echovol_R:
                AUBase::FillInParameterName (outParameterInfo, kParameter_echovol_R_Name, false);
                outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
                outParameterInfo.minValue = kMinimumValue_n128;
                outParameterInfo.maxValue = kMaximumValue_127;
                outParameterInfo.defaultValue = kDefaultValue_echovol_R;
                break;
            case kParam_echoFB:
                AUBase::FillInParameterName (outParameterInfo, kParameter_echoFB_Name, false);
                outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
                outParameterInfo.minValue = kMinimumValue_n128;
                outParameterInfo.maxValue = kMaximumValue_127;
                outParameterInfo.defaultValue = kDefaultValue_echoFB;
                break;
			case kParam_echodelay:
                AUBase::FillInParameterName (outParameterInfo, kParameter_echodelay_Name, false);
                outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
                outParameterInfo.minValue = kMinimumValue_0;
                outParameterInfo.maxValue = kMaximumValue_15;
                outParameterInfo.defaultValue = kDefaultValue_echodelay;
                break;
            case kParam_fir0:
                AUBase::FillInParameterName (outParameterInfo, kParameter_firC0_Name, false);
                outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
                outParameterInfo.minValue = kMinimumValue_n128;
                outParameterInfo.maxValue = kMaximumValue_127;
                outParameterInfo.defaultValue = kDefaultValue_fir0;
                break;
            case kParam_fir1:
                AUBase::FillInParameterName (outParameterInfo, kParameter_firC1_Name, false);
                outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
                outParameterInfo.minValue = kMinimumValue_n128;
                outParameterInfo.maxValue = kMaximumValue_127;
                outParameterInfo.defaultValue = kDefaultValue_fir1;
                break;
            case kParam_fir2:
                AUBase::FillInParameterName (outParameterInfo, kParameter_firC2_Name, false);
                outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
                outParameterInfo.minValue = kMinimumValue_n128;
                outParameterInfo.maxValue = kMaximumValue_127;
                outParameterInfo.defaultValue = kDefaultValue_fir2;
                break;
            case kParam_fir3:
                AUBase::FillInParameterName (outParameterInfo, kParameter_firC3_Name, false);
                outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
                outParameterInfo.minValue = kMinimumValue_n128;
                outParameterInfo.maxValue = kMaximumValue_127;
                outParameterInfo.defaultValue = kDefaultValue_fir3;
                break;
            case kParam_fir4:
                AUBase::FillInParameterName (outParameterInfo, kParameter_firC4_Name, false);
                outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
                outParameterInfo.minValue = kMinimumValue_n128;
                outParameterInfo.maxValue = kMaximumValue_127;
                outParameterInfo.defaultValue = kDefaultValue_fir4;
                break;
            case kParam_fir5:
                AUBase::FillInParameterName (outParameterInfo, kParameter_firC5_Name, false);
                outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
                outParameterInfo.minValue = kMinimumValue_n128;
                outParameterInfo.maxValue = kMaximumValue_127;
                outParameterInfo.defaultValue = kDefaultValue_fir5;
                break;
            case kParam_fir6:
                AUBase::FillInParameterName (outParameterInfo, kParameter_firC6_Name, false);
                outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
                outParameterInfo.minValue = kMinimumValue_n128;
                outParameterInfo.maxValue = kMaximumValue_127;
                outParameterInfo.defaultValue = kDefaultValue_fir6;
                break;
            case kParam_fir7:
                AUBase::FillInParameterName (outParameterInfo, kParameter_firC7_Name, false);
                outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
                outParameterInfo.minValue = kMinimumValue_n128;
                outParameterInfo.maxValue = kMaximumValue_127;
                outParameterInfo.defaultValue = kDefaultValue_fir7;
                break;
				
				
			default:
				result = kAudioUnitErr_InvalidParameter;
				break;
            }
	} else {
        result = kAudioUnitErr_InvalidParameter;
    }



return result;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	Chip700::GetPropertyInfo
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ComponentResult		Chip700::GetPropertyInfo (AudioUnitPropertyID	inID,
											  AudioUnitScope		inScope,
											  AudioUnitElement	inElement,
											  UInt32 &		outDataSize,
											  Boolean &		outWritable)
{
	if (inScope == kAudioUnitScope_Global) {
		switch (inID) {
			case kAudioUnitCustomProperty_BaseKey:
			case kAudioUnitCustomProperty_LowKey:
			case kAudioUnitCustomProperty_HighKey:
			case kAudioUnitCustomProperty_AR:
			case kAudioUnitCustomProperty_DR:
			case kAudioUnitCustomProperty_SL:
			case kAudioUnitCustomProperty_SR:
			case kAudioUnitCustomProperty_VolL:
			case kAudioUnitCustomProperty_VolR:
			case kAudioUnitCustomProperty_EditingProgram:
			case kAudioUnitCustomProperty_LoopPoint:
				outDataSize = sizeof(int);
				outWritable = false;
				return noErr;
				
			case kAudioUnitCustomProperty_Rate:
				outDataSize = sizeof(double);
				outWritable = false;
				return noErr;
			
			case kAudioUnitCustomProperty_Loop:
				outDataSize = sizeof(bool);
				outWritable = false;
				return noErr;
			
			case kAudioUnitCustomProperty_Echo:
				outDataSize = sizeof(bool);
				outWritable = false;
				return noErr;

			case kAudioUnitCustomProperty_BRRData:
				outDataSize = sizeof(BRRData);
				outWritable = false;
				return noErr;
				
			case kAudioUnitCustomProperty_PGDictionary:
				outDataSize = sizeof(CFDictionaryRef);
				outWritable = false;
				return noErr;
				
			case kAudioUnitCustomProperty_XIData:
				outDataSize = sizeof(CFDataRef);
				outWritable = false;
				return noErr;
			
			case kAudioUnitCustomProperty_ProgramName:
				outDataSize = sizeof(CFStringRef);
				outWritable = false;
				return noErr;
				
			//エコー
			case kAudioUnitCustomProperty_Band1:
			case kAudioUnitCustomProperty_Band2:
			case kAudioUnitCustomProperty_Band3:
			case kAudioUnitCustomProperty_Band4:
			case kAudioUnitCustomProperty_Band5:
				outWritable = false;
				outDataSize = sizeof(Float32);
				return noErr;
		}
	}
	
	return AUMonotimbralInstrumentBase::GetPropertyInfo(inID, inScope, inElement, outDataSize, outWritable);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	Chip700::GetProperty
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ComponentResult		Chip700::GetProperty(	AudioUnitPropertyID inID,
											AudioUnitScope 		inScope,
											AudioUnitElement 	inElement,
											void *			outData )
{
	if (inScope == kAudioUnitScope_Global) {
		switch (inID) {
			case kAudioUnitCustomProperty_BRRData:
				*((BRRData *)outData) = mVPset[mEditProg].brr;
				return noErr;
			
			case kAudioUnitCustomProperty_Rate:
				*((double *)outData) = mVPset[mEditProg].rate;
				return noErr;
			
			case kAudioUnitCustomProperty_BaseKey:
				*((int *)outData) = mVPset[mEditProg].basekey;
				return noErr;
			case kAudioUnitCustomProperty_LowKey:
				*((int *)outData) = mVPset[mEditProg].lowkey;
				return noErr;
			case kAudioUnitCustomProperty_HighKey:
				*((int *)outData) = mVPset[mEditProg].highkey;
				return noErr;
			
			case kAudioUnitCustomProperty_LoopPoint:
				*((int *)outData) = mVPset[mEditProg].lp;
				return noErr;
				
			case kAudioUnitCustomProperty_Loop:
				*((bool *)outData) = mVPset[mEditProg].loop;
				return noErr;
				
			case kAudioUnitCustomProperty_Echo:
				*((bool *)outData) = mVPset[mEditProg].echo;
				return noErr;

			case kAudioUnitCustomProperty_AR:
				*((int *)outData) = mVPset[mEditProg].ar;
				return noErr;
				
			case kAudioUnitCustomProperty_DR:
				*((int *)outData) = mVPset[mEditProg].dr;
				return noErr;
				
			case kAudioUnitCustomProperty_SL:
				*((int *)outData) = mVPset[mEditProg].sl;
				return noErr;
				
			case kAudioUnitCustomProperty_SR:
				*((int *)outData) = mVPset[mEditProg].sr;
				return noErr;
				
			case kAudioUnitCustomProperty_VolL:
				*((int *)outData) = mVPset[mEditProg].volL;
				return noErr;
				
			case kAudioUnitCustomProperty_VolR:
				*((int *)outData) = mVPset[mEditProg].volR;
				return noErr;
				
			case kAudioUnitCustomProperty_EditingProgram:
				*((int *)outData) = mEditProg;
				return noErr;
				
			case kAudioUnitCustomProperty_PGDictionary:
			{
				CFDictionaryRef	pgdata;
				CreatePGDataDic(&pgdata, mEditProg);
				*((CFDictionaryRef *)outData) = pgdata;	//使用後要release
				return noErr;
			}
			
			case kAudioUnitCustomProperty_XIData:
			{
				CFDataRef xidata;
				CreateXIData( &xidata );
				*((CFDataRef *)outData) = xidata;	//使用後要release
				return noErr;
			}
				
			case kAudioUnitCustomProperty_ProgramName:
				*((CFStringRef *)outData) = mVPset[mEditProg].pgname;
				return noErr;
				
			//エコー
			case kAudioUnitCustomProperty_Band1:
			case kAudioUnitCustomProperty_Band2:
			case kAudioUnitCustomProperty_Band3:
			case kAudioUnitCustomProperty_Band4:
			case kAudioUnitCustomProperty_Band5:
				*((Float32 *)outData) = mFilterBand[inID-kAudioUnitCustomProperty_Band1];
				return noErr;
		}
	}
	return AUMonotimbralInstrumentBase::GetProperty(inID, inScope, inElement, outData);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	Chip700::SetProperty
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ComponentResult		Chip700::SetProperty(	AudioUnitPropertyID inID,
											AudioUnitScope 		inScope,
											AudioUnitElement 	inElement,
											const void *		inData,
											UInt32              inDataSize)
{
	if (inScope == kAudioUnitScope_Global) {
		switch (inID) {
			
			case kAudioUnitCustomProperty_BRRData:
			{
				BRRData *brr = (BRRData *)inData;
				//brrデータをこちら側に移動する
				if (brr->data) {
					mBRRdata[mEditProg].Allocate(brr->size);
					mVPset[mEditProg].brr.data = &mBRRdata[mEditProg][0];
					memmove(mVPset[mEditProg].brr.data,brr->data,brr->size);
					mVPset[mEditProg].brr.size = brr->size;
				}
				else {
					if (mVPset[mEditProg].brr.data) {
						mBRRdata[mEditProg].Deallocate();
						mVPset[mEditProg].brr.data = NULL;
						mVPset[mEditProg].brr.size = 0;
						if (mVPset[mEditProg].pgname)
							CFRelease(mVPset[mEditProg].pgname);
						mVPset[mEditProg].pgname = NULL;
						PropertyChanged(kAudioUnitCustomProperty_ProgramName, kAudioUnitScope_Global, 0);
					}
				}
				return noErr;
			}
			
			case kAudioUnitCustomProperty_Rate:
				mVPset[mEditProg].rate = *((double*)inData);
				return noErr;
				
			case kAudioUnitCustomProperty_BaseKey:
				mVPset[mEditProg].basekey = *((int*)inData);
				RefreshKeyMap();
				return noErr;
			case kAudioUnitCustomProperty_LowKey:
				mVPset[mEditProg].lowkey = *((int*)inData);
				RefreshKeyMap();
				return noErr;
			case kAudioUnitCustomProperty_HighKey:
				mVPset[mEditProg].highkey = *((int*)inData);
				RefreshKeyMap();
				return noErr;
				
			case kAudioUnitCustomProperty_LoopPoint:
				mVPset[mEditProg].lp = *((int*)inData);
				if (mVPset[mEditProg].lp > mVPset[mEditProg].brr.size)
					mVPset[mEditProg].lp = mVPset[mEditProg].brr.size;
				return noErr;
				
			case kAudioUnitCustomProperty_Loop:
				mVPset[mEditProg].loop = *((bool*)inData);
				return noErr;
				
			case kAudioUnitCustomProperty_Echo:
				mVPset[mEditProg].echo = *((bool*)inData);
				return noErr;
				
			case kAudioUnitCustomProperty_AR:
				mVPset[mEditProg].ar = *((int*)inData);
				return noErr;
				
			case kAudioUnitCustomProperty_DR:
				mVPset[mEditProg].dr = *((int*)inData);
				return noErr;
				
			case kAudioUnitCustomProperty_SL:
				mVPset[mEditProg].sl = *((int*)inData);
				return noErr;
				
			case kAudioUnitCustomProperty_SR:
				mVPset[mEditProg].sr = *((int*)inData);
				return noErr;
			
			case kAudioUnitCustomProperty_VolL:
				mVPset[mEditProg].volL = *((int *)inData);
				return noErr;
				
			case kAudioUnitCustomProperty_VolR:
				mVPset[mEditProg].volR = *((int *)inData);
				return noErr;
				
			case kAudioUnitCustomProperty_EditingProgram:
			{
				int	pg = *((int*)inData);
				if (pg>127) pg=127;
				if (pg<0) pg=0;
				mEditProg = pg;
				
				Globals()->SetParameter(kParam_program, mEditProg);
				for (int i=kAudioUnitCustomProperty_ProgramName; i<=kAudioUnitCustomProperty_Echo; i++) {
					PropertyChanged(i, kAudioUnitScope_Global, 0);
				}
				return noErr;
			}
				
			case kAudioUnitCustomProperty_PGDictionary:
			{
				CFDictionaryRef	pgdata = *((CFDictionaryRef*)inData);
				RestorePGDataDic(pgdata, mEditProg);
				return noErr;
			}
				
			case kAudioUnitCustomProperty_XIData:
			{
				return noErr;
			}
			
			case kAudioUnitCustomProperty_ProgramName:
				if (mVPset[mEditProg].pgname)
					CFRelease(mVPset[mEditProg].pgname);
				mVPset[mEditProg].pgname = CFStringCreateCopy(NULL,*((CFStringRef*)inData));
				return noErr;
				
			//エコー
			case kAudioUnitCustomProperty_Band1:
			case kAudioUnitCustomProperty_Band2:
			case kAudioUnitCustomProperty_Band3:
			case kAudioUnitCustomProperty_Band4:
			case kAudioUnitCustomProperty_Band5:
				SetBandParam( inID-kAudioUnitCustomProperty_Band1, *((Float32*)inData) );
				return noErr;
		}
	}
	return AUMonotimbralInstrumentBase::SetProperty(inID, inScope, inElement, inData, inDataSize);
}

void Chip700::SetBandParam( int band, Float32 value )
{
	float bandfactor[8][5] = {
		{0.0625,0.125,		0.12500,	0.25,		0.4375},
		{0.0625,0.11548,	0.08839,	0.04784,	-0.314209},
		{0.0625,0.08839,	0.00000,	-0.21339,	0.0625},
		{0.0625,0.04784,	-0.08839,	-0.11548,	0.093538},	
		{0.0625,0.00000,	-0.12500,	0.125,		-0.0625},
		{0.0625,-0.04784,	-0.08839,	0.11548,	-0.041761},
		{0.0625,-0.08839,	0.00000,	-0.03661,	0.0625},
		{0.0625,-0.11548,	0.08839,	-0.04784,	0.012432}
	};
	float	temp;
	AudioUnitEvent auEvent;
	
	mFilterBand[band] = value;
	
	for (int j=0; j<8; j++) {
		temp = 0;
		for (int i=0; i<5; i++) {
			temp += mFilterBand[i] * bandfactor[j][i];
		}
		if (temp < 0) {
			temp = ceil(temp*127);
		}
		else {
			temp = floor(temp*127);
		}
		Globals()->SetParameter(kParam_fir0+j, temp);
		auEvent.mEventType = kAudioUnitEvent_ParameterValueChange;
		auEvent.mArgument.mParameter.mAudioUnit = (AudioUnit)GetComponentInstance();
		auEvent.mArgument.mParameter.mScope = kAudioUnitScope_Global;
		auEvent.mArgument.mParameter.mParameterID = kParam_fir0+j;
		auEvent.mArgument.mParameter.mElement = 0;
		AUEventListenerNotify(NULL, NULL, &auEvent);
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	Chip700::GetPresets
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ComponentResult Chip700::GetPresets(CFArrayRef *outData) const
{	
	if (outData == NULL) return noErr;
	
	CFMutableArrayRef theArray = CFArrayCreateMutable(NULL, kNumberPresets, NULL);
	for (int i = 0; i < kNumberPresets; ++i) {
		CFArrayAppendValue(theArray, &kPresets[i]);
    }
    
	*outData = (CFArrayRef)theArray;
	return noErr;
}

OSStatus Chip700::NewFactoryPresetSet(const AUPreset &inNewFactoryPreset)
{
	SInt32 chosenPreset = inNewFactoryPreset.presetNumber;
	for(int i = 0; i < kNumberPresets; ++i)
	{
		if(chosenPreset == kPresets[i].presetNumber)
		{
			switch(chosenPreset) {
				case 0:
					for (int j=0; j<128; j++) {
						if (mVPset[j].brr.data) {
							mBRRdata[j].Deallocate();
							mVPset[j].brr.data = NULL;
							mVPset[j].brr.size = 0;
							if (mVPset[j].pgname)
								CFRelease(mVPset[j].pgname);
							mVPset[j].pgname = NULL;
						}
					}
					PropertyChanged(kAudioUnitCustomProperty_BRRData, kAudioUnitScope_Global, 0);
					PropertyChanged(kAudioUnitCustomProperty_ProgramName, kAudioUnitScope_Global, 0);
					break;
				case 1:
					//プリセット音色
					mVPset[0].brr.size=0x36;
					mBRRdata[0].Allocate(mVPset[0].brr.size);
					mVPset[0].brr.data=&mBRRdata[0][0];
					memmove(mVPset[0].brr.data,sinewave_brr,mVPset[0].brr.size);
					mVPset[0].basekey=81;
					mVPset[0].lowkey=0;
					mVPset[0].highkey=127;
					mVPset[0].loop=true;
					mVPset[0].echo=true;
					mVPset[0].lp=18;
					mVPset[0].rate=28160.0;
					mVPset[0].volL=100;
					mVPset[0].volR=100;
					if (mVPset[0].pgname)
						CFRelease(mVPset[0].pgname);
						mVPset[0].pgname = CFStringCreateCopy(NULL,CFSTR("Sine Wave"));
					
					mVPset[1].brr.size=0x2d;
					mBRRdata[1].Allocate(mVPset[1].brr.size);
					mVPset[1].brr.data=&mBRRdata[1][0];
					memmove(mVPset[1].brr.data,squarewave_brr,mVPset[1].brr.size);
					mVPset[1].basekey=69;
					mVPset[1].lowkey=0;
					mVPset[1].highkey=127;
					mVPset[1].loop=true;
					mVPset[1].echo=true;
					mVPset[1].lp=9;
					mVPset[1].rate=28160.0;
					mVPset[1].volL=100;
					mVPset[1].volR=100;
					if (mVPset[1].pgname)
						CFRelease(mVPset[1].pgname);
						mVPset[1].pgname = CFStringCreateCopy(NULL,CFSTR("Square Wave"));
					
					mVPset[2].brr.size=0x2d;
					mBRRdata[2].Allocate(mVPset[2].brr.size);
					mVPset[2].brr.data=&mBRRdata[2][0];
					memmove(mVPset[2].brr.data,pulse1_brr,mVPset[2].brr.size);
					mVPset[2].basekey=69;
					mVPset[2].lowkey=0;
					mVPset[2].highkey=127;
					mVPset[2].loop=true;
					mVPset[2].echo=true;
					mVPset[2].lp=9;
					mVPset[2].rate=28160.0;
					mVPset[2].volL=100;
					mVPset[2].volR=100;
					if (mVPset[2].pgname)
						CFRelease(mVPset[2].pgname);
						mVPset[2].pgname = CFStringCreateCopy(NULL,CFSTR("25% Pulse"));
					
					mVPset[3].brr.size=0x2d;
					mBRRdata[3].Allocate(mVPset[3].brr.size);
					mVPset[3].brr.data=&mBRRdata[3][0];
					memmove(mVPset[3].brr.data,pulse2_brr,mVPset[3].brr.size);
					mVPset[3].basekey=69;
					mVPset[3].lowkey=0;
					mVPset[3].highkey=127;
					mVPset[3].loop=true;
					mVPset[3].echo=true;
					mVPset[3].lp=9;
					mVPset[3].rate=28160.0;
					mVPset[3].volL=100;
					mVPset[3].volR=100;
					if (mVPset[3].pgname)
						CFRelease(mVPset[3].pgname);
						mVPset[3].pgname = CFStringCreateCopy(NULL,CFSTR("12.5% Pulse"));
					
					mVPset[4].brr.size=0x2d;
					mBRRdata[4].Allocate(mVPset[4].brr.size);
					mVPset[4].brr.data=&mBRRdata[4][0];
					memmove(mVPset[4].brr.data,pulse3_brr,mVPset[4].brr.size);
					mVPset[4].basekey=69;
					mVPset[4].lowkey=0;
					mVPset[4].highkey=127;
					mVPset[4].loop=true;
					mVPset[4].echo=true;
					mVPset[4].lp=9;
					mVPset[4].rate=28160.0;
					mVPset[4].volL=100;
					mVPset[4].volR=100;
					if (mVPset[4].pgname)
						CFRelease(mVPset[4].pgname);
						mVPset[4].pgname = CFStringCreateCopy(NULL,CFSTR("6.25% Pulse"));
					
					for (int i=kAudioUnitCustomProperty_ProgramName; i<=kAudioUnitCustomProperty_VolR; i++) {
						PropertyChanged(i, kAudioUnitScope_Global, 0);
					}
					break;
			}
			SetAFactoryPresetAsCurrent(kPresets[i]);
			
			RefreshKeyMap();
			
			return noErr;
		}
	}
	
	return kAudioUnitErr_InvalidPropertyValue;
}


static void AddNumToDictionary(CFMutableDictionaryRef dict, CFStringRef key, int value)
{
	CFNumberRef num = CFNumberCreate(NULL, kCFNumberIntType, &value);
	CFDictionarySetValue(dict, key, num);
	CFRelease(num);
}

static void AddBooleanToDictionary(CFMutableDictionaryRef dict, CFStringRef key, bool value)
{
	//CFBooleanRef num = CFNumberCreate(NULL, kCFNumberIntType, &value);
	if ( value ) {
		CFDictionarySetValue(dict, key, kCFBooleanTrue);
	}
	else {
		CFDictionarySetValue(dict, key, kCFBooleanFalse);
	}
	//CFRelease(num);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	Chip700::SaveState
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ComponentResult	Chip700::SaveState(CFPropertyListRef *outData)
{
	ComponentResult result;
	result = AUMonotimbralInstrumentBase::SaveState(outData);
	CFMutableDictionaryRef	dict=(CFMutableDictionaryRef)*outData;
	if (result == noErr) {
		CFDictionaryRef	pgdata;
		CFStringRef pgnum,pgname;
		
		for (int i=0; i<128; i++) {
			if (mVPset[i].brr.data) {
				CreatePGDataDic(&pgdata, i);
				pgnum = CFStringCreateWithFormat(NULL,NULL,CFSTR("pg%03d"),i);
				CFDictionarySetValue(dict, pgnum, pgdata);
				CFRelease(pgdata);
				CFRelease(pgnum);
			}
		}
		
		AddNumToDictionary(dict, kSaveKey_EditProg, mEditProg);
		
		pgname = CFStringCreateCopy(NULL,CFSTR("C700"));
		CFDictionarySetValue(dict, CFSTR(kAUPresetNameKey), pgname);
		CFRelease(pgname);
	}
	return result;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	Chip700::RestoreState
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ComponentResult	Chip700::RestoreState(CFPropertyListRef plist)
{
	ComponentResult result;
	result = AUMonotimbralInstrumentBase::RestoreState(plist);
	CFDictionaryRef dict = static_cast<CFDictionaryRef>(plist);
	if (result == noErr) {
		//波形情報の復元
		CFStringRef pgnum;
		CFDictionaryRef	pgdata;
		for (int i=0; i<128; i++) {
			pgnum = CFStringCreateWithFormat(NULL,NULL,CFSTR("pg%03d"),i);
			if (CFDictionaryContainsKey(dict, pgnum)) {
				pgdata = reinterpret_cast<CFDictionaryRef>(CFDictionaryGetValue(dict, pgnum));
				RestorePGDataDic(pgdata, i);
			}
			CFRelease(pgnum);
		}
		
		if (CFDictionaryContainsKey(dict, kSaveKey_EditProg)) {
			CFNumberRef cfnum = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(dict, kSaveKey_EditProg));
			CFNumberGetValue(cfnum, kCFNumberIntType, &mEditProg);
			
			Globals()->SetParameter(kParam_program, mEditProg);
			//変更の通知
			PropertyChanged(kAudioUnitCustomProperty_EditingProgram, kAudioUnitScope_Global, 0);
		}
	}
	return result;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Chip700::RefreshKeyMap(void)
{
	for (int i=0; i<128; i++) {
		mKeyMap[i]=0;

	}	for (int i=0; i<128; i++) {
		if (mVPset[i].brr.data) {
			for (int j=mVPset[i].lowkey; j<=mVPset[i].highkey; j++) {
				mKeyMap[j]=i;
			}
		}
	}
}

int Chip700::CreateXIData( CFDataRef *data )
{
	CFMutableDataRef mdata;
	mdata = CFDataCreateMutable( NULL, 0 );
	
	int start_prg = 0;
	int end_prg = 127;
	bool multisample = Globals()->GetParameter(kParam_drummode) != 0 ? true:false;
	
	if ( !multisample ) {
		start_prg = end_prg = mEditProg;
	}
	
	XIFILEHEADER xfh;
	XIINSTRUMENTHEADER xih;
	XISAMPLEHEADER xsh;
	int nsamples = 0;
	
	// XI File Header
	memset(&xfh, 0, sizeof(xfh));
	memcpy(xfh.extxi, "Extended Instrument: ", 21);
	if ( multisample ) {
		memcpy(xfh.name, "Multi sampled Inst", 22);
	}
	else {
		if ( CFStringGetCString(mVPset[mEditProg].pgname,xsh.name,22,kCFStringEncodingASCII) == false ) {
			memcpy(xsh.name, "Inst", 22);
		}
	}
	xfh.name[22] = 0x1A;
	memcpy(xfh.trkname, "FastTracker v2.00   ", 20);
	xfh.shsize = EndianU16_NtoL( 0x0102 );
	CFDataAppendBytes( mdata, (const UInt8 *)&xfh, sizeof(xfh) );
	
	// XI Instrument Header
	memset(&xih, 0, sizeof(xih));
	
	int vol = 64;
	
	if ( multisample ) {
		for ( int i=0; i<96; i++ ) {
			xih.snum[i] = mKeyMap[i+12];
			if ( (mKeyMap[i+12]+1) > nsamples ) {
				nsamples = mKeyMap[i+12]+1;
				end_prg = mKeyMap[i+12];
			}
		}
	}
	else {
		for ( int i=0; i<96; i++ ) {
			xih.snum[i] = 0;
		}
		nsamples = 1;
		vol = (int)( abs(mVPset[mEditProg].volL) + abs(mVPset[mEditProg].volR) ) / 4;
	}

	//エンベロープの近似
	Float64 tempo = mTempo;

	if ( multisample ) {
		//サンプル毎には設定出来ないので非対応
		for (int i=0; i<4; i++) {
			xih.venv[i*2] = EndianU16_NtoL( i*10 );
			xih.venv[i*2+1] = EndianU16_NtoL( 64 );
		}
		xih.venv[7] = 0;
	}
	else {
		xih.venv[0] = 0;
		if ( mVPset[mEditProg].ar == 15 ) {
			xih.venv[1] = EndianU16_NtoL( vol );
		}
		else {
			xih.venv[1] = 0;
		}
		xih.venv[2] = EndianU16_NtoL( GetARTicks( mVPset[mEditProg].ar, tempo ) );	//tick数値はテンポ値に依存する
		xih.venv[3] = EndianU16_NtoL( vol );
		xih.venv[4] = EndianU16_NtoL( xih.venv[2] + GetDRTicks( mVPset[mEditProg].dr, tempo ) );
		xih.venv[5] = EndianU16_NtoL( vol * (mVPset[mEditProg].sl + 1) / 8 );
		if (mVPset[mEditProg].sr == 0) {
			xih.venv[6] = EndianU16_NtoL( xih.venv[4]+1 );
			xih.venv[7] = EndianU16_NtoL( xih.venv[5] );
		}
		else {
			xih.venv[6] = EndianU16_NtoL( xih.venv[4] + GetSRTicks( mVPset[mEditProg].sr, tempo ) );
			xih.venv[7] = EndianU16_NtoL( vol / 10 );
		}
	}
	
	xih.vnum = 4;
	xih.pnum = 0;
	xih.vsustain = 3;
	xih.vloops = 3;
	xih.vloope = 3;
	xih.psustain = 0;
	xih.ploops = 0;
	xih.ploope = 0;
	xih.vtype = 3;	//ENV_VOLUME + ENV_VOLSUSTAIN
	xih.ptype = 0;
	xih.volfade = EndianU16_NtoL( 5000 );
	xih.reserved2 = EndianU16_NtoL( nsamples );
	CFDataAppendBytes( mdata, (const UInt8 *)&xih, sizeof(xih) );
	
	// XI Sample Headers
	for (int ismp=start_prg; ismp<=end_prg; ismp++) {
		xsh.samplen = EndianU32_NtoL( mVPset[ismp].brr.size/9*32 );
		xsh.loopstart = EndianU32_NtoL( mVPset[ismp].lp/9*32 );
		xsh.looplen = EndianU32_NtoL( xsh.samplen - xsh.loopstart );
		double avr = ( abs(mVPset[ismp].volL) + abs(mVPset[ismp].volR) ) / 2;
		double pan = (abs(mVPset[ismp].volR) * 128) / avr;
		if ( pan > 256 ) pan = 256;
		xsh.vol = 64;	//変化しない？
		xsh.pan = pan;
		xsh.type = 0x10;	//CHN_16BIT
		if ( mVPset[ismp].loop ) {
			xsh.type |= 0x01;	//Normal Loop
		}
		double trans = 12*(log(mVPset[ismp].rate / 8363.0)/log(2.0));
		int trans_i = (trans + (60-mVPset[ismp].basekey) ) * 128 + 0.5;
		xsh.relnote = trans_i >> 7;
		xsh.finetune = trans_i & 0x7f;
		xsh.res = 0;
		if ( mVPset[ismp].pgname == NULL || CFStringGetCString(mVPset[ismp].pgname,xsh.name,22,kCFStringEncodingASCII) == false ) {
			memcpy(xsh.name, "Sample", 22);
		}
		CFDataAppendBytes( mdata, (const UInt8 *)&xsh, sizeof(xsh) );
	}
	
	// XI Sample Data
	for (int ismp=start_prg; ismp<=end_prg; ismp++) {
		short	*wavedata;
		long	numSamples = mVPset[ismp].brr.size/9*16;
		wavedata = new short[numSamples];
		brrdecode(mVPset[ismp].brr.data, wavedata,0,0);
		short s_new,s_old;
		s_old = wavedata[0];
		for ( int i=0; i<numSamples; i++ ) {
			s_new = wavedata[i];
			wavedata[i] = s_new - s_old;
			wavedata[i] = EndianS16_NtoL( wavedata[i] );
			s_old = s_new;
		}
		CFDataAppendBytes( mdata, (const UInt8 *)wavedata, numSamples*2 );
		delete [] wavedata;
	}
	*data = mdata;
	
	return 0;
}

int GetARTicks( int ar, Float64 tempo )
{
	Float64	basetime[16] = {
		4.1,
		2.6,
		1.5,
		1.0,
		0.640,
		0.380,
		0.260,
		0.160,
		0.096,
		0.064,
		0.040,
		0.024,
		0.016,
		0.010,
		0.006,
		0
	};
	//四分音符＝25tick
	Float64 tick_per_sec = tempo/60.0 * 25;
	return (int)(basetime[ar] * tick_per_sec);
}

int GetDRTicks( int dr, Float64 tempo )
{
	Float64	basetime[8] = {
		1.2,
		0.740,
		0.440,
		0.290,
		0.180,
		0.110,
		0.074,
		0.037
	};
	//四分音符＝25tick
	Float64 tick_per_sec = tempo/60.0 * 25;
	//tick_per_sec /= 2;	//実際は半分くらいなような気がする？？
	return (int)(basetime[dr] * tick_per_sec);
}

int GetSRTicks( int sr, Float64 tempo )
{
	Float64	basetime[32] = {
		0,
		38,
		28,
		24,
		19,
		14,
		12,
		9.4,
		7.1,
		5.9,
		4.7,
		3.5,
		2.9,
		2.4,
		1.8,
		1.5,
		1.2,
		0.880,
		0.740,
		0.590,
		0.440,
		0.370,
		0.290,
		0.220,
		0.180,
		0.150,
		0.110,
		0.092,
		0.074,
		0.055,
		0.037,
		0.028
	};
	//四分音符＝25tick
	Float64 tick_per_sec = tempo/60.0 * 25;
	return (int)(basetime[sr] * tick_per_sec);
}

int Chip700::CreatePGDataDic(CFDictionaryRef *data, int pgnum)
{
	CFMutableDictionaryRef dict = CFDictionaryCreateMutable	(NULL, 0, 
								&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	
	if (mVPset[pgnum].loop) {
		mBRRdata[pgnum][mVPset[pgnum].brr.size - 9] |= 2;
	}
	else {
		mBRRdata[pgnum][mVPset[pgnum].brr.size - 9] &= ~2;
	}
	CFDataRef	brrdata = CFDataCreate(NULL, mVPset[pgnum].brr.data, mVPset[pgnum].brr.size);
	CFDictionarySetValue(dict, kSaveKey_brrdata, brrdata);
	CFRelease(brrdata);
	
	AddNumToDictionary(dict, kSaveKey_looppoint, mVPset[pgnum].lp);
	CFNumberRef	num = CFNumberCreate(NULL, kCFNumberDoubleType, &mVPset[pgnum].rate);
	CFDictionarySetValue(dict, kSaveKey_samplerate, num);
	CFRelease(num);
	AddNumToDictionary(dict, kSaveKey_basekey, mVPset[pgnum].basekey);
	AddNumToDictionary(dict, kSaveKey_lowkey, mVPset[pgnum].lowkey);
	AddNumToDictionary(dict, kSaveKey_highkey, mVPset[pgnum].highkey);
	AddNumToDictionary(dict, kSaveKey_ar, mVPset[pgnum].ar);
	AddNumToDictionary(dict, kSaveKey_dr, mVPset[pgnum].dr);
	AddNumToDictionary(dict, kSaveKey_sl, mVPset[pgnum].sl);
	AddNumToDictionary(dict, kSaveKey_sr, mVPset[pgnum].sr);
	AddNumToDictionary(dict, kSaveKey_volL, mVPset[pgnum].volL);
	AddNumToDictionary(dict, kSaveKey_volR, mVPset[pgnum].volR);
	AddBooleanToDictionary(dict, kSaveKey_echo, mVPset[pgnum].echo);
	
	if (mVPset[pgnum].pgname)
		CFDictionarySetValue(dict, kSaveKey_ProgName, mVPset[pgnum].pgname);
	
	*data = dict;
	return 0;
}

void Chip700::RestorePGDataDic(CFPropertyListRef data, int pgnum)
{
	CFDictionaryRef dict = static_cast<CFDictionaryRef>(data);
	CFNumberRef cfnum;
	
	CFDataRef cfdata = reinterpret_cast<CFDataRef>(CFDictionaryGetValue(dict, kSaveKey_brrdata));
	int	size = CFDataGetLength(cfdata);
	const UInt8	*dataptr = CFDataGetBytePtr(cfdata);
	mBRRdata[pgnum].Allocate(size);
	mVPset[pgnum].brr.data = &mBRRdata[pgnum][0];
	memmove(mVPset[pgnum].brr.data,dataptr,size);
	mVPset[pgnum].brr.size = size;
	mVPset[pgnum].loop = mBRRdata[pgnum][mVPset[pgnum].brr.size-9]&2?true:false;
	
	int	value;
	if (CFDictionaryContainsKey(dict, kSaveKey_looppoint)) {
		cfnum = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(dict, kSaveKey_looppoint));
		CFNumberGetValue(cfnum, kCFNumberIntType, &value);
		mVPset[pgnum].lp = value;
	}
	
	double	dvalue;
	if (CFDictionaryContainsKey(dict, kSaveKey_samplerate)) {
		cfnum = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(dict, kSaveKey_samplerate));
		CFNumberGetValue(cfnum, kCFNumberDoubleType, &dvalue);
		mVPset[pgnum].rate = dvalue;
	}
	
	if (CFDictionaryContainsKey(dict, kSaveKey_basekey)) {
		cfnum = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(dict, kSaveKey_basekey));
		CFNumberGetValue(cfnum, kCFNumberIntType, &value);
		mVPset[pgnum].basekey = value;
	}
	else mVPset[pgnum].basekey = 60;
	if (CFDictionaryContainsKey(dict, kSaveKey_lowkey)) {
		cfnum = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(dict, kSaveKey_lowkey));
		CFNumberGetValue(cfnum, kCFNumberIntType, &value);
		mVPset[pgnum].lowkey = value;
	}
	else mVPset[pgnum].lowkey = 0;
	if (CFDictionaryContainsKey(dict, kSaveKey_highkey)) {
		cfnum = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(dict, kSaveKey_highkey));
		CFNumberGetValue(cfnum, kCFNumberIntType, &value);
		mVPset[pgnum].highkey = value;
	}
	else mVPset[pgnum].highkey = 127;
	RefreshKeyMap();
	
	if (CFDictionaryContainsKey(dict, kSaveKey_ar)) {
		cfnum = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(dict, kSaveKey_ar));
		CFNumberGetValue(cfnum, kCFNumberIntType, &value);
		mVPset[pgnum].ar = value;
	}
	
	if (CFDictionaryContainsKey(dict, kSaveKey_dr)) {
		cfnum = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(dict, kSaveKey_dr));
		CFNumberGetValue(cfnum, kCFNumberIntType, &value);
		mVPset[pgnum].dr = value;
	}
	
	if (CFDictionaryContainsKey(dict, kSaveKey_sl)) {
		cfnum = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(dict, kSaveKey_sl));
		CFNumberGetValue(cfnum, kCFNumberIntType, &value);
		mVPset[pgnum].sl = value;
	}
	
	if (CFDictionaryContainsKey(dict, kSaveKey_sr)) {
		cfnum = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(dict, kSaveKey_sr));
		CFNumberGetValue(cfnum, kCFNumberIntType, &value);
		mVPset[pgnum].sr = value;
	}
	
	if (CFDictionaryContainsKey(dict, kSaveKey_volL)) {
		cfnum = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(dict, kSaveKey_volL));
		CFNumberGetValue(cfnum, kCFNumberIntType, &value);
		mVPset[pgnum].volL = value;
	}
	else mVPset[pgnum].volL = 100;
	
	if (CFDictionaryContainsKey(dict, kSaveKey_volR)) {
		cfnum = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(dict, kSaveKey_volR));
		CFNumberGetValue(cfnum, kCFNumberIntType, &value);
		mVPset[pgnum].volR = value;
	}
	else mVPset[pgnum].volR = 100;
	
	if (CFDictionaryContainsKey(dict, kSaveKey_echo)) {
		CFBooleanRef cfbool = reinterpret_cast<CFBooleanRef>(CFDictionaryGetValue(dict, kSaveKey_echo));
		mVPset[pgnum].echo = CFBooleanGetValue(cfbool) ? true:false;
	}
	else mVPset[pgnum].echo = false;
	
	if (mVPset[pgnum].pgname)
		CFRelease(mVPset[pgnum].pgname);
	if (CFDictionaryContainsKey(dict, kSaveKey_ProgName)) {
		mVPset[pgnum].pgname = CFStringCreateCopy(NULL,reinterpret_cast<CFStringRef>(CFDictionaryGetValue(dict, kSaveKey_ProgName)));
	}
	else {
		mVPset[pgnum].pgname = NULL;
	}
	
	if (pgnum == mEditProg) {
		for (int i=kAudioUnitCustomProperty_ProgramName; i<=kAudioUnitCustomProperty_VolR; i++) {
			PropertyChanged(i, kAudioUnitScope_Global, 0);
		}
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	Chip700::HandleControlChange
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Chip700::HandleControlChange(	int 	inChannel,
									UInt8 	inController,
									UInt8 	inValue,
									long	inStartFrame)
{
	//モジュレーションホイール
	if (inController == kMidiController_ModWheel) {
		AudioUnitParameterID	paramID = kParam_vibdepth;
		if ( inChannel > 0 ) {
			paramID = kParam_vibdepth_2 - 1 + inChannel;
		}
		Globals()->SetParameter(paramID, inValue);
		AudioUnitEvent auEvent;
		auEvent.mEventType = kAudioUnitEvent_ParameterValueChange;
		auEvent.mArgument.mParameter.mAudioUnit = (AudioUnit)GetComponentInstance();
		auEvent.mArgument.mParameter.mScope = kAudioUnitScope_Global;
		auEvent.mArgument.mParameter.mParameterID = paramID;
		auEvent.mArgument.mParameter.mElement = 0;
		AUEventListenerNotify(NULL, NULL, &auEvent);
	}
	AUMonotimbralInstrumentBase::HandleControlChange(inChannel, inController, inValue, inStartFrame);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	Chip700::HandleProgramChange
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void Chip700::HandleProgramChange(	int 	inChannel,
									UInt8 	inValue)
{
	AudioUnitParameterID	paramID = kParam_program;
	if ( inChannel > 0 ) {
		paramID = kParam_program_2 - 1 + inChannel;
	}
	Globals()->SetParameter(paramID, inValue);
	AudioUnitEvent auEvent;
	auEvent.mEventType = kAudioUnitEvent_ParameterValueChange;
	auEvent.mArgument.mParameter.mAudioUnit = (AudioUnit)GetComponentInstance();
	auEvent.mArgument.mParameter.mScope = kAudioUnitScope_Global;
	auEvent.mArgument.mParameter.mParameterID = paramID;
	auEvent.mArgument.mParameter.mElement = 0;
	AUEventListenerNotify(NULL, NULL, &auEvent);
	AUMonotimbralInstrumentBase::HandleProgramChange(inChannel, inValue);
}
