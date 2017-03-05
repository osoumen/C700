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
ChunkReader::ChunkReader( const void *data, int dataSize )
: FileAccess(NULL, true)
, DataBuffer( data, dataSize )
{
}

//-----------------------------------------------------------------------------
ChunkReader::ChunkReader( const char *path )
: FileAccess(path, false)
, DataBuffer(1)
{
    SetAllowExtend(true);
#if MAC
	CFURLRef	url = CFURLCreateFromFileSystemRepresentation(NULL, (UInt8*)path, strlen(path), false);
	
	CFReadStreamRef	filestream = CFReadStreamCreateWithFile(NULL, url);
	if (CFReadStreamOpen(filestream) == false) {
        CFRelease( filestream );
		CFRelease( url );
		return;
	}
    
    UInt8   readBuf[65536];
    CFIndex	readbytes = 0;
    do {
        readbytes = CFReadStreamRead(filestream, (UInt8*)readBuf, 65536);
        writeData(readBuf, readbytes);
    } while (readbytes > 0);
    
	CFReadStreamClose(filestream);
    CFRelease( filestream );
	CFRelease( url );
#else
	// Windows環境のファイル読み込み処理
	HANDLE	hFile;
	
    unsigned char   readBuf[65536];
	hFile = CreateFile( path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile != INVALID_HANDLE_VALUE ) {
		DWORD	readbytes = 0;
        do {
            ReadFile( hFile, readBuf, 65536, &readbytes, NULL );
            writeData(readBuf, readbytes);
        } while (readbytes > 0);
		CloseHandle( hFile );
	}
#endif
    setPos(0);
    SetAllowExtend(false);
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
    return WriteToFile(GetFilePath());
}
