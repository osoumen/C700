//
//  PlayerCodeReader.cpp
//  C700
//
//  Created by osoumen on 2017/02/26.
//
//

#include "PlayerCodeReader.h"

#define CHUNKID(a, b, c, d) \
((((int)a) << 0) | (((int)b) << 8) | (((int)c) << 16) | (((int)d) << 24))

PlayerCodeReader::PlayerCodeReader(const void *data, int dataSize, bool copy)
: ChunkReader(data, dataSize, copy)
, m_pSmcNativeVector(NULL)
, m_pSmcEmulationVector(NULL)
, m_pSmcPlayerCode(NULL)
, mSmcPlayerCodeSize(0)
, m_pSpcPlayerCode(NULL)
, mSpcPlayerCodeSize(0)
{
    Load();
}

PlayerCodeReader::PlayerCodeReader(const char *path)
: ChunkReader(path)
, m_pSmcNativeVector(NULL)
, m_pSmcEmulationVector(NULL)
, m_pSmcPlayerCode(NULL)
, mSmcPlayerCodeSize(0)
, m_pSpcPlayerCode(NULL)
, mSpcPlayerCodeSize(0)
{
    Load();
}

PlayerCodeReader::~PlayerCodeReader()
{
    if (m_pSmcNativeVector != NULL) {
        delete m_pSmcNativeVector;
    }
    if (m_pSmcEmulationVector != NULL) {
        delete m_pSmcEmulationVector;
    }
    if (m_pSmcPlayerCode != NULL) {
        delete m_pSmcPlayerCode;
    }
    if (m_pSpcPlayerCode != NULL) {
        delete m_pSpcPlayerCode;
    }
}

bool PlayerCodeReader::Load()
{
    // 必ず先頭にあるべきheadチャンクを確認する
    int		ckType;
    long	ckSize;
    readChunkHead(&ckType, &ckSize);
    if (ckType != CHUNKID('h','e','a','d') ) {
        return false;
    }
    if (ckSize < 16) {
        return false;
    }
    char headerStr[17];
    readData(headerStr, 16, &ckSize);
    if (strncmp(headerStr, "ppse song player", 16) != 0) {
        return false;
    }
    
    int checkFlag = 0;  // 必要なチャンクが全てあれば0x1fになる
    
    while ( (mDataSize - mDataPos) > (int)sizeof( MyChunkHead ) ) {
		readChunkHead(&ckType, &ckSize);
        ckSize &= 0x7fff;   // 現状wla-dxの方で0x8000 で出力されてしまうため
		switch (ckType) {
            case CHUNKID('v','e','r','s'):
                readData(&mVersion, ckSize, &ckSize);
                checkFlag |= 0x01;
                break;
            case CHUNKID('s','p','c','p'):
                m_pSpcPlayerCode = new char[ckSize];
                readData(m_pSpcPlayerCode, ckSize, &ckSize);
                mSpcPlayerCodeSize = ckSize;
                checkFlag |= 0x02;
                break;
            case CHUNKID('e','m','u','v'):
                m_pSmcEmulationVector = new char[ckSize];
                readData(m_pSmcEmulationVector, ckSize, &ckSize);
                if (ckSize == 12) checkFlag |= 0x04;
                break;
            case CHUNKID('n','t','v','v'):
                m_pSmcNativeVector = new char[ckSize];
                readData(m_pSmcNativeVector, ckSize, &ckSize);
                if (ckSize == 12) checkFlag |= 0x08;
                break;
            case CHUNKID('s','m','c','p'):
                m_pSmcPlayerCode = new char[ckSize];
                readData(m_pSmcPlayerCode, ckSize, &ckSize);
                mSmcPlayerCodeSize = ckSize;
                checkFlag |= 0x10;
                break;
            default:
				//不明チャンクの場合は飛ばす
				AdvDataPos(ckSize);
				break;
        }
    }
    
    // 正常にロードできている
    if (checkFlag == 0x1f) {
        mIsLoaded = true;
    }
    return true;
}

int PlayerCodeReader::getVersion()
{
    return mVersion;
}

void *PlayerCodeReader::getSmcNativeVector()
{
    return m_pSmcNativeVector;
}

void *PlayerCodeReader::getSmcEmulationVector()
{
    return m_pSmcEmulationVector;
}

void *PlayerCodeReader::getSmcPlayerCode()
{
    return m_pSmcPlayerCode;
}

int PlayerCodeReader::getSmcPlayerCodeSize()
{
    return mSmcPlayerCodeSize;
}

void *PlayerCodeReader::getSpcPlayerCode()
{
    return m_pSpcPlayerCode;
}

int PlayerCodeReader::getSpcPlayerCodeSize()
{
    return mSpcPlayerCodeSize;
}
