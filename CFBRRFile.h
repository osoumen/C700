/*
 *  CFBRRFile.h
 *  Chip700
 *
 *  Created by “¡“c ‹§•F on 12/10/11.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "FileAccess.h"

class CFBRRFile : public FileAccess {
public:
	CFBRRFile( const char *path, bool isWriteable );
	virtual ~CFBRRFile();
	
	virtual bool	Load();
	virtual bool	Write();
	
	void SetDictionaryData( CFDictionaryRef propertydata );
	
public:
	CFDictionaryRef	mPropertydata;
};
