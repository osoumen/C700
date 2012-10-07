/*
 *  LabelOnOffButton.cpp
 *  文字ラベル付きオンオフボタン
 *
 *  Created by osoumen on 12/10/03.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "LabelOnOffButton.h"

extern CFontRef kLabelFont;

//-----------------------------------------------------------------------------
CLabelOnOffButton::CLabelOnOffButton(const CRect& size, CControlListener* listener, long tag, CBitmap* background, const char *txt, long style)
: COnOffButton(size, listener, tag, background, style)
, text(NULL)
{
	setText(txt);
}

//-----------------------------------------------------------------------------
CLabelOnOffButton::~CLabelOnOffButton()
{
	freeText();
}

//------------------------------------------------------------------------
void CLabelOnOffButton::freeText()
{
	if (text)
	{
		free(text);
	}
	text = 0;
}

//-----------------------------------------------------------------------------
void CLabelOnOffButton::setText(const char* txt)
{
	if (!text && !txt || (text && txt && strcmp(text, txt) == 0))
	{
		return;
	}
	freeText();
	if (txt)
	{
		text = (char*)malloc(strlen(txt)+1);
		strcpy(text, txt);
	}
	setDirty(true);
}

//-----------------------------------------------------------------------------
const char* CLabelOnOffButton::getText() const
{
	return text;
}

//-----------------------------------------------------------------------------
void CLabelOnOffButton::draw(CDrawContext* pContext)
{
	COnOffButton::draw(pContext);
	//文字を描画
	
//	CRect oldClip;
//	pContext->getClipRect(oldClip);
	CRect newClip(size);
	newClip.offset( getBackground()->getWidth(), 0 );
//	newClip.bound(oldClip);
//	pContext->setClipRect(newClip);
	pContext->setFont(kLabelFont);
	pContext->setFontColor(kBlackCColor);
	
#if VSTGUI_USES_UTF8
	pContext->drawStringUTF8(text, newClip, kLeftText, true);
#else
	pContext->drawString(text, newClip, true, kLeftText);
#endif
//	pContext->setClipRect(oldClip);
}
