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
void CTextKickButton::drawRoundRect(CDrawContext* pContext, const CRect &_rect, int radius, const CDrawStyle drawStyle)
{
    int diameter = radius * 2;
    CRect   corner(0,0,diameter,diameter);
    CPoint  points[8];
    points[0](_rect.x, _rect.y+radius);
    points[1](_rect.x, _rect.y2-radius);
    points[2](_rect.x+radius, _rect.y2);
    points[3](_rect.x2-radius, _rect.y2);
    points[4](_rect.x2-1, _rect.y2-radius);
    points[5](_rect.x2-1, _rect.y+radius);
    points[6](_rect.x2-radius, _rect.y+1);
    points[7](_rect.x+radius, _rect.y+1);
    
    pContext->setDrawMode(kAntialias);
    
    corner.moveTo(_rect.x, _rect.y);
    pContext->drawArc(corner, 0, 90, drawStyle);
    
    pContext->moveTo(points[0]);
    pContext->lineTo(points[1]);
    
    corner.moveTo(_rect.x, _rect.y2-diameter);
    pContext->drawArc(corner, 90, 180, drawStyle);
    
    pContext->moveTo(points[2]);
    pContext->lineTo(points[3]);

    corner.moveTo(_rect.x2-diameter, _rect.y2-diameter);
    pContext->drawArc(corner, 180, 270, drawStyle);
    
    pContext->moveTo(points[4]);
    pContext->lineTo(points[5]);

    corner.moveTo(_rect.x2-diameter, _rect.y);
    pContext->drawArc(corner, 270, 360, drawStyle);

    pContext->moveTo(points[6]);
    pContext->lineTo(points[7]);
    
    if (drawStyle == kDrawFilled || drawStyle == kDrawFilledAndStroked) {
        pContext->setDrawMode(kCopyMode);
        pContext->drawPolygon(points, 8, drawStyle, true);
    }
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
        drawRoundRect(pContext, size, 5, kDrawFilled);
    }
    else {
        drawRoundRect(pContext, size, 5, kDrawStroked);
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

