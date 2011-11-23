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

void Chip700Note::VoiceState::Reset()
{
	smp1=0;
	smp2=0;
	sampbuf[0]=0;
	sampbuf[1]=0;
	sampbuf[2]=0;
	sampbuf[3]=0;
	
	brrdata = silence_brr;
	loopPoint = 0;
	loop = false;
	
//	FRFlag = false;
	
	pitch = 0;
	
	vibPhase = 0.0f;
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

void Chip700Note::Reset()
{
	SynthNote::Reset();

	mInternalClock=32000;
	for (int i=0; i<16; i++) {
		mProcessbuf[0][i]=0;
		mProcessbuf[1][i]=0;
	}
	mProcessFrac=0;
	mProcessbufPtr=0;
	
	mNoteEvt.clear();
	
	for ( int i=0; i<MAX_VOICES; i++ ) {
		mVoice[i].Reset();
	}

	/*
	mSmp1=0;
	mSmp2=0;
	mSampbuf[0]=0;
	mSampbuf[1]=0;
	mSampbuf[2]=0;
	mSampbuf[3]=0;
	
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
	*/
	
	mEcho[0].Reset();
	mEcho[1].Reset();
}

void Chip700Note::Attack(const MusicDeviceNoteParams &inParams)
{
	//MIDIチャンネルの取得
	SynthGroupElement	*group = GetGroup();
	unsigned int		chID = group->GroupID();
	
	ScheduleKeyOn( chID, inParams.mPitch, inParams.mVelocity, GetRelativeStartFrame() );
	
	_note = inParams.mPitch;	//仮
}

void Chip700Note::ScheduleKeyOn( unsigned char ch, unsigned char note, unsigned char velo, int inFrame )
{
	NoteEvt			mNoteOnEvt;
	mNoteOnEvt.type = NOTE_ON;
	mNoteOnEvt.note = note;
	mNoteOnEvt.velo = velo;
	mNoteOnEvt.ch = ch;
	mNoteOnEvt.remain_samples = inFrame;
	mNoteEvt.push_back( mNoteOnEvt );
}

void Chip700Note::ScheduleKeyOff( unsigned char ch, unsigned char note, unsigned char velo, int inFrame )
{
	NoteEvt			mNoteOffEvt;
	mNoteOffEvt.type = NOTE_OFF;
	mNoteOffEvt.note = note;
	mNoteOffEvt.velo = velo;
	mNoteOffEvt.ch = ch;
	mNoteOffEvt.remain_samples = inFrame;
	mNoteEvt.push_back( mNoteOffEvt );
}

int Chip700Note::FindVoice( const NoteEvt *evt )
{
	return 0;		//仮
}

void Chip700Note::Release(UInt32 inFrame)
{
	SynthNote::Release(inFrame);

	//MIDIチャンネルの取得
	SynthGroupElement	*group = GetGroup();
	unsigned int		chID = group->GroupID();
	
	ScheduleKeyOff( chID, _note, 0, inFrame );
}

void Chip700Note::FastRelease(UInt32 inFrame)
{
	SynthNote::Release(inFrame);
	
	//MIDIチャンネルの取得
	SynthGroupElement	*group = GetGroup();
	unsigned int		chID = group->GroupID();
	
	ScheduleKeyOff( chID, _note, 0, inFrame );
}

void Chip700Note::Kill(UInt32 inFrame)
{
	SynthNote::Kill(inFrame);
	mVoice[0].envstate = FASTRELEASE;
}

void Chip700Note::KeyOn(NoteEvt *evt)
{
	Chip700		*synth;
	VoiceParams		vp;
	
	//波形アドレスの取得
	synth = (Chip700*)GetAudioUnit();
	if (GetGlobalParameter(kParam_drummode)) {
		vp = synth->getMappedVP(evt->note);
	}
	else {
		if ( evt->ch == 0 ) {
			vp = synth->getVP(GetGlobalParameter(kParam_program));
		}
		else {
			vp = synth->getVP(GetGlobalParameter(kParam_program_2 + evt->ch - 1));
		}
	}
	
	//波形データが存在しない場合は、ここで中断
	if (vp.brr.data == NULL) {
		return;
	}
	
	//空きボイスを取得
	int	v = FindVoice( evt );
	
	//ベロシティの取得
	if (GetGlobalParameter(kParam_velocity) != 0.) {
		//mVoice[v].velo = evt->velo;
		mVoice[v].velo = VELOCITY_CURB[evt->velo];
	}
	else {
		mVoice[v].velo=VELOCITY_CURB[127];
	}
	
	mVoice[v].brrdata = vp.brr.data;
	mVoice[v].loopPoint = vp.lp;
	mVoice[v].loop = vp.loop;
	mVoice[v].echoOn = vp.echo;
	
	//中心周波数の算出
	mVoice[v].pitch = pow(2., (evt->note - vp.basekey) / 12.)/mInternalClock*vp.rate*4096 + 0.5;
	
	mVoice[v].vol_l=vp.volL;
	mVoice[v].vol_r=vp.volR;
	mVoice[v].ar=vp.ar;
	mVoice[v].dr=vp.dr;
	mVoice[v].sl=vp.sl;
	mVoice[v].sr=vp.sr;
	
	mVoice[v].vibPhase = 0.0f;
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

Float32	Chip700Note::Amplitude()
{
	Float32	max = .0;
	for ( int i=0; i<MAX_VOICES; i++ ) {
		if ( mVoice[i].envx > max ) {
			max = mVoice[i].envx;
		}
	}
	return max;
}

OSStatus Chip700Note::Render(UInt32 inNumFrames, AudioBufferList& inBufferList)
{
	float			*output[2];
	int				main_vol_l, main_vol_r;
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
	unsigned int	endFrame = 0xFFFFFFFF;
	
	//バッファの確保
	{
		int numChans = inBufferList.mNumberBuffers;
		if (numChans > 2) return -1;
		output[0] = (float*)inBufferList.mBuffers[0].mData;
		output[1] = numChans==2 ? (float*)inBufferList.mBuffers[1].mData : NULL;
	}

	//MIDIチャンネルの取得
	SynthGroupElement	*group = GetGroup();
	unsigned int		chID = group->GroupID();
	
	//パラメータの読み込み
	vibfreq = GetGlobalParameter(kParam_vibrate)*((onepi*2)/mInternalClock);
	if ( chID == 0 ) {
		vibdepth = GetGlobalParameter(kParam_vibdepth);
	}
	else {
		vibdepth = GetGlobalParameter(kParam_vibdepth_2 - 1 + chID);
	}
	vibdepth2 = GetGlobalParameter(kParam_vibdepth2)/2;
	reg_pmod = vibdepth > 0 ? true:false;
	clipper = GetGlobalParameter(kParam_clipnoise)==0 ? 0:1;
	pbrange = GetGlobalParameter(kParam_bendrange)/2.0;
	
	pb = (pow(2., (PitchBend()*pbrange) / 12.) - 1.0)*mVoice[0].pitch;	//仮
	
	//エコーパラメータの読み込み
	main_vol_l = GetGlobalParameter(kParam_mainvol_L);
	mEcho[0].SetEchoVol( GetGlobalParameter(kParam_echovol_L) );
	mEcho[0].SetFBLevel( GetGlobalParameter(kParam_echoFB) );
	for ( int i=0; i<8; i++ ) {
		mEcho[0].SetFIRTap( i, GetGlobalParameter( kParam_fir0+i ) );
	}
	mEcho[0].SetDelaySamples( GetGlobalParameter( kParam_echodelay ) );
	
	main_vol_r = GetGlobalParameter(kParam_mainvol_R);
	mEcho[1].SetEchoVol( GetGlobalParameter(kParam_echovol_R) );
	mEcho[1].SetFBLevel( GetGlobalParameter(kParam_echoFB) );
	for ( int i=0; i<8; i++ ) {
		mEcho[1].SetFIRTap( i, GetGlobalParameter( kParam_fir0+i ) );
	}
	mEcho[1].SetDelaySamples( GetGlobalParameter( kParam_echodelay ) );
	
	//メイン処理
	for (unsigned int frame=0; frame<inNumFrames; ++frame) {
		//イベント処理
		if ( !mNoteEvt.empty() ) {
			std::list<NoteEvt>::iterator	it = mNoteEvt.begin();
			while ( it != mNoteEvt.end() ) {
				if ( it->remain_samples >= 0 ) {
					it->remain_samples--;
					if ( it->remain_samples < 0 ) {
						if ( it->type == NOTE_ON ) {
							KeyOn( &(*it) );
							if (endFrame != 0xFFFFFFFF) endFrame = 0xFFFFFFFF;
						}
						else if ( it->type == NOTE_OFF ) {
							int voice = FindVoice( &(*it) );
							mVoice[voice].envstate = RELEASE;
						}
						mNoteEvt.erase( it );
					}
				}
				it++;
			}
		}
		
		for ( ; mProcessFrac >= 0; mProcessFrac -= 21168 ) {
		int outl=0,outr=0;
		for ( int v=0; v<MAX_VOICES; v++ ) {
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
					
				case FASTRELEASE:
					mVoice[v].envx -= 0x40;
					if ( mVoice[v].envx <= 0 ) {
						mVoice[v].envx = -1;
					}
					break;
			}
		}
		
		if ( mVoice[v].envx < 0 ) {
			outx = 0;
			if (endFrame == 0xFFFFFFFF) endFrame = frame;
			continue;
		}
		
		//ピッチの算出
		pitch = (mVoice[v].pitch + pb) & 0x3fff;
		
		if (reg_pmod == true) {
			mVoice[v].vibPhase += vibfreq;
			if (mVoice[v].vibPhase > onepi) {
				mVoice[v].vibPhase -= onepi*2;
			}
			
			float vibwave = VibratoWave(mVoice[v].vibPhase);
			outx = (vibwave*vibdepth2)*VELOCITY_CURB[vibdepth];
			
			pitch = ( pitch * ( outx + 32768 ) ) >> 15;
			if (pitch <= 0) {
				pitch=1;
			}
		}
		
		for( ; mVoice[v].mixfrac >= 0; mVoice[v].mixfrac -= 4096 ) {
			if( !mVoice[v].headerCnt ) {	//ブロックの始まり
				if( mVoice[v].end & 1 ) {	//データ終了フラグあり
					if( mVoice[v].loop ) {
						mVoice[v].memPtr = mVoice[v].loopPoint;	//読み出し位置をループポイントまで戻す
					}
					else {	//ループなし
						if (endFrame == 0xFFFFFFFF) {
							endFrame = frame;	//キー状態をオフにする
						}
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
				vl = ( unsigned char )mVoice[v].brrdata[mVoice[v].memPtr++];
				mVoice[v].range = vl >> 4;
				mVoice[v].end = vl & 3;
				mVoice[v].filter = ( vl & 12 ) >> 2;
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
			
			if ( outx < -32768 ) {
				outx = -32768;
			}
			else if ( outx > 32767 ) {
				outx = 32767;
			}
			if (clipper) {
				mVoice[v].smp2 = ( signed short )mVoice[v].smp1;
				mVoice[v].smp1 = ( signed short )( outx << 1 );
				mVoice[v].sampbuf[mVoice[v].sampptr] = ( signed short )mVoice[v].smp1;
			}
			else {
				mVoice[v].smp2 = mVoice[v].smp1;
				mVoice[v].smp1 = outx << 1;
				mVoice[v].sampbuf[mVoice[v].sampptr] = mVoice[v].smp1;
			}
			mVoice[v].sampptr = ( mVoice[v].sampptr + 1 ) & 3;
		}
		
		vl = mVoice[v].mixfrac >> 4;
		vr = ( ( G4[ -vl ] * mVoice[v].sampbuf[ mVoice[v].sampptr ] ) >> 11 ) & ~1;
		vr += ( ( G3[ -vl ]
				  * mVoice[v].sampbuf[ ( mVoice[v].sampptr + 1 ) & 3 ] ) >> 11 ) & ~1;
		vr += ( ( G2[ vl ]
				  * mVoice[v].sampbuf[ ( mVoice[v].sampptr + 2 ) & 3 ] ) >> 11 ) & ~1;
		
		if (clipper) {
			vr = ( signed short )vr;
		}
		vr += ( ( G1[ vl ]
				  * mVoice[v].sampbuf[ ( mVoice[v].sampptr + 3 ) & 3 ] ) >> 11 ) & ~1;
		
		if ( vr > 32767 ) {
			vr = 32767;
		}
		else if ( vr < -32768 ) {
			vr = -32768;
		}
		outx = vr;
		
		mVoice[v].mixfrac += pitch;
		
		outx = ( ( outx * mVoice[v].envx ) >> 11 ) & ~1;
		outx = ( outx * mVoice[v].velo ) >> 11;
		
		//ボリューム値の反映
		vl = ( mVoice[v].vol_l * outx ) >> 7;
		vr = ( mVoice[v].vol_r * outx ) >> 7;
		
		//エコー処理
		if ( mVoice[v].echoOn ) {
			mEcho[0].Input(vl);
			mEcho[1].Input(vr);
		}
		//メインボリュームの反映
		outl += ( vl * main_vol_l ) >> 7;
		outr += ( vr * main_vol_r ) >> 7;
		}
		outl += mEcho[0].GetOut();
		outr += mEcho[1].GetOut();
		mProcessbuf[0][mProcessbufPtr] = outl;
		mProcessbuf[1][mProcessbufPtr] = outr;
		mProcessbufPtr=(mProcessbufPtr+1)&0x0f;
		}
		//--
		//16pointSinc補間
		for ( int ch=0; ch<2; ch++ ) {
			int inputFrac = mProcessFrac+21168;
			int tabidx1 = ( inputFrac/1764 ) << 4;
			int tabidx2 = tabidx1 + 16;
			int a1 = 0, a2 = 0;
			for (int i=0; i<4; i++) {
				a1 += sinctable[tabidx1++] * mProcessbuf[ch][mProcessbufPtr] >> 15;
				a2 += sinctable[tabidx2++] * mProcessbuf[ch][mProcessbufPtr] >> 15;
				mProcessbufPtr=(mProcessbufPtr+1)&0x0f;
				a1 += sinctable[tabidx1++] * mProcessbuf[ch][mProcessbufPtr] >> 15;
				a2 += sinctable[tabidx2++] * mProcessbuf[ch][mProcessbufPtr] >> 15;
				mProcessbufPtr=(mProcessbufPtr+1)&0x0f;
				a1 += sinctable[tabidx1++] * mProcessbuf[ch][mProcessbufPtr] >> 15;
				a2 += sinctable[tabidx2++] * mProcessbuf[ch][mProcessbufPtr] >> 15;
				mProcessbufPtr=(mProcessbufPtr+1)&0x0f;
				a1 += sinctable[tabidx1++] * mProcessbuf[ch][mProcessbufPtr] >> 15;
				a2 += sinctable[tabidx2++] * mProcessbuf[ch][mProcessbufPtr] >> 15;
				mProcessbufPtr=(mProcessbufPtr+1)&0x0f;
			}
			if ( output[ch] ) {
				output[ch][frame] += ( a1 + ( (( a2 - a1 ) * ( inputFrac % 1764 )) / 1764 ) ) / 32768.0f;
			}
		}
		//--
		
		mProcessFrac += procstep;
	}
	
	if (endFrame != 0xFFFFFFFF) {
		NoteEnded(endFrame);
	}

	return noErr;
}

#pragma mark ____EchoKernel
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void EchoKernel::Input(int samp)
{
	m_input += samp;
}

int EchoKernel::GetOut()
{
	int echo = m_input;
	
	mEchoIndex &= 0x7fff;
	
	//ディレイ信号にFIRフィルタを掛けてから出力に加算する
	mFIRbuf[mFIRIndex] = mEchoBuffer[mEchoIndex];
	int i;
	int sum = 0;
	for (i=0; i<mFIRLength-1; i++) {
		sum += mFIRbuf[mFIRIndex] * m_fir_taps[i];
		mFIRIndex = (mFIRIndex + 1)%mFIRLength;
	}
	sum += mFIRbuf[mFIRIndex] * m_fir_taps[i];
	sum >>= 7;
	//mFIRbufへの書き込みは、右から左へと行われる
	int output = ( sum * m_echo_vol ) >> 7;
	
	//入力にフィードバックを加算したものをバッファキューに入れる
	echo += ( sum * m_fb_lev ) >> 7;
	mEchoBuffer[mEchoIndex++] = echo;
	if (mEchoIndex >= m_delay_samples) {
		mEchoIndex=0;
	}
	
	m_input = 0;
	
	return output;
}

void EchoKernel::Reset()
{
	mEchoBuffer.Clear();
	mFIRbuf.Clear();
	mEchoIndex=0;
	mFIRIndex=0;
	m_input = 0;
}
