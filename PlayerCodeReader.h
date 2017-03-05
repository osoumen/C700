//
//  PlayerCodeReader.h
//  C700
//
//  Created by osoumen on 2017/02/26.
//
//

#ifndef __C700__PlayerCodeReader__
#define __C700__PlayerCodeReader__

#include "ChunkReader.h"

class PlayerCodeReader : public ChunkReader {
public:
    PlayerCodeReader(const void *data, int dataSize, bool copy=false);
    PlayerCodeReader(const char *path);
    virtual ~PlayerCodeReader();
    
    virtual bool	Load();
	virtual bool	IsLoaded() const { return mIsLoaded; }

    // 以下の関数、IsLoadedがtrueでなければNULLを返す
    int getVersion();
    void *getSmcNativeVector();
    void *getSmcEmulationVector();
    void *getSmcPlayerCode();
    int getSmcPlayerCodeSize();
    void *getSpcPlayerCode();
    int getSpcPlayerCodeSize();

private:
    PlayerCodeReader();
    
    int     mVersion;            // BCD
    char    *m_pSmcNativeVector;    // 12バイト固定
    char    *m_pSmcEmulationVector; // 12バイト固定
    char    *m_pSmcPlayerCode;
    int     mSmcPlayerCodeSize;
    char    *m_pSpcPlayerCode;
    int     mSpcPlayerCodeSize;
};

#endif /* defined(__C700__PlayerCodeReader__) */
