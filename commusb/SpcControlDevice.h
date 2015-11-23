//
//  SpcControlDevice.h
//  gimicUsbSpcPlay
//
//  Created by osoumen on 2014/06/14.
//  Copyright (c) 2014年 osoumen. All rights reserved.
//

#ifndef __gimicUsbSpcPlay__SpcControlDevice__
#define __gimicUsbSpcPlay__SpcControlDevice__

#include "ControlUSB.h"

class SpcControlDevice {
public:
    SpcControlDevice();
    ~SpcControlDevice();
    
    int Init();
    int Close();
    void HwReset();
    void SwReset();
    void PortWrite(int addr, unsigned char data);
    unsigned char PortRead(int addr);
    void BlockWrite(int addr, unsigned char data);
    void BlockWrite(int addr, unsigned char data, unsigned char data2);
    void BlockWrite(int addr, unsigned char data, unsigned char data2, unsigned char data3);
    void BlockWrite(int addr, unsigned char data, unsigned char data2, unsigned char data3, unsigned char data4);
    void ReadAndWait(int addr, unsigned char waitValue);
    void WriteAndWait(int addr, unsigned char waitValue);
    void WriteBuffer();
    void WriteBufferAsync();
    
    int UploadRAMDataIPL(const unsigned char *ram, int addr, int size, unsigned char initialP0state);
    int WaitReady();
    int JumpToCode(int addr, unsigned char initialP0state);
    
    int CatchTransferError();
    
    void setDeviceAddedFunc( void (*func) (void* ownerClass), void* ownerClass );
	void setDeviceRemovedFunc( void (*func) (void* ownerClass) , void* ownerClass );

private:
    static const int GIMIC_USBVID = 0x16c0;
    static const int GIMIC_USBPID = 0x05e5;
    static const int GIMIC_USBWPIPE = 0x02;
    static const int GIMIC_USBRPIPE = 0x85;
    static const int BLOCKWRITE_CMD_LEN = 4;
    static const int MAX_ASYNC_READ = 64;
    static const int PACKET_SIZE = 64;

    ControlUSB      *mUsbDev;
    unsigned char   mWriteBuf[64];
    unsigned char   mReadBuf[64];
    int             mWriteBytes;
    
    //int mNumReads;  // Read待ちのパケット数
};

#endif /* defined(__gimicUsbSpcPlay__SpcControlDevice__) */
