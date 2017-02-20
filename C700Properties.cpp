//
//  C700Properties.cpp
//  C700
//
//  Created by osoumen on 2017/02/16.
//
//

#include "C700Properties.h"

PropertyDescription sPropertyDescription[] = {
    {
        kAudioUnitCustomProperty_ProgramName,   // propId
        sizeof(void*),                          // outDataSize
        false,                                  // outWritable
        propertyDataTypePtr,                    // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false                                   // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_BRRData,       // propId
        sizeof(BRRData),                        // outDataSize
        false,                                  // outWritable
        propertyDataTypeStruct,                 // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false                                   // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_Rate,          // propId
        sizeof(double),                         // outDataSize
        false,                                  // outWritable
        propertyDataTypeDouble,                 // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false                                   // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_BaseKey,       // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false                                   // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_LowKey,        // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false                                   // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_HighKey,       // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false                                   // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_LoopPoint,     // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false                                   // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_Loop,          // propId
        sizeof(bool),                           // outDataSize
        false,                                  // outWritable
        propertyDataTypeBool,                   // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false                                   // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_AR,            // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false                                   // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_DR,            // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false                                   // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_SL,            // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false                                   // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_SR,            // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false                                   // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_VolL,          // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false                                   // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_VolR,          // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false                                   // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_Echo,          // propId
        sizeof(bool),                           // outDataSize
        false,                                  // outWritable
        propertyDataTypeBool,                   // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false                                   // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_Bank,          // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false                                   // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_EditingProgram,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        true,                                   // saveToSong
        false                                   // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_EditingChannel,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        true,                                   // saveToSong
        false                                   // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_Band1,         // propId
        sizeof(float),                          // outDataSize
        false,                                  // outWritable
        propertyDataTypeFloat32,                // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        true,                                   // saveToSong
        false                                   // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_Band2,         // propId
        sizeof(float),                          // outDataSize
        false,                                  // outWritable
        propertyDataTypeFloat32,                // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        true,                                   // saveToSong
        false                                   // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_Band3,         // propId
        sizeof(float),                          // outDataSize
        false,                                  // outWritable
        propertyDataTypeFloat32,                // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        true,                                   // saveToSong
        false                                   // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_Band4,         // propId
        sizeof(float),                          // outDataSize
        false,                                  // outWritable
        propertyDataTypeFloat32,                // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        true,                                   // saveToSong
        false                                   // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_Band5,         // propId
        sizeof(float),                          // outDataSize
        false,                                  // outWritable
        propertyDataTypeFloat32,                // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        true,                                   // saveToSong
        false                                   // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_TotalRAM,      // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true,                                   // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false                                   // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_PGDictionary,  // propId
        sizeof(void*),                          // outDataSize
        false,                                  // outWritable
        propertyDataTypePtr,                    // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false                                   // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_XIData,        // propId
        sizeof(void*),                          // outDataSize
        false,                                  // outWritable
        propertyDataTypePtr,                    // dataType
        true,                                   // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_NoteOnTrack_1, // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true,                                   // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_NoteOnTrack_2, // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true,                                   // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_NoteOnTrack_3, // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true,                                   // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_NoteOnTrack_4, // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true,                                   // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_NoteOnTrack_5, // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true,                                   // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_NoteOnTrack_6, // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true,                                   // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_NoteOnTrack_7, // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true,                                   // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_NoteOnTrack_8, // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true,                                   // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_NoteOnTrack_9, // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true,                                   // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_NoteOnTrack_10,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true,                                   // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_NoteOnTrack_11,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true,                                   // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_NoteOnTrack_12,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true,                                   // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_NoteOnTrack_13,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true,                                   // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_NoteOnTrack_14,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true,                                   // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_NoteOnTrack_15,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true,                                   // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_NoteOnTrack_16,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true,                                   // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_1,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true,                                   // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_2,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true,                                   // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_3,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true,                                   // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_4,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true,                                   // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_5,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true,                                   // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_6,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true,                                   // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_7,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true,                                   // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_8,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true,                                   // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_9,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true,                                   // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_10,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true,                                   // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_11,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true,                                   // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_12,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true,                                   // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_13,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true,                                   // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_14,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true,                                   // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_15,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true,                                   // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_16,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true,                                   // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_SourceFileRef, // propId
        sizeof(void*),                          // outDataSize
        false,                                  // outWritable
        propertyDataTypePtr,                    // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false                                   // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_IsEmaphasized, // propId
        sizeof(bool),                           // outDataSize
        false,                                  // outWritable
        propertyDataTypeBool,                   // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false                                   // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_SustainMode,   // propId
        sizeof(bool),                           // outDataSize
        false,                                  // outWritable
        propertyDataTypeBool,                   // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false                                   // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_MonoMode,      // propId
        sizeof(bool),                           // outDataSize
        false,                                  // outWritable
        propertyDataTypeBool,                   // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false                                   // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_PortamentoOn,  // propId
        sizeof(bool),                           // outDataSize
        false,                                  // outWritable
        propertyDataTypeBool,                   // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false                                   // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_PortamentoRate,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false                                   // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_NoteOnPriority,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false                                   // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_ReleasePriority,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_IsHwConnected, // propId
        sizeof(bool),                           // outDataSize
        false,                                  // outWritable
        propertyDataTypeBool,                   // dataType
        true,                                   // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_SongRecordPath,// propId
        sizeof(void*),                          // outDataSize
        false,                                  // outWritable
        propertyDataTypePtr,                    // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        true,                                   // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_RecSaveAsSpc,  // propId
        sizeof(bool),                           // outDataSize
        false,                                  // outWritable
        propertyDataTypeBool,                   // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        true,                                   // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_RecSaveAsSmc,  // propId
        sizeof(bool),                           // outDataSize
        false,                                  // outWritable
        propertyDataTypeBool,                   // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        true,                                   // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_RecordStartBeatPos,// propId
        sizeof(double),                         // outDataSize
        false,                                  // outWritable
        propertyDataTypeDouble,                 // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        true,                                   // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_RecordLoopStartBeatPos,// propId
        sizeof(double),                         // outDataSize
        false,                                  // outWritable
        propertyDataTypeDouble,                 // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        true,                                   // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_RecordEndBeatPos,// propId
        sizeof(double),                         // outDataSize
        false,                                  // outWritable
        propertyDataTypeDouble,                 // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        true,                                   // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_TimeBaseForSmc,// propId
        sizeof(double),                         // outDataSize
        false,                                  // outWritable
        propertyDataTypeDouble,                 // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        true,                                   // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_GameTitle,     // propId
        sizeof(void*),                          // outDataSize
        false,                                  // outWritable
        propertyDataTypePtr,                    // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        true,                                   // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_SongTitle,     // propId
        sizeof(void*),                          // outDataSize
        false,                                  // outWritable
        propertyDataTypePtr,                    // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        true,                                   // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_NameOfDumper,  // propId
        sizeof(void*),                          // outDataSize
        false,                                  // outWritable
        propertyDataTypePtr,                    // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        true,                                   // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_ArtistOfSong,  // propId
        sizeof(void*),                          // outDataSize
        false,                                  // outWritable
        propertyDataTypePtr,                    // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        true,                                   // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_SongComments,  // propId
        sizeof(void*),                          // outDataSize
        false,                                  // outWritable
        propertyDataTypePtr,                    // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        true,                                   // saveToSong
        false,                                  // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_SmcNativeVector,// propId
        sizeof(void*),                          // outDataSize
        false,                                  // outWritable
        propertyDataTypePtr,                    // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        true,                                   // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_SmcEmulationVector,// propId
        sizeof(void*),                          // outDataSize
        false,                                  // outWritable
        propertyDataTypePtr,                    // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        true,                                   // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_SmcPlayerCode, // propId
        sizeof(void*),                          // outDataSize
        false,                                  // outWritable
        propertyDataTypePtr,                    // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        true,                                   // saveToGlobal
    },
    {
        kAudioUnitCustomProperty_SpcPlayerCode, // propId
        sizeof(void*),                          // outDataSize
        false,                                  // outWritable
        propertyDataTypePtr,                    // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        true,                                   // saveToGlobal
    }
};


void createPropertyParamMap(std::map<int, PropertyDescription> &m)
{
    int numProperties = sizeof(sPropertyDescription) / sizeof(PropertyDescription);
    
    for (int i=0; i<numProperties; i++) {
        m[sPropertyDescription[i].propId] = sPropertyDescription[i];
    }
}
