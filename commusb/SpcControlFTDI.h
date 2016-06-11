//
//  SpcControlFTDI.h
//  gimicUsbSpcPlay
//
//  Created by osoumen on 2016/02/13.
//  Copyright (c) 2016年 osoumen. All rights reserved.
//

#ifndef __gimicUsbSpcPlay__SpcControlFTDI__
#define __gimicUsbSpcPlay__SpcControlFTDI__

#include "SpcControlDevice.h"

#include "ftd2xx.h"

class SpcControlFTDI : public SpcControlDevice {
public:
    SpcControlFTDI();
    virtual ~SpcControlFTDI() {}
    
    virtual bool InitUsb();
    virtual bool CloseUsb();
    virtual int Init();
    virtual int Close();
    virtual void Reset();
    virtual bool CheckHasRequiredModule();
    virtual void PortWrite(int addr, unsigned char data);
    virtual unsigned char PortRead(int addr);
    virtual void BlockWrite(int addr, unsigned char data);
    virtual void BlockWrite(int addr, unsigned char data, unsigned char data2);
    virtual void BlockWrite(int addr, unsigned char data, unsigned char data2, unsigned char data3);
    virtual void BlockWrite(int addr, unsigned char data, unsigned char data2, unsigned char data3, unsigned char data4);
    virtual void ReadAndWait(int addr, unsigned char waitValue);
    virtual void WriteAndWait(int addr, unsigned char waitValue);
    virtual void WriteBuffer();
    virtual int CatchTransferError();

    int		bulkWrite(unsigned char *buf, unsigned int size);
    int		bulkRead(unsigned char *buf, unsigned int size);
    
    virtual void setDeviceAddedFunc( void (*func) (void* ownerClass), void* ownerClass );
	virtual void setDeviceRemovedFunc( void (*func) (void* ownerClass) , void* ownerClass );
    
private:
    static const int PACKET_SIZE = 64;
    
    unsigned char   mWriteBuf[64];
    int             mWriteBytes;
    
    volatile bool	mIsPlugged;
    
    FT_HANDLE       ftHandle;
    
    unsigned char   mBuf[128];
    
    void(*mDeviceAddedFunc)		(void* ownerClass);
	void                        *mDeviceAddedFuncClass;
	void(*mDeviceRemovedFunc)	(void* ownerClass);
	void                        *mDeviceRemovedFuncClass;
    
    //void		printErr(IOReturn kr);
    
    FT_STATUS	resetrPipe();
	FT_STATUS	resetwPipe();
};

#endif /* defined(__gimicUsbSpcPlay__SpcControlFTDI__) */
