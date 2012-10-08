/*
 *  Chip700defines.h
 *  Chip700
 *
 *  Created by osoumen on 06/10/12.
 *  Copyright 2006 ppse部 All rights reserved.
 *
 */


#pragma once

#include "vstgui.h"

#pragma mark ____Chip700 Parameters

// parameters
enum {
	kParam_poly=0,
	kParam_mainvol_L,
	kParam_mainvol_R,
	kParam_vibdepth,
	kParam_vibrate,
	kParam_vibdepth2,
	kParam_velocity,
	kParam_bendrange,
	kParam_program,
	kParam_clipnoise,
	kParam_bankAmulti,
	
	kParam_program_2,
	kParam_program_3,
	kParam_program_4,
	kParam_program_5,
	kParam_program_6,
	kParam_program_7,
	kParam_program_8,
	kParam_program_9,
	kParam_program_10,
	kParam_program_11,
	kParam_program_12,
	kParam_program_13,
	kParam_program_14,
	kParam_program_15,
	kParam_program_16,
	
	kParam_vibdepth_2,
	kParam_vibdepth_3,
	kParam_vibdepth_4,
	kParam_vibdepth_5,
	kParam_vibdepth_6,
	kParam_vibdepth_7,
	kParam_vibdepth_8,
	kParam_vibdepth_9,
	kParam_vibdepth_10,
	kParam_vibdepth_11,
	kParam_vibdepth_12,
	kParam_vibdepth_13,
	kParam_vibdepth_14,
	kParam_vibdepth_15,
	kParam_vibdepth_16,
	
	//↓エコー部
	kParam_echovol_L,
	kParam_echovol_R,
	kParam_echoFB,
	kParam_echodelay,
	kParam_fir0,
	kParam_fir1,
	kParam_fir2,
	kParam_fir3,
	kParam_fir4,
	kParam_fir5,
	kParam_fir6,
	kParam_fir7,
	
	kParam_bankBmulti,			// 追加 12.04.16
	kParam_bankCmulti,			// 追加 12.04.16
	kParam_bankDmulti,			// 追加 12.04.16
	
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
	kAudioUnitCustomProperty_Echo,
	kAudioUnitCustomProperty_Bank,			// 追加 12.04.16
	kAudioUnitCustomProperty_EditingProgram,
	kAudioUnitCustomProperty_EditingChannel,
	
	//↓エコー部
	kAudioUnitCustomProperty_Band1,
	kAudioUnitCustomProperty_Band2,
	kAudioUnitCustomProperty_Band3,
	kAudioUnitCustomProperty_Band4,
	kAudioUnitCustomProperty_Band5,
	
	kAudioUnitCustomProperty_TotalRAM,			// read only
	
	kAudioUnitCustomProperty_PGDictionary,
	kAudioUnitCustomProperty_XIData,			// read only
	
	kAudioUnitCustomProperty_NoteOnTrack_1,		// read only
	kAudioUnitCustomProperty_NoteOnTrack_2,		// read only
	kAudioUnitCustomProperty_NoteOnTrack_3,		// read only
	kAudioUnitCustomProperty_NoteOnTrack_4,		// read only
	kAudioUnitCustomProperty_NoteOnTrack_5,		// read only
	kAudioUnitCustomProperty_NoteOnTrack_6,		// read only
	kAudioUnitCustomProperty_NoteOnTrack_7,		// read only
	kAudioUnitCustomProperty_NoteOnTrack_8,		// read only
	kAudioUnitCustomProperty_NoteOnTrack_9,		// read only
	kAudioUnitCustomProperty_NoteOnTrack_10,	// read only
	kAudioUnitCustomProperty_NoteOnTrack_11,	// read only
	kAudioUnitCustomProperty_NoteOnTrack_12,	// read only
	kAudioUnitCustomProperty_NoteOnTrack_13,	// read only
	kAudioUnitCustomProperty_NoteOnTrack_14,	// read only
	kAudioUnitCustomProperty_NoteOnTrack_15,	// read only
	kAudioUnitCustomProperty_NoteOnTrack_16,	// read only
	
	kAudioUnitCustomProperty_MaxNoteTrack_1,	// read only
	kAudioUnitCustomProperty_MaxNoteTrack_2,	// read only
	kAudioUnitCustomProperty_MaxNoteTrack_3,	// read only
	kAudioUnitCustomProperty_MaxNoteTrack_4,	// read only
	kAudioUnitCustomProperty_MaxNoteTrack_5,	// read only
	kAudioUnitCustomProperty_MaxNoteTrack_6,	// read only
	kAudioUnitCustomProperty_MaxNoteTrack_7,	// read only
	kAudioUnitCustomProperty_MaxNoteTrack_8,	// read only
	kAudioUnitCustomProperty_MaxNoteTrack_9,	// read only
	kAudioUnitCustomProperty_MaxNoteTrack_10,	// read only
	kAudioUnitCustomProperty_MaxNoteTrack_11,	// read only
	kAudioUnitCustomProperty_MaxNoteTrack_12,	// read only
	kAudioUnitCustomProperty_MaxNoteTrack_13,	// read only
	kAudioUnitCustomProperty_MaxNoteTrack_14,	// read only
	kAudioUnitCustomProperty_MaxNoteTrack_15,	// read only
	kAudioUnitCustomProperty_MaxNoteTrack_16,	// read only
	
	kAudioUnitCustomProperty_SourceFileRef,
	kAudioUnitCustomProperty_IsEmaphasized,
	
	kNumberOfProperties = kAudioUnitCustomProperty_MaxNoteTrack_16-kAudioUnitCustomProperty_First+1
};

// control commands
enum 
{
	kControlCommandsFirst = 70000,
	
	kControlBankDBtn = 70000,
	kControlBankCBtn,
	kControlBankBBtn,
	kControlBankABtn,
	
	kControlXMSNESText = 70004,
	
	kControlSelectTrack16 = 70015,
	kControlSelectTrack15,
	kControlSelectTrack14,
	kControlSelectTrack13,
	kControlSelectTrack12,
	kControlSelectTrack11,
	kControlSelectTrack10,
	kControlSelectTrack9,
	kControlSelectTrack8,
	kControlSelectTrack7,
	kControlSelectTrack6,
	kControlSelectTrack5,
	kControlSelectTrack4,
	kControlSelectTrack3,
	kControlSelectTrack2,
	kControlSelectTrack1
};

static const float kMinimumValue_n128 = -128;
static const float kMinimumValue_0 = 0;
static const float kMinimumValue_1 = 1;

static const float kMaximumValue_3 = 3;
static const float kMaximumValue_7 = 7;
static const float kMaximumValue_15 = 15;
static const float kMaximumValue_31 = 31;
static const float kMaximumValue_127 = 127;

static const int kMaximumVoices = 16;

static const int NUM_BANKS = 4;

typedef struct {
	int				size;
	unsigned char	*data;
} BRRData;


typedef struct {
	int		basekey,lowkey,highkey,loop,lp,lp_end;
	double	srcSamplerate;
} InstData;

typedef struct {
	CFStringRef			pgname;
	int					ar,dr,sl,sr;
	int					volL,volR;
	double				rate;
	int					basekey,lowkey,highkey;
	int					lp;
	bool				loop;
	bool				echo;
	int					bank;			// 追加 12.04.16
	BRRData				brr;
	CFURLRef			sourceFile;
	bool				isEmphasized;
} VoiceParams;

typedef struct {
	char extxi[21];		// Extended Instrument:
	char name[23];		// Name, 1Ah
	char trkname[20];	// FastTracker v2.00
	unsigned short shsize;		// 0x0102
} XIFILEHEADER;

typedef struct {
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

typedef struct {
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
	unsigned long manufacturer;
	unsigned long product;
	unsigned long sample_period;
	unsigned long note;
	unsigned long pitch_fraction;
	unsigned long smpte_format;
	unsigned long smpte_offset;
	unsigned long loops;
	unsigned long sampler_data;
	
	unsigned long cue_id;
	unsigned long type;
	unsigned long start;
	unsigned long end;
	unsigned long fraction;
	unsigned long play_count;
} WAV_smpl;


typedef struct {
	unsigned int	kind_sig;
	unsigned int	kind;
	char			title[256];
	int				value;
	int				minimum;
	int				maximum;
	unsigned int	sig;
	int				id;
	unsigned int	command;
	int				x;
	int				y;
	int				w;
	int				h;
	char			fontname[256];
	int				fontsize;
	CHoriTxtAlign	fontalign;
	int				style;
	int				futureuse;
} ControlInstances;
