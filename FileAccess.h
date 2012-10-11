/*
 *  FileAccess.h
 *  Chip700
 *
 *  Created by “¡“c ‹§•F on 12/10/10.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

class FileAccess {
public:
	FileAccess( const char *path, bool isWriteable );
	virtual ~FileAccess();
	
	const char*		GetFilePath() { return mPath; }
	void			SetFilePath( const char *path );
	virtual bool	Load() { return false; };
	virtual bool	Write() { return false; }
	virtual bool	IsLoaded() const { return mIsLoaded; }
	
protected:
	char		mPath[1024];
	bool		mIsWriteable;
	bool		mIsLoaded;
};
