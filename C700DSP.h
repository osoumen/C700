//
//  C700DSP.h
//  C700
//
//  Created by osoumen on 2014/11/30.
//
//

#ifndef __C700__C700DSP__
#define __C700__C700DSP__

#include "DspController.h"
#include "EchoKernel.h"
#include "C700defines.h"
#include "RegisterLogger.h"

//-----------------------------------------------------------------------------
typedef enum
{
    ATTACK,
    DECAY,
    SUSTAIN,
    RELEASE,
} env_state_t32;

class C700DSP {
public:
    enum SmcTimeBase {
        SmcTimeBaseNTSC,
        SmcTimeBasePAL
    };
    
    C700DSP();
    ~C700DSP();
    
    void ResetVoice(int voice);
    void KeyOffAll();
    void ResetEcho();
    
    void SetVoiceLimit(int value);
    void SetNewADPCM(bool value);
    void SetRealEmulation(bool value);
    void SetMainVolumeL(int value);
    void SetMainVolumeR(int value);
    void SetEchoVol_L( int value );
    void SetEchoVol_R( int value );
    void SetFeedBackLevel( int value );
    void SetDelayTime( int value );
    void SetFIRTap( int tap, int value );
    void SetAR(int v, int value);
    void SetDR(int v, int value);
    void SetARDR(int v, int ar, int dr);
    void SetSLSR(int v, int sl, int sr);
    void SetSL(int v, int value);
    void SetSR(int v, int value);
    void SetVol_L(int v, int value);
    void SetVol_R(int v, int value);
    void SetPitch(int v, int value);
    void SetEchoOn(int v, bool isOn);
    void SetEchoOnFlg(int flg, int mask);
    void SetSrcn(int v, int value);
    void SetDir(int value);
    
    void WriteRam(int addr, const unsigned char *data, int size);
    void WriteRam(int addr, unsigned char data);
    
    void KeyOffVoice(int v);
    void KeyOffVoiceFlg(int flg);
    void KeyOnVoice(int v);
    void KeyOnVoiceFlg(int flg);
    
    void Process1Sample(int &outl, int &outr);
    void BeginFrameProcess(double frameTime);
    
    bool IsHwAvailable() { return mDsp.IsHwAvailable(); }
    
    void setBrr(int v, unsigned char *brrdata, unsigned int loopPoint);
    
    void BeginRegisterLog();
    void MarkRegisterLogLoop();
    void EndRegisterLog();
    void SetSongRecordPath(const char *path);
    void SetRecSaveAsSpc(bool enable);
    void SetRecSaveAsSmc(bool enable);
    void SetTimeBaseForSmc(SmcTimeBase timebase);
    void SetGameTitle(const char *title);
    void SetSongTitle(const char *title);
    void SetNameOfDumper(const char *dumper);
    void SetArtistOfSong(const char *artist);
    void SetSongComments(const char *comments);
    char* GetSongRecordPath() { return mSongRecordPath; }
    bool GetRecSaveAsSpc() { return mRecSaveAsSpc; }
    bool GetRecSaveAsSmc() { return mRecSaveAsSmc; }
    SmcTimeBase GetTimeBaseForSmc() { return mTimeBaseForSmc; }
    char* GetGameTitle() { return mGameTitle; }
    char* GetSongTitle() { return mSongTitle; }
    char* GetNameOfDumper() { return mNameOfDumper; }
    char* GetArtistOfSong() { return mArtistOfSong; }
    char* GetSongComments() { return mSongComments; }
    int GetSongPlayCodeVer();
    void SetSmcNativeVector(const void *vec);
    void SetSmcEmulationVector(const void *vec);
    void SetSmcPlayerCode(const void *code, int size);
    void SetSpcPlayerCode(const void *code, int size);
    void SetSongPlayCodeVer(int ver);
    
private:
    bool writeDsp(int addr, unsigned char data);

    int saveRegisterLog(const char *path);
    bool canSaveRegisterLog();

    static void onDeviceReady(void *ref);
    static void onDeviceStop(void *ref);
    
private:
    struct DSPState {
        // 音源内部状態
		int				ar,dr,sl,sr;
        int             vol_l,vol_r;
		bool			ecen;
        bool            ecenNotWrited;
        
		unsigned char	*brrdata;
		unsigned int	loopPoint;
        
		int				memPtr;        /* Sample data memory pointer   */
		int             end;            /* End or loop after block      */
        int             loop;
		int             envcnt;         /* Counts to envelope update    */
		env_state_t32   envstate;       /* Current envelope state       */
		int             envx;           /* Last env height (0-0x7FFF)   */
		int             filter;         /* Last header's filter         */
		int             half;           /* Active nybble of BRR         */
		int             headerCnt;     /* Bytes before new header (0-8)*/
		int             mixfrac;        /* Fractional part of smpl pstn */	//サンプル間を4096分割した位置
		int				pitch;          /* Sample pitch (4096->32000Hz) */
		int             range;          /* Last header's range          */
		int             sampptr;        /* Where in sampbuf we are      */
		int				smp1;           /* Last sample (for BRR filter) */
		int				smp2;           /* Second-to-last sample decoded*/
		int				sampbuf[4];   /* Buffer for Gaussian interp   */
        
		void Reset();
	};
    EchoKernel		mEcho[2];
    
    DSPState		mVoice[kMaximumVoices];		//ボイスの状況
	
	int				mVoiceLimit;
	int				mMainVolume_L;
	int				mMainVolume_R;
    int             mDirAddr;
	bool			mNewADPCM;
    bool            mUseRealEmulation;
    
    int             mEchoVolL;
    int             mEchoVolR;
    int             mEchoStartAddr;
    int             mEchoDelay;
    int             mEchoFeedBack;
    int             mEchoEnableWait;
    
    unsigned char   mRam[65536];
    int             mBrrStartAddr;
    int             mBrrEndAddr;
    
    DspController   mDsp;
    
    RegisterLogger  mLogger;
    bool            mIsLoggerRunning;
    int             mLoggerSamplePos;

    char            mSongRecordPath[PATH_LEN_MAX];
    bool            mRecSaveAsSpc;
    bool            mRecSaveAsSmc;
    SmcTimeBase     mTimeBaseForSmc;
    char            mGameTitle[33];
    char            mSongTitle[33];
    char            mNameOfDumper[17];
    char            mArtistOfSong[33];
    char            mSongComments[33];
    unsigned char   mSmcNativeVector[12];
    unsigned char   mSmcEmulationVector[12];
    unsigned char   *mSmcPlayerCode;
    int             mSmcPlayerCodeSize;
    unsigned char   *mSpcPlayerCode;
    int             mSpcPlayerCodeSize;
    int             mCodeVer;
};

#endif /* defined(__C700__C700DSP__) */
