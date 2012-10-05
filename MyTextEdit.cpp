/*
 *  MyTextEdit.cpp
 *  Chip700
 *
 *  Created by osoumen on 12/10/05.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "MyTextEdit.h"

//------------------------------------------------------------------------
CMyTextEdit::CMyTextEdit(const CRect& size, CControlListener* listener, long tag, const char* txt, CBitmap* background, const long style)
: CTextEdit(size, listener, tag, txt, background, style)
{
}

//------------------------------------------------------------------------
CMyTextEdit::~CMyTextEdit()
{
}
