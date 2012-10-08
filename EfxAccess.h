/*
 *  EfxAccess.h
 *  Chip700
 *
 *  Created by “¡“c ‹§•F on 12/10/08.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#if AU
#include <AudioUnit/AUComponent.h>
#include <AudioToolbox/AudioToolbox.h>
#endif

#include "Chip700defines.h"

class EfxAccess
{
public:
	EfxAccess( void *efx );
	~EfxAccess();
#if AU
	void	SetEventListener( AUEventListenerRef listener ) { mEventListener = listener; }
#endif
	
	bool	GetBRRData( BRRData *data, int *size );
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
