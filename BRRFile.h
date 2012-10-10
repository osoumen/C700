/*
 *  BRRFile.h
 *  Chip700
 *
 *  Created by “¡“c ‹§•F on 12/10/10.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "FileAccess.h"

class BRRFile : public FileAccess {
public:
	BRRFile( const char *path, bool isWriteable );
	virtual ~BRRFile();
};
