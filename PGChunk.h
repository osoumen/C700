/*
 *  PGChunk.h
 *  C700
 *
 *  Created by osoumen on 12/10/17.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "C700defines.h"
#include "ChunkReader.h"

class PGChunk : public ChunkReader {
public:
	PGChunk( int allocMemSize );
	PGChunk( const void *data, int dataSize );
	virtual ~PGChunk();
	
	bool				AppendDataFromVP( const InstParams *vp );
	bool				ReadDataToVP( InstParams *vp );
	
	static int			getPGChunkSize( const InstParams *vp );

private:	
	static const int CKID_NULL_DATA	= 0;
	int				mNumPrograms;
};
