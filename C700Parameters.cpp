/*
 *  C700Parameters.cpp
 *  Chip700
 *
 *  Created by osoumen on 12/10/13.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "C700Parameters.h"

static const float kMinimumValue_n128 = -128;
static const float kMinimumValue_0 = 0;
static const float kMinimumValue_1 = 1;

static const float kMaximumValue_3 = 3;
static const float kMaximumValue_7 = 7;
static const float kMaximumValue_15 = 15;
static const float kMaximumValue_31 = 31;
static const float kMaximumValue_127 = 127;


static const float kDefaultValue_poly = 8;
static const float kDefaultValue_mainvol_L = 64;
static const float kDefaultValue_mainvol_R = 64;
static const float kDefaultValue_vibdepth = 0;
static const float kDefaultValue_vibrate = 7;
static const float kMinimumValue_vibrate = 0.01;
static const float kMaximumValue_vibrate = 35;
static const float kDefaultValue_vibdepth2 = 1;
static const float kDefaultValue_velocity = 1;
static const float kDefaultValue_clipnoise = 1;
static const float kDefaultValue_bendrange = 2;
static const float kDefaultValue_program = 0;
static const float kDefaultValue_bankAmulti = 0;
static const float kDefaultValue_bankBmulti = 0;
static const float kDefaultValue_bankCmulti = 0;
static const float kDefaultValue_bankDmulti = 0;

//エコー部
static const float kDefaultValue_echovol_L = 50;
static const float kDefaultValue_echovol_R = -50;
static const float kDefaultValue_echoFB = -70;
static const float kDefaultValue_echodelay = 6;
static const float kDefaultValue_fir0 = 127;
static const float kDefaultValue_fir1 = 0;
static const float kDefaultValue_fir2 = 0;
static const float kDefaultValue_fir3 = 0;
static const float kDefaultValue_fir4 = 0;
static const float kDefaultValue_fir5 = 0;
static const float kDefaultValue_fir6 = 0;
static const float kDefaultValue_fir7 = 0;


//-----------------------------------------------------------------------------
float ConvertToVSTValue( float value, float min, float max )
{
	return (value - min) / (max - min);
}

//-----------------------------------------------------------------------------
float ConvertFromVSTValue( float value, float min, float max )
{
	return (value * (max - min) + min);
}


//-----------------------------------------------------------------------------
C700Parameters::C700Parameters(int numParams)
{
	mNumParams = numParams;
	mParams = new float[numParams];
	for ( int i=0; i<numParams; i++ ) {
		mParams[i] = .0f;
	}
}

//-----------------------------------------------------------------------------
C700Parameters::~C700Parameters()
{
	delete [] mParams;
}

//-----------------------------------------------------------------------------

bool C700Parameters::SetParameter( int id, float value )
{
	if ( id < mNumParams && id >= 0 ) {
		mParams[id] = value;
		bounceValue(id);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
void C700Parameters::bounceValue( int id )
{
	if ( mParams[id] > GetParameterMax(id) ) {
		mParams[id] = GetParameterMax(id);
	}
	if ( mParams[id] < GetParameterMin(id) ) {
		mParams[id] = GetParameterMin(id);
	}
}

//-----------------------------------------------------------------------------
float C700Parameters::GetParameter( int id )
{
	if ( id < mNumParams && id >= 0 ) {
		return mParams[id];
	}
	return .0f;
}
//-----------------------------------------------------------------------------
const char *C700Parameters::GetParameterName( int id )
{
	static char paramName[kNumberOfParameters][64] = {
		"Voices",
		"MainVol(L)",
		"MainVol(R)",
		"Vibrato Depth",
		"Vibrato Rate",
		"Vibrato Depth2",
		"Velocity",
		"Bend Range",
		"Program",
		"New ADPCM",
		"Bank A Multi",
		
		"Program2",
		"Program3",
		"Program4",
		"Program5",
		"Program6",
		"Program7",
		"Program8",
		"Program9",
		"Program10",
		"Program11",
		"Program12",
		"Program13",
		"Program14",
		"Program15",
		"Program16",
		
		"Vibrato Depth2",
		"Vibrato Depth3",
		"Vibrato Depth4",
		"Vibrato Depth5",
		"Vibrato Depth6",
		"Vibrato Depth7",
		"Vibrato Depth8",
		"Vibrato Depth9",
		"Vibrato Depth10",
		"Vibrato Depth11",
		"Vibrato Depth12",
		"Vibrato Depth13",
		"Vibrato Depth14",
		"Vibrato Depth15",
		"Vibrato Depth16",
		
		"Echo(L)",
		"Echo(R)",
		"FeedBack",
		"Delay",
		"Filter a0",
		"Filter a1",
		"Filter a2",
		"Filter a3",
		"Filter a4",
		"Filter a5",
		"Filter a6",
		"Filter a7",
		
		"Bank B Multi",
		"Bank C Multi",
		"Bank D Multi"
	};
	return paramName[id];
}

//-----------------------------------------------------------------------------
/*static*/ float C700Parameters::GetParameterMax( int id )
{
	switch(id)
	{
		case kParam_poly:
			return kMaximumVoices;
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
			return kMaximumValue_127;
		case kParam_vibrate:
			return kMaximumValue_vibrate;
		case kParam_vibdepth2:
			return kMaximumValue_15;
		case kParam_velocity:
			return 2;
		case kParam_clipnoise:
			return 1;
		case kParam_bendrange:
			return 24;
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
			return kMaximumValue_127;
			
		case kParam_bankAmulti:
			return 1;
		case kParam_bankBmulti:
			return 1;
		case kParam_bankCmulti:
			return 1;
		case kParam_bankDmulti:
			return 1;
			
			//エコー
		case kParam_mainvol_L:
			return kMaximumValue_127;
		case kParam_mainvol_R:
			return kMaximumValue_127;
		case kParam_echovol_L:
			return kMaximumValue_127;
		case kParam_echovol_R:
			return kMaximumValue_127;
		case kParam_echoFB:
			return kMaximumValue_127;
		case kParam_echodelay:
			return kMaximumValue_15;
		case kParam_fir0:
			return kMaximumValue_127;
		case kParam_fir1:
			return kMaximumValue_127;
		case kParam_fir2:
			return kMaximumValue_127;
		case kParam_fir3:
			return kMaximumValue_127;
		case kParam_fir4:
			return kMaximumValue_127;
		case kParam_fir5:
			return kMaximumValue_127;
		case kParam_fir6:
			return kMaximumValue_127;
		case kParam_fir7:
			return kMaximumValue_127;
			
		default:
			return 0;
	}
}

//-----------------------------------------------------------------------------

/*static*/ float C700Parameters::GetParameterMin( int id )
{
	switch(id)
	{
		case kParam_poly:
			return kMinimumValue_1;
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
			return kMinimumValue_0;
		case kParam_vibrate:
			return kMinimumValue_vibrate;
		case kParam_vibdepth2:
			return kMinimumValue_1;
		case kParam_velocity:
			return kMinimumValue_0;
		case kParam_clipnoise:
			return kMinimumValue_0;
		case kParam_bendrange:
			return 1;
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
			return kMinimumValue_0;
			
		case kParam_bankAmulti:
			return kMinimumValue_0;
		case kParam_bankBmulti:
			return kMinimumValue_0;
		case kParam_bankCmulti:
			return kMinimumValue_0;
		case kParam_bankDmulti:
			return kMinimumValue_0;
			
			//エコー
		case kParam_mainvol_L:
			return kMinimumValue_n128;
		case kParam_mainvol_R:
			return kMinimumValue_n128;
		case kParam_echovol_L:
			return kMinimumValue_n128;
		case kParam_echovol_R:
			return kMinimumValue_n128;
		case kParam_echoFB:
			return kMinimumValue_n128;
		case kParam_echodelay:
			return kMinimumValue_0;
		case kParam_fir0:
			return kMinimumValue_n128;
		case kParam_fir1:
			return kMinimumValue_n128;
		case kParam_fir2:
			return kMinimumValue_n128;
		case kParam_fir3:
			return kMinimumValue_n128;
		case kParam_fir4:
			return kMinimumValue_n128;
		case kParam_fir5:
			return kMinimumValue_n128;
		case kParam_fir6:
			return kMinimumValue_n128;
		case kParam_fir7:
			return kMinimumValue_n128;
			
		default:
			return 0;
	}	
}

//-----------------------------------------------------------------------------

/*static*/ float C700Parameters::GetParameterDefault( int id )
{
	switch(id)
	{
		case kParam_poly:
			return kDefaultValue_poly;
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
			return kDefaultValue_vibdepth;
		case kParam_vibrate:
			return kDefaultValue_vibrate;
		case kParam_vibdepth2:
			return kDefaultValue_vibdepth2;
		case kParam_velocity:
			return kDefaultValue_velocity;
		case kParam_clipnoise:
			return kDefaultValue_clipnoise;
		case kParam_bendrange:
			return kDefaultValue_bendrange;
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
			return kDefaultValue_program;
			
		case kParam_bankAmulti:
			return kDefaultValue_bankAmulti;
		case kParam_bankBmulti:
			return kDefaultValue_bankBmulti;
		case kParam_bankCmulti:
			return kDefaultValue_bankCmulti;
		case kParam_bankDmulti:
			return kDefaultValue_bankDmulti;
			
			//エコー
		case kParam_mainvol_L:
			return kDefaultValue_mainvol_L;
		case kParam_mainvol_R:
			return kDefaultValue_mainvol_R;
		case kParam_echovol_L:
			return kDefaultValue_echovol_L;
		case kParam_echovol_R:
			return kDefaultValue_echovol_R;
		case kParam_echoFB:
			return kDefaultValue_echoFB;
		case kParam_echodelay:
			return kDefaultValue_echodelay;
		case kParam_fir0:
			return kDefaultValue_fir0;
		case kParam_fir1:
			return kDefaultValue_fir1;
		case kParam_fir2:
			return kDefaultValue_fir2;
		case kParam_fir3:
			return kDefaultValue_fir3;
		case kParam_fir4:
			return kDefaultValue_fir4;
		case kParam_fir5:
			return kDefaultValue_fir5;
		case kParam_fir6:
			return kDefaultValue_fir6;
		case kParam_fir7:
			return kDefaultValue_fir7;
			
		default:
			return .0f;
	}	
}
