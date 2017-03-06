//
//  PlayingFileGenerateBase.cpp
//  C700
//
//  Created by osoumen on 2017/01/27.
//
//

#include "PlayingFileGenerateBase.h"

static int getCommandLength(unsigned char cmd);
void getFileNameParentPath( const char *path, char *out, int maxLen );

//std::map<int,int> regStat;


//-----------------------------------------------------------------------------
PlayingFileGenerateBase::PlayingFileGenerateBase(int allocSize)
: mTickPerTime( 15734.0 / 32000.0 )
, mRegLogBuffer( allocSize )
{
    
}

//-----------------------------------------------------------------------------
PlayingFileGenerateBase::~PlayingFileGenerateBase()
{
    
}

//-----------------------------------------------------------------------------
void PlayingFileGenerateBase::writeDspRegion( DataBuffer &buffer, const RegisterLogger &reglog )
{
    buffer.writeData(reglog.mDspRegionData, RegisterLogger::DSP_REGION_LEN);
}

//-----------------------------------------------------------------------------
void PlayingFileGenerateBase::writeDirRegionWithHeader( DataBuffer &buffer, const RegisterLogger &reglog )
{
    buffer.writeU16( reglog.mDirRegionLocateAddr & 0xffff );
    buffer.writeU16( reglog.mDirRegionSize & 0xffff );
    buffer.writeData(reglog.mDirRegionData, reglog.mDirRegionSize);
}

//-----------------------------------------------------------------------------
void PlayingFileGenerateBase::writeBrrRegionWithHeader( DataBuffer &buffer, const RegisterLogger &reglog, int bankSize )
{
    if (bankSize <= 0) {
        bankSize = 0x7fffffff;
    }
    // 合計サイズがbankSizeを超える場合は分割する(通常は32KB)
    const int bankHeaderSize = 4;
    unsigned char *data = reglog.mBrrRegionData;
    int startAddr = reglog.mBrrRegionLocateAddr;
    int writeBytes = reglog.mBrrRegionSize;
    
    while (writeBytes > 0) {
        int toWrite = (writeBytes > (bankSize-bankHeaderSize)) ? (bankSize-bankHeaderSize):writeBytes;
        buffer.writeU16( startAddr );
        buffer.writeU16( toWrite );
        buffer.writeData(data, toWrite);
        writeBytes -= toWrite;
        startAddr += toWrite;
        data += toWrite;
    }
}

//-----------------------------------------------------------------------------
void PlayingFileGenerateBase::writeRegLogWithLoopPoint( DataBuffer &buffer, const RegisterLogger &reglog, double tickPerSec )
{
    // タイムベースの変換
    unsigned char *optimizedData = new unsigned char [mRegLogBuffer.GetDataSize()];
    int optimizedLoopPoint;
    int optimizedDataSize = convertLogData( reglog, tickPerSec, optimizedData, mRegLogBuffer.GetDataSize(), &optimizedLoopPoint, false );
    
    int loopAddr = optimizedLoopPoint + 3;
    unsigned char loopStart[3];
    loopStart[0] = loopAddr & 0xff;
    loopStart[1] = ((loopAddr >> 8) & 0x7f) + 0x80;
    loopStart[2] = (loopAddr >> 15) & 0xff;
    buffer.writeData(loopStart, 3);
    buffer.writeData(optimizedData, optimizedDataSize);
    delete [] optimizedData;
}

//-----------------------------------------------------------------------------
void PlayingFileGenerateBase::writeWaitTable( DataBuffer &buffer, const RegisterLogger &reglog )
{
    buffer.writeData(mWaitvalTable, WAIT_TABLE_LEN);
}

//-----------------------------------------------------------------------------
bool PlayingFileGenerateBase::WriteToFile( const char *path, const RegisterLogger &reglog, double tickPerSec )
{
    // pathからディレクトリを抽出
    char directory[PATH_LEN_MAX];
    getFileNameParentPath(path, directory, PATH_LEN_MAX);
    
    char fname[PATH_LEN_MAX];
    {
        // DSP領域の書き出し
        DataBuffer buffer(RegisterLogger::DSP_REGION_LEN);
        writeDspRegion(buffer, reglog);
        strncpy(fname, directory, PATH_LEN_MAX);
#if MAC
        strncat(fname, "/regdump.dat", 12);
#else
        strncat(fname, "¥¥regdump.dat", 12);
#endif
        buffer.WriteToFile(fname);
    }
    {
        // DIR領域の書き出し
        DataBuffer buffer(reglog.mDirRegionSize + 4);
        writeDirRegionWithHeader(buffer, reglog);
        strncpy(fname, directory, PATH_LEN_MAX);
#if MAC
        strncat(fname, "/dirregion.dat", 14);
#else
        strncat(fname, "¥¥dirregion.dat", 14);
#endif
        buffer.WriteToFile(fname);
    }
    {
        // BRR領域の書き出し
        DataBuffer buffer(reglog.mBrrRegionSize + 4);
        writeBrrRegionWithHeader(buffer, reglog, 0x8000);
        strncpy(fname, directory, PATH_LEN_MAX);
#if MAC
        strncat(fname, "/brrregion.dat", 14);
#else
        strncat(fname, "¥¥brrregion.dat", 14);
#endif
        buffer.WriteToFile(fname);
    }
    {
        // レジスタログの書き出し
        //DataBuffer buffer(optimizedDataSize + 3);
        DataBuffer buffer(1024 * 1024 * 4);     // 仮
        writeRegLogWithLoopPoint(buffer, reglog, tickPerSec);
        strncpy(fname, directory, PATH_LEN_MAX);
#if MAC
        strncat(fname, "/spclog.dat", 11);
#else
        strncat(fname, "¥¥spclog.dat", 11);
#endif
        buffer.WriteToFile(fname);
    }
    {
        // WaitTableの書き出し
        DataBuffer buffer(WAIT_TABLE_LEN);
        writeWaitTable(buffer, reglog);
        strncpy(fname, directory, PATH_LEN_MAX);
#if MAC
        strncat(fname, "/waittable.dat", 14);
#else
        strncat(fname, "¥¥waittable.dat", 14);
#endif
        buffer.WriteToFile(fname);
    }
    
    return true;
}

//-----------------------------------------------------------------------------
int PlayingFileGenerateBase::convertLogData( const RegisterLogger &reglog, double tickPerSec, unsigned char *outData, int outDataSize, int *outLoopPoint, bool skipInitialize )
{
    mTickPerTime = tickPerSec / reglog.mProcessSampleRate;
    
    if ( reglog.mLogCommandsPos == 0) {
        return 0;
    }
    
    beginConvert(0);
    for (int i=0; i<reglog.mLogCommandsPos; i++) {
        if (i == reglog.mLogCommandsLoopPoint) {
            markLoopPoint();
        }
        if (skipInitialize) {
            if ((i >= reglog.mBeginInitializationPoint) && (i < reglog.mEndInitializationPoint)) {
                continue;
            }
        }
        unsigned char cmd = reglog.m_pLogCommands[i].data[0];
        int cmdLen = getCommandLength(cmd);
        if (cmd < 0x80) {
            if (cmdLen == 2) {
                addRegWrite( 0, cmd, reglog.m_pLogCommands[i].data[1], reglog.m_pLogCommands[i].time );
            }
            else if (cmdLen == 3) {
                addPitchRegWrite( 0, cmd, reglog.m_pLogCommands[i].data[2], reglog.m_pLogCommands[i].data[1], reglog.m_pLogCommands[i].time );
            }
        }
        else if (cmd == 0x9e) {
            endConvert(reglog.m_pLogCommands[i].time);
            break;
        }
    }
    
    // wait値を削減
    return optimizeWaits(mRegLogBuffer.GetDataPtr(), mRegLogBuffer.GetDataUsed(), outData, outDataSize, outLoopPoint);
}

//-----------------------------------------------------------------------------
void PlayingFileGenerateBase::beginConvert( int time )
{
    int tick = convertTime2Tick(time);
    
	mDumpBeginTime = tick;
	mPrevTime = mDumpBeginTime;
	
	mRegLogBuffer.Clear();
	mLoopPoint = 0;
    
    //regStat.clear();
    mWaitStat.clear();
	
    //	printf("--BeginDump--¥n");
}

//-----------------------------------------------------------------------------
bool PlayingFileGenerateBase::addRegWrite( int device, int addr, unsigned char data, int time )
{
    int tick = convertTime2Tick(time);
    
    if (tick < mDumpBeginTime) {
        return false;
    }
    
    writeWaitFromPrev(tick);
    
    if ( mRegLogBuffer.GetLeftSize() >= 3 ) {
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
bool PlayingFileGenerateBase::addPitchRegWrite( int device, int addr, unsigned char data_l, unsigned char data_m, int time )
{
    int tick = convertTime2Tick(time);
    
    if (tick < mDumpBeginTime) {
        return false;
    }
    
    if ( (addr & 0x0f) == 0x03 ) {
        
        writeWaitFromPrev(tick);
        
        if ( mRegLogBuffer.GetLeftSize() >= 4 ) {
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
void PlayingFileGenerateBase::markLoopPoint()
{
	mLoopPoint = mRegLogBuffer.GetDataPos();
    //	printf("--MarkLoopPoint--¥n");
}

//-----------------------------------------------------------------------------
void PlayingFileGenerateBase::endConvert(int time)
{
    int tick = convertTime2Tick(time);
    
	if ( mRegLogBuffer.GetDataUsed() > 0 ) {
		writeWaitFromPrev(tick);
		writeEndByte();
		/*
         for (auto it = regStat.begin(); it != regStat.end(); it++) {
         std::cout << "$" << std::hex << it->first << "," << std::dec << it->second << std::endl;
         }
         
         for (auto it = mWaitStat.begin(); it != mWaitStat.end(); it++) {
         std::cout << it->first << "," << it->second << std::endl;
         }*/
        //		printf("--EndDump-- %d¥n",time);
	}
}

//-----------------------------------------------------------------------------
bool PlayingFileGenerateBase::writeByte( unsigned char byte )
{
	if ( ( mRegLogBuffer.GetDataPos() + 1 ) > (mRegLogBuffer.GetDataSize()-1) ) {	//END/LOOPが書き込める様に１バイト残しておく
		return false;
	}
    return mRegLogBuffer.writeByte(byte);
}

//-----------------------------------------------------------------------------
bool PlayingFileGenerateBase::writeEndByte()
{
	return mRegLogBuffer.writeByte(0x9e);
}

//-----------------------------------------------------------------------------
bool PlayingFileGenerateBase::writeWaitFromPrev(int tick)
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
        DataBuffer::DataBufferState state = mRegLogBuffer.SaveState();
        
        result = writeByte(0x94);
        if ( result == false ) return false;
        result = writeByte(0xff);
        if ( result == false ) {
            //書き込んだ分を巻き戻す
            mRegLogBuffer.RestoreState(state);
            return false;
        }
        result = writeByte(0xff);
        if ( result == false ) {
            //書き込んだ分を巻き戻す
            mRegLogBuffer.RestoreState(state);
            return false;
        }
        addWaitStatistic(0xffff);
    }
    if (mod > 0) {
        if (mod < 0x100) {
            DataBuffer::DataBufferState state = mRegLogBuffer.SaveState();
            
            result = writeByte(0x92);
            if ( result == false ) return false;
            result = writeByte(mod & 0xff);
            if ( result == false ) {
                //書き込んだ分を巻き戻す
                mRegLogBuffer.RestoreState(state);
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
                DataBuffer::DataBufferState state = mRegLogBuffer.SaveState();
                
                result = writeByte(0x94);
                if ( result == false ) return false;
                result = writeByte(mod & 0xff);
                if ( result == false ) {
                    //書き込んだ分を巻き戻す
                    mRegLogBuffer.RestoreState(state);
                    return false;
                }
                result = writeByte(mod >> 8);
                if ( result == false ) {
                    //書き込んだ分を巻き戻す
                    mRegLogBuffer.RestoreState(state);
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
bool PlayingFileGenerateBase::addWaitStatistic(int tick)
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
int PlayingFileGenerateBase::optimizeWaits(const unsigned char *inData, int inDataSize, unsigned char *outData, int outDataSize, int *outLoopPoint)
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
                if (outPtr < outDataSize) {
                    outData[outPtr++] = (it->second) * 2 + 0xa0;
                }
                inPtr += len;
                found = true;
            }
            else if (cmd == 0x94) {
                auto it = frequentWaitValue.find(value-1);
                if (it != frequentWaitValue.end()) {
                    if (outPtr < outDataSize) {
                        outData[outPtr++] = (it->second) * 2 + 0xa0;
                    }
                    if (outPtr < outDataSize) {
                        outData[outPtr++] = 0x90;
                    }
                    inPtr += len;
                    found = true;
                }
            }
        }
        
        if (found == false) {
            for (int i=0; i<len; i++) {
                if (outPtr < outDataSize) {
                    outData[outPtr++] = inData[inPtr++];
                }
            }
        }
    }
    
    // 削減後のバイト数を返す
    return outPtr;
}

//-----------------------------------------------------------------------------
int PlayingFileGenerateBase::getFrequentWaitValue(std::map<int,int> &outValues, int numValues)
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
int PlayingFileGenerateBase::convertTime2Tick(int time)
{
    int tick = static_cast<int>(time * mTickPerTime + 0.5);
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

