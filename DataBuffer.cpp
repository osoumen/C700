/*
 *  DataBuffer.cpp
 *  C700
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
        memset(m_pData, 0, allocMemSize);
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
void DataBuffer::Clear()
{
    mDataUsed = 0;
	mDataPos = 0;
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

//-----------------------------------------------------------------------------
bool DataBuffer::writeByte( unsigned char byte )
{
	if ( ( mDataPos + 1 ) > mDataSize ) {
		return false;
	}
	m_pData[mDataPos] = byte;
	mDataPos++;
	if ( mDataPos > mDataUsed ) {
		mDataUsed = mDataPos;
	}
	return true;
}

//-----------------------------------------------------------------------------
bool DataBuffer::writeByte( unsigned char byte, int len )
{
    for (int i=0; i<len; i++) {
        writeByte(byte);
    }
    return true;
}

//-----------------------------------------------------------------------------
DataBuffer::DataBufferState DataBuffer::SaveState()
{
    DataBufferState state;
    state.pos = mDataPos;
    state.used = mDataUsed;
    return state;
}

//-----------------------------------------------------------------------------
void DataBuffer::RestoreState(const DataBufferState &state)
{
    mDataPos = state.pos;
    mDataUsed = state.used;
}

//-----------------------------------------------------------------------------
bool DataBuffer::WriteToFile(const char *path)
{
    // Buffer にあるデータを指定したファイルパスに書き込む
    
#if MAC
    CFURLRef	savefile = CFURLCreateFromFileSystemRepresentation(NULL, (UInt8*)path, strlen(path), false);
    
    CFWriteStreamRef	filestream = CFWriteStreamCreateWithFile(NULL,savefile);
    if (CFWriteStreamOpen(filestream)) {
        CFWriteStreamWrite(filestream, GetDataPtr(), GetDataSize() );
        CFWriteStreamClose(filestream);
    }
    CFRelease(filestream);
    CFRelease(savefile);
#else
    HANDLE	hFile;
	
	hFile = CreateFile( path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile != INVALID_HANDLE_VALUE ) {
		DWORD	writeSize;
		WriteFile( hFile, GetDataPtr(), GetDataPos(), &writeSize, NULL );
		CloseHandle( hFile );
	}
    delete [] optimizedData;
#endif
    return true;
}
