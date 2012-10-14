/*
 *  BRRFile.cpp
 *  Chip700
 *
 *  Created by osoumen on 12/10/10.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "BRRFile.h"

//-----------------------------------------------------------------------------
BRRFile::BRRFile( const char *path, bool isWriteable )
: FileAccess(path, isWriteable)
{
}

//-----------------------------------------------------------------------------
BRRFile::~BRRFile()
{
}
