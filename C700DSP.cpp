//
//  C700DSP.cpp
//  C700
//
//  Created by osoumen on 2014/11/30.
//
//

#include "C700DSP.h"
#include "gauss.h"
#include "SmcFileGenerate.h"
#include "SpcFileGenerate.h"
//#include <iomanip>

#define filter1(a1)	(( a1 >> 1 ) + ( ( -a1 ) >> 5 ))
#define filter2(a1,a2)	(a1 + ( ( -( a1 + ( a1 >> 1 ) ) ) >> 5 ) - ( a2 >> 1 ) + ( a2 >> 5 ))
#define filter3(a1,a2)	(a1  + ( ( -( a1 + ( a1 << 2 ) + ( a1 << 3 ) ) ) >> 7 )  - ( a2 >> 1 )  + ( ( a2 + ( a2 >> 1 ) ) >> 4 ))

static const int	*G1 = &gauss[256];
static const int	*G2 = &gauss[512];
static const int	*G3 = &gauss[255];
static const int	*G4 = &gauss[0];

static const int	CNT_INIT = 0x7800;
static const int	ENVCNT[32]
= {
    0x0000, 0x000F, 0x0014, 0x0018, 0x001E, 0x0028, 0x0030, 0x003C,
    0x0050, 0x0060, 0x0078, 0x00A0, 0x00C0, 0x00F0, 0x0140, 0x0180,
    0x01E0, 0x0280, 0x0300, 0x03C0, 0x0500, 0x0600, 0x0780, 0x0A00,
    0x0C00, 0x0F00, 0x1400, 0x1800, 0x1E00, 0x2800, 0x3C00, 0x7800
};

static unsigned char silence_brr[] = {
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#if WIN32
char C700DSP::mPathSeparatorChar = '\\';
#else
char C700DSP::mPathSeparatorChar = '/';
#endif

//-----------------------------------------------------------------------------
void C700DSP::DSPState::Reset()
{
    ar = -1;
    dr = -1;
    sl = -1;
    sr = -1;
    vol_l = -200;
    vol_r = -200;
    ecen = false;
    ecenNotWrited = true;
    pmon = false;
    pmonNotWrited = true;
    non = false;
    nonNotWrited = true;
    
	smp1=0;
	smp2=0;
	sampbuf[0]=0;
	sampbuf[1]=0;
	sampbuf[2]=0;
	sampbuf[3]=0;
	brrdata = silence_brr;
	loopPoint = 0;
	pitch = 0;
	memPtr = 0;
	headerCnt = 0;
	half = 0;
	envx = 0;
	end = 0;
	loop = 0;
	sampptr = 0;
	mixfrac=0;
	envcnt = CNT_INIT;
	envstate = RELEASE;
}

C700DSP::C700DSP() :
mNewADPCM( false ),
mUseRealEmulation( true )
{
    //Initialize
	mMainVolume_L = 127;
	mMainVolume_R = 127;
    mDirAddr = 0x200;
    mEchoVolL = 0;
    mEchoVolR = 0;
    mEchoStartAddr = 0xff;
    mEchoDelay = 0;
    mEchoFeedBack = 0;
    mEchoEnableWait = 0;
    /*
    mEchoEnableWait = 8000;
    gettimeofday(&mEchoChangeTime, NULL);
    mEchoChangeWaitusec = 250000;
     */
    mBrrStartAddr = 0x10000;
    mBrrEndAddr = 0;

    mDsp.setDeviceReadyFunc(onDeviceReady, this);
    mDsp.setDeviceExitFunc(onDeviceStop, this);
    mDsp.init();
    
    mIsLoggerRunning = false;
    
    mSongRecordPath[0] = 0;
    mRecSaveAsSpc = false;
    mRecSaveAsSmc = false;
    mTimeBaseForSmc = SmcTimeBaseNTSC;
    mGameTitle[0] = 0;
    mSongTitle[0] = 0;
    mNameOfDumper[0] = 0;
    mArtistOfSong[0] = 0;
    mSongComments[0] = 0;
    mRepeatNumForSpc = 1;
    mFadeMsTimeForSpc = 5000;
    mSmcNativeVector[0] = 0;
    mSmcEmulationVector[0] = 0;
    mSmcPlayerCode = NULL;
    mSmcPlayerCodeSize = 0;
    mSpcPlayerCode = NULL;
    mSpcPlayerCodeSize = 0;
    mCodeVer = 0;
}

C700DSP::~C700DSP()
{

}

void C700DSP::ResetVoice(int voice)
{
    mVoice[voice].Reset();
    if (voice < 8) {
        writeDsp(DSP_KOF, 1 << voice);
        //writeDsp(DSP_KOF, 0x00);
    }
}

void C700DSP::KeyOffAll()
{
    for (int i=0; i<kMaximumVoices; i++) {
        mVoice[i].Reset();
    }
    writeDsp(DSP_KOF, 0xff);
}

void C700DSP::ResetEcho()
{
    mEcho[0].Reset();
	mEcho[1].Reset();
    // エコー領域のRAMをリセット
    //writeDsp(DSP_EVOLL, 0);
    //writeDsp(DSP_EVOLR, 0);
    //writeDsp(DSP_EFB, 0);
    //writeDsp(DSP_FLG, 0x20);
    /*
    if (mEchoEnableWait < mEchoDelay * 480) {
        mEchoEnableWait = mEchoDelay * 480;
        gettimeofday(&mEchoChangeTime, NULL);
        mEchoChangeWaitusec = mEchoDelay * 16000;
    }
     */
}

void C700DSP::SetVoiceLimit(int value)
{
    mVoiceLimit = value;
}

void C700DSP::SetNewADPCM(bool value)
{
    mNewADPCM = value;
}

void C700DSP::SetRealEmulation(bool value)
{
    mUseRealEmulation = value;
    if (value) {
        mDsp.EndMuteEmulation();
        if (!mDsp.IsHwAvailable()) {
            mDsp.WriteRam(0x200, &mRam[0x200], 0x400);
        }
    }
    else {
        mDsp.StartMuteEmulation();
    }
}

void C700DSP::SetMainVolumeL(int value)
{
    if (mMainVolume_L != value) {
        mMainVolume_L = value;
    }
    writeDsp(DSP_MVOLL, static_cast<unsigned char>(value & 0xff));
}

void C700DSP::SetMainVolumeR(int value)
{
    if (mMainVolume_R != value) {
        mMainVolume_R = value;
    }
    writeDsp(DSP_MVOLR, static_cast<unsigned char>(value & 0xff));
}

void C700DSP::SetEchoVol_L( int value )
{
    if (mEchoVolL != value) {
        mEchoVolL = value & 0xff;
        mEcho[0].SetEchoVol( value );
    }
    if (mEchoEnableWait <= 0) {
        writeDsp(DSP_EVOLL, static_cast<unsigned char>(mEchoVolL));
    }
}

void C700DSP::SetEchoVol_R( int value )
{
    if (mEchoVolR != value) {
        mEchoVolR = value & 0xff;
        mEcho[1].SetEchoVol( value );
    }
    if (mEchoEnableWait <= 0) {
        writeDsp(DSP_EVOLR, static_cast<unsigned char>(mEchoVolR));
    }
}

void C700DSP::SetFeedBackLevel( int value )
{
    if (mEchoFeedBack != value) {
        mEchoFeedBack = value & 0xff;
        mEcho[0].SetFBLevel( value );
        mEcho[1].SetFBLevel( value );
    }
    if (mEchoEnableWait <= 0) {
        writeDsp(DSP_EFB, static_cast<unsigned char>(mEchoFeedBack));
    }
}

void C700DSP::SetDelayTime( int value )
{
    //std::cout << "edl:0x" << std::hex << std::setw(2) << std::setfill('0') << value << std::endl;
    //assert(mEchoDelay != value);
    if (mEchoDelay != value) {
        mEchoDelay = value & 0xff;
        mEchoStartAddr = 0x06;  // DIRの直後
        
        mEcho[0].SetDelayTime( value );
        mEcho[1].SetDelayTime( value );
    }
    //writeDsp(DSP_EVOLL, 0);
    //writeDsp(DSP_EVOLR, 0);
    //writeDsp(DSP_EFB, 0);
    //writeDsp(DSP_FLG, 0x20);
    //writeDsp(DSP_EDL, 0);
    
    //writeDsp(DSP_ESA, static_cast<unsigned char>(mEchoStartAddr));
    writeDsp(DSP_EDL, static_cast<unsigned char>(mEchoDelay));
    /*
     mEchoEnableWait = 8000; // 250ms
     gettimeofday(&mEchoChangeTime, NULL);
     mEchoChangeWaitusec = 250000;
     */
}

void C700DSP::SetFIRTap( int tap, int value )
{
	mEcho[0].SetFIRTap(tap, value);
	mEcho[1].SetFIRTap(tap, value);
    writeDsp(DSP_FIR + 0x10*tap, static_cast<unsigned char>(value & 0xff));
}

void C700DSP::KeyOffVoice(int v)
{
    mVoice[v].envstate = RELEASE;
    writeDsp(DSP_KOF, static_cast<unsigned char>(0x01 << v));
    //writeDsp(DSP_KOF, 0);  // ドライバ側で行う
}

void C700DSP::KeyOffVoiceFlg(int flg)
{
    for (int v=0; v<kMaximumVoices; v++) {
        if (flg & (1 << v)) {
            mVoice[v].envstate = RELEASE;
        }
    }
    writeDsp(DSP_KOF, static_cast<unsigned char>(flg & 0xff));
    //writeDsp(DSP_KOF, 0);  // ドライバ側で行う
}

void C700DSP::KeyOnVoice(int v)
{
    mVoice[v].memPtr = 0;
    mVoice[v].headerCnt = 0;
    mVoice[v].half = 0;
    mVoice[v].envx = 0;
    mVoice[v].end = 0;
    mVoice[v].sampptr = 0;
    mVoice[v].mixfrac = 3 * 4096;
    mVoice[v].envcnt = CNT_INIT;
    mVoice[v].envstate = ATTACK;
    writeDsp(DSP_KON, static_cast<unsigned char>(0x01 << v));
}

void C700DSP::KeyOnVoiceFlg(int flg)
{
    for (int v=0; v<kMaximumVoices; v++) {
        if (flg & (1 << v)) {
            mVoice[v].memPtr = 0;
            mVoice[v].headerCnt = 0;
            mVoice[v].half = 0;
            mVoice[v].envx = 0;
            mVoice[v].end = 0;
            mVoice[v].sampptr = 0;
            mVoice[v].mixfrac = 3 * 4096;
            mVoice[v].envcnt = CNT_INIT;
            mVoice[v].envstate = ATTACK;
        }
    }
    writeDsp(DSP_KON, static_cast<unsigned char>(flg & 0xff));
}

void C700DSP::SetAR(int v, int value)
{
    unsigned char data = 0x80;
    data |= value & 0x0f;
    data |= (mVoice[v].dr & 0x07) << 4;
    if (mVoice[v].ar != value) {
        mVoice[v].ar = value;
    }
    if (v < 8) {
        writeDsp(DSP_ADSR + 0x10*v, data);
    }
}

void C700DSP::SetDR(int v, int value)
{
    unsigned char data = 0x80;
    data |= mVoice[v].ar & 0x0f;
    data |= (value & 0x07) << 4;
    if (mVoice[v].dr != value) {
        mVoice[v].dr = value;
    }
    if (v < 8) {
        writeDsp(DSP_ADSR + 0x10*v, data);
    }
}

void C700DSP::SetARDR(int v, int ar, int dr)
{
    unsigned char data = 0x80;
    data |= ar & 0x0f;
    data |= (dr & 0x07) << 4;
    if (mVoice[v].ar != ar) {
        mVoice[v].ar = ar;
    }
    if (mVoice[v].dr != dr) {
        mVoice[v].dr = dr;
    }
    if (v < 8) {
        writeDsp(DSP_ADSR + 0x10*v, data);
    }
}

void C700DSP::SetSL(int v, int value)
{
    unsigned char data = 0;
    data |= mVoice[v].sr & 0x1f;
    data |= (value & 0x07) << 5;
    if (mVoice[v].sl != value) {
        mVoice[v].sl = value;
    }
    if (v < 8) {
        writeDsp(DSP_ADSR+1 + 0x10*v, data);
    }
}

void C700DSP::SetSR(int v, int value)
{
    unsigned char data = 0;
    data |= value & 0x1f;
    data |= (mVoice[v].sl & 0x07) << 5;
    if (mVoice[v].sr != value) {
        mVoice[v].sr = value;
    }
    if (v < 8) {
        writeDsp(DSP_ADSR+1 + 0x10*v, data);
    }
}

void C700DSP::SetSLSR(int v, int sl, int sr)
{
    unsigned char data = 0;
    data |= sr & 0x1f;
    data |= (sl & 0x07) << 5;
    if (mVoice[v].sl != sl) {
        mVoice[v].sl = sl;
    }
    if (mVoice[v].sr != sr) {
        mVoice[v].sr = sr;
    }
    if (v < 8) {
        writeDsp(DSP_ADSR+1 + 0x10*v, data);
    }
}

void C700DSP::SetVol_L(int v, int value)
{
    if (mVoice[v].vol_l != value) {
        mVoice[v].vol_l = value;
    }
    if (v < 8) {
        writeDsp(DSP_VOL + 0x10*v, static_cast<unsigned char>(value));
    }
}

void C700DSP::SetVol_R(int v, int value)
{
    if (mVoice[v].vol_r != value) {
        mVoice[v].vol_r = value;
    }
    if (v < 8) {
        writeDsp(DSP_VOL+1 + 0x10*v, static_cast<unsigned char>(value));
    }
}

void C700DSP::SetPitch(int v, int value)
{
    if (mVoice[v].pitch != value) {
        mVoice[v].pitch = value;
    }
    if (v < 8) {
        int addr_l = DSP_P + 0x10*v;
        int addr_m = DSP_P+1 + 0x10*v;
        unsigned char data_l = static_cast<unsigned char>(value&0xff);
        unsigned char data_m = static_cast<unsigned char>((value>>8)&0x3f);
        
        if ( mIsLoggerRunning ) {
            mLogger.DumpApuPitch(0, addr_l, data_l, data_m, mLoggerSamplePos);
        }
        mDsp.WriteDsp(addr_l, data_l, false);
        mDsp.WriteDsp(addr_m, data_m, false);
    }
}

void C700DSP::SetEchoOn(int v, bool isOn)
{
    if ((mVoice[v].ecen != isOn) || mVoice[v].ecenNotWrited) {
        mVoice[v].ecen = isOn;
        mVoice[v].ecenNotWrited = false;
    }
    unsigned char data = 0;
    for (int i=0; i<8; i++) {
        if (mVoice[i].ecen) {
            data |= 1 << i;
        }
    }
    writeDsp(DSP_EON, data);
}

void C700DSP::SetEchoOnFlg(int flg, int mask)
{
    unsigned char data = 0;
    for (int i=0; i<8; i++) {
        if (mVoice[i].ecen) {
            data |= 1 << i;
        }
    }
    data = (data & ~mask) | (flg & mask);
    
    for (int v=0; v<8; v++) {
        bool isOn = (data & (1 << v))?true:false;
        if ((mVoice[v].ecen != isOn) || mVoice[v].ecenNotWrited) {
            mVoice[v].ecen = isOn;
            mVoice[v].ecenNotWrited = false;
        }
    }
    writeDsp(DSP_EON, data);
}

void C700DSP::SetPMOn(int v, bool isOn)
{
    if ((mVoice[v].pmon != isOn) || mVoice[v].pmonNotWrited) {
        mVoice[v].pmon = isOn;
        mVoice[v].pmonNotWrited = false;
    }
    unsigned char data = 0;
    for (int i=0; i<8; i++) {
        if (mVoice[i].pmon) {
            data |= 1 << i;
        }
    }
    writeDsp(DSP_PMON, data);
}

void C700DSP::SetPMOnFlg(int flg, int mask)
{
    unsigned char data = 0;
    for (int i=0; i<8; i++) {
        if (mVoice[i].pmon) {
            data |= 1 << i;
        }
    }
    data = (data & ~mask) | (flg & mask);
    
    for (int v=0; v<8; v++) {
        bool isOn = (data & (1 << v))?true:false;
        if ((mVoice[v].pmon != isOn) || mVoice[v].pmonNotWrited) {
            mVoice[v].pmon = isOn;
            mVoice[v].pmonNotWrited = false;
        }
    }
    writeDsp(DSP_PMON, data);
}

void C700DSP::SetNoiseOn(int v, bool isOn)
{
    if ((mVoice[v].non != isOn) || mVoice[v].nonNotWrited) {
        mVoice[v].non = isOn;
        mVoice[v].nonNotWrited = false;
    }
    unsigned char data = 0;
    for (int i=0; i<8; i++) {
        if (mVoice[i].non) {
            data |= 1 << i;
        }
    }
    writeDsp(DSP_NON, data);
}

void C700DSP::SetNoiseOnFlg(int flg, int mask)
{
    unsigned char data = 0;
    for (int i=0; i<8; i++) {
        if (mVoice[i].non) {
            data |= 1 << i;
        }
    }
    data = (data & ~mask) | (flg & mask);
    
    for (int v=0; v<8; v++) {
        bool isOn = (data & (1 << v))?true:false;
        if ((mVoice[v].non != isOn) || mVoice[v].nonNotWrited) {
            mVoice[v].non = isOn;
            mVoice[v].nonNotWrited = false;
        }
    }
    writeDsp(DSP_NON, data);
}

void C700DSP::SetSrcn(int v, int value)
{
    // v chのsrcnからbrrdata,loopPointを設定する
    int brrPtr = mDirAddr + value * 4;
    int loopPtr = brrPtr + 2;
    int brrAddr = mRam[brrPtr+1] * 256 + mRam[brrPtr];
    int loopAddr = mRam[loopPtr+1] * 256 + mRam[loopPtr];
    
    setBrr( v, &mRam[brrAddr], loopAddr - brrAddr);
    if (v < 8) {
        writeDsp(DSP_SRCN + 0x10*v, static_cast<unsigned char>(value&0xff));
    }
}

void C700DSP::SetDir(int value)
{
    mDirAddr = (value & 0xff) << 8;
    writeDsp(DSP_DIR, static_cast<unsigned char>(value&0xff));
}

void C700DSP::SetNoiseFreq(int value)
{
    // TODO: D5-7を考慮
    writeDsp(DSP_FLG, static_cast<unsigned char>(value&0x1f));
}

void C700DSP::setBrr(int v, unsigned char *brrdata, unsigned int loopPoint)
{
    mVoice[v].brrdata = brrdata;
    mVoice[v].loopPoint = loopPoint;
}

void C700DSP::WriteRam(int addr, const unsigned char *data, int size)
{
    // addrにdataをsizeバイト分転送する
    int startaddr = 0;
    int endaddr = addr + size;
    if (addr > startaddr) {
        startaddr = addr;
    }
    if (endaddr > 0x10000) {
        endaddr = 0x10000;
    }
    const unsigned char *srcPtr = data;
    for (int i=startaddr; i<endaddr; i++) {
        mRam[i] = *srcPtr;
        srcPtr++;
    }
    mDsp.WriteRam(addr, data, size);
    
    if (mBrrStartAddr > startaddr) {
        mBrrStartAddr = startaddr;
    }
    if (mBrrEndAddr < endaddr) {
        mBrrEndAddr = endaddr;
    }
}

void C700DSP::WriteRam(int addr, unsigned char data)
{
    mRam[addr & 0xffff] = data;
    mDsp.WriteRam(addr, data, false);
}

void C700DSP::Process1Sample(int &outl, int &outr)
{
    if (mUseRealEmulation) {
        mDsp.Process1Sample(outl, outr);
    }
    else if (!mDsp.IsHwAvailable()) {
        int		outx;
        
        for ( int v=0; v<kMaximumVoices; v++ ) {
            //outx = 0;
            //--
            {
                switch( mVoice[v].envstate ) {
                    case ATTACK:
                        if ( mVoice[v].ar == 15 ) {
                            mVoice[v].envx += 0x400;
                        }
                        else {
                            mVoice[v].envcnt -= ENVCNT[ ( mVoice[v].ar << 1 ) + 1 ];
                            if ( mVoice[v].envcnt > 0 ) {
                                break;
                            }
                            mVoice[v].envx += 0x20;       /* 0x020 / 0x800 = 1/64         */
                            mVoice[v].envcnt = CNT_INIT;
                        }
                        
                        if ( mVoice[v].envx > 0x7FF ) {
                            mVoice[v].envx = 0x7FF;
                            mVoice[v].envstate = DECAY;
                        }
                        break;
                        
                    case DECAY:
                        mVoice[v].envcnt -= ENVCNT[ mVoice[v].dr*2 + 0x10 ];
                        if ( mVoice[v].envcnt <= 0 ) {
                            mVoice[v].envcnt = CNT_INIT;
                            mVoice[v].envx -= ( ( mVoice[v].envx - 1 ) >> 8 ) + 1;
                        }
                        
                        if ( mVoice[v].envx <= 0x100 * ( mVoice[v].sl + 1 ) ) {
                            mVoice[v].envstate = SUSTAIN;
                        }
                        break;
                        
                    case SUSTAIN:
                        mVoice[v].envcnt -= ENVCNT[ mVoice[v].sr ];
                        if ( mVoice[v].envcnt > 0 ) {
                            break;
                        }
                        mVoice[v].envcnt = CNT_INIT;
                        mVoice[v].envx -= ( ( mVoice[v].envx - 1 ) >> 8 ) + 1;
                        break;
                        
                    case RELEASE:
                        mVoice[v].envx -= 0x8;
                        if ( mVoice[v].envx <= 0 ) {
                            mVoice[v].envx = -1;
                        }
                        break;
                }
            }
            
            if ( mVoice[v].envx < 0 ) {
                //outx = 0;
                continue;
            }
            
            for( ; mVoice[v].mixfrac >= 0; mVoice[v].mixfrac -= 4096 ) {
                if( !mVoice[v].headerCnt ) {	//ブロックの始まり
                    if( mVoice[v].end & 1 ) {	//データ終了フラグあり
                        if( mVoice[v].loop ) {
                            mVoice[v].memPtr = mVoice[v].loopPoint;	//読み出し位置をループポイントまで戻す
                        }
                        else {	//ループなし
                            mVoice[v].envx = 0;
                            while( mVoice[v].mixfrac >= 0 ) {
                                mVoice[v].sampbuf[mVoice[v].sampptr] = 0;
                                //outx = 0;
                                mVoice[v].sampptr  = ( mVoice[v].sampptr + 1 ) & 3;
                                mVoice[v].mixfrac -= 4096;
                            }
                            break;
                        }
                    }
                    
                    //開始バイトの情報を取得
                    mVoice[v].headerCnt = 8;
                    int headbyte = ( unsigned char )mVoice[v].brrdata[mVoice[v].memPtr++];
                    mVoice[v].range = headbyte >> 4;
                    mVoice[v].end = headbyte & 1;
                    mVoice[v].loop = headbyte & 2;
                    mVoice[v].filter = ( headbyte & 12 ) >> 2;
                }
                
                if ( mVoice[v].half == 0 ) {
                    mVoice[v].half = 1;
                    outx = ( ( signed char )mVoice[v].brrdata[ mVoice[v].memPtr ] ) >> 4;
                }
                else {
                    mVoice[v].half = 0;
                    outx = ( signed char )( mVoice[v].brrdata[ mVoice[v].memPtr++ ] << 4 );
                    outx >>= 4;
                    mVoice[v].headerCnt--;
                }
                //outx:4bitデータ
                
                if ( mVoice[v].range <= 0xC ) {
                    outx = ( outx << mVoice[v].range ) >> 1;
                }
                else {
                    outx &= ~0x7FF;
                }
                //outx:4bitデータ*Range
                
                switch( mVoice[v].filter ) {
                    case 0:
                        break;
                        
                    case 1:
                        outx += filter1(mVoice[v].smp1);
                        break;
                        
                    case 2:
                        outx += filter2(mVoice[v].smp1,mVoice[v].smp2);
                        break;
                        
                    case 3:
                        outx += filter3(mVoice[v].smp1,mVoice[v].smp2);
                        break;
                }
                
                // フィルタ後にクリップ
                if ( outx < -32768 ) {
                    outx = -32768;
                }
                else if ( outx > 32767 ) {
                    outx = 32767;
                }
                // y[-1]へ送る際に２倍されたらクランプ
                if (mNewADPCM) {
                    mVoice[v].smp2 = mVoice[v].smp1;
                    mVoice[v].smp1 = ( signed short )( outx << 1 );
                }
                else {
                    // 古いエミュレータの一部にはクランプを行わないものもある
                    // 音は実機と異なる
                    mVoice[v].smp2 = mVoice[v].smp1;
                    mVoice[v].smp1 = outx << 1;
                }
                mVoice[v].sampbuf[mVoice[v].sampptr] = mVoice[v].smp1;
                mVoice[v].sampptr = ( mVoice[v].sampptr + 1 ) & 3;
            }
            
            int fracPos = mVoice[v].mixfrac >> 4;
            int smpl = ( ( G4[ -fracPos - 1 ] * mVoice[v].sampbuf[ mVoice[v].sampptr ] ) >> 11 ) & ~1;
            smpl += ( ( G3[ -fracPos ]
                       * mVoice[v].sampbuf[ ( mVoice[v].sampptr + 1 ) & 3 ] ) >> 11 ) & ~1;
            smpl += ( ( G2[ fracPos ]
                       * mVoice[v].sampbuf[ ( mVoice[v].sampptr + 2 ) & 3 ] ) >> 11 ) & ~1;
            // openspcではなぜかここでもクランプさせていた
            // ここも無いと実機と違ってしまう
            if (mNewADPCM) {
                smpl = ( signed short )smpl;
            }
            smpl += ( ( G1[ fracPos ]
                       * mVoice[v].sampbuf[ ( mVoice[v].sampptr + 3 ) & 3 ] ) >> 11 ) & ~1;
            
            // ガウス補間後にクリップ
            if ( smpl > 32767 ) {
                smpl = 32767;
            }
            else if ( smpl < -32768 ) {
                smpl = -32768;
            }
            outx = smpl;
            
            mVoice[v].mixfrac += mVoice[v].pitch;
            
            outx = ( ( outx * mVoice[v].envx ) >> 11 ) & ~1;
            
            // ゲイン値の反映
            int vl = ( mVoice[v].vol_l * outx ) >> 7;
            int vr = ( mVoice[v].vol_r * outx ) >> 7;
            
            // エコー処理
            if ( mVoice[v].ecen ) {
                mEcho[0].Input(vl);
                mEcho[1].Input(vr);
            }
            //メインボリュームの反映
            outl += ( vl * mMainVolume_L ) >> 7;
            outr += ( vr * mMainVolume_R ) >> 7;
        }
        outl += mEcho[0].GetFxOut();
        outr += mEcho[1].GetFxOut();
    }
    else {
        outl = 0;
        outr = 0;
		mDsp.IncSampleInFrame();
    }
    if ( mIsLoggerRunning ) {
        mLoggerSamplePos++;
    }
}

void C700DSP::BeginFrameProcess(double frameTime)
{
    mDsp.BeginFrameProcess(frameTime);
}

bool C700DSP::writeDsp(int addr, unsigned char data)
{
    //レジスタをログへ
	if ( mIsLoggerRunning ) {
		mLogger.DumpReg(0, addr, data, mLoggerSamplePos);
	}
    
    return mDsp.WriteDsp(addr, data, false);
}

void C700DSP::BeginRegisterLog()
{
	mLoggerSamplePos = 0;
    mLogger.SetProcessSampleRate(32000);
	mLogger.BeginDump(0);
	mIsLoggerRunning = true;
    
    // DIR領域の設定
    {
        mLogger.addDirRegion(0x200, 0x400, &mRam[0x200]);
    }
    // 波形領域の設定
    {
        mLogger.addBrrRegion(mBrrStartAddr, mBrrEndAddr - mBrrStartAddr, &mRam[mBrrStartAddr]);
    }
    // 演奏開始時点のDSP領域の設定
    {
        unsigned char dspreg[256];
        for (int i=0; i<128; i++) {
            int reg = mDsp.GetDspMirror(i);
            if (reg >= 0 && reg <= 0xff) {
                dspreg[i] = reg;
            }
            else {
                dspreg[i] = 0;
            }
        }
        mLogger.addDspRegRegion(dspreg);
    }
    // 現在のレジスタ値をログに出力
    mLogger.BeginDspInitialization();
    for (int i=0; i<128; i++) {
        int reg = mDsp.GetDspMirror(i);
        if ((i & 0x0f) == 0x03) {
            mLogger.DumpApuPitch(0, i-1, mDsp.GetDspMirror(i-1), reg, 0);
            continue;
        }
        if ((i & 0x0f) == 0x02) {
            continue;
        }
        if (reg >= 0 && reg <= 0xff) {
            mLogger.DumpReg(0, i, reg, 0);
        }
    }
    mLogger.EndDspInitialization();
}

void C700DSP::MarkRegisterLogLoop()
{
	if ( mIsLoggerRunning ) {
        
		mLogger.MarkLoopPoint();
	}
}

void C700DSP::EndRegisterLog()
{
	if ( mIsLoggerRunning ) {
		mLogger.EndDump(mLoggerSamplePos);
		mIsLoggerRunning = false;
        
        // ファイルへ書き出しテスト
        saveRegisterLog(mSongRecordPath);
	}
}

int C700DSP::saveRegisterLog(const char *path)
{
	if ( *path == 0 ) {
		return(-1);
	}
    if (path[0] == 0) {
        // 保存パスが未設定
        return(-1);
    }
	if ( canSaveRegisterLog() == false ) {
		return(-1);
	}
    
    char saveFilePath[PATH_LEN_MAX];
    strncpy(saveFilePath, path, PATH_LEN_MAX);
    strncat(saveFilePath, &mPathSeparatorChar, 1);
    if (mSongTitle[0] != 0) {
        strncat(saveFilePath, mSongTitle, 32);
    }
    else if (mGameTitle[0] != 0) {
        strncat(saveFilePath, mGameTitle, 32);
    }
    else {
        time_t timer;
        struct tm *local;
        timer = time(NULL);
        local = localtime(&timer);
        char dateStr[16];
        sprintf(dateStr, "%04d%02d%02d%02d%02d", local->tm_year + 1900, local->tm_mon + 1, local->tm_mday, local->tm_hour, local->tm_min);
        strncat(saveFilePath, "c700song", 32);
        strncat(saveFilePath, dateStr, 12);
    }
    //PlayingFileGenerateBase exporter;
    //exporter.WriteToFile(path, mLogger, 16000);
    
    // 設定でチェックを入れているフォーマットだけ出力する
    if (mRecSaveAsSmc) {
        char targetFilePath[PATH_LEN_MAX];
        strncpy(targetFilePath, saveFilePath, PATH_LEN_MAX);
        strncat(targetFilePath, ".smc", 4);
        SmcFileGenerate exporter;
        exporter.SetSmcPlayCode(mSmcPlayerCode, mSmcPlayerCodeSize, mSmcNativeVector, mSmcEmulationVector);
        exporter.SetGameTitle(mGameTitle);
        if (mTimeBaseForSmc == SmcTimeBaseNTSC) {
            exporter.SetCountryCode(0);
            exporter.WriteToFile(targetFilePath, mLogger, 15734);
        }
        else if (mTimeBaseForSmc == SmcTimeBasePAL) {
            exporter.SetCountryCode(2);
            exporter.WriteToFile(targetFilePath, mLogger, 15625);
        }
    }
    if (mRecSaveAsSpc) {
        char targetFilePath[PATH_LEN_MAX];
        strncpy(targetFilePath, saveFilePath, PATH_LEN_MAX);
        strncat(targetFilePath, ".spc", 4);
        SpcFileGenerate exporter;
        exporter.SetSpcPlayCode(mSpcPlayerCode, mSpcPlayerCodeSize);
        exporter.SetGameTitle(mGameTitle);
        exporter.SetSongTitle(mSongTitle);
        exporter.SetNameOfDumper(mNameOfDumper);
        exporter.SetArtistOfSong(mArtistOfSong);
        exporter.SetSongComments(mSongComments);
        exporter.SetPlaySeconds(ceil(mLogger.CalcBeforeLoopTime() + mLogger.CalcAfterLoopTime() * mRepeatNumForSpc));
        exporter.SetFadeMs(mFadeMsTimeForSpc);
        exporter.WriteToFile(targetFilePath, mLogger);
    }
	return 0;
}

bool C700DSP::canSaveRegisterLog()
{
	if ( mIsLoggerRunning == false && mLogger.IsEnded() ) {
		return true;
	}
	return false;
}

void C700DSP::onDeviceReady(void *ref)
{
    C700DSP   *This = reinterpret_cast<C700DSP*>(ref);
    // RAMを転送
    int writeBytes = This->mBrrEndAddr - This->mBrrStartAddr;
    if (writeBytes > 0) {
        // DIR領域を転送
        This->mDsp.WriteRam(0x200, &This->mRam[0x200], 0x400);
        // 波形領域を転送
        This->mDsp.WriteRam(This->mBrrStartAddr, &This->mRam[This->mBrrStartAddr], writeBytes);
    }
}

void C700DSP::onDeviceStop(void *ref)
{
    C700DSP   *This = reinterpret_cast<C700DSP*>(ref);
    
    // もしハード側にだけ書き込まれていたような場合のためにDIR領域を転送
    This->mDsp.WriteRam(0x200, &This->mRam[0x200], 0x400);
}

void C700DSP::SetSongRecordPath(const char *path)
{
    strncpy(mSongRecordPath, path, PATH_LEN_MAX-1);
    mSongRecordPath[PATH_LEN_MAX-1] = 0;
}

void C700DSP::SetRecSaveAsSpc(bool enable)
{
    mRecSaveAsSpc = enable;
}

void C700DSP::SetRecSaveAsSmc(bool enable)
{
    mRecSaveAsSmc = enable;
}

void C700DSP::SetTimeBaseForSmc(SmcTimeBase timebase)
{
    mTimeBaseForSmc = timebase;
}

void C700DSP::SetGameTitle(const char *title)
{
    strncpy(mGameTitle, title, 32);
    mGameTitle[32] = 0;
}

void C700DSP::SetSongTitle(const char *title)
{
    strncpy(mSongTitle, title, 32);
    mSongTitle[32] = 0;
}

void C700DSP::SetNameOfDumper(const char *dumper)
{
    strncpy(mNameOfDumper, dumper, 16);
    mNameOfDumper[16] = 0;
}

void C700DSP::SetArtistOfSong(const char *artist)
{
    strncpy(mArtistOfSong, artist, 32);
    mArtistOfSong[32] = 0;
}

void C700DSP::SetSongComments(const char *comments)
{
    strncpy(mSongComments, comments, 32);
    mSongComments[32] = 0;
}

void C700DSP::SetSmcNativeVector(const void *vec)
{
    memcpy(mSmcNativeVector, vec, 12);
}

void C700DSP::SetSmcEmulationVector(const void *vec)
{
    memcpy(mSmcEmulationVector, vec, 12);
}

void C700DSP::SetSmcPlayerCode(const void *code, int size)
{
    if (mSmcPlayerCode != NULL) {
        delete mSmcPlayerCode;
    }
    mSmcPlayerCodeSize = size;
    mSmcPlayerCode = new unsigned char[size];
    memcpy(mSmcPlayerCode, code, size);
}

void C700DSP::SetSpcPlayerCode(const void *code, int size)
{
    if (mSpcPlayerCode != NULL) {
        delete mSpcPlayerCode;
    }
    mSpcPlayerCodeSize = size;
    mSpcPlayerCode = new unsigned char[size];
    memcpy(mSpcPlayerCode, code, size);
}

int C700DSP::GetSongPlayCodeVer()
{
    return mCodeVer;
}

void C700DSP::SetSongPlayCodeVer(int ver)
{
    mCodeVer = ver;
}

void C700DSP::SetRepeatNumForSpc(float num)
{
    mRepeatNumForSpc = num;
}

void C700DSP::SetFadeMsTimeForSpc(int time)
{
    mFadeMsTimeForSpc = time;
}
