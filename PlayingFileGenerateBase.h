//
//  PlayingFileGenerateBase.h
//  C700
//
//  Created by osoumen on 2017/01/27.
//
//

#ifndef __C700__PlayingFileGenerateBase__
#define __C700__PlayingFileGenerateBase__

#include <map>
#include "RegisterLogger.h"

class PlayingFileGenerateBase {
public:
    PlayingFileGenerateBase(int allocSize=4*1024*1024);
    virtual ~PlayingFileGenerateBase();
    
    virtual bool        WriteToFile( const char *path, const RegisterLogger &reglog, double tickPerSec );
    
protected:
    void                writeDspRegion( DataBuffer &buffer, const RegisterLogger &reglog );
    void                writeDirRegionWithHeader( DataBuffer &buffer, const RegisterLogger &reglog );
    void                writeBrrRegionWithHeader( DataBuffer &buffer, const RegisterLogger &reglog, int bankSize=0 );
    void                writeRegLogWithLoopPoint( DataBuffer &buffer, const RegisterLogger &reglog, double tickPerSec );
    void                writeWaitTable( DataBuffer &buffer, const RegisterLogger &reglog );
    int                 convertLogData( const RegisterLogger &reglog, double tickPerSec, unsigned char *outData, int outDataSize, int *outLoopPoint, bool skipInitialize );
    
private:
    static const int    WAIT_VAL_NUM = 32;
    static const int    WAIT_TABLE_LEN = WAIT_VAL_NUM * 2;
    
    double              mTickPerTime;
    unsigned char       mWaitvalTable[WAIT_TABLE_LEN];
    DataBuffer          mRegLogBuffer;
	int                 mDumpBeginTime;
	int                 mPrevTime;
	int                 mLoopPoint;
    std::map<int,int>   mWaitStat;
    
    void				beginConvert( int time );
	bool				addRegWrite( int device, int addr, unsigned char data, int time );
    bool                addPitchRegWrite( int device, int addr, unsigned char data_l, unsigned char data_m, int time );
	void				markLoopPoint();
	void				endConvert(int time);
    
	int                 optimizeWaits(const unsigned char *inData, int inDataSize, unsigned char *outData, int outDataSize, int *outLoopPoint);
    bool				writeByte( unsigned char byte );
	bool				writeEndByte();
	bool				writeWaitFromPrev(int tick);
    bool                addWaitStatistic(int tick);
    int                 getFrequentWaitValue(std::map<int,int> &outValues, int numValues);
    int                 convertTime2Tick(int time);

};

#endif /* defined(__C700__PlayingFileGenerateBase__) */
