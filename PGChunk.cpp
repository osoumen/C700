/*
 *  PGChunk.cpp
 *  Chip700
 *
 *  Created by osoumen on 12/10/17.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include <string.h>
#include "PGChunk.h"

//-----------------------------------------------------------------------------
PGChunk::PGChunk(int allocMemSize)
: FileAccess(NULL, true)
, mIsBufferInternal( true )
, m_pData( NULL )
, mDataSize( allocMemSize )
, mDataUsed( 0 )
, mDataPos( 0 )
, mNumPrograms( 0 )
, mReadOnly( false )
{
	if ( allocMemSize > 0 ) {
		m_pData = new unsigned char[allocMemSize];
	}
}

//-----------------------------------------------------------------------------
PGChunk::PGChunk( const void *data, int dataSize )
: FileAccess(NULL, true)
, mIsBufferInternal( false )
, m_pData( (unsigned char*)data )
, mDataSize( dataSize )
, mDataUsed( dataSize )
, mDataPos( 0 )
, mNumPrograms( 0 )
, mReadOnly( true )
{
}

//-----------------------------------------------------------------------------
PGChunk::~PGChunk()
{
	if ( (m_pData != NULL) && mIsBufferInternal ) {
		delete [] m_pData;
	}
}

//-----------------------------------------------------------------------------
bool PGChunk::AppendDataFromVP( VoiceParams *vp )
{
	if ( mReadOnly ) {
		return false;
	}
	
	int		intValue;
	double	doubleValue;
	
	//プログラム名
	if (vp->pgname[0] != 0) {
		writeChunk(kAudioUnitCustomProperty_ProgramName, vp->pgname, PROGRAMNAME_MAX_LEN);
	}

	int	brrSize = vp->brr.size;
	//最終ブロックをループフラグにする
	if (vp->loop) {
		vp->brr.data[brrSize - 9] |= 2;
	}
	else {
		vp->brr.data[brrSize - 9] &= ~2;
	}
	writeChunk(kAudioUnitCustomProperty_BRRData, vp->brr.data, brrSize);
	doubleValue = vp->rate;
	writeChunk(kAudioUnitCustomProperty_Rate, &doubleValue, sizeof(double));
	intValue = vp->basekey;
	writeChunk(kAudioUnitCustomProperty_BaseKey, &intValue, sizeof(int));
	intValue = vp->lowkey;
	writeChunk(kAudioUnitCustomProperty_LowKey, &intValue, sizeof(int));
	intValue = vp->highkey;
	writeChunk(kAudioUnitCustomProperty_HighKey, &intValue, sizeof(int));
	intValue = vp->lp;
	writeChunk(kAudioUnitCustomProperty_LoopPoint, &intValue, sizeof(int));

	intValue = vp->ar;
	writeChunk(kAudioUnitCustomProperty_AR, &intValue, sizeof(int));
	intValue = vp->dr;
	writeChunk(kAudioUnitCustomProperty_DR, &intValue, sizeof(int));
	intValue = vp->sl;
	writeChunk(kAudioUnitCustomProperty_SL, &intValue, sizeof(int));
	intValue = vp->sr;
	writeChunk(kAudioUnitCustomProperty_SR, &intValue, sizeof(int));

	intValue = vp->volL;
	writeChunk(kAudioUnitCustomProperty_VolL, &intValue, sizeof(int));
	intValue = vp->volR;
	writeChunk(kAudioUnitCustomProperty_VolR, &intValue, sizeof(int));

	intValue = vp->echo ? 1:0;
	writeChunk(kAudioUnitCustomProperty_Echo, &intValue, sizeof(int));
	intValue = vp->bank;
	writeChunk(kAudioUnitCustomProperty_Bank, &intValue, sizeof(int));
	
	//元波形情報
	intValue = vp->isEmphasized ? 1:0;
	writeChunk(kAudioUnitCustomProperty_IsEmaphasized, &intValue, sizeof(int));
	if ( vp->sourceFile[0] ) {
		writeChunk(kAudioUnitCustomProperty_SourceFileRef, vp->sourceFile, PATH_LEN_MAX);
	}
	
	mNumPrograms++;
	
	return true;
}

//-----------------------------------------------------------------------------
int PGChunk::getPGChunkSize( const VoiceParams *vp )
{
	int cksize = 0;
	if ( vp->brr.data ) {
		cksize += sizeof( MyChunkHead ) * 17;
		cksize += sizeof( int ) * 13;	//int型データ×13
		cksize += sizeof(double);		//double型データ１つ
		cksize += PROGRAMNAME_MAX_LEN;
		cksize += PATH_LEN_MAX;
		cksize += vp->brr.size;
	}
	return cksize;
}

//-----------------------------------------------------------------------------
bool PGChunk::ReadDataToVP( VoiceParams *vp )
{
	while ( (mDataSize - mDataPos) > (int)sizeof( MyChunkHead ) ) {
		int		ckType;
		long	ckSize;
		readChunkHead(&ckType, &ckSize);
		switch (ckType) {
			case kAudioUnitCustomProperty_ProgramName:
				readData(vp->pgname, ckSize, &ckSize);
				break;
			case kAudioUnitCustomProperty_BRRData:
			{
				if ( vp->brr.data ) {
					delete [] vp->brr.data;
				}
				vp->brr.data = new unsigned char[ckSize];
				long	actSize;
				readData(vp->brr.data, ckSize, &actSize);
				vp->brr.size = actSize;
				break;
			}
			case kAudioUnitCustomProperty_Rate:
				readData(&vp->rate, ckSize, NULL);
				break;
			case kAudioUnitCustomProperty_BaseKey:
				readData(&vp->basekey, ckSize, NULL);
				break;
			case kAudioUnitCustomProperty_LowKey:
				readData(&vp->lowkey, ckSize, NULL);
				break;
			case kAudioUnitCustomProperty_HighKey:
				readData(&vp->highkey, ckSize, NULL);
				break;
			case kAudioUnitCustomProperty_LoopPoint:
				readData(&vp->lp, ckSize, NULL);
				break;

			case kAudioUnitCustomProperty_AR:
				readData(&vp->ar, ckSize, NULL);
				break;
			case kAudioUnitCustomProperty_DR:
				readData(&vp->dr, ckSize, NULL);
				break;
			case kAudioUnitCustomProperty_SL:
				readData(&vp->sl, ckSize, NULL);
				break;
			case kAudioUnitCustomProperty_SR:
				readData(&vp->sr, ckSize, NULL);
				break;

			case kAudioUnitCustomProperty_VolL:
				readData(&vp->volL, ckSize, NULL);
				break;
			case kAudioUnitCustomProperty_VolR:
				readData(&vp->volR, ckSize, NULL);
				break;
				
			case kAudioUnitCustomProperty_Echo:
			{
				int value;
				readData(&value, ckSize, NULL);
				vp->echo = value ? true:false;
				break;
			}
			case kAudioUnitCustomProperty_Bank:
				readData(&vp->bank, ckSize, NULL);
				break;
				
			case kAudioUnitCustomProperty_IsEmaphasized:
			{
				int value;
				readData(&value, ckSize, NULL);
				vp->isEmphasized = value ? true:false;
				break;
			}
			case kAudioUnitCustomProperty_SourceFileRef:
				readData(vp->sourceFile, ckSize, &ckSize);
				break;
			default:
				//不明チャンクの場合は飛ばす
				mDataPos += ckSize;
				break;
		}
	}
	return true;
}

//-----------------------------------------------------------------------------
bool PGChunk::writeChunk( int type, const void *data, int byte )
{
	if ( mReadOnly ) {
		return false;
	}
	
	MyChunkHead	ckHead = {type, byte};
	
	//空き容量チェック
	if ( mDataSize < ( mDataPos + byte + (int)sizeof(MyChunkHead) ) ) {
		return false;
	}
	
	memcpy(m_pData+mDataPos, &ckHead, sizeof(MyChunkHead));
	mDataPos += sizeof(MyChunkHead);
	
	memcpy(m_pData+mDataPos, data, byte);
	mDataPos += byte;
	
	if ( mDataPos > mDataUsed ) {
		mDataUsed = mDataPos;
	}
	return true;
}

//-----------------------------------------------------------------------------
bool PGChunk::readChunkHead( int *type, long *byte )
{
	int		toRead = sizeof( MyChunkHead );
	
	MyChunkHead	head;
	memcpy(&head, m_pData+mDataPos, toRead);
	mDataPos += toRead;
	*type = head.type;
	*byte = head.size;
	return true;
}

//-----------------------------------------------------------------------------
bool PGChunk::readData( void *data, long byte, long *actualReadByte )
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
