/*
 *  PGChunk.cpp
 *  Chip700
 *
 *  Created by osoumen on 12/10/17.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "PGChunk.h"

//-----------------------------------------------------------------------------
PGChunk::PGChunk(int allocMemSize)
: FileAccess(NULL, true)
, m_pData( NULL )
, mDataSize( allocMemSize )
, mDataUsed( 0 )
, mDataPos( 0 )
{
	if ( allocMemSize > 0 ) {
		m_pData = new unsigned char[allocMemSize];
	}
}

//-----------------------------------------------------------------------------
PGChunk::~PGChunk()
{
	if ( m_pData ) {
		delete [] m_pData;
	}
}