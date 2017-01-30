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

class PlayingFileGenerateBase;

class RegisterLogger : public ChunkReader {
public:
    typedef struct {
        int    time;
        unsigned char data[4];
    } LogCommands;
    
	RegisterLogger(int allocSize=4*1024*1024);
	~RegisterLogger();
	
    void                addDspRegRegion(const unsigned char *data);  // 256ƒoƒCƒgŒÅ’è
    void                addDirRegion(int locateAddr, int size, unsigned char *data);
    void                addBrrRegion(int locateAddr, int size, unsigned char *data);
    
    unsigned char*      getDirRegionData() { return mDirRegionData; }
    int                 getDirRegionLocateAddr() { return mDirRegionLocateAddr; }
    int                 getDirRegionSize() { return mDirRegionSize; }
    unsigned char*      getBrrRegionData() { return mBrrRegionData; }
    int                 getBrrRegionLocateAddr() { return mBrrRegionLocateAddr; }
    int                 getBrrRegionSize() { return mBrrRegionSize; }
    
    bool                Write();
    
	bool				IsEnded() const { return mIsEnded; }
    void                SetProcessSampleRate( int rate );
    
    void				BeginDump( int time );
	bool				DumpReg( int device, int addr, unsigned char data, int time );
    bool                DumpApuPitch( int device, int addr, unsigned char data_l, unsigned char data_m, int time );
	void				MarkLoopPoint();
	void				EndDump(int time);

    friend PlayingFileGenerateBase;
    
protected:
    static const int    DSP_REGION_LEN = 256;
    
    unsigned char       mDspRegionData[DSP_REGION_LEN];
    unsigned char       *mDirRegionData;
    int                 mDirRegionLocateAddr;
    int                 mDirRegionSize;
    unsigned char       *mBrrRegionData;
    int                 mBrrRegionLocateAddr;
    int                 mBrrRegionSize;
    
    bool                mIsEnded;
    int                 mProcessSampleRate;
    short               mReg[256];
    
    LogCommands         *m_pLogCommands;
    int                 mLogCommandsSize;
    int                 mLogCommandsPos;
    int                 mLogCommandsLoopPoint;
};
