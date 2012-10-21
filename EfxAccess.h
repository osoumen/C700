/*
 *  EfxAccess.h
 *  Chip700
 *
 *  Created by osoumen on 12/10/08.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "Chip700defines.h"

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

class EfxAccess
{
public:
	EfxAccess( void *efx );
	~EfxAccess();
#if AU
	void	SetEventListener( AUEventListenerRef listener ) { mEventListener = listener; }
#endif
	
	bool	CreateBRRFileData( RawBRRFile **outData );
	bool	SetBRRFileData( const RawBRRFile *data );
	bool	CreateXIFileData( XIFile **outData );
	bool	CreatePlistBRRFileData( PlistBRRFile **outData );
	bool	SetPlistBRRFileData( const PlistBRRFile *data );
	
	bool	SetSourceFilePath( const char *path );
	bool	GetSourceFilePath( char *path, int maxLen );
	bool	SetProgramName( const char *pgname );
	bool	GetProgramName( char *pgname, int maxLen );
	bool	GetBRRData( BRRData *data );
	bool	SetBRRData( const BRRData *data );
	
	float	GetPropertyValue( int propertyId );
	void	SetPropertyValue( int propertyID, float value );
	float	GetParameter( int parameterId );
	void	SetParameter( void *sender, int index, float value );
	
private:
#if AU
	AudioUnit			mAU;
	AUEventListenerRef	mEventListener;
#else
	C700VST*			mEfx;
#endif
};
