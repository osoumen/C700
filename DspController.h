//
//  DspController.h
//  C700
//
//  Created by osoumen on 2015/11/14.
//
//

#ifndef __C700__DspController__
#define __C700__DspController__

//#define USE_OPENSPC

#include "SpcControlDevice.h"
#include "DspRegFIFO.h"
#ifdef USE_OPENSPC
#include "openspc.h"
#else
#include "SNES_SPC.h"
#endif
#include "C700TimeThread.h"

#define DSP_VOL		(0x00)
#define DSP_P		(0x02)
#define DSP_SRCN	(0x04)
#define DSP_ADSR	(0x05)
#define DSP_GAIN	(0x07)
#define DSP_ENVX	(0x08)
#define DSP_OUTX	(0x09)
#define DSP_MVOLL	(0x0c)
#define DSP_MVOLR	(0x1c)
#define DSP_EVOLL	(0x2c)
#define DSP_EVOLR	(0x3c)
#define DSP_KON		(0x4c)
#define DSP_KOF		(0x5c)
#define DSP_FLG		(0x6c)
#define DSP_ENDX	(0x7c)
#define DSP_EFB		(0x0d)
#define DSP_PMON	(0x2d)
#define DSP_NON		(0x3d)
#define DSP_EON		(0x4d)
#define DSP_DIR		(0x5d)
#define DSP_ESA		(0x6d)
#define DSP_EDL		(0x7d)
#define DSP_FIR		(0x0F)

class DspController {
public:
    DspController();
    ~DspController();
    
    void init();
    
    bool IsHwAvailable() { return mIsHwAvailable; }
    void WriteRam(int addr, const unsigned char *data, int size);
    void WriteRam(int addr, unsigned char data, bool nonRealtime);
    bool WriteDsp(int addr, unsigned char data, bool nonRealtime);
    void Process1Sample(int &outl, int &outr);
	void IncSampleInFrame() { mSampleInFrame++; }
    void BeginFrameProcess(double frameTime);
    void StartMuteEmulation();
    void EndMuteEmulation();
    
    void setDeviceReadyFunc( void (*func) (void* ownerClass), void* ownerClass );
	void setDeviceExitFunc( void (*func) (void* ownerClass) , void* ownerClass );
    
private:
    static const int dspAccCodeAddr = 0x0010;
    static const int dspOutBufSize = 4096;
    
    static unsigned char dspregAccCode[];
    
    bool                mIsHwAvailable;
    bool                mMuteEmulation;
    DspRegFIFO          mEmuFifo;
    DspRegFIFO          mHwFifo;
    int                 mDspMirror[128];
    
    MutexObject         mEmuMtx;
#ifndef USE_OPENSPC
    SNES_SPC            mDsp;
#endif
    int                 mPort0stateEmu;
    int                 mWaitPort;
    int                 mWaitByte;
    // mEmuMtx
    
    MutexObject         mHwMtx;
    SpcControlDevice    *mSpcDev;
    int                 mPort0stateHw;
    // mHwMtx
    
#ifdef USE_OPENSPC
    short               mOutSamples[dspOutBufSize];
#else
    SNES_SPC::sample_t  mOutSamples[dspOutBufSize];
#endif
    
    int                 mSampleInFrame;
    OSTime              mPrevFrameStartTime;
    OSTime              mFrameStartTime;
    OSTime              mNextFrameStartTime;
    MSTime              mHwDelayTime;
    double              mFrameTime;
    ThreadObject        mWriteHwThread;
    
    void               (*mDeviceReadyFunc) (void* ownerClass);
	void               *mDeviceReadyFuncClass;
    void               (*mDeviceExitFunc) (void* ownerClass);
	void               *mDeviceExitFuncClass;
    
#ifdef _MSC_VER
	static DWORD WINAPI writeHwThreadFunc(LPVOID);
#else
    static void *writeHwThreadFunc(void *arg);
#endif
    void doWriteDspHw(int addr, unsigned char data);
    void doWriteRamHw(int addr, unsigned char data);
    
    static void onDeviceAdded(void *ref);
    static void onDeviceRemoved(void *ref);
};

#endif /* defined(__C700__DspController__) */
