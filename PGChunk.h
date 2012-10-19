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
	PGChunk( const void *data, int dataSize );
	virtual ~PGChunk();
	
	bool				AppendDataFromVP( VoiceParams *vp );
	bool				ReadDataToVP( VoiceParams *vp );
	const unsigned char	*GetDataPtr() const { return m_pData; }
	int					GetDataSize() const { return mDataUsed; }
	int					GetDataPos() const { return mDataPos; }
	void				AdvDataPos( int adv ) { mDataPos+=adv; }
	
	bool				writeChunk( int type, const void* data, int byte );	//容量不足で全部を書き込めないときはfalse
	bool				readChunkHead( int *type, long *byte );
	bool				readData( void *data, long byte, long *actualReadByte );
	
	static int			getPGChunkSize( const VoiceParams *vp );

	typedef struct {
		int		type;
		long	size;	//ヘッダを除いたデータサイズ
	} MyChunkHead;
	
private:	
	static const int CKID_NULL_DATA	= 0;
	
	bool			mIsBufferInternal;
	unsigned char	*m_pData;
	int				mDataSize;
	int				mDataUsed;
	int				mDataPos;
	int				mNumPrograms;
	bool			mReadOnly;
};
