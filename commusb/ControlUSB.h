/*
 *  ControlUSB.h
 *  VOPM
 *
 *  Created by osoumen on 2013/09/07.
 *  Copyright 2013 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#if __APPLE_CC__

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/usb/IOUSBLib.h>
#include <pthread.h>

class ControlUSB {
public:
	ControlUSB();
	virtual ~ControlUSB();
	
	void	BeginPortWait(int vendor, int product, int wpipe, int rpipe);
	void	EndPortWait();

	bool		isPlugged() { return mIsPlugged; }
	void		removeDevice();
	IOReturn	resetrPipe();
	IOReturn	resetwPipe();
	SInt32		bulkWrite(UInt8 *buf, UInt32 size);
	SInt32		bulkWriteAsync(UInt8 *buf, UInt32 size);
	SInt32		bulkRead(UInt8 *buf, UInt32 size, UInt32 timeout);
    SInt32      read(UInt8 *buf, UInt32 size);
    SInt32      getReadableBytes();
	
	static void NewDeviceAdded(void *refCon, io_iterator_t iterator);
	static void NewDeviceRemoved(void *refCon, io_iterator_t iterator);

    void		setDeviceAddedFunc( void (*func) (void* ownerClass), void* ownerClass );
	void		setDeviceRemovedFunc( void (*func) (void* ownerClass) , void* ownerClass );
    
private:
	static const int			WRITE_BUFFER_SIZE = 4096;
    static const int			READ_BUFFER_SIZE = 4096;
    static const int            READ_TIMEOUT = 100;
	
	bool	mIsRun;
	volatile bool	mIsPlugged;
	
	IONotificationPortRef		mNotifyPort;
	io_iterator_t				mNewDeviceAddedIter;
	io_iterator_t				mNewDeviceRemovedIter;
	CFRunLoopSourceRef          mAsyncRunLoopSource;
	
	IOUSBDeviceInterface300		**mDev;
	IOUSBInterfaceInterface300 	**mIntf;
	io_service_t				mUsbDevice;
	int                         mWPipe;
    int                         mRPipe;
    
	UInt8						mWriteBuffer[WRITE_BUFFER_SIZE];
	int							mWriteBufferPtr;
    UInt8						mReadBuffer[READ_BUFFER_SIZE];
	int							mReadBufferReadPtr;
    int							mReadBufferWritePtr;
    
    pthread_t                   mReadThread;
    
    void                        (*mDeviceAddedFunc) (void* ownerClass);
	void                        *mDeviceAddedFuncClass;
    void                        (*mDeviceRemovedFunc) (void* ownerClass);
	void                        *mDeviceRemovedFuncClass;
	
	IOReturn	configureDevice(IOUSBDeviceInterface300 **dev);
	IOReturn	controlWrite(IOUSBDeviceInterface300 **dev, UInt16 address, UInt16 length, UInt8 *data);
	IOReturn	findInterfaces(IOUSBDeviceInterface300 **dev);
	void		releaseAsyncRunLoopSource();
	
	void		printErr(IOReturn kr);

    static void *readThreadFunc(void *arg);
};

#else

#include <string>
#include <Windows.h>
#include <winusb.h>

class ControlUSB {
public:
	ControlUSB();
	virtual ~ControlUSB();
	
	void	BeginPortWait(LPGUID guid);
	void	EndPortWait();

	bool		isPlugged() { return mIsPlugged; }
	void		removeDevice();
	bool		resetrPipe();
	bool		resetwPipe();
	int			bulkWrite(UINT8 *buf, UINT32 size);
	int			bulkWriteAsync(UINT8 *buf, UINT32 size);
	int			bulkRead(UINT8 *buf, UINT32 size, UINT32 timeout);
	int		    read(UINT8 *buf, UINT32 size);
	int		    getReadableBytes();

	void		setDeviceAddedFunc(void(*func) (void* ownerClass), void* ownerClass);
	void		setDeviceRemovedFunc(void(*func) (void* ownerClass), void* ownerClass);

private:
	static const int			WRITE_BUFFER_SIZE = 4096;
	static const int			READ_BUFFER_SIZE = 4096;

	bool						mIsRun;
	volatile bool				mIsPlugged;

	HANDLE						m_hDev;
	WINUSB_INTERFACE_HANDLE		m_hWinUsb;
	std::basic_string<TCHAR>	mDevPath;
	UCHAR						mInPipeId;
	USHORT						mInPipeMaxPktSize;
	UCHAR						mOutPipeId;
	USHORT						mOutPipeMaxPktSize;


	UINT8						mWriteBuffer[WRITE_BUFFER_SIZE];
	int							mWriteBufferPtr;
	UINT8						mReadBuffer[READ_BUFFER_SIZE];
	int							mReadBufferReadPtr;
	int							mReadBufferWritePtr;

	void(*mDeviceAddedFunc)		(void* ownerClass);
	void                        *mDeviceAddedFuncClass;
	void(*mDeviceRemovedFunc)	(void* ownerClass);
	void                        *mDeviceRemovedFuncClass;

	bool openDevice(std::basic_string<TCHAR> devpath);
};
#endif
