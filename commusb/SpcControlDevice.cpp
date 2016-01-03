//
//  SpcControlDevice.cpp
//  gimicUsbSpcPlay
//
//  Created by osoumen on 2014/06/14.
//  Copyright (c) 2014年 osoumen. All rights reserved.
//

#include "SpcControlDevice.h"
#include <iomanip>
#include <iostream>
#ifdef _MSC_VER
#include <Windows.h>
// デバイスドライバのinf内で定義したGUID
// (WinUSB.sys使用デバイスに対する識別子）
// {63275336-530B-4069-92B6-5F8AE3465462}
DEFINE_GUID(GUID_DEVINTERFACE_WINUSBTESTTARGET,
	0x63275336, 0x530b, 0x4069, 0x92, 0xb6, 0x5f, 0x8a, 0xe3, 0x46, 0x54, 0x62);
#else
#include <unistd.h>
#endif

void printBytes(const unsigned char *data, int bytes)
{
    /*
    for (int i=0; i<bytes; i++) {
        std::cout << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]) << " ";
    }
    std::cout << std::endl;
     */
}

SpcControlDevice::SpcControlDevice()
{
    mUsbDev = new ControlUSB();
}

SpcControlDevice::~SpcControlDevice()
{
    delete mUsbDev;
}

int SpcControlDevice::Init()
{
    mWriteBytes = BLOCKWRITE_CMD_LEN;    // 0xFD,0xB2,0xNN分

#ifdef _MSC_VER
	mUsbDev->BeginPortWait((LPGUID)&GUID_DEVINTERFACE_WINUSBTESTTARGET);
#else
    mUsbDev->BeginPortWait(GIMIC_USBVID, GIMIC_USBPID, 1, 2);
#endif

#ifdef USB_CONSOLE_TEST
    int retryRemain = 100;
    while (!mUsbDev->isPlugged() && retryRemain > 0) {
#ifdef _MSC_VER
		::Sleep(10);
#else
        usleep(10000);
#endif
        retryRemain--;
    }
    if (!mUsbDev->isPlugged()) {
        return 1;
    }
#endif
    return 0;
}

int SpcControlDevice::Close()
{
    mUsbDev->removeDevice();
    return 0;
}

void SpcControlDevice::HwReset()
{
    mUsbDev->resetrPipe();
    mUsbDev->resetwPipe();
    
    unsigned char cmd[] = {0xfd, 0x81, 0xff};
    int wb = sizeof(cmd);
    mUsbDev->bulkWrite(cmd, wb);
    
    printBytes(cmd, wb);
}

void SpcControlDevice::SwReset()
{
    unsigned char cmd[] = {0xfd, 0x82, 0xff};
    int wb = sizeof(cmd);
    mUsbDev->bulkWrite(cmd, wb);
    
    printBytes(cmd, wb);
}

void SpcControlDevice::PortWrite(int addr, unsigned char data)
{
    unsigned char cmd[] = {0x00, 0x00, 0xff};
    cmd[0] = addr;
    cmd[1] = data;
    int wb = sizeof(cmd);
    mUsbDev->bulkWrite(cmd, wb);
    
    printBytes(cmd, wb);
}

unsigned char SpcControlDevice::PortRead(int addr)
{
    unsigned char cmd[] = {0xfd, 0xb0, 0x00, 0x00, 0xff};
    cmd[2] = addr;
    int wb = sizeof(cmd);
    mUsbDev->bulkWrite(cmd, wb);
    printBytes(cmd, wb);
    
    int rb = 64;
#if 0
    int retry = 500;
    while (mUsbDev->getReadableBytes() < rb) {
        usleep(1000);
        retry--;
        if (retry == 0) {
            break;
        }
    }
    if (retry > 0) {
        mUsbDev->read(mReadBuf, rb);
    }
#else
    mUsbDev->bulkRead(mReadBuf, rb, 500);
    //std::cout << ">";
    printBytes(mReadBuf, 1);
#endif
    return mReadBuf[0];
}

void SpcControlDevice::BlockWrite(int addr, unsigned char data)
{
    // 残り2バイト未満なら書き込んでから追加する
    if (mWriteBytes > (PACKET_SIZE-2)) {
        WriteBuffer();
    }
    mWriteBuf[mWriteBytes] = addr & 0x03;
    mWriteBytes++;
    mWriteBuf[mWriteBytes] = data;
    mWriteBytes++;
}

void SpcControlDevice::BlockWrite(int addr, unsigned char data, unsigned char data2)
{
    // 残り3バイト未満なら書き込んでから追加する
    if (mWriteBytes > (PACKET_SIZE-3)) {
        WriteBuffer();
    }
    mWriteBuf[mWriteBytes] = (addr & 0x03) | 0x10;
    mWriteBytes++;
    mWriteBuf[mWriteBytes] = data;
    mWriteBytes++;
    mWriteBuf[mWriteBytes] = data2;
    mWriteBytes++;
}

void SpcControlDevice::BlockWrite(int addr, unsigned char data, unsigned char data2, unsigned char data3)
{
    // 残り4バイト未満なら書き込んでから追加する
    if (mWriteBytes > (PACKET_SIZE-4)) {
        WriteBuffer();
    }
    mWriteBuf[mWriteBytes] = (addr & 0x03) | 0x20;
    mWriteBytes++;
    mWriteBuf[mWriteBytes] = data;
    mWriteBytes++;
    mWriteBuf[mWriteBytes] = data2;
    mWriteBytes++;
    mWriteBuf[mWriteBytes] = data3;
    mWriteBytes++;
}

void SpcControlDevice::BlockWrite(int addr, unsigned char data, unsigned char data2, unsigned char data3, unsigned char data4)
{
    // 残り5バイト未満なら書き込んでから追加する
    if (mWriteBytes > (PACKET_SIZE-5)) {
        WriteBuffer();
    }
    mWriteBuf[mWriteBytes] = (addr & 0x03) | 0x30;
    mWriteBytes++;
    mWriteBuf[mWriteBytes] = data;
    mWriteBytes++;
    mWriteBuf[mWriteBytes] = data2;
    mWriteBytes++;
    mWriteBuf[mWriteBytes] = data3;
    mWriteBytes++;
    mWriteBuf[mWriteBytes] = data4;
    mWriteBytes++;
}

void SpcControlDevice::ReadAndWait(int addr, unsigned char waitValue)
{
    if (mWriteBytes > (PACKET_SIZE-2)) {
        WriteBuffer();
    }
    mWriteBuf[mWriteBytes] = addr | 0x80;
    mWriteBytes++;
    mWriteBuf[mWriteBytes] = waitValue;
    mWriteBytes++;
}

void SpcControlDevice::WriteAndWait(int addr, unsigned char waitValue)
{
    if (mWriteBytes > (PACKET_SIZE-2)) {
        WriteBuffer();
    }
    mWriteBuf[mWriteBytes] = addr | 0xc0;
    mWriteBytes++;
    mWriteBuf[mWriteBytes] = waitValue;
    mWriteBytes++;
}

void SpcControlDevice::WriteBuffer()
{
    if (!mUsbDev->isPlugged()) {
        return;
    }
    /*
    if (mWriteBytes > 62) {
        // TODO: Assert
        return;
    }
    */
    if (mWriteBytes > BLOCKWRITE_CMD_LEN) {
        mWriteBuf[0] = 0xfd;
        mWriteBuf[1] = 0xb2;
        mWriteBuf[2] = 0x00;
        mWriteBuf[3] = 0x00;
        for (int i=0; i<1; i++) {
            if (mWriteBytes < 64) {
                mWriteBuf[mWriteBytes] = 0xff;
                mWriteBytes++;
            }
        }
        /*
        puts("\n--Dump--");
        for (int i=3; i<64; i+=2) {
            int blockaddr = mWriteBuf[i];
            int blockdata = mWriteBuf[i+1];
            printf("Block : 0x%02X / 0x%02X\n", blockaddr, blockdata);
            if (blockaddr == 0xFF && blockdata == 0xFF)break;
        }
         */
        //printf("mWriteBytes:%d\n", mWriteBytes);
        // GIMIC側のパケットは64バイト固定なので満たない場合0xffを末尾に追加
        if (mWriteBytes < 64) {
            mWriteBuf[mWriteBytes++] = 0xff;
        }
        //if (mWriteBuf[6] == 0x7d && mWriteBuf[7] == 0xc0) {
            printBytes(mWriteBuf, mWriteBytes);
        //}
        mUsbDev->bulkWrite(mWriteBuf, mWriteBytes);
        mWriteBytes = BLOCKWRITE_CMD_LEN;
    }
}

void SpcControlDevice::WriteBufferAsync()
{
    if (!mUsbDev->isPlugged()) {
        return;
    }
    if (mWriteBytes > BLOCKWRITE_CMD_LEN) {
        mWriteBuf[0] = 0xfd;
        mWriteBuf[1] = 0xb2;
        mWriteBuf[2] = 0x00;
        mWriteBuf[3] = 0x00;
        for (int i=0; i<1; i++) {
            if (mWriteBytes < 64) {
                mWriteBuf[mWriteBytes] = 0xff;
                mWriteBytes++;
            }
        }
        // GIMIC側のパケットは64バイト固定なので満たない場合0xffを末尾に追加
        if (mWriteBytes < 64) {
            mWriteBuf[mWriteBytes++] = 0xff;
        }
        //if (mWriteBuf[6] == 0x7d && mWriteBuf[7] == 0xc0) {
            printBytes(mWriteBuf, mWriteBytes);
        //}
        mUsbDev->bulkWriteAsync(mWriteBuf, mWriteBytes);
        mWriteBytes = BLOCKWRITE_CMD_LEN;
    }
}

int SpcControlDevice::CatchTransferError()
{
    if (mUsbDev->getReadableBytes() >= 4) {
        unsigned char msg[4];
        mUsbDev->read(msg, 4);
        int err = *(reinterpret_cast<unsigned int*>(msg));
        if (err == 0xfefefefe) {
            return err;
        }
    }
    return 0;
}

void SpcControlDevice::setDeviceAddedFunc( void (*func) (void* ownerClass), void* ownerClass )
{
    mUsbDev->setDeviceAddedFunc(func, ownerClass);
}

void SpcControlDevice::setDeviceRemovedFunc( void (*func) (void* ownerClass) , void* ownerClass )
{
    mUsbDev->setDeviceRemovedFunc(func, ownerClass);
}

//-----------------------------------------------------------------------------

int SpcControlDevice::WaitReady()
{
    if (mUsbDev->isPlugged()) {
        ReadAndWait(0, 0xaa);
        ReadAndWait(1, 0xbb);
        WriteBuffer();
    }
    
    int err = CatchTransferError();
    if (err) {
        return err;
    }
    return 0;
}

int SpcControlDevice::UploadRAMDataIPL(const unsigned char *ram, int addr, int size, unsigned char initialP0state)
{
    BlockWrite(1, 0x01, addr & 0xff, (addr >> 8) & 0xff); // 非0なのでP2,P3は書き込み開始アドレス
    unsigned char port0State = initialP0state;
    WriteAndWait(0, port0State&0xff);
    WriteBuffer();
    port0State = 0;
    for (int i=0; i<size; i++) {
        BlockWrite(1, ram[i]);
        WriteAndWait(0, port0State);
        port0State++;
        if ((i % 256) == 255) {
            //std::cout << ".";
        }
        if (i == (size-1)) {
            WriteBuffer();
        }
        int err = CatchTransferError();
        if (err) {
            return err;
        }
    }
    return port0State;
}

int SpcControlDevice::JumpToCode(int addr, unsigned char initialP0state)
{
    BlockWrite(2, addr & 0xff, (addr >> 8) & 0xff);
    BlockWrite(1, 0);    // 0なのでP2,P3はジャンプ先アドレス
    unsigned char port0state = initialP0state & 0xff;
    BlockWrite(0, port0state);
    WriteBuffer();
    
    int err = CatchTransferError();
    if (err) {
        return err;
    }
    return 0;
}