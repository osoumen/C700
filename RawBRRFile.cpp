/*
 *  RawBRRFile.cpp
 *  C700
 *
 *  Created by osoumen on 12/10/10.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "RawBRRFile.h"
#include <stdio.h>

void getFileNameDeletingPathExt( const char *path, char *out, int maxLen );

//-----------------------------------------------------------------------------
void getInstFileName( const char *path, char *out, int maxLen )
{
	//拡張子を.smplに変えたファイルパスを得る
#if MAC
	CFURLRef	url = CFURLCreateFromFileSystemRepresentation(NULL, (UInt8*)path, strlen(path), false);
	CFURLRef	extlesspath=CFURLCreateCopyDeletingPathExtension(NULL, url);
	CFStringRef	filename = CFURLCopyFileSystemPath(extlesspath,kCFURLPOSIXPathStyle);
	CFStringGetCString(filename, out, maxLen-1, kCFStringEncodingUTF8);
	strcat(out, ".smpl");
	CFRelease(filename);
	CFRelease(extlesspath);
	CFRelease(url);
#else
	int	len = static_cast<int>(strlen(path));
	int extPos = len;
	//"."の位置を検索
	for ( int i=len-1; i>=0; i-- ) {
		if ( path[i] == '.' ) {
			extPos = i;
			break;
		}
	}
	strncpy(out, path, extPos);
	out[extPos] = 0;
	strcat(out, ".smpl");
#endif
}

//-----------------------------------------------------------------------------
RawBRRFile::RawBRRFile( const char *path, bool isWriteable )
: FileAccess(path, isWriteable)
{
}

//-----------------------------------------------------------------------------
RawBRRFile::~RawBRRFile()
{
}

//-----------------------------------------------------------------------------
bool RawBRRFile::Load()
{
	bool result;
	//最初に、先頭2バイトにループポイントがあると仮定して読み込みを試みる
	result = tryLoad(false);
	//読めなかった場合は、生のrrデータとして読み込む
	if ( result == false ) {
		result = tryLoad(true);
	}
	return result;
}

//-----------------------------------------------------------------------------
bool RawBRRFile::tryLoad(bool noLoopPoint)
{
	int	dataOffset=0;
	
	if ( strlen(mPath) == 0 ) {
		return false;
	}
	
	//ループポイント有りのデータは、3バイト目以降に読み込む
	if( noLoopPoint ) {
		dataOffset = 2;
		mFileData[0] = 0;
		mFileData[1] = 0;
	}
	
#if MAC
	CFURLRef	url = CFURLCreateFromFileSystemRepresentation(NULL, (UInt8*)mPath, strlen(mPath), false);
	
	CFReadStreamRef	filestream = CFReadStreamCreateWithFile(NULL, url);
	if (CFReadStreamOpen(filestream) == false) {
		CFRelease( url );
		return false;
	}
	
	CFIndex	readbytes=CFReadStreamRead(filestream, mFileData+dataOffset, MAX_FILE_SIZE);
	mFileSize = readbytes+dataOffset;
	CFReadStreamClose(filestream);
	CFRelease( url );
#else
	//VSTのときのファイル読み込み処理
	HANDLE	hFile;
	
	hFile = CreateFile( mPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile != INVALID_HANDLE_VALUE ) {
		DWORD	readSize;
		ReadFile( hFile, mFileData+dataOffset, MAX_FILE_SIZE, &readSize, NULL );
		mFileSize = readSize+dataOffset;
		CloseHandle( hFile );
	}
#endif
	
	mInst.lp = (mFileData[1] << 8) | mFileData[0];
	//先頭2バイト(リトルエンディアン)の数値+2よりファイルサイズが大きい
	if ( mInst.lp+2 >= mFileSize ) {
		return false;
	}
	
	//ループポイントの次のバイトから9バイトずつ進め、エンドフラグを探す
	mInst.brr.data = mFileData+2;
	mInst.brr.size = mFileSize-2;
	int	endflag_pos = 0;
	int	num_endflag = 0;
	for ( int i=0; i<mInst.brr.size; i+=9 ) {
		int end_flag = mInst.brr.data[i] & 0x01;
		if ( end_flag ) {
			endflag_pos = i;
			mInst.loop = (mInst.brr.data[i] & 0x02)?true:false;	//最終ブロックのループフラグでループ有り無しを判断
			num_endflag++;
		}
	}
	
	//エンドフラグの数が１つ以外だとエラー
	if ( num_endflag != 1 ) {
		return false;
	}
	
	//エンドフラグの位置がループポイントより前だとエラー
	if ( endflag_pos < mInst.lp ) {
		return false;
	}
	
	//instデータの初期化
	getFileNameDeletingPathExt(mPath, mInst.pgname, PROGRAMNAME_MAX_LEN);
	mHasData = HAS_PGNAME;
	mInst.rate = 32000.0;
	mInst.basekey = 60;
	mInst.lowkey = 0;
	mInst.highkey = 127;
	mInst.ar = kDefaultValue_AR;
	mInst.dr = kDefaultValue_DR;
	mInst.sl = kDefaultValue_SL;
	mInst.sr = kDefaultValue_SR;
    mInst.sustainMode = true;
	mInst.volL = 100;
	mInst.volR	= 100;
	mInst.echo = false;
	mInst.bank = 0;
    mInst.monoMode = false;
    mInst.portamentoOn = false;
    mInst.portamentoRate = 0;
    mInst.noteOnPriority = 64;
    mInst.releasePriority = 0;
	mInst.isEmphasized = false;
	mInst.sourceFile[0] = 0;
	
	//同名で、拡張子が '.smpl'のファイルがある
	getInstFileName(mPath,mInstFilePath,PATH_LEN_MAX);
	FILE	*fp;
	fp = fopen(mInstFilePath, "r");
	if ( fp == NULL ) {
		goto success;
	}
	//ヘッダのチェック
	char	buf[1024];
	fgets(buf, sizeof(buf), fp);
	if ( strncmp(buf, "[C700SMPL]", 10) != 0 ) {
		goto success;
	}
	
	//inst情報を読み込む
	while ( fscanf(fp, "%[^=]s", buf) != EOF ) {
		getc(fp);	//"="の空読み
		if ( strcmp(buf, "progname")==0 ) {
			fscanf(fp, "%[^\n]s", buf);
			strncpy(mInst.pgname, buf, PROGRAMNAME_MAX_LEN);
			mHasData |= HAS_PGNAME;
		}
		else if ( strcmp(buf, "samplerate")==0 ) {
			fscanf(fp, "%lf", &mInst.rate );
			mHasData |= HAS_RATE;
		}
		else if ( strcmp(buf, "key")==0 ) {
			fscanf(fp, "%d", &mInst.basekey );
			mHasData |= HAS_BASEKEY;
		}
		else if ( strcmp(buf, "lowkey")==0 ) {
			fscanf(fp, "%d", &mInst.lowkey );
			mHasData |= HAS_LOWKEY;
		}
		else if ( strcmp(buf, "highkey")==0 ) {
			fscanf(fp, "%d", &mInst.highkey );
			mHasData |= HAS_HIGHKEY;
		}
		else if ( strcmp(buf, "ar")==0 ) {
			fscanf(fp, "%d", &mInst.ar );
			mHasData |= HAS_AR;
		}
		else if ( strcmp(buf, "dr")==0 ) {
			fscanf(fp, "%d", &mInst.dr );
			mHasData |= HAS_DR;
		}
		else if ( strcmp(buf, "sl")==0 ) {
			fscanf(fp, "%d", &mInst.sl );
			mHasData |= HAS_SL;
		}
		else if ( strcmp(buf, "sr")==0 ) {
			fscanf(fp, "%d", &mInst.sr );
			mHasData |= HAS_SR;
		}
		else if ( strcmp(buf, "volL")==0 ) {
			fscanf(fp, "%d", &mInst.volL );
			mHasData |= HAS_VOLL;
		}
		else if ( strcmp(buf, "volR")==0 ) {
			fscanf(fp, "%d", &mInst.volR );
			mHasData |= HAS_VOLR;
		}
		else if ( strcmp(buf, "echo")==0 ) {
			int	val;
			fscanf(fp, "%d", &val );
			mInst.echo = val?true:false;
			mHasData |= HAS_ECHO;
		}
		else if ( strcmp(buf, "bank")==0 ) {
			fscanf(fp, "%d", &mInst.bank );
			mHasData |= HAS_BANK;
		}
		else if ( strcmp(buf, "sustainMode")==0 ) {
			int	val;
			fscanf(fp, "%d", &val );
			mInst.sustainMode = val?true:false;
			mHasData |= HAS_SUSTAINMODE;
		}
        else if ( strcmp(buf, "monoMode")==0 ) {
			int	val;
			fscanf(fp, "%d", &val );
			mInst.monoMode = val?true:false;
			mHasData |= HAS_MONOMODE;
		}
		else if ( strcmp(buf, "portamentoOn")==0 ) {
			int	val;
			fscanf(fp, "%d", &val );
			mInst.portamentoOn = val?true:false;
			mHasData |= HAS_PORTAMENTOON;
		}
        else if ( strcmp(buf, "portamentoRate")==0 ) {
			fscanf(fp, "%d", &mInst.portamentoRate );
			mHasData |= HAS_PORTAMENTORATE;
		}
        else if ( strcmp(buf, "noteOnPriority")==0 ) {
			fscanf(fp, "%d", &mInst.noteOnPriority );
			mHasData |= HAS_NOTEONPRIORITY;
		}
        else if ( strcmp(buf, "releasePriority")==0 ) {
			fscanf(fp, "%d", &mInst.releasePriority );
			mHasData |= HAS_RELEASEPRIORITY;
		}
		else if ( strcmp(buf, "isemph")==0 ) {
			int	val;
			fscanf(fp, "%d", &val );
			mInst.isEmphasized = val?true:false;
			mHasData |= HAS_ISEMPHASIZED;
		}
		else if ( strcmp(buf, "srcfile")==0 ) {
			fscanf(fp, "%[^\n]s", buf);
			strncpy(mInst.sourceFile, buf, PATH_LEN_MAX);
			mHasData |= HAS_SOURCEFILE;
		}
		fgets(buf, sizeof(buf), fp);	//"\n"の空読み
    }
	fclose(fp);
	
	//読み込みに成功
success:
	mIsLoaded = true;
	return true;
}

//-----------------------------------------------------------------------------
bool RawBRRFile::Write()
{
	if ( strlen(mPath) == 0 ) {
		return false;
	}
	
	if ( mIsLoaded != true ) {
		return false;
	}
	//instファイルのパスを作成
	getInstFileName(mPath,mInstFilePath,PATH_LEN_MAX);
	
	//.brrファイルを書き出す
	FILE	*fp;
	fp = fopen(mPath, "wb");
	fwrite(mFileData, sizeof(unsigned char), mFileSize, fp);
	fclose(fp);
	
	//.instファイルに音色パラメータを書き出す
	fp = fopen(mInstFilePath, "w");
    if (fp == NULL) {
        return false;
    }
	fprintf(fp, "[C700SMPL]\n");
	fprintf(fp, "progname=%s\n",mInst.pgname);
	fprintf(fp, "samplerate=%lf\n",mInst.rate);
	fprintf(fp, "key=%d\n",mInst.basekey);
	fprintf(fp, "lowkey=%d\n",mInst.lowkey);
	fprintf(fp, "highkey=%d\n",mInst.highkey);
	fprintf(fp, "ar=%d\n",mInst.ar);
	fprintf(fp, "dr=%d\n",mInst.dr);
	fprintf(fp, "sl=%d\n",mInst.sl);
	fprintf(fp, "sr=%d\n",mInst.sr);
    fprintf(fp, "sustainMode=%d\n",mInst.sustainMode?1:0);
	fprintf(fp, "volL=%d\n",mInst.volL);
	fprintf(fp, "volR=%d\n",mInst.volR);
	fprintf(fp, "echo=%d\n",mInst.echo?1:0);
	fprintf(fp, "bank=%d\n",mInst.bank);
    fprintf(fp, "monoMode=%d\n",mInst.monoMode?1:0);
    fprintf(fp, "portamentoOn=%d\n",mInst.portamentoOn?1:0);
    fprintf(fp, "portamentoRate=%d\n",mInst.portamentoRate);
    fprintf(fp, "noteOnPriority=%d\n",mInst.noteOnPriority);
    fprintf(fp, "releasePriority=%d\n",mInst.releasePriority);
	fprintf(fp, "isemph=%d\n",mInst.isEmphasized?1:0);
	if ( mHasData & HAS_SOURCEFILE ) {
		fprintf(fp, "srcfile=%s\n",mInst.sourceFile);
	}
	fclose(fp);
	
	return true;
}

//-----------------------------------------------------------------------------
const InstParams *RawBRRFile::GetLoadedInst() const
{
	if ( mIsLoaded ) {
		return &mInst;
	}
	return NULL;
}

//-----------------------------------------------------------------------------
void RawBRRFile::StoreInst( const InstParams *inst )
{
	mInst = *inst;
	mHasData = 0x3fff;		//HAS_SOURCEFILE以外のフラグ
	if ( strlen(mInst.sourceFile) > 0 ) {
		mHasData |= HAS_SOURCEFILE;
	}
	
	//データサイズにループポイントを加えたサイズがファイルサイズ
	mFileSize = mInst.brr.size + 2;
	//ファイルサイズに一応上限を設ける
	if ( mFileSize > MAX_FILE_SIZE ) mFileSize = MAX_FILE_SIZE;
	
	//ファイルの先頭2バイトにループポイントをセット
	mFileData[0] = mInst.lp & 0xff;
	mFileData[1] = (mInst.lp >> 8) & 0xff;
	
	memcpy(mFileData+2, mInst.brr.data, mFileSize-2);
	
	mInst.brr.data = mFileData+2;
	
	if (mInst.loop) {
		mInst.brr.data[mInst.brr.size - 9] |= 2;
	}
	else {
		mInst.brr.data[mInst.brr.size - 9] &= ~2;
	}
	
	mIsLoaded = true;
}
