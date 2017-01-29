//
//  PlayingFileGenerateBase.cpp
//  C700
//
//  Created by osoumen on 2017/01/27.
//
//

#include "PlayingFileGenerateBase.h"

static int getCommandLength(unsigned char cmd);

//std::map<int,int> regStat;

//-----------------------------------------------------------------------------
PlayingFileGenerateBase::PlayingFileGenerateBase(int allocSize)
: mDataBuffer( allocSize )
, mTickPerTime( 15734.0 / 32000.0 )
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
void PlayingFileGenerateBase::writeDirRegion( DataBuffer &buffer, const RegisterLogger &reglog )
{
    unsigned char header[4];
    header[0] = reglog.mDirRegionLocateAddr & 0xff;
    header[1] = (reglog.mDirRegionLocateAddr >> 8) & 0xff;
    header[2] = reglog.mDirRegionSize & 0xff;
    header[3] = (reglog.mDirRegionSize >> 8) & 0xff;
    buffer.writeData(header, 4);
    buffer.writeData(reglog.mDirRegionData, reglog.mDirRegionSize);
}

//-----------------------------------------------------------------------------
void PlayingFileGenerateBase::writeBrrRegion( DataBuffer &buffer, const RegisterLogger &reglog, int bankSize )
{
    if (bankSize <= 0) {
        bankSize = 0x7fffffff;
    }
    // 合計サイズがbankSizeを超える場合は分割する(通常は32KB)
    const int bankHeaderSize = 4;
    unsigned char header[bankHeaderSize];
    unsigned char *data = reglog.mBrrRegionData;
    int startAddr = reglog.mBrrRegionLocateAddr;
    int writeBytes = reglog.mBrrRegionSize;
    
    while (writeBytes > 0) {
        int toWrite = (writeBytes > (bankSize-bankHeaderSize)) ? (bankSize-bankHeaderSize):writeBytes;
        header[0] = startAddr & 0xff;
        header[1] = (startAddr >> 8) & 0xff;
        header[2] = toWrite & 0xff;
        header[3] = (toWrite >> 8) & 0xff;
        buffer.writeData(header, 4);
        buffer.writeData(data, toWrite);
        writeBytes -= toWrite;
        startAddr += toWrite;
        data += toWrite;
    }
}

//-----------------------------------------------------------------------------
void PlayingFileGenerateBase::writeRegLog( DataBuffer &buffer, const RegisterLogger &reglog, double tickPerSec )
{
    // タイムベースの変換
    compileLogData( reglog, tickPerSec );
    
    // データの削減
    unsigned char *optimizedData = new unsigned char [mDataBuffer.GetMaxDataSize()];
    int optimizedDataSize;
    int optimizedLoopPoint;
    optimizedDataSize = optimizeWaits(mDataBuffer.GetDataPtr(), optimizedData, mDataBuffer.GetDataSize(), &optimizedLoopPoint);
    
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
    // TODO: pathからディレクトリを抽出
    char directory[] = "/Users/osoumen/Desktop/";    // 仮
    char fname[PATH_LEN_MAX];
    {
        // DSP領域の書き出し
        DataBuffer buffer(RegisterLogger::DSP_REGION_LEN);
        writeDspRegion(buffer, reglog);
        strncpy(fname, directory, PATH_LEN_MAX);
        strncat(fname, "regdump.dat", 12);
        buffer.WriteToFile(fname);
    }
    {
        // DIR領域の書き出し
        DataBuffer buffer(reglog.mDirRegionSize + 4);
        writeDirRegion(buffer, reglog);
        strncpy(fname, directory, PATH_LEN_MAX);
        strncat(fname, "dirregion.dat", 14);
        buffer.WriteToFile(fname);
    }
    {
        // BRR領域の書き出し
        DataBuffer buffer(reglog.mBrrRegionSize + 4);
        writeBrrRegion(buffer, reglog, 0x8000);
        strncpy(fname, directory, PATH_LEN_MAX);
        strncat(fname, "brrregion.dat", 14);
        buffer.WriteToFile(fname);
    }
    {
        // レジスタログの書き出し
        //DataBuffer buffer(optimizedDataSize + 3);
        DataBuffer buffer(1024 * 1024 * 6);     // 仮
        writeRegLog(buffer, reglog, tickPerSec);
        strncpy(fname, directory, PATH_LEN_MAX);
        strncat(fname, "spclog.dat", 11);
        buffer.WriteToFile(fname);
    }
    {
        // WaitTableの書き出し
        DataBuffer buffer(WAIT_TABLE_LEN);
        writeWaitTable(buffer, reglog);
        strncpy(fname, directory, PATH_LEN_MAX);
        strncat(fname, "waittable.dat", 14);
        buffer.WriteToFile(fname);
    }
    
    return true;
}

//-----------------------------------------------------------------------------
void PlayingFileGenerateBase::compileLogData( const RegisterLogger &reglog, double tickPerSec )
{
    mTickPerTime = tickPerSec / reglog.mProcessSampleRate;
    
    if ( reglog.mLogCommandsPos == 0) {
        return;
    }
    
    BeginDump_(0);
    for (int i=0; i<reglog.mLogCommandsPos; i++) {
        if (i == reglog.mLogCommandsLoopPoint) {
            MarkLoopPoint_();
        }
        unsigned char cmd = reglog.m_pLogCommands[i].data[0];
        int cmdLen = getCommandLength(cmd);
        if (cmd < 0x80) {
            if (cmdLen == 2) {
                DumpReg_( 0, cmd, reglog.m_pLogCommands[i].data[1], reglog.m_pLogCommands[i].time );
            }
            else if (cmdLen == 3) {
                DumpApuPitch_( 0, cmd, reglog.m_pLogCommands[i].data[2], reglog.m_pLogCommands[i].data[1], reglog.m_pLogCommands[i].time );
            }
        }
        else if (cmd == 0x9e) {
            EndDump_(reglog.m_pLogCommands[i].time);
            break;
        }
    }
}

//-----------------------------------------------------------------------------
void PlayingFileGenerateBase::BeginDump_( int time )
{
    int tick = convertTime2Tick(time);
    
	mDumpBeginTime = tick;
	mPrevTime = mDumpBeginTime;
	
	mDataBuffer.Clear();
	mLoopPoint = 0;
    
    //regStat.clear();
    mWaitStat.clear();
	
    //	printf("--BeginDump--¥n");
}

//-----------------------------------------------------------------------------
bool PlayingFileGenerateBase::DumpReg_( int device, int addr, unsigned char data, int time )
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
bool PlayingFileGenerateBase::DumpApuPitch_( int device, int addr, unsigned char data_l, unsigned char data_m, int time )
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
void PlayingFileGenerateBase::MarkLoopPoint_()
{
	mLoopPoint = mDataBuffer.GetDataPos();
    //	printf("--MarkLoopPoint--¥n");
}

//-----------------------------------------------------------------------------
void PlayingFileGenerateBase::EndDump_(int time)
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
        //		printf("--EndDump-- %d¥n",time);
	}
}

//-----------------------------------------------------------------------------
bool PlayingFileGenerateBase::writeByte( unsigned char byte )
{
	if ( ( mDataBuffer.GetDataPos() + 1 ) > (mDataBuffer.GetMaxDataSize()-1) ) {	//END/LOOPが書き込める様に１バイト残しておく
		return false;
	}
    return mDataBuffer.writeByte(byte);
}

//-----------------------------------------------------------------------------
bool PlayingFileGenerateBase::writeEndByte()
{
	return mDataBuffer.writeByte(0x9e);
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
int PlayingFileGenerateBase::optimizeWaits(const unsigned char *inData, unsigned char *outData, int inDataSize, int *outLoopPoint)
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

