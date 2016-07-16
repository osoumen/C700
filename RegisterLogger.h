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

class RegisterLogger {
public:
	RegisterLogger(int allocSize=4*1024*1024);
	~RegisterLogger();
	
	bool				IsEnded() const { return mIsEnded; }
	bool				SaveToFile( const char *path, int clock=0 );
	void				SetResolution( int numerator, int denominator );
	void				BeginDump( int time );
	bool				DumpReg( int device, int addr, unsigned char data, int time );
    bool                DumpApuPitch( int device, int addr, unsigned char data_l, unsigned char data_m, int time );
	void				MarkLoopPoint();
	void				EndDump(int time);
    unsigned char       *GetWaitvalTable() { return mWaitvalTable; }

protected:
    static const int    WAIT_VAL_NUM = 32;
    
	int					GetDataSize() const { return mDataUsed; }
	int					GetWritableSize() const { return (mDataSize - mDataPos - 1); }
	const unsigned char	*GetDataPtr() const { return m_pData; }
	int					GetDataPos() const { return mDataPos; }
	void				AdvDataPos( int adv ) { mDataPos+=adv; }
	bool				SetPos( int pos );
	
	bool				writeByte( unsigned char byte );
	bool				writeEndByte();
	bool				writeWaitFromPrev(int time);
    bool                addWaitStatistic(int time);
	int                 optimizeWaits(unsigned char *inData, unsigned char *outData, int inDataSize, int *outLoopPoint);
    int                 getFrequentWaitValue(std::map<int,int> &outValues, int numValues);
    
	unsigned char	*m_pData;
	int				mDataSize;
	int				mDataUsed;
	int				mDataPos;	
	
	int				mDumpBeginTime;
	int				mPrevTime;
	int				mLoopPoint;
	bool			mIsEnded;
	short			mReg[256];
	int				mTimeNumerator;
	int				mTimeDenominator;
    
    unsigned char   mWaitvalTable[WAIT_VAL_NUM*2];
    
    std::map<int,int> mWaitStat;
};
