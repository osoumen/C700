/*
 *  DataBuffer.cpp
 *  Chip700
 *
 *  Created by osoumen on 12/10/10.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "DataBuffer.h"
#include <string.h>

//-----------------------------------------------------------------------------
DataBuffer::DataBuffer(int allocMemSize)
: mIsBufferInternal( true )
, m_pData( NULL )
, mDataSize( allocMemSize )
, mDataUsed( 0 )
, mDataPos( 0 )
, mReadOnly( false )
{
	if ( allocMemSize > 0 ) {
		m_pData = new unsigned char[allocMemSize];
	}
}

//-----------------------------------------------------------------------------
DataBuffer::DataBuffer( const void *data, int dataSize )
: mIsBufferInternal( false )
, m_pData( (unsigned char*)data )
, mDataSize( dataSize )
, mDataUsed( dataSize )
, mDataPos( 0 )
, mReadOnly( true )
{
}

//-----------------------------------------------------------------------------
DataBuffer::~DataBuffer()
{
	if ( (m_pData != NULL) && mIsBufferInternal ) {
		delete [] m_pData;
	}
}

//-----------------------------------------------------------------------------
bool DataBuffer::setPos( int pos )
{
	if ( mDataSize < pos ) {
		return false;
	}
	mDataPos = pos;
	if ( mDataPos > mDataUsed ) {
		mDataUsed = mDataPos;
	}
	return true;
}

//-----------------------------------------------------------------------------
bool DataBuffer::readData( void *data, long byte, long *actualReadByte )
{
	int		toRead = byte;
	
	if ( mDataPos >= mDataSize ) {
		return false;
	}
	if ( mDataSize < ( mDataPos + byte ) ) {
		toRead = mDataSize - mDataPos;
	}
	
	memcpy(data, m_pData+mDataPos, toRead);
	mDataPos += toRead;
	
	if ( actualReadByte ) {
		*actualReadByte = toRead;
	}
	return true;
}

//-----------------------------------------------------------------------------
bool DataBuffer::writeData( const void *data, long byte, long *actualWriteByte )
{
	int		toWrite = byte;
	
	if ( mDataPos >= mDataSize ) {
		return false;
	}
	if ( mDataSize < ( mDataPos + byte ) ) {
		toWrite = mDataSize - mDataPos;
	}
	
	memcpy(m_pData+mDataPos, data, toWrite);
	mDataPos += toWrite;
	
	if ( mDataPos > mDataUsed ) {
		mDataUsed = mDataPos;
	}
	
	if ( actualWriteByte ) {
		*actualWriteByte = toWrite;
	}
	return true;
}