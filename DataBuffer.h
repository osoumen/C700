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
	DataBuffer( const void *data, int dataSize );
	virtual ~DataBuffer();
	
    void                Clear();
	const unsigned char	*GetDataPtr() const { return m_pData; }
	int					GetDataSize() const { return mDataUsed; }
    int					GetMaxDataSize() const { return mDataSize; }
    int					GetWritableSize() const { return (mDataSize - mDataPos - 1); }
	int					GetDataPos() const { return mDataPos; }
	void				AdvDataPos( int adv ) { mDataPos+=adv; }
	bool				setPos( int pos );

	bool				readData( void *data, long byte, long *actualReadByte );
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
