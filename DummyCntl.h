/*
 *  DummyCntl.h
 *  C700
 *
 *  Created by osoumen on 12/10/04.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "vstgui.h"

class CDummyCntl : public CControl
{
public:
	CDummyCntl(const CRect& size, CControlListener* listener = 0, long tag = 0, CBitmap* pBackground = 0);
	virtual ~CDummyCntl() {}
	
	CLASS_METHODS(CDummyCntl, CControl)
protected:
	virtual void draw(CDrawContext* pContext);
};
