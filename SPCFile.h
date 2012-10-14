/*
 *  SPCFile.h
 *  Chip700
 *
 *  Created by osoumen on 12/10/10.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "FileAccess.h"

class SPCFile : public FileAccess {
public:
	SPCFile( const char *path, bool isWriteable );
	virtual ~SPCFile();
	
	unsigned char	*GetRamData() { return m_pRamData; }
	unsigned char	*GetSampleIndex( int sampleIndex, int *size );
	int				GetLoopSizeIndex( int samleIndex );	//ïâêîÇ≈ÉãÅ[Évñ≥Çµ
	
	virtual bool	Load();
	
private:
	static const int SPC_READ_SIZE = 0x101c0;
	
	unsigned char	*m_pFileData;
	unsigned char	*m_pRamData;
	
	int				mSrcTableAddr;
	int				mSampleStart[128];
	int				mLoopSize[128];
	int				mSampleBytes[128];
	bool			mIsLoop[128];
};
