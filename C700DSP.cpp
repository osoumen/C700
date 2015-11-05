//
//  C700DSP.cpp
//  C700
//
//  Created by osoumen on 2014/11/30.
//
//

#include "C700DSP.h"
#include "gauss.h"

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

//-----------------------------------------------------------------------------
void C700DSP::DSPState::Reset()
{
	smp1=0;
	smp2=0;
	sampbuf[0]=0;
	sampbuf[1]=0;
	sampbuf[2]=0;
	sampbuf[3]=0;
	/*
	pb = 0;
	reg_pmod = 0;
	vibdepth = 0;
	vibPhase = 0.0f;
    portaPitch = .0f;
    pan = 64;
	*/
	brrdata = silence_brr;
	loopPoint = 0;
	loop = false;
	pitch = 0;
	memPtr = 0;
	headerCnt = 0;
	half = 0;
	envx = 0;
	end = 0;
	sampptr = 0;
	mixfrac=0;
	envcnt = CNT_INIT;
	envstate = RELEASE;
}

C700DSP::C700DSP() : mNewADPCM( false )
{
    //Initialize
	mMainVolume_L = 127;
	mMainVolume_R = 127;
}

C700DSP::~C700DSP()
{
    
}

void C700DSP::ResetVoice(int voice)
{
    mVoice[voice].Reset();
}

void C700DSP::ResetEcho()
{
    mEcho[0].Reset();
	mEcho[1].Reset();
}

void C700DSP::SetVoiceLimit(int value)
{
    mVoiceLimit = value;
}

void C700DSP::SetNewADPCM(bool value)
{
    mNewADPCM = value;
}

void C700DSP::SetMainVolumeL(int value)
{
    mMainVolume_L = value;
}

void C700DSP::SetMainVolumeR(int value)
{
    mMainVolume_R = value;
}

void C700DSP::SetEchoVol_L( int value )
{
	mEcho[0].SetEchoVol( value );
}

void C700DSP::SetEchoVol_R( int value )
{
	mEcho[1].SetEchoVol( value );
}

void C700DSP::SetFeedBackLevel( int value )
{
	mEcho[0].SetFBLevel( value );
	mEcho[1].SetFBLevel( value );
}

void C700DSP::SetDelayTime( int value )
{
	mEcho[0].SetDelayTime( value );
	mEcho[1].SetDelayTime( value );
}

void C700DSP::SetFIRTap( int tap, int value )
{
	mEcho[0].SetFIRTap(tap, value);
	mEcho[1].SetFIRTap(tap, value);
}

void C700DSP::KeyOffVoice(int v)
{
    mVoice[v].envstate = RELEASE;
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
}

void C700DSP::SetAR(int v, int value)
{
    mVoice[v].ar = value;
}

void C700DSP::SetDR(int v, int value)
{
    mVoice[v].dr = value;
}

void C700DSP::SetSL(int v, int value)
{
    mVoice[v].sl = value;
}

void C700DSP::SetSR(int v, int value)
{
    mVoice[v].sr = value;
}

void C700DSP::SetVol_L(int v, int value)
{
    mVoice[v].vol_l = value;
}

void C700DSP::SetVol_R(int v, int value)
{
    mVoice[v].vol_r = value;
}

void C700DSP::SetPitch(int v, int value)
{
    mVoice[v].pitch = value;
}

void C700DSP::SetEchoOn(int v, bool isOn)
{
    mVoice[v].echoOn = isOn;
}

void C700DSP::SetSrcn(int v, int value)
{
    // TODO: v chのsrcnを設定する
}

void C700DSP::setBrr(int v, unsigned char *brrdata, unsigned int loopPoint, bool loop)
{
    mVoice[v].brrdata = brrdata;
    mVoice[v].loopPoint = loopPoint;
    mVoice[v].loop = loop;
}

int C700DSP::TransferBrrData(int srcn, unsigned char *data, int size)
{
    // TODO: srcnにdataをsizeバイト分転送して、転送したアドレスを返す
    
    return 0;
}

void C700DSP::ReleaseBrrData(int srcn)
{
    // TODO: srcnの波形を解放する
}

void C700DSP::Process1Sample(int &outl, int &outr)
{
    int		outx;
    
    for ( int v=0; v<kMaximumVoices; v++ ) {
        outx = 0;
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
            outx = 0;
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
                            outx = 0;
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
                mVoice[v].end = headbyte & 3;
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
        if ( mVoice[v].echoOn ) {
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
