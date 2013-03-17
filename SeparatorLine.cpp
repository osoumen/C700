/*
 *  SeparatorLine.cpp
 *  C700
 *
 *  Created by osoumen on 12/10/05.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "SeparatorLine.h"

const CColor kSeparatorColor		= {96, 96, 96, 255};

//-----------------------------------------------------------------------------
CSeparatorLine::CSeparatorLine(const CRect& size, CControlListener* listener, long tag, CBitmap* pBackground)
: CControl(size, listener, tag, pBackground)
{
}

//-----------------------------------------------------------------------------
void CSeparatorLine::draw(CDrawContext* pContext)
{
	//‹æØ‚èü‚ð•`‚­
	pContext->setFrameColor(kSeparatorColor);
	CPoint point(0,getHeight()/2);
	point.offset(size.left, size.top);
	pContext->moveTo(point);
	point.offset(getWidth(), 0);
	pContext->lineTo(point);
}
