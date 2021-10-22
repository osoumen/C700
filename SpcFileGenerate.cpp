//
//  SpcFileGenerate.cpp
//  C700
//
//  Created by osoumen on 2017/01/30.
//
//

#include "SpcFileGenerate.h"

SpcFileGenerate::SpcFileGenerate(int allocSize)
: PlayingFileGenerateBase(allocSize)
{
    //SetSpcPlayCode( spcplayercode, sizeof(spcplayercode) );
    strncpy(mSongTitle, "Song title", 32);
    strncpy(mGameTitle, "Game title", 32);
    strncpy(mNameOfDumper, "dumper", 16);
    strncpy(mArtistOfSong, "Artist of song", 32);
    strncpy(mSongComments, "Comments", 32);
    mPlaySec = 120;
    mFadeMs = 5000;

}

SpcFileGenerate::~SpcFileGenerate()
{
    
}

void SpcFileGenerate::SetSpcPlayCode( const void *code, int size, const void *smccode, int smcsize )
{
    m_pSpcPlayCode = (unsigned char*)code;
    mSpcPlayCodeSize = size;
	mSpcPlayCodeSize2 = findDspregAccCode( smccode, smcsize, &m_pSpcPlayCode2 );
}

bool SpcFileGenerate::WriteToFile( const char *path, const RegisterLogger &reglog, double tickPerSec )
{
    DataBuffer  spcFile(0x10200);
    
    // レジスタログの生成
    unsigned char *reglogData = new unsigned char [4 * 1024 * 1024];
    int loopPoint;
    int reglogSize = convertLogData( reglog, tickPerSec, reglogData, 4 * 1024 * 1024, &loopPoint, true );
    
	const int echoSize = reglog.getDspRegionData()[0x7d] << 11;
	int logAddr = 0x600 + echoSize;
	int brrAddr = reglog.getBrrRegionLocateAddr();
	bool outputScript700 = false;
	if (reglogSize > (brrAddr - logAddr))
	{
		// 64kに収まらない場合、700ファイルの形式で書き出す
		char script700path[PATH_LEN_MAX];
		get700FileName(path, script700path, PATH_LEN_MAX);
		exportScript700(script700path, reglog);
		outputScript700 = true;
	}
	
    // SPCヘッダの書き出し
    spcFile.writeData("SNES-SPC700 Sound File Data v0.30", 33);
    spcFile.writeByte(26, 3);
    spcFile.writeByte(30);      // Version minor
	if (outputScript700) {
		spcFile.writeU16(0x31);    // PC
	}
	else {
		spcFile.writeU16(0x100);    // PC
	}
    spcFile.writeByte(0);       // A
    spcFile.writeByte(0);       // X
    spcFile.writeByte(0);       // Y
    spcFile.writeByte(0x02);    // PSW
    spcFile.writeByte(0xff);    // SP
    spcFile.writeByte(0, 2);    // reserved
    
    spcFile.writeData(mSongTitle, 32);      // Song title
    spcFile.writeData(mGameTitle, 32);      // Game title
    spcFile.writeData(mNameOfDumper, 16);   // Name of dumper
    spcFile.writeData(mSongComments, 32);   // Comments
    {
        time_t timer;
        struct tm *local;
        timer = time(NULL);
        local = localtime(&timer);
        char dateStr[16];
        sprintf(dateStr, "%02d/%02d/%04d", local->tm_mon + 1, local->tm_mday, local->tm_year + 1900);
        spcFile.writeData(dateStr, 11);    // Date SPC was dumped (MM/DD/YYYY)
    }
    {
        char str[4] = {0,0,0,0};
        sprintf(str, "%d", mPlaySec);
        spcFile.writeData(str, 3);            // Number of seconds to play song before fading out
    }
    {
        char str[6] = {0,0,0,0,0,0};
        sprintf(str, "%d", mFadeMs);
        spcFile.writeData(str, 5);          // Length of fade in milliseconds
    }
    spcFile.writeData(mArtistOfSong, 32);   // Artist of song
    spcFile.writeByte(0);       // Default channel disables (0 = enable, 1 = disable)
    spcFile.writeByte(0);       // Emulator used to dump SPC: 0 = unknown, 1 = ZSNES, 2 = Snes9x
    spcFile.writeByte(0, 45);   // reserved (set to all 0's)
    
    // WaitTableの書き出し
	if (!outputScript700) {
		spcFile.setPos(0x130);
		writeWaitTable(spcFile, reglog);
	}
    
    // 実行コードの書き出し
	if (outputScript700) {
		spcFile.setPos(0x110);
		spcFile.writeData(m_pSpcPlayCode2, mSpcPlayCodeSize2);
	}
	else {
		spcFile.setPos(0x170);
		spcFile.writeData(m_pSpcPlayCode, mSpcPlayCodeSize);
	}
    
    // DIR領域の書き出し
    spcFile.setPos(0x300);
    spcFile.writeData(reglog.getDirRegionData(), reglog.getDirRegionSize());
    
    // レジスタログの書き出し
	if (!outputScript700) {
		// エコー領域の後に演奏データを入れる
		spcFile.setPos(0x700 + echoSize);    // DSP_EDL
		spcFile.writeData(reglogData, reglogSize);
	}
    
    // BRR領域の書き出し
    spcFile.setPos(0x100 + brrAddr);
    spcFile.writeData(reglog.getBrrRegionData(), reglog.getBrrRegionSize());
    
    // 初期変数の設定
	if (!outputScript700) {
		spcFile.setPos(0x102);                  // 変数領域へ移動
		spcFile.writeU16(logAddr & 0xffff);     // 演奏データ開始アドレス
		spcFile.writeU16(loopPoint & 0xffff);   // ループポイント
		spcFile.writeByte(0xff);                // _INITIAL_WAIT_FRAMES
	}
    
    // DSP領域の書き出し
    spcFile.setPos(0x10100);
    writeDspRegion(spcFile, reglog);
	
	// KON,KOFフラグをクリアする
	spcFile.setPos(0x1014c);
	spcFile.writeByte(0x00);
	spcFile.setPos(0x1015c);
	spcFile.writeByte(0x00);

    spcFile.WriteToFile(path);
    
    delete [] reglogData;
    
    return true;
}

void SpcFileGenerate::SetGameTitle(const char *title)
{
    setHeaderString(mGameTitle, title, 32);
}

void SpcFileGenerate::SetSongTitle(const char *title)
{
    setHeaderString(mSongTitle, title, 32);
}

void SpcFileGenerate::SetNameOfDumper(const char *dumper)
{
    setHeaderString(mNameOfDumper, dumper, 16);
}

void SpcFileGenerate::SetArtistOfSong(const char *artist)
{
    setHeaderString(mArtistOfSong, artist, 32);
}

void SpcFileGenerate::SetSongComments(const char *comments)
{
    setHeaderString(mSongComments, comments, 32);
}

void SpcFileGenerate::setHeaderString(char *dst, const char *src, int len)
{
    if (src[0] == 0) {
        return;
    }
    memset(dst, 0x20, len);
    for (int i=0; (i<len) && (src[i]!=0); i++) {
        dst[i] = src[i];
    }
}

void SpcFileGenerate::SetPlaySeconds(int sec)
{
    mPlaySec = sec;
    if (mPlaySec > 999) {
        mPlaySec = 999;
    }
    if (mPlaySec < 0) {
        mPlaySec = 0;
    }
}

void SpcFileGenerate::SetFadeMs(int ms)
{
    mFadeMs = ms;
    if (mFadeMs > 99999) {
        mFadeMs = 99999;
    }
    if (mFadeMs < 0) {
        mFadeMs = 0;
    }
}

//-----------------------------------------------------------------------------
void SpcFileGenerate::get700FileName( const char *path, char *out, int maxLen )
{
#if MAC
	CFURLRef	url = CFURLCreateFromFileSystemRepresentation(NULL, (UInt8*)path, strlen(path), false);
	CFURLRef	extlesspath=CFURLCreateCopyDeletingPathExtension(NULL, url);
	CFStringRef	filename = CFURLCopyFileSystemPath(extlesspath,kCFURLPOSIXPathStyle);
	CFStringGetCString(filename, out, maxLen-1, kCFStringEncodingUTF8);
	strcat(out, ".700");
	CFRelease(filename);
	CFRelease(extlesspath);
	CFRelease(url);
#else
	int	len = static_cast<int>(strlen(path));
	int extPos = len;
	for ( int i=len-1; i>=0; i-- ) {
		if ( path[i] == '.' ) {
			extPos = i;
			break;
		}
	}
	strncpy(out, path, extPos);
	out[extPos] = 0;
	strcat(out, ".700");
#endif
}

//-----------------------------------------------------------------------------
int SpcFileGenerate::findDspregAccCode( const void *code, int size, const unsigned char **outCode )
{
	const unsigned char *p_code = (unsigned char*)code;
	for (int i=0; i<(size-3); ++i) {
		if (p_code[i] == 0x8f && p_code[i+1] == 0x00 && p_code[i+2] == 0xf1) {
			*outCode = &p_code[i];
			return size - i;
		}
	}
	return 0;
}
