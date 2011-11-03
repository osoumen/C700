/*
 *  Chip700defines.h
 *  Chip700
 *
 *  Created by 開発用 on 06/10/12.
 *  Copyright 2006 Vermicelli Magic. All rights reserved.
 *
 */


#ifndef __Chip700defines__
#define __Chip700defines__

#pragma mark ____Chip700 Parameters

// parameters
enum {
	kParam_poly=0,
	kParam_volL,	//廃止
	kParam_volR,	//廃止
	kParam_vibdepth,
	kParam_vibrate,
	kParam_vibdepth2,
	//kParam_noise,
	kParam_velocity,
	kParam_bendrange,
	kParam_program,
	kParam_clipnoise,
	kParam_drummode,
	
	kNumberOfParameters
};

// properties
enum
{
	kAudioUnitCustomProperty_First = 64000,

	kAudioUnitCustomProperty_ProgramName = kAudioUnitCustomProperty_First,
	kAudioUnitCustomProperty_BRRData,
	kAudioUnitCustomProperty_Rate,
	kAudioUnitCustomProperty_BaseKey,
	kAudioUnitCustomProperty_LowKey,
	kAudioUnitCustomProperty_HighKey,
	kAudioUnitCustomProperty_LoopPoint,
	kAudioUnitCustomProperty_Loop,
	kAudioUnitCustomProperty_AR,
	kAudioUnitCustomProperty_DR,
	kAudioUnitCustomProperty_SL,
	kAudioUnitCustomProperty_SR,
	kAudioUnitCustomProperty_VolL,
	kAudioUnitCustomProperty_VolR,
	kAudioUnitCustomProperty_EditingProgram,
	kAudioUnitCustomProperty_PGDictionary,
	
	kNumberOfProperties = 16
};

static const float kMinimumValue_n128 = -128;
static const float kMinimumValue_0 = 0;
static const float kMinimumValue_1 = 1;

static const float kMaximumValue_3 = 3;
static const float kMaximumValue_7 = 7;
static const float kMaximumValue_15 = 15;
static const float kMaximumValue_31 = 31;
static const float kMaximumValue_127 = 127;
static const UInt32 kMaximumVoices = 8;


typedef struct {
	int				size;
	unsigned char	*data;
} BRRData;

typedef struct {
	CFStringRef			pgname;
	int					ar,dr,sl,sr;
	int					volL,volR;
	double				rate;
	int					basekey,lowkey,highkey;
	int					lp;
	bool				loop;
	BRRData				brr;
} VoiceParams;



#endif
