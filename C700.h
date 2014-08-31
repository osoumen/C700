#pragma once

#include "AUInstrumentBase.h"
#include "C700Version.h"
#include "C700Generator.h"

#ifdef USE_CARBON_UI
#include "AUCarbonViewBase.h"
#endif

#if AU_DEBUG_DISPATCHER
	#include "AUDebugDispatcher.h"
#endif

#include "C700defines.h"
#include "C700Kernel.h"

#pragma mark ____C700
class C700 : public AUInstrumentBase
{
public:
	C700(AudioUnit component);
	virtual ~C700();
	
	virtual OSStatus			Initialize();
	virtual void				Cleanup();
	virtual OSStatus			Reset(	AudioUnitScope 					inScope,
										AudioUnitElement 				inElement);

	virtual OSStatus			SetParameter(					AudioUnitParameterID	inID,
									 AudioUnitScope 			inScope,
									 AudioUnitElement 			inElement,
									 Float32					inValue,
									 UInt32						inBufferOffsetInFrames);
	
	virtual	OSStatus			GetParameterInfo(AudioUnitScope			inScope,
												 AudioUnitParameterID	inParameterID,
												 AudioUnitParameterInfo	&outParameterInfo);
    
	virtual OSStatus			GetPropertyInfo(AudioUnitPropertyID		inID,
												AudioUnitScope			inScope,
												AudioUnitElement		inElement,
												UInt32 &			outDataSize,
												Boolean	&			outWritable );
	
	virtual OSStatus			GetProperty(AudioUnitPropertyID inID,
											AudioUnitScope 		inScope,
											AudioUnitElement 		inElement,
											void *			outData);
	
	virtual OSStatus			SetProperty(AudioUnitPropertyID inID,
											AudioUnitScope 		inScope,
											AudioUnitElement 	inElement,
											const void *		inData,
											UInt32              inDataSize);
	
	virtual ComponentResult		GetPresets(CFArrayRef	*outData) const;
    
    virtual OSStatus			NewFactoryPresetSet (const AUPreset & inNewFactoryPreset);
	
	
	virtual OSStatus			SaveState(CFPropertyListRef *outData);
	virtual OSStatus			RestoreState(CFPropertyListRef plist);
	
 	virtual	bool				SupportsTail() {return true;}
//virtual Float64				GetLatency() {return 8.0/32000.0;}
	
	virtual OSStatus			RealTimeStartNote(		SynthGroupElement 			*inGroup,
														NoteInstanceID 				inNoteInstanceID, 
														UInt32 						inOffsetSampleFrame, 
														const MusicDeviceNoteParams &inParams);
	
	virtual OSStatus			RealTimeStopNote(		MusicDeviceGroupID 			inGroup, 
														NoteInstanceID 				inNoteInstanceID, 
														UInt32 						inOffsetSampleFrame);

	virtual OSStatus			HandlePitchWheel(	UInt8 	inChannel,
								   UInt8 	inPitch1,
								   UInt8 	inPitch2,
								   UInt32	inStartFrame);
	
	virtual OSStatus			HandleControlChange(UInt8 	inChannel,
											UInt8 	inController,
											UInt8 	inValue,
											UInt32	inStartFrame);
	
	virtual OSStatus			HandleProgramChange(UInt8 	inChannel,
													UInt8 	inValue,
                                                    UInt32  inStartFrame);
	
	virtual OSStatus			HandleResetAllControllers(	UInt8 	inChannel);
	
	virtual OSStatus			HandleAllNotesOff( UInt8 	inChannel);
	
	virtual OSStatus			HandleAllSoundOff( UInt8 	inChannel);
	
	virtual OSStatus			Render(   AudioUnitRenderActionFlags &	ioActionFlags,
										const AudioTimeStamp &			inTimeStamp,
									   UInt32							inNumberFrames);
	
	/*! @method Version */
	virtual OSStatus	Version() { return kC700Version; }
	
#ifdef USE_CARBON_UI
	int		GetNumCustomUIComponents () { return 1; }
	
	void	GetUIComponentDescs (ComponentDescription* inDescArray) {
        inDescArray[0].componentType = kAudioUnitCarbonViewComponentType;
        inDescArray[0].componentSubType = C700_COMP_SUBTYPE;
        inDescArray[0].componentManufacturer = C700_COMP_MANF;
        inDescArray[0].componentFlags = 0;
        inDescArray[0].componentFlagsMask = 0;
	}
#endif

private:
	static void PropertyNotifyFunc(int propID, void* userData);
	static void ParameterSetFunc(int paramID, float value, void* userData);
	
	double				mTempo;
	
	C700Kernel			*mEfx;
	AUPreset			*mPresets;
    bool                mParameterHasChanged[kNumberOfParameters];
	
	int			CreatePGDataDic(CFDictionaryRef *data, int pgnum);
	void		RestorePGDataDic(CFPropertyListRef data, int pgnum);
};
