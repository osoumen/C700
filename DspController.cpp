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
    0x8F ,0x6C ,0xF2  //                           	mov SPC_REGADDR,#DSP_FLG
    ,0x8F ,0xA0 ,0xF3 //                            	mov SPC_REGDATA,#$a0
    ,0xE4 ,0xF4       //                          	mov a,SPC_PORT0
    ,0x8F ,0x00 ,0x04 //                            	mov $04,#$00
    //                      ack:
    ,0x8F ,0x77 ,0xF7 //                            	mov SPC_PORT3,#$77
    //                      loop:
    ,0x64 ,0xF4       //                          	cmp a,SPC_PORT0		; 3
    ,0xF0 ,0xFC       //                          	beq loop			; 2
    ,0xE4 ,0xF4       //                          	mov a,SPC_PORT0		; 3
    ,0x30 ,0x18       //                          	bmi toram			; 2
    ,0xF8 ,0xF6       //                          	mov x,SPC_PORT2		; 3
    ,0xD8 ,0xF2       //                          	mov SPC_REGADDR,x	; 4
    ,0xFA ,0xF5 ,0xF3 //                            	mov SPC_REGDATA,SPC_PORT1
    ,0xC4 ,0xF4       //                          	mov SPC_PORT0,a		; 4
    //                      	; wait 64 - 32 cycle
    ,0xC8 ,0x4C       //                          	cmp x,#DSP_KON	; 3
    ,0xF0 ,0x04       //                          	beq wait	; 4
    ,0xC8 ,0x5C       //                          	cmp x,#DSP_KOF	; 3
    ,0xD0 ,0xE7       //                          	bne loop	; 4
    //                      wait:
    ,0x8D ,0x05       //                          	mov y,#5	; 2
    //                      -
    ,0xFE ,0xFE       //                          	dbnz y,-	; 4/6
    ,0x00             //                        	nop			; 2
    ,0x2F ,0xE0       //                          	bra loop	; 4
    //                      toram:
    ,0x5D             //                        	mov x,a
    ,0x28 ,0x40       //                          	and a,#$40
    ,0xD0 ,0x13       //                          	bne blockTrans
    ,0x8D ,0x00       //                          	mov y,#0
    ,0xE4 ,0xF5       //                          	mov a,SPC_PORT1
    ,0xD7 ,0xF6       //                          	mov [SPC_PORT2]+y,a
    ,0x7D             //                        	mov a,x
    ,0xC4 ,0xF4       //                          	mov SPC_PORT0,a
    ,0xF8 ,0x04       //                          	mov x,$04
    ,0xF0 ,0xCE       //                          	beq loop	; $0004に0以外が書き込まれたらIPLに飛ぶ
    ,0x8F ,0xB0 ,0xF1 //                            	mov SPC_CONTROL,#$b0
    ,0x5F ,0xC9 ,0xFF //                            	jmp !$ffc9
    //                      blockTrans:
    ,0xFA ,0xF6 ,0x06 //                            	mov $06,SPC_PORT2
    ,0xFA ,0xF7 ,0x07 //                            	mov $07,SPC_PORT3
    ,0x7D             //                        	mov a,x
    ,0x8D ,0x00       //                          	mov y,#0
    ,0xC4 ,0xF4       //                          	mov SPC_PORT0,a
    //                      loop2:
    ,0x64 ,0xF4       //                          	cmp a,SPC_PORT0
    ,0xF0 ,0xFC       //                          	beq loop2
    ,0xE4 ,0xF4       //                          	mov a,SPC_PORT0
    ,0x30 ,0xB2       //                          	bmi ack
    ,0x5D             //                        	mov x,a
    ,0xE4 ,0xF5       //                          	mov a,SPC_PORT1
    ,0xD7 ,0x06       //                          	mov [$06]+y,a
    ,0x3A ,0x06       //                          	incw $06
    ,0xE4 ,0xF6       //                          	mov a,SPC_PORT2
    ,0xD7 ,0x06       //                          	mov [$06]+y,a
    ,0x3A ,0x06       //                          	incw $06
    ,0xE4 ,0xF7       //                          	mov a,SPC_PORT3
    ,0xD7 ,0x06       //                          	mov [$06]+y,a
    ,0x3A ,0x06       //                          	incw $06
    ,0x7D             //                        	mov a,x
    ,0xC4 ,0xF4       //                          	mov SPC_PORT0,a
    ,0x2F ,0xE0       //                          	bra loop2
};

DspController::DspController()
{
    mIsHwAvailable = false;
    
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
    
    memset(mDspMirror, 0xefefefef, 128);
    WriteDsp(DSP_FLG, 0x00);
    
    // エコー音量をオフ
    WriteDsp(DSP_EVOLL, 0x00);
    WriteDsp(DSP_EVOLR, 0x00);
    
    // NONをオフ
    WriteDsp(DSP_NON, 0x00);
    
    // PMONをオフ
    WriteDsp(DSP_PMON, 0x00);

    pthread_mutex_init(&mMtx, 0);
    
    mSpcDev.setDeviceAddedFunc(onDeviceAdded, this);
    mSpcDev.setDeviceRemovedFunc(onDeviceRemoved, this);
    mSpcDev.Init();
}

DspController::~DspController()
{
    mSpcDev.Close();
    pthread_mutex_destroy(&mMtx);
}

void DspController::onDeviceAdded(void *ref)
{
    DspController   *This = reinterpret_cast<DspController*>(ref);
    
    int err = 0;
    
    // ハードウェアリセット
    This->mSpcDev.HwReset();
    // ソフトウェアリセット
    This->mSpcDev.SwReset();
    
    // $BBAA 待ち
    err = This->mSpcDev.WaitReady();
    if (err) {
        return;
    }
    
    // EDL,ESAを初期化
    unsigned char dspWrite[2];
    dspWrite[0] = DSP_EDL;
    dspWrite[1] = 0;
    err = This->mSpcDev.UploadRAMDataIPL(dspWrite, 0x00f2, 2, 0xcc);
    if (err < 0) {
        return;
    }
    dspWrite[0] = DSP_ESA;
    dspWrite[1] = 0xff;
    err = This->mSpcDev.UploadRAMDataIPL(dspWrite, 0x00f2, 2, err+1);
    if (err < 0) {
        return;
    }
    usleep(240000); // 240ms待ち
    
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
    This->mSpcDev.ReadAndWait(3, 0x77);
    This->mSpcDev.WriteBuffer();
    
    This->mPort0stateHw = 1;
    
    This->mSpcDev.WriteBuffer();

    // TODO: 必要なRAMデータを転送
    
    // DSPの復元
    for (int i=0; i<128; i++) {
        if ((i != DSP_FLG) && (i != DSP_KON) && (This->mDspMirror[i] != 0xefefefef)) {
            This->mSpcDev.BlockWrite(1, This->mDspMirror[i], i);
            This->mSpcDev.WriteAndWait(0, This->mPort0stateHw);
            This->mPort0stateHw = This->mPort0stateHw ^ 1;
        }
    }
    This->mSpcDev.WriteBuffer();
    
    This->mIsHwAvailable = true;
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
    pthread_mutex_lock(&mMtx);
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

    if (mIsHwAvailable) {
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
    }
    pthread_mutex_unlock(&mMtx);
}

void DspController::WriteRam(int addr, unsigned char data)
{
    mFifo.AddRamWrite(0, addr, data);
    if (mIsHwAvailable) {
        mSpcDev.BlockWrite(1, data, addr & 0xff, (addr>>8) & 0xff);
        mSpcDev.WriteAndWait(0, mPort0stateHw | 0x80);
        mSpcDev.WriteBufferAsync();
        mPort0stateHw = mPort0stateHw ^ 0x01;
    }
}

void DspController::WriteDsp(int addr, unsigned char data)
{
    assert(addr < 128);
    if (addr == 0x4c || addr == 0x5c || mDspMirror[addr] != data) {
        mDspMirror[addr] = data;
        mFifo.AddDspWrite(0, addr, data);
        if (mIsHwAvailable) {
            mSpcDev.BlockWrite(1, data, addr & 0xff);
            mSpcDev.WriteAndWait(0, mPort0stateHw);
            mSpcDev.WriteBufferAsync();
            mPort0stateHw = mPort0stateHw ^ 0x01;
        }
    }
}

void DspController::Process1Sample(int &outl, int &outr)
{
    if (pthread_mutex_trylock(&mMtx) == EBUSY) {
        outl = 0;
        outr = 0;
        return;
    }
    
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
    }
    else {
        size_t numWrites = mFifo.GetNumWrites();
        if (numWrites > 0) {
            DspRegFIFO::DspWrite write = mFifo.PopFront();
            if (write.isRam) {
#ifndef USE_OPENSPC
                mDsp.write_port(0, 1, write.data);
                mDsp.write_port(0, 2, write.addr & 0xff);
                mDsp.write_port(0, 3, (write.addr>>8) & 0xff);
                mDsp.write_port(0, 0, mPort0stateEmu | 0x80);
#else
                OSPC_WritePort1(write.data);
                OSPC_WritePort2(write.addr & 0xff);
                OSPC_WritePort3((write.addr>>8) & 0xff);
                OSPC_WritePort0(mPort0stateEmu | 0x80);
#endif
                mWaitPort = 0;
                mWaitByte = mPort0stateEmu | 0x80;
                mPort0stateEmu = mPort0stateEmu ^ 0x01;
            }
            else {
#ifndef USE_OPENSPC
                mDsp.write_port(0, 1, write.data);
                mDsp.write_port(0, 2, write.addr);
                mDsp.write_port(0, 0, mPort0stateEmu);
#else
                OSPC_WritePort1(write.data);
                OSPC_WritePort2(write.addr);
                OSPC_WritePort0(mPort0stateEmu);
#endif
                mWaitPort = 0;
                mWaitByte = mPort0stateEmu;
                mPort0stateEmu = mPort0stateEmu ^ 0x01;
            }
        }
    }
#ifndef USE_OPENSPC
    blargg_err_t err = mDsp.play(2, mOutSamples);
    assert(err == NULL);
#else
    OSPC_Run(32, mOutSamples, dspOutBufSize);
#endif
    outl = mOutSamples[0];
    outr = mOutSamples[1];
    pthread_mutex_unlock(&mMtx);
    if (mIsHwAvailable) {
        outl = 0;
        outr = 0;
    }
}
