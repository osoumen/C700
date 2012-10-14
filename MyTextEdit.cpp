/*
 *  MyTextEdit.cpp
 *  Chip700
 *
 *  Created by osoumen on 12/10/05.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "MyTextEdit.h"
#include <stdio.h>

//------------------------------------------------------------------------
CMyTextEdit::CMyTextEdit(const CRect& size, CControlListener* listener, long tag, const char* txt, bool isFreeText, bool showDecimal, CBitmap* background, const long style)
: CTextEdit(size, listener, tag, txt, background, style)
, mShowDecimal(showDecimal)
{
	if ( !isFreeText ) {
		setStringConvert(stringConvert, this);
	}
}

//------------------------------------------------------------------------
CMyTextEdit::~CMyTextEdit()
{
}
//------------------------------------------------------------------------
void CMyTextEdit::stringConvert(float value, char *string, void *userData)
{
	CMyTextEdit	*This = static_cast<CMyTextEdit*>(userData);
	if ( This->mShowDecimal ) {
		sprintf(string, "%.3f", value);
	}
	else {
		int	intValue = value;
		sprintf(string, "%d", intValue);
	}
}

