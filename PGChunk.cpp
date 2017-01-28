/*
 *  PGChunk.cpp
 *  C700
 *
 *  Created by osoumen on 12/10/17.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include <string.h>
#include "PGChunk.h"

//-----------------------------------------------------------------------------
PGChunk::PGChunk(int allocMemSize)
: ChunkReader(allocMemSize)
, mNumPrograms( 0 )
{
}

//-----------------------------------------------------------------------------
PGChunk::PGChunk( const void *data, int dataSize )
: ChunkReader( data, dataSize )
, mNumPrograms( 0 )
{
}

//-----------------------------------------------------------------------------
PGChunk::~PGChunk()
{
}

//-----------------------------------------------------------------------------
bool PGChunk::AppendDataFromVP( const InstParams *vp )
{
	if ( mReadOnly ) {
		return false;
	}
	
	int		intValue;
	double	doubleValue;
	
	//プログラム名
	if (vp->pgname[0] != 0) {
		addChunk(kAudioUnitCustomProperty_ProgramName, vp->pgname, PROGRAMNAME_MAX_LEN);
	}

	int	brrSize = vp->brrSize();
	addChunk(kAudioUnitCustomProperty_BRRData, vp->brrData(), brrSize);
	doubleValue = vp->rate;
	addChunk(kAudioUnitCustomProperty_Rate, &doubleValue, sizeof(double));
	intValue = vp->basekey;
	addChunk(kAudioUnitCustomProperty_BaseKey, &intValue, sizeof(int));
	intValue = vp->lowkey;
	addChunk(kAudioUnitCustomProperty_LowKey, &intValue, sizeof(int));
	intValue = vp->highkey;
	addChunk(kAudioUnitCustomProperty_HighKey, &intValue, sizeof(int));
	intValue = vp->lp;
	addChunk(kAudioUnitCustomProperty_LoopPoint, &intValue, sizeof(int));

	intValue = vp->ar;
	addChunk(kAudioUnitCustomProperty_AR, &intValue, sizeof(int));
	intValue = vp->dr;
	addChunk(kAudioUnitCustomProperty_DR, &intValue, sizeof(int));
	intValue = vp->sl;
	addChunk(kAudioUnitCustomProperty_SL, &intValue, sizeof(int));
	intValue = vp->sr;
	addChunk(kAudioUnitCustomProperty_SR, &intValue, sizeof(int));

	intValue = vp->volL;
	addChunk(kAudioUnitCustomProperty_VolL, &intValue, sizeof(int));
	intValue = vp->volR;
	addChunk(kAudioUnitCustomProperty_VolR, &intValue, sizeof(int));

	intValue = vp->echo ? 1:0;
	addChunk(kAudioUnitCustomProperty_Echo, &intValue, sizeof(int));
	intValue = vp->bank;
	addChunk(kAudioUnitCustomProperty_Bank, &intValue, sizeof(int));
	
    intValue = vp->sustainMode ? 1:0;
    addChunk(kAudioUnitCustomProperty_SustainMode, &intValue, sizeof(int));
	
    intValue = vp->monoMode ? 1:0;
    addChunk(kAudioUnitCustomProperty_MonoMode, &intValue, sizeof(int));

    intValue = vp->portamentoOn ? 1:0;
    addChunk(kAudioUnitCustomProperty_PortamentoOn, &intValue, sizeof(int));

    intValue = vp->portamentoRate;
    addChunk(kAudioUnitCustomProperty_PortamentoRate, &intValue, sizeof(int));

    intValue = vp->noteOnPriority;
    addChunk(kAudioUnitCustomProperty_NoteOnPriority, &intValue, sizeof(int));
    
    intValue = vp->releasePriority;
    addChunk(kAudioUnitCustomProperty_ReleasePriority, &intValue, sizeof(int));

	//元波形情報
	intValue = vp->isEmphasized ? 1:0;
	addChunk(kAudioUnitCustomProperty_IsEmaphasized, &intValue, sizeof(int));
	if ( vp->sourceFile[0] ) {
		addChunk(kAudioUnitCustomProperty_SourceFileRef, vp->sourceFile, PATH_LEN_MAX);
	}
    
	mNumPrograms++;
	
	return true;
}

//-----------------------------------------------------------------------------
int PGChunk::getPGChunkSize( const InstParams *vp )
{
	int cksize = 0;
	if ( vp->hasBrrData() ) {
		cksize += sizeof( MyChunkHead ) * 23;
		cksize += sizeof( int ) * 19;	//int型データ×14
		cksize += sizeof(double);		//double型データ１つ
		cksize += PROGRAMNAME_MAX_LEN;
		cksize += PATH_LEN_MAX;
		cksize += vp->brrSize();
	}
	return cksize;
}

//-----------------------------------------------------------------------------
bool PGChunk::ReadDataToVP( InstParams *vp )
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
				vp->releaseBrr();
                BRRData brr;
                brr.data = new unsigned char[ckSize];
				long	actSize;
				readData(brr.data, ckSize, &actSize);
				brr.size = actSize;
				vp->loop = brr.data[actSize-9]&2?true:false;
				vp->setBRRData(&brr);
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

            case kAudioUnitCustomProperty_SustainMode:
			{
				int value;
				readData(&value, ckSize, NULL);
				vp->sustainMode = value ? true:false;
				break;
			}
            
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
				
            case kAudioUnitCustomProperty_MonoMode:
			{
				int value;
				readData(&value, ckSize, NULL);
				vp->monoMode = value ? true:false;
				break;
			}
            
            case kAudioUnitCustomProperty_PortamentoOn:
			{
				int value;
				readData(&value, ckSize, NULL);
				vp->portamentoOn = value ? true:false;
				break;
			}
                
            case kAudioUnitCustomProperty_PortamentoRate:
				readData(&vp->portamentoRate, ckSize, NULL);
				break;
                
            case kAudioUnitCustomProperty_NoteOnPriority:
				readData(&vp->noteOnPriority, ckSize, NULL);
				break;
                
            case kAudioUnitCustomProperty_ReleasePriority:
				readData(&vp->releasePriority, ckSize, NULL);
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
				AdvDataPos(ckSize);
				break;
		}
	}
	return true;
}
