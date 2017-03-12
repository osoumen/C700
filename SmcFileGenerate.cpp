//
//  SmcFileGenerate.cpp
//  C700
//
//  Created by osoumen on 2017/01/29.
//
//

#include "SmcFileGenerate.h"

SmcFileGenerate::SmcFileGenerate(int allocSize)
: PlayingFileGenerateBase(allocSize)
{
    //SetSmcPlayCode( smcplayercode, sizeof(smcplayercode), &s_nativeVector, &s_emuVector );
    strncpy(mGameTitle, "C700Player", 21);
    mCountryCode = 0;
}

SmcFileGenerate::~SmcFileGenerate()
{
    
}

void SmcFileGenerate::SetSmcPlayCode( const void *code, int size, const void *nativeVector, const void *emuVector )
{
    m_pSmcPlayCode = (unsigned char*)code;
    mSmcPlayCodeSize = size;
    memcpy(mNativeVector, nativeVector, 12);
    memcpy(mEmuVector, emuVector, 12);
}

bool SmcFileGenerate::WriteToFile( const char *path, const RegisterLogger &reglog, double tickPerSec )
{
    DataBuffer  smcFile(1024 * 1024 * 4);
    
    // 実行コードの書き出し
    smcFile.writeData(m_pSmcPlayCode, mSmcPlayCodeSize);
    
    // DIR領域の書き出し
    smcFile.setPos(0x7b00);
    writeDirRegionWithHeader(smcFile, reglog);
    
    // BRR領域の書き出し
    smcFile.setPos(0x8000);
    writeBrrRegionWithHeader(smcFile, reglog, 0x8000);
    
    // レジスタログの書き出し
    smcFile.setPos(0x18000);
    writeRegLogWithLoopPoint(smcFile, reglog, tickPerSec);
    
    // WaitTableの書き出し
    smcFile.setPos(0x7a00);
    writeWaitTable(smcFile, reglog);
    
    // ROMヘッダーの書き出し
    SmcHeader header;
    memset(&header, 0, sizeof(SmcHeader));
    setHeaderString(header.title, mGameTitle, 21);
    header.map = 0x30;      // FastLoROM
    header.cartType = 0x00; // ROM only
    header.romSize = 1;
    while ((1024 << header.romSize) < smcFile.GetDataUsed()) {
        header.romSize++;
    }
    smcFile.setPos(1024 << header.romSize);
    header.sramSize = 0;
    header.country = mCountryCode;
    header.licenseeCode = 0;
    header.version = 0;
    
    smcFile.setPos(0x7fb2);
    smcFile.writeData("SNES", 4);
    smcFile.setPos(0x7fc0);
    smcFile.writeData(&header, sizeof(SmcHeader));
    
    // ベクタの設定
    smcFile.setPos(0x7fe4);
    smcFile.writeData(mNativeVector, 12);
    smcFile.setPos(0x7ff4);
    smcFile.writeData(mEmuVector, 12);
    
    // CRCの修正
    unsigned short sum = 0;
    for (int i=0; i<smcFile.GetDataUsed(); i++) {
        sum += smcFile.GetDataPtr()[i];
    }
    header.checksum = sum;
    header.checksum += sum & 0xff;
    header.checksum += (sum >> 8) & 0xff;
    header.checksum += (~sum) & 0xff;
    header.checksum += (~sum >> 8) & 0xff;
    header.checksum_complement = ~header.checksum;
    smcFile.setPos(0x7fc0);
    smcFile.writeData(&header, sizeof(SmcHeader));
    
    smcFile.WriteToFile(path);
    
    return true;
}

void SmcFileGenerate::SetGameTitle(const char *title)
{
    strncpy(mGameTitle, title, 21);
    mGameTitle[21] = 0;
}

void SmcFileGenerate::setHeaderString(char *dst, const char *src, int len)
{
    memset(dst, 0x20, len);
    for (int i=0; (i<len) && (src[i]!=0); i++) {
        dst[i] = src[i];
    }
}

void SmcFileGenerate::SetCountryCode(int country)
{
    mCountryCode = country;
}