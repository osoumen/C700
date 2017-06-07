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
#include "samplebrr.h"
#include <math.h>
#if MAC
#include "macOSUtils.h"
#else
#include <Shlobj.h>
#endif

void CalcFIRParam( const float band[5], int filter[8] );

//-----------------------------------------------------------------------------
C700Kernel::C700Kernel()
: C700Parameters(kNumberOfParameters)
, propertyNotifyFunc(NULL)
, propNotifyUserData(NULL)
, parameterSetFunc(NULL)
, paramSetUserData(NULL)
, mCodeFile(NULL)
, mGlobalSettingsHasChanged(false)
{
    createPropertyParamMap(mPropertyParams);
    
	for ( int i=0; i<kNumberOfParameters; i++ ) {
		SetParameter(i, GetParameterDefault(i));
	}

	for (int i=0; i<5; i++) {
		mFilterBand[i] = 1.0f;
	}
	
	mEditProg = 0;
	mEditChannel = 0;
    
    mIsHwAvailable = false;
    
    mCurrentPosInTimeLine = .0;
    mRecordStartBeatPos = .0;
    mRecordLoopStartBeatPos = .0;
    mRecordEndBeatPos = .0;
    mIsPlaying = false;
    mSampleRate = 44100.0;
    
	// ノートオンインジケータ初期化
	for ( int i=0; i<16; i++ ) {
		mOnNotes[i] = 0;
		mMaxNote[i] = 0;
	}
    mTotalOnNotes = 0;
	
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
		
		mVPset[i].ar = mPropertyParams[kAudioUnitCustomProperty_AR].defaultValue;
		mVPset[i].dr = mPropertyParams[kAudioUnitCustomProperty_DR].defaultValue;
		mVPset[i].sl = mPropertyParams[kAudioUnitCustomProperty_SL].defaultValue;
		mVPset[i].sr1 = mPropertyParams[kAudioUnitCustomProperty_SR1].defaultValue;
		mVPset[i].sr2 = mPropertyParams[kAudioUnitCustomProperty_SR2].defaultValue;
        
        mVPset[i].sustainMode = mPropertyParams[kAudioUnitCustomProperty_SustainMode].defaultValue!=0?true:false;
        mVPset[i].monoMode = mPropertyParams[kAudioUnitCustomProperty_MonoMode].defaultValue!=0?true:false;
        mVPset[i].portamentoOn = mPropertyParams[kAudioUnitCustomProperty_PortamentoOn].defaultValue!=0?true:false;
        mVPset[i].portamentoRate = mPropertyParams[kAudioUnitCustomProperty_PortamentoRate].defaultValue;
        mVPset[i].noteOnPriority = mPropertyParams[kAudioUnitCustomProperty_NoteOnPriority].defaultValue;
        mVPset[i].releasePriority = mPropertyParams[kAudioUnitCustomProperty_ReleasePriority].defaultValue;
        mVPset[i].pmOn = mPropertyParams[kAudioUnitCustomProperty_PitchModulationOn].defaultValue!=0?true:false;
        mVPset[i].noiseOn = mPropertyParams[kAudioUnitCustomProperty_NoiseOn].defaultValue!=0?true:false;
	}
	
    restoreGlobalProperties();
    
	// 音源にプログラムのメモリを渡す
	mDriver.SetVPSet(mVPset);
}

//-----------------------------------------------------------------------------
C700Kernel::~C700Kernel()
{
    if (mGlobalSettingsHasChanged) {
        storeGlobalProperties();
    }
    
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
    mTotalOnNotes = 0;
    propertyNotifyFunc( kAudioUnitCustomProperty_MaxNoteOnTotal, propNotifyUserData );
    
    if (mGlobalSettingsHasChanged) {
        storeGlobalProperties();
    }
}

//-----------------------------------------------------------------------------
void C700Kernel::SetTempo( double tempo )
{
	mTempo = tempo;
}

//-----------------------------------------------------------------------------
void C700Kernel::SetCurrentSampleInTimeLine( double currentSample )
{
    mCurrentPosInTimeLine = currentSample;
}

//-----------------------------------------------------------------------------
void C700Kernel::SetIsPlaying( bool isPlaying )
{
    mIsPlaying = isPlaying;
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
			
		case kAudioUnitCustomProperty_SR1:
			return mVPset[mEditProg].sr1;
			
		case kAudioUnitCustomProperty_SR2:
			return mVPset[mEditProg].sr2;

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
            
        case kAudioUnitCustomProperty_PitchModulationOn:
			return mVPset[mEditProg].pmOn ? 1.0f:.0f;

        case kAudioUnitCustomProperty_NoiseOn:
			return mVPset[mEditProg].noiseOn ? 1.0f:.0f;

        case kAudioUnitCustomProperty_PortamentoRate:
            return mVPset[mEditProg].portamentoRate;
            
        case kAudioUnitCustomProperty_NoteOnPriority:
            return mVPset[mEditProg].noteOnPriority;
            
        case kAudioUnitCustomProperty_ReleasePriority:
            return mVPset[mEditProg].releasePriority;
            
        case kAudioUnitCustomProperty_IsHwConnected:
            return mDriver.GetDsp()->IsHwAvailable() ? 1.0f:.0f;
            
        case kAudioUnitCustomProperty_RecSaveAsSpc:
            return mDriver.GetDsp()->GetRecSaveAsSpc() ? 1.0f:0.f;
            
        case kAudioUnitCustomProperty_RecSaveAsSmc:
            return mDriver.GetDsp()->GetRecSaveAsSmc() ? 1.0f:0.f;
            
        case kAudioUnitCustomProperty_TimeBaseForSmc:
        {
            switch (mDriver.GetDsp()->GetTimeBaseForSmc()) {
                case C700DSP::SmcTimeBaseNTSC:
                    return 0;   // NTSC
                case C700DSP::SmcTimeBasePAL:
                    return 1;
            }
        }
        
        case kAudioUnitCustomProperty_RepeatNumForSpc:
            return mDriver.GetDsp()->GetRepeatNumForSpc();
            
        case kAudioUnitCustomProperty_FadeMsTimeForSpc:
            return mDriver.GetDsp()->GetFadeMsTimeForSpc();
            
        case kAudioUnitCustomProperty_SongPlayerCodeVer:
            return mDriver.GetDsp()->GetSongPlayCodeVer();
            
        case kAudioUnitCustomProperty_HostBeatPos:
            return mCurrentPosInTimeLine;
            
        case kAudioUnitCustomProperty_MaxNoteOnTotal:
            return mTotalOnNotes;
            
		default:
			return 0;
	}	
}

//-----------------------------------------------------------------------------
double C700Kernel::GetPropertyDoubleValue( int inID )
{
    switch (inID) {
        case kAudioUnitCustomProperty_Rate:
			return mVPset[mEditProg].rate;
            
        case kAudioUnitCustomProperty_RecordStartBeatPos:
            return mRecordStartBeatPos;
            
        case kAudioUnitCustomProperty_RecordLoopStartBeatPos:
            return mRecordLoopStartBeatPos;
            
        case kAudioUnitCustomProperty_RecordEndBeatPos:
            return mRecordEndBeatPos;
            
        case kAudioUnitCustomProperty_SongPlayerCodeVer:
            return mDriver.GetDsp()->GetSongPlayCodeVer();
            
        case kAudioUnitCustomProperty_HostBeatPos:
            return mCurrentPosInTimeLine;
            
        default:
			return 0;
    }
}

//-----------------------------------------------------------------------------
int C700Kernel::GetPropertyPtrDataSize( int inID )
{
    switch (inID) {
        case kAudioUnitCustomProperty_SongPlayerCode:
            if (mCodeFile != NULL) {
                return mCodeFile->GetDataUsed();
            }
            break;
            
        case kAudioUnitCustomProperty_BRRData:
            if (GetBRRData()->data == NULL) {
                return 0;
            }
            return GetBRRData()->size;
    }
    return 0;
}

//-----------------------------------------------------------------------------
const void *C700Kernel::GetPropertyPtrValue( int inID )
{
    switch (inID) {
#if AU
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
            
            fileData.SetDataFromChip(GetDriver(),
                                     GetPropertyValue(kAudioUnitCustomProperty_EditingProgram), mTempo);
            
            if ( fileData.IsLoaded() ) {
                CFDataRef xidata;
                xidata = CFDataCreate(NULL, fileData.GetDataPtr(), fileData.GetDataUsed() );
                return (void *)xidata;	//使用後要release
            }
            else {
                return NULL;
            }
        }
#endif
        case kAudioUnitCustomProperty_BRRData:
            return GetBRRData()->data;
            
        case kAudioUnitCustomProperty_SongPlayerCode:
        {
            if (mCodeFile != NULL) {
                return mCodeFile->GetDataPtr();
            }
            break;
        }
        case kAudioUnitCustomProperty_SourceFileRef:
            return GetSourceFilePath();
            
		case kAudioUnitCustomProperty_ProgramName:
            return GetProgramName();
            
        case kAudioUnitCustomProperty_SongRecordPath:
            return mDriver.GetDsp()->GetSongRecordPath();

        case kAudioUnitCustomProperty_GameTitle:
            return mDriver.GetDsp()->GetGameTitle();

        case kAudioUnitCustomProperty_SongTitle:
            return mDriver.GetDsp()->GetSongTitle();

        case kAudioUnitCustomProperty_NameOfDumper:
            return mDriver.GetDsp()->GetNameOfDumper();

        case kAudioUnitCustomProperty_ArtistOfSong:
            return mDriver.GetDsp()->GetArtistOfSong();

        case kAudioUnitCustomProperty_SongComments:
            return mDriver.GetDsp()->GetSongComments();

        default:
			break;
    }
    return 0;
}

//-----------------------------------------------------------------------------
bool C700Kernel::GetPropertyStructValue( int inID, void *outData )
{
	return false;
}

//-----------------------------------------------------------------------------
bool C700Kernel::SetPropertyValue( int inID, float value )
{
	bool		boolData = value>0.5f?true:false;
	
    if (mPropertyParams[inID].saveToGlobal) {
        mGlobalSettingsHasChanged = true;
    }
    
	switch (inID) {
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
			
		case kAudioUnitCustomProperty_SR1:
			mVPset[mEditProg].sr1 = value;
			return true;
			
		case kAudioUnitCustomProperty_SR2:
			mVPset[mEditProg].sr2 = value;
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
                auto it = mPropertyParams.begin();
                while (it != mPropertyParams.end()) {
                    if (it->second.saveToProg) {
                        propertyNotifyFunc(it->first, propNotifyUserData);
                    }
                    it++;
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
                auto it = mPropertyParams.begin();
                while (it != mPropertyParams.end()) {
                    if (it->second.saveToProg) {
                        propertyNotifyFunc(it->first, propNotifyUserData);
                    }
                    it++;
                }
                propertyNotifyFunc(kAudioUnitCustomProperty_EditingProgram, propNotifyUserData);
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
            
        case kAudioUnitCustomProperty_PitchModulationOn:
            mVPset[mEditProg].pmOn = boolData;
            return true;
        
        case kAudioUnitCustomProperty_NoiseOn:
            mVPset[mEditProg].noiseOn = boolData;
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

        case kAudioUnitCustomProperty_RecSaveAsSpc:
            mDriver.GetDsp()->SetRecSaveAsSpc(value==0 ? false:true);
            return true;
            
        case kAudioUnitCustomProperty_RecSaveAsSmc:
            mDriver.GetDsp()->SetRecSaveAsSmc(value==0 ? false:true);
            return true;
            
        case kAudioUnitCustomProperty_TimeBaseForSmc:
        {
            if (value == 0) {
                mDriver.GetDsp()->SetTimeBaseForSmc(C700DSP::SmcTimeBaseNTSC);
            }
            else {
                mDriver.GetDsp()->SetTimeBaseForSmc(C700DSP::SmcTimeBasePAL);
            }
            return true;
        }
            
        case kAudioUnitCustomProperty_RepeatNumForSpc:
            mDriver.GetDsp()->SetRepeatNumForSpc(value);
            return true;
            
        case kAudioUnitCustomProperty_FadeMsTimeForSpc:
            mDriver.GetDsp()->SetFadeMsTimeForSpc(value);
            return true;
            
        case kAudioUnitCustomProperty_MaxNoteOnTotal:
            mTotalOnNotes = value;
            return true;
            
		default:
			return false;
	}
}

//-----------------------------------------------------------------------------
bool C700Kernel::SetPropertyDoubleValue( int inID, double value )
{
    if (mPropertyParams[inID].saveToGlobal) {
        mGlobalSettingsHasChanged = true;
    }
    
    switch (inID) {
		case kAudioUnitCustomProperty_Rate:
			mVPset[mEditProg].rate = value;
			return true;
			
        case kAudioUnitCustomProperty_RecordStartBeatPos:
            mRecordStartBeatPos = value;
            return true;
            
        case kAudioUnitCustomProperty_RecordLoopStartBeatPos:
            mRecordLoopStartBeatPos = value;
            return true;
            
        case kAudioUnitCustomProperty_RecordEndBeatPos:
            mRecordEndBeatPos = value;
            return true;
            
        default:
			return false;
    }
}

bool C700Kernel::SetPropertyPtrValue( int inID, const void *inPtr, int size )
{
    if (mPropertyParams[inID].saveToGlobal) {
        mGlobalSettingsHasChanged = true;
    }
    
    switch (inID) {
#if AU
		case kAudioUnitCustomProperty_PGDictionary:
        {
            CFDictionaryRef	pgdata = reinterpret_cast<CFDictionaryRef>(inPtr);
            int				editProg = GetPropertyValue(kAudioUnitCustomProperty_EditingProgram);
            RestorePGDataDic(pgdata, editProg);
            return true;
        }
        
#endif
		case kAudioUnitCustomProperty_BRRData:
        {
            SetBRRData(reinterpret_cast<const unsigned char *>(inPtr), size);
            return true;
        }

        case kAudioUnitCustomProperty_SongPlayerCode:
        {
			PlayerCodeReader *code = new PlayerCodeReader(inPtr, size, true);
            if (!code->IsLoaded()) {
				delete code;
                return true;
            }
			if (mCodeFile != NULL) {
				delete mCodeFile;
			}
			mCodeFile = code;
            mDriver.GetDsp()->SetSpcPlayerCode(mCodeFile->getSpcPlayerCode(), mCodeFile->getSpcPlayerCodeSize());
            mDriver.GetDsp()->SetSmcEmulationVector(mCodeFile->getSmcEmulationVector());
            mDriver.GetDsp()->SetSmcNativeVector(mCodeFile->getSmcNativeVector());
            mDriver.GetDsp()->SetSmcPlayerCode(mCodeFile->getSmcPlayerCode(), mCodeFile->getSmcPlayerCodeSize());
            mDriver.GetDsp()->SetSongPlayCodeVer(mCodeFile->getVersion());
            return true;
        }
        case kAudioUnitCustomProperty_SourceFileRef:
        {
            SetSourceFilePath(reinterpret_cast<const char *>(inPtr));
            return true;
        }
		case kAudioUnitCustomProperty_ProgramName:
        {
            SetProgramName(reinterpret_cast<const char *>(inPtr));
            return true;
        }
        case kAudioUnitCustomProperty_SongRecordPath:
        {
            mDriver.GetDsp()->SetSongRecordPath(reinterpret_cast<const char *>(inPtr));
            return true;
        }
        case kAudioUnitCustomProperty_GameTitle:
        {
            mDriver.GetDsp()->SetGameTitle(reinterpret_cast<const char *>(inPtr));
            return true;
        }
        case kAudioUnitCustomProperty_SongTitle:
        {
            mDriver.GetDsp()->SetSongTitle(reinterpret_cast<const char *>(inPtr));
            return true;
        }
        case kAudioUnitCustomProperty_NameOfDumper:
        {
            mDriver.GetDsp()->SetNameOfDumper(reinterpret_cast<const char *>(inPtr));
            return true;
        }
        case kAudioUnitCustomProperty_ArtistOfSong:
        {
            mDriver.GetDsp()->SetArtistOfSong(reinterpret_cast<const char *>(inPtr));
            return true;
        }
        case kAudioUnitCustomProperty_SongComments:
        {
            mDriver.GetDsp()->SetSongComments(reinterpret_cast<const char *>(inPtr));
            return true;
        }
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
	if ((data != NULL) && (size > 0)) {
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
        
        SetPropertyValue(kAudioUnitCustomProperty_Loop, data[size-9]&2?true:false);
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
                auto it = mPropertyParams.begin();
                while (it != mPropertyParams.end()) {
                    if (it->second.saveToProg) {
                        propertyNotifyFunc(it->first, propNotifyUserData);
                    }
                    it++;
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
    // 記録開始、終了
    if (mIsPlaying) {
        double beatFrameRatio = (mSampleRate * 60) / mTempo;
        double framesBeat = frames / beatFrameRatio;
        if ((mCurrentPosInTimeLine <= mRecordStartBeatPos) &&
            (mRecordStartBeatPos < (mCurrentPosInTimeLine + framesBeat))) {
            if (mRecordStartBeatPos != mRecordEndBeatPos) {
                mDriver.StartRegisterLog((mRecordStartBeatPos-mCurrentPosInTimeLine) * beatFrameRatio);
            }
        }
        if ((mCurrentPosInTimeLine <= mRecordLoopStartBeatPos) &&
            (mRecordLoopStartBeatPos < (mCurrentPosInTimeLine + framesBeat))) {
            if (mRecordStartBeatPos != mRecordEndBeatPos) {
                mDriver.MarkLoopRegisterLog((mRecordLoopStartBeatPos-mCurrentPosInTimeLine) * beatFrameRatio);
            }
        }
        if ((mCurrentPosInTimeLine <= mRecordEndBeatPos) &&
            (mRecordEndBeatPos < (mCurrentPosInTimeLine + framesBeat))) {
            if (mRecordStartBeatPos != mRecordEndBeatPos) {
                mDriver.EndRegisterLog((mRecordEndBeatPos-mCurrentPosInTimeLine) * beatFrameRatio);
            }
        }
    }
    
	mDriver.Process(frames, output);
    
    // MIDIインジケーターへの反映
    int onNotesTotal = 0;
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
        onNotesTotal += onNotes;
    }
    // 合計発音数表示
    if (onNotesTotal > mTotalOnNotes) {
        mTotalOnNotes = onNotesTotal;
        if ( propertyNotifyFunc ) {
            propertyNotifyFunc( kAudioUnitCustomProperty_MaxNoteOnTotal, propNotifyUserData );
        }
    }
    // ハードウェア接続チェック
    if (mIsHwAvailable != mDriver.GetDsp()->IsHwAvailable()) {
        mIsHwAvailable = mDriver.GetDsp()->IsHwAvailable();
        propertyNotifyFunc( kAudioUnitCustomProperty_IsHwConnected, propNotifyUserData );
    }
}

//-----------------------------------------------------------------------------

void C700Kernel::HandleNoteOn( int ch, int note, int vel, int uniqueID, int inFrame )
{
    mDriver.NoteOn(ch, note, vel, uniqueID, inFrame);
    //printf("NoteOn inFrame:%d\n", inFrame);
}

//-----------------------------------------------------------------------------

void C700Kernel::HandleNoteOff( int ch, int note, int uniqueID, int inFrame )
{
    mDriver.NoteOff(ch, note, 0, uniqueID, inFrame);
    //printf("NoteOff inFrame:%d\n", inFrame);
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
            setDataEntryLSB(ch, value, inFrame);
            break;
            
        case 38:
            // データ・エントリー(MSB)
            setDataEntryMSB(ch, value, inFrame);
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
    mIsSettingNRPN[ch] = false;
}

//-----------------------------------------------------------------------------
void C700Kernel::setRPNMSB(int ch, int value)
{
    mRPN[ch] &= 0x00ff;
    mRPN[ch] |= (value & 0x7f) << 8;
    mIsSettingNRPN[ch] = false;
}

//-----------------------------------------------------------------------------
void C700Kernel::setNRPNLSB(int ch, int value)
{
    mNRPN[ch] &= 0xff00;
    mNRPN[ch] |= value & 0x7f;
    mIsSettingNRPN[ch] = true;
}

//-----------------------------------------------------------------------------
void C700Kernel::setNRPNMSB(int ch, int value)
{
    mNRPN[ch] &= 0x00ff;
    mNRPN[ch] |= (value & 0x7f) << 8;
    mIsSettingNRPN[ch] = true;
}

//-----------------------------------------------------------------------------
void C700Kernel::setDataEntryLSB(int ch, int value, int inFrame)
{
    mDataEntryValue[ch] &= 0xff00;
    mDataEntryValue[ch] |= value & 0x7f;
    sendDataEntryValue(ch, inFrame);
}

//-----------------------------------------------------------------------------
void C700Kernel::setDataEntryMSB(int ch, int value, int inFrame)
{
    mDataEntryValue[ch] &= 0x00ff;
    mDataEntryValue[ch] |= (value & 0x7f) << 8;
}

//-----------------------------------------------------------------------------
void C700Kernel::sendDataEntryValue(int ch, int inFrame)
{
    if (mIsSettingNRPN) {
        if ((mNRPN[ch] & 0xff00) == 0x7e00) {   // #98:rr #99:126
            mDriver.DirectRegisterWrite(ch, mNRPN[ch] & 0x00ff, mDataEntryValue[ch], inFrame);
        }
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

//-----------------------------------------------------------------------------
void C700Kernel::restoreGlobalProperties()
{
    char path[PATH_LEN_MAX];
    // SongRecordPathの初期値を設定
    getDocumentsFolder(path, PATH_LEN_MAX);
    mDriver.GetDsp()->SetSongRecordPath(path);
    
    // saveToGlobalのプロパティの復元
    getPreferenceFolder(path, PATH_LEN_MAX);
    //std::cout << path << std::endl;
    
    ChunkReader settings(path);
    if (settings.GetDataSize() < 2) {
        // 読み込めなかった時は初期値の1バイトのはず
        return;
    }
    
    while ( settings.GetLeftSize() >= (int)sizeof( ChunkReader::MyChunkHead ) ) {
		int		ckType;
		long	ckSize;
		settings.readChunkHead(&ckType, &ckSize);
        
        auto it = mPropertyParams.find(ckType);
        if (it == mPropertyParams.end() || it->second.saveToGlobal == false) {
            // 不明チャンクの場合は飛ばす
            settings.AdvDataPos(ckSize);
        }
        else if (RestorePropertyFromData(&settings, ckSize, it->second) == false) {
            // RestorePropertyFromDataで読み込まれなかったら読み飛ばす
            settings.AdvDataPos(ckSize);
        }
    }
    mGlobalSettingsHasChanged = false;
}

//-----------------------------------------------------------------------------
void C700Kernel::storeGlobalProperties()
{
    char path[PATH_LEN_MAX];
    getPreferenceFolder(path, PATH_LEN_MAX);
    
    ChunkReader settings(1024*2);
    settings.SetAllowExtend(true);
    settings.SetFilePath(path);
    
    auto it = mPropertyParams.begin();
    while (it != mPropertyParams.end()) {
        if (it->second.saveToGlobal) {
            SetPropertyToChunk(&settings, it->second);
        }
        it++;
    }
    settings.Write();
    
    mGlobalSettingsHasChanged = false;
}

//-----------------------------------------------------------------------------
void C700Kernel::getPreferenceFolder(char *outPath, int inSize)
{
#if MAC
    GetHomeDirectory(outPath, inSize);
    strncat(outPath, "/Library/Application Support/C700/C700.settings", inSize);
#else
    // Windowsのホームフォルダを取得
    SHGetSpecialFolderPath(NULL, outPath, CSIDL_APPDATA, TRUE);
    strncat(outPath, "\\C700\\C700.settings", inSize);
#endif
}

//-----------------------------------------------------------------------------
void C700Kernel::getDocumentsFolder(char *outPath, int inSize)
{
#if MAC
    GetHomeDirectory(outPath, inSize);
    strncat(outPath, "/Documents", inSize);
#else
    // Windowsのホームフォルダを取得
	SHGetSpecialFolderPath(NULL, outPath, CSIDL_MYDOCUMENTS, TRUE);
#endif
}

//-----------------------------------------------------------------------------
void C700Kernel::SetPropertyToChunk(ChunkReader *chunk, const PropertyDescription &prop)
{
    switch (prop.dataType) {
        case propertyDataTypeInt32:
        case propertyDataTypeBool:
        {
            int		intValue = GetPropertyValue(prop.propId);
            chunk->addChunk(prop.propId, &intValue, sizeof(int));
            break;
        }
        case propertyDataTypeFloat32:
        {
            float	floatValue = GetPropertyValue(prop.propId);
            chunk->addChunk(prop.propId, &floatValue, sizeof(float));
            break;
        }
        case propertyDataTypeDouble:
        {
            double	doubleValue = GetPropertyDoubleValue(prop.propId);
            chunk->addChunk(prop.propId, &doubleValue, sizeof(double));
            break;
        }
            break;
        case propertyDataTypeString:
        case propertyDataTypeFilePath:
        {
            char *string = (char*)GetPropertyPtrValue(prop.propId);
            if (string != NULL && string[0] != 0) {
                chunk->addChunk(prop.propId, string, prop.outDataSize);
            }
            break;
        }
        case propertyDataTypeVariableData:
        {
            char *data = (char*)GetPropertyPtrValue(prop.propId);
            if (data != NULL) {
                chunk->addChunk(prop.propId, data, GetPropertyPtrDataSize(prop.propId));
            }
            break;
        }
        case propertyDataTypeStruct:
        case propertyDataTypePointer:
            break;
    }
}

//-----------------------------------------------------------------------------
bool C700Kernel::RestorePropertyFromData(DataBuffer *data, int ckSize, const PropertyDescription &prop)
{
    switch (prop.dataType) {
        case propertyDataTypeFloat32:
        {
            float value;
            data->readData(&value, ckSize);
            SetPropertyValue(prop.propId, value);
            break;
        }
        case propertyDataTypeInt32:
        case propertyDataTypeBool:
        {
            // VSTではBoolもInt32型で保存する仕様とする
            int value;
            data->readData(&value, ckSize);
            SetPropertyValue(prop.propId, value);
            break;
        }
        case propertyDataTypeDouble:
        {
            double value;
            data->readData(&value, ckSize);
            SetPropertyDoubleValue(prop.propId, value);
            break;
        }
        case propertyDataTypeString:
        {
            char	string[PROGRAMNAME_MAX_LEN];
            data->readData(&string, ckSize);
            SetPropertyPtrValue(prop.propId, string, 0);
            break;
        }
        case propertyDataTypeFilePath:
        {
            char	string[PATH_LEN_MAX];
            data->readData(&string, ckSize);
            SetPropertyPtrValue(prop.propId, string, 0);
            break;
        }
        case propertyDataTypeVariableData:
        {
            char    *buf = new char[ckSize];
            data->readData(buf, ckSize);
            SetPropertyPtrValue(prop.propId, buf, ckSize);
            delete [] buf;
            break;
        }
        case propertyDataTypeStruct:
        case propertyDataTypePointer:
            return false;
    }
    if (prop.dataType != propertyDataTypeBool &&
        prop.dataType != propertyDataTypeVariableData) {
        assert(ckSize == prop.outDataSize);
    }
    
    return true;
}

#if AU

//-----------------------------------------------------------------------------
void C700Kernel::AddNumToDictionary(CFMutableDictionaryRef dict, CFStringRef key, int value)
{
	CFNumberRef num = CFNumberCreate(NULL, kCFNumberIntType, &value);
	CFDictionarySetValue(dict, key, num);
	CFRelease(num);
}

//-----------------------------------------------------------------------------
void C700Kernel::AddDoubleToDictionary(CFMutableDictionaryRef dict, CFStringRef key, double value)
{
    CFNumberRef	num = CFNumberCreate(NULL, kCFNumberDoubleType, &value);
	CFDictionarySetValue(dict, key, num);
	CFRelease(num);
}

//-----------------------------------------------------------------------------
void C700Kernel::AddFloatToDictionary(CFMutableDictionaryRef dict, CFStringRef key, float value)
{
    CFNumberRef	num = CFNumberCreate(NULL, kCFNumberFloatType, &value);
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
void C700Kernel::AddStringToDictionary(CFMutableDictionaryRef dict, CFStringRef key, const char *string)
{
    if (string[0] != 0) {
        CFStringRef	str = CFStringCreateWithCString(NULL, string, kCFStringEncodingUTF8);
        CFDictionarySetValue(dict, key, str);
        CFRelease(str);
    }
}

//-----------------------------------------------------------------------------
void C700Kernel::AddFilePathToDictionary(CFMutableDictionaryRef dict, CFStringRef key, const char *path)
{
    if (path[0] != 0) {
        CFURLRef	url =
        CFURLCreateFromFileSystemRepresentation(NULL, (UInt8*)path, strlen(path), false);
        CFDataRef urlData = CFURLCreateData( NULL, url, kCFStringEncodingUTF8, false );
        CFDictionarySetValue(dict, key, urlData);
        CFRelease(urlData);
        CFRelease(url);
    }
}

//-----------------------------------------------------------------------------
void C700Kernel::AddDataToDictionary(CFMutableDictionaryRef dict, CFStringRef key, const void *data, int size)
{
    if (data) {
        CFDataRef	dataRef = CFDataCreate(NULL, (UInt8*)data, size);
        CFDictionarySetValue(dict, key, dataRef);
        CFRelease(dataRef);
    }
}

//-----------------------------------------------------------------------------
void C700Kernel::SetPropertyToDict(CFMutableDictionaryRef dict, const PropertyDescription &prop)
{
    CFStringRef saveKey = CFStringCreateWithCString(NULL, prop.savekey, kCFStringEncodingASCII);
    switch (prop.dataType) {
        case propertyDataTypeInt32:
            AddNumToDictionary(dict, saveKey, GetPropertyValue(prop.propId));
            break;
        case propertyDataTypeFloat32:
            AddFloatToDictionary(dict, saveKey, GetPropertyValue(prop.propId));
            break;
        case propertyDataTypeDouble:
            AddDoubleToDictionary(dict, saveKey, GetPropertyDoubleValue(prop.propId));
            break;
        case propertyDataTypeBool:
            AddBooleanToDictionary(dict, saveKey, GetPropertyValue(prop.propId));
            break;
        case propertyDataTypeString:
            AddStringToDictionary(dict, saveKey, (char*)GetPropertyPtrValue(prop.propId));
            break;
        case propertyDataTypeFilePath:
            AddFilePathToDictionary(dict, saveKey, (char*)GetPropertyPtrValue(prop.propId));
            break;
        case propertyDataTypeVariableData:
            AddDataToDictionary(dict, saveKey, GetPropertyPtrValue(prop.propId), GetPropertyPtrDataSize(prop.propId));
            break;
        case propertyDataTypeStruct:
        case propertyDataTypePointer:
            // 基本的には保存しないタイプのプロパティ
            break;
    }
    CFRelease(saveKey);
}

//-----------------------------------------------------------------------------
int C700Kernel::CreatePGDataDic(CFDictionaryRef *data, int pgnum)
{
    int editProg = mEditProg;
    mEditProg = pgnum;
    
	CFMutableDictionaryRef dict = CFDictionaryCreateMutable	(NULL, 0,
                                                             &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CorrectLoopFlagForSave(pgnum);
    
    auto it = mPropertyParams.begin();
    while (it != mPropertyParams.end()) {
        if (it->second.saveToProg) {
            SetPropertyToDict(dict, it->second);
        }
        it++;
    }
	
    mEditProg = editProg;
    
	*data = dict;
	return 0;
}

//-----------------------------------------------------------------------------
bool C700Kernel::RestorePropertyFromDict(CFDictionaryRef dict, const PropertyDescription &prop)
{
    bool contains = false;
    CFStringRef saveKey = CFStringCreateWithCString(NULL, prop.savekey, kCFStringEncodingASCII);
    if (CFDictionaryContainsKey(dict, saveKey)) {
        
        switch (prop.dataType) {
            case propertyDataTypeFloat32:
            {
                float value;
                CFNumberRef cfnum = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(dict, saveKey));
                CFNumberGetValue(cfnum, kCFNumberFloatType, &value);
                SetPropertyValue(prop.propId, value);
                break;
            }
            case propertyDataTypeInt32:
            {
                int value;
                CFNumberRef cfnum = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(dict, saveKey));
                CFNumberGetValue(cfnum, kCFNumberIntType, &value);
                SetPropertyValue(prop.propId, value);
                break;
            }
            case propertyDataTypeDouble:
            {
                double value;
                CFNumberRef cfnum = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(dict, saveKey));
                CFNumberGetValue(cfnum, kCFNumberDoubleType, &value);
                SetPropertyDoubleValue(prop.propId, value);
                break;
            }
            case propertyDataTypeBool:
            {
                CFBooleanRef cfbool = reinterpret_cast<CFBooleanRef>(CFDictionaryGetValue(dict, saveKey));
                SetPropertyValue(prop.propId,CFBooleanGetValue(cfbool) ? 1.0f:.0f);
                break;
            }
            case propertyDataTypeString:
            {
                char	string[PROGRAMNAME_MAX_LEN];
                CFStringGetCString(reinterpret_cast<CFStringRef>(CFDictionaryGetValue(dict, saveKey)),
                                   string, PROGRAMNAME_MAX_LEN, kCFStringEncodingUTF8);
                SetPropertyPtrValue(prop.propId, string, 0);
                break;
            }
            case propertyDataTypeFilePath:
            {
                CFDataRef	urlData = reinterpret_cast<CFDataRef>(CFDictionaryGetValue(dict, saveKey));
                CFURLRef	url = CFURLCreateWithBytes( NULL, CFDataGetBytePtr(urlData),
                                                       CFDataGetLength(urlData), kCFStringEncodingUTF8, NULL );
                CFStringRef pathStr = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
                char	path[PATH_LEN_MAX];
                CFStringGetCString(pathStr, path, PATH_LEN_MAX-1, kCFStringEncodingUTF8);
                SetPropertyPtrValue(prop.propId, path, 0);
                CFRelease(pathStr);
                CFRelease(url);
                break;
            }
            case propertyDataTypeVariableData:
            {
                CFDataRef	data = reinterpret_cast<CFDataRef>(CFDictionaryGetValue(dict, saveKey));
                SetPropertyPtrValue(prop.propId, CFDataGetBytePtr(data), CFDataGetLength(data));
                break;
            }
            case propertyDataTypeStruct:
            case propertyDataTypePointer:
                break;
        }
        contains = true;
    }
    else {
        // デフォルト値を設定
        if (prop.defaultValue >= 0) {
            // 負数でデフォルト値を無効化する
            // FIRBandが設定されていない場合に係数がデフォルトに戻ってしまうのを防止
            SetPropertyValue(prop.propId, prop.defaultValue);
            SetPropertyDoubleValue(prop.propId, prop.defaultValue);
        }
        contains = false;
    }
    CFRelease(saveKey);
    
    return contains;
}

void C700Kernel::RestorePGDataDic(CFPropertyListRef data, int pgnum)
{
    int editProg = mEditProg;
	mEditProg = pgnum;
    
	CFDictionaryRef dict = static_cast<CFDictionaryRef>(data);
	
    SetProgramName(""); // 文字列型は初期値が設定できないのでここで設定する
    
    bool    isSustainModeSet = false;
    bool    isSourceFileRefSet = false;
    bool    isSR2Set = false;
    
    auto it = mPropertyParams.begin();
    while (it != mPropertyParams.end()) {
        if (it->second.saveToProg) {
            if (RestorePropertyFromDict(dict, it->second)) {
                if (it->first == kAudioUnitCustomProperty_SustainMode) {
                    isSustainModeSet = true;
                }
                if (it->first == kAudioUnitCustomProperty_SourceFileRef) {
                    isSourceFileRefSet = true;
                }
                if (it->first == kAudioUnitCustomProperty_SR2) {
                    isSR2Set = true;
                }
            }
        }
        it++;
    }

	// AU版のSustainModeが無かった期間のバージョンとの互換性のために初期値を適切に設定する
    
	if (isSourceFileRefSet) {
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
    
    if (!isSR2Set) {
        if (GetPropertyValue(kAudioUnitCustomProperty_SustainMode) != 0) {
            SetPropertyValue(kAudioUnitCustomProperty_SR2, GetPropertyValue(kAudioUnitCustomProperty_SR1));
            SetPropertyValue(kAudioUnitCustomProperty_SR1, 0);
        }
    }
	
	//UIに変更をここでは反映しない

    mEditProg = editProg;
	
	GetDriver()->RefreshKeyMap();
}

#else
// VST

//-----------------------------------------------------------------------------
bool C700Kernel::SetPGDataToChunk(ChunkReader *chunk, int pgnum)
{
	if ( chunk->isReadOnly() ) {
		return false;
	}
    
    int editProg = mEditProg;
    mEditProg = pgnum;
    
    CorrectLoopFlagForSave(pgnum);
    
    auto it = mPropertyParams.begin();
    while (it != mPropertyParams.end()) {
        if (it->second.saveToProg) {
            SetPropertyToChunk(chunk, it->second);
        }
        it++;
    }
	
    mEditProg = editProg;

	return true;
}


//-----------------------------------------------------------------------------
int C700Kernel::GetPGChunkSize( int pgnum )
{
    int editProg = mEditProg;
    mEditProg = pgnum;
    
    const InstParams	*vpSet = GetVP();
    
	int cksize = 0;
	if ( vpSet[pgnum].hasBrrData() ) {
        auto it = mPropertyParams.begin();
        while (it != mPropertyParams.end()) {
            if (it->second.saveToProg) {
                cksize += sizeof( ChunkReader::MyChunkHead );
                if (it->second.dataType == propertyDataTypeVariableData) {
                    cksize += GetPropertyPtrDataSize(it->second.propId);
                }
                else if (it->second.dataType == propertyDataTypeBool) {
                    cksize += 4;    // VSTではbool値はint型で保存する
                }
                else {
                    cksize += it->second.outDataSize;
                }
            }
            it++;
        }
	}
    mEditProg = editProg;
    
	return cksize;
}

//-----------------------------------------------------------------------------
bool C700Kernel::RestorePGDataFromChunk( ChunkReader *chunk, int pgnum )
{
    int editProg = mEditProg;
    mEditProg = pgnum;
    
    // デフォルト値の設定
    auto it = mPropertyParams.begin();
    while (it != mPropertyParams.end()) {
        if (it->second.saveToProg) {
            if (it->second.defaultValue >= 0) {
                // 負数でデフォルト値を無効化する
                // FIRBandが設定されていない場合に係数がデフォルトに戻ってしまうのを防止
                SetPropertyValue(it->second.propId, it->second.defaultValue);
                SetPropertyDoubleValue(it->second.propId, it->second.defaultValue);
            }
        }
        it++;
    }
    
    bool isSR2Set = false;
    
	while ( chunk->GetLeftSize() >= (int)sizeof( ChunkReader::MyChunkHead ) ) {
		int		ckType;
		long	ckSize;
		chunk->readChunkHead(&ckType, &ckSize);
        
        if (ckType == kAudioUnitCustomProperty_SR2) {
            isSR2Set = true;
        }
        
        auto it = mPropertyParams.find(ckType);
        if (it == mPropertyParams.end() || it->second.saveToProg == false) {
            //不明チャンクの場合は飛ばす
            chunk->AdvDataPos(ckSize);
        }
        else if (RestorePropertyFromData(chunk, ckSize, it->second) == false) {
            // 特になし
        }
	}
    
    if (!isSR2Set) {
        if (GetPropertyValue(kAudioUnitCustomProperty_SustainMode) != 0) {
            SetPropertyValue(kAudioUnitCustomProperty_SR2, GetPropertyValue(kAudioUnitCustomProperty_SR1));
            SetPropertyValue(kAudioUnitCustomProperty_SR1, 0);
        }
    }
    
    mEditProg = editProg;
    
    GetDriver()->RefreshKeyMap();
    
	return true;
}

#endif
