/*
 *  EfxAccess.h
 *  Chip700
 *
 *  Created by “¡“c ‹§•F on 12/10/08.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "Chip700defines.h"

#if AU
#include <AudioUnit/AUComponent.h>
#include <AudioToolbox/AudioToolbox.h>
#endif

#include "SPCFile.h"
#include "PlistBRRFile.h"
#include "BRRFile.h"
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
	
	bool	GetBRRFileData( BRRFile **outData );
	bool	SetBRRFileData( const BRRFile *data );
	bool	GetXIFileData( XIFile **outData );
	bool	GetPlistBRRFileData( PlistBRRFile **outData );
	bool	SetPlistBRRFileData( const PlistBRRFile *data );
	
	bool	SetSourceFilePath( const char *path );
	bool	SetProgramName( const char *pgname );
	bool	GetBRRData( BRRData *data, int *size );
	bool	SetBRRData( const BRRData *data );
	float	GetPropertyValue( int propertyId );
	float	GetParameter( int parameterId );
	
	void	SetParam( void *sender, int index, float value );
	void	SetProperty( int propertyID, float value );
	
private:
#if AU
	AudioUnit			mAU;
	AUEventListenerRef	mEventListener;
#else
	AudioEffect*		mEfx;
#endif
};
