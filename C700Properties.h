//
//  C700Properties.h
//  C700
//
//  Created by osoumen on 2017/02/16.
//
//

#ifndef __C700__C700Properties__
#define __C700__C700Properties__

#include "C700defines.h"
#include <map>


// properties
enum
{
    kAudioUnitCustomProperty_Begin = 64000,
    
    kAudioUnitCustomProperty_ProgramName = kAudioUnitCustomProperty_Begin,
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
    kAudioUnitCustomProperty_Bank,
    kAudioUnitCustomProperty_EditingProgram,
    kAudioUnitCustomProperty_EditingChannel,
    
    //↓エコー部
    kAudioUnitCustomProperty_Band1,
    kAudioUnitCustomProperty_Band2,
    kAudioUnitCustomProperty_Band3,
    kAudioUnitCustomProperty_Band4,
    kAudioUnitCustomProperty_Band5,
    
    kAudioUnitCustomProperty_TotalRAM,          // read only
    
    kAudioUnitCustomProperty_PGDictionary,
    kAudioUnitCustomProperty_XIData,            // read only
    
    kAudioUnitCustomProperty_NoteOnTrack_1,     // read only
    kAudioUnitCustomProperty_NoteOnTrack_2,     // read only
    kAudioUnitCustomProperty_NoteOnTrack_3,     // read only
    kAudioUnitCustomProperty_NoteOnTrack_4,     // read only
    kAudioUnitCustomProperty_NoteOnTrack_5,     // read only
    kAudioUnitCustomProperty_NoteOnTrack_6,     // read only
    kAudioUnitCustomProperty_NoteOnTrack_7,     // read only
    kAudioUnitCustomProperty_NoteOnTrack_8,     // read only
    kAudioUnitCustomProperty_NoteOnTrack_9,     // read only
    kAudioUnitCustomProperty_NoteOnTrack_10,    // read only
    kAudioUnitCustomProperty_NoteOnTrack_11,    // read only
    kAudioUnitCustomProperty_NoteOnTrack_12,    // read only
    kAudioUnitCustomProperty_NoteOnTrack_13,    // read only
    kAudioUnitCustomProperty_NoteOnTrack_14,    // read only
    kAudioUnitCustomProperty_NoteOnTrack_15,    // read only
    kAudioUnitCustomProperty_NoteOnTrack_16,    // read only
    
    kAudioUnitCustomProperty_MaxNoteTrack_1,    // read only
    kAudioUnitCustomProperty_MaxNoteTrack_2,    // read only
    kAudioUnitCustomProperty_MaxNoteTrack_3,    // read only
    kAudioUnitCustomProperty_MaxNoteTrack_4,    // read only
    kAudioUnitCustomProperty_MaxNoteTrack_5,    // read only
    kAudioUnitCustomProperty_MaxNoteTrack_6,    // read only
    kAudioUnitCustomProperty_MaxNoteTrack_7,    // read only
    kAudioUnitCustomProperty_MaxNoteTrack_8,    // read only
    kAudioUnitCustomProperty_MaxNoteTrack_9,    // read only
    kAudioUnitCustomProperty_MaxNoteTrack_10,   // read only
    kAudioUnitCustomProperty_MaxNoteTrack_11,   // read only
    kAudioUnitCustomProperty_MaxNoteTrack_12,   // read only
    kAudioUnitCustomProperty_MaxNoteTrack_13,   // read only
    kAudioUnitCustomProperty_MaxNoteTrack_14,   // read only
    kAudioUnitCustomProperty_MaxNoteTrack_15,   // read only
    kAudioUnitCustomProperty_MaxNoteTrack_16,   // read only
    
    kAudioUnitCustomProperty_SourceFileRef,
    kAudioUnitCustomProperty_IsEmaphasized,
    
    kAudioUnitCustomProperty_SustainMode,
    kAudioUnitCustomProperty_MonoMode,
    kAudioUnitCustomProperty_PortamentoOn,
    kAudioUnitCustomProperty_PortamentoRate,
    kAudioUnitCustomProperty_NoteOnPriority,
    kAudioUnitCustomProperty_ReleasePriority,
    
    kAudioUnitCustomProperty_IsHwConnected,     // read only
    
    // 追加
    // グローバル設定ファイルはau,vst共通にするためsaveToGlobalなものはCStringに
    kAudioUnitCustomProperty_SongRecordPath,    // char[PATH_LEN_MAX] ->dsp
    kAudioUnitCustomProperty_RecSaveAsSpc,      // bool ->dsp
    kAudioUnitCustomProperty_RecSaveAsSmc,      // bool ->dsp
    kAudioUnitCustomProperty_RecordStartBeatPos,// double ->driver
    kAudioUnitCustomProperty_RecordLoopStartBeatPos,// double ->driver
    kAudioUnitCustomProperty_RecordEndBeatPos,  // double ->driver
    kAudioUnitCustomProperty_TimeBaseForSmc,    // int ->dsp
    kAudioUnitCustomProperty_GameTitle,         // char[32](smc出力時には21バイトに切り詰められる) ->dsp
    kAudioUnitCustomProperty_SongTitle,         // char[32] ->dsp
    kAudioUnitCustomProperty_NameOfDumper,      // char[16] ->dsp
    kAudioUnitCustomProperty_ArtistOfSong,      // char[32] ->dsp
    kAudioUnitCustomProperty_SongComments,      // char[32] ->dsp
    
    kAudioUnitCustomProperty_SongPlayerCode,    // CFData ->dsp write only
    kAudioUnitCustomProperty_SongPlayerCodeVer, // int32 read only
    
    kAudioUnitCustomProperty_HostBeatPos,       // double read only
    
    kAudioUnitCustomProperty_End,
    kNumberOfProperties = kAudioUnitCustomProperty_End-kAudioUnitCustomProperty_Begin
};

enum PropertyDataType {
    propertyDataTypeFloat32,
    propertyDataTypeDouble,
    propertyDataTypeInt32,
    propertyDataTypeBool,
    propertyDataTypeStruct,
    propertyDataTypeCString,
    propertyDataTypeFilePath,
    propertyDataTypeCFDataRef,
};

typedef struct {
    unsigned int     propId;
    unsigned int     outDataSize;
    bool             outWritable;
    PropertyDataType dataType;
    bool             readOnly;
    bool             saveToProg;
    bool             saveToSong;
    bool             saveToGlobal;
    char             savekey[32];
    double           defaultValue;
} PropertyDescription;

void createPropertyParamMap(std::map<int, PropertyDescription> &m);

#endif /* defined(__C700__C700Properties__) */
