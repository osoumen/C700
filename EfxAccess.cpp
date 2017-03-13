/*
 *  EfxAccess.cpp
 *  C700
 *
 *  Created by osoumen on 12/10/08.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "PlayerCodeReader.h"
#include "EfxAccess.h"
#ifndef AU
#include "C700VST.h"
#endif

//-----------------------------------------------------------------------------
EfxAccess::EfxAccess( void *efx )
#if AU
: mAU((AudioUnit)efx)
, mEventListener(NULL)
#else
: mEfx((C700VST*)efx)
#endif
{
#if AU
    mLastGetBrr.data = NULL;
    mLastGetBrr.size = 0;
#endif
    createPropertyParamMap(mPropertyParams);
}

//-----------------------------------------------------------------------------
EfxAccess::~EfxAccess()
{
#if AU
    if (mLastGetBrr.data != NULL) {
        delete [] mLastGetBrr.data;
    }
#endif
}

//-----------------------------------------------------------------------------
bool	EfxAccess::CreateBRRFileData( RawBRRFile **outData )
{
	//エフェクタ側から現在のプログラムの情報を取得してRawBRRFileを作成
#if AU
	InstParams	inst;
    BRRData     brr;
	GetBRRData(&brr);
    inst.setBRRData(&brr);
	GetCStringProperty(kAudioUnitCustomProperty_ProgramName, inst.pgname, PROGRAMNAME_MAX_LEN);
	inst.ar = GetPropertyValue(kAudioUnitCustomProperty_AR);
	inst.dr = GetPropertyValue(kAudioUnitCustomProperty_DR);
	inst.sl = GetPropertyValue(kAudioUnitCustomProperty_SL);
	inst.sr = GetPropertyValue(kAudioUnitCustomProperty_SR);
	inst.volL = GetPropertyValue(kAudioUnitCustomProperty_VolL);
	inst.volR = GetPropertyValue(kAudioUnitCustomProperty_VolR);
	inst.rate = GetPropertyValue(kAudioUnitCustomProperty_Rate);
	inst.basekey= GetPropertyValue(kAudioUnitCustomProperty_BaseKey);
	inst.lowkey = GetPropertyValue(kAudioUnitCustomProperty_LowKey);
	inst.highkey = GetPropertyValue(kAudioUnitCustomProperty_HighKey);
	inst.lp = GetPropertyValue(kAudioUnitCustomProperty_LoopPoint);
	inst.loop = GetPropertyValue(kAudioUnitCustomProperty_Loop)!=0?true:false;
	inst.echo = GetPropertyValue(kAudioUnitCustomProperty_Echo)!=0?true:false;
	inst.bank = GetPropertyValue(kAudioUnitCustomProperty_Bank);
    inst.sustainMode = GetPropertyValue(kAudioUnitCustomProperty_SustainMode)!=0?true:false;
    inst.monoMode = GetPropertyValue(kAudioUnitCustomProperty_MonoMode)!=0?true:false;
    inst.portamentoOn = GetPropertyValue(kAudioUnitCustomProperty_PortamentoOn)!=0?true:false;
    inst.portamentoRate = GetPropertyValue(kAudioUnitCustomProperty_PortamentoRate);
    inst.noteOnPriority = GetPropertyValue(kAudioUnitCustomProperty_NoteOnPriority);
    inst.releasePriority = GetPropertyValue(kAudioUnitCustomProperty_ReleasePriority);
	inst.isEmphasized = GetPropertyValue(kAudioUnitCustomProperty_IsEmaphasized)!=0?true:false;
	GetFilePathProperty(kAudioUnitCustomProperty_SourceFileRef,inst.sourceFile,PATH_LEN_MAX);
	
	RawBRRFile	*file = new RawBRRFile(NULL,true);
	file->StoreInst(&inst);
	*outData = file;
	return true;
#else
	//VST時の処理
	int	editProg = GetPropertyValue(kAudioUnitCustomProperty_EditingProgram);
	const InstParams	*inst = mEfx->mEfx->GetVP();
	if ( inst ) {
		RawBRRFile	*file = new RawBRRFile(NULL,true);
		file->StoreInst(&inst[editProg]);
		*outData = file;
		return true;
	}
#endif
	return false;
}

//-----------------------------------------------------------------------------
bool	EfxAccess::CreateXIFileData( XIFile **outData )
{
#if AU
	XIFile	*file = NULL;
	
	//AU内部で生成されたデータを取得する
	//問題が無ければVST時と同じでも良いかも？
	//AU内部でやっていることもほぼ同じ
	CFDataRef	cfdata;
	UInt32 size = sizeof(CFDataRef);
	if (
		AudioUnitGetProperty(mAU,kAudioUnitCustomProperty_XIData,
							 kAudioUnitScope_Global, 0, &cfdata, &size)
		== noErr )
	{
		if ( cfdata ) {
            file = new XIFile(NULL, CFDataGetLength(cfdata));
            file->writeData(CFDataGetBytePtr(cfdata), CFDataGetLength(cfdata));
			CFRelease(cfdata);
            *outData = file;
			return true;
		}
		else {
			return false;
		}
	}
    *outData = file;
	return true;
#else
	//VST時の処理
	//ホスト側からテンポを取得
	double	tempo = 125.0;
	VstTimeInfo*	info = mEfx->getTimeInfo(kVstTempoValid);
	if ( info ) {
		tempo = info->tempo;
	}
	
	XIFile	*file = new XIFile(NULL);
	file->SetDataFromChip( mEfx->mEfx->GetDriver(),
							 mEfx->mEfx->GetPropertyValue(kAudioUnitCustomProperty_EditingProgram),
							 tempo );
	if ( file->IsLoaded() ) {
		*outData = file;
		return true;
	}
	*outData = NULL;
	delete file;
	return false;
#endif
}

//-----------------------------------------------------------------------------
bool	EfxAccess::CreatePlistBRRFileData( PlistBRRFile **outData )
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
bool EfxAccess::SetFilePathProperty( int propertyId, const char *path )
{
	if ( strlen(path) == 0 ) return false;
#if AU
	UInt32		inSize = sizeof(CFStringRef);
	CFURLRef	url = CFURLCreateFromFileSystemRepresentation(NULL, (UInt8*)path, strlen(path), false);

	if (
		AudioUnitSetProperty(mAU, propertyId,
							 kAudioUnitScope_Global, 0, &url, inSize)
		== noErr ) {
		CFRelease( url );
		return true;
	}
	CFRelease( url );
	return false;
#else
	//VST時の処理
	mEfx->mEfx->SetPropertyPtrValue(propertyId, path, 0);
	return true;
#endif
}

//-----------------------------------------------------------------------------
bool EfxAccess::GetFilePathProperty( int propertyId, char *path, int maxLen )
{
#if AU
	CFURLRef	url;
	UInt32		outSize = sizeof(CFURLRef);
	
	//データを取得する
	if (
		AudioUnitGetProperty(mAU,propertyId,
							 kAudioUnitScope_Global, 0, &url, &outSize)
		== noErr )
	{
		if ( url ) {
			CFStringRef pathStr = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
			CFStringGetCString(pathStr, path, maxLen-1, kCFStringEncodingUTF8);
			CFRelease(pathStr);
			CFRelease(url);
			return true;
		}
		else {
			path[0] = 0;
		}
	}
	return false;
#else
	//VST時の処理
	const char	*outpath = (char*)mEfx->mEfx->GetPropertyPtrValue(propertyId);
	if ( outpath ) {
		strncpy(path, outpath, maxLen-1);
		return true;
	}
	return false;
#endif
}

//-----------------------------------------------------------------------------
bool EfxAccess::SetCStringProperty( int propertyId, const char *string )
{
#if AU
	
	UInt32		inSize = sizeof(CFStringRef);
	CFStringRef	pgnameRef = CFStringCreateWithCString(NULL, string, kCFStringEncodingUTF8);
	
	if (
		AudioUnitSetProperty(mAU, propertyId, kAudioUnitScope_Global, 0, &pgnameRef, inSize)
		== noErr ) {
		CFRelease( pgnameRef );
		return true;
	}
	CFRelease( pgnameRef );
	return false;
#else
	//VST時の処理
	mEfx->mEfx->SetPropertyPtrValue( propertyId, string, 0 );
	mEfx->PropertyNotifyFunc(propertyId, mEfx);
	return true;
#endif
}

//-----------------------------------------------------------------------------
bool EfxAccess::GetCStringProperty( int propertyId, char *string, int maxLen )
{
#if AU
	
	UInt32		outSize = sizeof(CFStringRef);
	CFStringRef	pgnameRef;
	
	if (
		AudioUnitGetProperty(mAU, propertyId, kAudioUnitScope_Global, 0, &pgnameRef, &outSize)
		== noErr ) {
		if ( pgnameRef ) {
			CFStringGetCString(pgnameRef, string, maxLen-1, kCFStringEncodingUTF8);
			CFRelease(pgnameRef);
			return true;
		}
		else {
			string[0] = 0;
		}
	}
	return false;
#else
	//VST時の処理
	const char *outpgname = (char*)mEfx->mEfx->GetPropertyPtrValue(propertyId);
	if ( outpgname ) {
		strncpy(string, outpgname, maxLen-1);
		return true;
	}
	return false;
#endif
}

//-----------------------------------------------------------------------------
bool EfxAccess::GetBRRData( BRRData *data )
{
#if AU
	UInt32		outSize = sizeof(CFDataRef);
    CFDataRef   out;
	if (
	AudioUnitGetProperty(mAU, kAudioUnitCustomProperty_BRRData, kAudioUnitScope_Global, 0, &out, &outSize)
		== noErr ) {
        if (mLastGetBrr.data != NULL) {
            delete [] mLastGetBrr.data;
        }
        if ((CFDataGetLength(out) == 0) || (CFDataGetBytePtr(out) == NULL)) {
            mLastGetBrr.data = NULL;
            mLastGetBrr.size = 0;
        }
        else {
            mLastGetBrr.data = new unsigned char[CFDataGetLength(out)];
            mLastGetBrr.size = CFDataGetLength(out);
            memcpy(mLastGetBrr.data, CFDataGetBytePtr(out), mLastGetBrr.size);
        }
        CFRelease(out);
        *data = mLastGetBrr;
		return true;
	}
	return false;
#else
	//VST時の処理
	const BRRData	*brr = mEfx->mEfx->GetBRRData();
	if ( brr ) {
		*data = *brr;
		return true;
	}
	return true;
#endif
}

//-----------------------------------------------------------------------------
bool EfxAccess::SetBRRData( const BRRData *data )
{
#if AU
	UInt32		inSize = sizeof(CFDataRef);
    CFDataRef   brr = CFDataCreate(NULL, data->data, data->size);
	if (
		AudioUnitSetProperty(mAU, kAudioUnitCustomProperty_BRRData, kAudioUnitScope_Global, 0, &brr, inSize)
		== noErr ) {
        CFRelease(brr);
		return true;
	}
    CFRelease(brr);
	return false;
#else
	//VST時の処理
	mEfx->mEfx->SetBRRData(data->data, data->size);
	mEfx->PropertyNotifyFunc(kAudioUnitCustomProperty_BRRData, mEfx);
	return true;
#endif
}

//-----------------------------------------------------------------------------
double EfxAccess::GetPropertyValue( int propertyId )
{
#if AU
	double		value = .0;
	char		outDataPtr[16];
	UInt32		outDataSize=16;
	
	if (
        propertyId != kAudioUnitCustomProperty_ProgramName &&
		propertyId != kAudioUnitCustomProperty_PGDictionary &&
		propertyId != kAudioUnitCustomProperty_XIData &&
		propertyId != kAudioUnitCustomProperty_SourceFileRef ) {
		AudioUnitGetProperty(mAU, propertyId, kAudioUnitScope_Global, 0, &outDataPtr, &outDataSize);
	}
	
    auto it = mPropertyParams.find(propertyId);
    if (it != mPropertyParams.end()) {
        switch (it->second.dataType) {
            case propertyDataTypeFloat32:
                value = *((Float32*)outDataPtr);
                break;
            case propertyDataTypeDouble:
                value = *((double*)outDataPtr);
                break;
            case propertyDataTypeInt32:
                value = *((int*)outDataPtr);
                break;
            case propertyDataTypeBool:
                value = *((bool*)outDataPtr);
                break;
            case propertyDataTypeStruct:
            case propertyDataTypeString:
            case propertyDataTypeFilePath:
            case propertyDataTypeVariableData:
            case propertyDataTypePointer:
                break;
        }
    }
	return value;
#else
	//VST時の処理
    switch (mPropertyParams[propertyId].dataType) {
        case propertyDataTypeFloat32:
        case propertyDataTypeInt32:
        case propertyDataTypeBool:
            return mEfx->mEfx->GetPropertyValue(propertyId);
        case propertyDataTypeDouble:
            return mEfx->mEfx->GetPropertyDoubleValue(propertyId);
        case propertyDataTypeStruct:
        case propertyDataTypeString:
        case propertyDataTypeFilePath:
        case propertyDataTypeVariableData:
        case propertyDataTypePointer:
            break;
    }
	return 0;
#endif
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
	float	param = mEfx->getParameter(parameterId);
	return mEfx->expandParam( parameterId, param );
#endif
}

//-----------------------------------------------------------------------------
void EfxAccess::SetParameter( void *sender, int index, float value )
{
#if AU
	AudioUnitParameter parameter = { mAU, index, kAudioUnitScope_Global, 0 };
	AUParameterSet(	mEventListener, sender, &parameter, value, 0);
	//AUParameterListenerNotify( mEventListener, this, &parameter );
#else
	//VST時の処理
	mEfx->setParameter(index, mEfx->shrinkParam( index, value ) );
#endif
}

//-----------------------------------------------------------------------------
void EfxAccess::SetPropertyValue( int propertyID, double value )
{
#if AU
	double		doubleData = value;
	float		floatData = value;
	int			intData = value;
	bool		boolData = value>0.5f?true:false;
	void*		outDataPtr = NULL;
	UInt32		outDataSize = 0;
	
    auto it = mPropertyParams.find(propertyID);
    if (it != mPropertyParams.end()) {
        if (it->second.readOnly == false) {
            switch (it->second.dataType) {
                case propertyDataTypeFloat32:
                    outDataSize = sizeof(Float32);
                    outDataPtr = (void*)&floatData;
                    break;
                case propertyDataTypeDouble:
                    outDataSize = sizeof(double);
                    outDataPtr = (void*)&doubleData;
                    break;
                case propertyDataTypeInt32:
                    outDataSize	= sizeof(int);
                    outDataPtr	= (void*)&intData;
                    break;
                case propertyDataTypeBool:
                    outDataSize = sizeof(bool);
                    outDataPtr = (void*)&boolData;
                    break;
                case propertyDataTypeStruct:
                case propertyDataTypeString:
                case propertyDataTypeFilePath:
                case propertyDataTypeVariableData:
                case propertyDataTypePointer:
                    outDataPtr = NULL;
                    outDataSize = 0;
                    break;
            }
        }
    }
	if ( outDataPtr ) {
		AudioUnitSetProperty(mAU, propertyID, kAudioUnitScope_Global, 0, outDataPtr, outDataSize);
	}
#else
	//VST時の処理
    switch (mPropertyParams[propertyID].dataType) {
        case propertyDataTypeFloat32:
        case propertyDataTypeInt32:
        case propertyDataTypeBool:
            mEfx->mEfx->SetPropertyValue(propertyID, value);
            break;
        case propertyDataTypeDouble:
            mEfx->mEfx->SetPropertyDoubleValue(propertyID, value);
            break;
        case propertyDataTypeStruct:
        case propertyDataTypeString:
        case propertyDataTypeFilePath:
        case propertyDataTypeVariableData:
        case propertyDataTypePointer:
            break;
    }
	mEfx->PropertyNotifyFunc(propertyID, mEfx);
#endif
}

//-----------------------------------------------------------------------------
bool EfxAccess::LoadSongPlayerCode( const char *path )
{
    DataBuffer codeFile(path);
    if (codeFile.GetDataUsed() <= 1) {
        return false;
    }
    
#if AU
    CFDataRef   dataRef = CFDataCreate(NULL, codeFile.GetDataPtr(), codeFile.GetDataSize());
    UInt32      inSize = sizeof(CFDataRef);
    if (
		AudioUnitSetProperty(mAU, kAudioUnitCustomProperty_SongPlayerCode,
							 kAudioUnitScope_Global, 0, &dataRef, inSize)
		== noErr ) {
        CFRelease(dataRef);
		return true;
	}
    CFRelease(dataRef);
	return false;
#else
    mEfx->mEfx->SetPropertyPtrValue(kAudioUnitCustomProperty_SongPlayerCode, codeFile.GetDataPtr(), codeFile.GetDataSize());
    return true;
#endif
}

