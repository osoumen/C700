//
//  DspController.h
//  C700
//
//  Created by osoumen on 2015/11/14.
//
//

#ifndef __C700__DspController__
#define __C700__DspController__

#define USE_OPENSPC

#include "DspRegFIFO.h"
#ifdef USE_OPENSPC
#include "openspc.h"
#else
#include "SNES_SPC.h"
#endif
#include <pthread.h>

class DspController {
public:
    DspController();
    ~DspController();
    
    void WriteRam(int addr, const unsigned char *data, int size);
    void WriteRam(int addr, unsigned char data);
    void WriteDsp(int addr, unsigned char data);
    void Process1Sample(int &outl, int &outr);
    
private:
    static const int dspAccCodeAddr = 0x0010;
    static const int dspOutBufSize = 4096;
    
    static unsigned char dspregAccCode[];
    
    DspRegFIFO          mFifo;
#ifndef USE_OPENSPC
    SNES_SPC            mDsp;
#endif
    int                 mPort0state;
    int                 mWaitPort;
    int                 mWaitByte;
    unsigned char       mDspMirror[128];
    
    pthread_mutex_t     mMtx;
    
#ifdef USE_OPENSPC
    short               mOutSamples[dspOutBufSize];
#else
    SNES_SPC::sample_t  mOutSamples[dspOutBufSize];
#endif
};

#endif /* defined(__C700__DspController__) */
