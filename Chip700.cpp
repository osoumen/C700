#include "Chip700.h"
#include "samplebrr.h"
#include <AudioToolbox/AudioToolbox.h>


static CFStringRef kParam_poly_Name = CFSTR("Voices");
static const float kDefaultValue_poly = 8;

static CFStringRef kParam_volL_Name = CFSTR("Vol(L)");
static const float kDefaultValue_volL = 64;

static CFStringRef kParam_volR_Name = CFSTR("Vol(R)");
static const float kDefaultValue_volR = 64;

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
	Globals()->SetParameter(kParam_volL, kDefaultValue_volL);
	Globals()->SetParameter(kParam_volR, kDefaultValue_volR);
	Globals()->SetParameter(kParam_vibdepth, kDefaultValue_vibdepth);
	Globals()->SetParameter(kParam_vibrate, kDefaultValue_vibrate);
	Globals()->SetParameter(kParam_vibdepth2, kDefaultValue_vibdepth2);
	Globals()->SetParameter(kParam_velocity, kDefaultValue_velocity);
	Globals()->SetParameter(kParam_clipnoise, kDefaultValue_clipnoise);
	Globals()->SetParameter(kParam_bendrange, kDefaultValue_bendrange);
	Globals()->SetParameter(kParam_program, kDefaultValue_program);
	Globals()->SetParameter(kParam_drummode, kDefaultValue_drummode);
	
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
		mVPset[i].lp=0;
		mVPset[i].rate=0;
		mVPset[i].volL=100;
		mVPset[i].volR=100;
		
		mVPset[i].ar=kDefaultValue_AR;
		mVPset[i].dr=kDefaultValue_DR;
		mVPset[i].sl=kDefaultValue_SL;
		mVPset[i].sr=kDefaultValue_SR;
		
		mKeyMap[i] = i;
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
			case kParam_volL:
				AUBase::FillInParameterName (outParameterInfo, kParam_volL_Name, false);
				outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
				outParameterInfo.minValue = kMinimumValue_n128;
				outParameterInfo.maxValue = kMaximumValue_127;
				outParameterInfo.defaultValue = kDefaultValue_volL;
				break;
			case kParam_volR:
				AUBase::FillInParameterName (outParameterInfo, kParam_volR_Name, false);
				outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
				outParameterInfo.minValue = kMinimumValue_n128;
				outParameterInfo.maxValue = kMaximumValue_127;
				outParameterInfo.defaultValue = kDefaultValue_volR;
				break;
			case kParam_vibdepth:
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
			
			case kAudioUnitCustomProperty_BRRData:
				outDataSize = sizeof(BRRData);
				outWritable = false;
				return noErr;
				
			case kAudioUnitCustomProperty_PGDictionary:
				outDataSize = sizeof(CFDictionaryRef);
				outWritable = false;
				return noErr;
				
			case kAudioUnitCustomProperty_ProgramName:
				outDataSize = sizeof(CFStringRef);
				outWritable = false;
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
				for (int i=kAudioUnitCustomProperty_ProgramName; i<=kAudioUnitCustomProperty_VolR; i++) {
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
				
			case kAudioUnitCustomProperty_ProgramName:
				if (mVPset[mEditProg].pgname)
					CFRelease(mVPset[mEditProg].pgname);
				mVPset[mEditProg].pgname = CFStringCreateCopy(NULL,*((CFStringRef*)inData));
				return noErr;
		}
	}
	
	return AUMonotimbralInstrumentBase::SetProperty(inID, inScope, inElement, inData, inDataSize);
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
		
		pgname = CFStringCreateCopy(NULL,CFSTR("Chip700"));
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
	bool multisample = Globals()->GetParameter(kParam_drummode_Name) != 0 ? true:false;
	
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
	memcpy(xfh.name, "(Inst Name)", 22);
	xfh.name[22] = 0x1A;
	memcpy(xfh.trkname, "FastTracker v2.00   ", 20);
	xfh.shsize = 0x102;
	CFDataAppendBytes( mdata, &xfh, sizeof(xfh) );
	
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
		vol = (int)( abs(mVPset[mEditProg].volL) + abs(mVPset[mEditProg].volR) ) / 2;
	}
	for (int i=0; i<3; i++) {
		xih.venv[i*2] = i*5;	//ADSRを反映させたい
		xih.venv[i*2+1] = vol;
		//xih.penv[i*2] = 0
	}
	xih.vnum = 4;
	xih.pnum = 0;
	xih.vsustain = 2;
	xih.vloops = 2;
	xih.vloope = 2;
	xih.psustain = 0;
	xih.ploops = 0;
	xih.ploope = 0;
	xih.vtype = 3;	//ENV_VOLUME + ENV_VOLSUSTAIN
	xih.ptype = 0;
	xih.volfade = 5000;
	xih.reserved2 = nsamples;
	CFDataAppendBytes( mdata, &xih, sizeof(xih) );
	
	// XI Sample Headers
	for (int ismp=start_prg; ismp<end_prg; ismp++) {
		xsh.samplen = mVPset[ismp].brr.size/9*32;
		xsh.loopstart = mVPset[ismp].lp/9*32;
		xsh.looplen = xsh.samplen - xsh.loopstart;
		xsh.vol = 64;	//変化しない？
		xsh.type = 0x10;	//CHN_16BIT
		if ( mVPset[ismp].loop ) {
			xsh.type = 0x01;	//Normal Loop
		}
		xsh.pan = 128;
		xsh.relnote = 0;
		xsh.finetune = 0;
		xsh.res = 0;
		if ( CFStringGetCString(mVPset[ismp].pgname,xsh.name,22,kCFStringEncodingASCII) == false ) {
			memcpy(xsh.name, "Sample", 22);
		}
		CFDataAppendBytes( mdata, &xsh, sizeof(xsh) );
	}
	
	// XI Sample Data
	for (int ismp=start_prg; ismp<end_prg; ismp++) {
		short	*wavedata;
		long	numSamples = mVPset[ismp].brr.size/9*16;
		wavedata = new short[numSamples];
		brrdecode(mVPset[ismp].brr.data, wavedata,0,0);
		CFDataAppendBytes( mdata, wavedata, numSamples*2 );
		delete [] wavedata;
	}
	data = mdata;
	
	return 0;
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
		Globals()->SetParameter(kParam_vibdepth, inValue);
		AudioUnitEvent auEvent;
		auEvent.mEventType = kAudioUnitEvent_ParameterValueChange;
		auEvent.mArgument.mParameter.mAudioUnit = (AudioUnit)GetComponentInstance();
		auEvent.mArgument.mParameter.mScope = kAudioUnitScope_Global;
		auEvent.mArgument.mParameter.mParameterID = kParam_vibdepth;
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
	Globals()->SetParameter(kParam_program, inValue);
	AudioUnitEvent auEvent;
	auEvent.mEventType = kAudioUnitEvent_ParameterValueChange;
	auEvent.mArgument.mParameter.mAudioUnit = (AudioUnit)GetComponentInstance();
	auEvent.mArgument.mParameter.mScope = kAudioUnitScope_Global;
	auEvent.mArgument.mParameter.mParameterID = kParam_program;
	auEvent.mArgument.mParameter.mElement = 0;
	AUEventListenerNotify(NULL, NULL, &auEvent);
	AUMonotimbralInstrumentBase::HandleProgramChange(inChannel, inValue);
}
