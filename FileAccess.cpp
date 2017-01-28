/*
 *  FileAccess.cpp
 *  C700
 *
 *  Created by osoumen on 12/10/10.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "FileAccess.h"
#include <string.h>

//-----------------------------------------------------------------------------
FileAccess::FileAccess( const char *path, bool isWriteable, int pathLenMax )
: mIsWriteable( isWriteable )
, mIsLoaded( false )
, mPathLenMax( pathLenMax )
{
    mPath = new char[pathLenMax];
	SetFilePath( path );
}

//-----------------------------------------------------------------------------
FileAccess::~FileAccess()
{
    delete mPath;
}

//-----------------------------------------------------------------------------
void FileAccess::SetFilePath( const char *path )
{
	if ( path ) {
		strncpy(mPath, path, mPathLenMax-1);
	}
}
