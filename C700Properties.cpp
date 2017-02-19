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
        sizeof(CFStringRef),                    // outDataSize
        false,                                  // outWritable
        propertyDataTypePtr,                    // dataType
        false                                   // readOnly
    },
    {
        kAudioUnitCustomProperty_BRRData,       // propId
        sizeof(BRRData),                        // outDataSize
        false,                                  // outWritable
        propertyDataTypeStruct,                 // dataType
        false                                   // readOnly
    },
    {
        kAudioUnitCustomProperty_Rate,          // propId
        sizeof(double),                         // outDataSize
        false,                                  // outWritable
        propertyDataTypeDouble,                 // dataType
        false                                   // readOnly
    },
    {
        kAudioUnitCustomProperty_BaseKey,       // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false                                   // readOnly
    },
    {
        kAudioUnitCustomProperty_LowKey,        // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false                                   // readOnly
    },
    {
        kAudioUnitCustomProperty_HighKey,       // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false                                   // readOnly
    },
    {
        kAudioUnitCustomProperty_LoopPoint,     // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false                                   // readOnly
    },
    {
        kAudioUnitCustomProperty_Loop,          // propId
        sizeof(bool),                           // outDataSize
        false,                                  // outWritable
        propertyDataTypeBool,                   // dataType
        false                                   // readOnly
    },
    {
        kAudioUnitCustomProperty_AR,            // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false                                   // readOnly
    },
    {
        kAudioUnitCustomProperty_DR,            // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false                                   // readOnly
    },
    {
        kAudioUnitCustomProperty_SL,            // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false                                   // readOnly
    },
    {
        kAudioUnitCustomProperty_SR,            // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false                                   // readOnly
    },
    {
        kAudioUnitCustomProperty_VolL,          // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false                                   // readOnly
    },
    {
        kAudioUnitCustomProperty_VolR,          // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false                                   // readOnly
    },
    {
        kAudioUnitCustomProperty_Echo,          // propId
        sizeof(bool),                           // outDataSize
        false,                                  // outWritable
        propertyDataTypeBool,                   // dataType
        false                                   // readOnly
    },
    {
        kAudioUnitCustomProperty_Bank,          // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false                                   // readOnly
    },
    {
        kAudioUnitCustomProperty_EditingProgram,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false                                   // readOnly
    },
    {
        kAudioUnitCustomProperty_EditingChannel,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false                                   // readOnly
    },
    {
        kAudioUnitCustomProperty_Band1,         // propId
        sizeof(float),                          // outDataSize
        false,                                  // outWritable
        propertyDataTypeFloat32,                // dataType
        false                                   // readOnly
    },
    {
        kAudioUnitCustomProperty_Band2,         // propId
        sizeof(float),                          // outDataSize
        false,                                  // outWritable
        propertyDataTypeFloat32,                // dataType
        false                                   // readOnly
    },
    {
        kAudioUnitCustomProperty_Band3,         // propId
        sizeof(float),                          // outDataSize
        false,                                  // outWritable
        propertyDataTypeFloat32,                // dataType
        false                                   // readOnly
    },
    {
        kAudioUnitCustomProperty_Band4,         // propId
        sizeof(float),                          // outDataSize
        false,                                  // outWritable
        propertyDataTypeFloat32,                // dataType
        false                                   // readOnly
    },
    {
        kAudioUnitCustomProperty_Band5,         // propId
        sizeof(float),                          // outDataSize
        false,                                  // outWritable
        propertyDataTypeFloat32,                // dataType
        false                                   // readOnly
    },
    {
        kAudioUnitCustomProperty_TotalRAM,      // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true                                    // readOnly
    },
    {
        kAudioUnitCustomProperty_PGDictionary,  // propId
        sizeof(void*),                          // outDataSize
        false,                                  // outWritable
        propertyDataTypePtr,                    // dataType
        false                                   // readOnly
    },
    {
        kAudioUnitCustomProperty_XIData,        // propId
        sizeof(void*),                          // outDataSize
        false,                                  // outWritable
        propertyDataTypePtr,                    // dataType
        true                                    // readOnly
    },
    {
        kAudioUnitCustomProperty_NoteOnTrack_1, // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true                                    // readOnly
    },
    {
        kAudioUnitCustomProperty_NoteOnTrack_2, // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true                                    // readOnly
    },
    {
        kAudioUnitCustomProperty_NoteOnTrack_3, // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true                                    // readOnly
    },
    {
        kAudioUnitCustomProperty_NoteOnTrack_4, // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true                                    // readOnly
    },
    {
        kAudioUnitCustomProperty_NoteOnTrack_5, // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true                                    // readOnly
    },
    {
        kAudioUnitCustomProperty_NoteOnTrack_6, // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true                                    // readOnly
    },
    {
        kAudioUnitCustomProperty_NoteOnTrack_7, // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true                                    // readOnly
    },
    {
        kAudioUnitCustomProperty_NoteOnTrack_8, // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true                                    // readOnly
    },
    {
        kAudioUnitCustomProperty_NoteOnTrack_9, // propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true                                    // readOnly
    },
    {
        kAudioUnitCustomProperty_NoteOnTrack_10,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true                                    // readOnly
    },
    {
        kAudioUnitCustomProperty_NoteOnTrack_11,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true                                    // readOnly
    },
    {
        kAudioUnitCustomProperty_NoteOnTrack_12,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true                                    // readOnly
    },
    {
        kAudioUnitCustomProperty_NoteOnTrack_13,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true                                    // readOnly
    },
    {
        kAudioUnitCustomProperty_NoteOnTrack_14,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true                                    // readOnly
    },
    {
        kAudioUnitCustomProperty_NoteOnTrack_15,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true                                    // readOnly
    },
    {
        kAudioUnitCustomProperty_NoteOnTrack_16,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true                                    // readOnly
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_1,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true                                    // readOnly
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_2,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true                                    // readOnly
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_3,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true                                    // readOnly
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_4,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true                                    // readOnly
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_5,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true                                    // readOnly
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_6,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true                                    // readOnly
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_7,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true                                    // readOnly
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_8,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true                                    // readOnly
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_9,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true                                    // readOnly
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_10,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true                                    // readOnly
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_11,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true                                    // readOnly
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_12,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true                                    // readOnly
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_13,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true                                    // readOnly
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_14,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true                                    // readOnly
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_15,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true                                    // readOnly
    },
    {
        kAudioUnitCustomProperty_MaxNoteTrack_16,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        true                                    // readOnly
    },
    {
        kAudioUnitCustomProperty_SourceFileRef, // propId
        sizeof(void*),                          // outDataSize
        false,                                  // outWritable
        propertyDataTypePtr,                    // dataType
        false                                   // readOnly
    },
    {
        kAudioUnitCustomProperty_IsEmaphasized, // propId
        sizeof(bool),                           // outDataSize
        false,                                  // outWritable
        propertyDataTypeBool,                   // dataType
        false                                   // readOnly
    },
    {
        kAudioUnitCustomProperty_SustainMode,   // propId
        sizeof(bool),                           // outDataSize
        false,                                  // outWritable
        propertyDataTypeBool,                   // dataType
        false                                   // readOnly
    },
    {
        kAudioUnitCustomProperty_MonoMode,      // propId
        sizeof(bool),                           // outDataSize
        false,                                  // outWritable
        propertyDataTypeBool,                   // dataType
        false                                   // readOnly
    },
    {
        kAudioUnitCustomProperty_PortamentoOn,  // propId
        sizeof(bool),                           // outDataSize
        false,                                  // outWritable
        propertyDataTypeBool,                   // dataType
        false                                   // readOnly
    },
    {
        kAudioUnitCustomProperty_PortamentoRate,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false                                   // readOnly
    },
    {
        kAudioUnitCustomProperty_NoteOnPriority,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false                                   // readOnly
    },
    {
        kAudioUnitCustomProperty_ReleasePriority,// propId
        sizeof(int),                            // outDataSize
        false,                                  // outWritable
        propertyDataTypeInt32,                  // dataType
        false                                   // readOnly
    },
    {
        kAudioUnitCustomProperty_IsHwConnected, // propId
        sizeof(bool),                           // outDataSize
        false,                                  // outWritable
        propertyDataTypeBool,                   // dataType
        true                                    // readOnly
    },
    /*
    {
        kAudioUnitCustomProperty_SongRecordPath,// propId
    },
    {
        kAudioUnitCustomProperty_RecSaveAsSpc,  // propId
    },
    {
        kAudioUnitCustomProperty_RecSaveAsSmc,  // propId
    },
    {
        kAudioUnitCustomProperty_RecordStart,   // propId
    },
    {
        kAudioUnitCustomProperty_RecordLoopStart,// propId
    },
    {
        kAudioUnitCustomProperty_RecordEnd,     // propId
    },
    {
        kAudioUnitCustomProperty_TimeBaseForSmc,// propId
    },
    {
        kAudioUnitCustomProperty_GameTitle,     // propId
    },
    {
        kAudioUnitCustomProperty_SongTitle,     // propId
    },
    {
        kAudioUnitCustomProperty_NameOfDumper,  // propId
    },
    {
        kAudioUnitCustomProperty_ArtistOfSong,  // propId
    },
    {
        kAudioUnitCustomProperty_SongComments,  // propId
    },
    {
        kAudioUnitCustomProperty_SmcNativeVector,// propId
    },
    {
        kAudioUnitCustomProperty_SmcEmulationVector,// propId
    },
    {
        kAudioUnitCustomProperty_SmcPlayerCode, // propId
    },
    {
        kAudioUnitCustomProperty_SpcPlayerCode, // propId
    }
     */
};


void createPropertyParamMap(std::map<int, PropertyDescription> &m)
{
    int numProperties = sizeof(sPropertyDescription) / sizeof(PropertyDescription);
    
    for (int i=0; i<numProperties; i++) {
        m[sPropertyDescription[i].propId] = sPropertyDescription[i];
    }
}
