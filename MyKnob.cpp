/*
 *  CMyKnob.cpp
 *  C700
 *
 *  Created by osoumen on 12/10/02.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "MyKnob.h"

//-----------------------------------------------------------------------------
CMyKnob::CMyKnob(const CRect& size, CControlListener* listener, long tag, CBitmap* background, CBitmap* handle, const CPoint& offset)
: CKnob(size, listener, tag, background, handle, offset)
{
}

//-----------------------------------------------------------------------------
CMyKnob::~CMyKnob()
{
}

//-----------------------------------------------------------------------------
void CMyKnob::drawHandle(CDrawContext* pContext)
{
	CDrawMode	oldDrawMode		= pContext->getDrawMode();
	CCoord		oldLineWidth	= pContext->getLineWidth();
	
	pContext->setDrawMode(kAntialias);
	pContext->setLineWidth(2);
	
	CKnob::drawHandle( pContext );
	
	pContext->setDrawMode(oldDrawMode);
	pContext->setLineWidth(oldLineWidth);
}
//-----------------------------------------------------------------------------
