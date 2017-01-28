/*
 *  RegisterLogger.h
 *  VOPM
 *
 *  Created by osoumen on 12/11/04.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include <time.h>
#include <map>

#include "ChunkReader.h"

class RegisterLogger : public ChunkReader {
public:
    typedef struct {
        int    time;
        unsigned char data[4];
    } LogCommands;
    
	RegisterLogger(int allocSize=4*1024*1024);
	~RegisterLogger();
	
    void                addWaitTable(const unsigned char *data);     // 64バイト固定
    void                addDspRegRegion(const unsigned char *data);  // 256バイト固定
    void                addDirRegion(int locateAddr, int size, unsigned char *data);
    void                addBrrRegion(int locateAddr, int size, unsigned char *data);
    
    bool                Write();
    
	bool				IsEnded() const { return mIsEnded; }
	//bool				SaveToFile( const char *path, double tickPerSec );
    void                SetProcessSampleRate( int rate );
    
    void				BeginDump( int time );
	bool				DumpReg( int device, int addr, unsigned char data, int time );
    bool                DumpApuPitch( int device, int addr, unsigned char data_l, unsigned char data_m, int time );
	void				MarkLoopPoint();
	void				EndDump(int time);
    
    unsigned char       *GetWaitvalTable() { return mWaitvalTable; }

protected:
    static const int    WAIT_VAL_NUM = 32;
    static const int    WAIT_TABLE_LEN = 64;
    static const int    DSP_REGION_LEN = 256;

    void                compileLogData( double tickPerSec );
    void				BeginDump_( int time );
	bool				DumpReg_( int device, int addr, unsigned char data, int time );
    bool                DumpApuPitch_( int device, int addr, unsigned char data_l, unsigned char data_m, int time );
	void				MarkLoopPoint_();
	void				EndDump_(int time);
    
	bool				writeByte( unsigned char byte );
	bool				writeEndByte();
	bool				writeWaitFromPrev(int tick);
    bool                addWaitStatistic(int tick);
	int                 optimizeWaits(const unsigned char *inData, unsigned char *outData, int inDataSize, int *outLoopPoint);
    int                 getFrequentWaitValue(std::map<int,int> &outValues, int numValues);
    int                 convertTime2Tick(int time);
    
    unsigned char   mWaitTableData[WAIT_TABLE_LEN];
    unsigned char   mDspRegionData[DSP_REGION_LEN];
    unsigned char   *mDirRegionData;
    unsigned char   *mBrrRegionData;
    
    int             mDirRegionLocateAddr;
    int             mDirRegionSize;
    int             mBrrRegionLocateAddr;
    int             mBrrRegionSize;
    
    DataBuffer      mDataBuffer;
	int				mDumpBeginTime;
	int				mPrevTime;
	int				mLoopPoint;
	bool			mIsEnded;
	short			mReg[256];
	double          mTickPerSec;
    int             mProcessSampleRate;
    
    unsigned char   mWaitvalTable[WAIT_VAL_NUM*2];
    
    LogCommands     *m_pLogCommands;
    int             mLogCommandsSize;
    int             mLogCommandsPos;
    int             mLogCommandsLoopPoint;
    
    std::map<int,int> mWaitStat;
};
