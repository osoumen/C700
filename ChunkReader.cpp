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
	
	//空き容量チェック
	if ( mDataSize < ( mDataPos + byte + (int)sizeof(MyChunkHead) ) ) {
		return false;
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
	
	//空き容量チェック
	if ( mDataSize < ( mDataPos + byte + (int)sizeof(MyChunkHead) ) ) {
		return false;
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
    const char *path = GetFilePath();
    
#if MAC
    CFURLRef	savefile = CFURLCreateFromFileSystemRepresentation(NULL, (UInt8*)path, strlen(path), false);
    
    CFWriteStreamRef	filestream = CFWriteStreamCreateWithFile(NULL,savefile);
    if (CFWriteStreamOpen(filestream)) {
        CFWriteStreamWrite(filestream, GetDataPtr(), GetDataPos() );
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
