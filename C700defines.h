/*
 *  C700defines.h
 *  C700
 *
 *  Created by osoumen on 06/10/12.
 *  Copyright 2006 ppse All rights reserved.
 *
 */


#pragma once

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
    kParam_engine,
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
    
    //Å´ÉGÉRÅ[ïî
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
    
    kParam_bankBmulti,
    kParam_bankCmulti,
    kParam_bankDmulti,
    
    kParam_alwaysDelayNote,
    kParam_voiceAllocMode,
    kParam_fastReleaseAsKeyOff,
    
    kNumberOfParameters
};

// control commands
enum 
{
    kControlCommandsFirst = 70000,
    
    kControlBankDBtn = kControlCommandsFirst,
    kControlBankCBtn,
    kControlBankBBtn,
    kControlBankABtn,
    
    kControlXMSNESText,
    
    kControlButtonCopy,
    kControlButtonPreemphasis,
    kControlButtonUnload,
    kControlButtonLoad,
    kControlButtonSave,
    kControlButtonSaveXI,
    kControlButtonAutoSampleRate,
    kControlButtonAutoKey,
    kControlButtonChangeLoopPoint,
    kControlButtonChangeProgram,
    
    kControlSelectTrack16,
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
    kControlSelectTrack1,
    
    kControlButtonKhaos,
    
    kControlButtonOpenRecordingSettings,
    kControlButtonChooseRecordPath,
    kControlButtonSetRecordStart,
    kControlButtonSetRecordLoopStart,
    kControlButtonSetRecordEnd,
    kControlButtonLoadPlayerCode,
    kControlButtonRecordSettingExit
};

enum InstChangeFlag {
    HAS_PGNAME          = 1 << 0,
    HAS_RATE            = 1 << 1,
    HAS_BASEKEY         = 1 << 2,
    HAS_LOWKEY          = 1 << 3,
    HAS_HIGHKEY         = 1 << 4,
    HAS_AR              = 1 << 5,
    HAS_DR              = 1 << 6,
    HAS_SL              = 1 << 7,
    HAS_SR              = 1 << 8,
    HAS_VOLL            = 1 << 9,
    HAS_VOLR            = 1 << 10,
    HAS_ECHO            = 1 << 11,
    HAS_BANK            = 1 << 12,
    HAS_ISEMPHASIZED    = 1 << 13,
    HAS_SOURCEFILE      = 1 << 14,
    HAS_SUSTAINMODE     = 1 << 15,
    HAS_MONOMODE        = 1 << 16,
    HAS_PORTAMENTOON    = 1 << 17,
    HAS_PORTAMENTORATE  = 1 << 18,
    HAS_NOTEONPRIORITY  = 1 << 19,
    HAS_RELEASEPRIORITY = 1 << 20
};

static const int kMaximumVoices = 16;
static const int NUM_BANKS = 4;
static const int NUM_PRESETS = 2;
static const int NUM_OUTPUTS = 2;

static const int PROGRAMNAME_MAX_LEN = 256;
//static const int PATH_LEN_MAX = 1024;

static const int BRR_STARTADDR = 0x600;
static const int BRR_ENDADDR = 0x10000;

static const int CKID_PROGRAM_TOTAL = 0x20000;
static const int CKID_PROGRAM_DATA  = 0x30000;

static const int kDefaultValue_AR = 15;
static const int kDefaultValue_DR = 7;
static const int kDefaultValue_SL = 7;
static const int kDefaultValue_SR = 31;

static const bool kDefaultValue_SustainMode = true;
static const bool kDefaultValue_MonoMode = false;
static const bool kDefaultValue_PortamentoOn = false;
static const int kDefaultValue_PortamentoRate = 0;
static const int kDefaultValue_NoteOnPriority = 64;
static const int kDefaultValue_ReleasePriority = 0;

typedef struct BRRData {
    int             size;
    unsigned char   *data;
    BRRData() : size(0), data(0L) {}
    int samples() const { return (size/9)*16; }
} BRRData;

typedef struct {
public:
    char        pgname[PROGRAMNAME_MAX_LEN];
    int         ar,dr,sl,sr;
    int         volL,volR;
    double      rate;
    int         basekey,lowkey,highkey;
    int         lp;
    bool        loop;
    bool        echo;
    int         bank;
    char        sourceFile[PATH_LEN_MAX];
    bool        isEmphasized;
    bool        sustainMode;
    bool        monoMode;
    bool        portamentoOn;
    int         portamentoRate;
    int         noteOnPriority;
    int         releasePriority;
    
    bool        hasBrrData() const
    {
        return (brr.data != 0L)?true:false;
    }
    void        setLoop()
    {
        if (brr.size >= 9) {
            brr.data[brr.size - 9] |= 2;
        }
    }
    void        unsetLoop()
    {
        if (brr.size >= 9) {
            brr.data[brr.size - 9] &= ~2;
        }
    }
    bool        isLoop() const
    {
        if (brr.size < 9) {
            return false;
        }
        return (brr.data[brr.size - 9] & 2)?true:false;
    }
    unsigned char *brrData() const { return brr.data; }
    int brrSize() const { return brr.size; }
    int brrSamples() const { return (brr.size/9)*16; }
    int brrLpSamples() const { return (lp/9)*16; }
    const BRRData *getBRRData() const { return &brr; }
    void setBRRData(const BRRData *srcbrr) { brr = *srcbrr; }
    void releaseBrr() {
        if (brr.data) {
            delete [] brr.data;
        }
        brr.data = 0L;
        brr.size = 0;
    }
private:
    BRRData     brr;    
} InstParams;

float ConvertToVSTValue( float value, float min, float max );
float ConvertFromVSTValue( float value, float min, float max );