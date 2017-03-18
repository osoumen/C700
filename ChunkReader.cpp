//
//  ChunkReader.cpp
//  C700
//
//  Created by osoumen on 2017/01/24.
//
//

#include "ChunkReader.h"

ChunkReader::ChunkReader(int allocMemSize)
: FileAccess(NULL, true)
, DataBuffer(allocMemSize)
{
}

//-----------------------------------------------------------------------------
ChunkReader::ChunkReader( const void *data, int dataSize, bool copy )
: FileAccess(NULL, true)
, DataBuffer( data, dataSize, copy )
{
}

//-----------------------------------------------------------------------------
ChunkReader::ChunkReader( const char *path )
: FileAccess(path, false)
, DataBuffer(path)
{
}

//-----------------------------------------------------------------------------
ChunkReader::~ChunkReader()
{
}

//-----------------------------------------------------------------------------
bool ChunkReader::addChunk( int type, const void *data, int byte )
{
	if ( mReadOnly ) {
		return false;
	}
	
	MyChunkHead	ckHead = {type, byte};
	
    if (!mAllowExtend) {
        //空き容量チェック
        if ( mDataSize < ( mDataPos + byte + (int)sizeof(MyChunkHead) ) ) {
            return false;
        }
    }
	
	long	writeSize;
	if ( writeData(&ckHead, sizeof(MyChunkHead), &writeSize) == false ) {
		return false;
	}
	
	if ( writeData(data, byte, &writeSize) == false ) {
		return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
bool ChunkReader::addChunkWithHeader(int type, const unsigned char *data, int byte, unsigned char *header, int headerByte)
{
    if ( mReadOnly ) {
		return false;
	}
	
	MyChunkHead	ckHead = {type, byte+headerByte};
	
    if (!mAllowExtend) {
        //空き容量チェック
        if ( mDataSize < ( mDataPos + byte + (int)sizeof(MyChunkHead) ) ) {
            return false;
        }
    }
	
	long	writeSize;
	if ( writeData(&ckHead, sizeof(MyChunkHead), &writeSize) == false ) {
		return false;
	}
	
    if ( writeData(header, headerByte, &writeSize) == false ) {
		return false;
	}
    
	if ( writeData(data, byte, &writeSize) == false ) {
		return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
bool ChunkReader::readChunkHead( int *type, long *byte )
{
	long		toRead = sizeof( MyChunkHead );
	
	MyChunkHead	head;
	if ( readData(&head, toRead, &toRead) == false ) {
		return false;
	}
	(*type) = head.type;
	(*byte) = head.size;
	return true;
}

//-----------------------------------------------------------------------------
bool ChunkReader::Write()
{
    // DataBuffer にあるデータをFileAccessに保持してあるファイルパスに書き込む
    return WriteToFile(mPath);
}
