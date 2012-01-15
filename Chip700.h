#include "AUInstrumentBase.h"
#include "Chip700Version.h"
#include "Chip700Generator.h"

#if AU_DEBUG_DISPATCHER
	#include "AUDebugDispatcher.h"
#endif


#ifndef __Chip700_h__
#define __Chip700_h__

#include "Chip700defines.h"

#pragma mark ____Chip700
class Chip700 : public AUInstrumentBase
{
public:
	Chip700(AudioUnit component);
#if AU_DEBUG_DISPATCHER
	virtual ~Chip700 () { delete mDebugDispatcher; }
#endif
	
	virtual ComponentResult		Initialize();
	virtual void				Cleanup();
	virtual ComponentResult		Reset(	AudioUnitScope 					inScope,
										AudioUnitElement 				inElement);

	virtual OSStatus			SetParameter(					AudioUnitParameterID	inID,
									 AudioUnitScope 			inScope,
									 AudioUnitElement 			inElement,
									 Float32					inValue,
									 UInt32						inBufferOffsetInFrames);
	
	virtual	ComponentResult		GetParameterInfo(AudioUnitScope			inScope,
												 AudioUnitParameterID	inParameterID,
												 AudioUnitParameterInfo	&outParameterInfo);
    
	virtual ComponentResult		GetPropertyInfo(AudioUnitPropertyID		inID,
												AudioUnitScope			inScope,
												AudioUnitElement		inElement,
												UInt32 &			outDataSize,
												Boolean	&			outWritable );
	
	virtual ComponentResult		GetProperty(AudioUnitPropertyID inID,
											AudioUnitScope 		inScope,
											AudioUnitElement 		inElement,
											void *			outData);
	
	virtual ComponentResult		SetProperty(AudioUnitPropertyID inID,
											AudioUnitScope 		inScope,
											AudioUnitElement 	inElement,
											const void *		inData,
											UInt32              inDataSize);
	
	virtual ComponentResult		GetPresets(CFArrayRef	*outData) const;
    
    virtual OSStatus			NewFactoryPresetSet (const AUPreset & inNewFactoryPreset);
	
	
	virtual ComponentResult		SaveState(CFPropertyListRef *outData);
	virtual ComponentResult		RestoreState(CFPropertyListRef plist);
	
 	virtual	bool				SupportsTail() {return true;}
	//virtual Float64				GetLatency() {return 8.0/32000.0;}
	
	virtual ComponentResult		RealTimeStartNote(		SynthGroupElement 			*inGroup,
												  NoteInstanceID 				inNoteInstanceID, 
												  UInt32 						inOffsetSampleFrame, 
												  const MusicDeviceNoteParams &inParams);
	
	virtual ComponentResult		RealTimeStopNote(		SynthGroupElement 			*inGroup, 
												 NoteInstanceID 				inNoteInstanceID, 
												 UInt32 						inOffsetSampleFrame);

	virtual void HandlePitchWheel(	int 	inChannel,
								   UInt8 	inPitch1,
								   UInt8 	inPitch2,
								   long	inStartFrame);
	
	virtual void				HandleControlChange(int 	inChannel,
											UInt8 	inController,
											UInt8 	inValue,
											long	inStartFrame);
	
	virtual void				HandleProgramChange(int 	inChannel,
											UInt8 	inValue);
	
	virtual void				HandleResetAllControllers(	UInt8 	inChannel);
	
	virtual void				HandleAllNotesOff( UInt8 	inChannel);
	
	virtual void				HandleAllSoundOff( UInt8 	inChannel);
	
	virtual ComponentResult		Render(   AudioUnitRenderActionFlags &	ioActionFlags,
									   const AudioTimeStamp &			inTimeStamp,
									   UInt32							inNumberFrames);
	
	/*! @method Version */
	virtual ComponentResult	Version() { return kChip700Version; }
	
	int		GetNumCustomUIComponents () { return 1; }
	
	void	GetUIComponentDescs (ComponentDescription* inDescArray) {
        inDescArray[0].componentType = kAudioUnitCarbonViewComponentType;
        inDescArray[0].componentSubType = Chip700_COMP_SUBTYPE;
        inDescArray[0].componentManufacturer = Chip700_COMP_MANF;
        inDescArray[0].componentFlags = 0;
        inDescArray[0].componentFlagsMask = 0;
	}

private:
	int					mEditProg;		//編集中のプログラムNo.
	Float64				mTempo;
	
	VoiceParams			mVPset[128];
	TAUBuffer<UInt8>	mBRRdata[128];
	//エコー
	Float32				mFilterBand[5];
	
	Chip700Generator	mGenerator;
	
	int			CreateXIData( CFDataRef *data );
	int			CreatePGDataDic(CFDictionaryRef *data, int pgnum);
	void		RestorePGDataDic(CFPropertyListRef data, int pgnum);
	void		SetBandParam( int band, Float32 value );
	UInt32		GetTotalRAM();
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


#endif