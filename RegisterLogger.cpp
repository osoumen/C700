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

std::map<int,int> regStat;

static int getCommandLength(unsigned char cmd);

//-----------------------------------------------------------------------------
RegisterLogger::RegisterLogger(int allocSize)
: m_pData( NULL )
, mDataSize( allocSize )
, mDataUsed( 0 )
, mDataPos( 0 )
, mTickPerSec( 15734 )
, mProcessSampleRate( 32000 )
{
	if ( allocSize > 0 ) {
		m_pData = new unsigned char[allocSize];
	}
	BeginDump(0);
}

//-----------------------------------------------------------------------------
RegisterLogger::~RegisterLogger()
{
	if ( m_pData != NULL ) {
		delete [] m_pData;
	}
}

//-----------------------------------------------------------------------------
bool RegisterLogger::SetPos( int pos )
{
	if ( mDataSize < pos ) {
		return false;
	}
	mDataPos = pos;
	if ( mDataPos > mDataUsed ) {
		mDataUsed = mDataPos;
	}
	return true;
}

//-----------------------------------------------------------------------------
bool RegisterLogger::SaveToFile( const char *path, int clock )
{
    // データの削減
    unsigned char *optimizedData = new unsigned char [mDataSize];
    int optimizedDataSize;
    int optimizedLoopPoint;
    optimizedDataSize = optimizeWaits(m_pData, optimizedData, mDataUsed, &optimizedLoopPoint);

    int loopAddr = optimizedLoopPoint + 3;
    unsigned char loopStart[3];
    loopStart[0] = loopAddr & 0xff;
    loopStart[1] = ((loopAddr >> 8) & 0x7f) + 0x80;
    loopStart[2] = (loopAddr >> 15) & 0xff;
	
#if __APPLE_CC__
	CFURLRef	savefile = CFURLCreateFromFileSystemRepresentation(NULL, (UInt8*)path, strlen(path), false);
	
	CFWriteStreamRef	filestream = CFWriteStreamCreateWithFile(NULL,savefile);
	if (CFWriteStreamOpen(filestream)) {
		//CFWriteStreamWrite(filestream, reinterpret_cast<UInt8*> (&header), sizeof(S98Header) );
        CFWriteStreamWrite(filestream, loopStart, 3 );
		CFWriteStreamWrite(filestream, optimizedData, optimizedDataSize );
		//CFWriteStreamWrite(filestream, reinterpret_cast<UInt8*> (tag), sizeof(tag) );
		CFWriteStreamClose(filestream);
	}
	CFRelease(filestream);
	CFRelease(savefile);
	
    delete [] optimizedData;
	return true;
#else
	HANDLE	hFile;
	
	hFile = CreateFile( path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile != INVALID_HANDLE_VALUE ) {
		DWORD	writeSize;
		//WriteFile( hFile, &header, sizeof(S98Header), &writeSize, NULL );
        WriteFile( hFile, loopStart, 3, &writeSize, NULL );
		WriteFile( hFile, optimizedData, optimizedDataSize, &writeSize, NULL );
		//WriteFile( hFile, tag, sizeof(tag), &writeSize, NULL );
		CloseHandle( hFile );
	}
    delete [] optimizedData;
	return true;
#endif
}

//-----------------------------------------------------------------------------
void RegisterLogger::SetResolution( double tickPerSec )
{
	mTickPerSec = tickPerSec;
}

//-----------------------------------------------------------------------------
void RegisterLogger::SetProcessSampleRate( int rate )
{
    mProcessSampleRate = rate;
}

//-----------------------------------------------------------------------------
void RegisterLogger::BeginDump( int time )
{
    int tick = static_cast<int>((time * mTickPerSec) / mProcessSampleRate + 0.5);
    
	mDumpBeginTime = tick;
	mPrevTime = mDumpBeginTime;
	for ( int i=0; i<256; i++ ) {
        mReg[i] = -1;
	}
	mDataUsed = 0;
	mDataPos = 0;
	mLoopPoint = 0;
	mIsEnded = false;
    
    regStat.clear();
    mWaitStat.clear();
	
//	printf("--BeginDump--\n");
}

//-----------------------------------------------------------------------------
bool RegisterLogger::DumpReg( int device, int addr, unsigned char data, int time )
{
    int tick = static_cast<int>((time * mTickPerSec) / mProcessSampleRate + 0.5);
    
    if (tick < mDumpBeginTime) {
        return false;
    }
    
	if (addr >= 0 && addr < 128) {
		
		if ( mReg[addr] != data || addr == 0x4c || addr == 0x5c) {
			mReg[addr] = data;

			writeWaitFromPrev(tick);
			
			if ( GetWritableSize() >= 3 ) {
				writeByte( addr );
				writeByte( data );
                if (regStat.count(addr) == 0) {
                    regStat[addr] = 1;
                }
                else {
                    regStat[addr] = regStat[addr]+1;
                }
			}
			
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
bool RegisterLogger::DumpApuPitch( int device, int addr, unsigned char data_l, unsigned char data_m, int time )
{
    int tick = static_cast<int>((time * mTickPerSec) / mProcessSampleRate + 0.5);
    
    if (tick < mDumpBeginTime) {
        return false;
    }
    
    if ( (addr & 0x0f) == 0x02 ) {
        if (data_m == mReg[addr+1]) {
            return DumpReg( device, addr, data_l, time );
        }
        mReg[addr] = data_l;
        mReg[addr+1] = data_m;
        
        writeWaitFromPrev(tick);
        
        if ( GetWritableSize() >= 4 ) {
            writeByte( addr+1 );
            writeByte( data_m );
            writeByte( data_l );
            if (regStat.count(addr+1) == 0) {
                regStat[addr+1] = 1;
            }
            else {
                regStat[addr+1] = regStat[addr+1]+1;
            }
        }
        
        return true;
    }
	return false;
}

//-----------------------------------------------------------------------------
void RegisterLogger::MarkLoopPoint()
{
	mLoopPoint = mDataPos;
	//ループ直後は常にレジスタが書き込まれるようにする
	for ( int i=0; i<256; i++ ) {
		mReg[i] = -1;
	}
//	printf("--MarkLoopPoint--\n");
}

//-----------------------------------------------------------------------------
void RegisterLogger::EndDump(int time)
{
    int tick = static_cast<int>((time * mTickPerSec) / mProcessSampleRate + 0.5);
    
	if ( mDataUsed > 0 && mIsEnded == false ) {
		writeWaitFromPrev(tick);
		writeEndByte();
		mIsEnded = true;
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
	if ( ( mDataPos + 1 ) > (mDataSize-1) ) {	//END/LOOPが書き込める様に１バイト残しておく
		return false;
	}
	m_pData[mDataPos] = byte;
	mDataPos++;
	if ( mDataPos > mDataUsed ) {
		mDataUsed = mDataPos;
	}
	return true;
}

//-----------------------------------------------------------------------------
bool RegisterLogger::writeEndByte()
{
	if ( ( mDataPos + 1 ) > mDataSize) {
		return false;
	}
	m_pData[mDataPos] = 0x9e;
	mDataPos++;
	if ( mDataPos > mDataUsed ) {
		mDataUsed = mDataPos;
	}
	return true;
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
        result = writeByte(0x94);
        if ( result == false ) return false;
        result = writeByte(0xff);
        if ( result == false ) {
            //書き込んだ分を巻き戻す
            mDataPos -= 1;
            mDataUsed = mDataPos;
            return false;
        }
        result = writeByte(0xff);
        if ( result == false ) {
            //書き込んだ分を巻き戻す
            mDataPos -= 2;
            mDataUsed = mDataPos;
            return false;
        }
        addWaitStatistic(0xffff);
    }
    if (mod > 0) {
        if (mod < 0x100) {
            result = writeByte(0x92);
            if ( result == false ) return false;
            result = writeByte(mod & 0xff);
            if ( result == false ) {
                //書き込んだ分を巻き戻す
                mDataPos -= 1;
                mDataUsed = mDataPos;
                return false;
            }
            addWaitStatistic(mod & 0xff);
        }
        else {
#if 1
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
                result = writeByte(0x94);
                if ( result == false ) return false;
                result = writeByte(mod & 0xff);
                if ( result == false ) {
                    //書き込んだ分を巻き戻す
                    mDataPos -= 1;
                    mDataUsed = mDataPos;
                    return false;
                }
                result = writeByte(mod >> 8);
                if ( result == false ) {
                    //書き込んだ分を巻き戻す
                    mDataPos -= 2;
                    mDataUsed = mDataPos;
                    return false;
                }
            }
            addWaitStatistic(mod);
#else
            //テスト
            result = writeByte(0xc0);
            if ( result == false ) return false;
            result = writeByte(mod & 0xff);
            if ( result == false ) {
                //書き込んだ分を巻き戻す
                mDataPos -= 1;
                mDataUsed = mDataPos;
                return false;
            }
            result = writeByte(mod >> 8);
            if ( result == false ) {
                //書き込んだ分を巻き戻す
                mDataPos -= 2;
                mDataUsed = mDataPos;
                return false;
            }
            result = writeByte(0xa0);
#endif
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
int RegisterLogger::optimizeWaits(unsigned char *inData, unsigned char *outData, int inDataSize, int *outLoopPoint)
{
    // 頻度の高い16wait値を取得
    std::map<int,int> frequentWaitValue;
    getFrequentWaitValue(frequentWaitValue, WAIT_VAL_NUM);
    
    int inPtr = 0;
    int outPtr = 0;
    
#if 0
    // wait値設定コマンドを出力
    for (auto it = frequentWaitValue.begin(); it != frequentWaitValue.end(); it++) {
        int value = it->first;
        int ind = it->second;
        outData[outPtr++] = 0xc0 + ind*2;
        outData[outPtr++] = value & 0xff;
        outData[outPtr++] = (value >> 8) & 0xff;
    }
#else
    for (auto it = frequentWaitValue.begin(); it != frequentWaitValue.end(); it++) {
        int value = it->first;
        int ind = it->second;
        mWaitvalTable[ind * 2] = value & 0xff;
        mWaitvalTable[ind * 2+1] = (value >> 8) & 0xff;
    }
#endif
    
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