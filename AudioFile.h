/*
 *  AudioFile.h
 *  Chip700
 *
 *  Created by 藤田 匡彦 on 12/10/10.
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
	bool	IsVarid();	//ロードが可能であるならtrue
	
private:
	static const unsigned int EXPAND_BUFFER = 4096;
	
	short		*m_pAudioData;
	int			mLoadedSamples;
	InstData	mInstData;
};
