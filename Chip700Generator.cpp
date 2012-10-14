/*
 *  Chip700Generator.cpp
 *  Chip700
 *
 *  Created by osoumen on 06/09/06.
 *  Copyright 2006 Vermicelli Magic. All rights reserved.
 *
 */

#include "Chip700defines.h"
#include "Chip700Generator.h"
#include <math.h>
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

//-----------------------------------------------------------------------------
Chip700Generator::Chip700Generator()
: mSampleRate(44100.0),
  mClipper( false ),
  mVelocityMode( kVelocityMode_Square ),
  mVPset(NULL)
{
	for ( int i=0; i<NUM_BANKS; i++ ) {
		mDrumMode[i] = false;
	}
	for ( int bnk=0; bnk<NUM_BANKS; bnk++ ) {
		for ( int i=0; i<128; i++ ) {
			mKeyMap[bnk][i] = 0;
		}
	}
	Reset();
}

//-----------------------------------------------------------------------------
void Chip700Generator::VoiceState::Reset()
{
	midi_ch = 0;
	uniqueID = 0;
	
	smp1=0;
	smp2=0;
	sampbuf[0]=0;
	sampbuf[1]=0;
	sampbuf[2]=0;
	sampbuf[3]=0;
	
	pb = 0;
	reg_pmod = 0;
	vibdepth = 0;
	vibPhase = 0.0f;
	
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

//-----------------------------------------------------------------------------
void Chip700Generator::Reset()
{
	for (int i=0; i<16; i++) {
		mProcessbuf[0][i]=0;
		mProcessbuf[1][i]=0;
	}
	mProcessFrac=0;
	mProcessbufPtr=0;
	
	mNoteEvt.clear();
	
	for ( int i=0; i<kMaximumVoices; i++ ) {
		mVoice[i].Reset();
	}
	
	mVoiceLimit = 8;
	mMainVolume_L = 127;
	mMainVolume_R = 127;
	mVibfreq = 0.00137445;
	mVibdepth = 0.5;
	mPbrange = 1.0;
	
	for (int i=0; i<16; i++) {
		mChProgram[i] = 0;
		mChPitchBend[i] = 0;
		mChVibDepth[i] = 0;
	}
	
	mEcho[0].Reset();
	mEcho[1].Reset();
}

//-----------------------------------------------------------------------------
void Chip700Generator::KeyOn( unsigned char ch, unsigned char note, unsigned char velo, unsigned int uniqueID, int inFrame )
{
	NoteEvt			mNoteOnEvt;
	mNoteOnEvt.type = NOTE_ON;
	mNoteOnEvt.note = note;
	mNoteOnEvt.velo = velo;
	mNoteOnEvt.ch = ch;
	mNoteOnEvt.uniqueID = uniqueID;
	mNoteOnEvt.remain_samples = inFrame;
	mNoteEvt.push_back( mNoteOnEvt );
}

//-----------------------------------------------------------------------------
void Chip700Generator::KeyOff( unsigned char ch, unsigned char note, unsigned char velo, unsigned int uniqueID, int inFrame )
{
	NoteEvt			mNoteOffEvt;
	mNoteOffEvt.type = NOTE_OFF;
	mNoteOffEvt.note = note;
	mNoteOffEvt.velo = velo;
	mNoteOffEvt.ch = ch;
	mNoteOffEvt.uniqueID = uniqueID;
	mNoteOffEvt.remain_samples = inFrame;
	mNoteEvt.push_back( mNoteOffEvt );
}

//-----------------------------------------------------------------------------
void Chip700Generator::AllNotesOff()
{
	mNoteEvt.clear();
	for ( int i=0; i<kMaximumVoices; i++ ) {
		mVoice[i].Reset();
	}
}

//-----------------------------------------------------------------------------
void Chip700Generator::AllSoundOff()
{
	mNoteEvt.clear();
	for ( int i=0; i<kMaximumVoices; i++ ) {
		mVoice[i].Reset();
	}
	mEcho[0].Reset();
	mEcho[1].Reset();	
}

//-----------------------------------------------------------------------------
void Chip700Generator::ResetAllControllers()
{
	for (int i=0; i<16; i++) {
		mChPitchBend[i] = 0;
		mChVibDepth[i] = 0;
	}	
}

//-----------------------------------------------------------------------------
void Chip700Generator::ProgramChange( int ch, int pgnum, int inFrame )
{
	mChProgram[ch] = pgnum;
}

//-----------------------------------------------------------------------------
int Chip700Generator::CalcPBValue( float pitchBend, int basePitch )
{
	return (int)((pow(2., (pitchBend*mPbrange) / 12.) - 1.0)*basePitch);
}

//-----------------------------------------------------------------------------
void Chip700Generator::PitchBend( int ch, int value, int inFrame )
{
	float pb_value = value / 8192.0;
	
	mChPitchBend[ch] = pb_value;
	for ( int i=0; i<kMaximumVoices; i++ ) {
		if ( mVoice[i].midi_ch == ch ) {
			mVoice[i].pb = CalcPBValue( pb_value, mVoice[i].pitch );
		}
	}
}

//-----------------------------------------------------------------------------
void Chip700Generator::ModWheel( int ch, int value, int inFrame )
{
	mChVibDepth[ch] = value;
	for ( int i=0; i<kMaximumVoices; i++ ) {
		if ( mVoice[i].midi_ch == ch ) {
			mVoice[i].vibdepth = value;
			mVoice[i].reg_pmod = value > 0 ? true:false;
		}
	}
}

//-----------------------------------------------------------------------------
void Chip700Generator::Damper( int ch, int value, int inFrame )
{
}

//-----------------------------------------------------------------------------
void Chip700Generator::SetVoiceLimit( int value )
{
	mVoiceLimit = value;
}

//-----------------------------------------------------------------------------
void Chip700Generator::SetPBRange( float value )
{
	mPbrange = value / 2.0;
}

//-----------------------------------------------------------------------------
void Chip700Generator::SetClipper( bool value )
{
	mClipper = value;
}

//-----------------------------------------------------------------------------
void Chip700Generator::SetMultiMode( int bank, bool value )
{
	mDrumMode[bank] = value;
}

//-----------------------------------------------------------------------------
bool Chip700Generator::GetMultiMode( int bank ) const
{
	return mDrumMode[bank];
}

//-----------------------------------------------------------------------------
void Chip700Generator::SetVelocityMode( velocity_mode value )
{
	mVelocityMode = value;
}

//-----------------------------------------------------------------------------
void Chip700Generator::SetVibFreq( float value )
{
	mVibfreq = value*((onepi*2)/INTERNAL_CLOCK);
}

//-----------------------------------------------------------------------------
void Chip700Generator::SetVibDepth( float value )
{
	mVibdepth = value / 2;
}

//-----------------------------------------------------------------------------
void Chip700Generator::SetMainVol_L( int value )
{
	mMainVolume_L = value;
}

//-----------------------------------------------------------------------------
void Chip700Generator::SetMainVol_R( int value )
{
	mMainVolume_R = value;
}

//-----------------------------------------------------------------------------
void Chip700Generator::SetEchoVol_L( int value )
{
	mEcho[0].SetEchoVol( value );
}

//-----------------------------------------------------------------------------
void Chip700Generator::SetEchoVol_R( int value )
{
	mEcho[1].SetEchoVol( value );
}

//-----------------------------------------------------------------------------
void Chip700Generator::SetFeedBackLevel( int value )
{
	mEcho[0].SetFBLevel( value );
	mEcho[1].SetFBLevel( value );
}

//-----------------------------------------------------------------------------
void Chip700Generator::SetDelayTime( int value )
{
	mEcho[0].SetDelayTime( value );
	mEcho[1].SetDelayTime( value );
}

//-----------------------------------------------------------------------------
void Chip700Generator::SetFIRTap( int tap, int value )
{
	mEcho[0].SetFIRTap(tap, value);
	mEcho[1].SetFIRTap(tap, value);
}


//-----------------------------------------------------------------------------
int Chip700Generator::FindFreeVoice( const NoteEvt *evt )
{
	int	v=-1;

	//空きボイスを探す
	for (int i=0; i<kMaximumVoices && i<mVoiceLimit; i++ ) {
		if ( mVoice[i].envstate == RELEASE ) {
			v = i;
		}
	}
	
	//空きボイスがあった場合
	if ( v != -1 ) {
		return v;
	}
	
	//空きボイスが無かった場合
	v = 0;
	for ( int i=0; i<kMaximumVoices && i<mVoiceLimit; i++ ) {
		if ( mVoice[i].envx < mVoice[v].envx ) {
			v = i;
		}
	}
	return v;
}

//-----------------------------------------------------------------------------
int Chip700Generator::StopPlayingVoice( const NoteEvt *evt )
{
	int	stops=0;

	for ( int i=0; i<kMaximumVoices; i++ ) {
		if ( mVoice[i].uniqueID == evt->uniqueID ) {
			mVoice[i].envstate = RELEASE;
			stops++;
		}
	}
	return stops;
}

//-----------------------------------------------------------------------------
void Chip700Generator::DoKeyOn(NoteEvt *evt)
{
	VoiceParams		*vp;
	
	//波形アドレスの取得
	vp = getVP(mChProgram[evt->ch]);
	if ( mDrumMode[vp->bank]) {
		vp = getMappedVP(vp->bank, evt->note);
	}
	
	//波形データが存在しない場合は、ここで中断
	if (vp->brr.data == NULL) {
		return;
	}
	
	//空きボイスを取得
	int	v = FindFreeVoice( evt );
	
	//MIDIチャンネルを設定
	mVoice[v].midi_ch = evt->ch;
	mVoice[v].uniqueID = evt->uniqueID;
	
	//ベロシティの取得
	if ( mVelocityMode == kVelocityMode_Square ) {
		mVoice[v].velo = VELOCITY_CURB[evt->velo];
	}
	else if ( mVelocityMode == kVelocityMode_Linear ) {
		mVoice[v].velo = evt->velo << 4;
	}
	else {
		mVoice[v].velo=VELOCITY_CURB[127];
	}
	
	mVoice[v].brrdata = vp->brr.data;
	mVoice[v].loopPoint = vp->lp;
	mVoice[v].loop = vp->loop;
	mVoice[v].echoOn = vp->echo;
	
	//中心周波数の算出
	mVoice[v].pitch = pow(2., (evt->note - vp->basekey) / 12.)/INTERNAL_CLOCK*vp->rate*4096 + 0.5;
	
	mVoice[v].pb = CalcPBValue( mChPitchBend[evt->ch], mVoice[v].pitch );
	mVoice[v].vibdepth = mChVibDepth[evt->ch];
	mVoice[v].reg_pmod = mVoice[v].vibdepth>0 ? true:false;
	mVoice[v].vibPhase = 0.0f;
	
	mVoice[v].vol_l=vp->volL;
	mVoice[v].vol_r=vp->volR;
	mVoice[v].ar=vp->ar;
	mVoice[v].dr=vp->dr;
	mVoice[v].sl=vp->sl;
	mVoice[v].sr=vp->sr;
	
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

//-----------------------------------------------------------------------------
float Chip700Generator::VibratoWave(float phase)
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

//-----------------------------------------------------------------------------
void Chip700Generator::Process( unsigned int frames, float *output[2] )
{
	int		outx;
	int		vl, vr;
	int		pitch;
	int		procstep = (INTERNAL_CLOCK*21168) / mSampleRate;
	
	//メイン処理
	for (unsigned int frame=0; frame<frames; ++frame) {
		//イベント処理
		if ( !mNoteEvt.empty() ) {
			std::list<NoteEvt>::iterator	it = mNoteEvt.begin();
			while ( it != mNoteEvt.end() ) {
				if ( it->remain_samples >= 0 ) {
					it->remain_samples--;
					if ( it->remain_samples < 0 ) {
						if ( it->type == NOTE_ON ) {
							DoKeyOn( &(*it) );
						}
						else if ( it->type == NOTE_OFF ) {
							StopPlayingVoice( &(*it) );
						}
						it = mNoteEvt.erase( it );
						continue;
					}
				}
				it++;
			}
		}
		
		for ( ; mProcessFrac >= 0; mProcessFrac -= 21168 ) {
			int outl=0,outr=0;
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
					continue;
				}
				
				//ピッチの算出
				pitch = (mVoice[v].pitch + mVoice[v].pb) & 0x3fff;
				
				if (mVoice[v].reg_pmod) {
					mVoice[v].vibPhase += mVibfreq;
					if (mVoice[v].vibPhase > onepi) {
						mVoice[v].vibPhase -= onepi*2;
					}
					
					float vibwave = VibratoWave(mVoice[v].vibPhase);
					outx = (vibwave*mVibdepth)*VELOCITY_CURB[mVoice[v].vibdepth];
					
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
					if (mClipper) {
						mVoice[v].smp2 = mVoice[v].smp1;
						mVoice[v].smp1 = ( signed short )( outx << 1 );
						mVoice[v].sampbuf[mVoice[v].sampptr] = mVoice[v].smp1;
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
				
				if (mClipper) {
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
				outl += ( vl * mMainVolume_L ) >> 7;
				outr += ( vr * mMainVolume_R ) >> 7;
			}
			outl += mEcho[0].GetFxOut();
			outr += mEcho[1].GetFxOut();
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
}
					  
//-----------------------------------------------------------------------------
void Chip700Generator::RefreshKeyMap(void)
{
	if ( mVPset ) {
		bool	initialized[NUM_BANKS];
		for (int i=0; i<NUM_BANKS; i++ ) {
			initialized[i] = false;
		}
		
		for (int prg=0; prg<128; prg++) {
			if (mVPset[prg].brr.data) {
				if ( !initialized[mVPset[prg].bank] ) {
					// 一番最初のプログラムで初期化することで、未使用パッチが0にならないようにする
					for (int i=0; i<128; i++) {
						mKeyMap[mVPset[prg].bank][i]=prg;
					}
					initialized[mVPset[prg].bank] = true;
				}
				for (int i=mVPset[prg].lowkey; i<=mVPset[prg].highkey; i++) {
					mKeyMap[mVPset[prg].bank][i]=prg;
				}
			}
		}
	}
}
