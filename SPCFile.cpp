/*
 *  SPCFile.cpp
 *  Chip700
 *
 *  Created by osoumen on 12/10/10.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "SPCFile.h"
#include "brrcodec.h"
#include <string.h>

//-----------------------------------------------------------------------------
SPCFile::SPCFile( const char *path, bool isWriteable )
: FileAccess(path, isWriteable)
{
	m_pFileData = new unsigned char[SPC_READ_SIZE];
	m_pRamData = m_pFileData + 0x100;
}

//-----------------------------------------------------------------------------
SPCFile::~SPCFile()
{
	delete [] m_pFileData;
}

//-----------------------------------------------------------------------------
bool SPCFile::Load()
{
#if MAC
	CFURLRef	url = CFURLCreateFromFileSystemRepresentation(NULL, (UInt8*)mPath, strlen(mPath), false);
	
	CFReadStreamRef	filestream = CFReadStreamCreateWithFile(NULL, url);
	if (CFReadStreamOpen(filestream) == false) {
		CFRelease( url );
		return false;
	}
	
	CFIndex	readbytes=CFReadStreamRead(filestream, m_pFileData, SPC_READ_SIZE);
	if (readbytes < SPC_READ_SIZE) {
		CFRelease( url );
		CFReadStreamClose(filestream);
		return false;
	}
	CFReadStreamClose(filestream);
	CFRelease( url );
#else
	//WindowsVSTのときのSPCファイル読み込み処理
	HANDLE	hFile;
	
	hFile = CreateFile( mPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile != INVALID_HANDLE_VALUE ) {
		DWORD	readSize;
		ReadFile( hFile, m_pFileData, SPC_READ_SIZE, &readSize, NULL );
		CloseHandle( hFile );
	}
#endif
	
	//ファイルチェック
//	if ( strncmp((char*)m_pFileData, "SNES-SPC700 Sound File Data v0.30", 33) != 0 ) {
	if ( strncmp((char*)m_pFileData, "SNES-SPC700 Sound File Data v", 29) != 0 ) {
		return false;
	}
	
	mSrcTableAddr = (int)m_pRamData[0x1005d] << 8;
	for (int i=0; i<128; i++ ) {
		int	startaddr;
		int	loopaddr;
		startaddr	= (int)m_pRamData[mSrcTableAddr + i*4];
		startaddr	+= (int)m_pRamData[mSrcTableAddr + i*4 + 1] << 8;
		loopaddr	= (int)m_pRamData[mSrcTableAddr + i*4 + 2];
		loopaddr	+= (int)m_pRamData[mSrcTableAddr + i*4 + 3] << 8;
		mSampleStart[i]	= startaddr;
		mLoopSize[i]	= loopaddr-startaddr;
		mIsLoop[i]		= checkbrrsize(&m_pRamData[startaddr], &mSampleBytes[i]) == 1?true:false;
		
		if ( startaddr == 0 || startaddr == 0xffff ||
			mLoopSize[i] < 0 || mSampleBytes[i] < mLoopSize[i] || (mLoopSize[i]%9) != 0 ) {
			mSampleBytes[i] = 0;
		}
	}
	
	mIsLoaded = true;

	return true;
}

//-----------------------------------------------------------------------------
unsigned char *SPCFile::GetSampleIndex( int sampleIndex, int *size )
{
	if ( mIsLoaded ) {
		if ( size ) {
			*size = mSampleBytes[sampleIndex];
		}
		if ( mSampleBytes[sampleIndex] == 0 ) {
			return NULL;
		}
		return &m_pRamData[mSampleStart[sampleIndex]];
	}
	return NULL;
}

//-----------------------------------------------------------------------------
int SPCFile::GetLoopSizeIndex( int samleIndex )
{
	if ( mIsLoaded ) {
		if ( mIsLoop[samleIndex] ) {
			return mLoopSize[samleIndex];
		}
		else {
			return -1;
		}
	}
	return 0;
}

