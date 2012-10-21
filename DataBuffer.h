/*
 *  DataBuffer.h
 *  Chip700
 *
 *  Created by osoumen on 12/10/10.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "Chip700defines.h"

#if _WIN32
#include <windows.h>
#endif

class DataBuffer {
public:
	DataBuffer( int allocMemSize );
	DataBuffer( const void *data, int dataSize );
	virtual ~DataBuffer();
	
	const unsigned char	*GetDataPtr() const { return m_pData; }
	int					GetDataSize() const { return mDataUsed; }
	int					GetDataPos() const { return mDataPos; }
	void				AdvDataPos( int adv ) { mDataPos+=adv; }
	bool				setPos( int pos );

	bool				readData( void *data, long byte, long *actualReadByte );
	bool				writeData( const void *data, long byte, long *actualWriteByte );
	
protected:
	bool			mIsBufferInternal;
	unsigned char	*m_pData;
	int				mDataSize;
	int				mDataUsed;
	int				mDataPos;
	bool			mReadOnly;
};
