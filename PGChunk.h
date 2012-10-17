/*
 *  PGChunk.h
 *  Chip700
 *
 *  Created by osoumen on 12/10/17.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "Chip700defines.h"
#include "FileAccess.h"

class PGChunk : public FileAccess {
public:
	PGChunk( int allocMemSize );
	PGChunk( unsigned char *data, int dataSize );
	virtual ~PGChunk();
	
	bool				SetDataFromVP( int pgnum, VoiceParams *vp );
	bool				ReadDataToVP( int index, VoiceParams *vp );
	const unsigned char	*GetDataPtr() const { return m_pData; }
	int					GetDataSize() const { return mDataUsed; }
	
private:
	static const int CKID_NULL_DATA	= 0;
	static const int CKID_PROGRAM_TOTAL	= 0x20000;
	static const int CKID_PROGRAM_DATA	= 0x30000;
	typedef struct {
		int		type;
		long	size;	//ヘッダを除いたデータサイズ
	} MyChunkHead;
	
	bool			writeChunk( int type, const void* data, int byte );	//容量不足で全部を書き込めないときはfalse
	bool			readChunkHead( int *type, long *byte );
	bool			readData( void *data, long byte, long *actualReadByte );
	
	bool			mShouldRelease;
	unsigned char	*m_pData;
	int				mDataSize;
	int				mDataUsed;
	int				mDataPos;
	int				mNumPrograms;
};
