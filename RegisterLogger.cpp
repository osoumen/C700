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

//std::map<int,int> regStat;

static int getCommandLength(unsigned char cmd);

//-----------------------------------------------------------------------------
RegisterLogger::RegisterLogger(int allocSize)
: ChunkReader(1024 * 1024 * 32)  // TODO: 適切なサイズを計算する
, mDataBuffer( allocSize )
, mTickPerSec( 15734 )
, mProcessSampleRate( 32000 )
{
	if ( allocSize > 0 ) {
        m_pLogCommands = new LogCommands[allocSize];
	}
    mLogCommandsSize = allocSize;
    mLogCommandsPos = 0;
    mLogCommandsLoopPoint = 0;
    
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
void RegisterLogger::addWaitTable(const unsigned char *data)
{
    memcpy(mWaitTableData, data, WAIT_TABLE_LEN);
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
    
    addChunk('WTBL', mWaitTableData, WAIT_TABLE_LEN);
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
    header[1] = ((loopAddr >> 8) & 0x7f) + 0x80;
    header[2] = (loopAddr >> 15) & 0xff;
    header[3] = (loopAddr >> 24) & 0xff;
    addChunkWithHeader('SEQR', (unsigned char *)m_pLogCommands, mLogCommandsPos, header, 4);
    
	return ChunkReader::Write();
}
#if 0
//-----------------------------------------------------------------------------
bool RegisterLogger::SaveToFile( const char *path, double tickPerSec )
{
    compileLogData( tickPerSec );
    
    // データの削減
    unsigned char *optimizedData = new unsigned char [mDataBuffer.GetMaxDataSize()];
    int optimizedDataSize;
    int optimizedLoopPoint;
    optimizedDataSize = optimizeWaits(mDataBuffer.GetDataPtr(), optimizedData, mDataBuffer.GetDataSize(), &optimizedLoopPoint);
	
    // ChunkReaderにデータを詰める
    setPos(0);
    
    addChunk('WTBL', mWaitTableData, WAIT_TABLE_LEN);
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
    
    int loopAddr = optimizedLoopPoint + 3;
    unsigned char loopStart[3];
    loopStart[0] = loopAddr & 0xff;
    loopStart[1] = ((loopAddr >> 8) & 0x7f) + 0x80;
    loopStart[2] = (loopAddr >> 15) & 0xff;
    addChunkWithHeader('SEQR', optimizedData, optimizedDataSize, loopStart, 3);
    
    delete [] optimizedData;

    SetFilePath(path);
	return ChunkReader::Write();
}
#endif
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
    for ( int i=0; i<256; i++ ) {
        mReg[i] = -1;
	}
    mIsEnded = false;
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

//-----------------------------------------------------------------------------
void RegisterLogger::compileLogData( double tickPerSec )
{
    mTickPerSec = tickPerSec;
    
    if ( mLogCommandsPos == 0) {
        return;
    }
    
    BeginDump_(0);
    for (int i=0; i<mLogCommandsPos; i++) {
        if (i == mLogCommandsLoopPoint) {
            MarkLoopPoint_();
        }
        unsigned char cmd = m_pLogCommands[i].data[0];
        int cmdLen = getCommandLength(cmd);
        if (cmd < 0x80) {
            if (cmdLen == 2) {
                DumpReg_( 0, cmd, m_pLogCommands[i].data[1], m_pLogCommands[i].time );
            }
            else if (cmdLen == 3) {
                DumpApuPitch_( 0, cmd, m_pLogCommands[i].data[2], m_pLogCommands[i].data[1], m_pLogCommands[i].time );
            }
        }
        else if (cmd == 0x9e) {
            EndDump_(m_pLogCommands[i].time);
            break;
        }
    }
}

//-----------------------------------------------------------------------------
void RegisterLogger::BeginDump_( int time )
{
    int tick = convertTime2Tick(time);
    
	mDumpBeginTime = tick;
	mPrevTime = mDumpBeginTime;
	
	mDataBuffer.Clear();
	mLoopPoint = 0;
    
    //regStat.clear();
    mWaitStat.clear();
	
//	printf("--BeginDump--\n");
}

//-----------------------------------------------------------------------------
bool RegisterLogger::DumpReg_( int device, int addr, unsigned char data, int time )
{
    int tick = convertTime2Tick(time);
    
    if (tick < mDumpBeginTime) {
        return false;
    }
    
    writeWaitFromPrev(tick);
    
    if ( mDataBuffer.GetWritableSize() >= 3 ) {
        writeByte( addr );
        writeByte( data );
        /*
        if (regStat.count(addr) == 0) {
            regStat[addr] = 1;
        }
        else {
            regStat[addr] = regStat[addr]+1;
        }
         */
    }
    
    return true;
}

//-----------------------------------------------------------------------------
bool RegisterLogger::DumpApuPitch_( int device, int addr, unsigned char data_l, unsigned char data_m, int time )
{
    int tick = convertTime2Tick(time);
    
    if (tick < mDumpBeginTime) {
        return false;
    }
    
    if ( (addr & 0x0f) == 0x03 ) {

        writeWaitFromPrev(tick);
        
        if ( mDataBuffer.GetWritableSize() >= 4 ) {
            writeByte( addr );
            writeByte( data_m );
            writeByte( data_l );
            /*
            if (regStat.count(addr) == 0) {
                regStat[addr] = 1;
            }
            else {
                regStat[addr] = regStat[addr]+1;
            }
             */
        }
        
        return true;
    }
	return false;
}

//-----------------------------------------------------------------------------
void RegisterLogger::MarkLoopPoint_()
{
	mLoopPoint = mDataBuffer.GetDataPos();
//	printf("--MarkLoopPoint--\n");
}

//-----------------------------------------------------------------------------
void RegisterLogger::EndDump_(int time)
{
    int tick = convertTime2Tick(time);
    
	if ( mDataBuffer.GetDataSize() > 0 ) {
		writeWaitFromPrev(tick);
		writeEndByte();
		/*
        for (auto it = regStat.begin(); it != regStat.end(); it++) {
            std::cout << "$" << std::hex << it->first << "," << std::dec << it->second << std::endl;
        }
        
        for (auto it = mWaitStat.begin(); it != mWaitStat.end(); it++) {
            std::cout << it->first << "," << it->second << std::endl;
        }*/
//		printf("--EndDump-- %d\n",time);
	}
}

//-----------------------------------------------------------------------------
bool RegisterLogger::writeByte( unsigned char byte )
{
	if ( ( mDataBuffer.GetDataPos() + 1 ) > (mDataBuffer.GetMaxDataSize()-1) ) {	//END/LOOPが書き込める様に１バイト残しておく
		return false;
	}
    return mDataBuffer.writeByte(byte);
}

//-----------------------------------------------------------------------------
bool RegisterLogger::writeEndByte()
{
	return mDataBuffer.writeByte(0x9e);
}

//-----------------------------------------------------------------------------
bool RegisterLogger::writeWaitFromPrev(int tick)
{
	bool		result;
	
	int		now_time	= tick - mDumpBeginTime;
	int		prev_time	= mPrevTime - mDumpBeginTime;
	int		adv_time	= now_time - prev_time;
	
	//先頭に空白があれば飛ばす
	if ( mPrevTime == mDumpBeginTime ) {
		adv_time = 0;
	}
	
    long div = adv_time >> 16;
    long mod = adv_time & 0xffff;
    
    for (int i=0; i<div; i++) {
        DataBuffer::DataBufferState state = mDataBuffer.SaveState();
        
        result = writeByte(0x94);
        if ( result == false ) return false;
        result = writeByte(0xff);
        if ( result == false ) {
            //書き込んだ分を巻き戻す
            mDataBuffer.RestoreState(state);
            return false;
        }
        result = writeByte(0xff);
        if ( result == false ) {
            //書き込んだ分を巻き戻す
            mDataBuffer.RestoreState(state);
            return false;
        }
        addWaitStatistic(0xffff);
    }
    if (mod > 0) {
        if (mod < 0x100) {
            DataBuffer::DataBufferState state = mDataBuffer.SaveState();
            
            result = writeByte(0x92);
            if ( result == false ) return false;
            result = writeByte(mod & 0xff);
            if ( result == false ) {
                //書き込んだ分を巻き戻す
                mDataBuffer.RestoreState(state);
                return false;
            }
            addWaitStatistic(mod & 0xff);
        }
        else {
            if ((mod & ~0x01fe) == 0) {
                result = writeByte(0x96);
                result = writeByte(mod >> 1);
            }
            else if ((mod & ~0x03fc) == 0) {
                result = writeByte(0x98);
                result = writeByte(mod >> 2);
            }
            else if ((mod & ~0x07f8) == 0) {
                result = writeByte(0x9a);
                result = writeByte(mod >> 3);
            }
            else if ((mod & ~0x0ff0) == 0) {
                result = writeByte(0x9c);
                result = writeByte(mod >> 4);
            }
            else {
                DataBuffer::DataBufferState state = mDataBuffer.SaveState();
                
                result = writeByte(0x94);
                if ( result == false ) return false;
                result = writeByte(mod & 0xff);
                if ( result == false ) {
                    //書き込んだ分を巻き戻す
                    mDataBuffer.RestoreState(state);
                    return false;
                }
                result = writeByte(mod >> 8);
                if ( result == false ) {
                    //書き込んだ分を巻き戻す
                    mDataBuffer.RestoreState(state);
                    return false;
                }
            }
            addWaitStatistic(mod);
        }
    }
    
	mPrevTime = tick;
    /*
	if ( adv_time < 0 ) {
		mPrevTime -= adv_time;
	}*/
	return true;
}

//-----------------------------------------------------------------------------
bool RegisterLogger::addWaitStatistic(int tick)
{
    if (mWaitStat.count(tick) == 0) {
        mWaitStat[tick] = 1;
    }
    else {
        mWaitStat[tick] = mWaitStat[tick]+1;
    }
    return true;
}

//-----------------------------------------------------------------------------
int RegisterLogger::optimizeWaits(const unsigned char *inData, unsigned char *outData, int inDataSize, int *outLoopPoint)
{
    // 頻度の高い16wait値を取得
    std::map<int,int> frequentWaitValue;
    getFrequentWaitValue(frequentWaitValue, WAIT_VAL_NUM);
    
    int inPtr = 0;
    int outPtr = 0;
    
    for (auto it = frequentWaitValue.begin(); it != frequentWaitValue.end(); it++) {
        int value = it->first;
        int ind = it->second;
        mWaitvalTable[ind * 2] = value & 0xff;
        mWaitvalTable[ind * 2+1] = (value >> 8) & 0xff;
    }
    
    while (inPtr < inDataSize) {
        // ループポイントの変換
        if (inPtr == mLoopPoint) {
            *outLoopPoint = outPtr;
        }
        
        unsigned char cmd = inData[inPtr];
        int len = getCommandLength(cmd);
        bool found = false;
        int value;
        bool isWaitCmd = true;
        switch (cmd) {
            case 0x92:
                // 8bit値シンク
                value = inData[inPtr+1];
                break;
            case 0x94:
                // 16bit値シンク
                value = inData[inPtr+1] | (inData[inPtr+2] << 8);
                break;
            case 0x96:
                // 8bitx2シンク
                value = inData[inPtr+1] << 1;
                break;
            case 0x98:
                // 8bitx4シンク
                value = inData[inPtr+1] << 2;
                break;
            case 0x9a:
                // 8bitx8シンク
                value = inData[inPtr+1] << 3;
                break;
            case 0x9c:
                // 8bitx16シンク
                value = inData[inPtr+1] << 4;
                break;
            default:
                isWaitCmd = false;
                break;
        }
        if (isWaitCmd) {
            auto it = frequentWaitValue.find(value);
            if (it != frequentWaitValue.end()) {
                outData[outPtr++] = (it->second) * 2 + 0xa0;
                inPtr += len;
                found = true;
            }
            else if (cmd == 0x94) {
                auto it = frequentWaitValue.find(value-1);
                if (it != frequentWaitValue.end()) {
                    outData[outPtr++] = (it->second) * 2 + 0xa0;
                    outData[outPtr++] = 0x90;
                    inPtr += len;
                    found = true;
                }
            }
        }
        
        if (found == false) {
            for (int i=0; i<len; i++) {
                outData[outPtr++] = inData[inPtr++];
            }
        }
    }
    
    // 削減後のバイト数を返す
    return outPtr;
}

//-----------------------------------------------------------------------------
int RegisterLogger::getFrequentWaitValue(std::map<int,int> &outValues, int numValues)
{
    // キーにwait値、valueに何番目の値かが入ったmapを返す
    std::map<int, int> waitStat(mWaitStat);
    int foundValues = 0;
    while (foundValues < numValues && waitStat.size() > 0) {
        int max = 0;
        int maxWait = 0;
        for(auto it = waitStat.begin(); it != waitStat.end(); it++) {
            if( max < it->second ) {
                max = it->second;
                maxWait = it->first;
            }
        }
        auto maxIt = waitStat.find(maxWait);
        if (maxIt != waitStat.end()) {
            waitStat.erase(maxIt);
            outValues[maxWait] = foundValues++;
        }
    }
    return foundValues;
}

//-----------------------------------------------------------------------------
int RegisterLogger::convertTime2Tick(int time)
{
    int tick = static_cast<int>((time * mTickPerSec) / mProcessSampleRate + 0.5);
    return tick;
}

//-----------------------------------------------------------------------------
int getCommandLength(unsigned char cmd)
{
    if (cmd < 0x80) {
        if ((cmd & 0x0f) == 0x03) {
            return 3;
        }
        return 2;
    }
    if (cmd >= 0xa0 && cmd < 0xc0) {
        return 1;
    }
    if (cmd >= 0xc0 && cmd < 0xe0) {
        return 3;
    }
    switch (cmd) {
        case 0x80:
            return 4;
        case 0x90:
            return 1;
        case 0x92:
            return 2;
        case 0x94:
            return 3;
        case 0x96:
            return 2;
        case 0x98:
            return 2;
        case 0x9a:
            return 2;
        case 0x9c:
            return 2;
        case 0x9e:
            return 1;
        default:
            return 1;
    }
}
