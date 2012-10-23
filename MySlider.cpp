/*
 *  MySlider.cpp
 *  Chip700
 *
 *  Created by osoumen on 12/10/02.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "Chip700defines.h"
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
	
	//中心線を描く
	if ( style & kVertical ) {
		CPoint point( getWidth()/2, 0 );
		point.offset(size.left, size.top);
		pContext->moveTo(point);
		point( getWidth()/2, getHeight() );
		point.offset(size.left, size.top);
		pContext->lineTo(point);
	}
	else if ( style & kHorizontal )
	{
		CPoint point( 0, getHeight()/2 );
		point.offset(size.left, size.top);
		pContext->moveTo(point);
		point( getWidth(), getHeight()/2 );
		point.offset(size.left, size.top);
		pContext->lineTo(point);
	}
	
	pContext->setDrawMode(oldDrawMode);
	pContext->setLineWidth(oldLineWidth);
	pContext->setFrameColor(oldFrameColor);
	
	//最大値最小値の変更への対応
	float	origVal = value;
	value = ConvertToVSTValue(value, vmin, vmax);
	CSlider::draw(pContext);
	value = origVal;
}
