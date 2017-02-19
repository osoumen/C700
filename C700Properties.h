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
    kAudioUnitCustomProperty_SongRecordPath,    // CFURLRef
    kAudioUnitCustomProperty_RecSaveAsSpc,      // bool
    kAudioUnitCustomProperty_RecSaveAsSmc,      // bool
    kAudioUnitCustomProperty_RecordStart,       // double
    kAudioUnitCustomProperty_RecordLoopStart,   // double
    kAudioUnitCustomProperty_RecordEnd,         // double
    kAudioUnitCustomProperty_TimeBaseForSmc,    // int
    kAudioUnitCustomProperty_GameTitle,         // Cstring[32](smc出力時には21バイトに切り詰められる)
    kAudioUnitCustomProperty_SongTitle,         // Cstring[32]
    kAudioUnitCustomProperty_NameOfDumper,      // Cstring[16]
    kAudioUnitCustomProperty_ArtistOfSong,      // Cstring[32]
    kAudioUnitCustomProperty_SongComments,      // Cstring[32]
    kAudioUnitCustomProperty_SmcNativeVector,
    kAudioUnitCustomProperty_SmcEmulationVector,
    kAudioUnitCustomProperty_SmcPlayerCode,
    kAudioUnitCustomProperty_SpcPlayerCode,
    
    kAudioUnitCustomProperty_End,
    kNumberOfProperties = kAudioUnitCustomProperty_End-kAudioUnitCustomProperty_Begin
};

enum PropertyDataType {
    propertyDataTypeFloat32,
    propertyDataTypeDouble,
    propertyDataTypeInt32,
    propertyDataTypeBool,
    propertyDataTypePtr,
    propertyDataTypeStruct,
};

typedef struct {
    unsigned int     propId;
    unsigned int     outDataSize;
    bool             outWritable;
    PropertyDataType dataType;
    bool             readOnly;
} PropertyDescription;

void createPropertyParamMap(std::map<int, PropertyDescription> &m);

#endif /* defined(__C700__C700Properties__) */
