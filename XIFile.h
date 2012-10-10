/*
 *  XIFile.h
 *  Chip700
 *
 *  Created by “¡“c ‹§•F on 12/10/10.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "FileAccess.h"

class XIFile : public FileAccess {
public:
	XIFile( const char *path, bool isWriteable );
	virtual ~XIFile();
};
