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
    void                SetGameTitle(const char *title);
    void                SetSongTitle(const char *title);
    void                SetNameOfDumper(const char *dumper);
    void                SetArtistOfSong(const char *artist);
    void                SetSongComments(const char *comments);
    void                SetPlaySeconds(int sec);
    void                SetFadeMs(int ms);
    
protected:
    
    typedef struct {
        
    } SpcHeader;
private:
    const unsigned char   *m_pSpcPlayCode;
    int                   mSpcPlayCodeSize;
    char                  mGameTitle[32];
    char                  mSongTitle[32];
    char                  mNameOfDumper[16];
    char                  mArtistOfSong[32];
    char                  mSongComments[32];
    int                   mPlaySec;
    int                   mFadeMs;
    
    void setHeaderString(char *dst, const char *src, int len);
};

#endif /* defined(__C700__SpcFileGenerate__) */
