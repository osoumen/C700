/*
 *  AudioFile.h
 *  Chip700
 *
 *  Created by “¡“c ‹§•F on 12/10/10.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "Chip700defines.h"
#include "FileAccess.h"

class AudioFile : public FileAccess {
public:
	AudioFile( const char *path, bool isWriteable );
	virtual ~AudioFile();
	
	virtual bool	Load();

	short	*GetAudioData();
	int		GetLoadedSamples();
	bool	GetInstData( InstData *instData );
	
private:
	static const unsigned int EXPAND_BUFFER = 4096;
	
	short		*m_pAudioData;
	int			mLoadedSamples;
	InstData	mInstData;
};
