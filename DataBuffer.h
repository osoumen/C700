/*
 *  DataBuffer.h
 *  C700
 *
 *  Created by osoumen on 12/10/10.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#if _WIN32
#include <windows.h>
#endif

class DataBuffer {
public:
    typedef struct {
        int pos;
        int used;
    } DataBufferState;
    
	DataBuffer( int allocMemSize );
	DataBuffer( const void *data, int dataSize );   // コピーされない、解放されない
	virtual ~DataBuffer();
	
    void                Clear();
	const unsigned char	*GetDataPtr() const { return m_pData; }
	int					GetDataUsed() const { return mDataUsed; }
    int					GetDataSize() const { return mDataSize; }
    int					GetLeftSize() const { return (mDataSize - mDataPos - 1); }
	int					GetDataPos() const { return mDataPos; }
	void				AdvDataPos( int adv ) { mDataPos+=adv; }
	bool				setPos( int pos );
    int                 getPos() { return mDataPos; }
    bool                isReadOnly() { return mReadOnly; }

	bool				readData( void *data, long byte, long *actualReadByte=NULL );
	bool				writeData( const void *data, long byte, long *actualWriteByte=NULL );
    bool                writeByte( unsigned char byte );
    bool                writeByte( unsigned char byte, int len );
    bool                writeU16( unsigned short word );
    bool                writeU24( int value );
    bool                writeS32( int value );
    DataBufferState     SaveState();
    void                RestoreState(const DataBufferState &state);
	
    bool                WriteToFile(const char *path);
    
protected:
	bool			mIsBufferInternal;
    
	unsigned char	*m_pData;
	int				mDataSize;
	int				mDataUsed;
	int				mDataPos;
    
	bool			mReadOnly;
};
