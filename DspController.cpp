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
    ,0x8F ,0x20 ,0xF3 //                            	mov SPC_REGDATA,#,0x20
    ,0x8F ,0x7D ,0xF2 //                            	mov SPC_REGADDR,#DSP_EDL
    ,0x8F ,0x00 ,0xF3 //                            	mov SPC_REGDATA,#,0x00
    ,0x8F ,0x6D ,0xF2 //                            	mov SPC_REGADDR,#DSP_ESA
    ,0x8F ,0xFF ,0xF3 //                            	mov SPC_REGDATA,#,0xff
    ,0x8F ,0x6C ,0xF2 //                            	mov SPC_REGADDR,#DSP_FLG
    ,0x8F ,0x00 ,0xF3 //                            	mov SPC_REGDATA,#,0x00
    ,0xE8 ,0x00       //                          	mov a,#,0x00
    ,0xC4 ,0x04       //                          	mov ,0x04,a
    ,0x8F ,0x77 ,0xF7 //                            	mov SPC_PORT3,#,0x77
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
    ,0x8D ,0x00       //                          	mov y,#0
    ,0xE4 ,0xF5       //                          	mov a,SPC_PORT1
    ,0xD7 ,0xF6       //                          	mov [SPC_PORT2]+y,a
    ,0x7D             //                        	mov a,x
    ,0xC4 ,0xF4       //                          	mov SPC_PORT0,a
    ,0xF8 ,0x04       //                          	mov x,,0x04
    ,0xF0 ,0xD2       //                          	beq loop	; ,0x0004に0以外が書き込まれたらIPLに飛ぶ
    ,0x8F ,0xB0 ,0xF1 //                            	mov SPC_CONTROL,#,0xb0
    ,0x5F ,0xCF ,0xFF //                            	jmp !,0xffcf
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
    memset(mDspMirror, 0, 128);
    WriteDsp(0x6c, 0x18);
    //WriteDsp(0x3d, 0xff);   // NON テスト
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
    
    // DSPアクセス用コードを転送
    err = This->mSpcDev.UploadRAMDataIPL(dspregAccCode, dspAccCodeAddr, sizeof(dspregAccCode), 0xcc);
    if (err < 0) {
        return;
    }
    
    err = This->mSpcDev.JumpToCode(dspAccCodeAddr, err+1);
    if (err < 0) {
        return;
    }
    This->mSpcDev.ReadAndWait(3, 0x77);
    This->mSpcDev.WriteBuffer();
    
    This->mPort0stateHw = 1;
    
    // NONをオフ
    This->mSpcDev.BlockWrite(1, 0, 0x2d);
    This->mSpcDev.WriteAndWait(0, This->mPort0stateHw);
    This->mSpcDev.WriteBuffer();
    This->mPort0stateHw = This->mPort0stateHw ^ 1;
    
    // PMONをオフ
    This->mSpcDev.BlockWrite(1, 0, 0x3d);
    This->mSpcDev.WriteAndWait(0, This->mPort0stateHw);
    This->mSpcDev.WriteBuffer();
    This->mPort0stateHw = This->mPort0stateHw ^ 1;
    
    // TODO: 必要なRAMデータを転送
    
    This->mIsHwAvailable = true;
}

void DspController::onDeviceRemoved(void *ref)
{
    DspController   *This = reinterpret_cast<DspController*>(ref);
    
    This->mIsHwAvailable = false;
}

void DspController::WriteRam(int addr, const unsigned char *data, int size)
{
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
        WriteDsp(0x6c, 0x20);
        // IPLに戻るために0x0004に非０を書き込む
        mSpcDev.BlockWrite(1, 0x01, 0x04, 0x00);
        mSpcDev.WriteAndWait(0, mPort0stateHw | 0x80);
        mSpcDev.WriteBuffer();
        // IPLを使用してRAMにデータを転送する
        mPort0stateHw = mSpcDev.UploadRAMDataIPL(data, addr, size, 0xcc);
        // メインプログラムに戻る
        mSpcDev.JumpToCode(dspAccCodeAddr, mPort0stateHw+1);
        mSpcDev.ReadAndWait(3, 0x77);
        mSpcDev.WriteBuffer();
        mPort0stateHw = 1;
        WriteDsp(0x6c, 0x00);
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
        //mSpcDev.WriteBuffer();
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
            //mSpcDev.WriteBuffer();
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
                mDsp.write_port(0, 0, mPort0state | 0x80);
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
                mDsp.write_port(0, 0, mPort0state);
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
