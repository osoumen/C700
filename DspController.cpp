//
//  DspController.cpp
//  C700
//
//  Created by osoumen on 2015/11/14.
//
//

#include "DspController.h"

//-----------------------------------------------------------------------------
unsigned char DspController::dspregAccCode[] =
{
    /*
     mov a,#$00
     mov SPC_PORT3,#$77
     loop:
     cmp a,SPC_PORT0		; 3
     beq loop			; 2
     mov a,SPC_PORT0		; 3
     bmi toram			; 2
     mov x,SPC_PORT2		; 3
     mov SPC_REGADDR,x	; 4
     mov SPC_REGDATA,SPC_PORT1
     mov SPC_PORT0,a		; 4
     ; wait 64 - 32 cycle
     cmp x,#$4c	; 3
     beq wait	; 4
     cmp x,#$5c	; 3
     bne loop	; 4
     wait:
     mov y,#5	; 2
     -
     dbnz y,-	; 4/6
     nop			; 2
     bra loop	; 4
     toram:
     mov x,a
     mov y,#0
     mov a,SPC_PORT1
     mov [SPC_PORT2]+y,a
     mov a,x
     mov SPC_PORT0,a
     bra loop
     */
    0xe8, 0x00, 0x8f, 0x77, 0xf7, 0x64, 0xf4, 0xf0, 0xfc, 0xe4, 0xf4, 0x30, 0x18, 0xf8, 0xf6, 0xd8,
    0xf2, 0xfa, 0xf5, 0xf3, 0xc4, 0xf4, 0xc8, 0x4c, 0xf0, 0x04, 0xc8, 0x5c, 0xd0, 0xe7, 0x8d, 0x05,
    0xfe, 0xfe, 0x00, 0x2f, 0xe0, 0x5d, 0x8d, 0x00, 0xe4, 0xf5, 0xd7, 0xf6, 0x7d, 0xc4, 0xf4, 0x2f,
    0xd4
};

DspController::DspController()
{
    mDsp.init();
    unsigned char spcdata[0x10200] = "SNES-SPC700 Sound File Data v0.30\x1A\x1A";
    memset(spcdata+33, 0, 0x10200-33);
    spcdata[0x25] = dspAccCodeAddr & 0xff;
    spcdata[0x26] = (dspAccCodeAddr >> 8) & 0xff;
    spcdata[0x100 + 0xf0] = 0x0a;
    memcpy(&spcdata[0x100 + dspAccCodeAddr], dspregAccCode, sizeof(dspregAccCode));
    mDsp.load_spc(spcdata, 0x10200);
    mPort0state = 0x01;
    mWaitPort = -1;
    mWaitByte = 0;
    do {
        mDsp.play(2, NULL);
    } while (mDsp.read_port(0, 3) != 0x77);
    memset(mDspMirror, 0, 128);
    pthread_mutex_init(&mMtx, 0);
}

DspController::~DspController()
{
    pthread_mutex_destroy(&mMtx);
}

void DspController::WriteRam(int addr, const unsigned char *data, int size)
{
    pthread_mutex_lock(&mMtx);
    for (int i=0; i<size; i++) {
        mDsp.write_port(0, 1, data[i]);
        mDsp.write_port(0, 2, (addr + i) & 0xff);
        mDsp.write_port(0, 3, ((addr + i)>>8) & 0xff);
        mDsp.write_port(0, 0, mPort0state | 0x80);
        mWaitPort = 0;
        mWaitByte = mPort0state | 0x80;
        mPort0state = mPort0state ^ 0x01;
        do {
            mDsp.play(2, NULL);
        } while (mDsp.read_port(0, mWaitPort) != mWaitByte);
        mWaitPort = -1;
    }
    pthread_mutex_unlock(&mMtx);
}

void DspController::WriteRam(int addr, unsigned char data)
{
    mFifo.AddRamWrite(0, addr, data);
}

void DspController::WriteDsp(int addr, unsigned char data)
{
    assert(addr < 128);
    if (addr == 0x4c || addr == 0x5c || mDspMirror[addr] != data) {
        mDspMirror[addr] = data;
        mFifo.AddDspWrite(0, addr, data);
    }
}

void DspController::Process1Sample(int &outl, int &outr)
{
    if (pthread_mutex_trylock(&mMtx) == EBUSY) {
        outl = 0;
        outr = 0;
        return;
    }
    
    //assert(mFifo.GetNumWrites() > 2000);
    if (mWaitPort >= 0) {
        if (mDsp.read_port(0, mWaitPort) == mWaitByte) {
            mWaitPort = -1;
            mWaitCycle = 0;
        }
        else {
            mWaitCycle++;
            //assert(mWaitCycle < 1000);
        }
    }
    else {
        size_t numWrites = mFifo.GetNumWrites();
        if (numWrites > 0) {
            DspRegFIFO::DspWrite write = mFifo.PopFront();
            if (write.isRam) {
                mDsp.write_port(0, 1, write.data);
                mDsp.write_port(0, 2, write.addr & 0xff);
                mDsp.write_port(0, 3, (write.addr>>8) & 0xff);
                mDsp.write_port(0, 0, mPort0state | 0x80);
                mWaitPort = 0;
                mWaitByte = mPort0state | 0x80;
                mPort0state = mPort0state ^ 0x01;
            }
            else {
                mDsp.write_port(0, 1, write.data);
                mDsp.write_port(0, 2, write.addr);
                mDsp.write_port(0, 0, mPort0state);
                mWaitPort = 0;
                mWaitByte = mPort0state;
                mPort0state = mPort0state ^ 0x01;
            }
        }
    }
    blargg_err_t err = mDsp.play(2, mOutSamples);
    assert(err == NULL);
    outl = mOutSamples[0];
    outr = mOutSamples[1];
    pthread_mutex_unlock(&mMtx);
}
