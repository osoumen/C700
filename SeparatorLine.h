/*
 *  SeparatorLine.h
 *  C700
 *
 *  Created by osoumen on 12/10/05.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "vstgui.h"

class CSeparatorLine : public CControl
{
public:
	CSeparatorLine(const CRect& size, CControlListener* listener = 0, long tag = 0, CBitmap* pBackground = 0);
	virtual ~CSeparatorLine() {}
	
	CLASS_METHODS(CSeparatorLine, CControl)
protected:
	virtual void draw(CDrawContext* pContext);
};
