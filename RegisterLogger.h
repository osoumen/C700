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
	void				MarkLoopPoint();
	void				EndDump(int time);

protected:
	typedef struct {
		char			magic[3];
		char			format_version;
		unsigned int	timer_info;
		unsigned int	timer_info2;
		unsigned int	compressing;
		unsigned int	file_offset_to_tag;
		unsigned int	file_offset_to_dumpdata;
		unsigned int	file_offset_to_looppoint;
		unsigned int	device_count;	//"1"
		unsigned int	device_type;
		unsigned int	clock;
		unsigned int	pan;
		unsigned int	reserve;
	} S98Header;
	
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
    
    std::map<int,int> mWaitStat;
};
