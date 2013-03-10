/*
 *  RawBRRFile.cpp
 *  Chip700
 *
 *  Created by osoumen on 12/10/10.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "RawBRRFile.h"

void getFileNameDeletingPathExt( const char *path, char *out, int maxLen );

//-----------------------------------------------------------------------------
void getInstFileName( const char *path, char *out, int maxLen )
{
#if MAC
	CFURLRef	url = CFURLCreateFromFileSystemRepresentation(NULL, (UInt8*)path, strlen(path), false);
	CFURLRef	extlesspath=CFURLCreateCopyDeletingPathExtension(NULL, url);
	CFStringRef	filename = CFURLCopyFileSystemPath(extlesspath,kCFURLPOSIXPathStyle);
	CFStringGetCString(filename, out, maxLen-1, kCFStringEncodingUTF8);
	strcat(out, ".inst");
	CFRelease(filename);
	CFRelease(extlesspath);
	CFRelease(url);
#else
	//Windowsでの拡張子除去処理
	int	len = strlen(path);
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
	strcat(out, ".inst");
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
#if MAC
	CFURLRef	url = CFURLCreateFromFileSystemRepresentation(NULL, (UInt8*)mPath, strlen(mPath), false);
	
	CFReadStreamRef	filestream = CFReadStreamCreateWithFile(NULL, url);
	if (CFReadStreamOpen(filestream) == false) {
		CFRelease( url );
		return false;
	}
	
	CFIndex	readbytes=CFReadStreamRead(filestream, mFileData, MAX_BRR_SIZE);
	/*
	if (readbytes < SPC_READ_SIZE) {
		CFRelease( url );
		CFReadStreamClose(filestream);
		return false;
	}
	 */
	mFileSize = readbytes;
	CFReadStreamClose(filestream);
	CFRelease( url );
#else
	//VSTのときのファイル読み込み処理
	HANDLE	hFile;
	
	hFile = CreateFile( mPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile != INVALID_HANDLE_VALUE ) {
		DWORD	readSize;
		ReadFile( hFile, m_pFileData, MAX_BRR_SIZE, &readSize, NULL );
		mFileSize = readSize;
		CloseHandle( hFile );
	}
#endif
	//先頭2バイト(リトルエンディアン)の数値+2よりファイルサイズが大きい
	mVoice.lp = (mFileData[1] << 8) | mFileData[0];
	if ( mVoice.lp+2 >= mFileSize ) {
		return false;
	}
	
	//ループポイントの次のバイトから9バイトずつ進め、ファイルの終端までにエンドフラグが出現する
	mVoice.brr.data = mFileData+2;
	mVoice.brr.size = mFileSize - 2;
	int	end_flag = 0;
	int	end_ptr = 0;
	for ( int i=0; i<mVoice.brr.size; i+=9 ) {
		end_flag |= mVoice.brr.data[i] & 0x01;
		if ( end_flag ) {
			end_ptr = i;
			mVoice.loop = (mVoice.brr.data[i] & 0x02)?true:false;	//最終ブロックのループフラグでループ有り無しを判断
			break;
		}
	}
	if ( end_flag == 0 ) {
		return false;
	}
	
	//最初のエンドフラグの出現位置が、ループポイント以降の位置である
	if ( end_ptr <= mVoice.lp ) {
		return false;
	}
	
	//instデータの初期化
	getFileNameDeletingPathExt( mPath, mVoice.pgname, PROGRAMNAME_MAX_LEN );
	mHasData = HAS_PGNAME;
	/*
	mVoice.rate = 32000.0;
	mVoice.basekey = 60;
	mVoice.lowkey = 0;
	mVoice.highkey = 127;
	mVoice.ar = 15;
	mVoice.dr = 7;
	mVoice.sl = 7;
	mVoice.sr = 0;
	mVoice.volL = 100;
	mVoice.volR	= 100;
	mVoice.echo = false;
	mVoice.bank = 0;
	mVoice.isEmphasized = false;
	mVoice.sourceFile[0] = 0;
	 */
	
	//同名で、拡張子が '.inst'のファイルがある
	getInstFileName(mPath,mInstFile,PATH_LEN_MAX);
	FILE	*fp;
	fp = fopen(mInstFile, "r");
	if ( fp == NULL ) {
		goto success;
	}
	//ヘッダのチェック
	char	buf[1024];
	fgets(buf, sizeof(buf), fp);
	if ( strncmp(buf, "[C700INST]", 10) != 0 ) {
		goto success;
	}
	
	//inst情報を読み込む
	while ( fscanf(fp, "%[^=]s", buf) != EOF ) {
		getc(fp);	//"="の空読み
		if ( strcmp(buf, "progname")==0 ) {
			fscanf(fp, "%[^\n]s", buf);
			strncpy(mVoice.pgname, buf, PROGRAMNAME_MAX_LEN);
			mHasData |= HAS_PGNAME;
		}
		else if ( strcmp(buf, "samplerate")==0 ) {
			fscanf(fp, "%lf", &mVoice.rate );
			mHasData |= HAS_RATE;
		}
		else if ( strcmp(buf, "key")==0 ) {
			fscanf(fp, "%d", &mVoice.basekey );
			mHasData |= HAS_BASEKEY;
		}
		else if ( strcmp(buf, "lowkey")==0 ) {
			fscanf(fp, "%d", &mVoice.lowkey );
			mHasData |= HAS_LOWKEY;
		}
		else if ( strcmp(buf, "highkey")==0 ) {
			fscanf(fp, "%d", &mVoice.highkey );
			mHasData |= HAS_HIGHKEY;
		}
		else if ( strcmp(buf, "ar")==0 ) {
			fscanf(fp, "%d", &mVoice.ar );
			mHasData |= HAS_AR;
		}
		else if ( strcmp(buf, "dr")==0 ) {
			fscanf(fp, "%d", &mVoice.dr );
			mHasData |= HAS_DR;
		}
		else if ( strcmp(buf, "sl")==0 ) {
			fscanf(fp, "%d", &mVoice.sl );
			mHasData |= HAS_SL;
		}
		else if ( strcmp(buf, "sr")==0 ) {
			fscanf(fp, "%d", &mVoice.sr );
			mHasData |= HAS_SR;
		}
		else if ( strcmp(buf, "volL")==0 ) {
			fscanf(fp, "%d", &mVoice.volL );
			mHasData |= HAS_VOLL;
		}
		else if ( strcmp(buf, "volR")==0 ) {
			fscanf(fp, "%d", &mVoice.volR );
			mHasData |= HAS_VOLR;
		}
		else if ( strcmp(buf, "echo")==0 ) {
			int	val;
			fscanf(fp, "%d", &val );
			mVoice.echo = val?true:false;
			mHasData |= HAS_ECHO;
		}
		else if ( strcmp(buf, "bank")==0 ) {
			fscanf(fp, "%d", &mVoice.bank );
			mHasData |= HAS_BANK;
		}
		else if ( strcmp(buf, "isemph")==0 ) {
			int	val;
			fscanf(fp, "%d", &val );
			mVoice.isEmphasized = val?true:false;
			mHasData |= HAS_ISEMPHASIZED;
		}
		else if ( strcmp(buf, "srcfile")==0 ) {
			fscanf(fp, "%[^\n]s", buf);
			strncpy(mVoice.sourceFile, buf, PATH_LEN_MAX);
			mHasData |= HAS_SOURCEFILE;
		}
		fgets(buf, sizeof(buf), fp);	//"\n"の空読み
    }
	
	//読み込みに成功
success:
	mIsLoaded = true;
	return true;
}

//-----------------------------------------------------------------------------
bool RawBRRFile::Write()
{
	return false;
}

//-----------------------------------------------------------------------------
const VoiceParams *RawBRRFile::GetLoadedVoice() const
{
	if ( mIsLoaded ) {
		return &mVoice;
	}
	return NULL;
}
