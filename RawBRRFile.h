/*
 *  RawBRRFile.h
 *  C700
 *
 *  Created by osoumen on 12/10/10.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "FileAccess.h"

class RawBRRFile : public FileAccess {
public:
	static const int	HAS_PGNAME = 1 << 0;
	static const int	HAS_RATE = 1 << 1;
	static const int	HAS_BASEKEY = 1 << 2;
	static const int	HAS_LOWKEY = 1 << 3;
	static const int	HAS_HIGHKEY = 1 << 4;
	static const int	HAS_AR = 1 << 5;
	static const int	HAS_DR = 1 << 6;
	static const int	HAS_SL = 1 << 7;
	static const int	HAS_SR = 1 << 8;
	static const int	HAS_VOLL = 1 << 9;
	static const int	HAS_VOLR = 1 << 10;
	static const int	HAS_ECHO = 1 << 11;
	static const int	HAS_BANK = 1 << 12;
	static const int	HAS_ISEMPHASIZED = 1 << 13;
	static const int	HAS_SOURCEFILE = 1 << 14;
    static const int    HAS_SUSTAINMODE = 1 << 15;
	
	RawBRRFile( const char *path, bool isWriteable=true );
	virtual ~RawBRRFile();
	
	virtual bool	Load();
	virtual bool	Write();
	
	const InstParams		*GetLoadedInst() const;
	unsigned int			GetHasFlag() const { return mHasData; }
	void					StoreInst( const InstParams *inst);

private:
	static const int	MAX_FILE_SIZE = 65538;
	
	unsigned char	mFileData[MAX_FILE_SIZE];
	int				mFileSize;
	
	InstParams		mInst;
	char			mInstFilePath[PATH_LEN_MAX+1];
	unsigned int	mHasData;
	
	bool			tryLoad(bool noLoopPoint);
};
