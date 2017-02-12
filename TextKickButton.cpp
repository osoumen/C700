//
//  TextKickButton.cpp
//  C700
//
//  Created by osoumen on 2017/02/11.
//
//

#include "TextKickButton.h"

//-----------------------------------------------------------------------------
CTextKickButton::CTextKickButton(const CRect& size, CControlListener* listener, long tag, CBitmap* background, const char *txt, CFontRef font)
: CControl(size, listener, tag, background)
, text(NULL)
{
    mLabelFont = font;
    font->remember();
	setText(txt);
}

//-----------------------------------------------------------------------------
CTextKickButton::~CTextKickButton()
{
	freeText();
	mLabelFont->forget();
}

//------------------------------------------------------------------------
void CTextKickButton::freeText()
{
	if (text)
	{
		free(text);
	}
	text = 0;
}

//-----------------------------------------------------------------------------
void CTextKickButton::setText(const char* txt)
{
	if ((!text && !txt) || (text && txt && strcmp(text, txt) == 0))
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
const char* CTextKickButton::getText() const
{
	return text;
}

//-----------------------------------------------------------------------------
void CTextKickButton::draw(CDrawContext* pContext)
{
    // 枠線を描画
    CDrawMode	oldDrawMode		= pContext->getDrawMode();
	CCoord		oldLineWidth	= pContext->getLineWidth();
	pContext->setDrawMode(kAntialias);
	pContext->setLineWidth(1);
    pContext->setFrameColor(kBlackCColor);
    pContext->setFillColor(kBlackCColor);
    
    if (value > 0) {
        pContext->drawRect(size, kDrawFilled);
    }
    else {
        pContext->drawRect(size, kDrawStroked);
    }
	
	pContext->setDrawMode(oldDrawMode);
	pContext->setLineWidth(oldLineWidth);
    
	// 文字を描画
	CRect newClip(size);
	pContext->setFont(mLabelFont);
    if (value > 0) {
        pContext->setFontColor(kWhiteCColor);
    }
    else {
        pContext->setFontColor(kBlackCColor);
    }
	
#if VSTGUI_USES_UTF8
	pContext->drawStringUTF8(text, newClip, kCenterText, true);
#else
	pContext->drawString(text, newClip, true, kCenterText);
#endif
}

//------------------------------------------------------------------------
CMouseEventResult CTextKickButton::onMouseDown (CPoint& where, const long& buttons)
{
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;
	fEntryState = value;
	beginEdit ();
	return onMouseMoved (where, buttons);
}

//------------------------------------------------------------------------
CMouseEventResult CTextKickButton::onMouseUp (CPoint& where, const long& buttons)
{
	if (value && listener)
		listener->valueChanged (this);
	value = 0.0f;
	if (listener)
		listener->valueChanged (this);
	if (isDirty ())
		invalid ();
	endEdit ();
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CTextKickButton::onMouseMoved (CPoint& where, const long& buttons)
{
	if (buttons & kLButton)
	{
		if (where.h >= size.left && where.v >= size.top  &&
			where.h <= size.right && where.v <= size.bottom)
			value = !fEntryState;
		else
			value = fEntryState;
		
		if (isDirty ())
			invalid ();
	}
	return kMouseEventHandled;
}

