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
