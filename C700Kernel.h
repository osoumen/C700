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
	bool			SetPropertyValue( int inID, float value );
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
    void              SetVP(int pg, const InstParams *vp) { mVPset[pg] = *vp; }
    
    void            CorrectLoopFlagForSave(int pgnum);
    
    double          GetProcessDelayTime() { return mGenerator.GetProcessDelayTime(); }
	
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
    
    void            setRPNLSB(int ch, int value);
    void            setRPNMSB(int ch, int value);
    void            setNRPNLSB(int ch, int value);
    void            setNRPNMSB(int ch, int value);
    void            setDataEntryLSB(int ch, int value);
    void            setDataEntryMSB(int ch, int value);
    void            sendDataEntryValue(int ch);

};
