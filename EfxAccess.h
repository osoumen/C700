/*
 *  EfxAccess.h
 *  C700
 *
 *  Created by osoumen on 12/10/08.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *  GUI内からエフェクタへアクセスするためのクラス
 */

#pragma once

#include "C700defines.h"

#if AU
#include <AudioUnit/AUComponent.h>
#include <AudioToolbox/AudioToolbox.h>
#else
#include "audioeffectx.h"
class C700VST;
#endif

#include "SPCFile.h"
#include "PlistBRRFile.h"
#include "RawBRRFile.h"
#include "AudioFile.h"
#include "XIFile.h"
#include "C700Properties.h"

class EfxAccess
{
public:
	EfxAccess( void *efx );
	~EfxAccess();
#if AU
	void	SetEventListener( AUEventListenerRef listener ) { mEventListener = listener; }
#endif
	
	bool	CreateBRRFileData( RawBRRFile **outData );
	bool	CreateXIFileData( XIFile **outData );
	bool	CreatePlistBRRFileData( PlistBRRFile **outData );
	bool	SetPlistBRRFileData( const PlistBRRFile *data );
	
	bool    SetFilePathProperty( int propertyId, const char *path );
	bool    GetFilePathProperty( int propertyId, char *path, int maxLen );
	bool	SetCStringProperty( int propertyId, const char *string );
	bool	GetCStringProperty( int propertyId, char *string, int maxLen );
	bool	GetBRRData( BRRData *data );
	bool	SetBRRData( const BRRData *data );
	
	double	GetPropertyValue( int propertyId );
	void	SetPropertyValue( int propertyID, double value );
	float	GetParameter( int parameterId );
	void	SetParameter( void *sender, int index, float value );
	
    bool    LoadSongPlayerCode( const char *path );
    
private:
#if AU
	AudioUnit			mAU;
	AUEventListenerRef	mEventListener;
    BRRData             mLastGetBrr;
#else
	C700VST*			mEfx;
#endif
    std::map<int, PropertyDescription>  mPropertyParams;
};
