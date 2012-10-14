/*
 *  PlistBRRFile.h
 *  Chip700
 *
 *  Created by osoumen on 12/10/11.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "FileAccess.h"

class PlistBRRFile : public FileAccess {
public:
	PlistBRRFile( const char *path, bool isWriteable=true );
	virtual ~PlistBRRFile();
#if __APPLE_CC__
	
	virtual bool	Load();
	virtual bool	Write();
	
	void				SetPlistData( CFPropertyListRef propertydata );
	CFPropertyListRef	GetPlistData() const;
	
public:
	CFPropertyListRef	mPropertydata;
#endif
};
