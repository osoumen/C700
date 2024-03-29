/*
 *  ControlUSB.cpp
 *  VOPM
 *
 *  Created by osoumen on 2013/09/07.
 *  Copyright 2013 __MyCompanyName__. All rights reserved.
 *
 */

#include "ControlUSB.h"

#if __APPLE_CC__

#include <stdio.h>
#include <unistd.h>
#include <mach/mach.h>

#ifdef DEBUG_PRINT
static int sNumInstances = 0;	//デバッグ用
#endif

ControlUSB::ControlUSB()
: mIsRun(false)
, mIsPlugged(false)
, mNotifyPort(NULL)
, mNewDeviceAddedIter(NULL)
, mNewDeviceRemovedIter(NULL)
, mAsyncRunLoopSource(NULL)
, mDev(NULL)
, mUsbDevice(-1)
, mWriteBufferPtr(0)
{
    mReadBufferReadPtr = 0;
    mReadBufferWritePtr = 0;
    mDeviceAddedFunc = NULL;
    mDeviceRemovedFunc = NULL;
#ifdef DEBUG_PRINT
	sNumInstances++;
#endif
}

ControlUSB::~ControlUSB()
{
	removeDevice();
	EndPortWait();
#ifdef DEBUG_PRINT
	sNumInstances--;
#endif
}

void	ControlUSB::removeDevice()
{
	if (mIsPlugged) {
		(*mDev)->USBDeviceClose(mDev);
		(*mDev)->Release(mDev);
		mIsPlugged = false;
		releaseAsyncRunLoopSource();
	}
}

void	ControlUSB::releaseAsyncRunLoopSource()
{
	if (mAsyncRunLoopSource) {
		CFRunLoopSourceInvalidate(mAsyncRunLoopSource);
		CFRunLoopRemoveSource(CFRunLoopGetCurrent(), mAsyncRunLoopSource,
							  kCFRunLoopDefaultMode);
		CFRelease(mAsyncRunLoopSource);
		mAsyncRunLoopSource = NULL;
	}
}

void	ControlUSB::BeginPortWait(int vendor, int product, int wpipe, int rpipe)
{
    mWPipe = wpipe;
    mRPipe = rpipe;
    
	mach_port_t				masterPort    = 0;
	CFMutableDictionaryRef  matchingDict  = 0;
	CFRunLoopSourceRef		runLoopSource = 0;
	kern_return_t			result;
	
	mNotifyPort           = 0;
	mNewDeviceAddedIter   = 0;
	mNewDeviceRemovedIter = 0;
	
	// IOMasterPortを取得する。
	result = IOMasterPort(MACH_PORT_NULL, &masterPort);
	if (result || !masterPort) goto bail;
	
	// デバイスのマッチング用の辞書を作成する。
	matchingDict = IOServiceMatching(kIOUSBDeviceClassName);
	if (!matchingDict) goto bail;
	
	// Notification Portを生成し、それをRun Loop Sourceへ登録する。
	mNotifyPort   = IONotificationPortCreate(masterPort);
	runLoopSource = IONotificationPortGetRunLoopSource(mNotifyPort);
	CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopDefaultMode);
	
	// 辞書にファームウェアのダウンロード後のベンダーIDとプロダクトIDを登録する。
	CFDictionarySetValue(matchingDict, 
						 CFSTR(kUSBVendorID), 
						 CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &vendor)); 
	CFDictionarySetValue(matchingDict, 
						 CFSTR(kUSBProductID), 
						 CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &product)); 
	
	matchingDict = (CFMutableDictionaryRef)CFRetain(matchingDict); 
	matchingDict = (CFMutableDictionaryRef)CFRetain(matchingDict); 
	
	// ノーティフィケーションを設定する。
	result = IOServiceAddMatchingNotification(mNotifyPort,
											  kIOFirstMatchNotification,
											  matchingDict,
											  NewDeviceAdded,
											  (void*)this,
											  &mNewDeviceAddedIter);
	result = IOServiceAddMatchingNotification(mNotifyPort,
											  kIOTerminatedNotification,
											  matchingDict,
											  NewDeviceRemoved,
											  (void*)this,
											  &mNewDeviceRemovedIter);
	NewDeviceRemoved((void*)this, mNewDeviceRemovedIter);
	NewDeviceAdded((void*)this, mNewDeviceAddedIter);
	
	// IOMasterPortを破棄する。
	mach_port_deallocate(mach_task_self(), masterPort);
	masterPort = 0;
	
	mIsRun = true;
	return;
	
bail:
	if (masterPort) {
		mach_port_deallocate(mach_task_self(), masterPort);
		masterPort = 0;
	}
	mIsRun = false;
}

void	ControlUSB::EndPortWait()
{
	if (mNotifyPort) {
		IONotificationPortDestroy(mNotifyPort);
		mNotifyPort = 0;
	}
    if (mNewDeviceAddedIter) {
        IOObjectRelease(mNewDeviceAddedIter);
        mNewDeviceAddedIter = 0;
    }
    if (mNewDeviceRemovedIter) {
        IOObjectRelease(mNewDeviceRemovedIter);
        mNewDeviceRemovedIter = 0;
    }
	mIsRun = false;
}

IOReturn	ControlUSB::resetrPipe()
{
	if (!mIsPlugged) return -1;
	
	IOReturn			kr = noErr;
	
	kr = (*mIntf)->ResetPipe(mIntf,mRPipe);
	
	return kr;
}

IOReturn	ControlUSB::resetwPipe()
{
	if (!mIsPlugged) return -1;

	IOReturn			kr = noErr;
	
	kr = (*mIntf)->ResetPipe(mIntf,mWPipe);
	
	return kr;
}

SInt32		ControlUSB::bulkWrite(UInt8 *buf, UInt32 size)
{
	if (!mIsPlugged) return -1;
	IOReturn			kr = noErr;
	UInt32				len = size;
	
	UInt32				bufferRest = WRITE_BUFFER_SIZE - mWriteBufferPtr;
	if (bufferRest < len) {
		mWriteBufferPtr = 0;
	}
	memcpy(&mWriteBuffer[mWriteBufferPtr], buf, len);
	
	kr = (*mIntf)->WritePipe(mIntf, mWPipe, &mWriteBuffer[mWriteBufferPtr], len);
	if (kr != noErr) {
		//printf("Write error in bulkWrite.\n");
		printErr(kr);
		return -1;
	}
	
	mWriteBufferPtr += len;
	return 0;
}

SInt32		ControlUSB::bulkWriteAsync(UInt8 *buf, UInt32 size)
{
	if (!mIsPlugged) return -1;

	IOReturn			kr = noErr;
	UInt32				len = size;
	
	UInt32				bufferRest = WRITE_BUFFER_SIZE - mWriteBufferPtr;
	if (bufferRest < len) {
		mWriteBufferPtr = 0;
	}
	memcpy(&mWriteBuffer[mWriteBufferPtr], buf, len);
	
	kr = (*mIntf)->WritePipeAsync(mIntf, mWPipe, &mWriteBuffer[mWriteBufferPtr], len, NULL, NULL);
	if (kr != noErr) {
		//printf("Write error in bulkWrite.\n");
		printErr(kr);
		return -1;
	}
	
	mWriteBufferPtr += len;
	return 0;
}

#if 1
SInt32		ControlUSB::bulkRead(UInt8 *buf, UInt32 size, UInt32 timeout)
{
	if (!mIsPlugged) return -1;

	IOReturn			kr = noErr;
	UInt32				len = size;
	
	kr = (*mIntf)->ReadPipeTO(mIntf, mRPipe, buf, &len, timeout, timeout);
	if (kr != noErr) {
		//printf("Read error in bulkRead.\n");
		printErr(kr);
		return 0;
	}
	return len;
}
#endif

SInt32      ControlUSB::read(UInt8 *buf, UInt32 size)
{
    SInt32 bytes = getReadableBytes();
    if (bytes == 0) {
        return 0;
    }
    if (bytes > size) {
        bytes = size;
    }
    for (int i=0; i<bytes; i++) {
        buf[i] = mReadBuffer[mReadBufferReadPtr];
        mReadBufferReadPtr = (mReadBufferReadPtr + 1) % READ_BUFFER_SIZE;
    }
    return bytes;
}

SInt32      ControlUSB::getReadableBytes()
{
    return ((mReadBufferWritePtr + READ_BUFFER_SIZE) - mReadBufferReadPtr) % READ_BUFFER_SIZE;
}

IOReturn	ControlUSB::controlWrite(IOUSBDeviceInterface300 **dev, UInt16 address, UInt16 length, UInt8 *data)
{
	if (!mIsPlugged) return -1;

	IOUSBDevRequest request;
    request.bmRequestType = USBmakebmRequestType(kUSBOut, kUSBVendor, kUSBDevice);
    request.bRequest      = 0xa0;
    request.wValue        = address;
    request.wIndex        = 0;
    request.wLength       = length;
    request.pData         = data;
    return (*dev)->DeviceRequest(dev, &request);
}

IOReturn	ControlUSB::configureDevice(IOUSBDeviceInterface300 **dev)
{
	UInt8							numConf;
    IOReturn						kr;
    IOUSBConfigurationDescriptorPtr	confDesc;
    
    kr = (*dev)->GetNumberOfConfigurations(dev, &numConf);
    if(!numConf) {
        return -1;
	}
	//printf("Device has %d configurations\n", numConf);
    
    // コンフィグレーション・ディスクリプタを取得する。
    kr = (*dev)->GetConfigurationDescriptorPtr(dev, 0, &confDesc);
    if(kr) {
        return kr;
    }
	kr = (*dev)->SetConfiguration(dev, confDesc->bConfigurationValue);
	if (kr) {
		printErr(kr);
		return kr;
    }
    return kIOReturnSuccess;
}

IOReturn	ControlUSB::findInterfaces(IOUSBDeviceInterface300 **dev)
{
	IOReturn					kr;
    IOUSBFindInterfaceRequest	request;
    io_iterator_t				iterator;
    io_service_t				usbInterface;
    IOCFPlugInInterface			**plugInInterface = NULL;
    IOUSBInterfaceInterface300 	**intf = NULL;
    HRESULT						res;
    SInt32						score;
    
    request.bInterfaceClass    = kIOUSBFindInterfaceDontCare;
    request.bInterfaceSubClass = kIOUSBFindInterfaceDontCare;
    request.bInterfaceProtocol = kIOUSBFindInterfaceDontCare;
    request.bAlternateSetting  = kIOUSBFindInterfaceDontCare;
	
    kr = (*dev)->CreateInterfaceIterator(dev, &request, &iterator);
    
    while((usbInterface = IOIteratorNext(iterator))) {
        kr = IOCreatePlugInInterfaceForService(usbInterface, kIOUSBInterfaceUserClientTypeID, kIOCFPlugInInterfaceID, &plugInInterface, &score);
        kr = IOObjectRelease(usbInterface);
        if((kIOReturnSuccess != kr) || !plugInInterface) {
            break;
        }
		
        // デバイス・インターフェースから、さらにインターフェースを取得する
        res = (*plugInInterface)->QueryInterface(plugInInterface, CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID300), (LPVOID*) &intf);
        (*plugInInterface)->Release(plugInInterface);
        if(res || !intf) {
            break;
        }
		mIntf = intf;
		
		// インターフェースをオープンする
        kr = (*intf)->USBInterfaceOpen(intf);
        if(kIOReturnSuccess != kr) {
			
			//printf("USBInterfaceOpen error.\n");
			printErr(kr);
			
            (void) (*intf)->Release(intf);
            continue;
        }
		
		// インターフェースに存在するエンドポイント数を得る
		UInt8                       interfaceNumEndpoints;
        kr = (*intf)->GetNumEndpoints(intf, &interfaceNumEndpoints);
        if (kr != kIOReturnSuccess) {
            printf("Unable to get number of endpoints (%08x)\n", kr);
            (void) (*intf)->USBInterfaceClose(intf);
            (void) (*intf)->Release(intf);
            break;
        }
#ifdef DEBUG_PRINT
		printf("Interface has %d endpoints\n", interfaceNumEndpoints);
#endif
		
		int		pipeRef;
		for (pipeRef = 1; pipeRef <= interfaceNumEndpoints; pipeRef++)
        {
            IOReturn        kr2;
            UInt8           direction;
            UInt8           number;
            UInt8           transferType;
            UInt16          maxPacketSize;
            UInt8           interval;
			
            kr2 = (*intf)->GetPipeProperties(intf,
											 pipeRef, &direction,
											 &number, &transferType,
											 &maxPacketSize, &interval);
			
#ifdef DEBUG_PRINT
            const char            *message;
            if (kr2 != kIOReturnSuccess)
                printf("Unable to get properties of pipe %d (%08x)\n",
					   pipeRef, kr2);
            else
            {
                printf("PipeRef %d: ", pipeRef);
                switch (direction)
                {
					case kUSBOut:
					{
						const char msg_str[] = "out";
						message = msg_str;
						break;
					}
					case kUSBIn:
					{
						const char msg_str[] = "in";
						message = msg_str;
						break;
					}
					case kUSBNone:
					{
						const char msg_str[] = "none";
						message = msg_str;
						break;
					}
					case kUSBAnyDirn:
					{
						const char msg_str[] = "any";
						message = msg_str;
						break;
					}
					default:
					{
						const char msg_str[] = "???";
						message = msg_str;
					}
				}
				printf("direction %s, ", message);
				
				switch (transferType)
				{
					case kUSBControl:
					{
						const char msg_str[] = "control";
						message = msg_str;
						break;
					}
					case kUSBIsoc:
					{
						const char msg_str[] = "isoc";
						message = msg_str;
						break;
					}
					case kUSBBulk:
					{
						const char msg_str[] = "bulk";
						message = msg_str;
						break;
					}
					case kUSBInterrupt:
					{
						const char msg_str[] = "interrupt";
						message = msg_str;
						break;
					}
					case kUSBAnyType:
					{
						const char msg_str[] = "any";
						message = msg_str;
						break;
					}
					default:
					{
						const char msg_str[] = "???";
						message = msg_str;
					}
                }
                printf("transfer type %s, maxPacketSize %d\n", message,
					   maxPacketSize);
            }
#endif
		}
		
		//非同期動作用のランループソースを登録する
		kr = (*intf)->CreateInterfaceAsyncEventSource(intf, &mAsyncRunLoopSource);
		if (kr != kIOReturnSuccess)
        {
            printf("Unable to create asynchronous event source(%08x)\n", kr);
            (void) (*intf)->USBInterfaceClose(intf);
            (void) (*intf)->Release(intf);
			CFRunLoopAddSource(CFRunLoopGetCurrent(), mAsyncRunLoopSource,
							   kCFRunLoopDefaultMode);
            break;
        }
        break;
    }
    return kr;	
}

void		ControlUSB::printErr(IOReturn kr)
{
#ifdef DEBUG_PRINT
	printf("result:%08x\n", kr);
	
	IOReturn	err;
	err = err_get_system( kr );
	printf("err system:%02x\n", err);
	
	err = err_get_sub( kr );
	printf("err subsystem:%04x\n", err);
	
	err = err_get_code( kr );
	printf("err code:%04x\n", err);
#endif
}

/*static*/ void ControlUSB::NewDeviceAdded(void *refCon, io_iterator_t iterator)
{
	ControlUSB				*cusb = static_cast<ControlUSB*>(refCon);
    kern_return_t			kr;
    io_service_t			usbDevice;
    IOCFPlugInInterface 	**plugInInterface=NULL;
    IOUSBDeviceInterface300 **dev=NULL;
    HRESULT					res;
    SInt32					score;
	int						exclusiveErr = 0;

	
    while ( (usbDevice = IOIteratorNext(iterator)) )
    {
		if (cusb->mIsPlugged) {
			IOObjectRelease(usbDevice);
			continue;
		}
		
        kr = IOCreatePlugInInterfaceForService(usbDevice, kIOUSBDeviceUserClientTypeID, kIOCFPlugInInterfaceID, &plugInInterface, &score);
        if ((kIOReturnSuccess != kr) || !plugInInterface) {
			IOObjectRelease(usbDevice);
            continue;
        }
        kr = IOObjectRelease(usbDevice);
		
        // デバイス・プラグインからデバイス・インターフェースを取得する。
        res = (*plugInInterface)->QueryInterface(plugInInterface, CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID300), (LPVOID*)&dev);
        (*plugInInterface)->Release(plugInInterface);
        if (res || !dev) {
			//printf("NewDeviceAdded : QueryInterface error.\n");
            continue;
        }
		
        // デバイスをオープンする。
        do {
            kr = (*dev)->USBDeviceOpen(dev);
            if(kIOReturnExclusiveAccess == kr) {
				//printf("NewDeviceAdded : USBDeviceOpen exclusiveErr.\n");
                exclusiveErr++;
                usleep(1000);
            }
        } while((kIOReturnExclusiveAccess == kr) && (exclusiveErr < 5));	// 5回まで再試行する。
        if(kIOReturnSuccess != kr) {
			//printf("NewDeviceAdded : USBDeviceOpen error.\n");
            (*dev)->Release(dev);
            continue;
        }
		
        kr = cusb->configureDevice(dev);
        if (kIOReturnSuccess != kr) {
			//printf("NewDeviceAdded : configureDevice error.\n");
            (*dev)->USBDeviceClose(dev);
            (*dev)->Release(dev);
            continue;
        }
		
        kr = cusb->findInterfaces(dev);
        if (kIOReturnSuccess != kr) {
			//printf("NewDeviceAdded : findInterfaces error.\n");
            (*dev)->USBDeviceClose(dev);
            (*dev)->Release(dev);
            continue;
        }
				
		cusb->mUsbDevice = usbDevice;
		cusb->mDev = dev;
		cusb->mIsPlugged = true;
        if (cusb->mDeviceAddedFunc) {
            cusb->mDeviceAddedFunc(cusb->mDeviceAddedFuncClass);
        }
        
        pthread_create(&cusb->mReadThread, NULL, readThreadFunc, cusb);
    }
}

/*static*/ void ControlUSB::NewDeviceRemoved(void *refCon, io_iterator_t iterator)
{
	ControlUSB		*cusb = static_cast<ControlUSB*>(refCon);
    kern_return_t	kr;
	io_service_t			usbDevice;
    
    while((usbDevice = IOIteratorNext(iterator))) {
		if (usbDevice == cusb->mUsbDevice) {
            // Readスレッドを停止
			cusb->mIsPlugged = false;
            pthread_join(cusb->mReadThread, NULL);
            if (cusb->mDeviceRemovedFunc) {
                cusb->mDeviceRemovedFunc(cusb->mDeviceRemovedFuncClass);
            }
			cusb->releaseAsyncRunLoopSource();
		}
		kr = IOObjectRelease(usbDevice);
    }
}

void ControlUSB::setDeviceAddedFunc( void (*func) (void* ownerClass), void* ownerClass )
{
    mDeviceAddedFunc = func;
    mDeviceAddedFuncClass = ownerClass;
}

void ControlUSB::setDeviceRemovedFunc( void (*func) (void* ownerClass) , void* ownerClass )
{
    mDeviceRemovedFunc = func;
    mDeviceRemovedFuncClass = ownerClass;
}

/*static*/ void *ControlUSB::readThreadFunc(void *arg)
{
    ControlUSB  *This = static_cast<ControlUSB*>(arg);
    while (This->mIsPlugged) {
#if 0
        IOReturn    kr = noErr;
        UInt32      readByte = 1;
        
        if (This->getReadableBytes() == (READ_BUFFER_SIZE-1)) {
            usleep(1000);
            continue;
        }
        kr = (*This->mIntf)->ReadPipeTO(This->mIntf,
                                        This->mRPipe,
                                        &This->mReadBuffer[This->mReadBufferWritePtr],
                                        &readByte, READ_TIMEOUT, READ_TIMEOUT);
        if (kr == kIOReturnSuccess) {
            This->mReadBufferWritePtr = (This->mReadBufferWritePtr + readByte) % READ_BUFFER_SIZE;
        }
#else
        usleep(1000);
#endif
    }
    return 0;
}
#else

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "winusb.lib")

#include <setupapi.h>

ControlUSB::ControlUSB()
: mIsRun(false)
, mIsPlugged(false)
, mWriteBufferPtr(0)
{
	mReadBufferReadPtr = 0;
	mReadBufferWritePtr = 0;
	mDeviceAddedFunc = NULL;
	mDeviceRemovedFunc = NULL;
}
ControlUSB::~ControlUSB()
{
	removeDevice();
	EndPortWait();
}

void ControlUSB::BeginPortWait(LPGUID guid)
{
	BOOL bResult = TRUE;

	HDEVINFO devinf = INVALID_HANDLE_VALUE;
	SP_DEVICE_INTERFACE_DATA spid;
	PSP_DEVICE_INTERFACE_DETAIL_DATA fc_data = NULL;

	devinf = SetupDiGetClassDevs(
		guid,
		NULL,
		0,
		DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	if (devinf) {
		for (int i=0;; i++) {
			ZeroMemory(&spid, sizeof(spid));
			spid.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
			if (!SetupDiEnumDeviceInterfaces(devinf, NULL,
				guid, i, &spid)) {
				break;
			}

			unsigned long sz;
			std::basic_string<TCHAR> devpath;

			// 必要なバッファサイズ取得
			bResult = SetupDiGetDeviceInterfaceDetail(devinf, &spid, NULL, 0, &sz, NULL);
			PSP_INTERFACE_DEVICE_DETAIL_DATA dev_det = (PSP_INTERFACE_DEVICE_DETAIL_DATA)(malloc(sz));
			dev_det->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

			// デバイスノード取得
			if (!SetupDiGetDeviceInterfaceDetail(devinf, &spid, dev_det, sz, &sz, NULL)) {
				free(dev_det);
				break;
			}

			devpath = dev_det->DevicePath;
			free(dev_det);
			dev_det = NULL;

			if (openDevice(devpath)) {
				mIsRun = true;
			}
			else {
				mIsRun = false;
			}
		}
		SetupDiDestroyDeviceInfoList(devinf);
	}
}

bool ControlUSB::openDevice(std::basic_string<TCHAR> devpath)
{
	HANDLE hNewDev = CreateFile(
		devpath.c_str(),
		GENERIC_READ | GENERIC_WRITE,
		0 /*FILE_SHARE_READ|FILE_SHARE_WRITE*/,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED, //FILE_FLAG_NO_BUFFERING,
		NULL);

	if (hNewDev == INVALID_HANDLE_VALUE) {
		return false;
	}

	HANDLE hNewWinUsb = NULL;
	if (!WinUsb_Initialize(hNewDev, &hNewWinUsb)) {
		//DWORD err = GetLastError();
		CloseHandle(hNewDev);
		return false;
	}

	// エンドポイント情報取得
	USB_INTERFACE_DESCRIPTOR desc;
	if (!WinUsb_QueryInterfaceSettings(hNewWinUsb, 0, &desc)) {
		WinUsb_Free(hNewWinUsb);
		CloseHandle(hNewDev);
		return false;
	}

	for (int i = 0; i<desc.bNumEndpoints; i++) {
		WINUSB_PIPE_INFORMATION pipeInfo;
		if (WinUsb_QueryPipe(hNewWinUsb, 0, (UCHAR)i, &pipeInfo)) {
			if (pipeInfo.PipeType == UsbdPipeTypeBulk &&
				USB_ENDPOINT_DIRECTION_OUT(pipeInfo.PipeId)) {
				mOutPipeId = pipeInfo.PipeId;
				mOutPipeMaxPktSize = pipeInfo.MaximumPacketSize;
			}
			else if (pipeInfo.PipeType == UsbdPipeTypeBulk &&
				USB_ENDPOINT_DIRECTION_IN(pipeInfo.PipeId)) {
				mInPipeId = pipeInfo.PipeId;
				mInPipeMaxPktSize = pipeInfo.MaximumPacketSize;
			}
		}
	}

	// タイムアウト設定
	ULONG timeout = 500; //ms
	::WinUsb_SetPipePolicy(hNewWinUsb, mOutPipeId, PIPE_TRANSFER_TIMEOUT, sizeof(ULONG), &timeout);
	::WinUsb_SetPipePolicy(hNewWinUsb, mInPipeId, PIPE_TRANSFER_TIMEOUT, sizeof(ULONG), &timeout);

	// ここでハンドル更新
	m_hDev = hNewDev;
	m_hWinUsb = hNewWinUsb;
	mDevPath = devpath;

	WinUsb_FlushPipe(m_hWinUsb, mOutPipeId);
	WinUsb_FlushPipe(m_hWinUsb, mInPipeId);

	mIsPlugged = true;
	if (mDeviceAddedFunc) {
		mDeviceAddedFunc(mDeviceAddedFuncClass);
	}

	return true;
}
void ControlUSB::EndPortWait()
{
	mIsRun = false;
}

void ControlUSB::removeDevice()
{
	if (mIsPlugged) {
		WinUsb_Free(m_hWinUsb);
		CloseHandle(m_hDev);
		m_hDev = NULL;
		m_hWinUsb = NULL;
		mIsPlugged = false;

		if (mDeviceRemovedFunc) {
			mDeviceRemovedFunc(mDeviceRemovedFuncClass);
		}
	}
}
bool ControlUSB::resetrPipe()
{
	WinUsb_FlushPipe(m_hWinUsb, mInPipeId);
	return true;
}
bool ControlUSB::resetwPipe()
{
	WinUsb_FlushPipe(m_hWinUsb, mOutPipeId);
	return true;
}
int	 ControlUSB::bulkWrite(UINT8 *buf, UINT32 size)
{
	if (!mIsPlugged) return -1;

	ULONG len = size;
	UINT32	bufferRest = WRITE_BUFFER_SIZE - mWriteBufferPtr;
	if (bufferRest < len) {
		mWriteBufferPtr = 0;
	}
	memcpy(&mWriteBuffer[mWriteBufferPtr], buf, len);

	DWORD wlen;	
	BOOL ret = WinUsb_WritePipe(m_hWinUsb, mOutPipeId, &mWriteBuffer[mWriteBufferPtr], len, &wlen, NULL);

	if (ret == FALSE || size != wlen){
		//printf("Write error in bulkWrite.\n");
		//DWORD err = GetLastError();
		//printErr(err);
		return -1;
	}
	mWriteBufferPtr += len;
	return wlen;
}
int	 ControlUSB::bulkWriteAsync(UINT8 *buf, UINT32 size)
{
	return bulkWrite(buf, size);
}
int	 ControlUSB::bulkRead(UINT8 *buf, UINT32 size, UINT32 timeout)
{
	if (!mIsPlugged) return -1;

	DWORD rlen;
	BOOL ret = WinUsb_ReadPipe(m_hWinUsb, mInPipeId, buf, size, &rlen, NULL);

	if (ret == FALSE){
		//printf("Write error in bulkRead.\n");
		//DWORD err = GetLastError();
		//printErr(err);
		return -1;
	}
	return rlen;
}
int	 ControlUSB::read(UINT8 *buf, UINT32 size)
{
	return 0;
}
int	 ControlUSB::getReadableBytes()
{
	return 0;
}

void ControlUSB::setDeviceAddedFunc(void(*func) (void* ownerClass), void* ownerClass)
{
	mDeviceAddedFunc = func;
	mDeviceAddedFuncClass = ownerClass;
}

void ControlUSB::setDeviceRemovedFunc(void(*func) (void* ownerClass), void* ownerClass)
{
	mDeviceRemovedFunc = func;
	mDeviceRemovedFuncClass = ownerClass;
}
#endif
