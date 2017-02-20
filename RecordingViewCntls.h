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
		"Record Path",	//title
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
		-1,	//id
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
		'appl',	//sig
		'stxt',	//kind
		"1.1.0",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		0,	//sig
		kAudioUnitCustomProperty_RecordStartBeatPos,	//id
		0,	//command
		10, 84, 64, 14,	//x,y,w,h
		"Arial",	//fontname
		9,	//fontsize
		kCenterText,	//fontalign
		k3DOut,	//style
		0	//future use
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
		100, 82, 32, 14,	//x,y,w,h
		"Arial Black",	//fontname
		9,	//fontsize
		kCenterText,	//fontalign
		0,	//style
		0	//future use
	},
    {
		'appl',	//sig
		'stxt',	//kind
		"Record Start",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		0,	//sig
		-1,	//id
		0,	//command
		160, 84, 80, 11,	//x,y,w,h
		"",	//fontname
		0,	//fontsize
		kLeftText,	//fontalign
		0,	//style
		0	//future use
	},
    {
		'appl',	//sig
		'stxt',	//kind
		"1.1.0",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		0,	//sig
		kAudioUnitCustomProperty_RecordLoopStartBeatPos,	//id
		0,	//command
		10, 100, 64, 14,	//x,y,w,h
		"Arial",	//fontname
		9,	//fontsize
		kCenterText,	//fontalign
		k3DOut,	//style
		0	//future use
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
		100, 98, 32, 14,	//x,y,w,h
		"Arial Black",	//fontname
		9,	//fontsize
		kCenterText,	//fontalign
		0,	//style
		0	//future use
	},
    {
		'appl',	//sig
		'stxt',	//kind
		"Loop Start",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		0,	//sig
		-1,	//id
		0,	//command
		160, 100, 80, 11,	//x,y,w,h
		"",	//fontname
		0,	//fontsize
		kLeftText,	//fontalign
		0,	//style
		0	//future use
	},
    {
		'appl',	//sig
		'stxt',	//kind
		"1.1.0",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		0,	//sig
		kAudioUnitCustomProperty_RecordEndBeatPos,	//id
		0,	//command
		10, 116, 64, 14,	//x,y,w,h
		"Arial",	//fontname
		9,	//fontsize
		kCenterText,	//fontalign
		k3DOut,	//style
		0	//future use
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
		100, 114, 32, 14,	//x,y,w,h
		"Arial Black",	//fontname
		9,	//fontsize
		kCenterText,	//fontalign
		0,	//style
		0	//future use
	},
    {
		'appl',	//sig
		'stxt',	//kind
		"Record End",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		0,	//sig
		-1,	//id
		0,	//command
		160, 116, 80, 11,	//x,y,w,h
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
		10, 132, 50, 12,	//x,y,w,h
		"",	//fontname
		9,	//fontsize
		kCenterText,	//fontalign
		0,	//style
		0	//future use
	},
    {
		'appl',	//sig
		'stxt',	//kind
		"Timebase for smc",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		0,	//sig
		-1,	//id
		0,	//command
		160, 132, 80, 11,	//x,y,w,h
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
		10, 148, 120, 14,	//x,y,w,h
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
		160, 148, 80, 11,	//x,y,w,h
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
		10, 164, 120, 14,	//x,y,w,h
		"Arial",	//fontname
		0,	//fontsize
		kLeftText,	//fontalign
		0,	//style
		2	//future use
	},
    {
		'appl',	//sig
		'stxt',	//kind
		"Song Title (for spc)",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		0,	//sig
		-1,	//id
		0,	//command
		160, 164, 120, 11,	//x,y,w,h
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
		10, 180, 120, 14,	//x,y,w,h
		"Arial",	//fontname
		0,	//fontsize
		kLeftText,	//fontalign
		0,	//style
		2	//future use
	},
    {
		'appl',	//sig
		'stxt',	//kind
		"Name of dumper (for spc)",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		0,	//sig
		-1,	//id
		0,	//command
		160, 180, 120, 11,	//x,y,w,h
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
		10, 196, 120, 14,	//x,y,w,h
		"Arial",	//fontname
		0,	//fontsize
		kLeftText,	//fontalign
		0,	//style
		2	//future use
	},
    {
		'appl',	//sig
		'stxt',	//kind
		"Artist of Song (for spc)",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		0,	//sig
		-1,	//id
		0,	//command
		160, 196, 120, 11,	//x,y,w,h
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
		10, 212, 120, 14,	//x,y,w,h
		"Arial",	//fontname
		0,	//fontsize
		kLeftText,	//fontalign
		0,	//style
		2	//future use
	},
    {
		'appl',	//sig
		'stxt',	//kind
        "Comments (for spc)",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		0,	//sig
		-1,	//id
		0,	//command
		160, 212, 120, 11,	//x,y,w,h
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
		-1,	//id
		0,	//command
		10, 228, 64, 14,	//x,y,w,h
		"Arial",	//fontname
		9,	//fontsize
		kCenterText,	//fontalign
		k3DOut,	//style
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
		100, 228, 32, 14,	//x,y,w,h
		"Arial Black",	//fontname
		9,	//fontsize
		kCenterText,	//fontalign
		0,	//style
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
		160, 228, 120, 11,	//x,y,w,h
		"",	//fontname
		0,	//fontsize
		kLeftText,	//fontalign
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
		200, 260, 33, 14,	//x,y,w,h
		"Arial Black",	//fontname
		9,	//fontsize
		kCenterText,	//fontalign
		0,	//style
		0	//future use
	},
};

#endif
