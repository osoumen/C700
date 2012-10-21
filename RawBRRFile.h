/*
 *  RawBRRFile.h
 *  Chip700
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
};
