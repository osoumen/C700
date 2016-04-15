//
//  edControlFTDI.cpp
//  gimicUsbSpcPlay
//
//  Created by osoumen on 2016/02/13.
//  Copyright (c) 2016年 osoumen. All rights reserved.
//

#include "ftd2xx.h"
#include "edControlFTDI.h"
#include <iostream>

edControlFTDI::edControlFTDI()
{
    InitUsb();
}

edControlFTDI::~edControlFTDI()
{
    CloseUsb();
}

bool edControlFTDI::InitUsb()
{
    if (Init() != 0) {
        return false;
    }
    return true;
}

bool edControlFTDI::CloseUsb()
{
    if (mIsPlugged) {
		FT_Close(ftHandle);
		mIsPlugged = false;
	}
    return true;
}

int edControlFTDI::Init()
{
    mWriteBytes = 0;
    
    FT_STATUS	ftStatus;
    
#if 1
    static const int MAX_DEVICES = 4;
    char * 	pcBufLD[MAX_DEVICES + 1];
	char 	cBufLD[MAX_DEVICES][64];
    int	iNumDevs = 0;
    for (int i = 0; i < MAX_DEVICES; i++) {
		pcBufLD[i] = cBufLD[i];
	}
	pcBufLD[MAX_DEVICES] = NULL;
    
    ftStatus = FT_ListDevices(pcBufLD, &iNumDevs, FT_LIST_ALL | FT_OPEN_BY_DESCRIPTION);
    
    if(ftStatus != FT_OK) {
		printf("Error: FT_ListDevices(%d)\n", (int)ftStatus);
		return -1;
	}
    
    for (int i = 0; ( (i <MAX_DEVICES) && (i < iNumDevs) ); i++) {
		printf("Device %d Description - %s\n", i, cBufLD[i]);
	}
#endif
    
    ftStatus = FT_Open(0, &ftHandle);
    if(ftStatus != FT_OK) {
        std::cout << "Error FT_Open(" << (int)ftStatus << ")" << std::endl;
        return -1;
    }
    
    ftStatus = FT_ResetDevice(ftHandle);
    
    mIsPlugged = true;
    
    return 0;
}

int edControlFTDI::Close()
{
    CloseUsb();
    return 0;
}

FT_STATUS	edControlFTDI::resetrPipe()
{
	if (!mIsPlugged) return -1;
	
	FT_STATUS kr = noErr;
	
	kr = FT_Purge(ftHandle, FT_PURGE_RX);
	
	return kr;
}

FT_STATUS	edControlFTDI::resetwPipe()
{
	if (!mIsPlugged) return -1;
	
	FT_STATUS kr = noErr;
	
	kr = FT_Purge(ftHandle, FT_PURGE_TX);
	
	return kr;
}

void edControlFTDI::Reset()
{
    resetrPipe();
    resetwPipe();
    // リセットできません
}

SInt32		edControlFTDI::bulkWrite(UInt8 *buf, UInt32 size)
{
	if (!mIsPlugged) return -1;
	FT_STATUS			kr = noErr;
    DWORD               len;
    
    UInt32      writeSize = size;
    for (int i=0; i<writeSize; i++) {
        mBuf[i] = buf[i];
    }
    kr = FT_Write(ftHandle, mBuf, writeSize, &len);
	
	return len;
}

SInt32		edControlFTDI::bulkRead(UInt8 *buf, UInt32 size)
{
	if (!mIsPlugged) return -1;
	FT_STATUS			kr = noErr;
    DWORD               len;
    
    UInt32      readSize = size;
    kr = FT_Read(ftHandle, buf, readSize, &len);
	
	return len;
}

void edControlFTDI::writeCmd(const char *cmd)
{
    int len = (int)strlen(cmd);
    UInt8 *buff = new UInt8[len];
    memcpy(buff, cmd, len);
    bulkWrite(buff, len);
}

void edControlFTDI::BlockWrite(int port, unsigned char data)
{
    if (mWriteBytes > (PACKET_SIZE-2)) {
        WriteBuffer();
    }
    mWriteBuf[mWriteBytes] = port;
    mWriteBytes++;
    mWriteBuf[mWriteBytes] = data;
    mWriteBytes++;
}

void edControlFTDI::WriteBuffer()
{
    if (!mIsPlugged) {
        return;
    }
    if (mWriteBytes > 0) {
        bulkWrite(mWriteBuf, mWriteBytes);
        mWriteBytes = 0;
    }
}

bool edControlFTDI::CheckConnectN8()
{
    FT_SetTimeouts(ftHandle, 100, 100);
    
    writeCmd("*t");
    UInt8 readBuff;
    bulkRead(&readBuff, 1);
    if (readBuff != 'k') {
        return false;
    }
    return true;
}

bool edControlFTDI::LoadFirmN8(const unsigned char *data, int len)
{
    int dataSize = len;
    if (len % 512 != 0) {
        len = (len / 512) * 512 + 512;
    }
    int buffLen = len+1;
    unsigned char *buff = new unsigned char[buffLen];
    for (int i=0; i<buffLen; i++) {
        buff[i] = 0xff;
    }
    for (int i=0; i<dataSize; i++) {
        buff[i+1] = data[i];
    }
    buff[0] = len / 512;
    
    FT_SetTimeouts(ftHandle, 1000, 1000);
    
    writeCmd("*s*f");
    txData(buff, buffLen);
    
    UInt8 readBuff;
    bulkRead(&readBuff, 1);
    assert(readBuff == 'k');
    
    FT_SetTimeouts(ftHandle, 6000, 6000);
    
    writeCmd("*r*t");
    bulkRead(&readBuff, 1);
    assert(readBuff == 'k');
    
    delete [] buff;
    
    return true;
}

int edControlFTDI::LoadRomN8(const unsigned char *data, int len, bool map_loaded)
{
    const UInt8   *hdr = data;
    if (hdr[0] != 'N' || hdr[1] != 'E' || hdr[2] != 'S') {
        return -1;
    }
    int map_cfg = (int)(hdr[6] & 0x0b);
    int mapper = (int)((hdr[6] >> 4) | (hdr[7] & 0xf0));
    
    std::cout << "Mapper: " << mapper << std::endl;
    std::cout << "PRG: " << static_cast<int>(hdr[4]) << std::endl;
    std::cout << "CHR: " << static_cast<int>(hdr[5]) << std::endl;
    
    int prgLen = hdr[4] * 16384;
    int chrLen = hdr[5] * 8192;
    UInt8 *prg = new UInt8[prgLen];
    UInt8 *chr = new UInt8[chrLen];
    
    memcpy(prg, &data[16], prgLen);
    memcpy(chr, &data[16+prgLen], chrLen);
    
    if (map_loaded == false)
    {
        lpadIntMapper(mapper);
    }
    if (prgLen > 512 * 1024) {
        std::cout << "PRG size is too big" << std::endl;
        delete [] prg;
        delete [] chr;
        return -1;
    }
    if (chrLen > 512 * 1024) {
        std::cout << "CHR size is too big" << std::endl;
        delete [] prg;
        delete [] chr;
        return -1;
    }
    
    FT_SetTimeouts(ftHandle, 2000, 4000);

    writeCmd("*g");
    UInt8 writeHdr[3];
    writeHdr[0] = prgLen / 16384;
    writeHdr[1] = chrLen / 8192;
    writeHdr[2] = map_cfg;
    bulkWrite(writeHdr, 3);
    
    txData(prg, prgLen);

    UInt8 readBuff;
    bulkRead(&readBuff, 1);
    assert(readBuff == 'k');
    
    txData(chr, chrLen);

    bulkRead(&readBuff, 1);
    assert(readBuff == 'k');
    
    delete [] prg;
    delete [] chr;

    return mapper;
}

bool edControlFTDI::LoadFdsN8(const unsigned char *data, int len, bool map_loaded)
{
    int prg_size;
    int offset = 16;
    
    if (map_loaded == false)
    {
        lpadIntMapper(254);
    }
    
    std::cout << "Load FDS" << std::endl;
    
    const UInt8   *hdr = data;
    len -= 16;
    if (hdr[0] != 'F' || hdr[1] != 'D' || hdr[2] != 'S') {
        len += 16;
        offset = 0;
    }
    prg_size = (int)(len > 65536 ? 8 : 4);
    
    int buffLen = prg_size * 16384;
    UInt8 *buff = new UInt8[buffLen];
    memcpy(buff, data+offset, buffLen>len?len:buffLen);
    
    if (buff[11] != 'H' || buff[12] != 'V' || buff[13] != 'C') {
        delete [] buff;
        return false;
    }
    
    writeCmd("*g");
    UInt8 writeHdr[3];
    writeHdr[0] = prg_size;
    writeHdr[1] = 0;
    writeHdr[2] = 0;
    bulkWrite(writeHdr, 3);
    
    txData(buff, buffLen);
    
    UInt8 readBuff;
    bulkRead(&readBuff, 1);
    assert(readBuff == 'k');
    bulkRead(&readBuff, 1);
    assert(readBuff == 'k');
    
    delete [] buff;
    
    return true;
}

void edControlFTDI::StartGame(int mapper)
{
    writeCmd("*r");
    UInt8 buff = mapper;
    bulkWrite(&buff, 1);
    UInt8 readBuff;
    bulkRead(&readBuff, 1);
    assert(readBuff == 'k');
}

void edControlFTDI::txData(unsigned char *data, int len)
{
    mWriteBytes = 0;
    for (int i=0; i<len; i++) {
        if (mWriteBytes == PACKET_SIZE) {
            WriteBuffer();
        }
        mWriteBuf[mWriteBytes] = data[i];
        mWriteBytes++;
    }
    WriteBuffer();
}

void edControlFTDI::lpadIntMapper(int mapper)
{
    FT_SetTimeouts(ftHandle, 2000, 4000);
    writeCmd("*m");
    UInt8 buff = mapper;
    bulkWrite(&buff, 1);
    UInt8 readBuff;
    bulkRead(&readBuff, 1);
    assert(readBuff == 'k');
}
