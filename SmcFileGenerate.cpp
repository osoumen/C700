//
//  SmcFileGenerate.cpp
//  C700
//
//  Created by osoumen on 2017/01/29.
//
//

#include "SmcFileGenerate.h"
#include "smcplayercode.h"

SmcFileGenerate::SmcVector s_nativeVector = {0x88b5, 0x88b5, 0x88b5, 0x88b5, 0x0000, 0x8674};
SmcFileGenerate::SmcVector s_emuVector = {0x88b5, 0x0000, 0x88b5, 0x88b5, 0x8000, 0x88b5};

SmcFileGenerate::SmcFileGenerate(int allocSize)
: PlayingFileGenerateBase(allocSize)
{
    SetSmcPlayCode( smcplayercode, sizeof(smcplayercode), &s_nativeVector, &s_emuVector );
}

SmcFileGenerate::~SmcFileGenerate()
{
    
}

void SmcFileGenerate::SetSmcPlayCode( const void *code, int size, const SmcVector *nativeVector, const SmcVector *emuVector )
{
    m_pSmcPlayCode = (unsigned char*)code;
    mSmcPlayCodeSize = size;
    m_pNativeVector = nativeVector;
    m_pEmuVector = emuVector;
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
    strncpy(header.title, "C700Player           ", 21); // TODO: 設定方法
    header.map = 0x30;      // FastLoROM
    header.cartType = 0x00; // ROM only
    header.romSize = 1;
    while ((1024 << header.romSize) < smcFile.GetDataSize()) {
        header.romSize++;
    }
    smcFile.setPos(1024 << header.romSize);
    header.sramSize = 0;
    header.licenseeCode = 0;
    header.country = 0;
    header.version = 0;
    
    smcFile.setPos(0x7fb2);
    smcFile.writeData("SNES", 4);
    smcFile.setPos(0x7fc0);
    smcFile.writeData(&header, sizeof(SmcHeader));
    
    // ベクタの設定
    smcFile.setPos(0x7fe4);
    smcFile.writeData(m_pNativeVector, sizeof(SmcVector));
    smcFile.setPos(0x7ff4);
    smcFile.writeData(m_pEmuVector, sizeof(SmcVector));
    
    // CRCの修正
    unsigned short sum = 0;
    for (int i=0; i<smcFile.GetDataSize(); i++) {
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
