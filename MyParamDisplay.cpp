/*
 *  MyParamDisplay.cpp
 *  Chip700
 *
 *  Created by osoumen on 12/10/05.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "MyParamDisplay.h"

//------------------------------------------------------------------------
CMyParamDisplay::CMyParamDisplay(const CRect& size, long tag, float valueMultipler, char *unitStr, CBitmap* background, const long style)
: CParamDisplay(size, background, style)
, mValueMultipler(valueMultipler)
{
	setTag(tag);
	if ( unitStr && unitStr[0] != '0')
	{
		strncpy(mUnitStr, unitStr, UNITSTR_LEN-1);
	}
	else
	{
		mUnitStr[0] = 0;
	}
	setStringConvert(stringConvert, this);
}

//------------------------------------------------------------------------
CMyParamDisplay::~CMyParamDisplay()
{
}

//------------------------------------------------------------------------
void CMyParamDisplay::stringConvert(float value, char *string, void *userData)
{
	CMyParamDisplay	*This = static_cast<CMyParamDisplay*>(userData);
	int intValue = value * This->mValueMultipler;
	sprintf(string, "%d%s", intValue, This->mUnitStr);
}
