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
    
    return true;
}
