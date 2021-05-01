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
        PROGRAMNAME_MAX_LEN,                    // outDataSize
        false,                                  // outWritable
        propertyDataTypeString,                 // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "progname",                             // savekey
    },
    {
        kAudioUnitCustomProperty_BRRData,       // propId
        sizeof(void*),                          // outDataSize
        false,                                  // outWritable
        propertyDataTypeVariableData,           // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "brrdata",                              // savekey
    },
    {
        kAudioUnitCustomProperty_Rate,          // propId
        sizeof(double),                         // outDataSize
        false,                                  // outWritable
        propertyDataTypeDouble,                 // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "samplerate",                           // savekey
    },
    {
        kAudioUnitCustomProperty_BaseKey,       // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "key",                                  // savekey
        60,                                     // defalutValue
    },
    {
        kAudioUnitCustomProperty_LowKey,        // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "lowkey",                               // savekey
        0,                                      // defalutValue
    },
    {
        kAudioUnitCustomProperty_HighKey,       // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "highkey",                              // savekey
        127,                                    // defalutValue
    },
    {
        kAudioUnitCustomProperty_LoopPoint,     // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "looppoint",                            // savekey
    },
    {
        kAudioUnitCustomProperty_Loop,          // propId
        sizeof(bool),                           // outDataSize
        false,                                  // outWritable
        propertyDataTypeBool,                   // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "",                                     // savekey
    },
    {
        kAudioUnitCustomProperty_AR,            // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "ar",                                   // savekey
        15,                                     // defalutValue
    },
    {
        kAudioUnitCustomProperty_DR,            // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "dr",                                   // savekey
        7,                                      // defalutValue
    },
    {
        kAudioUnitCustomProperty_SL,            // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "sl",                                   // savekey
        7,                                      // defalutValue
    },
    {
        kAudioUnitCustomProperty_SR1,           // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "sr",                                   // savekey
        0,                                      // defalutValue
    },
    {
        kAudioUnitCustomProperty_VolL,          // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "volL",                                 // savekey
        100,                                    // defalutValue
    },
    {
        kAudioUnitCustomProperty_VolR,          // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "volR",                                 // savekey
        100,                                    // defalutValue
    },
    {
        kAudioUnitCustomProperty_Echo,          // propId
        sizeof(bool),                           // outDataSize
        false,                                  // outWritable
        propertyDataTypeBool,                   // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "echo",                                 // savekey
        0,                                      // defalutValue
    },
    {
        kAudioUnitCustomProperty_Bank,          // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "bank",                                 // savekey
        0,                                      // defalutValue
    },
    {
        kAudioUnitCustomProperty_EditingProgram,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        true,                                   // saveToSong
        false,                                  // saveToGlobal
        "editprog",                             // savekey
    },
    {
        kAudioUnitCustomProperty_EditingChannel,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        true,                                   // saveToSong
        false,                                  // saveToGlobal
        "editchan",                             // savekey
    },
    {
        kAudioUnitCustomProperty_Band1,         // propId
        sizeof(float),                          // outDataSize
        false,                                  // outWritable
        propertyDataTypeFloat32,                // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        true,                                   // saveToSong
        false,                                  // saveToGlobal
        "eqband1",                              // savekey
        -1,                                     // defaultValue
    },
    {
        kAudioUnitCustomProperty_Band2,         // propId
        sizeof(float),                          // outDataSize
        false,                                  // outWritable
        propertyDataTypeFloat32,                // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        true,                                   // saveToSong
        false,                                  // saveToGlobal
        "eqband2",                              // savekey
        -1,                                     // defaultValue
    },
    {
        kAudioUnitCustomProperty_Band3,         // propId
        sizeof(float),                          // outDataSize
        false,                                  // outWritable
        propertyDataTypeFloat32,                // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        true,                                   // saveToSong
        false,                                  // saveToGlobal
        "eqband3",                              // savekey
        -1,                                     // defaultValue
    },
    {
        kAudioUnitCustomProperty_Band4,         // propId
        sizeof(float),                          // outDataSize
        false,                                  // outWritable
        propertyDataTypeFloat32,                // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        true,                                   // saveToSong
        false,                                  // saveToGlobal
        "eqband4",                              // savekey
        -1,                                     // defaultValue
    },
    {
        kAudioUnitCustomProperty_Band5,         // propId
        sizeof(float),                          // outDataSize
        false,                                  // outWritable
        propertyDataTypeFloat32,                // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        true,                                   // saveToSong
        false,                                  // saveToGlobal
        "eqband5",                              // savekey
        -1,                                     // defaultValue
    },
    {
        kAudioUnitCustomProperty_TotalRAM,      // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true,                                   // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "",                                     // savekey
    },
    {
        kAudioUnitCustomProperty_PGDictionary,  // propId
        sizeof(void*),                          // outDataSize
        false,                                  // outWritable
        propertyDataTypePointer,                // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "",                                     // savekey
    },
    {
        kAudioUnitCustomProperty_XIData,        // propId
        sizeof(void*),                          // outDataSize
        false,                                  // outWritable
        propertyDataTypePointer,                // dataType
        true,                                   // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "",                                     // savekey
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
        "",                                     // savekey
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
        "",                                     // savekey
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
        "",                                     // savekey
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
        "",                                     // savekey
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
        "",                                     // savekey
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
        "",                                     // savekey
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
        "",                                     // savekey
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
        "",                                     // savekey
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
        "",                                     // savekey
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
        "",                                     // savekey
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
        "",                                     // savekey
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
        "",                                     // savekey
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
        "",                                     // savekey
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
        "",                                     // savekey
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
        "",                                     // savekey
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
        "",                                     // savekey
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_1,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "",                                     // savekey
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_2,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "",                                     // savekey
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_3,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "",                                     // savekey
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_4,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "",                                     // savekey
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_5,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "",                                     // savekey
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_6,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "",                                     // savekey
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_7,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "",                                     // savekey
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_8,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "",                                     // savekey
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_9,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "",                                     // savekey
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_10,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "",                                     // savekey
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_11,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "",                                     // savekey
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_12,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "",                                     // savekey
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_13,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "",                                     // savekey
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_14,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "",                                     // savekey
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_15,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "",                                     // savekey
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_16,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "",                                     // savekey
    },
    {
        kAudioUnitCustomProperty_SourceFileRef, // propId
        PATH_LEN_MAX,                           // outDataSize
        false,                                  // outWritable
        propertyDataTypeFilePath,               // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "srcfile",                              // savekey
    },
    {
        kAudioUnitCustomProperty_IsEmaphasized, // propId
        sizeof(bool),                           // outDataSize
        false,                                  // outWritable
        propertyDataTypeBool,                   // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "isemph",                               // savekey
    },
    {
        kAudioUnitCustomProperty_SustainMode,   // propId
        sizeof(bool),                           // outDataSize
        false,                                  // outWritable
        propertyDataTypeBool,                   // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "sustainmode",                          // savekey
        1,                                      // defalutValue
    },
    {
        kAudioUnitCustomProperty_MonoMode,      // propId
        sizeof(bool),                           // outDataSize
        false,                                  // outWritable
        propertyDataTypeBool,                   // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "monomode",                             // savekey
        0,                                      // defalutValue
    },
    {
        kAudioUnitCustomProperty_PortamentoOn,  // propId
        sizeof(bool),                           // outDataSize
        false,                                  // outWritable
        propertyDataTypeBool,                   // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "portamentoon",                         // savekey
        0,                                      // defalutValue
    },
    {
        kAudioUnitCustomProperty_PortamentoRate,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "portamentorate",                       // savekey
        0,                                      // defalutValue
    },
    {
        kAudioUnitCustomProperty_NoteOnPriority,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "noteonpriority",                       // savekey
        64,                                     // defalutValue
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
        "releasepriority",                      // savekey
        0,                                      // defalutValue
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
        "",                                     // savekey
    },
    {
        kAudioUnitCustomProperty_SongRecordPath,// propId
        PATH_LEN_MAX,                           // outDataSize
        false,                                  // outWritable
        propertyDataTypeFilePath,               // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        true,                                   // saveToGlobal
        "",                                     // savekey
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
        "",                                     // savekey
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
        "",                                     // savekey
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
        "recordstartbeatpos",                   // savekey
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
        "recordloopstartbeatpos",               // savekey
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
        "recordbeatendpos",                     // savekey
    },
    {
        kAudioUnitCustomProperty_TimeBaseForSmc,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        true,                                   // saveToGlobal
        "",                                     // savekey
    },
    {
        kAudioUnitCustomProperty_GameTitle,     // propId
        33,                                     // outDataSize
        false,                                  // outWritable
        propertyDataTypeString,                 // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        true,                                   // saveToSong
        false,                                  // saveToGlobal
        "gametitle",                            // savekey
    },
    {
        kAudioUnitCustomProperty_SongTitle,     // propId
        33,                                     // outDataSize
        false,                                  // outWritable
        propertyDataTypeString,                 // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        true,                                   // saveToSong
        false,                                  // saveToGlobal
        "songtitle",                            // savekey
    },
    {
        kAudioUnitCustomProperty_NameOfDumper,  // propId
        17,                                     // outDataSize
        false,                                  // outWritable
        propertyDataTypeString,                 // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        true,                                   // saveToSong
        false,                                  // saveToGlobal
        "nameofdumper",                         // savekey
    },
    {
        kAudioUnitCustomProperty_ArtistOfSong,  // propId
        33,                                     // outDataSize
        false,                                  // outWritable
        propertyDataTypeString,                 // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        true,                                   // saveToSong
        false,                                  // saveToGlobal
        "artistofsong",                         // savekey
    },
    {
        kAudioUnitCustomProperty_SongComments,  // propId
        33,                                     // outDataSize
        false,                                  // outWritable
        propertyDataTypeString,                 // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        true,                                   // saveToSong
        false,                                  // saveToGlobal
        "songcomments",                         // savekey
    },
    {
        kAudioUnitCustomProperty_RepeatNumForSpc,// propId
        sizeof(float),                          // outDataSize
        false,                                  // outWritable
        propertyDataTypeFloat32,                // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        true,                                   // saveToSong
        false,                                  // saveToGlobal
        "repeatnumforspc",                      // savekey
        1.0f,                                   // defaultValue
    },
    {
        kAudioUnitCustomProperty_FadeMsTimeForSpc,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        true,                                   // saveToSong
        false,                                  // saveToGlobal
        "fademstimeforspc",                     // savekey
        5000,                                   // defaultValue
    },
    {
        kAudioUnitCustomProperty_SongPlayerCode,// propId
        sizeof(void*),                          // outDataSize
        false,                                  // outWritable
        propertyDataTypeVariableData,           // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        true,                                   // saveToGlobal
        "",                                     // savekey
    },
    {
        kAudioUnitCustomProperty_SongPlayerCodeVer,// propId
        sizeof(double),                         // outDataSize
        false,                                  // outWritable
        propertyDataTypeDouble,                 // dataType
        true,                                   // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "",                                     // savekey
    },
    {
        kAudioUnitCustomProperty_HostBeatPos,   // propId
        sizeof(double),                         // outDataSize
        false,                                  // outWritable
        propertyDataTypeDouble,                 // dataType
        true,                                   // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "",                                     // savekey
    },
    {
        kAudioUnitCustomProperty_MaxNoteOnTotal,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        false,                                  // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "",                                     // savekey
    },

    {
        kAudioUnitCustomProperty_PitchModulationOn,// propId
        sizeof(bool),                           // outDataSize
        false,                                  // outWritable
        propertyDataTypeBool,                   // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "pmon",                                 // savekey
        0,                                      // defalutValue
    },
    {
        kAudioUnitCustomProperty_NoiseOn,       // propId
        sizeof(bool),                           // outDataSize
        false,                                  // outWritable
        propertyDataTypeBool,                   // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "noiseon",                              // savekey
        0,                                      // defalutValue
    },
    {
        kAudioUnitCustomProperty_SR2,           // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false,                                  // readOnly
        true,                                   // saveToProg
        false,                                  // saveToSong
        false,                                  // saveToGlobal
        "sr2",                                  // savekey
        31,                                     // defalutValue
    }
};


void createPropertyParamMap(std::map<int, PropertyDescription> &m)
{
    int numProperties = sizeof(sPropertyDescription) / sizeof(PropertyDescription);
    
    for (int i=0; i<numProperties; i++) {
//        assert((sPropertyDescription[i].savekey[0] != 0) ||
//               (//(sPropertyDescription[i].saveToGlobal == 0) &&
//                (sPropertyDescription[i].saveToProg == 0) &&
//                (sPropertyDescription[i].saveToSong == 0)));
        m[sPropertyDescription[i].propId] = sPropertyDescription[i];
    }
}
