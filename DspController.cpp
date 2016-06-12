//
//  DspController.cpp
//  C700
//
//  Created by osoumen on 2015/11/14.
//
//

#include "SpcControlFTDI.h"
#include "DspController.h"
#include <iomanip>

static const int p3waitValue = 0xee;

//-----------------------------------------------------------------------------
unsigned char DspController::dspregAccCode[] =
{
    0x8F ,0x00 ,0xF1 //       	mov SPC_CONTROL,#$00
    ,0x8F ,0x6C ,0xF2 //       	mov SPC_REGADDR,#DSP_FLG
    ,0x8F ,0x00 ,0xF3 //       	mov SPC_REGDATA,#$00
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
    ,0x8F ,0xEE ,0xF7 //       	mov SPC_PORT3,#$ee
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
    //
    ,0x80             //   	setc
    ,0xA8 ,0x40       //     	sbc a,#P0FLG_BLKTRAS
    ,0x30 ,0x0F       //     	bmi blockTrans
    ,0x28 ,0x20       //     	and a,#P0FLG_P0RST
    ,0xD0 ,0x39       //     	bne resetP0
    //
    ,0x8D ,0x00       //     	mov y,#0
    ,0xE4 ,0xF5       //     	mov a,SPC_PORT1
    ,0xD7 ,0xF6       //     	mov [SPC_PORT2]+y,a
    ,0x7D             //   	mov a,x
    ,0xC4 ,0xF4       //     	mov SPC_PORT0,a
    ,0x2F ,0xCB       //     	bra loop
    // blockTrans:
    ,0x8F ,0x00 ,0xF7 //       	mov SPC_PORT3,#$0
    ,0xFA ,0xF6 ,0x04 //       	mov $04,SPC_PORT2
    ,0xFA ,0xF7 ,0x05 //       	mov $05,SPC_PORT3
    ,0x7D             //   	mov a,x
    ,0x8D ,0x00       //     	mov y,#0
    ,0xC4 ,0xF4       //     	mov SPC_PORT0,a
    // loop2:
    ,0x64 ,0xF4       //     	cmp a,SPC_PORT0
    ,0xF0 ,0xFC       //     	beq loop2
    ,0xE4 ,0xF4       //     	mov a,SPC_PORT0
    ,0x30 ,0xB2       //     	bmi ack
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
    // resetP0:
    ,0x8F ,0xB0 ,0xF1 //       	mov SPC_CONTROL,#$b0
    ,0xD8 ,0xF4       //     	mov SPC_PORT0,x
    ,0x5F ,0xC0 ,0xFF //       	jmp !$ffc0
};

DspController::DspController()
: mSpcDev(NULL)
{
    mIsHwAvailable = false;
    mMuteEmulation = false;
    mSampleInFrame = 0;
    getNowOSTime(mPrevFrameStartTime);
    getNowOSTime(mFrameStartTime);
    getNowOSTime(mNextFrameStartTime);
    mFrameTime = .0;
    mHwDelayTime = 0;
    
    mDeviceReadyFunc = NULL;
    mDeviceExitFunc = NULL;
    
    MutexInit(mEmuMtx);
    MutexInit(mHwMtx);
    
    mSpcDev = new SpcControlFTDI();
}

void DspController::init()
{
#ifndef USE_OPENSPC
    mDsp.init();
#endif
    unsigned char spcdata[0x10200] = "SNES-SPC700 Sound File Data v0.30\x1A\x1A";
    memset(spcdata+33, 0, 0x10200-33);
    spcdata[0x25] = dspAccCodeAddr & 0xff;
    spcdata[0x26] = (dspAccCodeAddr >> 8) & 0xff;
    spcdata[0x2b] = 0xef;
    spcdata[0x100 + 0xf0] = 0x0a;
    spcdata[0x10100 + DSP_MVOLL] = 0;
    spcdata[0x10100 + DSP_MVOLR] = 0;
    spcdata[0x10100 + DSP_EVOLL] = 0;
    spcdata[0x10100 + DSP_EVOLR] = 0;
    spcdata[0x10100 + DSP_FLG] = 0x20;
    spcdata[0x10100 + DSP_EDL] = 0x00;
    spcdata[0x10100 + DSP_ESA] = 0x06;
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
    } while (mDsp.read_port(0, 3) != p3waitValue);
#else
    do {
        OSPC_Run(1, NULL, 0);
    } while ((unsigned char)OSPC_ReadPort3() != p3waitValue);
#endif
    
    memset(mDspMirror, 0xef, 128 * sizeof(int));
    // NONをオフ
    WriteDsp(DSP_NON, 0x00, true);
    mDspMirror[DSP_NON] = 0;
    
    // PMONをオフ
    WriteDsp(DSP_PMON, 0x00, true);
    mDspMirror[DSP_PMON] = 0;
    
    mSpcDev->setDeviceAddedFunc(onDeviceAdded, this);
    mSpcDev->setDeviceRemovedFunc(onDeviceRemoved, this);
    mSpcDev->Init();
}

DspController::~DspController()
{
    if (mIsHwAvailable) {
        mIsHwAvailable = false;
        ThreadJoin(mWriteHwThread);
    }
    mSpcDev->Close();
    mEmuFifo.Clear();
    mHwFifo.Clear();
    MutexDestroy(mHwMtx);
    MutexDestroy(mEmuMtx);
    
    if (mSpcDev != NULL) {
        delete mSpcDev;
        mSpcDev = NULL;
    }
}

void DspController::onDeviceAdded(void *ref)
{
    DspController   *This = reinterpret_cast<DspController*>(ref);
    
    //int err = 0;
    
    // SPCモジュールがあるかチェック
    if (This->mSpcDev->CheckHasRequiredModule() == false) {
        return;
    }
    
    MutexLock(This->mHwMtx);
    
#if 0   // プログラムの転送は実機側で行う
    // リセット
    This->mSpcDev->Reset();

    // $BBAA 待ち
    err = This->mSpcDev->WaitReady();
    if (err) {
        return;
    }
    // ノイズ回避のため音量を0に
    unsigned char dspWrite[2];
    err = 0xcc-1;
    dspWrite[0] = DSP_MVOLL;
    dspWrite[1] = 0;
    err = This->mSpcDev->UploadRAMDataIPL(dspWrite, 0x00f2, 2, err+1);
    if (err < 0) {
        return;
    }
    dspWrite[0] = DSP_MVOLR;
    dspWrite[1] = 0;
    err = This->mSpcDev->UploadRAMDataIPL(dspWrite, 0x00f2, 2, err+1);
    if (err < 0) {
        return;
    }
    dspWrite[0] = DSP_EVOLL;
    dspWrite[1] = 0;
    err = This->mSpcDev->UploadRAMDataIPL(dspWrite, 0x00f2, 2, err+1);
    if (err < 0) {
        return;
    }
    dspWrite[0] = DSP_EVOLR;
    dspWrite[1] = 0;
    err = This->mSpcDev->UploadRAMDataIPL(dspWrite, 0x00f2, 2, err+1);
    if (err < 0) {
        return;
    }
    
    // EDL,ESAを初期化
    dspWrite[0] = DSP_FLG;
    dspWrite[1] = 0x20;
    err = This->mSpcDev->UploadRAMDataIPL(dspWrite, 0x00f2, 2, err+1);
    if (err < 0) {
        return;
    }
    dspWrite[0] = DSP_EDL;
    dspWrite[1] = 0;
    err = This->mSpcDev->UploadRAMDataIPL(dspWrite, 0x00f2, 2, err+1);
    if (err < 0) {
        return;
    }
    dspWrite[0] = DSP_ESA;
    dspWrite[1] = 0x06; // DIRの直後
    err = This->mSpcDev->UploadRAMDataIPL(dspWrite, 0x00f2, 2, err+1);
    if (err < 0) {
        return;
    }
    WaitMicroSeconds(240000); // EDL,ESAを変更したので240ms待ち
    
    // DSPアクセス用コードを転送
    err = This->mSpcDev->UploadRAMDataIPL(dspregAccCode, dspAccCodeAddr, sizeof(dspregAccCode), err+1);
    if (err < 0) {
        return;
    }
    
    // 転送済みコードへジャンプ
    err = This->mSpcDev->JumpToCode(dspAccCodeAddr, err+1);
    if (err < 0) {
        return;
    }
#if 0
    while (This->mSpcDev->PortRead(3) != p3waitValue) {
        WaitMicroSeconds(10000);
    }
#else
    This->mSpcDev->ReadAndWait(3, p3waitValue);
    This->mSpcDev->WriteBuffer();
#endif
#endif
    This->mPort0stateHw = 1;
    
    // ダミー書き込み
    This->doWriteDspHw(0x1d, 0);
    
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
        This->mSpcDev->BlockWrite(1, This->mDspMirror[i], i);
        This->mSpcDev->WriteAndWait(0, This->mPort0stateHw);
        This->mPort0stateHw = This->mPort0stateHw ^ 1;
    }
    This->mSpcDev->WriteBuffer();
    
    This->mIsHwAvailable = true;
    
    MutexUnlock(This->mHwMtx);
    
    if (This->mDeviceReadyFunc) {
        This->mDeviceReadyFunc(This->mDeviceReadyFuncClass);
    }
    
    MutexLock(This->mEmuMtx);
    This->mWaitPort = -1;
    This->mWaitByte = 0;
    This->mEmuFifo.Clear();
    MutexUnlock(This->mEmuMtx);
    
    // スレッドの開始
    ThreadCreate(This->mWriteHwThread, writeHwThreadFunc, This);
}

void DspController::onDeviceRemoved(void *ref)
{
    DspController   *This = reinterpret_cast<DspController*>(ref);
    
    if (This->mIsHwAvailable == false) {
        return;
    }
    
    This->mIsHwAvailable = false;
    
    // スレッドの停止
    ThreadJoin(This->mWriteHwThread);
    
    // DSPの復元
    This->mWaitPort = -1;
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
        This->WriteDsp(i, This->mDspMirror[i], true);
    }
    
    This->mHwFifo.Clear();
    
    if (This->mDeviceExitFunc) {
        This->mDeviceExitFunc(This->mDeviceExitFuncClass);
    }
}

void DspController::WriteRam(int addr, const unsigned char *data, int size)
{
    if (size <= 0) {
        return;
    }
    
    MutexLock(mEmuMtx);
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
        mWaitPort = -1;
#endif
    }
    MutexUnlock(mEmuMtx);

    if (mIsHwAvailable) {
        MutexLock(mHwMtx);
        mSpcDev->BlockWrite(2, addr & 0xff, (addr>>8) & 0xff);
        mSpcDev->WriteAndWait(0, mPort0stateHw | 0xc0);
        mSpcDev->WriteBuffer();
        mPort0stateHw = mPort0stateHw ^ 0x01;
        int num = size / 3;
        int rest = size - num * 3;
        int ptr = 0;
        for (int i=0; i<num; i++) {
            mSpcDev->BlockWrite(1, data[ptr], data[ptr+1], data[ptr+2]);
            ptr += 3;
            mSpcDev->WriteAndWait(0, mPort0stateHw);
            mPort0stateHw = mPort0stateHw ^ 0x01;
        }
        mSpcDev->BlockWrite(0, mPort0stateHw | 0x80);
        mSpcDev->ReadAndWait(3, p3waitValue);
        mSpcDev->WriteBuffer();
        mPort0stateHw = mPort0stateHw ^ 0x01;
        /*
        while (mSpcDev.PortRead(3) != p3waitValue) {
            WaitMicroSeconds(5000);
        }
         */
        addr += num * 3;
        for (int i=0; i<rest; i++) {
            mSpcDev->BlockWrite(1, data[ptr], (addr + i) & 0xff, ((addr + i)>>8) & 0xff);
            ptr++;
            mSpcDev->WriteAndWait(0, mPort0stateHw | 0x80);
            mSpcDev->WriteBuffer();
            mPort0stateHw = mPort0stateHw ^ 0x01;
        }
        
        // 直後のDSP書き込みが失敗する場合があるので無意味なDSP書き込みを１回行う
        doWriteDspHw(0x1d, 0);
        MutexUnlock(mHwMtx);
    }
}

void DspController::WriteRam(int addr, unsigned char data, bool nonRealtime)
{
    if (nonRealtime) {
        while (mWaitPort >= 0) {
            int outl ,outr;
            Process1Sample(outl, outr);
            mSampleInFrame--;
        }
        
        MutexLock(mEmuMtx);
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
        MutexUnlock(mEmuMtx);
    }
    else {
        // mHwFifoに追加
        if (mIsHwAvailable) {
			long int frameTime = (mSampleInFrame * 1e6) / 32000 + mHwDelayTime;
            mHwFifo.AddRamWrite(frameTime, addr, data);
        }
        else if (!mMuteEmulation) {
            mEmuFifo.AddRamWrite(0, addr, data);
        }
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
            mSampleInFrame--;
        }
        MutexLock(mEmuMtx);
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
        MutexUnlock(mEmuMtx);
        doWrite = true;
    }
    else {
        if (doWrite) {
            mDspMirror[addr] = data;
            // mHwFifoに追加
            if (mIsHwAvailable) {
				long int frameTime = (mSampleInFrame * 1e6) / 32000 + mHwDelayTime;
                mHwFifo.AddDspWrite(frameTime, addr, data);
            }
            else if (!mMuteEmulation) {
                mEmuFifo.AddDspWrite(0, addr, data);
            }
        }
    }
    return doWrite;
}

void DspController::Process1Sample(int &outl, int &outr)
{
    if (mIsHwAvailable) {
        outl = 0;
        outr = 0;
    }
    else {
        MutexLock(mEmuMtx);
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
            MutexUnlock(mEmuMtx);
        }
        else {
            MutexUnlock(mEmuMtx);
            bool nowrite = false;
            do {
                size_t numWrites = mEmuFifo.GetNumWrites();
                if (numWrites > 0) {
                    DspRegFIFO::DspWrite write = mEmuFifo.PopFront();
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
        MutexLock(mEmuMtx);
#ifndef USE_OPENSPC
        blargg_err_t err = mDsp.play(2, mOutSamples);
        assert(err == NULL);
#else
        OSPC_Run(32, mOutSamples, dspOutBufSize);
#endif
        MutexUnlock(mEmuMtx);
        
        outl = mOutSamples[0];
        outr = mOutSamples[1];
    }
    mSampleInFrame++;
}

void DspController::BeginFrameProcess(double frameTime)
{
    mSampleInFrame = 0;
    
    // バッファにに残っている分の時間を進める
    OSTime nowTime;
    getNowOSTime(nowTime);
    MSTime elapsedTime = calcusTime(nowTime, mPrevFrameStartTime);
#ifdef DEBUG_PRINT
    //std::cout << elapsedTime << std::endl;
#endif
    MutexLock(mHwMtx);
    if (calcusTime(mNextFrameStartTime, nowTime) > 0) {
        mFrameStartTime = mNextFrameStartTime;
    }
    else {
        mFrameStartTime = nowTime;
        mNextFrameStartTime = mFrameStartTime;
        mFrameTime = 0;
    }
    mFrameTime += frameTime;
    MSTime advTime = mFrameTime * 1e6;
    mFrameTime -= advTime / 1000000.0;
    // 経過時間がフレーム時間の半分以下のときはオフライン処理とみなして加算しない
    if (elapsedTime > (advTime / 2)) {
        mNextFrameStartTime += advTime;
        mHwFifo.AddTime(-advTime);
    }
    else {
        mFrameTime = 0;
    }
    MutexUnlock(mHwMtx);
	mHwDelayTime = advTime / 4;
    mPrevFrameStartTime = nowTime;
}

void DspController::StartMuteEmulation()
{
    mMuteEmulation = true;
    while (mWaitPort >= 0) {
        int outl ,outr;
        Process1Sample(outl, outr);
        mSampleInFrame--;
    }
    mEmuFifo.Clear();
}

void DspController::EndMuteEmulation()
{
    mMuteEmulation = false;
    // 動作状態ならDSPの復元
    if (!mIsHwAvailable) {
        for (int i=0; i<128; i++) {
            if (mDspMirror[i] == 0xefefefef) {
                continue;
            }
            if (i == DSP_FLG) {
                continue;
            }
            if (i == DSP_KON) {
                continue;
            }
            WriteDsp(i, mDspMirror[i], true);
        }
    }
}

#ifdef _MSC_VER
DWORD WINAPI DspController::writeHwThreadFunc(LPVOID arg)
#else
void *DspController::writeHwThreadFunc(void *arg)
#endif
{
    DspController   *This = reinterpret_cast<DspController*>(arg);
    while (This->mIsHwAvailable) {
        OSTime nowTime;
        getNowOSTime(nowTime);
        MSTime elapsedTime = calcusTime(nowTime, This->mFrameStartTime);
		bool write = false;
        MutexLock(This->mHwMtx);
        while ((This->mHwFifo.GetNumWrites() > 0) &&
               (This->mHwFifo.GetFrontTime() < elapsedTime)) {
            DspRegFIFO::DspWrite writeData = This->mHwFifo.PopFront();
            if (writeData.isRam) {
                This->doWriteRamHw(writeData.addr, writeData.data);
            }
            else {
                This->doWriteDspHw(writeData.addr, writeData.data);
            }
			write = true;
        }
        This->mSpcDev->WriteBuffer();
        MutexUnlock(This->mHwMtx);
        
		if (!write) {
			WaitMicroSeconds(1000);
		}
    }
    return 0;
}

void DspController::doWriteDspHw(int addr, unsigned char data)
{
    if (mIsHwAvailable) {
        //pthread_mutex_lock(&mHwMtx);
        /*
         if (addr == DSP_EDL) {
         std::cout << "addr:0x" << std::hex << std::setw(2) << std::setfill('0') << addr;
         std::cout << " data:0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data) << std::endl;
         }
         */
        int rewrite = 2;
        if (((addr & 0x0f) < 0x0a) ||
            addr == DSP_KON ||
            addr == DSP_KOF ||
            addr == DSP_FLG) {
            rewrite = 1;
        }
        for (int i=0; i<rewrite; i++) {
            mSpcDev->BlockWrite(1, data, addr & 0xff);
            mSpcDev->WriteAndWait(0, mPort0stateHw);
            mPort0stateHw = mPort0stateHw ^ 0x01;
        }
        //mSpcDev.WriteBuffer();
        //pthread_mutex_unlock(&mHwMtx);
    }
}

void DspController::doWriteRamHw(int addr, unsigned char data)
{
    if (mIsHwAvailable) {
        //pthread_mutex_lock(&mHwMtx);
        mSpcDev->BlockWrite(1, data, addr & 0xff, (addr>>8) & 0xff);
        mSpcDev->WriteAndWait(0, mPort0stateHw | 0x80);
        //mSpcDev.WriteBuffer();
        mPort0stateHw = mPort0stateHw ^ 0x01;
        //pthread_mutex_unlock(&mHwMtx);
    }
}

void DspController::setDeviceReadyFunc( void (*func) (void* ownerClass), void* ownerClass )
{
    mDeviceReadyFunc = func;
    mDeviceReadyFuncClass = ownerClass;
}

void DspController::setDeviceExitFunc( void (*func) (void* ownerClass) , void* ownerClass )
{
    mDeviceExitFunc = func;
    mDeviceExitFuncClass = ownerClass;
}
