/*
 *  EfxAccess.cpp
 *  Chip700
 *
 *  Created by 藤田 匡彦 on 12/10/08.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "EfxAccess.h"

//-----------------------------------------------------------------------------
EfxAccess::EfxAccess( void *efx )
#if AU
: mAU((AudioUnit)efx)
, mEventListener(NULL)
#else
: mEfx((AudioEffect*)efx)
#endif
{
}

//-----------------------------------------------------------------------------
EfxAccess::~EfxAccess()
{
}

//-----------------------------------------------------------------------------
bool	EfxAccess::GetBRRFileData( BRRFile **outData )
{
	return false;
}

//-----------------------------------------------------------------------------
bool	EfxAccess::SetBRRFileData( const BRRFile *data )
{
	return false;
}

//-----------------------------------------------------------------------------
bool	EfxAccess::GetXIFileData( XIFile **outData )
{
#if AU
	XIFile	*file = new XIFile(NULL, true);
	*outData = file;
	
	//データを取得する
	CFDataRef	cfdata;
	UInt32 size = sizeof(CFDataRef);
	if (
		AudioUnitGetProperty(mAU,kAudioUnitCustomProperty_XIData,
							 kAudioUnitScope_Global, 0, &cfdata,&size)
		== noErr )
	{
		file->SetCFData( cfdata );
		CFRelease(cfdata);
		return true;
	}
	CFRelease(cfdata);
	return true;
#else
	return false;
#endif
}

//-----------------------------------------------------------------------------
bool	EfxAccess::GetPlistBRRFileData( PlistBRRFile **outData )
{
#if AU
	PlistBRRFile	*file = new PlistBRRFile(NULL, true);
	*outData = file;
	
	//Dictionaryデータを取得する
	CFPropertyListRef	propertydata;
	UInt32 size = sizeof(CFPropertyListRef);
	if (
	AudioUnitGetProperty(mAU,kAudioUnitCustomProperty_PGDictionary,
						 kAudioUnitScope_Global, 0, &propertydata,&size)
		== noErr )
	{
		file->SetPlistData( propertydata );
		CFRelease(propertydata);
		return true;
	}
	CFRelease(propertydata);
	return true;
#else
	return false;
#endif
}

//-----------------------------------------------------------------------------
bool	EfxAccess::SetPlistBRRFileData( const PlistBRRFile *data )
{
#if AU
	CFPropertyListRef	propertydata = data->GetPlistData();
	if ( propertydata == NULL ) return false;
	
	UInt32	inSize = sizeof(CFPropertyListRef);
	if (
		AudioUnitSetProperty(mAU, kAudioUnitCustomProperty_PGDictionary, 
							 kAudioUnitScope_Global, 0, &propertydata, inSize)
		== noErr )
	{
		return true;
	}
	
	return false;	
#else
	return false;
#endif
}

//-----------------------------------------------------------------------------
bool EfxAccess::SetSourceFilePath( const char *path )
{
#if AU
	UInt32		inSize = sizeof(CFStringRef);
	CFURLRef	url = CFURLCreateFromFileSystemRepresentation(NULL, (UInt8*)path, strlen(path), false);

	if (
		AudioUnitSetProperty(mAU, kAudioUnitCustomProperty_SourceFileRef, kAudioUnitScope_Global, 0, &url, inSize)
		== noErr ) {
		CFRelease( url );
		return true;
	}
	CFRelease( url );
	return false;
#else
	//VST時の処理
#endif
}
//-----------------------------------------------------------------------------
bool EfxAccess::SetProgramName( const char *pgname )
{
#if AU
	
	UInt32		inSize = sizeof(CFStringRef);
	CFStringRef	pgnameRef = CFStringCreateWithCString(NULL, pgname, kCFStringEncodingUTF8);
	
	if (
		AudioUnitSetProperty(mAU, kAudioUnitCustomProperty_ProgramName, kAudioUnitScope_Global, 0, &pgnameRef, inSize)
		== noErr ) {
		CFRelease( pgnameRef );
		return true;
	}
	CFRelease( pgnameRef );
	return false;
#else
	//VST時の処理
#endif
}

//-----------------------------------------------------------------------------
bool EfxAccess::GetBRRData( BRRData *data, int *size )
{
#if AU
	UInt32		outSize = *size;
	
	if (
	AudioUnitGetProperty(mAU, kAudioUnitCustomProperty_BRRData, kAudioUnitScope_Global, 0, data, &outSize)
		== noErr ) {
		return true;
	}
	return false;
#else
	//VST時の処理
#endif
}

//-----------------------------------------------------------------------------
bool EfxAccess::SetBRRData( const BRRData *data )
{
#if AU
	UInt32		inSize = sizeof(BRRData);
	
	if (
		AudioUnitSetProperty(mAU, kAudioUnitCustomProperty_BRRData, kAudioUnitScope_Global, 0, data, inSize)
		== noErr ) {
		return true;
	}
	return false;
#else
	//VST時の処理
#endif
}

//-----------------------------------------------------------------------------
float EfxAccess::GetPropertyValue( int propertyId )
{
	float		value = .0f;
	char		outDataPtr[8];
	UInt32		outDataSize=8;
	
	if ( 
		propertyId != kAudioUnitCustomProperty_PGDictionary &&
		propertyId != kAudioUnitCustomProperty_XIData &&
		propertyId != kAudioUnitCustomProperty_SourceFileRef ) {
		AudioUnitGetProperty(mAU, propertyId, kAudioUnitScope_Global, 0, &outDataPtr, &outDataSize);
	}
	
	switch (propertyId) {
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
		case kAudioUnitCustomProperty_EditingChannel:
		case kAudioUnitCustomProperty_Bank:
		case kAudioUnitCustomProperty_TotalRAM:
		case kAudioUnitCustomProperty_LoopPoint:
			value = *((int*)outDataPtr);
			break;
			
		case kAudioUnitCustomProperty_Rate:
			value = *((double*)outDataPtr);
			break;
			
		case kAudioUnitCustomProperty_Loop:
		case kAudioUnitCustomProperty_Echo:
		case kAudioUnitCustomProperty_IsEmaphasized:
			value = *((bool*)outDataPtr);
			break;
			
		case kAudioUnitCustomProperty_Band1:
		case kAudioUnitCustomProperty_Band2:
		case kAudioUnitCustomProperty_Band3:
		case kAudioUnitCustomProperty_Band4:
		case kAudioUnitCustomProperty_Band5:
			value = *((Float32*)outDataPtr);
			break;
			
		case kAudioUnitCustomProperty_NoteOnTrack_1:
		case kAudioUnitCustomProperty_NoteOnTrack_2:
		case kAudioUnitCustomProperty_NoteOnTrack_3:
		case kAudioUnitCustomProperty_NoteOnTrack_4:
		case kAudioUnitCustomProperty_NoteOnTrack_5:	
		case kAudioUnitCustomProperty_NoteOnTrack_6:
		case kAudioUnitCustomProperty_NoteOnTrack_7:
		case kAudioUnitCustomProperty_NoteOnTrack_8:
		case kAudioUnitCustomProperty_NoteOnTrack_9:
		case kAudioUnitCustomProperty_NoteOnTrack_10:
		case kAudioUnitCustomProperty_NoteOnTrack_11:
		case kAudioUnitCustomProperty_NoteOnTrack_12:
		case kAudioUnitCustomProperty_NoteOnTrack_13:
		case kAudioUnitCustomProperty_NoteOnTrack_14:
		case kAudioUnitCustomProperty_NoteOnTrack_15:
		case kAudioUnitCustomProperty_NoteOnTrack_16:
		case kAudioUnitCustomProperty_MaxNoteTrack_1:
		case kAudioUnitCustomProperty_MaxNoteTrack_2:
		case kAudioUnitCustomProperty_MaxNoteTrack_3:
		case kAudioUnitCustomProperty_MaxNoteTrack_4:
		case kAudioUnitCustomProperty_MaxNoteTrack_5:
		case kAudioUnitCustomProperty_MaxNoteTrack_6:
		case kAudioUnitCustomProperty_MaxNoteTrack_7:
		case kAudioUnitCustomProperty_MaxNoteTrack_8:
		case kAudioUnitCustomProperty_MaxNoteTrack_9:
		case kAudioUnitCustomProperty_MaxNoteTrack_10:
		case kAudioUnitCustomProperty_MaxNoteTrack_11:
		case kAudioUnitCustomProperty_MaxNoteTrack_12:
		case kAudioUnitCustomProperty_MaxNoteTrack_13:
		case kAudioUnitCustomProperty_MaxNoteTrack_14:
		case kAudioUnitCustomProperty_MaxNoteTrack_15:
		case kAudioUnitCustomProperty_MaxNoteTrack_16:
			value = *((UInt32*)outDataPtr);
			break;
			
		case kAudioUnitCustomProperty_BRRData:
		case kAudioUnitCustomProperty_ProgramName:
		case kAudioUnitCustomProperty_PGDictionary:
		case kAudioUnitCustomProperty_XIData:
		case kAudioUnitCustomProperty_SourceFileRef:
		default:
			value = .0f;
	}
	return value;
}

//-----------------------------------------------------------------------------
float EfxAccess::GetParameter( int parameterId )
{
#if AU
	Float32		param;
	AudioUnitGetParameter(mAU, parameterId, kAudioUnitScope_Global, 0, &param);
	return param;
#else
	//VST時の処理
#endif
}

//-----------------------------------------------------------------------------
void EfxAccess::SetParam( void *sender, int index, float value )
{
#if AU
	AudioUnitParameter parameter = { mAU, index, kAudioUnitScope_Global, 0 };
	AUParameterSet(	mEventListener, sender, &parameter, value, 0);
	//AUParameterListenerNotify( mEventListener, this, &parameter );
#else
	//VST時の処理
#endif
}

//-----------------------------------------------------------------------------
void EfxAccess::SetProperty( int propertyID, float value )
{
#if AU
	double		doubleData = value;
	float		floatData = value;
	int			intData = value;
	bool		boolData = value>0.5f?true:false;
	void*		outDataPtr = NULL;
	UInt32		outDataSize = 0;
	
	switch (propertyID) {
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
		case kAudioUnitCustomProperty_EditingChannel:
		case kAudioUnitCustomProperty_LoopPoint:
		case kAudioUnitCustomProperty_Bank:
			outDataSize	= sizeof(int);
			outDataPtr	= (void*)&intData;
			break;
			
		case kAudioUnitCustomProperty_Rate:
			outDataSize = sizeof(double);
			outDataPtr = (void*)&doubleData;
			break;
			
		case kAudioUnitCustomProperty_Loop:
		case kAudioUnitCustomProperty_Echo:
			outDataSize = sizeof(bool);
			outDataPtr = (void*)&boolData;
			break;
			
		case kAudioUnitCustomProperty_BRRData:
			//別関数で処理
			break;
			
		case kAudioUnitCustomProperty_PGDictionary:
			//別関数で処理
			break;
			
		case kAudioUnitCustomProperty_XIData:
			//read only
			break;
			
		case kAudioUnitCustomProperty_ProgramName:
			//別関数で処理
			break;
			
		case kAudioUnitCustomProperty_TotalRAM:
			//read only
			break;
			
		case kAudioUnitCustomProperty_Band1:
		case kAudioUnitCustomProperty_Band2:
		case kAudioUnitCustomProperty_Band3:
		case kAudioUnitCustomProperty_Band4:
		case kAudioUnitCustomProperty_Band5:
			outDataSize = sizeof(Float32);
			outDataPtr = (void*)&floatData;
			break;
			
		case kAudioUnitCustomProperty_NoteOnTrack_1:
		case kAudioUnitCustomProperty_NoteOnTrack_2:
		case kAudioUnitCustomProperty_NoteOnTrack_3:
		case kAudioUnitCustomProperty_NoteOnTrack_4:
		case kAudioUnitCustomProperty_NoteOnTrack_5:	
		case kAudioUnitCustomProperty_NoteOnTrack_6:
		case kAudioUnitCustomProperty_NoteOnTrack_7:
		case kAudioUnitCustomProperty_NoteOnTrack_8:
		case kAudioUnitCustomProperty_NoteOnTrack_9:
		case kAudioUnitCustomProperty_NoteOnTrack_10:
		case kAudioUnitCustomProperty_NoteOnTrack_11:
		case kAudioUnitCustomProperty_NoteOnTrack_12:
		case kAudioUnitCustomProperty_NoteOnTrack_13:
		case kAudioUnitCustomProperty_NoteOnTrack_14:
		case kAudioUnitCustomProperty_NoteOnTrack_15:
		case kAudioUnitCustomProperty_NoteOnTrack_16:
		case kAudioUnitCustomProperty_MaxNoteTrack_1:
		case kAudioUnitCustomProperty_MaxNoteTrack_2:
		case kAudioUnitCustomProperty_MaxNoteTrack_3:
		case kAudioUnitCustomProperty_MaxNoteTrack_4:
		case kAudioUnitCustomProperty_MaxNoteTrack_5:
		case kAudioUnitCustomProperty_MaxNoteTrack_6:
		case kAudioUnitCustomProperty_MaxNoteTrack_7:
		case kAudioUnitCustomProperty_MaxNoteTrack_8:
		case kAudioUnitCustomProperty_MaxNoteTrack_9:
		case kAudioUnitCustomProperty_MaxNoteTrack_10:
		case kAudioUnitCustomProperty_MaxNoteTrack_11:
		case kAudioUnitCustomProperty_MaxNoteTrack_12:
		case kAudioUnitCustomProperty_MaxNoteTrack_13:
		case kAudioUnitCustomProperty_MaxNoteTrack_14:
		case kAudioUnitCustomProperty_MaxNoteTrack_15:
		case kAudioUnitCustomProperty_MaxNoteTrack_16:
			//read only
			break;
			
		case kAudioUnitCustomProperty_SourceFileRef:
			//別関数で処理
			break;
			
		case kAudioUnitCustomProperty_IsEmaphasized:
			//別関数で処理
			break;
			
		default:
			outDataPtr = NULL;
			outDataSize = 0;
	}
	
	if ( outDataPtr ) {
		AudioUnitSetProperty(mAU, propertyID, kAudioUnitScope_Global, 0, outDataPtr, outDataSize);
	}
#else
	//VST時の処理
#endif
}
