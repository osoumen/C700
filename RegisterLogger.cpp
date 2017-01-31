/*
 *  RegisterLogger.cpp
 *  VOPM
 *
 *  Created by osoumen on 12/11/04.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "RegisterLogger.h"
#include <string.h>

#if __APPLE_CC__
#include <Carbon/Carbon.h>
#endif

#if _WIN32
#include <windows.h>
#endif

#include <iostream>

//-----------------------------------------------------------------------------
RegisterLogger::RegisterLogger(int allocSize)
: ChunkReader(1024 * 1024 * 32)  // TODO: 適切なサイズを計算する
, mProcessSampleRate( 32000 )
{
	if ( allocSize > 0 ) {
        m_pLogCommands = new LogCommands[allocSize];
	}
    mLogCommandsSize = allocSize;
    mLogCommandsPos = 0;
    mLogCommandsLoopPoint = 0;
    mBeginInitializationPoint = 0;
    mEndInitializationPoint = 0;
    
    mDirRegionData = NULL;
    mBrrRegionData = NULL;
    mDirRegionLocateAddr = 0;
    mDirRegionSize = 0;
    mBrrRegionLocateAddr = 0;
    mBrrRegionSize = 0;
    
	BeginDump(0);
}

//-----------------------------------------------------------------------------
RegisterLogger::~RegisterLogger()
{
    if (m_pLogCommands != NULL) {
        delete [] m_pLogCommands;
    }
    if (mDirRegionData != NULL) {
        delete mDirRegionData;
    }
    if (mBrrRegionData != NULL) {
        delete mBrrRegionData;
    }
}

//-----------------------------------------------------------------------------
void RegisterLogger::addDspRegRegion(const unsigned char *data)
{
    memcpy(mDspRegionData, data, DSP_REGION_LEN);
}

//-----------------------------------------------------------------------------
void RegisterLogger::addDirRegion(int locateAddr, int size, unsigned char *data)
{
    mDirRegionLocateAddr = locateAddr;
    mDirRegionSize = size;
    if (mDirRegionData) {
        delete mDirRegionData;
    }
    mDirRegionData = new unsigned char[size];
    memcpy(mDirRegionData, data, size);
}

//-----------------------------------------------------------------------------
void RegisterLogger::addBrrRegion(int locateAddr, int size, unsigned char *data)
{
    mBrrRegionLocateAddr = locateAddr;
    mBrrRegionSize = size;
    if (mBrrRegionData) {
        delete mBrrRegionData;
    }
    mBrrRegionData = new unsigned char[size];
    memcpy(mBrrRegionData, data, size);
}

//-----------------------------------------------------------------------------
bool RegisterLogger::Write()
{
    // ChunkReaderにデータを詰める
    setPos(0);
    
    addChunk('DSPR', mDspRegionData, DSP_REGION_LEN);
    
    unsigned char header[4];
    header[0] = mDirRegionLocateAddr & 0xff;
    header[1] = (mDirRegionLocateAddr >> 8) & 0xff;
    header[2] = mDirRegionSize & 0xff;
    header[3] = (mDirRegionSize >> 8) & 0xff;
    addChunkWithHeader('DIRR', mDirRegionData, mDirRegionSize, header, 4);
    
    header[0] = mBrrRegionLocateAddr & 0xff;
    header[1] = (mBrrRegionLocateAddr >> 8) & 0xff;
    header[2] = mBrrRegionSize & 0xff;
    header[3] = (mBrrRegionSize >> 8) & 0xff;
    addChunkWithHeader('BRRR', mBrrRegionData, mBrrRegionSize, header, 4);
    
    // 変換前のデータを出力するようにする
    int loopAddr = mLogCommandsLoopPoint;
    header[0] = loopAddr & 0xff;
    header[1] = ((loopAddr >> 8) & 0xff);
    header[2] = (loopAddr >> 16) & 0xff;
    header[3] = (loopAddr >> 24) & 0xff;
    addChunkWithHeader('SEQR', (unsigned char *)m_pLogCommands, mLogCommandsPos, header, 4);
    
	return ChunkReader::Write();
}

//-----------------------------------------------------------------------------
void RegisterLogger::SetProcessSampleRate( int rate )
{
    mProcessSampleRate = rate;
}

//-----------------------------------------------------------------------------
void RegisterLogger::BeginDump( int time )
{
    mLogCommandsPos = 0;
    mLogCommandsLoopPoint = 0;
    mBeginInitializationPoint = 0;
    mEndInitializationPoint = 0;
    for ( int i=0; i<256; i++ ) {
        mReg[i] = -1;
	}
    mIsEnded = false;
}

//-----------------------------------------------------------------------------
void RegisterLogger::BeginDspInitialization()
{
    mBeginInitializationPoint = mLogCommandsPos;
}

//-----------------------------------------------------------------------------
void RegisterLogger::EndDspInitialization()
{
    mEndInitializationPoint = mLogCommandsPos;
}

//-----------------------------------------------------------------------------
bool RegisterLogger::DumpReg( int device, int addr, unsigned char data, int time )
{
    if (addr >= 0 && addr < 128) {
		
		if ( mReg[addr] != data || addr == 0x4c || addr == 0x5c) {
			mReg[addr] = data;
            if ( (mLogCommandsSize - mLogCommandsPos) > 0 ) {
                m_pLogCommands[mLogCommandsPos].data[0] = addr;
                m_pLogCommands[mLogCommandsPos].data[1] = data;
                m_pLogCommands[mLogCommandsPos].time = time;
                mLogCommandsPos++;
			}
			return true;
		}
	}
    return false;
}

//-----------------------------------------------------------------------------
bool RegisterLogger::DumpApuPitch( int device, int addr, unsigned char data_l, unsigned char data_m, int time )
{
    if ( (addr & 0x0f) == 0x02 ) {
        if (data_m == mReg[addr+1]) {
            return DumpReg( device, addr, data_l, time );
        }
        mReg[addr] = data_l;
        mReg[addr+1] = data_m;
        
        if ( (mLogCommandsSize - mLogCommandsPos) > 0 ) {
            m_pLogCommands[mLogCommandsPos].data[0] = addr+1;
            m_pLogCommands[mLogCommandsPos].data[1] = data_m;
            m_pLogCommands[mLogCommandsPos].data[2] = data_l;
            m_pLogCommands[mLogCommandsPos].time = time;
            mLogCommandsPos++;
        }
        return true;
    }
	return false;
}

//-----------------------------------------------------------------------------
void RegisterLogger::MarkLoopPoint()
{
    mLogCommandsLoopPoint = mLogCommandsPos;
    //ループ直後は常にレジスタが書き込まれるようにする
	for ( int i=0; i<256; i++ ) {
		mReg[i] = -1;
	}
}

//-----------------------------------------------------------------------------
void RegisterLogger::EndDump(int time)
{
    if ( (mLogCommandsSize - mLogCommandsPos) > 0 && mIsEnded == false) {
        m_pLogCommands[mLogCommandsPos].data[0] = 0x9e;
        m_pLogCommands[mLogCommandsPos].time = time;
        mLogCommandsPos++;
        mIsEnded = true;
    }
}

