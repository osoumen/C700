/*
 *  C700Parameters.h
 *  Chip700
 *
 *  Created by osoumen on 12/10/13.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "Chip700defines.h"

class C700Parameters 
{
public:
	C700Parameters(int numParams);
	~C700Parameters();

	virtual float		GetParameter( int id );
	virtual bool		SetParameter( int id, float value );
	static const char	*GetParameterName( int id );
	static float		GetParameterMax( int id );
	static float		GetParameterMin( int id );
	static float		GetParameterDefault( int id );
	
private:
	void				bounceValue( int id );
	
	int		mNumParams;
	float	*mParams;
};
