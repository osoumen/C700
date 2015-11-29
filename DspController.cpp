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
    0x8F ,0x30 ,0xF1 //       	mov SPC_CONTROL,#$30
    ,0x8F ,0x6C ,0xF2 //       	mov SPC_REGADDR,#DSP_FLG
    ,0x8F ,0x00 ,0xF3 //       	mov SPC_REGDATA,#$a0
    ,0x8D ,0x00       //     	mov y,#0
    ,0xE8 ,0x00       //     	mov a,#0
    ,0x8F ,0x00 ,0x04 //       	mov $04,#$00
    ,0x8F ,0x06 ,0x05 //       	mov $05,#$06
    // initloop:
    ,0xD7 ,0x04       //     	mov [$04]+y,a	; 7
    ,0x3A ,0x04       //     	incw $04		; 6
    ,0x78 ,0x7E ,0x05 //       	cmp $05,#$7e	; 5
    ,0xD0 ,0xF7       //     	bne initloop	; 4
    ,0xE4 ,0xF4       //     	mov a,SPC_PORT0
    // ack:
    ,0x8F ,0x77 ,0xF7 //       	mov SPC_PORT3,#$77
    // loop:
    ,0x64 ,0xF4       //     	cmp a,SPC_PORT0		; 3
    ,0xF0 ,0xFC       //     	beq loop			; 2
    ,0xE4 ,0xF4       //     	mov a,SPC_PORT0		; 3
    ,0x30 ,0x18       //     	bmi toram			; 2
    ,0xF8 ,0xF6       //     	mov x,SPC_PORT2		; 3
    ,0xD8 ,0xF2       //     	mov SPC_REGADDR,x	; 4
    ,0xFA ,0xF5 ,0xF3 //       	mov SPC_REGDATA,SPC_PORT1
    ,0xC4 ,0xF4       //     	mov SPC_PORT0,a		; 4
    // 	; wait 64 - 32 cycle
    ,0xC8 ,0x4C       //     	cmp x,#DSP_KON	; 3
    ,0xF0 ,0x04       //     	beq wait	; 4
    ,0xC8 ,0x5C       //     	cmp x,#DSP_KOF	; 3
    ,0xD0 ,0xE7       //     	bne loop	; 4
    // wait:
    ,0x8D ,0x05       //     	mov y,#5	; 2
    // -
    ,0xFE ,0xFE       //     	dbnz y,-	; 4/6
    ,0x00             //   	nop			; 2
    ,0x2F ,0xE0       //     	bra loop	; 4
    // toram:
    ,0x5D             //   	mov x,a
    ,0x28 ,0x40       //     	and a,#$40
    ,0xD0 ,0x0B       //     	bne blockTrans
    ,0x8D ,0x00       //     	mov y,#0
    ,0xE4 ,0xF5       //     	mov a,SPC_PORT1
    ,0xD7 ,0xF6       //     	mov [SPC_PORT2]+y,a
    ,0x7D             //   	mov a,x
    ,0xC4 ,0xF4       //     	mov SPC_PORT0,a
    ,0x2F ,0xD0       //     	bra loop
    // blockTrans:
    ,0xFA ,0xF6 ,0x04 //       	mov $04,SPC_PORT2
    ,0xFA ,0xF7 ,0x05 //       	mov $05,SPC_PORT3
    ,0x7D             //   	mov a,x
    ,0x8D ,0x00       //     	mov y,#0
    ,0xC4 ,0xF4       //     	mov SPC_PORT0,a
    // loop2:
    ,0x64 ,0xF4       //     	cmp a,SPC_PORT0
    ,0xF0 ,0xFC       //     	beq loop2
    ,0xE4 ,0xF4       //     	mov a,SPC_PORT0
    ,0x30 ,0xBA       //     	bmi ack
    ,0x5D             //   	mov x,a
    ,0xE4 ,0xF5       //     	mov a,SPC_PORT1
    ,0xD7 ,0x04       //     	mov [$04]+y,a
    ,0x3A ,0x04       //     	incw $04
    ,0xE4 ,0xF6       //     	mov a,SPC_PORT2
    ,0xD7 ,0x04       //     	mov [$04]+y,a
    ,0x3A ,0x04       //     	incw $04
    ,0xE4 ,0xF7       //     	mov a,SPC_PORT3
    ,0xD7 ,0x04       //     	mov [$04]+y,a
    ,0x3A ,0x04       //     	incw $04
    ,0x7D             //   	mov a,x
    ,0xC4 ,0xF4       //     	mov SPC_PORT0,a
    ,0x2F ,0xE0       //     	bra loop2
};

DspController::DspController()
{
    mIsHwAvailable = false;
    
    pthread_mutex_init(&mEmuMtx, 0);
    pthread_mutex_init(&mHwMtx, 0);
    
#ifndef USE_OPENSPC
    mDsp.init();
#endif
    unsigned char spcdata[0x10200] = "SNES-SPC700 Sound File Data v0.30\x1A\x1A";
    memset(spcdata+33, 0, 0x10200-33);
    spcdata[0x25] = dspAccCodeAddr & 0xff;
    spcdata[0x26] = (dspAccCodeAddr >> 8) & 0xff;
    spcdata[0x2b] = 0xef;
    spcdata[0x100 + 0xf0] = 0x0a;
    memcpy(&spcdata[0x100 + dspAccCodeAddr], dspregAccCode, sizeof(dspregAccCode));
#ifndef USE_OPENSPC
    mDsp.load_spc(spcdata, 0x10200);
#else
    OSPC_Init(spcdata, 0x10200);
#endif
    mPort0stateEmu = 0x01;
    mWaitPort = -1;
    mWaitByte = 0;
#ifndef USE_OPENSPC
    do {
        mDsp.play(2, NULL);
    } while (mDsp.read_port(0, 3) != 0x77);
#else
    do {
        OSPC_Run(1, NULL, 0);
    } while ((unsigned char)OSPC_ReadPort3() != 0x77);
#endif
    
    memset(mDspMirror, 0xef, 128 * sizeof(int));
    WriteDsp(DSP_EDL, 0x00, true);
    WriteDsp(DSP_ESA, 0x06, true);
    WriteDsp(DSP_FLG, 0x00, true);
    WriteDsp(DSP_EFB, 0x00, true);
    
    // エコー音量をオフ
    WriteDsp(DSP_EVOLL, 0x00, true);
    WriteDsp(DSP_EVOLR, 0x00, true);
    
    // NONをオフ
    WriteDsp(DSP_NON, 0x00, true);
    
    // PMONをオフ
    WriteDsp(DSP_PMON, 0x00, true);

    mSpcDev.setDeviceAddedFunc(onDeviceAdded, this);
    mSpcDev.setDeviceRemovedFunc(onDeviceRemoved, this);
    mSpcDev.Init();
}

DspController::~DspController()
{
    mSpcDev.Close();
    mFifo.Clear();
    pthread_mutex_destroy(&mHwMtx);
    pthread_mutex_destroy(&mEmuMtx);
}

void DspController::onDeviceAdded(void *ref)
{
    DspController   *This = reinterpret_cast<DspController*>(ref);
    
    int err = 0;
    
    pthread_mutex_lock(&This->mHwMtx);
    
    // ハードウェアリセット
    This->mSpcDev.HwReset();
    // ソフトウェアリセット
    This->mSpcDev.SwReset();
    
    // $BBAA 待ち
    err = This->mSpcDev.WaitReady();
    if (err) {
        return;
    }
    
    // ノイズ回避のため音量を0に
    unsigned char dspWrite[2];
    err = 0xcc-1;
    dspWrite[0] = DSP_MVOLL;
    dspWrite[1] = 0;
    err = This->mSpcDev.UploadRAMDataIPL(dspWrite, 0x00f2, 2, err+1);
    if (err < 0) {
        return;
    }
    dspWrite[0] = DSP_MVOLR;
    dspWrite[1] = 0;
    err = This->mSpcDev.UploadRAMDataIPL(dspWrite, 0x00f2, 2, err+1);
    if (err < 0) {
        return;
    }
    dspWrite[0] = DSP_EVOLL;
    dspWrite[1] = 0;
    err = This->mSpcDev.UploadRAMDataIPL(dspWrite, 0x00f2, 2, err+1);
    if (err < 0) {
        return;
    }
    dspWrite[0] = DSP_EVOLR;
    dspWrite[1] = 0;
    err = This->mSpcDev.UploadRAMDataIPL(dspWrite, 0x00f2, 2, err+1);
    if (err < 0) {
        return;
    }
    
    // EDL,ESAを初期化
    dspWrite[0] = DSP_EDL;
    dspWrite[1] = 0;
    err = This->mSpcDev.UploadRAMDataIPL(dspWrite, 0x00f2, 2, err+1);
    if (err < 0) {
        return;
    }
    dspWrite[0] = DSP_ESA;
    dspWrite[1] = 0x06; // DIRの直後
    err = This->mSpcDev.UploadRAMDataIPL(dspWrite, 0x00f2, 2, err+1);
    if (err < 0) {
        return;
    }
    usleep(240000); // EDL,ESAを変更したので240ms待ち
    
    // DSPアクセス用コードを転送
    err = This->mSpcDev.UploadRAMDataIPL(dspregAccCode, dspAccCodeAddr, sizeof(dspregAccCode), err+1);
    if (err < 0) {
        return;
    }
    
    // 転送済みコードへジャンプ
    err = This->mSpcDev.JumpToCode(dspAccCodeAddr, err+1);
    if (err < 0) {
        return;
    }
#if 1
    while (This->mSpcDev.PortRead(3) != 0x77) {
        usleep(1000);
    }
#else
    This->mSpcDev.ReadAndWait(3, 0x77);
    This->mSpcDev.WriteBuffer();
#endif
    This->mPort0stateHw = 1;
    
    // TODO: 必要なRAMデータを転送
    
    // DSPの復元
    for (int i=0; i<128; i++) {
        if (This->mDspMirror[i] == 0xefefefef) {
            continue;
        }
        if (i == DSP_FLG) {
            continue;
        }
        if (i == DSP_KON) {
            continue;
        }
        This->mSpcDev.BlockWrite(1, This->mDspMirror[i], i);
        This->mSpcDev.WriteAndWait(0, This->mPort0stateHw);
        This->mPort0stateHw = This->mPort0stateHw ^ 1;
    }
    This->mSpcDev.WriteBuffer();
    
    This->mIsHwAvailable = true;
    
    pthread_mutex_unlock(&This->mHwMtx);
}

void DspController::onDeviceRemoved(void *ref)
{
    DspController   *This = reinterpret_cast<DspController*>(ref);
    
    This->mIsHwAvailable = false;
}

void DspController::WriteRam(int addr, const unsigned char *data, int size)
{
    if (size <= 0) {
        return;
    }
    
    pthread_mutex_lock(&mEmuMtx);
    for (int i=0; i<size; i++) {
#ifndef USE_OPENSPC
        /*
        mDsp.write_port(0, 1, data[i]);
        mDsp.write_port(0, 2, (addr + i) & 0xff);
        mDsp.write_port(0, 3, ((addr + i)>>8) & 0xff);
        mDsp.write_port(0, 0, mPort0state | 0x80);
         */
        mDsp.GetRam()[addr+i] = data[i];
#else
        OSPC_WritePort1(data[i]);
        OSPC_WritePort2((addr + i) & 0xff);
        OSPC_WritePort3(((addr + i)>>8) & 0xff);
        OSPC_WritePort0(mPort0stateEmu | 0x80);
        mWaitPort = 0;
        mWaitByte = mPort0stateEmu | 0x80;
        mPort0stateEmu = mPort0stateEmu ^ 0x01;
#endif
#ifndef USE_OPENSPC
        /*
        do {
            mDsp.play(2, NULL);
        } while (mDsp.read_port(0, mWaitPort) != mWaitByte);
         */
#else
        do {
            OSPC_Run(32, NULL, 0);
        } while ((unsigned char)OSPC_ReadPort0() != mWaitByte);
#endif
        mWaitPort = -1;
    }
    pthread_mutex_unlock(&mEmuMtx);

    if (mIsHwAvailable) {
        pthread_mutex_lock(&mHwMtx);
#if 0
        // IPLに戻るために0x0004に非０を書き込む
        mSpcDev.BlockWrite(1, 0x01, 0x04, 0x00);
        mSpcDev.BlockWrite(0, mPort0stateHw | 0x80);
        mSpcDev.WriteBuffer();
        while (mSpcDev.PortRead(0) != 0xaa) {
            usleep(1000);
        }
        while (mSpcDev.PortRead(1) != 0xbb) {
            usleep(1000);
        }
        //mSpcDev.ReadAndWait(1, 0xbb);
        
        // IPLを使用してRAMにデータを転送する
        mPort0stateHw = mSpcDev.UploadRAMDataIPL(data, addr, size, 0xcc);
        // メインプログラムに戻る
        if (mPort0stateHw == 0xff) {
            mPort0stateHw++;
        }
        mSpcDev.JumpToCode(dspAccCodeAddr, mPort0stateHw + 1);
        while (mSpcDev.PortRead(3) != 0x77) {
            usleep(1000);
        }
        //mSpcDev.ReadAndWait(3, 0x77);
        //mSpcDev.WriteBuffer();
        mPort0stateHw = 1;
#endif
#if 1
        mSpcDev.BlockWrite(2, addr & 0xff, (addr>>8) & 0xff);
        mSpcDev.WriteAndWait(0, mPort0stateHw | 0xc0);
        mSpcDev.WriteBuffer();
        mPort0stateHw = mPort0stateHw ^ 0x01;
        int num = size / 3;
        int rest = size - num * 3;
        int ptr = 0;
        for (int i=0; i<num; i++) {
            mSpcDev.BlockWrite(1, data[ptr], data[ptr+1], data[ptr+2]);
            ptr += 3;
            mSpcDev.WriteAndWait(0, mPort0stateHw);
            mPort0stateHw = mPort0stateHw ^ 0x01;
        }
        mSpcDev.BlockWrite(0, mPort0stateHw | 0x80);
        mSpcDev.ReadAndWait(3, 0x77);
        mSpcDev.WriteBuffer();
        mPort0stateHw = mPort0stateHw ^ 0x01;
        addr += num * 3;
        for (int i=0; i<rest; i++) {
            mSpcDev.BlockWrite(1, data[ptr], (addr + i) & 0xff, ((addr + i)>>8) & 0xff);
            ptr++;
            mSpcDev.WriteAndWait(0, mPort0stateHw | 0x80);
            mSpcDev.WriteBuffer();
            mPort0stateHw = mPort0stateHw ^ 0x01;
        }
#else
        for (int i=0; i<size; i++) {
            mSpcDev.BlockWrite(1, data[i], (addr + i) & 0xff, ((addr + i)>>8) & 0xff);
            mSpcDev.WriteAndWait(0, mPort0stateHw | 0x80);
            mPort0stateHw = mPort0stateHw ^ 0x01;
        }
        mSpcDev.WriteBuffer();
#endif
        pthread_mutex_unlock(&mHwMtx);
    }
}

void DspController::WriteRam(int addr, unsigned char data, bool nonRealtime)
{
    if (nonRealtime) {
        while (mWaitPort >= 0) {
            int outl ,outr;
            Process1Sample(outl, outr);
        }
        
        pthread_mutex_lock(&mEmuMtx);
#ifndef USE_OPENSPC
        /*
         mDsp.write_port(0, 1, write.data);
         mDsp.write_port(0, 2, write.addr & 0xff);
         mDsp.write_port(0, 3, (write.addr>>8) & 0xff);
         mDsp.write_port(0, 0, mPort0stateEmu | 0x80);
         */
        mDsp.GetRam()[addr] = data;
#else
        OSPC_WritePort1(data);
        OSPC_WritePort2(addr & 0xff);
        OSPC_WritePort3((addr>>8) & 0xff);
        OSPC_WritePort0(mPort0stateEmu | 0x80);
        mWaitPort = 0;
        mWaitByte = mPort0stateEmu | 0x80;
        mPort0stateEmu = mPort0stateEmu ^ 0x01;
#endif
        pthread_mutex_unlock(&mEmuMtx);
    }
    else {
        if (mIsHwAvailable) {
            pthread_mutex_lock(&mHwMtx);
            mSpcDev.BlockWrite(1, data, addr & 0xff, (addr>>8) & 0xff);
            mSpcDev.WriteAndWait(0, mPort0stateHw | 0x80);
            mSpcDev.WriteBufferAsync();
            mPort0stateHw = mPort0stateHw ^ 0x01;
            pthread_mutex_unlock(&mHwMtx);
        }
        mFifo.AddRamWrite(0, addr, data);
    }
}

bool DspController::WriteDsp(int addr, unsigned char data, bool nonRealtime)
{
    addr &= 0x7f;
    bool doWrite = (addr == DSP_KON || addr == DSP_KOF || mDspMirror[addr] != data)?true:false;
    
    if (nonRealtime) {
        while (mWaitPort >= 0) {
            int outl ,outr;
            Process1Sample(outl, outr);
        }
        mDspMirror[addr] = data;
        pthread_mutex_lock(&mEmuMtx);
#ifndef USE_OPENSPC
        mDsp.write_port(0, 1, data);
        mDsp.write_port(0, 2, addr);
        mDsp.write_port(0, 0, mPort0stateEmu);
#else
        OSPC_WritePort1(data);
        OSPC_WritePort2(addr);
        OSPC_WritePort0(mPort0stateEmu);
#endif
        mWaitPort = 0;
        mWaitByte = mPort0stateEmu;
        mPort0stateEmu = mPort0stateEmu ^ 0x01;
        pthread_mutex_unlock(&mEmuMtx);
        doWrite = true;
    }
    else {
        if (doWrite) {
            mDspMirror[addr] = data;
            if (mIsHwAvailable) {
                pthread_mutex_lock(&mHwMtx);
                mSpcDev.BlockWrite(1, data, addr & 0xff);
                mSpcDev.WriteAndWait(0, mPort0stateHw);
                mSpcDev.WriteBufferAsync();
                mPort0stateHw = mPort0stateHw ^ 0x01;
                pthread_mutex_unlock(&mHwMtx);
            }
            mFifo.AddDspWrite(0, addr, data);
        }
    }
    return doWrite;
}

void DspController::Process1Sample(int &outl, int &outr)
{
    pthread_mutex_lock(&mEmuMtx);
    if (mWaitPort >= 0) {
#ifndef USE_OPENSPC
        if (mDsp.read_port(0, mWaitPort) == mWaitByte) {
            mWaitPort = -1;
        }
#else
        unsigned char read = OSPC_ReadPort0();
        if (read == mWaitByte) {
            mWaitPort = -1;
        }
#endif
        pthread_mutex_unlock(&mEmuMtx);
    }
    else {
        pthread_mutex_unlock(&mEmuMtx);
        bool nowrite = false;
        do {
            size_t numWrites = mFifo.GetNumWrites();
            if (numWrites > 0) {
                DspRegFIFO::DspWrite write = mFifo.PopFront();
                if (write.isRam) {
                    WriteRam(write.addr, write.data, true);
                }
                else {
                    nowrite = !WriteDsp(write.addr, write.data, true);
                }
            }
            else {
                nowrite = false;
            }
        } while (nowrite);
    }
    pthread_mutex_lock(&mEmuMtx);
#ifndef USE_OPENSPC
    blargg_err_t err = mDsp.play(2, mOutSamples);
    assert(err == NULL);
#else
    OSPC_Run(32, mOutSamples, dspOutBufSize);
#endif
    pthread_mutex_unlock(&mEmuMtx);
    outl = mOutSamples[0];
    outr = mOutSamples[1];
    if (mIsHwAvailable) {
        outl = 0;
        outr = 0;
    }
}
