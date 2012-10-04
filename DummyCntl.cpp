/*
 *  DummyCntl.cpp
 *  Chip700
 *
 *  Created by osoumen on 12/10/04.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "DummyCntl.h"

//-----------------------------------------------------------------------------
CDummyCntl::CDummyCntl(const CRect& size, CControlListener* listener, long tag, CBitmap* pBackground)
: CControl(size, listener, tag, pBackground)
{
}

//-----------------------------------------------------------------------------
void CDummyCntl::draw(CDrawContext* pContext)
{
	pContext->setFrameColor(kBlackCColor);
	pContext->drawRect(size, kDrawStroked);
	pContext->drawString("Dummy", size, true);
}
