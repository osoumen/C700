/*
 *  MyParamDisplay.cpp
 *  C700
 *
 *  Created by osoumen on 12/10/05.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "MyParamDisplay.h"
#include <stdio.h>

//------------------------------------------------------------------------
CMyParamDisplay::CMyParamDisplay(const CRect& size, long tag, float valueMultipler, char *unitStr, CBitmap* background, const long style)
: CParamDisplay(size, background, style)
, mValueMultipler(valueMultipler)
{
	setTag(tag);
	if (valueMultipler == 0)
	{
		mValueMultipler = 1;
	}
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

//------------------------------------------------------------------------
CMouseEventResult CMyParamDisplay::onMouseDown (CPoint& where, const long& buttons)
{
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;
		
	mInitialPos = where;
	mInitialValue = value;
	
	beginEdit ();
	if (checkDefaultValue (buttons))
	{
		endEdit ();
		return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}
	if (buttons & kShift)
		return kMouseEventHandled;
	return onMouseMoved (where, buttons);
}

//------------------------------------------------------------------------
CMouseEventResult CMyParamDisplay::onMouseUp (CPoint& where, const long& buttons)
{
	endEdit ();
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CMyParamDisplay::onMouseMoved (CPoint& where, const long& buttons)
{
	if (buttons & kLButton)
	{		
		value = (int)(mInitialValue - (float)(where.v - mInitialPos.v)/3);
		
		bounceValue ();
		
		if (isDirty () && listener)
			listener->valueChanged (this);
		if (isDirty ())
			invalid ();
	}
	return kMouseEventHandled;
}
