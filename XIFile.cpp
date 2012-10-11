/*
 *  XIFile.cpp
 *  Chip700
 *
 *  Created by 藤田 匡彦 on 12/10/10.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "Chip700defines.h"
#include "XIFile.h"

//-----------------------------------------------------------------------------
XIFile::XIFile( const char *path, bool isWriteable )
: FileAccess(path, isWriteable)
{
}

//-----------------------------------------------------------------------------
XIFile::~XIFile()
{
#if AU
	if ( mIsLoaded ) {
		CFRelease(mCFData);
	}
#endif
}

//-----------------------------------------------------------------------------
bool XIFile::Write()
{
#if AU
	if ( IsLoaded() == false ) return false;
	if ( mPath == NULL ) return false;
	
	//CFURLを作成
	CFURLRef	savefile = CFURLCreateFromFileSystemRepresentation(NULL, (UInt8*)mPath, strlen(mPath), false);
	
	//バイナリ形式に変換
	CFWriteStreamRef	filestream = CFWriteStreamCreateWithFile(NULL,savefile);
	if (CFWriteStreamOpen(filestream)) {
		CFWriteStreamWrite(filestream,CFDataGetBytePtr(mCFData),CFDataGetLength(mCFData));
		CFWriteStreamClose(filestream);
	}
	CFRelease(filestream);
	CFRelease(savefile);
	
	return true;
#else
	return false;
#endif
}

//-----------------------------------------------------------------------------
#if AU
void XIFile::SetCFData( CFDataRef data )
{
	if ( mIsLoaded ) {
		CFRelease(mCFData);
	}
	mCFData = data;
	CFRetain(mCFData);
	mIsLoaded = true;
}

//-----------------------------------------------------------------------------
CFDataRef XIFile::GetCFData() const
{
	if ( IsLoaded() == false ) return NULL;
	
	return mCFData;
}
#endif