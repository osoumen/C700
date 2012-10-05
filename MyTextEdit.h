/*
 *  MyTextEdit.h
 *  Chip700
 *
 *  Created by osoumen on 12/10/05.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "vstgui.h"

class CMyTextEdit : public CTextEdit
{
public:
	CMyTextEdit(const CRect& size, CControlListener* listener, long tag, const char* txt = 0, CBitmap* background = 0, const long style = 0);
	virtual ~CMyTextEdit();
	
	CLASS_METHODS(CMyTextEdit, CTextEdit)
protected:
};
