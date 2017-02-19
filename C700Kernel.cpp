/*
 *  C700Kernel.cpp
 *  C700
 *
 *  Created by osoumen on 12/10/12.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "XIFile.h"
#include "C700Kernel.h"
#include "C700Properties.h"
#include "samplebrr.h"
#include <math.h>

void CalcFIRParam( const float band[5], int filter[8] );

//-----------------------------------------------------------------------------
C700Kernel::C700Kernel()
: C700Parameters(kNumberOfParameters)
, propertyNotifyFunc(NULL)
, propNotifyUserData(NULL)
, parameterSetFunc(NULL)
, paramSetUserData(NULL)
{
	for ( int i=0; i<kNumberOfParameters; i++ ) {
		SetParameter(i, GetParameterDefault(i));
	}

	for (int i=0; i<5; i++) {
		mFilterBand[i] = 1.0f;
	}
	
	mEditProg = 0;
	mEditChannel = 0;
    
    mIsHwAvailable = false;
	
	// ノートオンインジケータ初期化
	for ( int i=0; i<16; i++ ) {
		mOnNotes[i] = 0;
		mMaxNote[i] = 0;
	}
	
	//プログラムの初期化
	for (int i=0; i<128; i++) {
		mVPset[i].pgname[0] = 0;
		//mVPset[i].releaseBrr();   // BRRDataのコンストラクタで初期化される
		mVPset[i].basekey = 0;
		mVPset[i].lowkey = 0;
		mVPset[i].highkey = 0;
		mVPset[i].loop = false;
		mVPset[i].echo = false;
		mVPset[i].bank = 0;
		mVPset[i].lp = 0;
		mVPset[i].rate = 32000.0f;
		mVPset[i].volL = 100;
		mVPset[i].volR = 100;
		
		mVPset[i].sourceFile[0] = 0;
		mVPset[i].isEmphasized = true;
		
		mVPset[i].ar = kDefaultValue_AR;
		mVPset[i].dr = kDefaultValue_DR;
		mVPset[i].sl = kDefaultValue_SL;
		mVPset[i].sr = kDefaultValue_SR;
        
        mVPset[i].sustainMode = kDefaultValue_SustainMode;
        mVPset[i].monoMode = kDefaultValue_MonoMode;
        mVPset[i].portamentoOn = kDefaultValue_PortamentoOn;
        mVPset[i].portamentoRate = kDefaultValue_PortamentoRate;
        mVPset[i].noteOnPriority = kDefaultValue_NoteOnPriority;
        mVPset[i].releasePriority = kDefaultValue_ReleasePriority;
	}
	
	// 音源にプログラムのメモリを渡す
	mDriver.SetVPSet(mVPset);
}

//-----------------------------------------------------------------------------
C700Kernel::~C700Kernel()
{
	for (int i=0; i<128; i++) {
		mVPset[i].pgname[0] = 0;
		mVPset[i].sourceFile[0] = 0;
	}	
}

//-----------------------------------------------------------------------------

void C700Kernel::Reset()
{
	//音源リセット
	mDriver.Reset();
	
	for ( int i=0; i<16; i++ ) {
        //プラグイン状態のリセット
        mDataEntryValue[i] = 0;
        mRPN[i] = 0x7f7f;
        mNRPN[i] = 0x7f7f;
        mIsSettingNRPN[i] = false;

        // MIDIインジケータをリセット
		mOnNotes[i] = 0;
		mMaxNote[i] = 0;
		if ( propertyNotifyFunc ) {
			propertyNotifyFunc( kAudioUnitCustomProperty_NoteOnTrack_1+i, propNotifyUserData );
			propertyNotifyFunc( kAudioUnitCustomProperty_MaxNoteTrack_1+i, propNotifyUserData );
		}
	}
}

//-----------------------------------------------------------------------------
void C700Kernel::SetTempo( double tempo )
{
	mTempo = tempo;
}

//-----------------------------------------------------------------------------

bool C700Kernel::SetParameter( int id, float value )
{
	C700Parameters::SetParameter( id, value );
	
	switch ( id ) {
		case kParam_poly:
			mDriver.SetVoiceLimit(value);
			break;
			
		case kParam_mainvol_L:
			mDriver.SetMainVol_L(value);
			break;
			
		case kParam_mainvol_R:
			mDriver.SetMainVol_R(value);
			break;
			
		case kParam_vibdepth:
			mDriver.ModWheel(0, value);
			break;
			
		case kParam_vibrate:
			mDriver.SetVibFreq(-1, value);   // -1が全チャンネルを表す
			break;
			
		case kParam_vibdepth2:
			mDriver.SetVibDepth(-1, value);  // -1は全チャンネルを表す
			break;
			
		case kParam_velocity:
			switch ( (int)value ) {
				case 0:
					mDriver.SetVelocityMode( kVelocityMode_Constant );
					break;
				case 1:
					mDriver.SetVelocityMode( kVelocityMode_Square );
					break;
				case 2:
					mDriver.SetVelocityMode( kVelocityMode_Linear );
					break;
				default:
					mDriver.SetVelocityMode( kVelocityMode_Square );
			}
			break;
			
		case kParam_bendrange:
			mDriver.SetPBRange(value);
			break;
			
		case kParam_program:
			mDriver.ProgramChange(0, value, 0);
			break;
			
		case kParam_engine:
            switch ( (int)value ) {
                case 0:
                    mDriver.SetEngineType(kEngineType_Old);
                    break;
                case 1:
                    mDriver.SetEngineType(kEngineType_Relaxed);
                    break;
                case 2:
                    mDriver.SetEngineType(kEngineType_Accurate);
                    break;
            }
			break;
			
		case kParam_bankAmulti:
			mDriver.SetMultiMode(0, value==0 ? false:true);
			break;
			

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
			mDriver.ProgramChange( id - kParam_program_2 + 1, value, 0 );
			break;
			
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
			mDriver.ModWheel( id - kParam_vibdepth_2 + 1, value);
			break;
			
		case kParam_echovol_L:
			mDriver.SetEchoVol_L( value );
			break;
			
		case kParam_echovol_R:
			mDriver.SetEchoVol_R( value );
			break;
			
		case kParam_echoFB:
			mDriver.SetFeedBackLevel(value);
			break;
			
		case kParam_echodelay:
			mDriver.SetDelayTime(value);
			break;
			
		case kParam_fir0:
		case kParam_fir1:
		case kParam_fir2:
		case kParam_fir3:
		case kParam_fir4:
		case kParam_fir5:
		case kParam_fir6:
		case kParam_fir7:
			mDriver.SetFIRTap( id - kParam_fir0, value );
			break;
			
		case kParam_bankBmulti:
			mDriver.SetMultiMode(1, value==0 ? false:true);
			break;
			
		case kParam_bankCmulti:
			mDriver.SetMultiMode(2, value==0 ? false:true);
			break;
			
		case kParam_bankDmulti:
			mDriver.SetMultiMode(3, value==0 ? false:true);
			break;
			
        case kParam_alwaysDelayNote:
            mDriver.SetEventDelayClocks(value==0 ? 0:8192);
            break;
            
        case kParam_voiceAllocMode:
            // voiceAllocモードを設定
            switch ( (int)value ) {
                case 0:
                    mDriver.SetVoiceAllocMode(kVoiceAllocMode_Oldest);
                    break;
                case 1:
                    mDriver.SetVoiceAllocMode(kVoiceAllocMode_SameChannel);
                    break;
            }
            break;
            
        case kParam_fastReleaseAsKeyOff:
            mDriver.SetFastReleaseAsKeyOff(value==0 ? false:true);
            break;
            
		default:
			return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
float C700Kernel::GetParameter( int id )
{
	return C700Parameters::GetParameter( id );
}

//-----------------------------------------------------------------------------

float C700Kernel::GetPropertyValue( int inID )
{
	switch (inID) {
		case kAudioUnitCustomProperty_Rate:
			return mVPset[mEditProg].rate;
			
		case kAudioUnitCustomProperty_BaseKey:
			return mVPset[mEditProg].basekey;
		case kAudioUnitCustomProperty_LowKey:
			return mVPset[mEditProg].lowkey;
		case kAudioUnitCustomProperty_HighKey:
			return mVPset[mEditProg].highkey;
			
		case kAudioUnitCustomProperty_LoopPoint:
			return mVPset[mEditProg].lp;
			
		case kAudioUnitCustomProperty_Loop:
			return mVPset[mEditProg].loop ? 1.0f:.0f;
			
		case kAudioUnitCustomProperty_Echo:
			return mVPset[mEditProg].echo ? 1.0f:.0f;
			
		case kAudioUnitCustomProperty_Bank:
			return mVPset[mEditProg].bank;
			
		case kAudioUnitCustomProperty_AR:
			return mVPset[mEditProg].ar;
			
		case kAudioUnitCustomProperty_DR:
			return mVPset[mEditProg].dr;
			
		case kAudioUnitCustomProperty_SL:
			return mVPset[mEditProg].sl;
			
		case kAudioUnitCustomProperty_SR:
			return mVPset[mEditProg].sr;
			
		case kAudioUnitCustomProperty_VolL:
			return mVPset[mEditProg].volL;
			
		case kAudioUnitCustomProperty_VolR:
			return mVPset[mEditProg].volR;
			
		case kAudioUnitCustomProperty_EditingProgram:
			return mEditProg;
			
		case kAudioUnitCustomProperty_EditingChannel:
			return mEditChannel;
			
		case kAudioUnitCustomProperty_TotalRAM:
			return GetTotalRAM();
			
			//エコー
		case kAudioUnitCustomProperty_Band1:
		case kAudioUnitCustomProperty_Band2:
		case kAudioUnitCustomProperty_Band3:
		case kAudioUnitCustomProperty_Band4:
		case kAudioUnitCustomProperty_Band5:
			return mFilterBand[inID-kAudioUnitCustomProperty_Band1];
			
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
			return mOnNotes[inID-kAudioUnitCustomProperty_NoteOnTrack_1];
			
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
			return mMaxNote[inID-kAudioUnitCustomProperty_MaxNoteTrack_1];
			
		case kAudioUnitCustomProperty_IsEmaphasized:
			return mVPset[mEditProg].isEmphasized ? 1.0f:.0f;
            
        case kAudioUnitCustomProperty_SustainMode:
            return mVPset[mEditProg].sustainMode ? 1.0f:.0f;
            
        case kAudioUnitCustomProperty_MonoMode:
            return mVPset[mEditProg].monoMode ? 1.0f:.0f;
            
        case kAudioUnitCustomProperty_PortamentoOn:
            return mVPset[mEditProg].portamentoOn ? 1.0f:.0f;
            
        case kAudioUnitCustomProperty_PortamentoRate:
            return mVPset[mEditProg].portamentoRate;
            
        case kAudioUnitCustomProperty_NoteOnPriority:
            return mVPset[mEditProg].noteOnPriority;
            
        case kAudioUnitCustomProperty_ReleasePriority:
            return mVPset[mEditProg].releasePriority;
            
        case kAudioUnitCustomProperty_IsHwConnected:
            return mDriver.IsHwAvailable() ? 1.0f:.0f;
            
		default:
			return 0;
	}	
}

//-----------------------------------------------------------------------------
void *C700Kernel::GetPropertyPtrValue( int inID )
{
    switch (inID) {
#if AU
        case kAudioUnitCustomProperty_SourceFileRef:
        {
            const char *srcPath = GetSourceFilePath();
            CFURLRef	url =
            CFURLCreateFromFileSystemRepresentation(NULL, (UInt8*)srcPath, strlen(srcPath), false);
            return (void *)url;	//使用後要release
        }
		case kAudioUnitCustomProperty_ProgramName:
        {
            CFStringRef	str = CFStringCreateWithCString(NULL, GetProgramName(), kCFStringEncodingUTF8);
            return (void *)str;	//使用後要release
        }
		case kAudioUnitCustomProperty_PGDictionary:
        {
            CFDictionaryRef	pgdata;
            int				editProg = GetPropertyValue(kAudioUnitCustomProperty_EditingProgram);
            CreatePGDataDic(&pgdata, editProg);
            return (void *)pgdata;	//使用後要release
        }
		case kAudioUnitCustomProperty_XIData:
        {
            XIFile	fileData(NULL);
            
            fileData.SetDataFromChip(GetGenerator(),
                                     GetPropertyValue(kAudioUnitCustomProperty_EditingProgram), mTempo);
            
            if ( fileData.IsLoaded() ) {
                CFDataRef xidata;
                xidata = CFDataCreate(NULL, fileData.GetDataPtr(), fileData.GetDataSize() );
                return (void *)xidata;	//使用後要release
            }
            else {
                return NULL;
            }
        }
#endif
        default:
			return 0;
    }
}

//-----------------------------------------------------------------------------
bool C700Kernel::GetPropertyStructValue( int inID, void *outData )
{
    switch (inID) {
#if AU
		case kAudioUnitCustomProperty_BRRData:
            *((BRRData *)outData) = *(GetBRRData());
            return true;
#endif
        default:
			return false;
    }
}

//-----------------------------------------------------------------------------

bool C700Kernel::SetPropertyValue( int inID, float value )
{
	bool		boolData = value>0.5f?true:false;
	
	switch (inID) {
		case kAudioUnitCustomProperty_Rate:
			mVPset[mEditProg].rate = value;
			return true;
			
		case kAudioUnitCustomProperty_BaseKey:
			mVPset[mEditProg].basekey = value;
			mDriver.RefreshKeyMap();
			return true;
		case kAudioUnitCustomProperty_LowKey:
			mVPset[mEditProg].lowkey = value;
			mDriver.RefreshKeyMap();
			return true;
		case kAudioUnitCustomProperty_HighKey:
			mVPset[mEditProg].highkey = value;
			mDriver.RefreshKeyMap();
			return true;
			
		case kAudioUnitCustomProperty_LoopPoint:
			mVPset[mEditProg].lp = value;
            /*
			if (mVPset[mEditProg].lp > mVPset[mEditProg].brrSize()) {
				mVPset[mEditProg].lp = mVPset[mEditProg].brrSize();
			}*/
			if ( mVPset[mEditProg].lp < 0 ) {
				mVPset[mEditProg].lp = 0;
			}
            mDriver.UpdateLoopPoint(mEditProg);
			return true;
			
		case kAudioUnitCustomProperty_Loop:
			mVPset[mEditProg].loop = boolData;
            if (boolData) {
                mVPset[mEditProg].setLoop();
            }
            else {
                mVPset[mEditProg].unsetLoop();
            }
            mDriver.UpdateLoopFlag(mEditProg);
			return true;
			
		case kAudioUnitCustomProperty_Echo:
			mVPset[mEditProg].echo = boolData;
			return true;
			
		case kAudioUnitCustomProperty_Bank:
			mVPset[mEditProg].bank = value;
            mDriver.RefreshKeyMap();
			return true;
			
		case kAudioUnitCustomProperty_AR:
			mVPset[mEditProg].ar = value;
			return true;
			
		case kAudioUnitCustomProperty_DR:
			mVPset[mEditProg].dr = value;
			return true;
			
		case kAudioUnitCustomProperty_SL:
			mVPset[mEditProg].sl = value;
			return true;
			
		case kAudioUnitCustomProperty_SR:
			mVPset[mEditProg].sr = value;
			return true;
			
		case kAudioUnitCustomProperty_VolL:
			mVPset[mEditProg].volL = value;
			return true;
			
		case kAudioUnitCustomProperty_VolR:
			mVPset[mEditProg].volR = value;
			return true;
			
		case kAudioUnitCustomProperty_EditingProgram:
		{
			mEditProg = value;
			if (mEditProg>127) mEditProg=127;
			if (mEditProg<0) mEditProg=0;
			// 選択チャンネルのプログラムをチェンジ
			if ( parameterSetFunc ) {
				if ( mEditChannel == 0 ) {
					parameterSetFunc(kParam_program, mEditProg, paramSetUserData );
				}
				else {
					parameterSetFunc(kParam_program_2 + mEditChannel - 1, mEditProg, paramSetUserData );
				}
			}
			
			// 表示更新が必要なプロパティの変更を通知する
			if ( propertyNotifyFunc ) {
				for (int i=kAudioUnitCustomProperty_ProgramName; i<kAudioUnitCustomProperty_TotalRAM; i++) {
					propertyNotifyFunc( i, propNotifyUserData );
				}
                for (int i=kAudioUnitCustomProperty_SustainMode; i<=kAudioUnitCustomProperty_ReleasePriority; i++) {
					propertyNotifyFunc( i, propNotifyUserData );
				}
			}
			return true;
		}
			
		case kAudioUnitCustomProperty_EditingChannel:
		{
			mEditChannel = value;
			if (mEditChannel>15) mEditChannel=15;
			if (mEditChannel<0) mEditChannel=0;
			// 変更したチャンネルのプログラムチェンジを取得してmEditProgに設定する
			if ( parameterSetFunc ) {
				if ( mEditChannel == 0 ) {
					mEditProg = GetParameter(kParam_program);
				}
				else {
					mEditProg = GetParameter(kParam_program_2 + mEditChannel - 1);
				}
			}
			
			// 表示更新が必要なプロパティの変更を通知する
			if ( propertyNotifyFunc ) {
				for (int i=kAudioUnitCustomProperty_ProgramName; i<kAudioUnitCustomProperty_TotalRAM; i++) {
					propertyNotifyFunc( i, propNotifyUserData );
				}
                for (int i=kAudioUnitCustomProperty_SustainMode; i<=kAudioUnitCustomProperty_ReleasePriority; i++) {
					propertyNotifyFunc( i, propNotifyUserData );
				}
			}
			return true;
		}
			
			//エコー
		case kAudioUnitCustomProperty_Band1:
		case kAudioUnitCustomProperty_Band2:
		case kAudioUnitCustomProperty_Band3:
		case kAudioUnitCustomProperty_Band4:
		case kAudioUnitCustomProperty_Band5:
		{
			int		filter[8];
			mFilterBand[inID-kAudioUnitCustomProperty_Band1] = value;
			CalcFIRParam(mFilterBand, filter);
			if ( parameterSetFunc ) {
				for ( int i=0; i<8; i++ ) {
					parameterSetFunc(kParam_fir0+i, filter[i], paramSetUserData);
				}
			}
			return true;
		}
			
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
			return true;
			
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
			mMaxNote[inID-kAudioUnitCustomProperty_MaxNoteTrack_1] = value;
			mOnNotes[inID-kAudioUnitCustomProperty_NoteOnTrack_1] = 0;
			return true;
			
		case kAudioUnitCustomProperty_IsEmaphasized:
			mVPset[mEditProg].isEmphasized = boolData;
			return true;
			
        case kAudioUnitCustomProperty_SustainMode:
            mVPset[mEditProg].sustainMode = boolData;
            return true;
            
        case kAudioUnitCustomProperty_MonoMode:
            mVPset[mEditProg].monoMode = boolData;
            return true;

        case kAudioUnitCustomProperty_PortamentoOn:
            mVPset[mEditProg].portamentoOn = boolData;
            return true;
            
        case kAudioUnitCustomProperty_PortamentoRate:
            mVPset[mEditProg].portamentoRate = value;
            mDriver.UpdatePortamentoTime(mEditProg);
            return true;

        case kAudioUnitCustomProperty_NoteOnPriority:
            mVPset[mEditProg].noteOnPriority = value;
            mDriver.AllNotesOff();
            return true;
            
        case kAudioUnitCustomProperty_ReleasePriority:
            mVPset[mEditProg].releasePriority = value;
            mDriver.AllNotesOff();
            return true;
            
        case kAudioUnitCustomProperty_IsHwConnected:
            return true;

		default:
			return false;
	}
}

bool C700Kernel::SetPropertyPtrValue( int inID, const void *inPtr )
{
    switch (inID) {
#if AU
        case kAudioUnitCustomProperty_SourceFileRef:
        {
            CFStringRef pathStr = CFURLCopyFileSystemPath(reinterpret_cast<CFURLRef>(inPtr), kCFURLPOSIXPathStyle);
            char		path[PATH_LEN_MAX];
            CFStringGetCString(pathStr, path, PATH_LEN_MAX-1, kCFStringEncodingUTF8);
            CFRelease(pathStr);
            SetSourceFilePath(path);
            return true;
        }
		case kAudioUnitCustomProperty_BRRData:
        {
            const BRRData *brr = reinterpret_cast<const BRRData *>(inPtr);
            SetBRRData(brr->data, brr->size);
            return true;
        }
		case kAudioUnitCustomProperty_ProgramName:
        {
            char	pgname[PROGRAMNAME_MAX_LEN];
            CFStringGetCString(reinterpret_cast<CFStringRef>(inPtr), pgname, PROGRAMNAME_MAX_LEN-1, kCFStringEncodingUTF8);
            SetProgramName(pgname);
            return true;
        }
		case kAudioUnitCustomProperty_PGDictionary:
        {
            CFDictionaryRef	pgdata = reinterpret_cast<CFDictionaryRef>(inPtr);
            int				editProg = GetPropertyValue(kAudioUnitCustomProperty_EditingProgram);
            RestorePGDataDic(pgdata, editProg);
            return true;
        }
		case kAudioUnitCustomProperty_XIData:
#endif
        default:
			return false;
    }
}
//-----------------------------------------------------------------------------
void CalcFIRParam( const float band[5], int filter[8] )
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
	
	for (int j=0; j<8; j++) {
		float temp = 0;
		for (int i=0; i<5; i++) {
			temp += band[i] * bandfactor[j][i];
		}
		if (temp < 0) {
			temp = ceil(temp*127);
		}
		else {
			temp = floor(temp*127);
		}
		filter[j] = temp;
	}
}
//-----------------------------------------------------------------------------

const char *C700Kernel::GetSourceFilePath()
{
	return mVPset[mEditProg].sourceFile;
}

//-----------------------------------------------------------------------------

bool C700Kernel::SetSourceFilePath( const char *path )
{
	strncpy(mVPset[mEditProg].sourceFile, path, PATH_LEN_MAX-1);
	return true;
}

//-----------------------------------------------------------------------------

const char *C700Kernel::GetProgramName()
{
	return mVPset[mEditProg].pgname;
}

//-----------------------------------------------------------------------------

bool C700Kernel::SetProgramName( const char *pgname )
{
	strncpy(mVPset[mEditProg].pgname, pgname, PROGRAMNAME_MAX_LEN);
	return true;
}

//-----------------------------------------------------------------------------

const BRRData *C700Kernel::GetBRRData()
{
	return mVPset[mEditProg].getBRRData();
}

//-----------------------------------------------------------------------------

const BRRData *C700Kernel::GetBRRData(int prog)
{
	return mVPset[prog].getBRRData();
}

//-----------------------------------------------------------------------------

bool C700Kernel::SetBRRData( const unsigned char *data, int size, int prog, bool reset, bool notify )
{
    if (prog < 0 || prog > 127) {
        prog = mEditProg;
    }
    
	//発音を停止する
    if (reset) {
        Reset();
    }

	//brrデータをこちら側に移動する
	if (data) {
		if ( mVPset[prog].hasBrrData() ) {
			mVPset[prog].releaseBrr();
		}
        BRRData brr;
        brr.data = new unsigned char[size];
        brr.size = size;
		memmove(brr.data, data, size);
        mVPset[prog].setBRRData(&brr);
        // 波形の転送
        mDriver.SetBrrSample(prog, data, size, mVPset[prog].lp);
	}
	else {
        //NULLデータをセットされると削除を行う
		if (mVPset[prog].hasBrrData()) {
            mVPset[prog].releaseBrr();
			mVPset[prog].pgname[0] = 0;
            // 波形メモリの解放
            mDriver.DelBrrSample(prog);
			if ( propertyNotifyFunc ) {
				propertyNotifyFunc( kAudioUnitCustomProperty_ProgramName, propNotifyUserData );
			}
		}
	}
    
    if (notify) {
        if ( propertyNotifyFunc ) {
            propertyNotifyFunc( kAudioUnitCustomProperty_TotalRAM, propNotifyUserData );
        }
    }
	return true;
}

//-----------------------------------------------------------------------------

 /*static*/ int C700Kernel::GetPresetNum()
{
	return NUM_PRESETS;
}

//-----------------------------------------------------------------------------

 /*static*/ const char *C700Kernel::GetPresetName( int num )
{
	static char presetNames[NUM_PRESETS][64] = {
		"Empty",
		"Testtones"
	};
	if ( num < 0 || num >= NUM_PRESETS ) return NULL;
	return presetNames[num];
}

//-----------------------------------------------------------------------------

bool C700Kernel::SelectPreset( int num )
{
	switch(num) {
		case 0:
			for (int j=0; j<128; j++) {
				if (mVPset[j].hasBrrData()) {
					mVPset[j].releaseBrr();
					mVPset[j].pgname[0] = 0;
				}
			}
			if ( propertyNotifyFunc ) {
				propertyNotifyFunc( kAudioUnitCustomProperty_BRRData, propNotifyUserData );
				propertyNotifyFunc( kAudioUnitCustomProperty_ProgramName, propNotifyUserData );
			}
			break;
		case 1:
        {
			//プリセット音色
			mVPset[0].basekey=81;
			mVPset[0].lowkey=0;
			mVPset[0].highkey=127;
			mVPset[0].loop=true;
			mVPset[0].echo=false;
			mVPset[0].bank=0;
			mVPset[0].lp=18;
			mVPset[0].rate=28160.0;
			mVPset[0].volL=100;
			mVPset[0].volR=100;
            SetBRRData(sinewave_brr, 0x36, 0, false, false);
			strcpy(mVPset[0].pgname, "Sine Wave");
			
			mVPset[1].basekey=69;
			mVPset[1].lowkey=0;
			mVPset[1].highkey=127;
			mVPset[1].loop=true;
			mVPset[1].echo=true;
			mVPset[1].bank=0;
			mVPset[1].lp=9;
			mVPset[1].rate=28160.0;
			mVPset[1].volL=100;
			mVPset[1].volR=100;
            SetBRRData(squarewave_brr, 0x2d, 1, false, false);
			strcpy(mVPset[1].pgname, "Square Wave");
			
			mVPset[2].basekey=69;
			mVPset[2].lowkey=0;
			mVPset[2].highkey=127;
			mVPset[2].loop=true;
			mVPset[2].echo=true;
			mVPset[2].bank=0;
			mVPset[2].lp=9;
			mVPset[2].rate=28160.0;
			mVPset[2].volL=100;
			mVPset[2].volR=100;
            SetBRRData(pulse1_brr, 0x2d, 2, false, false);
			strcpy(mVPset[2].pgname, "25% Pulse");
			
			mVPset[3].basekey=69;
			mVPset[3].lowkey=0;
			mVPset[3].highkey=127;
			mVPset[3].loop=true;
			mVPset[3].echo=true;
			mVPset[3].bank=0;
			mVPset[3].lp=9;
			mVPset[3].rate=28160.0;
			mVPset[3].volL=100;
			mVPset[3].volR=100;
            SetBRRData(pulse2_brr, 0x2d, 3, false, false);
			strcpy(mVPset[3].pgname, "12.5% Pulse");
			
			mVPset[4].basekey=69;
			mVPset[4].lowkey=0;
			mVPset[4].highkey=127;
			mVPset[4].loop=true;
			mVPset[4].echo=true;
			mVPset[4].bank=0;
			mVPset[4].lp=9;
			mVPset[4].rate=28160.0;
			mVPset[4].volL=100;
			mVPset[4].volR=100;
            SetBRRData(pulse3_brr, 0x2d, 4, false, false);
			strcpy(mVPset[4].pgname, "6.25% Pulse");
			
			if ( propertyNotifyFunc ) {
				for (int i=kAudioUnitCustomProperty_Begin; i<kAudioUnitCustomProperty_Begin+kNumberOfProperties; i++) {
					propertyNotifyFunc(i,propNotifyUserData);
				}
			}
			break;
        }
		default:
			return false;
	}
	
	mDriver.RefreshKeyMap();
	
	return true;
}

//-----------------------------------------------------------------------------

void C700Kernel::Render( unsigned int frames, float *output[2] )
{
	mDriver.Process(frames, output);
    
    // MIDIインジケーターへの反映
    for (int i=0; i<16; i++) {
        int onNotes = mDriver.GetNoteOnNotes(i);
        if (onNotes != mOnNotes[i]) {
            mOnNotes[i] = onNotes;
            if ( propertyNotifyFunc ) {
                propertyNotifyFunc( kAudioUnitCustomProperty_NoteOnTrack_1+i, propNotifyUserData );
            }
            if ( mOnNotes[i] > mMaxNote[i] ) {
                mMaxNote[i] = mOnNotes[i];
                if ( propertyNotifyFunc ) {
                    propertyNotifyFunc( kAudioUnitCustomProperty_MaxNoteTrack_1+i, propNotifyUserData );
                }
            }
        }
    }
    
    if (mIsHwAvailable != mDriver.IsHwAvailable()) {
        mIsHwAvailable = mDriver.IsHwAvailable();
        propertyNotifyFunc( kAudioUnitCustomProperty_IsHwConnected, propNotifyUserData );
    }
}

//-----------------------------------------------------------------------------

void C700Kernel::HandleNoteOn( int ch, int note, int vel, int uniqueID, int inFrame )
{
    // TODO: ログ開始終了のノート番号を設定出来るようにする
    if (note == 0) {
        mDriver.StartRegisterLog(inFrame);
    }
    else if (note == 1) {
        mDriver.MarkLoopRegisterLog(inFrame);
    }
    else if (note == 2) {
        mDriver.EndRegisterLog(inFrame);
    }
    else {
        mDriver.NoteOn(ch, note, vel, uniqueID, inFrame);
    }
}

//-----------------------------------------------------------------------------

void C700Kernel::HandleNoteOff( int ch, int note, int uniqueID, int inFrame )
{
    if (note > 2) {
        mDriver.NoteOff(ch, note, 0, uniqueID, inFrame);
    }
}

//-----------------------------------------------------------------------------

void C700Kernel::HandlePitchBend( int ch, int pitch1, int pitch2, int inFrame )
{
	mDriver.PitchBend(ch, pitch1, pitch2, inFrame);
}

//-----------------------------------------------------------------------------

void C700Kernel::HandleControlChange( int ch, int controlNum, int value, int inFrame )
{
    switch (controlNum) {
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
            

        case 1:
        {
            //モジュレーションホイール
#if 0
            int	paramID = kParam_vibdepth;
            if ( ch > 0 ) {
                paramID = kParam_vibdepth_2 - 1 + ch;
            }
            if ( parameterSetFunc ) {
                parameterSetFunc( paramID, value, paramSetUserData );
            }
            break;
#endif
        }
            
        case 5:
            // ポルタメントタイム
        case 7:
            // ボリューム
        case 10:
            // パン
        case 11:
            // エクスプレッション
        case 64:
            // ホールド１（ダンパー）
        case 65:
            // ポルタメント・オン・オフ
        case 72:
            // SR
        case 73:
            // AR
        case 80:
            // SL
        case 75:
            // DR
        case 76:
            // ビブラート・レート
        case 77:
            // ビブラート・デプス
        case 84:
            // ポルタメント・コントロール
        case 91:
            // ECEN ON/OFF
            
        default:
           mDriver.ControlChange( ch, controlNum, value, inFrame);
           break;
    }
}

//-----------------------------------------------------------------------------
void C700Kernel::setRPNLSB(int ch, int value)
{
    mRPN[ch] &= 0xff00;
    mRPN[ch] |= value & 0x7f;
}

//-----------------------------------------------------------------------------
void C700Kernel::setRPNMSB(int ch, int value)
{
    mRPN[ch] &= 0x00ff;
    mRPN[ch] |= (value & 0x7f) << 8;
}

//-----------------------------------------------------------------------------
void C700Kernel::setNRPNLSB(int ch, int value)
{
    mNRPN[ch] &= 0xff00;
    mNRPN[ch] |= value & 0x7f;
}

//-----------------------------------------------------------------------------
void C700Kernel::setNRPNMSB(int ch, int value)
{
    mNRPN[ch] &= 0x00ff;
    mNRPN[ch] |= (value & 0x7f) << 8;
}

//-----------------------------------------------------------------------------
void C700Kernel::setDataEntryLSB(int ch, int value)
{
    mDataEntryValue[ch] &= 0xff00;
    mDataEntryValue[ch] |= value & 0x7f;
    sendDataEntryValue(ch);
}

//-----------------------------------------------------------------------------
void C700Kernel::setDataEntryMSB(int ch, int value)
{
    mDataEntryValue[ch] &= 0x00ff;
    mDataEntryValue[ch] |= (value & 0x7f) << 8;
}

//-----------------------------------------------------------------------------
void C700Kernel::sendDataEntryValue(int ch)
{
    if (mIsSettingNRPN) {
    }
    else  {
        switch (mRPN[ch]) {
            case 0x0000:
                mDriver.SetPBRange(ch, mDataEntryValue[ch]);
                break;
                
            default:
                break;
        }
    }
}

//-----------------------------------------------------------------------------

void C700Kernel::HandleProgramChange( int ch, int pg, int inFrame )
{
#if 0
	int	paramID = kParam_program;
	if ( ch > 0 ) {
		paramID = kParam_program_2 - 1 + ch;
	}
	if ( parameterSetFunc ) {
		parameterSetFunc( paramID, pg, paramSetUserData );
	}
#endif
#ifdef DEBUG_PRINT
    std::cout << "pg:" << pg << " inFrame:" << inFrame << std::endl;
#endif
    mDriver.ProgramChange(ch, pg, inFrame);
}

//-----------------------------------------------------------------------------

void C700Kernel::HandleResetAllControllers( int ch, int inFrame )
{
	mDriver.ResetAllControllers();
}

//-----------------------------------------------------------------------------

void C700Kernel::HandleAllNotesOff( int ch, int inFrame )
{
	mDriver.AllNotesOff();
	// ノートオンインジケータ初期化
	for ( int i=0; i<16; i++ ) {
		mOnNotes[i] = 0;
		if ( propertyNotifyFunc ) {
			propertyNotifyFunc( kAudioUnitCustomProperty_NoteOnTrack_1+i, propNotifyUserData );
		}
	}
}

//-----------------------------------------------------------------------------

void C700Kernel::HandleAllSoundOff( int ch, int inFrame )
{
	mDriver.AllSoundOff();
	// ノートオンインジケータ初期化
	for ( int i=0; i<16; i++ ) {
		mOnNotes[i] = 0;
		if ( propertyNotifyFunc ) {
			propertyNotifyFunc( kAudioUnitCustomProperty_NoteOnTrack_1+i, propNotifyUserData );
		}
	}
}

//-----------------------------------------------------------------------------
int C700Kernel::GetTotalRAM()
{
	//使用メモリを合計
	int	totalRam = 0;
	for ( int i=0; i<128; i++ ) {
		if ( mVPset[i].hasBrrData() ) {
			totalRam += mVPset[i].brrSize();
		}
	}
	totalRam += 2048 * ((int)GetParameter(kParam_echodelay));
	return totalRam;
}

//-----------------------------------------------------------------------------
void C700Kernel::SetPropertyNotifyFunc( void (*func) (int propID, void* userData), void* userData )
{
	propertyNotifyFunc = func;
	propNotifyUserData = userData;
}

//-----------------------------------------------------------------------------
void C700Kernel::SetParameterSetFunc( void (*func) (int paramID, float value, void* userData), void* userData )
{
	parameterSetFunc = func;
	paramSetUserData = userData;
}

//-----------------------------------------------------------------------------
void C700Kernel::CorrectLoopFlagForSave(int pgnum)
{
    if (mVPset[pgnum].loop) {
        mVPset[pgnum].setLoop();
	}
	else {
        mVPset[pgnum].unsetLoop();
	}
}

#if AU
CFStringRef C700Kernel::kSaveKey_ProgName = CFSTR("progname");
CFStringRef C700Kernel::kSaveKey_EditProg = CFSTR("editprog");
CFStringRef C700Kernel::kSaveKey_EditChan = CFSTR("editchan");
CFStringRef C700Kernel::kSaveKey_brrdata = CFSTR("brrdata");
CFStringRef C700Kernel::kSaveKey_looppoint = CFSTR("looppoint");
CFStringRef C700Kernel::kSaveKey_samplerate = CFSTR("samplerate");
CFStringRef C700Kernel::kSaveKey_basekey = CFSTR("key");
CFStringRef C700Kernel::kSaveKey_lowkey = CFSTR("lowkey");
CFStringRef C700Kernel::kSaveKey_highkey = CFSTR("highkey");
CFStringRef C700Kernel::kSaveKey_ar = CFSTR("ar");
CFStringRef C700Kernel::kSaveKey_dr = CFSTR("dr");
CFStringRef C700Kernel::kSaveKey_sl = CFSTR("sl");
CFStringRef C700Kernel::kSaveKey_sr = CFSTR("sr");
CFStringRef C700Kernel::kSaveKey_volL = CFSTR("volL");
CFStringRef C700Kernel::kSaveKey_volR = CFSTR("volR");
CFStringRef C700Kernel::kSaveKey_echo = CFSTR("echo");
CFStringRef C700Kernel::kSaveKey_bank = CFSTR("bank");
CFStringRef C700Kernel::kSaveKey_IsEmphasized = CFSTR("isemph");
CFStringRef C700Kernel::kSaveKey_SourceFile = CFSTR("srcfile");
CFStringRef C700Kernel::kSaveKey_SustainMode = CFSTR("sustainmode");
CFStringRef C700Kernel::kSaveKey_MonoMode = CFSTR("monomode");
CFStringRef C700Kernel::kSaveKey_PortamentoOn = CFSTR("portamentoon");
CFStringRef C700Kernel::kSaveKey_PortamentoRate = CFSTR("portamentorate");
CFStringRef C700Kernel::kSaveKey_NoteOnPriority = CFSTR("noteonpriority");
CFStringRef C700Kernel::kSaveKey_ReleasePriority = CFSTR("releasepriority");

//-----------------------------------------------------------------------------
void C700Kernel::AddNumToDictionary(CFMutableDictionaryRef dict, CFStringRef key, int value)
{
	CFNumberRef num = CFNumberCreate(NULL, kCFNumberIntType, &value);
	CFDictionarySetValue(dict, key, num);
	CFRelease(num);
}

//-----------------------------------------------------------------------------
void C700Kernel::AddBooleanToDictionary(CFMutableDictionaryRef dict, CFStringRef key, bool value)
{
	if ( value ) {
		CFDictionarySetValue(dict, key, kCFBooleanTrue);
	}
	else {
		CFDictionarySetValue(dict, key, kCFBooleanFalse);
	}
}

//-----------------------------------------------------------------------------
int C700Kernel::CreatePGDataDic(CFDictionaryRef *data, int pgnum)
{
	CFMutableDictionaryRef dict = CFDictionaryCreateMutable	(NULL, 0,
                                                             &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	const InstParams	*vpSet = GetVP();
	
    CorrectLoopFlagForSave(pgnum);
	CFDataRef	brrdata = CFDataCreate(NULL, vpSet[pgnum].brrData(), vpSet[pgnum].brrSize());
	CFDictionarySetValue(dict, kSaveKey_brrdata, brrdata);
	CFRelease(brrdata);
	
	AddNumToDictionary(dict, kSaveKey_looppoint, vpSet[pgnum].lp);
	CFNumberRef	num = CFNumberCreate(NULL, kCFNumberDoubleType, &vpSet[pgnum].rate);
	CFDictionarySetValue(dict, kSaveKey_samplerate, num);
	CFRelease(num);
	AddNumToDictionary(dict, kSaveKey_basekey, vpSet[pgnum].basekey);
	AddNumToDictionary(dict, kSaveKey_lowkey, vpSet[pgnum].lowkey);
	AddNumToDictionary(dict, kSaveKey_highkey, vpSet[pgnum].highkey);
	AddNumToDictionary(dict, kSaveKey_ar, vpSet[pgnum].ar);
	AddNumToDictionary(dict, kSaveKey_dr, vpSet[pgnum].dr);
	AddNumToDictionary(dict, kSaveKey_sl, vpSet[pgnum].sl);
	AddNumToDictionary(dict, kSaveKey_sr, vpSet[pgnum].sr);
    AddBooleanToDictionary(dict, kSaveKey_SustainMode, vpSet[pgnum].sustainMode);
	AddNumToDictionary(dict, kSaveKey_volL, vpSet[pgnum].volL);
	AddNumToDictionary(dict, kSaveKey_volR, vpSet[pgnum].volR);
	AddBooleanToDictionary(dict, kSaveKey_echo, vpSet[pgnum].echo);
	AddNumToDictionary(dict, kSaveKey_bank, vpSet[pgnum].bank);
	AddBooleanToDictionary(dict, kSaveKey_MonoMode, vpSet[pgnum].monoMode);
	AddBooleanToDictionary(dict, kSaveKey_PortamentoOn, vpSet[pgnum].portamentoOn);
    AddNumToDictionary(dict, kSaveKey_PortamentoRate, vpSet[pgnum].portamentoRate);
    AddNumToDictionary(dict, kSaveKey_NoteOnPriority, vpSet[pgnum].noteOnPriority);
    AddNumToDictionary(dict, kSaveKey_ReleasePriority, vpSet[pgnum].releasePriority);
    
	//元波形情報
	AddBooleanToDictionary(dict, kSaveKey_IsEmphasized, vpSet[pgnum].isEmphasized);
	if ( vpSet[pgnum].sourceFile[0] ) {
		CFURLRef	url =
		CFURLCreateFromFileSystemRepresentation(NULL, (UInt8*)vpSet[pgnum].sourceFile,
												strlen(vpSet[pgnum].sourceFile), false);
		CFDataRef urlData = CFURLCreateData( NULL, url, kCFStringEncodingUTF8, false );
		CFDictionarySetValue(dict, kSaveKey_SourceFile, urlData);
		CFRelease(urlData);
		CFRelease(url);
	}
	
	//プログラム名
	if (vpSet[pgnum].pgname[0] != 0) {
		CFStringRef	str = CFStringCreateWithCString(NULL, vpSet[pgnum].pgname, kCFStringEncodingUTF8);
		CFDictionarySetValue(dict, kSaveKey_ProgName, str);
		CFRelease(str);
	}
	
	*data = dict;
	return 0;
}

//-----------------------------------------------------------------------------
void C700Kernel::RestorePGDataDic(CFPropertyListRef data, int pgnum)
{
	int editProg = GetPropertyValue(kAudioUnitCustomProperty_EditingProgram);
	SetPropertyValue(kAudioUnitCustomProperty_EditingProgram, pgnum);
    
	CFDictionaryRef dict = static_cast<CFDictionaryRef>(data);
	CFNumberRef cfnum;
	
	int	value;
	if (CFDictionaryContainsKey(dict, kSaveKey_looppoint)) {
		cfnum = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(dict, kSaveKey_looppoint));
		CFNumberGetValue(cfnum, kCFNumberIntType, &value);
		SetPropertyValue(kAudioUnitCustomProperty_LoopPoint, value);
	}
	
	CFDataRef cfdata = reinterpret_cast<CFDataRef>(CFDictionaryGetValue(dict, kSaveKey_brrdata));
	int	size = CFDataGetLength(cfdata);
	const UInt8	*dataptr = CFDataGetBytePtr(cfdata);
	SetBRRData(dataptr, size);
	SetPropertyValue(kAudioUnitCustomProperty_Loop,
						   dataptr[size-9]&2?true:false);
	
	double	dvalue;
	if (CFDictionaryContainsKey(dict, kSaveKey_samplerate)) {
		cfnum = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(dict, kSaveKey_samplerate));
		CFNumberGetValue(cfnum, kCFNumberDoubleType, &dvalue);
		SetPropertyValue(kAudioUnitCustomProperty_Rate, dvalue);
	}
	
	if (CFDictionaryContainsKey(dict, kSaveKey_basekey)) {
		cfnum = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(dict, kSaveKey_basekey));
		CFNumberGetValue(cfnum, kCFNumberIntType, &value);
		SetPropertyValue(kAudioUnitCustomProperty_BaseKey, value);
	}
	else {
		SetPropertyValue(kAudioUnitCustomProperty_BaseKey, 60);
	}
	if (CFDictionaryContainsKey(dict, kSaveKey_lowkey)) {
		cfnum = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(dict, kSaveKey_lowkey));
		CFNumberGetValue(cfnum, kCFNumberIntType, &value);
		SetPropertyValue(kAudioUnitCustomProperty_LowKey, value);
	}
	else {
		SetPropertyValue(kAudioUnitCustomProperty_LowKey, 0);
	}
	if (CFDictionaryContainsKey(dict, kSaveKey_highkey)) {
		cfnum = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(dict, kSaveKey_highkey));
		CFNumberGetValue(cfnum, kCFNumberIntType, &value);
		SetPropertyValue(kAudioUnitCustomProperty_HighKey,value );
	}
	else {
		SetPropertyValue(kAudioUnitCustomProperty_HighKey,127 );
	}
	
	if (CFDictionaryContainsKey(dict, kSaveKey_ar)) {
		cfnum = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(dict, kSaveKey_ar));
		CFNumberGetValue(cfnum, kCFNumberIntType, &value);
		SetPropertyValue(kAudioUnitCustomProperty_AR, value);
	}
	
	if (CFDictionaryContainsKey(dict, kSaveKey_dr)) {
		cfnum = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(dict, kSaveKey_dr));
		CFNumberGetValue(cfnum, kCFNumberIntType, &value);
		SetPropertyValue(kAudioUnitCustomProperty_DR, value);
	}
	
	if (CFDictionaryContainsKey(dict, kSaveKey_sl)) {
		cfnum = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(dict, kSaveKey_sl));
		CFNumberGetValue(cfnum, kCFNumberIntType, &value);
		SetPropertyValue(kAudioUnitCustomProperty_SL, value);
	}
	
	if (CFDictionaryContainsKey(dict, kSaveKey_sr)) {
		cfnum = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(dict, kSaveKey_sr));
		CFNumberGetValue(cfnum, kCFNumberIntType, &value);
		SetPropertyValue(kAudioUnitCustomProperty_SR, value);
	}
	
    bool isSustainModeSet = false;
    if (CFDictionaryContainsKey(dict, kSaveKey_SustainMode)) {
		CFBooleanRef cfbool = reinterpret_cast<CFBooleanRef>(CFDictionaryGetValue(dict, kSaveKey_SustainMode));
		SetPropertyValue(kAudioUnitCustomProperty_SustainMode,CFBooleanGetValue(cfbool) ? 1.0f:.0f);
        isSustainModeSet = true;
	}
	
    if (CFDictionaryContainsKey(dict, kSaveKey_volL)) {
		cfnum = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(dict, kSaveKey_volL));
		CFNumberGetValue(cfnum, kCFNumberIntType, &value);
		SetPropertyValue(kAudioUnitCustomProperty_VolL, value);
	}
	else {
		SetPropertyValue(kAudioUnitCustomProperty_VolL, 100);
	}
	
	if (CFDictionaryContainsKey(dict, kSaveKey_volR)) {
		cfnum = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(dict, kSaveKey_volR));
		CFNumberGetValue(cfnum, kCFNumberIntType, &value);
		SetPropertyValue(kAudioUnitCustomProperty_VolR, value);
	}
	else {
		SetPropertyValue(kAudioUnitCustomProperty_VolR, 100);
	}
	
	if (CFDictionaryContainsKey(dict, kSaveKey_echo)) {
		CFBooleanRef cfbool = reinterpret_cast<CFBooleanRef>(CFDictionaryGetValue(dict, kSaveKey_echo));
		SetPropertyValue(kAudioUnitCustomProperty_Echo,CFBooleanGetValue(cfbool) ? 1.0f:.0f);
	}
	else {
		SetPropertyValue(kAudioUnitCustomProperty_Echo, .0f);
	}
	
	if (CFDictionaryContainsKey(dict, kSaveKey_bank)) {
		cfnum = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(dict, kSaveKey_bank));
		CFNumberGetValue(cfnum, kCFNumberIntType, &value);
		SetPropertyValue(kAudioUnitCustomProperty_Bank, value );
	}
	else {
		SetPropertyValue(kAudioUnitCustomProperty_Bank, 0 );
	}
	
    if (CFDictionaryContainsKey(dict, kSaveKey_MonoMode)) {
		CFBooleanRef cfbool = reinterpret_cast<CFBooleanRef>(CFDictionaryGetValue(dict, kSaveKey_MonoMode));
		SetPropertyValue(kAudioUnitCustomProperty_MonoMode,CFBooleanGetValue(cfbool) ? 1.0f:.0f);
	}
	else {
		SetPropertyValue(kAudioUnitCustomProperty_MonoMode, .0f);
	}
    
    if (CFDictionaryContainsKey(dict, kSaveKey_PortamentoOn)) {
		CFBooleanRef cfbool = reinterpret_cast<CFBooleanRef>(CFDictionaryGetValue(dict, kSaveKey_PortamentoOn));
		SetPropertyValue(kAudioUnitCustomProperty_PortamentoOn,CFBooleanGetValue(cfbool) ? 1.0f:.0f);
	}
	else {
		SetPropertyValue(kAudioUnitCustomProperty_PortamentoOn, .0f);
	}
    
    if (CFDictionaryContainsKey(dict, kSaveKey_PortamentoRate)) {
		cfnum = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(dict, kSaveKey_PortamentoRate));
		CFNumberGetValue(cfnum, kCFNumberIntType, &value);
		SetPropertyValue(kAudioUnitCustomProperty_PortamentoRate, value );
	}
	else {
		SetPropertyValue(kAudioUnitCustomProperty_PortamentoRate, 0 );
	}
    
    if (CFDictionaryContainsKey(dict, kSaveKey_NoteOnPriority)) {
		cfnum = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(dict, kSaveKey_NoteOnPriority));
		CFNumberGetValue(cfnum, kCFNumberIntType, &value);
		SetPropertyValue(kAudioUnitCustomProperty_NoteOnPriority, value );
	}
	else {
		SetPropertyValue(kAudioUnitCustomProperty_NoteOnPriority, 64 );
	}
    
    if (CFDictionaryContainsKey(dict, kSaveKey_ReleasePriority)) {
		cfnum = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(dict, kSaveKey_ReleasePriority));
		CFNumberGetValue(cfnum, kCFNumberIntType, &value);
		SetPropertyValue(kAudioUnitCustomProperty_ReleasePriority, value );
	}
	else {
		SetPropertyValue(kAudioUnitCustomProperty_ReleasePriority, 0 );
	}
    
	if (CFDictionaryContainsKey(dict, kSaveKey_ProgName)) {
		char	pgname[PROGRAMNAME_MAX_LEN];
		CFStringGetCString(reinterpret_cast<CFStringRef>(CFDictionaryGetValue(dict, kSaveKey_ProgName)),
						   pgname, PROGRAMNAME_MAX_LEN, kCFStringEncodingUTF8);
		SetProgramName(pgname);
	}
	else {
		SetProgramName("");
	}
	
	//元波形ファイル情報を復元
	if (CFDictionaryContainsKey(dict, kSaveKey_SourceFile)) {
		CFDataRef	urlData = reinterpret_cast<CFDataRef>(CFDictionaryGetValue(dict, kSaveKey_SourceFile));
		CFURLRef	url = CFURLCreateWithBytes( NULL, CFDataGetBytePtr(urlData),
											   CFDataGetLength(urlData), kCFStringEncodingUTF8, NULL );
		CFStringRef pathStr = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
		char	path[PATH_LEN_MAX];
		CFStringGetCString(pathStr, path, PATH_LEN_MAX-1, kCFStringEncodingUTF8);
		SetSourceFilePath(path);
		CFRelease(pathStr);
		CFRelease(url);
		
		CFBooleanRef cfbool = reinterpret_cast<CFBooleanRef>(CFDictionaryGetValue(dict, kSaveKey_IsEmphasized));
		SetPropertyValue(kAudioUnitCustomProperty_IsEmaphasized, CFBooleanGetValue(cfbool) ? 1.0f:.0f);
        
        // SRをリリース時に使用するけどSustainModeの設定項目は無い過渡的なバージョン
        if (!isSustainModeSet) {
            SetPropertyValue(kAudioUnitCustomProperty_SustainMode, 1.0f);
        }
	}
	else {
		SetSourceFilePath("");
		SetPropertyValue(kAudioUnitCustomProperty_IsEmaphasized, .0f);
        
        // SRをそのまま使う古いバージョン
        if (!isSustainModeSet) {
            SetPropertyValue(kAudioUnitCustomProperty_SustainMode, .0f);
        }
	}
	
	//UIに変更を反映
	if (pgnum == editProg) {
		SetPropertyValue(kAudioUnitCustomProperty_EditingProgram, editProg);
	}
	
	GetGenerator()->RefreshKeyMap();
}

#endif
