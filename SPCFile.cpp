/*
 *  SPCFile.cpp
 *  C700
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
    mSpcPlay.init();
}

//-----------------------------------------------------------------------------
SPCFile::~SPCFile()
{
}

//-----------------------------------------------------------------------------
bool SPCFile::Load()
{
    unsigned char fileData[SPC_READ_SIZE];
    
#if MAC
	CFURLRef	url = CFURLCreateFromFileSystemRepresentation(NULL, (UInt8*)mPath, strlen(mPath), false);
	
	CFReadStreamRef	filestream = CFReadStreamCreateWithFile(NULL, url);
	if (CFReadStreamOpen(filestream) == false) {
        CFRelease( filestream );
		CFRelease( url );
		return false;
	}
	
	CFIndex	readbytes=CFReadStreamRead(filestream, fileData, SPC_READ_SIZE);
	if (readbytes < SPC_READ_SIZE) {
		CFRelease( url );
		CFReadStreamClose(filestream);
        CFRelease( filestream );
		return false;
	}
	CFReadStreamClose(filestream);
    CFRelease( filestream );
	CFRelease( url );
#else
	//WindowsVST‚Ì‚Æ‚«‚ÌSPCƒtƒ@ƒCƒ‹“Ç‚Ýž‚Ýˆ—
	HANDLE	hFile;
	
	hFile = CreateFile( mPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile != INVALID_HANDLE_VALUE ) {
		DWORD	readSize;
		ReadFile( hFile, fileData, SPC_READ_SIZE, &readSize, NULL );
		CloseHandle( hFile );
	}
#endif
	
    blargg_err_t err;
    err = mSpcPlay.load_spc(fileData, SPC_READ_SIZE);
    if (err) {
        return false;
    }
    err = mSpcPlay.play(32000*5, NULL);   //‹ó“®ì‚³‚¹‚é
    unsigned char *ramData = mSpcPlay.GetRam();
	
	mSrcTableAddr = (int)mSpcPlay.GetDspReg(0x5d) << 8;
	for (int i=0; i<128; i++ ) {
		int	startaddr;
		int	loopaddr;
		startaddr	= (int)ramData[mSrcTableAddr + i*4];
		startaddr	+= (int)ramData[mSrcTableAddr + i*4 + 1] << 8;
		loopaddr	= (int)ramData[mSrcTableAddr + i*4 + 2];
		loopaddr	+= (int)ramData[mSrcTableAddr + i*4 + 3] << 8;
		mSampleStart[i]	= startaddr;
		mLoopSize[i]	= loopaddr-startaddr;
		mIsLoop[i]		= checkbrrsize(&ramData[startaddr], &mSampleBytes[i]) == 1?true:false;
		
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
        unsigned char *ramData = mSpcPlay.GetRam();
		if ( size ) {
			*size = mSampleBytes[sampleIndex];
		}
		if ( mSampleBytes[sampleIndex] == 0 ) {
			return NULL;
		}
		return &ramData[mSampleStart[sampleIndex]];
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

