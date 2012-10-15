/*
 *  LabelOnOffButton.h
 *  文字ラベル付きオンオフボタン
 *
 *  Created by osoumen on 12/10/03.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "vstgui.h"

class CLabelOnOffButton : public COnOffButton
{
public:
	CLabelOnOffButton(const CRect& size, CControlListener* listener, long tag, CBitmap* background, const char *txt=0, long style = 0);
	virtual ~CLabelOnOffButton();
	
	virtual void setText(const char* txt);
	virtual const char* getText() const;
	
	virtual	void draw(CDrawContext* pContext);
	
	CLASS_METHODS(CLabelOnOffButton, COnOffButton)
protected:
	void freeText();
	char* text;
	CFontRef	mLabelFont;
};
