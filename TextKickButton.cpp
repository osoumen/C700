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
#if WIN32
	radius--;
#endif
    int diameter = radius * 2;
    CRect   corner(0,0,diameter,diameter);
	CRect	rect = _rect;
#if MAC
	rect.x2 -= 1;
	rect.y += 1;
    radius--;
#else
	rect.x2 -= 1;
	rect.y2 -= 1;
#endif
    CPoint  points[8];

	points[0](rect.x, rect.y + radius);
	points[1](rect.x, rect.y2 - radius);
	points[2](rect.x + radius, rect.y2);
	points[3](rect.x2 - radius, rect.y2);
	points[4](rect.x2, rect.y2 - radius);
	points[5](rect.x2, rect.y + radius);
	points[6](rect.x2 - radius, rect.y);
	points[7](rect.x + radius, rect.y);
    
#if MAC
	rect.x2 += 1;
	rect.y -= 1;
    radius++;
#endif
    
    pContext->setDrawMode(kAntialias);
    
#if MAC
    corner.moveTo(rect.x, rect.y);
    pContext->drawArc(corner, 0, 90, drawStyle);
    corner.moveTo(rect.x, rect.y2-diameter);
    pContext->drawArc(corner, 90, 180, drawStyle);
    corner.moveTo(rect.x2-diameter, rect.y2-diameter);
    pContext->drawArc(corner, 180, 270, drawStyle);
    corner.moveTo(rect.x2-diameter, rect.y);
    pContext->drawArc(corner, 270, 360, drawStyle);
#else
	corner.moveTo(rect.x, rect.y);
	pContext->drawArc(corner, 180, 270, drawStyle);
	corner.moveTo(rect.x, rect.y2 - diameter);
	pContext->drawArc(corner, 90, 180, drawStyle);
	corner.moveTo(rect.x2 - diameter, rect.y2 - diameter);
	pContext->drawArc(corner, 0, 90, drawStyle);
	corner.moveTo(rect.x2 - diameter, rect.y);
	pContext->drawArc(corner, 270, 360, drawStyle);
#endif

	pContext->moveTo(points[0]);
	pContext->lineTo(points[1]);
	pContext->moveTo(points[2]);
	pContext->lineTo(points[3]);
	pContext->moveTo(points[4]);
	pContext->lineTo(points[5]);
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

#if WIN32
	newClip.offset(0, 2);
#endif
	
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

