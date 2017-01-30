//
//  SpcFileGenerate.h
//  C700
//
//  Created by osoumen on 2017/01/30.
//
//

#ifndef __C700__SpcFileGenerate__
#define __C700__SpcFileGenerate__

#include "PlayingFileGenerateBase.h"

class SpcFileGenerate : public PlayingFileGenerateBase {
public:
    SpcFileGenerate(int allocSize=4*1024*1024);
    virtual ~SpcFileGenerate();
    
    virtual bool        WriteToFile( const char *path, const RegisterLogger &reglog, double tickPerSec=16000 );
    void                SetSpcPlayCode( const void *code, int size );
    
protected:
    
    typedef struct {
        
    } SpcHeader;
private:
    const unsigned char   *m_pSpcPlayCode;
    int                   mSpcPlayCodeSize;
};

#endif /* defined(__C700__SpcFileGenerate__) */
