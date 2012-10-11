/*
 *  FileAccess.cpp
 *  Chip700
 *
 *  Created by “¡“c ‹§•F on 12/10/10.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "FileAccess.h"

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
		strncpy(mPath, path, 1023);
	}
}
