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

class C700Kernel : public C700Parameters
{
public:
	C700Kernel();
	virtual ~C700Kernel();
	
	void			Reset();
	
	void			SetSampleRate( double samplerate ) { mGenerator.SetSampleRate(samplerate); }
	void			SetTempo( double tempo );
	virtual float	GetParameter( int id );
	virtual bool	SetParameter( int id, float value );

	float			GetPropertyValue( int inID );
    void            *GetPropertyPtrValue( int inID );
    bool            GetPropertyStructValue( int inID, void *outData );
	bool			SetPropertyValue( int inID, float value );
	bool			SetPropertyPtrValue( int inID, const void *inPtr );
	const char		*GetSourceFilePath();
	bool			SetSourceFilePath( const char *path );
	const char		*GetProgramName();
	bool			SetProgramName( const char *pgname );
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
	
	C700Driver	*GetGenerator() { return &mGenerator; }
	
	void			SetPropertyNotifyFunc( void (*func) (int propID, void* userData), void* userData );
	void			SetParameterSetFunc( void (*func) (int paramID, float value, void* userData) , void* userData );
	
	const InstParams* GetVP() { return mVPset; }
    void              SetVP(int pg, const InstParams *vp) {
		BRRData temp = *mVPset[pg].getBRRData();
		mVPset[pg] = *vp;
		mVPset[pg].setBRRData(&temp);
		mGenerator.RefreshKeyMap();
	}
    
    void            CorrectLoopFlagForSave(int pgnum);
    
    double          GetProcessDelayTime() { return mGenerator.GetProcessDelayTime(); }
	
#if AU
    static CFStringRef kSaveKey_ProgName;
    static CFStringRef kSaveKey_EditProg;
    static CFStringRef kSaveKey_EditChan;
    static CFStringRef kSaveKey_brrdata;
    static CFStringRef kSaveKey_looppoint;
    static CFStringRef kSaveKey_samplerate;
    static CFStringRef kSaveKey_basekey;
    static CFStringRef kSaveKey_lowkey;
    static CFStringRef kSaveKey_highkey;
    static CFStringRef kSaveKey_ar;
    static CFStringRef kSaveKey_dr;
    static CFStringRef kSaveKey_sl;
    static CFStringRef kSaveKey_sr;
    static CFStringRef kSaveKey_volL;
    static CFStringRef kSaveKey_volR;
    static CFStringRef kSaveKey_echo;
    static CFStringRef kSaveKey_bank;
    static CFStringRef kSaveKey_IsEmphasized;
    static CFStringRef kSaveKey_SourceFile;
    static CFStringRef kSaveKey_SustainMode;
    static CFStringRef kSaveKey_MonoMode;
    static CFStringRef kSaveKey_PortamentoOn;
    static CFStringRef kSaveKey_PortamentoRate;
    static CFStringRef kSaveKey_NoteOnPriority;
    static CFStringRef kSaveKey_ReleasePriority;

    static void     AddNumToDictionary(CFMutableDictionaryRef dict, CFStringRef key, int value);
    static void     AddBooleanToDictionary(CFMutableDictionaryRef dict, CFStringRef key, bool value);
    int             CreatePGDataDic(CFDictionaryRef *data, int pgnum);
    void            RestorePGDataDic(CFPropertyListRef data, int pgnum);
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
	
	InstParams			mVPset[128];
	// エコー
	float				mFilterBand[5];
    
    // RPN, NRPN
    int                 mDataEntryValue[16];
    int                 mIsSettingNRPN[16];
    int                 mRPN[16];
    int                 mNRPN[16];
	
	C700Driver      mGenerator;
    
    bool            mIsHwAvailable;
    
    void            setRPNLSB(int ch, int value);
    void            setRPNMSB(int ch, int value);
    void            setNRPNLSB(int ch, int value);
    void            setNRPNMSB(int ch, int value);
    void            setDataEntryLSB(int ch, int value);
    void            setDataEntryMSB(int ch, int value);
    void            sendDataEntryValue(int ch);

};
