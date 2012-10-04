/*
 *  ControlInstances.h
 *  Chip700
 *
 *  Created by osoumen on 12/10/04.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "vstgui.h"

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
} ControlInstances;

ControlInstances	sCntl[] = {
	{
		'airy',	//sig
		'dtxt',	//kind
		"Monaco 9 255 255 255 0 2 1 0",	//title
		0,	//Value
		0,	//Minimum
		128,	//Maximum
		'user',	//sig
		57,	//id
		0,	//command
		363, 1, 14, 11	//x,y,w,h
	},
	{
		'airy',	//sig
		'dtxt',	//kind
		"Monaco 9 255 255 255 0 2 1 0",	//title
		0,	//Value
		0,	//Minimum
		128,	//Maximum
		'user',	//sig
		56,	//id
		0,	//command
		343, 1, 14, 11	//x,y,w,h
	},
	{
		'airy',	//sig
		'dtxt',	//kind
		"Monaco 9 255 255 255 0 2 1 0",	//title
		0,	//Value
		0,	//Minimum
		128,	//Maximum
		'user',	//sig
		55,	//id
		0,	//command
		323, 1, 14, 11	//x,y,w,h
	},
	{
		'airy',	//sig
		'dtxt',	//kind
		"Monaco 9 255 255 255 0 2 1 0",	//title
		0,	//Value
		0,	//Minimum
		128,	//Maximum
		'user',	//sig
		54,	//id
		0,	//command
		303, 1, 14, 11	//x,y,w,h
	},
	{
		'airy',	//sig
		'dtxt',	//kind
		"Monaco 9 255 255 255 0 2 1 0",	//title
		0,	//Value
		0,	//Minimum
		128,	//Maximum
		'user',	//sig
		53,	//id
		0,	//command
		283, 1, 14, 11	//x,y,w,h
	},
	{
		'airy',	//sig
		'dtxt',	//kind
		"Monaco 9 255 255 255 0 2 1 0",	//title
		0,	//Value
		0,	//Minimum
		128,	//Maximum
		'user',	//sig
		52,	//id
		0,	//command
		263, 1, 14, 11	//x,y,w,h
	},
	{
		'airy',	//sig
		'dtxt',	//kind
		"Monaco 9 255 255 255 0 2 1 0",	//title
		0,	//Value
		0,	//Minimum
		128,	//Maximum
		'user',	//sig
		51,	//id
		0,	//command
		243, 1, 14, 11	//x,y,w,h
	},
	{
		'airy',	//sig
		'dtxt',	//kind
		"Monaco 9 255 255 255 0 2 1 0",	//title
		0,	//Value
		0,	//Minimum
		128,	//Maximum
		'user',	//sig
		50,	//id
		0,	//command
		223, 1, 14, 11	//x,y,w,h
	},
	{
		'airy',	//sig
		'dtxt',	//kind
		"Monaco 9 255 255 255 0 2 1 0",	//title
		0,	//Value
		0,	//Minimum
		128,	//Maximum
		'user',	//sig
		49,	//id
		0,	//command
		203, 1, 14, 11	//x,y,w,h
	},
	{
		'airy',	//sig
		'dtxt',	//kind
		"Monaco 9 255 255 255 0 2 1 0",	//title
		0,	//Value
		0,	//Minimum
		128,	//Maximum
		'user',	//sig
		48,	//id
		0,	//command
		183, 1, 14, 11	//x,y,w,h
	},
	{
		'airy',	//sig
		'dtxt',	//kind
		"Monaco 9 255 255 255 0 2 1 0",	//title
		0,	//Value
		0,	//Minimum
		128,	//Maximum
		'user',	//sig
		47,	//id
		0,	//command
		163, 1, 14, 11	//x,y,w,h
	},
	{
		'airy',	//sig
		'dtxt',	//kind
		"Monaco 9 255 255 255 0 2 1 0",	//title
		0,	//Value
		0,	//Minimum
		128,	//Maximum
		'user',	//sig
		46,	//id
		0,	//command
		143, 1, 14, 11	//x,y,w,h
	},
	{
		'airy',	//sig
		'dtxt',	//kind
		"Monaco 9 255 255 255 0 2 1 0",	//title
		0,	//Value
		0,	//Minimum
		128,	//Maximum
		'user',	//sig
		45,	//id
		0,	//command
		123, 1, 14, 11	//x,y,w,h
	},
	{
		'airy',	//sig
		'dtxt',	//kind
		"Monaco 9 255 255 255 0 2 1 0",	//title
		0,	//Value
		0,	//Minimum
		128,	//Maximum
		'user',	//sig
		44,	//id
		0,	//command
		103, 1, 14, 11	//x,y,w,h
	},
	{
		'airy',	//sig
		'dtxt',	//kind
		"Monaco 9 255 255 255 0 2 1 0",	//title
		0,	//Value
		0,	//Minimum
		128,	//Maximum
		'user',	//sig
		43,	//id
		0,	//command
		83, 1, 14, 11	//x,y,w,h
	},
	{
		'airy',	//sig
		'dtxt',	//kind
		"Monaco 9 255 255 255 0 2 1 0",	//title
		0,	//Value
		0,	//Minimum
		128,	//Maximum
		'user',	//sig
		42,	//id
		0,	//command
		63, 1, 14, 11	//x,y,w,h
	},
	{
		'appl',	//sig
		'sepa',	//kind
		"",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		0,	//sig
		0,	//id
		0,	//command
		20, 36, 399, 5	//x,y,w,h
	},
	{
		'appl',	//sig
		'sepa',	//kind
		"",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		0,	//sig
		0,	//id
		0,	//command
		20, 77, 496, 5	//x,y,w,h
	},
	{
		'appl',	//sig
		'sepa',	//kind
		"",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		0,	//sig
		0,	//id
		0,	//command
		20, 268, 496, 5	//x,y,w,h
	},
	{
		'appl',	//sig
		'stxt',	//kind
		"Bank",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		'text',	//sig
		27,	//id
		0,	//command
		24, 93, 22, 11	//x,y,w,h
	},
	{
		'appl',	//sig
		'stxt',	//kind
		"Envelope",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		'text',	//sig
		28,	//id
		0,	//command
		414, 166, 42, 11	//x,y,w,h
	},
	{
		'appl',	//sig
		'stxt',	//kind
		"Filter",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		'text',	//sig
		26,	//id
		0,	//command
		272, 288, 42, 11	//x,y,w,h
	},
	{
		'appl',	//sig
		'stxt',	//kind
		"Feedback",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		'text',	//sig
		25,	//id
		0,	//command
		227, 321, 62, 11	//x,y,w,h
	},
	{
		'appl',	//sig
		'stxt',	//kind
		"Delay Time",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		'text',	//sig
		24,	//id
		0,	//command
		168, 284, 62, 11	//x,y,w,h
	},
	{
		'appl',	//sig
		'stxt',	//kind
		"R",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		'text',	//sig
		23,	//id
		0,	//command
		132, 290, 20, 11	//x,y,w,h
	},
	{
		'appl',	//sig
		'stxt',	//kind
		"L",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		'text',	//sig
		22,	//id
		0,	//command
		105, 290, 20, 11	//x,y,w,h
	},
	{
		'appl',	//sig
		'stxt',	//kind
		"Echo",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		'text',	//sig
		21,	//id
		0,	//command
		107, 278, 44, 11	//x,y,w,h
	},
	{
		'appl',	//sig
		'stxt',	//kind
		"R",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		'text',	//sig
		20,	//id
		0,	//command
		55, 290, 20, 11	//x,y,w,h
	},
	{
		'appl',	//sig
		'stxt',	//kind
		"L",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		'text',	//sig
		19,	//id
		0,	//command
		28, 290, 20, 11	//x,y,w,h
	},
	{
		'appl',	//sig
		'stxt',	//kind
		"Main",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		'text',	//sig
		18,	//id
		0,	//command
		29, 278, 44, 11	//x,y,w,h
	},
	{
		'appl',	//sig
		'stxt',	//kind
		"Vib.Rate",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		'text',	//sig
		17,	//id
		0,	//command
		474, 8, 44, 11	//x,y,w,h
	},
	{
		'appl',	//sig
		'stxt',	//kind
		"Vib.Depth",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		'text',	//sig
		16,	//id
		0,	//command
		427, 8, 44, 11	//x,y,w,h
	},
	{
		'appl',	//sig
		'stxt',	//kind
		"SR",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		'text',	//sig
		15,	//id
		0,	//command
		497, 180, 19, 11	//x,y,w,h
	},
	{
		'appl',	//sig
		'stxt',	//kind
		"SL",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		'text',	//sig
		14,	//id
		0,	//command
		470, 180, 19, 11	//x,y,w,h
	},
	{
		'appl',	//sig
		'stxt',	//kind
		"DR",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		'text',	//sig
		13,	//id
		0,	//command
		443, 180, 19, 11	//x,y,w,h
	},
	{
		'appl',	//sig
		'stxt',	//kind
		"AR",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		'text',	//sig
		12,	//id
		0,	//command
		416, 180, 19, 11	//x,y,w,h
	},
	{
		'appl',	//sig
		'stxt',	//kind
		"High Key",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		'text',	//sig
		11,	//id
		0,	//command
		2, 113, 38, 11	//x,y,w,h
	},
	{
		'appl',	//sig
		'stxt',	//kind
		"Low Key",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		'text',	//sig
		10,	//id
		0,	//command
		2, 133, 38, 11	//x,y,w,h
	},
	{
		'appl',	//sig
		'stxt',	//kind
		"Root Key",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		'text',	//sig
		9,	//id
		0,	//command
		2, 153, 38, 11	//x,y,w,h
	},
	{
		'appl',	//sig
		'stxt',	//kind
		"Sample Rate",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		'text',	//sig
		8,	//id
		0,	//command
		4, 211, 53, 11	//x,y,w,h
	},
	{
		'appl',	//sig
		'stxt',	//kind
		"R",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		'text',	//sig
		7,	//id
		0,	//command
		500, 103, 15, 11	//x,y,w,h
	},
	{
		'appl',	//sig
		'stxt',	//kind
		"L",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		'text',	//sig
		6,	//id
		0,	//command
		461, 103, 15, 11	//x,y,w,h
	},
	{
		'appl',	//sig
		'stxt',	//kind
		"Volume",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		'text',	//sig
		5,	//id
		0,	//command
		462, 92, 51, 11	//x,y,w,h
	},
	{
		'appl',	//sig
		'stxt',	//kind
		"Loop Point",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		'text',	//sig
		4,	//id
		0,	//command
		0, 173, 57, 11	//x,y,w,h
	},
	{
		'appl',	//sig
		'stxt',	//kind
		"Bend Range",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		'text',	//sig
		3,	//id
		0,	//command
		84, 55, 58, 11	//x,y,w,h
	},
	{
		'airy',	//sig
		'bttn',	//kind
		"bank_d",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'bank',	//sig
		3,	//id
		'bnkd',	//command
		96, 91, 12, 12	//x,y,w,h
	},
	{
		'airy',	//sig
		'bttn',	//kind
		"bank_c",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'bank',	//sig
		2,	//id
		'bnkc',	//command
		81, 91, 12, 12	//x,y,w,h
	},
	{
		'airy',	//sig
		'bttn',	//kind
		"bank_b",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'bank',	//sig
		1,	//id
		'bnkb',	//command
		66, 91, 12, 12	//x,y,w,h
	},
	{
		'airy',	//sig
		'bttn',	//kind
		"bank_a",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'bank',	//sig
		0,	//id
		'bnka',	//command
		51, 91, 12, 12	//x,y,w,h
	},
	{
		'airy',	//sig
		'valp',	//kind
		"track16_",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'trac',	//sig
		41,	//id
		'traF',	//command
		360, 13, 20, 20	//x,y,w,h
	},
	{
		'airy',	//sig
		'valp',	//kind
		"track15_",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'trac',	//sig
		40,	//id
		'traE',	//command
		340, 13, 20, 20	//x,y,w,h
	},
	{
		'airy',	//sig
		'valp',	//kind
		"track14_",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'trac',	//sig
		39,	//id
		'traD',	//command
		320, 13, 20, 20	//x,y,w,h
	},
	{
		'airy',	//sig
		'valp',	//kind
		"track13_",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'trac',	//sig
		38,	//id
		'traC',	//command
		300, 13, 20, 20	//x,y,w,h
	},
	{
		'airy',	//sig
		'valp',	//kind
		"track12_",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'trac',	//sig
		37,	//id
		'traB',	//command
		280, 13, 20, 20	//x,y,w,h
	},
	{
		'airy',	//sig
		'valp',	//kind
		"track11_",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'trac',	//sig
		36,	//id
		'traA',	//command
		260, 13, 20, 20	//x,y,w,h
	},
	{
		'airy',	//sig
		'valp',	//kind
		"track10_",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'trac',	//sig
		35,	//id
		'tra9',	//command
		240, 13, 20, 20	//x,y,w,h
	},
	{
		'airy',	//sig
		'valp',	//kind
		"track09_",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'trac',	//sig
		34,	//id
		'tra8',	//command
		220, 13, 20, 20	//x,y,w,h
	},
	{
		'airy',	//sig
		'valp',	//kind
		"track08_",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'trac',	//sig
		33,	//id
		'tra7',	//command
		200, 13, 20, 20	//x,y,w,h
	},
	{
		'airy',	//sig
		'valp',	//kind
		"track07_",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'trac',	//sig
		32,	//id
		'tra6',	//command
		180, 13, 20, 20	//x,y,w,h
	},
	{
		'airy',	//sig
		'valp',	//kind
		"track06_",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'trac',	//sig
		31,	//id
		'tra5',	//command
		160, 13, 20, 20	//x,y,w,h
	},
	{
		'airy',	//sig
		'valp',	//kind
		"track05_",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'trac',	//sig
		30,	//id
		'tra4',	//command
		140, 13, 20, 20	//x,y,w,h
	},
	{
		'airy',	//sig
		'valp',	//kind
		"track04_",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'trac',	//sig
		29,	//id
		'tra3',	//command
		120, 13, 20, 20	//x,y,w,h
	},
	{
		'airy',	//sig
		'valp',	//kind
		"track03_",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'trac',	//sig
		28,	//id
		'tra2',	//command
		100, 13, 20, 20	//x,y,w,h
	},
	{
		'airy',	//sig
		'valp',	//kind
		"track02_",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'trac',	//sig
		27,	//id
		'tra1',	//command
		80, 13, 20, 20	//x,y,w,h
	},
	{
		'airy',	//sig
		'valp',	//kind
		"track01_",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'trac',	//sig
		26,	//id
		'tra0',	//command
		60, 13, 20, 20	//x,y,w,h
	},
	{
		'appl',	//sig
		'stxt',	//kind
		"0 bytes",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		'text',	//sig
		1,	//id
		0,	//command
		444, 392, 84, 14	//x,y,w,h
	},
	{
		'appl',	//sig
		'stxt',	//kind
		"Poly",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		'text',	//sig
		2,	//id
		0,	//command
		34, 55, 23, 11	//x,y,w,h
	},
	{
		'appl',	//sig
		'stxt',	//kind
		"0123456abcde",	//title
		0,	//Value
		0,	//Minimum
		0,	//Maximum
		'text',	//sig
		0,	//id
		0,	//command
		132, 390, 159, 11	//x,y,w,h
	},
	{
		'airy',	//sig
		'bttn',	//kind
		"bt_copy",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		0,	//sig
		0,	//id
		'copy',	//command
		298, 389, 33, 14	//x,y,w,h
	},
	{
		'airy',	//sig
		'slid',	//kind
		"slider",	//title
		32767,	//Value
		0,	//Minimum
		32767,	//Maximum
		'user',	//sig
		22,	//id
		0,	//command
		451, 308, 16, 80	//x,y,w,h
	},
	{
		'airy',	//sig
		'slid',	//kind
		"slider",	//title
		32767,	//Value
		0,	//Minimum
		32767,	//Maximum
		'user',	//sig
		21,	//id
		0,	//command
		427, 308, 16, 80	//x,y,w,h
	},
	{
		'airy',	//sig
		'slid',	//kind
		"slider",	//title
		32767,	//Value
		0,	//Minimum
		32767,	//Maximum
		'user',	//sig
		20,	//id
		0,	//command
		403, 308, 16, 80	//x,y,w,h
	},
	{
		'airy',	//sig
		'slid',	//kind
		"slider",	//title
		32767,	//Value
		0,	//Minimum
		32767,	//Maximum
		'user',	//sig
		19,	//id
		0,	//command
		379, 308, 16, 80	//x,y,w,h
	},
	{
		'airy',	//sig
		'slid',	//kind
		"slider",	//title
		32767,	//Value
		0,	//Minimum
		32767,	//Maximum
		'user',	//sig
		18,	//id
		0,	//command
		355, 308, 16, 80	//x,y,w,h
	},
	{
		'airy',	//sig
		'dtxt',	//kind
		"Monaco 10 180 248 255 0 2 1 0",	//title
		-70,	//Value
		-128,	//Minimum
		127,	//Maximum
		'AUid',	//sig
		1043,	//id
		0,	//command
		246, 368, 24, 12	//x,y,w,h
	},
	{
		'airy',	//sig
		'dtxt',	//kind
		"Monaco 10 180 248 255 0 2 1 0",	//title
		-50,	//Value
		-128,	//Minimum
		127,	//Maximum
		'AUid',	//sig
		1042,	//id
		0,	//command
		130, 383, 24, 12	//x,y,w,h
	},
	{
		'airy',	//sig
		'dtxt',	//kind
		"Monaco 10 180 248 255 0 2 1 0",	//title
		50,	//Value
		-128,	//Minimum
		127,	//Maximum
		'AUid',	//sig
		1041,	//id
		0,	//command
		103, 383, 24, 12	//x,y,w,h
	},
	{
		'airy',	//sig
		'dtxt',	//kind
		"Monaco 10 180 248 255 0 2 1 0",	//title
		127,	//Value
		-128,	//Minimum
		127,	//Maximum
		'AUid',	//sig
		1002,	//id
		0,	//command
		53, 383, 24, 12	//x,y,w,h
	},
	{
		'airy',	//sig
		'dtxt',	//kind
		"Monaco 10 180 248 255 0 2 1 0",	//title
		127,	//Value
		-128,	//Minimum
		127,	//Maximum
		'AUid',	//sig
		1001,	//id
		0,	//command
		26, 383, 24, 12	//x,y,w,h
	},
	{
		'airy',	//sig
		'dtxt',	//kind
		"Monaco 10 180 248 255 0 2 16 ms",	//title
		6,	//Value
		0,	//Minimum
		15,	//Maximum
		'AUid',	//sig
		1044,	//id
		0,	//command
		229, 282, 34, 12	//x,y,w,h
	},
	{
		'airy',	//sig
		'eutx',	//kind
		"0",	//title
		0,	//Value
		-128,	//Minimum
		127,	//Maximum
		'AUid',	//sig
		52,	//id
		0,	//command
		487, 285, 24, 14	//x,y,w,h
	},
	{
		'airy',	//sig
		'eutx',	//kind
		"0",	//title
		0,	//Value
		-128,	//Minimum
		127,	//Maximum
		'AUid',	//sig
		51,	//id
		0,	//command
		461, 285, 24, 14	//x,y,w,h
	},
	{
		'airy',	//sig
		'eutx',	//kind
		"0",	//title
		0,	//Value
		-128,	//Minimum
		127,	//Maximum
		'AUid',	//sig
		50,	//id
		0,	//command
		436, 285, 24, 14	//x,y,w,h
	},
	{
		'airy',	//sig
		'eutx',	//kind
		"0",	//title
		0,	//Value
		-128,	//Minimum
		127,	//Maximum
		'AUid',	//sig
		49,	//id
		0,	//command
		411, 285, 24, 14	//x,y,w,h
	},
	{
		'airy',	//sig
		'eutx',	//kind
		"0",	//title
		0,	//Value
		-128,	//Minimum
		127,	//Maximum
		'AUid',	//sig
		48,	//id
		0,	//command
		386, 285, 24, 14	//x,y,w,h
	},
	{
		'airy',	//sig
		'eutx',	//kind
		"0",	//title
		0,	//Value
		-128,	//Minimum
		127,	//Maximum
		'AUid',	//sig
		47,	//id
		0,	//command
		361, 285, 24, 14	//x,y,w,h
	},
	{
		'airy',	//sig
		'eutx',	//kind
		"0",	//title
		0,	//Value
		-128,	//Minimum
		127,	//Maximum
		'AUid',	//sig
		46,	//id
		0,	//command
		336, 285, 24, 14	//x,y,w,h
	},
	{
		'airy',	//sig
		'eutx',	//kind
		"127",	//title
		0,	//Value
		-128,	//Minimum
		127,	//Maximum
		'AUid',	//sig
		45,	//id
		0,	//command
		310, 285, 24, 14	//x,y,w,h
	},
	{
		'airy',	//sig
		'slid',	//kind
		"slider",	//title
		6,	//Value
		0,	//Minimum
		15,	//Maximum
		'AUid',	//sig
		44,	//id
		0,	//command
		168, 301, 101, 16	//x,y,w,h
	},
	{
		'airy',	//sig
		'knob',	//kind
		"knob",	//title
		-70,	//Value
		-128,	//Minimum
		127,	//Maximum
		'AUid',	//sig
		43,	//id
		0,	//command
		242, 333, 32, 32	//x,y,w,h
	},
	{
		'airy',	//sig
		'slid',	//kind
		"slider",	//title
		-50,	//Value
		-128,	//Minimum
		127,	//Maximum
		'AUid',	//sig
		42,	//id
		0,	//command
		134, 301, 16, 80	//x,y,w,h
	},
	{
		'airy',	//sig
		'slid',	//kind
		"slider",	//title
		50,	//Value
		-128,	//Minimum
		127,	//Maximum
		'AUid',	//sig
		41,	//id
		0,	//command
		107, 301, 16, 80	//x,y,w,h
	},
	{
		'airy',	//sig
		'slid',	//kind
		"slider",	//title
		127,	//Value
		-128,	//Minimum
		127,	//Maximum
		'AUid',	//sig
		2,	//id
		0,	//command
		57, 301, 16, 80	//x,y,w,h
	},
	{
		'airy',	//sig
		'slid',	//kind
		"slider",	//title
		127,	//Value
		-128,	//Minimum
		127,	//Maximum
		'AUid',	//sig
		1,	//id
		0,	//command
		30, 301, 16, 80	//x,y,w,h
	},
	{
		'airy',	//sig
		'dtxt',	//kind
		"Monaco 10 180 248 255 0 2 1 0",	//title
		2,	//Value
		1,	//Minimum
		24,	//Maximum
		'AUid',	//sig
		7,	//id
		0,	//command
		142, 53, 18, 12	//x,y,w,h
	},
	{
		'airy',	//sig
		'bttn',	//kind
		"bt_preemphasis",	//title
		1,	//Value
		0,	//Minimum
		1,	//Maximum
		0,	//sig
		0,	//id
		'emph',	//command
		321, 248, 67, 12	//x,y,w,h
	},
	{
		'airy',	//sig
		'bttn',	//kind
		"bt_unload",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		0,	//sig
		0,	//id
		'unlo',	//command
		116, 248, 41, 14	//x,y,w,h
	},
	{
		'airy',	//sig
		'bttn',	//kind
		"bt_load",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		0,	//sig
		0,	//id
		'load',	//command
		273, 248, 40, 14	//x,y,w,h
	},
	{
		'airy',	//sig
		'bttn',	//kind
		"bt_save",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		0,	//sig
		0,	//id
		'save',	//command
		165, 248, 40, 14	//x,y,w,h
	},
	{
		'airy',	//sig
		'bttn',	//kind
		"bt_save_xi",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		0,	//sig
		0,	//id
		'sav2',	//command
		213, 248, 52, 14	//x,y,w,h
	},
	{
		'airy',	//sig
		'bttn',	//kind
		"bt_calc",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		0,	//sig
		0,	//id
		'drat',	//command
		81, 224, 29, 14	//x,y,w,h
	},
	{
		'airy',	//sig
		'bttn',	//kind
		"bt_calc",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		0,	//sig
		0,	//id
		'dkey',	//command
		81, 153, 29, 14	//x,y,w,h
	},
	{
		'VeMa',	//sig
		'wave',	//kind
		"",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'user',	//sig
		2001,	//id
		0,	//command
		258, 172, 144, 72	//x,y,w,h
	},
	{
		'VeMa',	//sig
		'wave',	//kind
		"",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'user',	//sig
		1001,	//id
		0,	//command
		114, 172, 144, 72	//x,y,w,h
	},
	{
		'VeMa',	//sig
		'wave',	//kind
		"",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'user',	//sig
		1,	//id
		0,	//command
		114, 108, 288, 64	//x,y,w,h
	},
	{
		'airy',	//sig
		'eutx',	//kind
		"",	//title
		0,	//Value
		0,	//Minimum
		127,	//Maximum
		'user',	//sig
		1016,	//id
		0,	//command
		114, 90, 23, 14	//x,y,w,h
	},
	{
		'airy',	//sig
		'eutx',	//kind
		"",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'user',	//sig
		1006,	//id
		0,	//command
		12, 186, 46, 14	//x,y,w,h
	},
	{
		'airy',	//sig
		'eutx',	//kind
		"",	//title
		0,	//Value
		0,	//Minimum
		127,	//Maximum
		'user',	//sig
		5,	//id
		0,	//command
		45, 112, 32, 14	//x,y,w,h
	},
	{
		'airy',	//sig
		'eutx',	//kind
		"",	//title
		0,	//Value
		0,	//Minimum
		127,	//Maximum
		'user',	//sig
		4,	//id
		0,	//command
		45, 132, 32, 14	//x,y,w,h
	},
	{
		'airy',	//sig
		'eutx',	//kind
		"",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'user',	//sig
		2,	//id
		0,	//command
		12, 223, 66, 14	//x,y,w,h
	},
	{
		'appl',	//sig
		'larr',	//kind
		"",	//title
		0,	//Value
		0,	//Minimum
		10000000,	//Maximum
		'user',	//sig
		6,	//id
		0,	//command
		59, 184, 13, 19	//x,y,w,h
	},
	{
		'appl',	//sig
		'larr',	//kind
		"",	//title
		0,	//Value
		0,	//Minimum
		127,	//Maximum
		'user',	//sig
		16,	//id
		0,	//command
		137, 87, 13, 19	//x,y,w,h
	},
	{
		'airy',	//sig
		'eutx',	//kind
		"",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'user',	//sig
		0,	//id
		0,	//command
		153, 90, 249, 14	//x,y,w,h
	},
	{
		'airy',	//sig
		'eutx',	//kind
		"",	//title
		0,	//Value
		0,	//Minimum
		127,	//Maximum
		'user',	//sig
		3,	//id
		0,	//command
		45, 152, 32, 14	//x,y,w,h
	},
	{
		'airy',	//sig
		'dtxt',	//kind
		"Monaco 10 180 248 255 0 2 1 0",	//title
		0,	//Value
		0,	//Minimum
		31,	//Maximum
		'user',	//sig
		1011,	//id
		0,	//command
		495, 248, 24, 12	//x,y,w,h
	},
	{
		'airy',	//sig
		'dtxt',	//kind
		"Monaco 10 180 248 255 0 2 1 0",	//title
		0,	//Value
		0,	//Minimum
		7,	//Maximum
		'user',	//sig
		1010,	//id
		0,	//command
		468, 248, 24, 12	//x,y,w,h
	},
	{
		'airy',	//sig
		'dtxt',	//kind
		"Monaco 10 180 248 255 0 2 1 0",	//title
		0,	//Value
		0,	//Minimum
		7,	//Maximum
		'user',	//sig
		1009,	//id
		0,	//command
		441, 248, 24, 12	//x,y,w,h
	},
	{
		'airy',	//sig
		'dtxt',	//kind
		"Monaco 10 180 248 255 0 2 1 0",	//title
		0,	//Value
		0,	//Minimum
		15,	//Maximum
		'user',	//sig
		1008,	//id
		0,	//command
		414, 248, 24, 12	//x,y,w,h
	},
	{
		'airy',	//sig
		'slid',	//kind
		"slider",	//title
		0,	//Value
		0,	//Minimum
		31,	//Maximum
		'user',	//sig
		11,	//id
		0,	//command
		499, 191, 16, 55	//x,y,w,h
	},
	{
		'airy',	//sig
		'slid',	//kind
		"slider",	//title
		0,	//Value
		0,	//Minimum
		7,	//Maximum
		'user',	//sig
		10,	//id
		0,	//command
		472, 191, 16, 55	//x,y,w,h
	},
	{
		'airy',	//sig
		'slid',	//kind
		"slider",	//title
		0,	//Value
		0,	//Minimum
		7,	//Maximum
		'user',	//sig
		9,	//id
		0,	//command
		445, 191, 16, 55	//x,y,w,h
	},
	{
		'airy',	//sig
		'slid',	//kind
		"slider",	//title
		0,	//Value
		0,	//Minimum
		15,	//Maximum
		'user',	//sig
		8,	//id
		0,	//command
		418, 191, 16, 55	//x,y,w,h
	},
	{
		'airy',	//sig
		'bttn',	//kind
		"bt_echo",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'user',	//sig
		14,	//id
		0,	//command
		414, 91, 33, 12	//x,y,w,h
	},
	{
		'airy',	//sig
		'bttn',	//kind
		"bt_loop",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'user',	//sig
		7,	//id
		0,	//command
		76, 187, 33, 12	//x,y,w,h
	},
	{
		'airy',	//sig
		'bttn',	//kind
		"bt_multi_bank_d",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'AUid',	//sig
		55,	//id
		0,	//command
		342, 59, 80, 12	//x,y,w,h
	},
	{
		'airy',	//sig
		'bttn',	//kind
		"bt_multi_bank_c",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'AUid',	//sig
		54,	//id
		0,	//command
		342, 47, 80, 12	//x,y,w,h
	},
	{
		'airy',	//sig
		'bttn',	//kind
		"bt_multi_bank_b",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'AUid',	//sig
		53,	//id
		0,	//command
		259, 59, 80, 12	//x,y,w,h
	},
	{
		'airy',	//sig
		'bttn',	//kind
		"bt_multi_bank_a",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'AUid',	//sig
		10,	//id
		0,	//command
		259, 47, 80, 12	//x,y,w,h
	},
	{
		'airy',	//sig
		'bttn',	//kind
		"bt_velocitysens",	//title
		1,	//Value
		0,	//Minimum
		2,	//Maximum
		'AUid',	//sig
		6,	//id
		0,	//command
		171, 53, 80, 12	//x,y,w,h
	},
	{
		'airy',	//sig
		'eutx',	//kind
		"7.0000",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'AUid',	//sig
		1004,	//id
		0,	//command
		476, 59, 40, 14	//x,y,w,h
	},
	{
		'airy',	//sig
		'eutx',	//kind
		"1.000",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'AUid',	//sig
		1005,	//id
		0,	//command
		430, 59, 40, 14	//x,y,w,h
	},
	{
		'airy',	//sig
		'knob',	//kind
		"knob",	//title
		199771360,	//Value
		0,	//Minimum
		1000000000,	//Maximum
		'AUid',	//sig
		4,	//id
		0,	//command
		480, 24, 32, 32	//x,y,w,h
	},
	{
		'airy',	//sig
		'knob',	//kind
		"knob",	//title
		0,	//Value
		0,	//Minimum
		1000000000,	//Maximum
		'AUid',	//sig
		5,	//id
		0,	//command
		434, 24, 32, 32	//x,y,w,h
	},
	{
		'airy',	//sig
		'dtxt',	//kind
		"Monaco 10 180 248 255 0 2 1 0",	//title
		0,	//Value
		-128,	//Minimum
		127,	//Maximum
		'user',	//sig
		1013,	//id
		0,	//command
		496, 148, 24, 12	//x,y,w,h
	},
	{
		'airy',	//sig
		'knob',	//kind
		"knob",	//title
		0,	//Value
		-128,	//Minimum
		127,	//Maximum
		'user',	//sig
		13,	//id
		0,	//command
		492, 114, 32, 32	//x,y,w,h
	},
	{
		'airy',	//sig
		'dtxt',	//kind
		"Monaco 10 180 248 255 0 2 1 0",	//title
		0,	//Value
		-128,	//Minimum
		127,	//Maximum
		'user',	//sig
		1012,	//id
		0,	//command
		456, 148, 24, 12	//x,y,w,h
	},
	{
		'airy',	//sig
		'dtxt',	//kind
		"Monaco 10 180 248 255 0 2 1 0",	//title
		8,	//Value
		1,	//Minimum
		16,	//Maximum
		'AUid',	//sig
		0,	//id
		0,	//command
		58, 53, 18, 12	//x,y,w,h
	},
	{
		'airy',	//sig
		'knob',	//kind
		"knob",	//title
		0,	//Value
		-128,	//Minimum
		127,	//Maximum
		'user',	//sig
		12,	//id
		0,	//command
		452, 114, 32, 32	//x,y,w,h
	},
	{
		'airy',	//sig
		'bttn',	//kind
		"ind_track",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'tsel',	//sig
		3015,	//id
		0,	//command
		360, 13, 20, 20	//x,y,w,h
	},
	{
		'airy',	//sig
		'bttn',	//kind
		"ind_track",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'tsel',	//sig
		3014,	//id
		0,	//command
		340, 13, 20, 20	//x,y,w,h
	},
	{
		'airy',	//sig
		'bttn',	//kind
		"ind_track",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'tsel',	//sig
		3013,	//id
		0,	//command
		320, 13, 20, 20	//x,y,w,h
	},
	{
		'airy',	//sig
		'bttn',	//kind
		"ind_track",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'tsel',	//sig
		3012,	//id
		0,	//command
		300, 13, 20, 20	//x,y,w,h
	},
	{
		'airy',	//sig
		'bttn',	//kind
		"ind_track",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'tsel',	//sig
		3011,	//id
		0,	//command
		280, 13, 20, 20	//x,y,w,h
	},
	{
		'airy',	//sig
		'bttn',	//kind
		"ind_track",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'tsel',	//sig
		3010,	//id
		0,	//command
		260, 13, 20, 20	//x,y,w,h
	},
	{
		'airy',	//sig
		'bttn',	//kind
		"ind_track",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'tsel',	//sig
		3009,	//id
		0,	//command
		240, 13, 20, 20	//x,y,w,h
	},
	{
		'airy',	//sig
		'bttn',	//kind
		"ind_track",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'tsel',	//sig
		3008,	//id
		0,	//command
		220, 13, 20, 20	//x,y,w,h
	},
	{
		'airy',	//sig
		'bttn',	//kind
		"ind_track",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'tsel',	//sig
		3007,	//id
		0,	//command
		200, 13, 20, 20	//x,y,w,h
	},
	{
		'airy',	//sig
		'bttn',	//kind
		"ind_track",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'tsel',	//sig
		3006,	//id
		0,	//command
		180, 13, 20, 20	//x,y,w,h
	},
	{
		'airy',	//sig
		'bttn',	//kind
		"ind_track",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'tsel',	//sig
		3005,	//id
		0,	//command
		160, 13, 20, 20	//x,y,w,h
	},
	{
		'airy',	//sig
		'bttn',	//kind
		"ind_track",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'tsel',	//sig
		3004,	//id
		0,	//command
		140, 13, 20, 20	//x,y,w,h
	},
	{
		'airy',	//sig
		'bttn',	//kind
		"ind_track",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'tsel',	//sig
		3003,	//id
		0,	//command
		120, 13, 20, 20	//x,y,w,h
	},
	{
		'airy',	//sig
		'bttn',	//kind
		"ind_track",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'tsel',	//sig
		3002,	//id
		0,	//command
		100, 13, 20, 20	//x,y,w,h
	},
	{
		'airy',	//sig
		'bttn',	//kind
		"ind_track",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'tsel',	//sig
		3001,	//id
		0,	//command
		80, 13, 20, 20	//x,y,w,h
	},
	{
		'airy',	//sig
		'bttn',	//kind
		"ind_track",	//title
		0,	//Value
		0,	//Minimum
		1,	//Maximum
		'tsel',	//sig
		3000,	//id
		0,	//command
		60, 13, 20, 20	//x,y,w,h
	}	
};
