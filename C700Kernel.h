/*
 *  C700Kernel.h
 *  C700
 *
 *  Created by osoumen on 12/10/12.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "C700defines.h"
#include "C700Driver.h"
#include "C700Parameters.h"
#include "C700Properties.h"
#include "ChunkReader.h"
#include "PlayerCodeReader.h"

class C700Kernel : public C700Parameters
{
public:
	C700Kernel();
	virtual ~C700Kernel();
	
	void			Reset();
	
	void			SetSampleRate( double samplerate ) {
        mSampleRate = samplerate;
        mDriver.SetSampleRate(samplerate);
    }
	void			SetTempo( double tempo );       // 毎フレームRenderを呼ぶ前に呼び出す
    void            SetCurrentSampleInTimeLine( double currentSample ); // 毎フレームRenderを呼ぶ前に呼び出す
    void            SetIsPlaying( bool isPlaying );
	virtual float	GetParameter( int id );
	virtual bool	SetParameter( int id, float value );

	float			GetPropertyValue( int inID );
    double          GetPropertyDoubleValue( int inID );
    int             GetPropertyPtrDataSize( int inID );
    const void      *GetPropertyPtrValue( int inID );
    bool            GetPropertyStructValue( int inID, void *outData );
	bool			SetPropertyValue( int inID, float value );
    bool            SetPropertyDoubleValue( int inID, double value );
	bool			SetPropertyPtrValue( int inID, const void *inPtr, int size );
	const BRRData	*GetBRRData();
	const BRRData	*GetBRRData(int prog);
	bool			SetBRRData( const unsigned char *data, int size, int prog=-1, bool reset=true, bool notify=true );
	
	static int		GetPresetNum();
	static const char	*GetPresetName( int num );
	bool			SelectPreset( int num );
	
	void			Render( unsigned int frames, float *output[2] );
	
	void			HandleNoteOn( int ch, int note, int vel, int uniqueID, int inFrame );
	void			HandleNoteOff( int ch, int note, int uniqueID, int inFrame );
	void			HandlePitchBend( int ch, int pitch1, int pitch2, int inFrame );
	void			HandleControlChange( int ch, int controlNum, int value, int inFrame );
	void			HandleProgramChange( int ch, int pg, int inFrame );
	void			HandleResetAllControllers( int ch, int inFrame );
	void			HandleAllNotesOff( int ch, int inFrame );
	void			HandleAllSoundOff( int ch, int inFrame );
	
	C700Driver	*GetDriver() { return &mDriver; }
	
	void			SetPropertyNotifyFunc( void (*func) (int propID, void* userData), void* userData );
	void			SetParameterSetFunc( void (*func) (int paramID, float value, void* userData) , void* userData );
	
	const InstParams* GetVP() { return mVPset; }
    void              SetVP(int pg, const InstParams *vp) {
		BRRData temp = *mVPset[pg].getBRRData();
		mVPset[pg] = *vp;
		mVPset[pg].setBRRData(&temp);
		mDriver.RefreshKeyMap();
	}
    
    void            CorrectLoopFlagForSave(int pgnum);
    
    double          GetProcessDelayTime() { return mDriver.GetProcessDelayTime(); }

    void            SetPropertyToChunk(ChunkReader *chunk, const PropertyDescription &prop);
    bool            RestorePropertyFromData(DataBuffer *data, int ckSize, const PropertyDescription &prop);
#if AU
    static void     AddNumToDictionary(CFMutableDictionaryRef dict, CFStringRef key, int value);
    static void     AddFloatToDictionary(CFMutableDictionaryRef dict, CFStringRef key, float value);
    static void     AddDoubleToDictionary(CFMutableDictionaryRef dict, CFStringRef key, double value);
    static void     AddBooleanToDictionary(CFMutableDictionaryRef dict, CFStringRef key, bool value);
    void            AddStringToDictionary(CFMutableDictionaryRef dict, CFStringRef key, const char *string);
    void            AddFilePathToDictionary(CFMutableDictionaryRef dict, CFStringRef key, const char *path);
    void            AddDataToDictionary(CFMutableDictionaryRef dict, CFStringRef key, const void *data, int size);
    void            SetPropertyToDict(CFMutableDictionaryRef dict, const PropertyDescription &prop);
    int             CreatePGDataDic(CFDictionaryRef *data, int pgnum);
    bool            RestorePropertyFromDict(CFDictionaryRef dict, const PropertyDescription &prop);
    void            RestorePGDataDic(CFPropertyListRef data, int pgnum);
#else
    bool            SetPGDataToChunk(ChunkReader *chunk, int pgnum);
    int             GetPGChunkSize( int pgnum );
    bool            RestorePGDataFromChunk( ChunkReader *chunk, int pgnum );
#endif
    
private:
	int		GetTotalRAM();
	
	void (*propertyNotifyFunc) (int propID, void* userData);
	void	*propNotifyUserData;
	void (*parameterSetFunc) (int paramID, float value, void* userData);
	void	*paramSetUserData;
	
	int					mEditProg;		// 編集中のプログラムNo.
	int					mEditChannel;	// 編集中のチャンネル
	double				mTempo;
	// MIDIチャンネルノート別発音数
	int					mOnNotes[16];
	int					mMaxNote[16];
	int					mTotalOnNotes;
	
	InstParams			mVPset[128];
	// エコー
	float				mFilterBand[5];
    
    // RPN, NRPN
    int                 mDataEntryValue[16];
    int                 mIsSettingNRPN[16];
    int                 mRPN[16];
    int                 mNRPN[16];
	
	C700Driver      mDriver;
    
    bool            mIsHwAvailable;
    
    std::map<int, PropertyDescription>  mPropertyParams;
    
    PlayerCodeReader *mCodeFile;
    bool            mGlobalSettingsHasChanged;
    
    double          mCurrentPosInTimeLine;
    bool            mIsPlaying;
    double          mSampleRate;
    
    double          mRecordStartBeatPos;
    double          mRecordLoopStartBeatPos;
    double          mRecordEndBeatPos;
	
    void            setRPNLSB(int ch, int value);
    void            setRPNMSB(int ch, int value);
    void            setNRPNLSB(int ch, int value);
    void            setNRPNMSB(int ch, int value);
    void            setDataEntryLSB(int ch, int value, int inFrame);
    void            setDataEntryMSB(int ch, int value, int inFrame);
    void            sendDataEntryValue(int ch, int inFrame);

    const char		*GetSourceFilePath();
	bool			SetSourceFilePath( const char *path );
	const char		*GetProgramName();
	bool			SetProgramName( const char *pgname );
    
    void            restoreGlobalProperties();
    void            storeGlobalProperties();
    void            getPreferenceFolder(char *outPath, int inSize);
    void            getDocumentsFolder(char *outPath, int inSize);
};
