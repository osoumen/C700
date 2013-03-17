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
FileAccess::FileAccess( const char *path, bool isWriteable )
: mIsWriteable( isWriteable )
, mIsLoaded( false )
{
	SetFilePath( path );
}

//-----------------------------------------------------------------------------
FileAccess::~FileAccess()
{
}

//-----------------------------------------------------------------------------
void FileAccess::SetFilePath( const char *path )
{
	if ( path ) {
		strncpy(mPath, path, PATH_LEN_MAX-1);
	}
}
