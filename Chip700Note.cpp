/*
 *  Chip700Note.cpp
 *  Chip700
 *
 *  Created by 開発用 on 06/09/06.
 *  Copyright 2006 Vermicelli Magic. All rights reserved.
 *
 */

#include "Chip700.h"
#include "Chip700Note.h"
#include "gauss.h"

#define filter1(a1)	(( a1 >> 1 ) + ( ( -a1 ) >> 5 ))
#define filter2(a1,a2)	(a1 \
						 + ( ( -( a1 + ( a1 >> 1 ) ) ) >> 5 ) \
						 - ( a2 >> 1 ) + ( a2 >> 5 ))
#define filter3(a1,a2)	(a1 \
						 + ( ( -( a1 + ( a1 << 2 ) \
								  + ( a1 << 3 ) ) ) >> 7 ) \
						 - ( a2 >> 1 ) \
						 + ( ( a2 + ( a2 >> 1 ) ) >> 4 ))

const float onepi = 3.14159265358979;

static const int	*G1 = &gauss[256];
static const int	*G2 = &gauss[512];
static const int	*G3 = &gauss[255];
static const int	*G4 = &gauss[-1];

static const int	CNT_INIT = 0x7800;
static const int	ENVCNT[32]
= {
    0x0000, 0x000F, 0x0014, 0x0018, 0x001E, 0x0028, 0x0030, 0x003C,
    0x0050, 0x0060, 0x0078, 0x00A0, 0x00C0, 0x00F0, 0x0140, 0x0180,
    0x01E0, 0x0280, 0x0300, 0x03C0, 0x0500, 0x0600, 0x0780, 0x0A00,
    0x0C00, 0x0F00, 0x1400, 0x1800, 0x1E00, 0x2800, 0x3C00, 0x7800
};

static const int	VELOCITY_CURB[128]
= {
	0x000, 0x001, 0x001, 0x001, 0x002, 0x003, 0x005, 0x006, 0x008, 0x00a, 0x00d, 0x00f, 0x012, 0x015, 0x019, 0x01d, 
	0x020, 0x025, 0x029, 0x02e, 0x033, 0x038, 0x03d, 0x043, 0x049, 0x04f, 0x056, 0x05d, 0x064, 0x06b, 0x072, 0x07a, 
	0x082, 0x08a, 0x093, 0x09b, 0x0a4, 0x0ae, 0x0b7, 0x0c1, 0x0cb, 0x0d5, 0x0e0, 0x0eb, 0x0f6, 0x101, 0x10d, 0x118, 
	0x124, 0x131, 0x13d, 0x14a, 0x157, 0x165, 0x172, 0x180, 0x18e, 0x19c, 0x1ab, 0x1ba, 0x1c9, 0x1d8, 0x1e8, 0x1f8, 
	0x208, 0x218, 0x229, 0x23a, 0x24b, 0x25c, 0x26e, 0x280, 0x292, 0x2a4, 0x2b7, 0x2ca, 0x2dd, 0x2f0, 0x304, 0x318, 
	0x32c, 0x341, 0x355, 0x36a, 0x380, 0x395, 0x3ab, 0x3c1, 0x3d7, 0x3ed, 0x404, 0x41b, 0x432, 0x44a, 0x461, 0x479, 
	0x492, 0x4aa, 0x4c3, 0x4dc, 0x4f5, 0x50f, 0x528, 0x542, 0x55d, 0x577, 0x592, 0x5ad, 0x5c8, 0x5e4, 0x600, 0x61c, 
	0x638, 0x655, 0x671, 0x68e, 0x6ac, 0x6c9, 0x6e7, 0x705, 0x724, 0x742, 0x761, 0x780, 0x79f, 0x7bf, 0x7df, 0x7ff
	/*
	0x000, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x002, 0x002, 0x003,
	0x004, 0x004, 0x005, 0x006, 0x007, 0x009, 0x00a, 0x00c, 0x00d, 0x00f, 0x011, 0x013, 0x015, 0x018, 0x01a, 0x01d,
	0x020, 0x023, 0x027, 0x02a, 0x02e, 0x032, 0x036, 0x03b, 0x03f, 0x044, 0x04a, 0x04f, 0x055, 0x05b, 0x061, 0x067,
	0x06e, 0x075, 0x07c, 0x084, 0x08c, 0x094, 0x09d, 0x0a6, 0x0af, 0x0b9, 0x0c2, 0x0cd, 0x0d7, 0x0e2, 0x0ee, 0x0f9,
	0x105, 0x112, 0x11f, 0x12c, 0x13a, 0x148, 0x156, 0x165, 0x174, 0x184, 0x194, 0x1a5, 0x1b6, 0x1c8, 0x1da, 0x1ec,
	0x1ff, 0x213, 0x226, 0x23b, 0x250, 0x265, 0x27b, 0x292, 0x2a9, 0x2c0, 0x2d8, 0x2f1, 0x30a, 0x323, 0x33e, 0x358,
	0x374, 0x390, 0x3ac, 0x3c9, 0x3e7, 0x405, 0x424, 0x443, 0x464, 0x484, 0x4a6, 0x4c8, 0x4ea, 0x50e, 0x532, 0x556,
	0x57b, 0x5a1, 0x5c8, 0x5ef, 0x617, 0x640, 0x669, 0x694, 0x6be, 0x6ea, 0x716, 0x743, 0x771, 0x79f, 0x7cf, 0x7ff
	 */
};

static const int sinctable[] = {
	-8,31,0,-462,1911,-4438,7057,32767,7057,-4438,1911,-462,0,31,-8,0,
	0,18,46,-535,1858,-3768,4443,32510,9863,-4978,1871,-347,-57,45,-9,0,
	0,7,80,-572,1726,-3016,2076,31745,12796,-5345,1725,-193,-125,61,-10,0,
	0,0,104,-574,1532,-2226,0,30497,15785,-5492,1464,0,-200,78,-11,0,
	0,-5,117,-548,1294,-1437,-1755,28803,18752,-5378,1087,227,-279,93,-11,0,
	0,-9,120,-500,1029,-685,-3169,26715,21615,-4969,596,481,-360,107,-11,0,
	0,-11,116,-435,753,0,-4239,24296,24296,-4239,0,753,-435,116,-11,0,
	0,-11,107,-360,481,596,-4969,21615,26715,-3169,-685,1029,-500,120,-9,0,
	0,-11,93,-279,227,1087,-5378,18752,28803,-1755,-1437,1294,-548,117,-5,0,
	0,-11,78,-200,0,1464,-5492,15785,30497,0,-2226,1532,-574,104,0,0,
	0,-10,61,-125,-193,1725,-5345,12796,31745,2076,-3016,1726,-572,80,7,0,
	0,-9,45,-57,-347,1871,-4978,9863,32510,4443,-3768,1858,-535,46,18,0,
	0,-8,31,0,-462,1911,-4438,7057,32767,7057,-4438,1911,-462,0,31,-8
};

static unsigned char silence_brr[] = {
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void Chip700Note::Reset()
{
	SynthNote::Reset();

	mInternalClock=32000;
	for (int i=0; i<16; i++)
		mProcessbuf[i]=0;
	mProcessFrac=0;
	mProcessbufPtr=0;
	
	mSmp1=0;
	mSmp2=0;
	mSampbuf[0]=0;
	mSampbuf[1]=0;
	mSampbuf[2]=0;
	mSampbuf[3]=0;
	
	mOnCnt = -1;
	mOffCnt = -1;
	
	brrdata = silence_brr;
	mLoopPoint = 0;
	mLoop = false;
	
	mFRFlag = false;
	
	mPitch = 0;
	
	mVibPhase = 0.0f;
	mMemPtr = 0;
	mHeaderCnt = 0;
	mHalf = 0;
	mEnvx = 0;
	mEnd = 0;
	mSampptr = 0;
	mMixfrac=0;
	mEnvcnt = CNT_INIT;
	mEnvstate = RELEASE;
}

void Chip700Note::Attack(const MusicDeviceNoteParams &inParams)
{
	Chip700		*synth;
	VoiceParams		vp;
	
	mParam = inParams;

	synth = (Chip700*)GetAudioUnit();
	if (GetGlobalParameter(kParam_drummode)) {
		vp = synth->getMappedVP(inParams.mPitch);
	}
	else {
		vp = synth->getVP(GetGlobalParameter(kParam_program));
	}
	
	if (vp.brr.data) {
		mOnCnt = GetRelativeStartFrame();
	}
	else {
		mEnvx = 0;
		brrdata = silence_brr;
		mEnvstate = RELEASE;
	}
}

void Chip700Note::Release(UInt32 inFrame)
{
	SynthNote::Release(inFrame);
	mOffCnt = inFrame;
	mFRFlag = false;
}

void Chip700Note::FastRelease(UInt32 inFrame)
{
	SynthNote::Release(inFrame);
	mOffCnt = inFrame;
	mFRFlag = true;
}

void Chip700Note::Kill(UInt32 inFrame)
{
	SynthNote::Kill(inFrame);
	mEnvstate = FASTRELEASE;
}

void Chip700Note::KeyOn(void)
{
	Chip700		*synth;
	VoiceParams		vp;
	
	//ベロシティの取得
	if (GetGlobalParameter(kParam_velocity) != 0.) {
		mVelo = mParam.mVelocity;
		mVelo = VELOCITY_CURB[mVelo];
	}
	else {
		mVelo=VELOCITY_CURB[127];
	}
	
	synth = (Chip700*)GetAudioUnit();
	if (GetGlobalParameter(kParam_drummode)) {
		vp = synth->getMappedVP(mParam.mPitch);
	}
	else {
		vp = synth->getVP(GetGlobalParameter(kParam_program));
	}
	
	brrdata = vp.brr.data;
	mLoopPoint = vp.lp;
	mLoop = vp.loop;
	
	//中心周波数の算出
	mPitch = pow(2., (mParam.mPitch - vp.basekey) / 12.)/mInternalClock*vp.rate*4096 + 0.5;
	
	vol_l=vp.volL;
	vol_r=vp.volR;
	ar=vp.ar;
	dr=vp.dr;
	sl=vp.sl;
	sr=vp.sr;
	
	mVibPhase = 0.0f;
	mMemPtr = 0;
	mHeaderCnt = 0;
	mHalf = 0;
	mEnvx = 0;
	mEnd = 0;
	mSampptr = 0;
	mMixfrac = 3 * 4096;
	mEnvcnt = CNT_INIT;
	mEnvstate = ATTACK;
}

float Chip700Note::VibratoWave(float phase)
{
	float x2=phase*phase;
	float vibwave = 7.61e-03f;
	vibwave *= x2;
	vibwave -= 1.6605e-01f;
	vibwave *= x2;
	vibwave += 1.0f;
	vibwave *= phase;
	return vibwave;
}

OSStatus Chip700Note::Render(UInt32 inNumFrames, AudioBufferList& inBufferList)
{
	float			*left, *right;
	int             envx;
	int				outx;
	int				vl,vr;
	bool			reg_pmod;
	float			vibfreq;
	int				vibdepth;
	float			vibdepth2;
	float			pbrange;
	int				clipper;
	int				pitch,pb;
	int				procstep=(32000*21168)/SampleRate();
	UInt32			endFrame = 0xFFFFFFFF;
	
	//バッファの確保
	{
		int numChans = inBufferList.mNumberBuffers;
		if (numChans > 2) return -1;
		left = (float*)inBufferList.mBuffers[0].mData;
		right = numChans==2 ? (float*)inBufferList.mBuffers[1].mData : 0;
	}

	//パラメータの読み込み
	vibfreq = GetGlobalParameter(kParam_vibrate)*((onepi*2)/mInternalClock);
	vibdepth = GetGlobalParameter(kParam_vibdepth);
	vibdepth2 = GetGlobalParameter(kParam_vibdepth2)/2;
	reg_pmod = vibdepth > 0 ? true:false;
	clipper = GetGlobalParameter(kParam_clipnoise)==0 ? 0:1;
	pbrange = GetGlobalParameter(kParam_bendrange)/2.0;
	
	pb = (pow(2., (PitchBend()*pbrange) / 12.) - 1.0)*mPitch;
	
	//メイン処理
	for (UInt32 frame=0; frame<inNumFrames; ++frame)
	{
		if (mOnCnt >= 0) {
			mOnCnt--;
			if (mOnCnt < 0) {
				KeyOn();
				if (endFrame != 0xFFFFFFFF) endFrame = 0xFFFFFFFF;
			}
		}
		
		if (mOffCnt >= 0) {
			mOffCnt--;
			if (mOffCnt < 0) {
				if (mFRFlag)
					mEnvstate = FASTRELEASE;
				else
					mEnvstate = RELEASE;
			}
		}
		
		
		outx = 0;
		
		for( ; mProcessFrac >= 0; mProcessFrac -= 21168 )
		{
			
		//--
		{
			int cnt = mEnvcnt;
			
			envx = mEnvx;
			/*
			switch (GetState()) {
				case kNoteState_FastReleased:
				case kNoteState_Released:
					mEnvstate = RELEASE;
					break;
				default:
					break;
			}
			 */
			switch( mEnvstate )
			{
				case ATTACK:
					if ( ar == 15 )
					{
						envx += 0x400;
					}
					else
					{
						cnt -= ENVCNT[ ( ar << 1 ) + 1 ];
						if( cnt > 0 )
						{
							break;
						}
						envx += 0x20;       /* 0x020 / 0x800 = 1/64         */
						cnt = CNT_INIT;
					}
					
					if( envx > 0x7FF )
					{
						envx = 0x7FF;
						mEnvstate = DECAY;
					}
					
					mEnvx = envx;
					break;
					
				case DECAY:
					cnt -= ENVCNT[ dr*2 + 0x10 ];
					if( cnt <= 0 )
					{
						cnt = CNT_INIT;
						envx -= ( ( envx - 1 ) >> 8 ) + 1;
						mEnvx = envx;
					}
						
					if( envx <= 0x100 * ( sl + 1 ) )
					{
						mEnvstate = SUSTAIN;
					}
					break;
					
				case SUSTAIN:
					cnt -= ENVCNT[ sr ];
					if( cnt > 0 )
					{
						break;
					}
						cnt = CNT_INIT;
					envx -= ( ( envx - 1 ) >> 8 ) + 1;
					
					mEnvx = envx;
					
					break;
					
				case RELEASE:
					envx -= 0x8;
					if( envx <= 0 )
					{
						envx = -1;
					}
					else {
						mEnvx = envx;
					}
					break;
					
				case FASTRELEASE:
					envx -= 0x40;
					if( envx <= 0 )
					{
						envx = -1;
					}
					else {
						mEnvx = envx;
					}
					break;
			}
			mEnvcnt = cnt;
		}
		
		if( envx < 0 )
		{
			outx = 0;
			if (endFrame == 0xFFFFFFFF) endFrame = frame;
			continue;
		}
		
		//ピッチの算出
		pitch = (mPitch + pb)&0x3fff;
		
		if (reg_pmod == true)
		{
			mVibPhase += vibfreq;
			if (mVibPhase > onepi) mVibPhase -= onepi*2;
			
			float vibwave = VibratoWave(mVibPhase);
			outx = (vibwave*vibdepth2)*VELOCITY_CURB[vibdepth];
			
			pitch = ( pitch * ( outx + 32768 ) ) >> 15;
			if (pitch <= 0) pitch=1;
		}
		
		for( ; mMixfrac >= 0; mMixfrac -= 4096 )
		{
			if( !mHeaderCnt )	//ブロックの始まり
			{
				if( mEnd & 1 )	//データ終了フラグあり
				{
					//if( mEnd & 2 )	//ループフラグあり
					if( mLoop )
					{
						mMemPtr = mLoopPoint;	//読み出し位置をループポイントまで戻す
					}
					else	//ループなし
					{
						if (endFrame == 0xFFFFFFFF) endFrame = frame;	//キー状態をオフにする
						mEnvx = 0;
						while( mMixfrac >= 0 )
						{
							mSampbuf[mSampptr] = 0;
							outx = 0;
							mSampptr  = ( mSampptr + 1 ) & 3;
							mMixfrac -= 4096;
						}
						break;
					}
				}
				
				//開始バイトの情報を取得
				mHeaderCnt = 8;
				vl = ( unsigned char )brrdata[mMemPtr++];
				mRange = vl >> 4;
				mEnd = vl & 3;
				mFilter = ( vl & 12 ) >> 2;
			}
			
			if( mHalf == 0 )
			{
				mHalf = 1;
				outx = ( ( signed char )brrdata[ mMemPtr ] ) >> 4;
			}
			else
			{
				mHalf = 0;
				outx = ( signed char )( brrdata[ mMemPtr++ ] << 4 );
				outx >>= 4;
				mHeaderCnt--;
			}
			//outx:4bitデータ
			
			if( mRange <= 0xC )
			{
				outx = ( outx << mRange ) >> 1;
			}
			else
			{
				outx &= ~0x7FF;
			}
			//outx:4bitデータ*Range
			
			switch( mFilter )
			{
				case 0:
					break;
					
				case 1:
					outx += filter1(mSmp1);
					break;
					
				case 2:
					outx += filter2(mSmp1,mSmp2);
					break;
					
				case 3:
					outx += filter3(mSmp1,mSmp2);
					break;
			}
			
			if( outx < -32768 )
			{
				outx = -32768;
			}
			else if( outx > 32767 )
			{
				outx = 32767;
			}
			if (clipper) {
				mSmp2 = ( signed short )mSmp1;
				mSmp1 = ( signed short )( outx << 1 );
				mSampbuf[mSampptr] = ( signed short )mSmp1;
			}
			else {
				mSmp2 = mSmp1;
				mSmp1 = outx << 1;
				mSampbuf[mSampptr] = mSmp1;
			}
			mSampptr = ( mSampptr + 1 ) & 3;
		}
		
		vl = mMixfrac >> 4;
		vr = ( ( G4[ -vl ] * mSampbuf[ mSampptr ] ) >> 11 ) & ~1;
		vr += ( ( G3[ -vl ]
				  * mSampbuf[ ( mSampptr + 1 ) & 3 ] ) >> 11 ) & ~1;
		vr += ( ( G2[ vl ]
				  * mSampbuf[ ( mSampptr + 2 ) & 3 ] ) >> 11 ) & ~1;
		
		if (clipper) {
			vr = ( signed short )vr;
		}
		vr += ( ( G1[ vl ]
				  * mSampbuf[ ( mSampptr + 3 ) & 3 ] ) >> 11 ) & ~1;
		
		if( vr > 32767 )
		{
			vr = 32767;
		}
		else if( vr < -32768 )
		{
			vr = -32768;
		}
		outx = vr;
		
		mMixfrac += pitch;
		
		outx = ( ( outx * envx ) >> 11 ) & ~1;
		outx = ( outx * mVelo ) >> 11;
		
		mProcessbuf[mProcessbufPtr]=outx;
		mProcessbufPtr=(mProcessbufPtr+1)&0x0f;
		}
		//--
		//16pointSinc補間
		{
			int inputFrac = mProcessFrac+21168;
			int tabidx1 = ( inputFrac/1764 ) << 4;
			int tabidx2 = tabidx1 + 16;
			int a1 = 0, a2 = 0;
			for (int i=0; i<4; i++) {
				a1 += sinctable[tabidx1++] * mProcessbuf[mProcessbufPtr] >> 15;
				a2 += sinctable[tabidx2++] * mProcessbuf[mProcessbufPtr] >> 15;
				mProcessbufPtr=(mProcessbufPtr+1)&0x0f;
				a1 += sinctable[tabidx1++] * mProcessbuf[mProcessbufPtr] >> 15;
				a2 += sinctable[tabidx2++] * mProcessbuf[mProcessbufPtr] >> 15;
				mProcessbufPtr=(mProcessbufPtr+1)&0x0f;
				a1 += sinctable[tabidx1++] * mProcessbuf[mProcessbufPtr] >> 15;
				a2 += sinctable[tabidx2++] * mProcessbuf[mProcessbufPtr] >> 15;
				mProcessbufPtr=(mProcessbufPtr+1)&0x0f;
				a1 += sinctable[tabidx1++] * mProcessbuf[mProcessbufPtr] >> 15;
				a2 += sinctable[tabidx2++] * mProcessbuf[mProcessbufPtr] >> 15;
				mProcessbufPtr=(mProcessbufPtr+1)&0x0f;
			}
			outx = a1 + ( (( a2 - a1 ) * ( inputFrac % 1764 )) / 1764 );
		}
		//--
		//ボリューム値の反映
		vl = ( vol_l * outx ) >> 7;
		vr = ( vol_r * outx ) >> 7;
		
		left[frame] += vl / 32768.0f;
		if (right) right[frame] += vr / 32768.0f;
		
		mProcessFrac += procstep;
	}
	if (endFrame != 0xFFFFFFFF)
	{
		NoteEnded(endFrame);
	}

	return noErr;
}
