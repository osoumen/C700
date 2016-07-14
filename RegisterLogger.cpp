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
, mTimeNumerator( 1 )
, mTimeDenominator( 15734 )
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
void RegisterLogger::SetResolution( int numerator, int denominator )
{
	mTimeNumerator = numerator;
	mTimeDenominator = denominator;
}

//-----------------------------------------------------------------------------
void RegisterLogger::BeginDump( int time )
{
	mDumpBeginTime = time;
	mPrevTime = mDumpBeginTime;
	for ( int i=0; i<256; i++ ) {
        mReg[i] = -1;
	}
	mDataUsed = 0;
	mDataPos = 0;
	mLoopPoint = 0;
	mIsEnded = false;
    
    mWaitStat.clear();
	
//	printf("--BeginDump--\n");
}

//-----------------------------------------------------------------------------
bool RegisterLogger::DumpReg( int device, int addr, unsigned char data, int time )
{
    if (time < mDumpBeginTime) {
        return false;
    }
    
	if (addr >= 0 && addr < 128) {
		
		if ( mReg[addr] != data || addr == 0x4c || addr == 0x5c) {
			mReg[addr] = data;

			writeWaitFromPrev(time);
			
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
	if ( mDataUsed > 0 && mIsEnded == false ) {
		writeWaitFromPrev(time);
		writeEndByte();
		mIsEnded = true;
		/*
        for (auto it = regStat.begin(); it != regStat.end(); it++) {
            std::cout << it->first << "," << it->second << std::endl;
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
bool RegisterLogger::writeWaitFromPrev(int time)
{
	bool		result;
	
	int		now_time	= time - mDumpBeginTime;
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
    
	mPrevTime = time;
    /*
	if ( adv_time < 0 ) {
		mPrevTime -= adv_time;
	}*/
	return true;
}

//-----------------------------------------------------------------------------
bool RegisterLogger::addWaitStatistic(int time)
{
    if (mWaitStat.count(time) == 0) {
        mWaitStat[time] = 1;
    }
    else {
        mWaitStat[time] = mWaitStat[time]+1;
    }
    return true;
}

//-----------------------------------------------------------------------------
int RegisterLogger::optimizeWaits(unsigned char *inData, unsigned char *outData, int inDataSize, int *outLoopPoint)
{
    // 頻度の高い16wait値を取得
    std::map<int,int> frequentWaitValue;
    getFrequentWaitValue(frequentWaitValue, 16);
    
    int inPtr = 0;
    int outPtr = 0;
    
    // wait値設定コマンドを出力
    for (auto it = frequentWaitValue.begin(); it != frequentWaitValue.end(); it++) {
        int value = it->first;
        int ind = it->second;
        outData[outPtr++] = 0xc0 + ind*2;
        outData[outPtr++] = value & 0xff;
        outData[outPtr++] = (value >> 8) & 0xff;
    }
    
    while (inPtr < inDataSize) {
        // ループポイントの変換
        if (inPtr == mLoopPoint) {
            *outLoopPoint = outPtr;
        }
        
        unsigned char cmd = inData[inPtr];
        int len = getCommandLength(cmd);
        bool found = false;
        if (cmd == 0x92) {
            // 8bit値シンク
            int value = inData[inPtr+1];
            auto it = frequentWaitValue.find(value);
            if (it != frequentWaitValue.end()) {
                outData[outPtr++] = (it->second) * 2 + 0xa0;
                inPtr += len;
                found = true;
            }
        }
        else if (cmd == 0x94) {
            // 16bit値シンク
            int value = inData[inPtr+1] | (inData[inPtr+2] << 8);
            auto it = frequentWaitValue.find(value);
            if (it != frequentWaitValue.end()) {
                outData[outPtr++] = (it->second) * 2 + 0xa0;
                inPtr += len;
                found = true;
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
        case 0x9e:
            return 1;
        default:
            return 1;
    }
}