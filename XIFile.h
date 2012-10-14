/*
 *  XIFile.h
 *  Chip700
 *
 *  Created by 藤田 匡彦 on 12/10/10.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "FileAccess.h"
#include "Chip700Generator.h"

class XIFile : public FileAccess {
public:
	XIFile( const char *path, int allocMemSize=1024*1024 );
	virtual ~XIFile();
	
	virtual bool		Write();
	
#if MAC
	void				SetCFData( CFDataRef propertydata );
	CFDataRef			GetCFData() const;
#endif
	
	bool				SetDataFromChip( const Chip700Generator *chip, int targetProgram, double tempo );
	const unsigned char	*GetDataPtr() const { return m_pData; }
	int					GetWriteSize() const { return mDataUsed; }
	
private:
	bool				writeData( void* data, int byte );	//容量不足で全部を書き込めないときはfalse
	bool				setPos( int pos );					//書き込み位置を移動する　出来なかったらfalse
	
#if MAC
	CFDataRef		mCFData;
#endif
	unsigned char	*m_pData;
	int				mDataSize;
	int				mDataUsed;
	int				mDataPos;
	
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
		unsigned long samplen;
		unsigned long loopstart;
		unsigned long looplen;
		unsigned char vol;
		signed char finetune;
		unsigned char type;
		unsigned char pan;
		signed char relnote;
		unsigned char res;
		char name[22];
	} XISAMPLEHEADER;	
};
