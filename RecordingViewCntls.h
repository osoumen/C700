//
//  RecordingViewCntls.h
//  C700
//
//  Created by osoumen on 2017/02/03.
//
//

#ifndef C700_RecordingViewCntls_h
#define C700_RecordingViewCntls_h


#include "GUIUtils.h"
#include "C700Properties.h"

ControlInstances	sRecordingViewCntls[] = {
    {
		'appl',	//sig
		'stxt',	//kind
		"Save Path",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		0,	//sig
		-1,	//id
		0,	//command
		10, 20, 80, 11,	//x,y,w,h
		"",	//fontname
		0,	//fontsize
		kLeftText,	//fontalign
		0,	//style
		0	//future use
	},
    {
		'appl',	//sig
		'stxt',	//kind
		"Path",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		0,	//sig
		kAudioUnitCustomProperty_SongRecordPath,	//id
		0,	//command
		10, 36, 350, 14,	//x,y,w,h
		"Arial",	//fontname
		11,	//fontsize
		kLeftText,	//fontalign
		k3DOut,	//style
		0	//future use
	},
    {
		'VeMa',	//sig
		'push',	//kind
		"Choose",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		0,	//sig
		kControlButtonChooseRecordPath,	//id
		0,	//command
		370, 34, 40, 14,	//x,y,w,h
		"Arial Black",	//fontname
		9,	//fontsize
		kCenterText,	//fontalign
		0,	//style
		0	//future use
	},
    {
		'airy',	//sig
		'cbtn',	//kind
		"Save as *.spc",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		0,	//sig
		kAudioUnitCustomProperty_RecSaveAsSpc,	//id
		0,	//command
		10, 52, 80, 14,	//x,y,w,h
		"",	//fontname
		0,	//fontsize
		kLeftText,	//fontalign
		0,	//style
		0	//future use
	},
    {
		'airy',	//sig
		'cbtn',	//kind
		"Save as *.smc",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		0,	//sig
		kAudioUnitCustomProperty_RecSaveAsSmc,	//id
		0,	//command
        10, 68, 80, 14,	//x,y,w,h
		"",	//fontname
		0,	//fontsize
		kLeftText,	//fontalign
		0,	//style
		0	//future use
	},
    {
		'airy',	//sig
		'eutx',	//kind
		"",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		0,	//sig
		kAudioUnitCustomProperty_RecordStartBeatPos,	//id
		0,	//command
		10, 92, 64, 14,	//x,y,w,h
		"Arial",	//fontname
		9,	//fontsize
		kCenterText,	//fontalign
		k3DOut,	//style
		1	//future use
	},
    {
		'VeMa',	//sig
		'push',	//kind
		"Set",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		0,	//sig
		kControlButtonSetRecordStart,	//id
		0,	//command
		378, 90, 32, 14,	//x,y,w,h
		"Arial Black",	//fontname
		9,	//fontsize
		kCenterText,	//fontalign
		0,	//style
		0	//future use
	},
    {
		'appl',	//sig
		'stxt',	//kind
		"Record Start Pos (ppq)",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		0,	//sig
		-1,	//id
		0,	//command
		210, 93, 100, 11,	//x,y,w,h
		"",	//fontname
		0,	//fontsize
		kLeftText,	//fontalign
		0,	//style
		0	//future use
	},
    {
		'airy',	//sig
		'eutx',	//kind
		"",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		0,	//sig
		kAudioUnitCustomProperty_RecordLoopStartBeatPos,	//id
		0,	//command
		10, 108, 64, 14,	//x,y,w,h
		"Arial",	//fontname
		9,	//fontsize
		kCenterText,	//fontalign
		k3DOut,	//style
		1	//future use
	},
    {
		'VeMa',	//sig
		'push',	//kind
		"Set",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		0,	//sig
		kControlButtonSetRecordLoopStart,	//id
		0,	//command
		378, 106, 32, 14,	//x,y,w,h
		"Arial Black",	//fontname
		9,	//fontsize
		kCenterText,	//fontalign
		0,	//style
		0	//future use
	},
    {
		'appl',	//sig
		'stxt',	//kind
		"Loop Start Pos (ppq)",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		0,	//sig
		-1,	//id
		0,	//command
		210, 109, 100, 11,	//x,y,w,h
		"",	//fontname
		0,	//fontsize
		kLeftText,	//fontalign
		0,	//style
		0	//future use
	},
    {
		'airy',	//sig
		'eutx',	//kind
		"",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		0,	//sig
		kAudioUnitCustomProperty_RecordEndBeatPos,	//id
		0,	//command
		10, 124, 64, 14,	//x,y,w,h
		"Arial",	//fontname
		9,	//fontsize
		kCenterText,	//fontalign
		k3DOut,	//style
		1	//future use
	},
    {
		'VeMa',	//sig
		'push',	//kind
		"Set",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		0,	//sig
		kControlButtonSetRecordEnd,	//id
		0,	//command
		378, 122, 32, 14,	//x,y,w,h
		"Arial Black",	//fontname
		9,	//fontsize
		kCenterText,	//fontalign
		0,	//style
		0	//future use
	},
    {
		'appl',	//sig
		'stxt',	//kind
		"Record End Pos (ppq)",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		0,	//sig
		-1,	//id
		0,	//command
		210, 125, 100, 11,	//x,y,w,h
		"",	//fontname
		0,	//fontsize
		kLeftText,	//fontalign
		0,	//style
		0	//future use
	},
    {
		'VeMa',	//sig
		'menu',	//kind
		"NTSC;PAL",	//title
		2,	//Value
		0,	//Minimum
		2,	//Maximum
		0,	//sig
		kAudioUnitCustomProperty_TimeBaseForSmc,	//id
		0,	//command
		346, 52, 64, 12,	//x,y,w,h
		"",	//fontname
		9,	//fontsize
		kCenterText,	//fontalign
		0,	//style
		0	//future use
	},
    {
		'appl',	//sig
		'stxt',	//kind
		"smc Format",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		0,	//sig
		-1,	//id
		0,	//command
		210, 53, 80, 11,	//x,y,w,h
		"",	//fontname
		0,	//fontsize
		kLeftText,	//fontalign
		0,	//style
		0	//future use
	},
    {
		'airy',	//sig
		'eutx',	//kind
		"game title",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'user',	//sig
		kAudioUnitCustomProperty_GameTitle,	//id
		0,	//command
		10, 148, 180, 14,	//x,y,w,h
		"Arial",	//fontname
		0,	//fontsize
		kLeftText,	//fontalign
		0,	//style
		2	//future use
	},
    {
		'appl',	//sig
		'stxt',	//kind
		"Game Title",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		0,	//sig
		-1,	//id
		0,	//command
		210, 149, 80, 11,	//x,y,w,h
		"",	//fontname
		0,	//fontsize
		kLeftText,	//fontalign
		0,	//style
		0	//future use
	},
    {
		'airy',	//sig
		'eutx',	//kind
		"song title",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'user',	//sig
		kAudioUnitCustomProperty_SongTitle,	//id
		0,	//command
		10, 164, 180, 14,	//x,y,w,h
		"Arial",	//fontname
		0,	//fontsize
		kLeftText,	//fontalign
		0,	//style
		2	//future use
	},
    {
		'appl',	//sig
		'stxt',	//kind
		"Song Title for spc",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		0,	//sig
		-1,	//id
		0,	//command
		210, 165, 120, 11,	//x,y,w,h
		"",	//fontname
		0,	//fontsize
		kLeftText,	//fontalign
		0,	//style
		0	//future use
	},
    {
		'airy',	//sig
		'eutx',	//kind
		"name of dumper",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'user',	//sig
		kAudioUnitCustomProperty_NameOfDumper,	//id
		0,	//command
		10, 180, 180, 14,	//x,y,w,h
		"Arial",	//fontname
		0,	//fontsize
		kLeftText,	//fontalign
		0,	//style
		2	//future use
	},
    {
		'appl',	//sig
		'stxt',	//kind
		"Name of dumper for spc",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		0,	//sig
		-1,	//id
		0,	//command
		210, 181, 120, 11,	//x,y,w,h
		"",	//fontname
		0,	//fontsize
		kLeftText,	//fontalign
		0,	//style
		0	//future use
	},
    {
		'airy',	//sig
		'eutx',	//kind
        "artist of Song",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'user',	//sig
		kAudioUnitCustomProperty_ArtistOfSong,	//id
		0,	//command
		10, 196, 180, 14,	//x,y,w,h
		"Arial",	//fontname
		0,	//fontsize
		kLeftText,	//fontalign
		0,	//style
		2	//future use
	},
    {
		'appl',	//sig
		'stxt',	//kind
		"Artist of Song for spc",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		0,	//sig
		-1,	//id
		0,	//command
		210, 197, 120, 11,	//x,y,w,h
		"",	//fontname
		0,	//fontsize
		kLeftText,	//fontalign
		0,	//style
		0	//future use
	},
    {
		'airy',	//sig
		'eutx',	//kind
		"comments",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'user',	//sig
		kAudioUnitCustomProperty_SongComments,	//id
		0,	//command
		10, 212, 180, 14,	//x,y,w,h
		"Arial",	//fontname
		0,	//fontsize
		kLeftText,	//fontalign
		0,	//style
		2	//future use
	},
    {
		'appl',	//sig
		'stxt',	//kind
        "Comments for spc",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		0,	//sig
		-1,	//id
		0,	//command
		210, 213, 120, 11,	//x,y,w,h
		"",	//fontname
		0,	//fontsize
		kLeftText,	//fontalign
		0,	//style
		0	//future use
	},
    {
		'airy',	//sig
		'dtxt',	//kind
		"Arial 9 180 248 255 0 2 0.1 x",	//title
		0,	//Value
		0,	//Minimum
		99,	//Maximum
		'user',	//sig
		kAudioUnitCustomProperty_RepeatNumForSpc,	//id
		0,	//command
		10, 228, 64, 12,	//x,y,w,h
		"",	//fontname
		0,	//fontsize
		kCenterText,	//fontalign
		0,	//style
		0	//future use
	},
    {
		'appl',	//sig
		'stxt',	//kind
        "Repeat num for spc",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		0,	//sig
		-1,	//id
		0,	//command
		210, 229, 120, 11,	//x,y,w,h
		"",	//fontname
		0,	//fontsize
		kLeftText,	//fontalign
		0,	//style
		0	//future use
	},
    {
		'airy',	//sig
		'eutx',	//kind
		"",	//title
		0,	//Value
		0,	//Minimum
		99999,	//Maximum
		0,	//sig
		kAudioUnitCustomProperty_FadeMsTimeForSpc,	//id
		0,	//command
		10, 244, 64, 14,	//x,y,w,h
		"Arial",	//fontname
		9,	//fontsize
		kCenterText,	//fontalign
		k3DOut,	//style
		0	//future use
	},
    {
		'appl',	//sig
		'stxt',	//kind
        "Fade milliseconds for spc",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		0,	//sig
		-1,	//id
		0,	//command
		210, 245, 120, 11,	//x,y,w,h
		"",	//fontname
		0,	//fontsize
		kLeftText,	//fontalign
		0,	//style
		0	//future use
	},
    {
		'appl',	//sig
		'stxt',	//kind
		"not Loaded",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		0,	//sig
		kAudioUnitCustomProperty_SongPlayerCodeVer,	//id
		0,	//command
		300, 68, 74, 14,	//x,y,w,h
		"Arial",	//fontname
		9,	//fontsize
		kCenterText,	//fontalign
		k3DOut,	//style
		0	//future use
	},
    {
		'appl',	//sig
		'stxt',	//kind
        "PlayerCode",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		0,	//sig
		-1,	//id
		0,	//command
		210, 69, 120, 11,	//x,y,w,h
		"",	//fontname
		0,	//fontsize
		kLeftText,	//fontalign
		0,	//style
		0	//future use
	},
    {
		'VeMa',	//sig
		'push',	//kind
		"Load",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		0,	//sig
		kControlButtonLoadPlayerCode,	//id
		0,	//command
		378, 66, 32, 14,	//x,y,w,h
		"Arial Black",	//fontname
		9,	//fontsize
		kCenterText,	//fontalign
		0,	//style
		0	//future use
	},
    {
		'VeMa',	//sig
		'push',	//kind
		"OK",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		0,	//sig
		kControlButtonRecordSettingExit,	//id
		0,	//command
		346, 276, 64, 14,	//x,y,w,h
		"Arial Black",	//fontname
		9,	//fontsize
		kCenterText,	//fontalign
		0,	//style
		0	//future use
	},
};

#endif
