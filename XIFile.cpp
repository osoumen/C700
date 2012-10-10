/*
 *  XIFile.cpp
 *  Chip700
 *
 *  Created by “¡“c ‹§•F on 12/10/10.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "XIFile.h"

//-----------------------------------------------------------------------------
XIFile::XIFile( const char *path, bool isWriteable )
: FileAccess(path, isWriteable)
{
}

//-----------------------------------------------------------------------------
XIFile::~XIFile()
{
}
