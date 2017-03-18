//
//  SmcFileGenerate.h
//  C700
//
//  Created by osoumen on 2017/01/29.
//
//

#ifndef __C700__SmcFileGenerate__
#define __C700__SmcFileGenerate__

#include "PlayingFileGenerateBase.h"

class SmcFileGenerate : public PlayingFileGenerateBase {
public:
    SmcFileGenerate(int allocSize=4*1024*1024);
    virtual ~SmcFileGenerate();
    
    virtual bool        WriteToFile( const char *path, const RegisterLogger &reglog, double tickPerSec=15734 );
    void                SetSmcPlayCode( const void *code, int size, const void *nativeVector, const void *emuVector );
    void                SetGameTitle(const char *title);
    void                SetCountryCode(int country);
    
protected:
    typedef struct {
        char title[21];
        unsigned char map;
        unsigned char cartType;
        unsigned char romSize;
        unsigned char sramSize;
        unsigned char country;
        unsigned char licenseeCode;
        unsigned char version;
        unsigned short checksum_complement;
        unsigned short checksum;
    } SmcHeader;
    
private:
    const unsigned char   *m_pSmcPlayCode;
    int                   mSmcPlayCodeSize;
    unsigned char         mNativeVector[12];
    unsigned char         mEmuVector[12];
    char                  mGameTitle[22];
    unsigned char         mCountryCode;
    
    void setHeaderString(char *dst, const char *src, int len);
};

#endif /* defined(__C700__SmcFileGenerate__) */
