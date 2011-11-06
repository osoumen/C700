#include "AUEffectBase.h"
#include "Chip700Version.h"
#include "Chip700Note.h"

#if AU_DEBUG_DISPATCHER
	#include "AUDebugDispatcher.h"
#endif


#ifndef __Chip700_h__
#define __Chip700_h__

#include "Chip700defines.h"

#pragma mark ____Chip700
class Chip700 : public AUMonotimbralInstrumentBase
{
public:
	Chip700(AudioUnit component);
#if AU_DEBUG_DISPATCHER
	virtual ~Chip700 () { delete mDebugDispatcher; }
#endif
	
	virtual ComponentResult		Initialize();
	virtual void				Cleanup();

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
	
	virtual void				HandleControlChange(int 	inChannel,
											UInt8 	inController,
											UInt8 	inValue,
											long	inStartFrame);
	
	virtual void				HandleProgramChange(int 	inChannel,
											UInt8 	inValue);
	
	virtual ComponentResult		StartNote(MusicDeviceInstrumentID		inInstrument, 
										  MusicDeviceGroupID 			inGroupID, 
										  NoteInstanceID 				&outNoteInstanceID, 
										  UInt32 						inOffsetSampleFrame, 
										  const MusicDeviceNoteParams &inParams);
	
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
	
	VoiceParams getVP(int pg) {return mVPset[pg];};
	VoiceParams getMappedVP(int key) {return mVPset[mKeyMap[key]];};
private:
	int					mEditProg;		//編集中のプログラムNo.
	int					mKeyMap[128];	//各キーに対応するプログラムNo.
	VoiceParams			mVPset[128];
	TAUBuffer<UInt8>	mBRRdata[128];
	Chip700Note			mChip700Notes[kMaximumVoices];
	Float64				mTempo;
	void					RefreshKeyMap(void);
	int						CreateXIData( CFDataRef *data );
	int						CreatePGDataDic(CFDictionaryRef *data, int pgnum);
	void					RestorePGDataDic(CFPropertyListRef data, int pgnum);
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


#endif