/*
 *  LabelOnOffButton.cpp
 *  Chip700
 *
 *  Created by “¡“c ‹§•F on 12/10/03.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "LabelOnOffButton.h"

static CFontDesc g_LabelFont("Helvetica Bold", 9);

//-----------------------------------------------------------------------------
CLabelOnOffButton::CLabelOnOffButton(const CRect& size, CFrame *frame, CControlListener* listener, long tag, CBitmap* background, const char *txt, long style)
: CViewContainer(size, frame, 0)
{
	setTransparency(true);
	
	CRect	local_size(0, 0, background->getWidth(), background->getHeight());
	m_pOnOffButton = new COnOffButton(local_size, listener, tag, background, style);
	addView(m_pOnOffButton);
	
	local_size(background->getWidth(), 0, size.getWidth() - background->getWidth(), size.getHeight() );
	m_pText = new CTextLabel(local_size, txt, 0, 0);
	
	m_pText->setFont(&g_LabelFont);
	m_pText->setHoriAlign(kLeftText);
	m_pText->setTransparency(true);
	m_pText->setFontColor(kBlackCColor);
	addView(m_pText);
}

//-----------------------------------------------------------------------------
CLabelOnOffButton::~CLabelOnOffButton()
{
	removeAll();
}

//-----------------------------------------------------------------------------
CMouseEventResult CLabelOnOffButton::onMouseDown(CPoint& where, const long& buttons)
{
	CMouseEventResult	result = CViewContainer::onMouseDown(where, buttons);
	if ( result != kMouseDownEventHandledButDontNeedMovedOrUpEvents )
	{
		result = m_pOnOffButton->onMouseDown(where, buttons);
	}
	return result;
}
