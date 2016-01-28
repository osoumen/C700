/*
 *  XIFile.cpp
 *  C700
 *
 *  Created by osoumen on 12/10/10.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "C700defines.h"
#include "XIFile.h"
#include "AudioFile.h"
#include "brrcodec.h"
#include <math.h>
#include <algorithm>

int RenumberKeyMap( unsigned char *snum, int size );
int GetARTicks( int ar, double tempo );
int GetDRTicks( int dr, double tempo );
int GetSRTicks( int sr, double tempo );

//-----------------------------------------------------------------------------
XIFile::XIFile( const char *path, int allocMemSize )
: FileAccess(path, true)
, DataBuffer( allocMemSize )
#if MAC
, mCFData( NULL )
#endif
{
}

//-----------------------------------------------------------------------------
XIFile::~XIFile()
{
#if MAC
	if ( mCFData ) {
		CFRelease(mCFData);
	}
#endif
}

//-----------------------------------------------------------------------------
bool XIFile::SetDataFromChip( const C700Driver *chip, int targetProgram, double tempo )
{
	mDataPos = 0;
	mDataUsed = 0;
	
	int start_prg;
	int end_prg;
    const InstParams  *vp = chip->getVP(targetProgram);
	int	selectBank = vp->bank;
	bool multisample = chip->GetMultiMode(selectBank);
	
	
	if ( multisample ) {
		start_prg = 0;
		end_prg = 127;
	}
	else {
		start_prg = end_prg = targetProgram;
	}
	
	XIFILEHEADER		xfh;
	XIINSTRUMENTHEADER	xih;
	XISAMPLEHEADER		xsh;
	int nsamples = 0;
	
	// XI File Header
	memset(&xfh, 0, sizeof(xfh));
	memcpy(xfh.extxi, "Extended Instrument: ", 21);
	if ( multisample ) {
		char multi_instname[][22] = {
			"Bank A (Multi)",
			"Bank B (Multi)",
			"Bank C (Multi)",
			"Bank D (Multi)"
		};
		memcpy(xfh.name, multi_instname[selectBank], 22);
	}
	else {
		bool	noname = false;
		if ( vp->pgname[0] == 0 ) {
			noname = true;
		}
		strncpy(xfh.name, vp->pgname, 22);
		if ( noname ) {
			memcpy(xfh.name, "Inst", 22);
		}
	}
	xfh.name[22] = 0x1A;
	memcpy(xfh.trkname, "FastTracker v2.00   ", 20);
	xfh.shsize = EndianU16_NtoL( 0x0102 );
	if ( writeData( &xfh, sizeof(xfh),NULL ) == false ) {
		return false;
	}
	
	// XI Instrument Header
	memset(&xih, 0, sizeof(xih));
	
	int vol = 64;
	
	if ( multisample ) {
		for ( int i=0; i<96; i++ ) {
			xih.snum[i] = chip->GetKeyMap(selectBank, i+12);
		}
		nsamples = RenumberKeyMap( xih.snum, 96 );
	}
	else {
		for ( int i=0; i<96; i++ ) {
			xih.snum[i] = 0;
		}
		nsamples = 1;
		vol = (int)( abs(vp->volL) + abs(vp->volR) ) / 4;
	}
	
	//エンベロープの近似
	if ( multisample ) {
		//サンプル毎には設定出来ないようなので非対応
		for (int i=0; i<4; i++) {
			xih.venv[i*2] = EndianU16_NtoL( i*10 );
			xih.venv[i*2+1] = EndianU16_NtoL( 64 );
		}
		xih.venv[7] = 0;
	}
	else {
		xih.venv[0] = 0;
		if ( vp->ar == 15 ) {
			xih.venv[1] = EndianU16_NtoL( vol );
		}
		else {
			xih.venv[1] = 0;
		}
		xih.venv[2] = EndianU16_NtoL( GetARTicks( vp->ar, tempo ) );	//tick数値はテンポ値に依存する
		xih.venv[3] = EndianU16_NtoL( vol );
		xih.venv[4] = EndianU16_NtoL( xih.venv[2] + GetDRTicks( vp->dr, tempo ) );
		xih.venv[5] = EndianU16_NtoL( vol * (vp->sl + 1) / 8 );
		if ((vp->sr == 0) || vp->sustainMode) {
			xih.venv[6] = EndianU16_NtoL( xih.venv[4]+1 );
			xih.venv[7] = EndianU16_NtoL( xih.venv[5] );
		}
		else {
			xih.venv[6] = EndianU16_NtoL( xih.venv[4] + GetSRTicks( vp->sr, tempo ) );
			xih.venv[7] = EndianU16_NtoL( vol / 10 );
		}
	}
	
	xih.vnum = 4;
	xih.pnum = 0;
	xih.vsustain = 3;
	xih.vloops = 3;
	xih.vloope = 3;
	xih.psustain = 0;
	xih.ploops = 0;
	xih.ploope = 0;
	xih.vtype = 3;	//ENV_VOLUME + ENV_VOLSUSTAIN
	xih.ptype = 0;
	xih.volfade = EndianU16_NtoL( 32767 );
	xih.reserved2 = EndianU16_NtoL( nsamples );
	if ( writeData( &xih, sizeof(xih),NULL ) == false ) {
		return false;
	}
	
	// XI Sample Headers
	for (int ismp=start_prg; ismp<=end_prg; ismp++) {
		if ( chip->getVP(ismp)->hasBrrData() && chip->getVP(ismp)->bank == selectBank ) {
			
			//元ファイルのヘッダー情報を読み込む
			bool	existSrcFile = false;
			if ( chip->getVP(ismp)->sourceFile[0] ) {
				AudioFile::InstData	inst;
				int		numSamples;
				
				AudioFile	origFile(chip->getVP(ismp)->sourceFile,false);
				origFile.Load();
				
				if ( origFile.IsLoaded() ) {
					numSamples = origFile.GetLoadedSamples();
					origFile.GetInstData(&inst);
					xsh.samplen = EndianU32_NtoL( numSamples * 2 );
					xsh.loopstart = EndianU32_NtoL( inst.lp * 2 );
					xsh.looplen = EndianU32_NtoL( (inst.lp_end - inst.lp) * 2 );
					existSrcFile = true;
				}
			}
			if ( existSrcFile == false ) {
				xsh.samplen = EndianU32_NtoL( chip->getVP(ismp)->brrSize()/9*32 );
				xsh.loopstart = EndianU32_NtoL( chip->getVP(ismp)->lp/9*32 );
				xsh.looplen = EndianU32_NtoL( xsh.samplen - xsh.loopstart );
			}
			
			double avr = ( abs(chip->getVP(ismp)->volL) + abs(chip->getVP(ismp)->volR) ) / 2;
			double pan = (abs(chip->getVP(ismp)->volR) * 128) / avr;
			if ( pan > 255 ) pan = 255;
			xsh.vol = 64;	//変化しない？
			xsh.pan = pan;
			xsh.type = 0x10;	//CHN_16BIT
			if ( chip->getVP(ismp)->loop ) {
				xsh.type |= 0x01;	//Normal Loop
			}
			double trans = 12*(log(chip->getVP(ismp)->rate / 8363.0)/log(2.0));
			int trans_i = (trans + (60-chip->getVP(ismp)->basekey) ) * 128 + 0.5;
			xsh.relnote = trans_i >> 7;
			xsh.finetune = trans_i & 0x7f;
			xsh.res = 0;
			if ( chip->getVP(ismp)->pgname[0] == 0 ) {
				strncpy(xsh.name, "Sample", 21);
			}
			else {
				strncpy(xsh.name, chip->getVP(ismp)->pgname, 21);
			}
			if ( writeData( &xsh, sizeof(xsh),NULL ) == false ) {
				return false;
			}
		}
	}
	
	// XI Sample Data
	for (int ismp=start_prg; ismp<=end_prg; ismp++) {
		if ( chip->getVP(ismp)->hasBrrData() && chip->getVP(ismp)->bank == selectBank ) {
			short	*wavedata;
			int		numSamples;
			bool	existSrcFile = false;	//元ファイルが存在するか？
			
			if ( chip->getVP(ismp)->sourceFile[0] ) {
				//元ファイルから波形を読み込む
				AudioFile	origFile(chip->getVP(ismp)->sourceFile,false);
				origFile.Load();
				
				if ( origFile.IsLoaded() ) {
					numSamples = origFile.GetLoadedSamples();
					wavedata = (short *)malloc(numSamples * sizeof(short));
					memcpy(wavedata, origFile.GetAudioData(), numSamples * sizeof(short));
					
					if ( chip->getVP(ismp)->isEmphasized ) {
						emphasis(wavedata, numSamples);
					}
					wavedata[0] = 0;	//先頭に挿入するのは本当は良いのだが
					
					existSrcFile = true;
				}
			}
			
			//ソースファイルが無い場合はbrrデータをデコードして使用する
			if ( existSrcFile == false ) {
				numSamples = chip->getVP(ismp)->brrSamples();
				//wavedata = new short[numSamples];
				wavedata = (short *)malloc(numSamples * sizeof(short));
				brrdecode(chip->getVP(ismp)->brrData(), wavedata,0,0);
			}
			
			short s_new,s_old;
			s_old = wavedata[0];
			for ( int i=0; i<numSamples; i++ ) {
				s_new = wavedata[i];
				// 差分値に変換が必要？
				wavedata[i] = s_new - s_old;
				wavedata[i] = EndianS16_NtoL( wavedata[i] );
				s_old = s_new;
			}
			if ( writeData( wavedata, numSamples*2,NULL ) == false ) {
				free(wavedata);
				return false;
			}
			free(wavedata);
		}
	}
	
	mIsLoaded = true;
	
	return true;
}

//-----------------------------------------------------------------------------
bool XIFile::Write()
{
	if ( IsLoaded() == false ) return false;
	if ( mPath == NULL ) return false;
#if MAC
	//CFURLを作成
	CFURLRef	savefile = CFURLCreateFromFileSystemRepresentation(NULL, (UInt8*)mPath, strlen(mPath), false);
	
	//バイナリ形式に変換
	CFWriteStreamRef	filestream = CFWriteStreamCreateWithFile(NULL,savefile);
	if (CFWriteStreamOpen(filestream)) {
		if ( mCFData == NULL ) {
			mCFData = CFDataCreate(NULL, GetDataPtr(), GetDataSize() );
		}
		CFWriteStreamWrite(filestream,CFDataGetBytePtr(mCFData),CFDataGetLength(mCFData));
		CFWriteStreamClose(filestream);
	}
	CFRelease(filestream);
	CFRelease(savefile);
	
	return true;
#else
	//VSTのときのXIファイル書き出し処理
	HANDLE	hFile;

	hFile = CreateFile( mPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile != INVALID_HANDLE_VALUE ) {
		DWORD	writeSize;
		WriteFile( hFile, GetDataPtr(), GetDataSize(), &writeSize, NULL );
		CloseHandle( hFile );
	}
	return true;
#endif
}

//-----------------------------------------------------------------------------
#if MAC
void XIFile::SetCFData( CFDataRef data )
{
	if ( mIsLoaded ) {
		CFRelease(mCFData);
	}
	mCFData = data;
	CFRetain(mCFData);
	mIsLoaded = true;
}

//-----------------------------------------------------------------------------
CFDataRef XIFile::GetCFData() const
{
	if ( IsLoaded() == false ) return NULL;
	
	return mCFData;
}
#endif


//-----------------------------------------------------------------------------
int	RenumberKeyMap( unsigned char *snum, int size )
{
	static const int MAX_PROGS = 128;
	int progs[MAX_PROGS];
	int num_progs = 0;
    
    for (int i=0; i<MAX_PROGS; i++) {
        progs[i] = 0;
    }
	// 総プログラム数を調べる
	for ( int i=0; i<size; i++ ) {
		bool exist = false;
		// 探す
		for ( int j=0; j<num_progs; j++ ) {
			if ( snum[i] == progs[j] ) {
				// 見つけた
				exist = true;
				break;
			}
		}
		// 新しいプログラム番号を見つけたら配列に追加
		if ( !exist ) {
			progs[num_progs] = snum[i];
			num_progs++;
		}
	}
	// progsを昇順にソート
	std::sort(progs, progs+num_progs);
	
	// 変換テーブルを作成
	int trans_table[MAX_PROGS];
	for ( int i=0; i<MAX_PROGS; i++ ) {
		trans_table[i] = i;
	}
	for ( int i=0; i<num_progs; i++ ) {
		trans_table[progs[i]] = i;
	}
	
	// 変換
	for ( int i=0; i<size; i++ ) {
		snum[i] = trans_table[ snum[i] ];
	}
	return num_progs;
}

//-----------------------------------------------------------------------------
int GetARTicks( int ar, double tempo )
{
	double	basetime[16] = {
		4.1,
		2.6,
		1.5,
		1.0,
		0.640,
		0.380,
		0.260,
		0.160,
		0.096,
		0.064,
		0.040,
		0.024,
		0.016,
		0.010,
		0.006,
		0
	};
	//四分音符＝25tick
	double tick_per_sec = tempo/60.0 * 25;
	return (int)(basetime[ar] * tick_per_sec);
}

//-----------------------------------------------------------------------------
int GetDRTicks( int dr, double tempo )
{
	double	basetime[8] = {
		1.2,
		0.740,
		0.440,
		0.290,
		0.180,
		0.110,
		0.074,
		0.037
	};
	//四分音符＝25tick
	double tick_per_sec = tempo/60.0 * 25;
	//tick_per_sec /= 2;	//実際は半分くらいなような気がする？？
	return (int)(basetime[dr] * tick_per_sec);
}

//-----------------------------------------------------------------------------
int GetSRTicks( int sr, double tempo )
{
	double	basetime[32] = {
		0,
		38,
		28,
		24,
		19,
		14,
		12,
		9.4,
		7.1,
		5.9,
		4.7,
		3.5,
		2.9,
		2.4,
		1.8,
		1.5,
		1.2,
		0.880,
		0.740,
		0.590,
		0.440,
		0.370,
		0.290,
		0.220,
		0.180,
		0.150,
		0.110,
		0.092,
		0.074,
		0.055,
		0.037,
		0.028
	};
	//四分音符＝25tick
	double tick_per_sec = tempo/60.0 * 25;
	return (int)(basetime[sr] * tick_per_sec);
}
