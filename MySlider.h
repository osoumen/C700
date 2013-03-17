/*
 *  MySlider.h
 *  C700
 *
 *  Created by osoumen on 12/10/02.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "vstgui.h"

class CMySlider : public CSlider
{
public:
	CMySlider(const CRect& size, CControlListener* listener, long tag, long iMinPos, long iMaxPos, CBitmap* handle, CBitmap* background, const CPoint& offset = CPoint (0, 0), const long style = kBottom|kVertical);
	CMySlider(const CRect& rect, CControlListener* listener, long tag, const CPoint& offsetHandle, long rangeHandle, CBitmap* handle, CBitmap* background, const CPoint& offset = CPoint (0, 0), const long style = kBottom|kVertical);
	virtual ~CMySlider();
	
	CLASS_METHODS(CMySlider, CSlider)
protected:
	virtual void draw(CDrawContext* pContext);
};
