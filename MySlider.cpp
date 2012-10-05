/*
 *  MySlider.cpp
 *  Chip700
 *
 *  Created by osoumen on 12/10/02.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "MySlider.h"

//-----------------------------------------------------------------------------
CMySlider::CMySlider (const CRect &rect, CControlListener* listener, long tag, long iMinPos, long iMaxPos, CBitmap* handle, CBitmap* background, const CPoint& offset, const long style)
: CSlider(rect, listener, tag, iMinPos, iMaxPos, handle, background, offset, style)
{
}

//-----------------------------------------------------------------------------
CMySlider::CMySlider (const CRect &rect, CControlListener* listener, long tag, const CPoint& offsetHandle, long rangeHandle, CBitmap* handle, CBitmap* background, const CPoint& offset, const long style)
: CSlider(rect, listener, tag, offsetHandle, rangeHandle, handle, background, offset, style)
{
}

//-----------------------------------------------------------------------------
CMySlider::~CMySlider()
{
}

//-----------------------------------------------------------------------------
void CMySlider::draw(CDrawContext* pContext)
{
	CDrawMode	oldDrawMode		= pContext->getDrawMode();
	CCoord		oldLineWidth	= pContext->getLineWidth();
	CColor		oldFrameColor	= pContext->getFrameColor();
	
	pContext->setDrawMode(kAntialias);
	pContext->setLineWidth(2);
	pContext->setFrameColor( MakeCColor(67, 75, 88, 255) );
	
	//’†Sü‚ð•`‚­
	if ( style & kVertical ) {
		CPoint point( getWidth()/2, getHandle()->getHeight() / 2 );
		point.offset(size.left, size.top);
		pContext->moveTo(point);
		point( getWidth()/2, getHeight() - getHandle()->getHeight() / 2 );
		point.offset(size.left, size.top);
		pContext->lineTo(point);
	}
	else if ( style & kHorizontal )
	{
		CPoint point( getHandle()->getWidth() / 2, getHeight()/2 );
		point.offset(size.left, size.top);
		pContext->moveTo(point);
		point( getWidth() - getHandle()->getWidth() / 2, getHeight()/2 );
		point.offset(size.left, size.top);
		pContext->lineTo(point);
	}
	
	pContext->setDrawMode(oldDrawMode);
	pContext->setLineWidth(oldLineWidth);
	pContext->setFrameColor(oldFrameColor);
	
	CSlider::draw(pContext);
}
