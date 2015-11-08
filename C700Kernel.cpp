/*
 *  C700Kernel.cpp
 *  C700
 *
 *  Created by osoumen on 12/10/12.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "C700Kernel.h"
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
	mGenerator.SetVPSet(mVPset);
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
	mGenerator.Reset();
	
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
			mGenerator.SetVoiceLimit(value);
			break;
			
		case kParam_mainvol_L:
			mGenerator.SetMainVol_L(value);
			break;
			
		case kParam_mainvol_R:
			mGenerator.SetMainVol_R(value);
			break;
			
		case kParam_vibdepth:
			mGenerator.ModWheel(0, value);
			break;
			
		case kParam_vibrate:
			mGenerator.SetVibFreq(-1, value);   // -1が全チャンネルを表す
			break;
			
		case kParam_vibdepth2:
			mGenerator.SetVibDepth(-1, value);  // -1は全チャンネルを表す
			break;
			
		case kParam_velocity:
			switch ( (int)value ) {
				case 0:
					mGenerator.SetVelocityMode( kVelocityMode_Constant );
					break;
				case 1:
					mGenerator.SetVelocityMode( kVelocityMode_Square );
					break;
				case 2:
					mGenerator.SetVelocityMode( kVelocityMode_Linear );
					break;
				default:
					mGenerator.SetVelocityMode( kVelocityMode_Square );
			}
			break;
			
		case kParam_bendrange:
			mGenerator.SetPBRange(value);
			break;
			
		case kParam_program:
			mGenerator.ProgramChange(0, value, 0);
			break;
			
		case kParam_newadpcm:
			mGenerator.SetADPCMMode( value==0 ? false:true );
			break;
			
		case kParam_bankAmulti:
			mGenerator.SetMultiMode(0, value==0 ? false:true);
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
			mGenerator.ProgramChange( id - kParam_program_2 + 1, value, 0 );
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
			mGenerator.ModWheel( id - kParam_vibdepth_2 + 1, value);
			break;
			
		case kParam_echovol_L:
			mGenerator.SetEchoVol_L( value );
			break;
			
		case kParam_echovol_R:
			mGenerator.SetEchoVol_R( value );
			break;
			
		case kParam_echoFB:
			mGenerator.SetFeedBackLevel(value);
			break;
			
		case kParam_echodelay:
			mGenerator.SetDelayTime(value);
			break;
			
		case kParam_fir0:
		case kParam_fir1:
		case kParam_fir2:
		case kParam_fir3:
		case kParam_fir4:
		case kParam_fir5:
		case kParam_fir6:
		case kParam_fir7:
			mGenerator.SetFIRTap( id - kParam_fir0, value );
			break;
			
		case kParam_bankBmulti:
			mGenerator.SetMultiMode(1, value==0 ? false:true);
			break;
			
		case kParam_bankCmulti:
			mGenerator.SetMultiMode(2, value==0 ? false:true);
			break;
			
		case kParam_bankDmulti:
			mGenerator.SetMultiMode(3, value==0 ? false:true);
			break;
			
        case kParam_alwaysDelayNote:
            mGenerator.SetEventDelayClocks(value==0 ? 0:8192);
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
			
		case kAudioUnitCustomProperty_SourceFileRef:
		case kAudioUnitCustomProperty_BRRData:
		case kAudioUnitCustomProperty_ProgramName:
		case kAudioUnitCustomProperty_PGDictionary:
		case kAudioUnitCustomProperty_XIData:
		default:
			return 0;
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
			mGenerator.RefreshKeyMap();
			return true;
		case kAudioUnitCustomProperty_LowKey:
			mVPset[mEditProg].lowkey = value;
			mGenerator.RefreshKeyMap();
			return true;
		case kAudioUnitCustomProperty_HighKey:
			mVPset[mEditProg].highkey = value;
			mGenerator.RefreshKeyMap();
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
            mGenerator.UpdateLoopPoint(mEditProg);
			return true;
			
		case kAudioUnitCustomProperty_Loop:
			mVPset[mEditProg].loop = boolData;
            if (boolData) {
                mVPset[mEditProg].setLoop();
            }
            else {
                mVPset[mEditProg].unsetLoop();
            }
            mGenerator.UpdateLoopFlag(mEditProg);
			return true;
			
		case kAudioUnitCustomProperty_Echo:
			mVPset[mEditProg].echo = boolData;
			return true;
			
		case kAudioUnitCustomProperty_Bank:
			mVPset[mEditProg].bank = value;
            mGenerator.RefreshKeyMap();
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
            mGenerator.UpdatePortamentoTime(mEditProg);
            return true;

        case kAudioUnitCustomProperty_NoteOnPriority:
            mVPset[mEditProg].noteOnPriority = value;
            mGenerator.AllNotesOff();
            return true;
            
        case kAudioUnitCustomProperty_ReleasePriority:
            mVPset[mEditProg].releasePriority = value;
            mGenerator.AllNotesOff();
            return true;

		case kAudioUnitCustomProperty_SourceFileRef:
		case kAudioUnitCustomProperty_BRRData:
		case kAudioUnitCustomProperty_ProgramName:
		case kAudioUnitCustomProperty_PGDictionary:
		case kAudioUnitCustomProperty_XIData:			
		case kAudioUnitCustomProperty_TotalRAM:
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
        mGenerator.SetBrrSample(prog, data, size, mVPset[prog].lp);
	}
	else {
        //NULLデータをセットされると削除を行う
		if (mVPset[prog].hasBrrData()) {
            mVPset[prog].releaseBrr();
			mVPset[prog].pgname[0] = 0;
            // 波形メモリの解放
            mGenerator.DelBrrSample(prog);
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
	
	mGenerator.RefreshKeyMap();
	
	return true;
}

//-----------------------------------------------------------------------------

void C700Kernel::Render( unsigned int frames, float *output[2] )
{
	mGenerator.Process(frames, output);
    
    // MIDIインジケーターへの反映
    for (int i=0; i<16; i++) {
        int onNotes = mGenerator.GetNoteOnNotes(i);
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
}

//-----------------------------------------------------------------------------

void C700Kernel::HandleNoteOn( int ch, int note, int vel, int uniqueID, int inFrame )
{
	mGenerator.NoteOn(ch, note, vel, uniqueID, inFrame);
}

//-----------------------------------------------------------------------------

void C700Kernel::HandleNoteOff( int ch, int note, int uniqueID, int inFrame )
{
	mGenerator.NoteOff(ch, note, 0, uniqueID, inFrame);
}

//-----------------------------------------------------------------------------

void C700Kernel::HandlePitchBend( int ch, int pitch1, int pitch2, int inFrame )
{
	mGenerator.PitchBend(ch, pitch1, pitch2, inFrame);
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
           mGenerator.ControlChange( ch, controlNum, value, inFrame);
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
                mGenerator.SetPBRange(ch, mDataEntryValue[ch]);
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
    mGenerator.ProgramChange(ch, pg, inFrame);
}

//-----------------------------------------------------------------------------

void C700Kernel::HandleResetAllControllers( int ch, int inFrame )
{
	mGenerator.ResetAllControllers();
}

//-----------------------------------------------------------------------------

void C700Kernel::HandleAllNotesOff( int ch, int inFrame )
{
	mGenerator.AllNotesOff();
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
	mGenerator.AllSoundOff();
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
