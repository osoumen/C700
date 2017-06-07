#include <AudioToolbox/AudioToolbox.h>
#include "C700.h"
#include "brrcodec.h"
#include "AudioFile.h"

//-----------------------------------------------------------------------------

COMPONENT_ENTRY(C700)

//-----------------------------------------------------------------------------
//	C700::C700
//-----------------------------------------------------------------------------
C700::C700(AudioUnit component)
: MusicDeviceBase(component, 0, 1, 32, 0)
, mEfx(NULL)
{
    createPropertyParamMap(mPropertyParams);
    
	CreateElements();
	Globals()->UseIndexedParameters(kNumberOfParameters);
	
	mEfx = new C700Kernel();
	mEfx->SetPropertyNotifyFunc(PropertyNotifyFunc, this);
	mEfx->SetParameterSetFunc(ParameterSetFunc, this);
	
	//プリセット名テーブルを作成する
	mPresets = new AUPreset[NUM_PRESETS];
	for (int i = 0; i < NUM_PRESETS; ++i) {
		const char		*pname;
		pname = C700Kernel::GetPresetName(i);
		mPresets[i].presetNumber = i;
		CFMutableStringRef cfpname = CFStringCreateMutable(NULL, 64);
		mPresets[i].presetName = cfpname;
		CFStringAppendCString( cfpname, pname, kCFStringEncodingASCII );
    }
	
	//デフォルト値を設定する
	for ( int i=0; i<kNumberOfParameters; i++ ) {
		Globals()->SetParameter(i, C700Kernel::GetParameterDefault(i) );
        mParameterHasChanged[i] = false;
	}
	
#if AU_DEBUG_DISPATCHER
	mDebugDispatcher = new AUDebugDispatcher(this);
#endif
}

//-----------------------------------------------------------------------------
C700::~C700()
{
	for (int i = 0; i < NUM_PRESETS; ++i) {
		CFRelease(mPresets[i].presetName);
	}
	delete [] mPresets;
	if ( mEfx ) {
		delete mEfx;
	}
	
#if AU_DEBUG_DISPATCHER
	delete mDebugDispatcher;
#endif
}

//-----------------------------------------------------------------------------
void C700::PropertyNotifyFunc(int propID, void* userData)
{
	C700	*This = reinterpret_cast<C700*> (userData);
	This->PropertyChanged(propID, kAudioUnitScope_Global, 0);
}

//-----------------------------------------------------------------------------
void C700::ParameterSetFunc(int paramID, float value, void* userData)
{
	C700	*This = reinterpret_cast<C700*> (userData);
	This->Globals()->SetParameter(paramID, value);
	AudioUnitEvent auEvent;
	auEvent.mEventType = kAudioUnitEvent_ParameterValueChange;
	auEvent.mArgument.mParameter.mAudioUnit = (AudioUnit)This->GetComponentInstance();
	auEvent.mArgument.mParameter.mScope = kAudioUnitScope_Global;
	auEvent.mArgument.mParameter.mParameterID = paramID;
	auEvent.mArgument.mParameter.mElement = 0;
	AUEventListenerNotify(NULL, NULL, &auEvent);
    This->mParameterHasChanged[paramID] = true;
}

//-----------------------------------------------------------------------------
ComponentResult C700::Initialize()
{	
	MusicDeviceBase::Initialize();
	return noErr;
}

//-----------------------------------------------------------------------------
void C700::Cleanup()
{
}

//-----------------------------------------------------------------------------
ComponentResult C700::Reset(	AudioUnitScope 		inScope,
							   AudioUnitElement 	inElement)
{
//	if (inScope == kAudioUnitScope_Global) {
		mEfx->Reset();
//	}
	return MusicDeviceBase::Reset(inScope, inElement);
}

//-----------------------------------------------------------------------------
bool C700::StreamFormatWritable(	AudioUnitScope				scope,
                                   AudioUnitElement				element)
{
	return IsInitialized() ? false : true;
}
//-----------------------------------------------------------------------------
OSStatus	C700::Render(   AudioUnitRenderActionFlags &	ioActionFlags,
												const AudioTimeStamp &			inTimeStamp,
												UInt32							inNumberFrames)
{
	OSStatus result = MusicDeviceBase::Render(ioActionFlags, inTimeStamp, inNumberFrames);
	
    {
        double tempo;
        double currentBeat;
        Boolean isPlaying;
        CallHostBeatAndTempo(&currentBeat, &tempo);
        CallHostTransportState(&isPlaying, NULL, NULL, NULL, NULL, NULL);
        mEfx->SetTempo( tempo );
        mEfx->SetCurrentSampleInTimeLine( currentBeat );
        mEfx->SetSampleRate( GetOutput(0)->GetStreamFormat().mSampleRate );
        mEfx->SetIsPlaying(isPlaying?true:false);
    }
	//バッファの確保
	float				*output[2];
	AudioBufferList&	bufferList = GetOutput(0)->GetBufferList();
	
    UInt32 numOutputs = Outputs().GetNumberOfElements();
	for (UInt32 j = 0; j < numOutputs; ++j)
	{
		AudioBufferList& bufferList = GetOutput(j)->GetBufferList();
		for (UInt32 k = 0; k < bufferList.mNumberBuffers; ++k)
		{
			memset(bufferList.mBuffers[k].mData, 0, bufferList.mBuffers[k].mDataByteSize);
		}
	}
    
	int numChans = bufferList.mNumberBuffers;
	if (numChans > 2) return -1;
	output[0] = (float*)bufferList.mBuffers[0].mData;
	output[1] = numChans==2 ? (float*)bufferList.mBuffers[1].mData : NULL;

	//パラメータの反映
	for ( int i=0; i<kNumberOfParameters; i++ ) {
        if (mParameterHasChanged[i]) {
            mEfx->SetParameter(i, Globals()->GetParameter(i));
            mParameterHasChanged[i] = false;
            if (i == kParam_alwaysDelayNote) {
                // 遅延時間の変更をホストに通知
                PropertyChanged(kAudioUnitProperty_Latency, kAudioUnitScope_Global, 0);
            }
        }
	}
	
	mEfx->Render(inNumberFrames, output);
	
	return result;
}

//-----------------------------------------------------------------------------
//	C700::SetParameter
//-----------------------------------------------------------------------------
OSStatus C700::SetParameter(	AudioUnitParameterID			inID,
								 AudioUnitScope 				inScope,
								 AudioUnitElement 				inElement,
								 Float32						inValue,
								 UInt32							inBufferOffsetInFrames)
{
	OSStatus result = MusicDeviceBase::SetParameter(inID, inScope, inElement, inValue, inBufferOffsetInFrames);
	if ( inScope == kAudioUnitScope_Global ) {
		//mEfx->SetParameter(inID, inValue);
        if (inID < kNumberOfParameters) {
            mParameterHasChanged[inID] = true;
        }
		switch ( inID ) {
			case kParam_echodelay:
				PropertyChanged(kAudioUnitCustomProperty_TotalRAM, kAudioUnitScope_Global, 0);
				break;
		}
	}
	return result;
}

//-----------------------------------------------------------------------------
AudioUnitParameterUnit getParameterUnit( int id )
{
	switch(id)
	{
		case kParam_poly:
			return kAudioUnitParameterUnit_Indexed;
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
			return kAudioUnitParameterUnit_Indexed;
		case kParam_vibrate:
			return kAudioUnitParameterUnit_Hertz;
		case kParam_vibdepth2:
			return kAudioUnitParameterUnit_Generic;
		case kParam_velocity:
			return kAudioUnitParameterUnit_Indexed;
		case kParam_engine:
			return kAudioUnitParameterUnit_Indexed;
		case kParam_bendrange:
			return kAudioUnitParameterUnit_Indexed;
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
			return kAudioUnitParameterUnit_Indexed;
			
		case kParam_bankAmulti:
			return kAudioUnitParameterUnit_Boolean;
		case kParam_bankBmulti:
			return kAudioUnitParameterUnit_Boolean;
		case kParam_bankCmulti:
			return kAudioUnitParameterUnit_Boolean;
		case kParam_bankDmulti:
			return kAudioUnitParameterUnit_Boolean;
			
			//エコー
		case kParam_mainvol_L:
			return kAudioUnitParameterUnit_Indexed;
		case kParam_mainvol_R:
			return kAudioUnitParameterUnit_Indexed;
		case kParam_echovol_L:
			return kAudioUnitParameterUnit_Indexed;
		case kParam_echovol_R:
			return kAudioUnitParameterUnit_Indexed;
		case kParam_echoFB:
			return kAudioUnitParameterUnit_Indexed;
		case kParam_echodelay:
			return kAudioUnitParameterUnit_Indexed;
		case kParam_fir0:
			return kAudioUnitParameterUnit_Indexed;
		case kParam_fir1:
			return kAudioUnitParameterUnit_Indexed;
		case kParam_fir2:
			return kAudioUnitParameterUnit_Indexed;
		case kParam_fir3:
			return kAudioUnitParameterUnit_Indexed;
		case kParam_fir4:
			return kAudioUnitParameterUnit_Indexed;
		case kParam_fir5:
			return kAudioUnitParameterUnit_Indexed;
		case kParam_fir6:
			return kAudioUnitParameterUnit_Indexed;
		case kParam_fir7:
			return kAudioUnitParameterUnit_Indexed;
            
        case kParam_voiceAllocMode:
            return kAudioUnitParameterUnit_Indexed;
        case kParam_fastReleaseAsKeyOff:
            return kAudioUnitParameterUnit_Boolean;
            
		default:
			return kAudioUnitParameterUnit_Indexed;
	}
}
//-----------------------------------------------------------------------------
//	C700::GetParameterInfo
//-----------------------------------------------------------------------------
ComponentResult		C700::GetParameterInfo(AudioUnitScope		inScope,
											  AudioUnitParameterID	inParameterID,
											  AudioUnitParameterInfo	&outParameterInfo )
{
	ComponentResult result = noErr;
	
	outParameterInfo.flags = 	kAudioUnitParameterFlag_IsWritable
		|		kAudioUnitParameterFlag_IsReadable;
    
    if (inScope == kAudioUnitScope_Global) {
		if ( /*inParameterID >= 0 &&*/ inParameterID < kNumberOfParameters ) {
			CFStringRef	cfName = CFStringCreateWithCString(NULL, mEfx->GetParameterName(inParameterID), kCFStringEncodingUTF8);
			AUBase::FillInParameterName(outParameterInfo, cfName, true);
			outParameterInfo.unit = getParameterUnit( inParameterID );
			outParameterInfo.minValue = C700Kernel::GetParameterMin(inParameterID);
			outParameterInfo.maxValue = C700Kernel::GetParameterMax(inParameterID);
			outParameterInfo.defaultValue = C700Kernel::GetParameterDefault(inParameterID);
		}
		else {
			result = kAudioUnitErr_InvalidParameter;
		}
	}
	else {
        result = kAudioUnitErr_InvalidParameter;
    }

	return result;
}

//-----------------------------------------------------------------------------
//	C700::GetPropertyInfo
//-----------------------------------------------------------------------------
ComponentResult		C700::GetPropertyInfo (AudioUnitPropertyID	inID,
											  AudioUnitScope		inScope,
											  AudioUnitElement	inElement,
											  UInt32 &		outDataSize,
											  Boolean &		outWritable)
{
	if (inScope == kAudioUnitScope_Global) {
#ifndef USE_CARBON_UI
        if (inID == kAudioUnitProperty_CocoaUI) {
            outWritable = false;
            outDataSize = sizeof (AudioUnitCocoaViewInfo);
            return noErr;
        }
#endif
        auto it = mPropertyParams.find(inID);
        if (it != mPropertyParams.end()) {
            if ((it->second.dataType == propertyDataTypeString) ||
                (it->second.dataType == propertyDataTypeFilePath)) {
                // outDataSizeには最大文字数が格納されている
                outDataSize = sizeof(void *);
            }
            else {
                outDataSize = it->second.outDataSize;
            }
            outWritable = it->second.outWritable;
            return noErr;
        }
	}
	
	return MusicDeviceBase::GetPropertyInfo(inID, inScope, inElement, outDataSize, outWritable);
}

//-----------------------------------------------------------------------------
//	C700::GetProperty
//-----------------------------------------------------------------------------
ComponentResult		C700::GetProperty(	AudioUnitPropertyID inID,
											AudioUnitScope 		inScope,
											AudioUnitElement 	inElement,
											void *			outData )
{
	if (inScope == kAudioUnitScope_Global) {
#ifndef USE_CARBON_UI
        if (inID == kAudioUnitProperty_CocoaUI) {
            // Look for a resource in the main bundle by name and type.
            CFBundleRef bundle = CFBundleGetBundleWithIdentifier( CFSTR("com.picopicose.audiounit.C700") );
            
            if (bundle == NULL) return fnfErr;
            
            CFURLRef bundleURL = CFBundleCopyBundleURL( bundle );
            
            if (bundleURL == NULL) return fnfErr;
            
            AudioUnitCocoaViewInfo cocoaInfo;
            cocoaInfo.mCocoaAUViewBundleLocation = bundleURL;
            cocoaInfo.mCocoaAUViewClass[0] = CFStringCreateWithCString(NULL, "C700_CocoaViewFactory", kCFStringEncodingUTF8);
            
            *((AudioUnitCocoaViewInfo *)outData) = cocoaInfo;
            
            return noErr;
        }
#endif
        auto it = mPropertyParams.find(inID);
        if (it != mPropertyParams.end()) {
            switch (it->second.dataType) {
                case propertyDataTypeFloat32:
                    *((Float32 *)outData) = mEfx->GetPropertyValue(inID);
                    break;
                case propertyDataTypeDouble:
                    *((double *)outData) = mEfx->GetPropertyDoubleValue(inID);
                    break;
                case propertyDataTypeInt32:
                    *((int *)outData) = mEfx->GetPropertyValue(inID);
                    break;
                case propertyDataTypeBool:
                    *((bool *)outData) = mEfx->GetPropertyValue(inID)>0.5f? true:false;
                    break;
                case propertyDataTypeStruct:
                    mEfx->GetPropertyStructValue(inID, outData);
                    break;
                case propertyDataTypeString:
                {
                    const char *string = (char *)mEfx->GetPropertyPtrValue(inID);
                    CFStringRef	str =
                    CFStringCreateWithCString(NULL, string, kCFStringEncodingUTF8);
                    *((char **)outData) = (char *)str;  //使用後要release
                    break;
                }
                case propertyDataTypeFilePath:
                {
                    const char *string = (char *)mEfx->GetPropertyPtrValue(inID);
                    CFURLRef	url =
                    CFURLCreateFromFileSystemRepresentation(NULL, (UInt8*)string, strlen(string), false);
                    *((char **)outData) = (char *)url;  //使用後要release
                    break;
                }
                    
                case propertyDataTypeVariableData:
                {
                    CFDataRef dataRef;
                    dataRef = CFDataCreate(NULL, (UInt8*)mEfx->GetPropertyPtrValue(inID), mEfx->GetPropertyPtrDataSize(inID));
                    *((char **)outData) = (char *)dataRef;  //使用後要release
                    break;
                }
                case propertyDataTypePointer:
                    *((char **)outData) = (char *)mEfx->GetPropertyPtrValue(inID);
                    break;
            }
            return noErr;
        }
    }
	return MusicDeviceBase::GetProperty(inID, inScope, inElement, outData);
}

//-----------------------------------------------------------------------------
//	C700::SetProperty
//-----------------------------------------------------------------------------
ComponentResult		C700::SetProperty(	AudioUnitPropertyID inID,
											AudioUnitScope 		inScope,
											AudioUnitElement 	inElement,
											const void *		inData,
											UInt32              inDataSize)
{
	if (inScope == kAudioUnitScope_Global) {
        auto it = mPropertyParams.find(inID);
        if (it != mPropertyParams.end()) {
            if (it->second.readOnly == false) {
                switch (it->second.dataType) {
                    case propertyDataTypeFloat32:
                        mEfx->SetPropertyValue(inID, *((float*)inData) );
                        break;
                    case propertyDataTypeDouble:
                        mEfx->SetPropertyDoubleValue(inID, *((double*)inData));
                        break;
                    case propertyDataTypeInt32:
                        mEfx->SetPropertyValue(inID, *((int*)inData) );
                        break;
                    case propertyDataTypeBool:
                        mEfx->SetPropertyValue(inID, *((bool*)inData) ? 1.0f:.0f );
                        break;
                    case propertyDataTypeStruct:
                        mEfx->SetPropertyPtrValue(inID, inData, it->second.outDataSize);
                        break;
                    case propertyDataTypeString:
                    {
                        char **ptr = (char**)inData;
                        CFIndex length = CFStringGetLength(reinterpret_cast<CFStringRef>(*ptr)) + 1;
                        char	*string = new char[length];
                        CFStringGetCString(reinterpret_cast<CFStringRef>(*ptr), string, length, kCFStringEncodingUTF8);
                        mEfx->SetPropertyPtrValue(inID, string, length);
                        delete [] string;
                        break;
                    }
                    case propertyDataTypeFilePath:
                    {
                        char **ptr = (char**)inData;
                        CFStringRef pathStr = CFURLCopyFileSystemPath(reinterpret_cast<CFURLRef>(*ptr), kCFURLPOSIXPathStyle);
                        char		*path = new char[it->second.outDataSize];
                        CFStringGetCString(pathStr, path, it->second.outDataSize, kCFStringEncodingUTF8);
                        CFRelease(pathStr);
                        mEfx->SetPropertyPtrValue(inID, path, it->second.outDataSize);
                        delete [] path;
                        break;
                    }
                    case propertyDataTypeVariableData:
                    {
                        char **ptr = (char**)inData;
                        CFDataRef data = reinterpret_cast<CFDataRef>(*ptr);
                        mEfx->SetPropertyPtrValue(inID, CFDataGetBytePtr(data), CFDataGetLength(data));
                        break;
                    }
                    case propertyDataTypePointer:
                    {
                        char **ptr = (char**)inData;
                        mEfx->SetPropertyPtrValue(inID, *ptr, it->second.outDataSize);
                        break;
                    }
                }
            }
            return noErr;
        }
	}
	return MusicDeviceBase::SetProperty(inID, inScope, inElement, inData, inDataSize);
}

//-----------------------------------------------------------------------------
//	C700::GetPresets
//-----------------------------------------------------------------------------
ComponentResult C700::GetPresets(CFArrayRef *outData) const
{	
	if (outData == NULL) return noErr;
	
	CFMutableArrayRef theArray = CFArrayCreateMutable(NULL, NUM_PRESETS, NULL);
	for (int i = 0; i < NUM_PRESETS; ++i) {
		CFArrayAppendValue(theArray, &mPresets[i]);
    }
    
	*outData = (CFArrayRef)theArray;
	return noErr;
}

//-----------------------------------------------------------------------------
OSStatus C700::NewFactoryPresetSet(const AUPreset &inNewFactoryPreset)
{
	UInt32 chosenPreset = inNewFactoryPreset.presetNumber;
	if ( chosenPreset < (UInt32)NUM_PRESETS ) {
		mEfx->SelectPreset(chosenPreset);
		char cname[32];
		CFStringGetCString(inNewFactoryPreset.presetName, cname, 32, kCFStringEncodingASCII);
		//mEfx->setProgramName(cname);
		SetAFactoryPresetAsCurrent(inNewFactoryPreset);
		for ( UInt32 i=0; i<kNumberOfParameters; i++ ) {
			Globals()->SetParameter(i, mEfx->GetParameter(i));
		}
		return noErr;
	}
	return kAudioUnitErr_InvalidPropertyValue;
}

//-----------------------------------------------------------------------------
//	C700::SaveState
//-----------------------------------------------------------------------------
ComponentResult	C700::SaveState(CFPropertyListRef *outData)
{
	ComponentResult result;
	result = MusicDeviceBase::SaveState(outData);
	CFMutableDictionaryRef	dict=(CFMutableDictionaryRef)*outData;
	if (result == noErr) {
		CFDictionaryRef	pgdata;
		CFStringRef pgnum,pgname;
		
		
        mEfx->SetPropertyToDict(dict, mPropertyParams[kAudioUnitCustomProperty_EditingChannel]);
        mEfx->SetPropertyToDict(dict, mPropertyParams[kAudioUnitCustomProperty_EditingProgram]);

        for (int i=0; i<128; i++) {
			const BRRData		*brr = mEfx->GetBRRData(i);
			if (brr->data) {
				mEfx->CreatePGDataDic(&pgdata, i);
				pgnum = CFStringCreateWithFormat(NULL,NULL,CFSTR("pg%03d"),i);
				CFDictionarySetValue(dict, pgnum, pgdata);
				CFRelease(pgdata);
				CFRelease(pgnum);
			}
		}
		
        // saveToSongの設定を保存
        auto it = mPropertyParams.begin();
        while (it != mPropertyParams.end()) {
            if (it->second.saveToSong) {
                if (it->first == kAudioUnitCustomProperty_EditingChannel ||
                    it->first == kAudioUnitCustomProperty_EditingProgram) {
                    it++;
                    continue;
                }
                mEfx->SetPropertyToDict(dict, it->second);
            }
            it++;
        }
		
		pgname = CFStringCreateCopy(NULL,CFSTR("C700"));
		CFDictionarySetValue(dict, CFSTR(kAUPresetNameKey), pgname);
		CFRelease(pgname);
	}
	return result;
}

//-----------------------------------------------------------------------------
//	C700::RestoreState
//-----------------------------------------------------------------------------
ComponentResult	C700::RestoreState(CFPropertyListRef plist)
{
	ComponentResult result;
	result = MusicDeviceBase::RestoreState(plist);
    for (int i=0; i<kNumberOfParameters; i++) {
        mParameterHasChanged[i] = true;
    }
	CFDictionaryRef dict = static_cast<CFDictionaryRef>(plist);
	if (result == noErr) {
		//プログラムの復元
		CFStringRef pgnum;
		CFDictionaryRef	pgdata;
		for (int i=0; i<128; i++) {
			pgnum = CFStringCreateWithFormat(NULL,NULL,CFSTR("pg%03d"),i);
			if (CFDictionaryContainsKey(dict, pgnum)) {
				pgdata = reinterpret_cast<CFDictionaryRef>(CFDictionaryGetValue(dict, pgnum));
				mEfx->RestorePGDataDic(pgdata, i);
			}
			CFRelease(pgnum);
		}
		
        // saveToSongのプロパティを復元
        auto it = mPropertyParams.begin();
        while (it != mPropertyParams.end()) {
            if (it->second.saveToSong) {
                if (it->first == kAudioUnitCustomProperty_EditingChannel ||
                    it->first == kAudioUnitCustomProperty_EditingProgram) {
                    it++;
                    continue;
                }
                mEfx->RestorePropertyFromDict(dict, it->second);
            }
            it++;
        }
        // EditChanの方が後に設定されてしまうとうまく復元されない
        mEfx->RestorePropertyFromDict(dict, mPropertyParams[kAudioUnitCustomProperty_EditingChannel]);
        mEfx->RestorePropertyFromDict(dict, mPropertyParams[kAudioUnitCustomProperty_EditingProgram]);
	}
	return result;
}

//-----------------------------------------------------------------------------
OSStatus C700::StartNote(	MusicDeviceInstrumentID 	inInstrument,
                            MusicDeviceGroupID 			inGroupID,
                            NoteInstanceID *			outNoteInstanceID,
                            UInt32 						inOffsetSampleFrame,
                            const MusicDeviceNoteParams &inParams)
{
	//MIDIチャンネルの取得
	unsigned int		chID = inGroupID % 16;
	
	mEfx->HandleNoteOn(chID, inParams.mPitch, inParams.mVelocity, inParams.mPitch+chID*256, inOffsetSampleFrame);
	return noErr;
}

//-----------------------------------------------------------------------------
OSStatus C700::StopNote(	MusicDeviceGroupID 			inGroupID,
                            NoteInstanceID 				inNoteInstanceID,
                            UInt32 						inOffsetSampleFrame)
{
	//MIDIチャンネルの取得
	unsigned int		chID = inGroupID % 16;
	
	mEfx->HandleNoteOff(chID, inNoteInstanceID, inNoteInstanceID+chID*256, inOffsetSampleFrame);
	return noErr;
}

//-----------------------------------------------------------------------------
OSStatus C700::HandlePitchWheel(	UInt8 	inChannel,
							   UInt8 	inPitch1,
							   UInt8 	inPitch2,
							   UInt32	inStartFrame)
{
	mEfx->HandlePitchBend(inChannel, inPitch1, inPitch2, inStartFrame);
	return noErr;
}

//-----------------------------------------------------------------------------
//	C700::HandleControlChange
//-----------------------------------------------------------------------------
OSStatus C700::HandleControlChange(	UInt8 	inChannel,
									UInt8 	inController,
									UInt8 	inValue,
									UInt32	inStartFrame)
{
	mEfx->HandleControlChange(inChannel, inController, inValue, inStartFrame);
	return MusicDeviceBase::HandleControlChange(inChannel, inController, inValue, inStartFrame);
}

//-----------------------------------------------------------------------------
//	C700::HandleProgramChange
//-----------------------------------------------------------------------------
OSStatus C700::HandleProgramChange(	UInt8	inChannel,
                                    UInt8	inValue,
                                    UInt32  inStartFrame)
{
	mEfx->HandleProgramChange(inChannel, inValue, inStartFrame);
	return MusicDeviceBase::HandleProgramChange(inChannel, inValue, inStartFrame);
}

//-----------------------------------------------------------------------------
OSStatus C700::HandleResetAllControllers( UInt8 	inChannel)
{
	mEfx->HandleResetAllControllers(inChannel, 0);
	return MusicDeviceBase::HandleResetAllControllers( inChannel);
}

//-----------------------------------------------------------------------------
OSStatus C700::HandleAllNotesOff( UInt8 inChannel )
{
	mEfx->HandleAllNotesOff(inChannel, 0);
	return MusicDeviceBase::HandleAllNotesOff( inChannel);
}

//-----------------------------------------------------------------------------
OSStatus C700::HandleAllSoundOff( UInt8 inChannel )
{
	mEfx->HandleAllSoundOff(inChannel, 0);
	return MusicDeviceBase::HandleAllSoundOff( inChannel);
}
