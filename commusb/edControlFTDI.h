//
//  edControlFTDI.h
//  gimicUsbSpcPlay
//
//  Created by osoumen on 2016/02/13.
//  Copyright (c) 2016年 osoumen. All rights reserved.
//

#ifndef __gimicUsbSpcPlay__edControlFTDI__
#define __gimicUsbSpcPlay__edControlFTDI__

#include <CoreFoundation/CoreFoundation.h>

typedef void	*FT_HANDLE;
typedef unsigned int	FT_STATUS;

class edControlFTDI {
public:
    edControlFTDI();
    virtual ~edControlFTDI();
    
    bool isPlugged() { return mIsPlugged; }
    virtual bool InitUsb();
    virtual bool CloseUsb();
    virtual int Init();
    virtual int Close();
    virtual void Reset();
    virtual void BlockWrite(int port, unsigned char data);
    virtual void WriteBuffer();

    bool CheckConnectN8();
    bool LoadFirmN8(const unsigned char *data, int len);
    int LoadRomN8(const unsigned char *data, int len, bool map_loaded=false);
    bool LoadFdsN8(const unsigned char *data, int len, bool map_loaded=false);
    void StartGame(int mapper);
    
private:
    static const int PACKET_SIZE = 64;
    
    unsigned char   mWriteBuf[64];
    int             mWriteBytes;
    
    volatile bool	mIsPlugged;
    
    FT_HANDLE       ftHandle;
    
    UInt8           mBuf[128];
    
    FT_STATUS	resetrPipe();
	FT_STATUS	resetwPipe();
    SInt32		bulkWrite(UInt8 *buf, UInt32 size);
    SInt32		bulkRead(UInt8 *buf, UInt32 size);
    void        writeCmd(const char *cmd);
    void        txData(unsigned char *data, int len);
    void        lpadIntMapper(int mapper);
};

#endif /* defined(__gimicUsbSpcPlay__edControlFTDI__) */
