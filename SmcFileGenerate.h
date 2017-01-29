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
    typedef struct {
        unsigned short cop;
        unsigned short abort;
        unsigned short nmi;
        unsigned short res;
        unsigned short brk;
        unsigned short irq;
    } SmcVector;

    SmcFileGenerate(int allocSize=4*1024*1024);
    virtual ~SmcFileGenerate();
    
    virtual bool        WriteToFile( const char *path, const RegisterLogger &reglog, double tickPerSec=15734 );
    void                SetSmcPlayCode( const void *code, int size, const SmcVector *nativeVector, const SmcVector *emuVector );
    
protected:
    typedef struct {
        char title[21];
        unsigned char map;
        unsigned char cartType;
        unsigned char romSize;
        unsigned char sramSize;
        unsigned char licenseeCode;
        unsigned char country;
        unsigned char version;
        unsigned short checksum_complement;
        unsigned short checksum;
    } SmcHeader;
    
private:
    const unsigned char   *m_pSmcPlayCode;
    int                   mSmcPlayCodeSize;
    const SmcVector       *m_pNativeVector;
    const SmcVector       *m_pEmuVector;
};

#endif /* defined(__C700__SmcFileGenerate__) */
