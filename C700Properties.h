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
    kAudioUnitCustomProperty_RecordStartBeatPos,// double ->kernel
    kAudioUnitCustomProperty_RecordLoopStartBeatPos,// double ->kernel
    kAudioUnitCustomProperty_RecordEndBeatPos,  // double ->kernel
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
    propertyDataTypeFloat32,        // 32bit実数型
    propertyDataTypeDouble,         // 64bit実数型
    propertyDataTypeInt32,          // 32bit整数型
    propertyDataTypeBool,           // bool型 vstではintとして持つ
    propertyDataTypeStruct,         // outDataSizeをサイズとする構造体
    propertyDataTypeString,         // AUではCFStringにwrapされる VSTではoutDataSizeを最大文字数とするCString
    propertyDataTypeFilePath,       // AUではCFURLにwrapされる VSTではoutDataSizeを最大文字数とするCString
    propertyDataTypeVariableData,   // AUではCFDataにwrapされる可変長データ VSTではSetGet時にsizeを設定する
    propertyDataTypePointer,        // 単なるポインタ型 UIとのやりとりのみに使用し、保存しないタイプのプロパティ
};

typedef struct {
    unsigned int     propId;        // プロパティID
    unsigned int     outDataSize;   // データのサイズ boolは1を示すが、VSTでchunk保存時には4バイトを使用する
    bool             outWritable;   // AUでGetしたデータに書き込みが出来るかどうか
    PropertyDataType dataType;      // データのタイプ
    bool             readOnly;      // setを実装しない場合にtrueを設定する
    bool             saveToProg;    // プログラム(音色)の中に含める
    bool             saveToSong;    // 曲保存時に含める
    bool             saveToGlobal;  // 初期設定の中に含める
    char             savekey[32];   // AUで保存の際に使用するキー 保存フラグのいずれかを指定した場合必要
    double           defaultValue;  // 未設定の場合に使用される初期値 負数を指定すると無効にできる
} PropertyDescription;

void createPropertyParamMap(std::map<int, PropertyDescription> &m);

#endif /* defined(__C700__C700Properties__) */
