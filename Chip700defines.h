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
	kAudioUnitCustomProperty_XIData,
	
	kNumberOfProperties = 17
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

typedef struct XIFILEHEADER
{
	char extxi[21];		// Extended Instrument:
	char name[23];		// Name, 1Ah
	char trkname[20];	// FastTracker v2.00
	unsigned short shsize;		// 0x0102
} XIFILEHEADER;

typedef struct XIINSTRUMENTHEADER
{
	unsigned char snum[96];
	unsigned short venv[24];
	unsigned short penv[24];
	unsigned char vnum, pnum;
	unsigned char vsustain, vloops, vloope, psustain, ploops, ploope;
	unsigned char vtype, ptype;
	unsigned char vibtype, vibsweep, vibdepth, vibrate;
	unsigned short volfade;
	unsigned short res;
	unsigned char reserved1[20];
	unsigned short reserved2;		//nsamples?
} XIINSTRUMENTHEADER;

typedef struct XISAMPLEHEADER
{
	unsigned long samplen;
	unsigned long loopstart;
	unsigned long looplen;
	unsigned char vol;
	signed char finetune;
	unsigned char type;
	unsigned char pan;
	signed char relnote;
	unsigned char res;
	char name[22];
} XISAMPLEHEADER;

typedef struct {
	UInt32 manufacturer;
	UInt32 product;
	UInt32 sample_period;
	UInt32 note;
	UInt32 pitch_fraction;
	UInt32 smpte_format;
	UInt32 smpte_offset;
	UInt32 loops;
	UInt32 sampler_data;
	
	UInt32 cue_id;
	UInt32 type;
	UInt32 start;
	UInt32 end;
	UInt32 fraction;
	UInt32 play_count;
} WAV_smpl;
/*
typedef struct {
	UInt32 id;
	UInt32 type;
	UInt32 start;
	UInt32 end;	
	UInt32 fraction;
	UInt32 play_count;
} WAV_Loop;
*/
#endif
