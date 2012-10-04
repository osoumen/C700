/*
 *  C700GUI.cpp
 *  Chip700
 *
 *  Created by “¡“c ‹§•F on 12/10/01.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "C700GUI.h"
#include "ControlInstances.h"

//-----------------------------------------------------------------------------
C700GUI::C700GUI(const CRect &inSize, CFrame *frame, CBitmap *pBackground)
: CViewContainer (inSize, frame, pBackground)
{
	//--CMyKnob--------------------------------------
	CBitmap *bgKnob = new CBitmap("knobBack.png");
	
 	CRect	size(0, 0, bgKnob->getWidth(), bgKnob->getHeight());
	size.offset(140 + 15, 15);
	CPoint	point(0, 0);
	cKnob = new CMyKnob(size, this, 801, bgKnob, 0, point);
	cKnob->setColorHandle( MakeCColor(67, 75, 88, 255) );
	cKnob->setColorShadowHandle( kTransparentCColor );
	cKnob->setInsetValue(1);
	addView(cKnob);
	bgKnob->forget();
	cKnob->setAttribute(kCViewTooltipAttribute,strlen("CMyKnob")+1,"CMyKnob");
	
	//--CMySlider--------------------------------------
	CBitmap *sliderHandleBitmap = new CBitmap("sliderThumb.png");
	
	size(0, 0, sliderHandleBitmap->getWidth(), 128);
	size.offset(280, 70);
#if 1
	point(0, 0);
	cVerticalSlider = new CMySlider(size, this, 505, size.top, size.top + 128 - sliderHandleBitmap->getHeight(), sliderHandleBitmap, 0, point, kBottom);
//	point(0, 0);
//	cVerticalSlider->setOffsetHandle(point);
#else
	point(0, 0);
	CPoint handleOffset(0, 0);
	cVerticalSlider = new CMySlider(size, this, 505, handleOffset, 
										   size.height() - 2 * handleOffset.v, sliderHandleBitmap, 0, point, kBottom);
#endif
//	cVerticalSlider->setFreeClick(true);
	addView(cVerticalSlider);
	cVerticalSlider->setAttribute(kCViewTooltipAttribute,strlen("CMySlider")+1,"CMySlider");
	
	sliderHandleBitmap->forget();
	
	//--CLabelOnOffButton--------------------------------------
	CBitmap *onOffButton = new CBitmap("bt_check.png");
	
	size(0, 0, 80, onOffButton->getHeight() / 2);
	size.offset(20, 20);
	cCheckBox = new CLabelOnOffButton(size, this, 808, onOffButton, "Echo");
	addView(cCheckBox);
	cCheckBox->setAttribute(kCViewTooltipAttribute,strlen("CLabelOnOffButton")+1,"CLabelOnOffButton");
	
	onOffButton->forget();
	
	//--CRockerSwitch--------------------------------------
	CBitmap *rocker = new CBitmap("rocker_sw.png");
 	size(0, 0, rocker->getWidth(), rocker->getHeight() / 3);
	size.offset(9, 70 + 29);
	point(0, 0);
	cRockerSwitch = new CRockerSwitch(size, this, 707, rocker->getHeight() / 3, rocker, point, kVertical);
	addView(cRockerSwitch);
	rocker->forget();
	cRockerSwitch->setAttribute(kCViewTooltipAttribute,strlen("CRockerSwitch")+1,"CRockerSwitch");
	
	//--CWaveView--------------------------------------
 	size(0, 0, 200, 100);
	size.offset(64, 128);
	cWaveView = new CWaveView(size, frame);
	{
		float	testWave[128];
		for ( int i=0; i<128; i++ )
		{
			testWave[i] = sin( 3.14*2*4 * i / 128 );
			cWaveView->setWave(testWave, 128);
		}
	}
	addView(cWaveView);
	cWaveView->setAttribute(kCViewTooltipAttribute,strlen("CWaveView")+1,"CWaveView");
	
	//--CDummyCntl--------------------------------------
 	size(0, 0, 200, 100);
	size.offset(256, 256);
	cDummyTest = new CDummyCntl(size);
	addView(cDummyTest);
	cDummyTest->setAttribute(kCViewTooltipAttribute,strlen("CDummyCntl")+1,"CDummyCntl");
}

//-----------------------------------------------------------------------------
C700GUI::~C700GUI()
{
	removeAll();
}

//-----------------------------------------------------------------------------
void C700GUI::valueChanged(CControl* control)
{
	int	tag = control->getTag();
	float value = control->getValue();
}

//-----------------------------------------------------------------------------
bool C700GUI::attached(CView* view)
{
	return CViewContainer::attached(view);
}

//-----------------------------------------------------------------------------
bool C700GUI::removed(CView* parent)
{
	return CViewContainer::removed(parent);
}

//-----------------------------------------------------------------------------
CMessageResult C700GUI::notify(CBaseObject* sender, const char* message)
{
	return CViewContainer::notify(sender, message);
}
