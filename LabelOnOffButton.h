/*
 *  LabelOnOffButton.h
 *  Chip700
 *
 *  Created by “¡“c ‹§•F on 12/10/03.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "vstgui.h"

class CLabelOnOffButton : public CViewContainer
{
public:
	CLabelOnOffButton(const CRect& size, CFrame *pParent, CControlListener* listener, long tag, CBitmap* background, const char *txt=0, long style = 0);
	virtual ~CLabelOnOffButton();
	
	virtual CMouseEventResult onMouseDown(CPoint& where, const long& buttons);
	
	CLASS_METHODS(CLabelOnOffButton, CViewContainer)
protected:
	COnOffButton	*m_pOnOffButton;
	CTextLabel		*m_pText;
};
