//
//  DspController.h
//  C700
//
//  Created by osoumen on 2015/11/14.
//
//

#ifndef __C700__DspController__
#define __C700__DspController__

#include "DspRegFIFO.h"
#include "SNES_SPC.h"
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
    SNES_SPC            mDsp;
    int                 mPort0state;
    int                 mWaitPort;
    int                 mWaitByte;
    unsigned char       mDspMirror[128];
    
    int                 mWaitCycle;
    
    pthread_mutex_t     mMtx;
    
    SNES_SPC::sample_t  mOutSamples[dspOutBufSize];
};

#endif /* defined(__C700__DspController__) */
