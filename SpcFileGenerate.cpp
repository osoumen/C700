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
}

SpcFileGenerate::~SpcFileGenerate()
{
    
}

void SpcFileGenerate::SetSpcPlayCode( const void *code, int size )
{
    m_pSpcPlayCode = (unsigned char*)code;
    mSpcPlayCodeSize = size;
}

bool SpcFileGenerate::WriteToFile( const char *path, const RegisterLogger &reglog, double tickPerSec )
{
    DataBuffer  spcFile(0x10200);
    
    // レジスタログの生成
    unsigned char *reglogData = new unsigned char [4 * 1024 * 1024];
    int loopPoint;
    int reglogSize = convertLogData( reglog, tickPerSec, reglogData, 4 * 1024 * 1024, &loopPoint, true );
    
    // TODO: 64kに収まらない場合、700ファイルの形式で書き出す
    
    // SPCヘッダの書き出し
    spcFile.writeData("SNES-SPC700 Sound File Data v0.30", 33);
    spcFile.writeByte(26, 3);
    spcFile.writeByte(30);      // Version minor
    spcFile.writeU16(0x100);    // PC
    spcFile.writeByte(0);       // A
    spcFile.writeByte(0);       // X
    spcFile.writeByte(0);       // Y
    spcFile.writeByte(0x02);    // PSW
    spcFile.writeByte(0xff);    // SP
    spcFile.writeByte(0, 2);    // reserved
    
    spcFile.writeData("C700 spctest                    ", 32);  // Song title
    spcFile.writeData("Game title                      ", 32);  // Game title
    spcFile.writeData("dumper          ", 16);                  // Name of dumper
    spcFile.writeData("Comments                        ", 32);  // Comments
    spcFile.writeData("2016/07/17", 11);                        // Date SPC was dumped (MM/DD/YYYY)
    spcFile.writeData("120", 3);                                // Number of seconds to play song before fading out
    spcFile.writeData("20000", 5);                              // Length of fade in milliseconds
    spcFile.writeData("Artist of song                  ", 32);  // Artist of song
    spcFile.writeByte(0);       // Default channel disables (0 = enable, 1 = disable)
    spcFile.writeByte(0);       // Emulator used to dump SPC: 0 = unknown, 1 = ZSNES, 2 = Snes9x
    spcFile.writeByte(0, 45);   // reserved (set to all 0's)
    
    // WaitTableの書き出し
    spcFile.setPos(0x130);
    writeWaitTable(spcFile, reglog);
    
    // 実行コードの書き出し
    spcFile.setPos(0x170);
    spcFile.writeData(m_pSpcPlayCode, mSpcPlayCodeSize);
    
    // DIR領域の書き出し
    spcFile.setPos(0x300);
    spcFile.writeData(reglog.getDirRegionData(), reglog.getDirRegionSize());
    
    // エコー領域を空ける
    spcFile.setPos(0x700 + (reglog.getDspRegionData()[0x7d] << 11));    // DSP_EDL
    
    // レジスタログの書き出し
    int logAddr = spcFile.getPos() - 0x100;
    int brrAddr = reglog.getBrrRegionLocateAddr();
    if (reglogSize > (brrAddr - logAddr)) {
        reglogSize = brrAddr - logAddr;
    }
    spcFile.writeData(reglogData, reglogSize);
    
    // BRR領域の書き出し
    spcFile.setPos(0x100 + brrAddr);
    spcFile.writeData(reglog.getBrrRegionData(), reglog.getBrrRegionSize());
    
    // 初期変数の設定
    spcFile.setPos(0x102);                  // 変数領域へ移動
    spcFile.writeU16(logAddr & 0xffff);     // 演奏データ開始アドレス
    spcFile.writeU16(loopPoint & 0xffff);   // ループポイント
    spcFile.writeByte(0xff);                // _INITIAL_WAIT_FRAMES
    
    // DSP領域の書き出し
    spcFile.setPos(0x10100);
    writeDspRegion(spcFile, reglog);
    
    spcFile.WriteToFile(path);
    
    delete [] reglogData;
    
    return true;
}
