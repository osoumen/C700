/*
 *  XIFile.h
 *  C700
 *
 *  Created by osoumen on 12/10/10.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "FileAccess.h"
#include "DataBuffer.h"
#include "C700Driver.h"

class XIFile : public FileAccess, public DataBuffer {
public:
	XIFile( const char *path, int allocMemSize=1024*1024 );
	virtual ~XIFile();
	
	virtual bool		Write();
	
	bool				SetDataFromChip( const C700Driver *chip, int targetProgram, double tempo );
	
public:
	
	typedef struct {
		char extxi[21];		// Extended Instrument:
		char name[23];		// Name, 1Ah
		char trkname[20];	// FastTracker v2.00
		unsigned short shsize;		// 0x0102
	} XIFILEHEADER;
	
	typedef struct {
		unsigned char snum[96];
		unsigned short venv[24];
		unsigned short penv[24];
		unsigned char vnum, pnum;
		unsigned char vsustain, vloops, vloope, psustain, ploops, ploope;
		unsigned char vtype, ptype;
		unsigned char vibtype, vibsweep, vibdepth, vibrate;
		unsigned short volfade;
		unsigned short res;
		unsigned char reserved1[20];
		unsigned short reserved2;		//nsamples?
	} XIINSTRUMENTHEADER;
	
	typedef struct {
		unsigned int samplen;
		unsigned int loopstart;
		unsigned int looplen;
		unsigned char vol;
		signed char finetune;
		unsigned char type;
		unsigned char pan;
		signed char relnote;
		unsigned char res;
		char name[22];
	} XISAMPLEHEADER;	
};
