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
, mAllowExtend( false )
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
, mAllowExtend( false )
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
	
    if ( mDataSize < ( mDataPos + byte ) ) {
        if (mAllowExtend) {
            int requiedBytes = ( mDataPos + byte ) - mDataSize;
            int quarterBytes = mDataSize / 4;
            extendDataSize(requiedBytes>quarterBytes?requiedBytes:quarterBytes);
        }
    }
    
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
    if ( mDataSize < ( mDataPos + 1 ) ) {
        if (mAllowExtend) {
            int requiedBytes = ( mDataPos + 1 ) - mDataSize;
            int quarterBytes = mDataSize / 4;
            extendDataSize(requiedBytes>quarterBytes?requiedBytes:quarterBytes);
        }
    }
    
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
bool DataBuffer::writeU16( unsigned short word )
{
    unsigned char data[2];
    data[0] = word & 0xff;
    data[1] = (word >> 8) & 0xff;
    return writeData(data, 2);
}

//-----------------------------------------------------------------------------
bool DataBuffer::writeU24( int value )
{
    unsigned char data[3];
    data[0] = value & 0xff;
    data[1] = (value >> 8) & 0xff;
    data[2] = (value >> 16) & 0xff;
    return writeData(data, 3);
}

//-----------------------------------------------------------------------------
bool DataBuffer::writeS32( int value )
{
    unsigned char data[4];
    data[0] = value & 0xff;
    data[1] = (value >> 8) & 0xff;
    data[2] = (value >> 16) & 0xff;
    data[3] = (value >> 24) & 0xff;
    return writeData(data, 4);
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
        CFWriteStreamWrite(filestream, GetDataPtr(), GetDataUsed() );
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
#endif
    return true;
}

//-----------------------------------------------------------------------------
void DataBuffer::extendDataSize(int extendBytes)
{
    if ( (m_pData != NULL) && mIsBufferInternal ) {
        int newSize = mDataSize + extendBytes;
        unsigned char	*newData = new unsigned char[newSize];
        memcpy(newData, m_pData, mDataSize);
        memset(newData+mDataSize, 0, extendBytes);
        delete [] m_pData;
        m_pData = newData;
        mDataSize = newSize;
    }
}