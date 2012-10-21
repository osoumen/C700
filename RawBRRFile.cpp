/*
 *  RawBRRFile.cpp
 *  Chip700
 *
 *  Created by osoumen on 12/10/10.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "RawBRRFile.h"

//-----------------------------------------------------------------------------
RawBRRFile::RawBRRFile( const char *path, bool isWriteable )
: FileAccess(path, isWriteable)
{
}

//-----------------------------------------------------------------------------
RawBRRFile::~RawBRRFile()
{
}
