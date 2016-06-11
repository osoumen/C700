//
//  SpcControlFTDI.cpp
//  gimicUsbSpcPlay
//
//  Created by osoumen on 2016/02/13.
//  Copyright (c) 2016年 osoumen. All rights reserved.
//

#include "SpcControlFTDI.h"
#include <iostream>

#ifdef _MSC_VER
#pragma comment(lib, "ftd2xx.lib")
#endif

SpcControlFTDI::SpcControlFTDI()
{
    mDeviceAddedFunc = NULL;
    mDeviceAddedFuncClass = NULL;
    mDeviceRemovedFunc = NULL;
    mDeviceRemovedFuncClass = NULL;
}

bool SpcControlFTDI::InitUsb()
{
    return true;
}

bool SpcControlFTDI::CloseUsb()
{
    if (mIsPlugged) {
		FT_Close(ftHandle);
		mIsPlugged = false;
	}
    return true;
}

int SpcControlFTDI::Init()
{
    mWriteBytes = 0;
    
    FT_STATUS	ftStatus;
    
#if 0
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
		return;
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
    if (mDeviceAddedFunc) {
		mDeviceAddedFunc(mDeviceAddedFuncClass);
	}
    
    return 0;
}

int SpcControlFTDI::Close()
{
    CloseUsb();
    return 0;
}

FT_STATUS	SpcControlFTDI::resetrPipe()
{
	if (!mIsPlugged) return -1;
	
	FT_STATUS kr = 0;
	
	kr = FT_Purge(ftHandle, FT_PURGE_RX);
	
	return kr;
}

FT_STATUS	SpcControlFTDI::resetwPipe()
{
	if (!mIsPlugged) return -1;
	
	FT_STATUS kr = 0;
	
	kr = FT_Purge(ftHandle, FT_PURGE_TX);
	
	return kr;
}

void SpcControlFTDI::Reset()
{
    resetrPipe();
    resetwPipe();
    // リセットできません
}

bool SpcControlFTDI::CheckHasRequiredModule()
{
    return true;
}

int		SpcControlFTDI::bulkWrite(unsigned char *buf, unsigned int size)
{
	if (!mIsPlugged) return -1;
	FT_STATUS			kr = 0;
    DWORD               len;
    
    int      writeSize = size;
    for (int i=0; i<writeSize; i++) {
        mBuf[i] = buf[i];
    }
    kr = FT_Write(ftHandle, mBuf, writeSize, &len);
	
	return len;
}

int		SpcControlFTDI::bulkRead(unsigned char *buf, unsigned int size)
{
	if (!mIsPlugged) return -1;
	FT_STATUS			kr = 0;
    DWORD               len;
    
    int      readSize = size;
    kr = FT_Read(ftHandle, buf, readSize, &len);
	
	return len;
}

void SpcControlFTDI::PortWrite(int addr, unsigned char data)
{
    unsigned char cmd[2];
    cmd[0] = addr;
    cmd[1] = data;
    bulkWrite(cmd, 2);
}

unsigned char SpcControlFTDI::PortRead(int addr)
{
    return 0;
}

void SpcControlFTDI::BlockWrite(int addr, unsigned char data)
{
    if (mWriteBytes > (PACKET_SIZE-2)) {
        WriteBuffer();
    }
    mWriteBuf[mWriteBytes] = (addr & 0x03) | 0x80;
    mWriteBytes++;
    mWriteBuf[mWriteBytes] = data;
    mWriteBytes++;
}

void SpcControlFTDI::BlockWrite(int addr, unsigned char data, unsigned char data2)
{
    if (mWriteBytes > (PACKET_SIZE-4)) {
        WriteBuffer();
    }
    mWriteBuf[mWriteBytes] = (addr & 0x03) | 0x80;
    mWriteBytes++;
    mWriteBuf[mWriteBytes] = data;
    mWriteBytes++;
    mWriteBuf[mWriteBytes] = ((addr+1) & 0x03) | 0x80;
    mWriteBytes++;
    mWriteBuf[mWriteBytes] = data2;
    mWriteBytes++;
}

void SpcControlFTDI::BlockWrite(int addr, unsigned char data, unsigned char data2, unsigned char data3)
{
    if (mWriteBytes > (PACKET_SIZE-6)) {
        WriteBuffer();
    }
    mWriteBuf[mWriteBytes] = (addr & 0x03) | 0x80;
    mWriteBytes++;
    mWriteBuf[mWriteBytes] = data;
    mWriteBytes++;
    mWriteBuf[mWriteBytes] = ((addr+1) & 0x03) | 0x80;
    mWriteBytes++;
    mWriteBuf[mWriteBytes] = data2;
    mWriteBytes++;
    mWriteBuf[mWriteBytes] = ((addr+2) & 0x03) | 0x80;
    mWriteBytes++;
    mWriteBuf[mWriteBytes] = data3;
    mWriteBytes++;
}

void SpcControlFTDI::BlockWrite(int addr, unsigned char data, unsigned char data2, unsigned char data3, unsigned char data4)
{
    if (mWriteBytes > (PACKET_SIZE-8)) {
        WriteBuffer();
    }
    mWriteBuf[mWriteBytes] = (addr & 0x03) | 0x80;
    mWriteBytes++;
    mWriteBuf[mWriteBytes] = data;
    mWriteBytes++;
    mWriteBuf[mWriteBytes] = ((addr+1) & 0x03) | 0x80;
    mWriteBytes++;
    mWriteBuf[mWriteBytes] = data2;
    mWriteBytes++;
    mWriteBuf[mWriteBytes] = ((addr+2) & 0x03) | 0x80;
    mWriteBytes++;
    mWriteBuf[mWriteBytes] = data3;
    mWriteBytes++;
    mWriteBuf[mWriteBytes] = ((addr+3) & 0x03) | 0x80;
    mWriteBytes++;
    mWriteBuf[mWriteBytes] = data4;
    mWriteBytes++;
}

void SpcControlFTDI::ReadAndWait(int addr, unsigned char waitValue)
{
    if (mWriteBytes > (PACKET_SIZE-2)) {
        WriteBuffer();
    }
    mWriteBuf[mWriteBytes] = addr | 0x40;
    mWriteBytes++;
    mWriteBuf[mWriteBytes] = waitValue;
    mWriteBytes++;
}

void SpcControlFTDI::WriteAndWait(int addr, unsigned char waitValue)
{
    if (mWriteBytes > (PACKET_SIZE-2)) {
        WriteBuffer();
    }
    mWriteBuf[mWriteBytes] = addr | 0xc0;
    mWriteBytes++;
    mWriteBuf[mWriteBytes] = waitValue;
    mWriteBytes++;
}

void SpcControlFTDI::WriteBuffer()
{
    if (!mIsPlugged) {
        return;
    }
    if (mWriteBytes > 0) {
        bulkWrite(mWriteBuf, mWriteBytes);
        mWriteBytes = 0;
    }
}

int SpcControlFTDI::CatchTransferError()
{
    return 0;
}

void SpcControlFTDI::setDeviceAddedFunc( void (*func) (void* ownerClass), void* ownerClass )
{
    mDeviceAddedFunc = func;
    mDeviceAddedFuncClass = ownerClass;
}

void SpcControlFTDI::setDeviceRemovedFunc( void (*func) (void* ownerClass) , void* ownerClass )
{
    mDeviceRemovedFunc = func;
    mDeviceRemovedFuncClass = ownerClass;
}
