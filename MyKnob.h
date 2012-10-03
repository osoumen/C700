/*
 *  CMyKnob.h
 *  Chip700
 *
 *  Created by “¡“c ‹§•F on 12/10/02.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "vstgui.h"

class CMyKnob : public CKnob
{
public:
	CMyKnob(const CRect& size, CControlListener* listener, long tag, CBitmap* background, CBitmap* handle, const CPoint& offset = CPoint (0, 0));
	virtual ~CMyKnob();
	
	CLASS_METHODS(CMyKnob, CKnob)
protected:
	virtual void drawHandle(CDrawContext* pContext);
};
