/*
 *  C700GUI.cpp
 *  Chip700
 *
 *  Created by osoumen on 12/10/01.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "C700GUI.h"
#include "ControlInstances.h"

static CFontDesc g_LabelFont("Helvetica Bold", 9);
CFontRef kLabelFont = &g_LabelFont;

//-----------------------------------------------------------------------------
CControl *C700GUI::makeControlFrom( const ControlInstances *desc, CFrame *frame )
{
	CControl	*cntl = NULL;
	CRect size(0, 0, desc->w , desc->h);
	size.offset(desc->x, desc->y);
	
	switch (desc->kind_sig) {
		case 'VeMa':
			switch (desc->kind ) {
				case 'wave':
				{
					CWaveView	*waveview;
					waveview = new CWaveView(size, frame, this, desc->id);
					cntl = waveview;
					break;
				}
				default:
					goto makeDummy;
					break;
			}
			break;
			
		case 'airy':
			switch (desc->kind) {
				case 'slid':
				{
					CMySlider	*slider;
					if ( desc->w < desc->h )
					{
						slider = new CMySlider(size, this, desc->id, size.top, size.bottom - sliderHandleBitmap->getHeight(), sliderHandleBitmap, 0, CPoint(0, 0), kBottom|kVertical );
					}
					else
					{
						slider = new CMySlider(size, this, desc->id, size.left, size.right - sliderHandleBitmap->getWidth(), sliderHandleBitmap, 0, CPoint(0, 0), kLeft|kHorizontal );
					}
					cntl = slider;
					break;
				}
				case 'knob':
				{
					CMyKnob		*knob;
					knob = new CMyKnob(size, this, desc->id, bgKnob, 0);
					knob->setColorHandle( MakeCColor(67, 75, 88, 255) );
					knob->setColorShadowHandle( kTransparentCColor );
					knob->setInsetValue(1);
					cntl = knob;
					break;
				}
				case 'cbtn':
				{
					CLabelOnOffButton	*button;
					button = new CLabelOnOffButton(size, this, desc->id, onOffButton, desc->title);
					cntl = button;
					break;
				}
				default:
					goto makeDummy;
					break;
			}
			break;
			
		case 'appl':
			switch (desc->kind) {
				case 'stxt':
				{
					CTextLabel	*textLabel;
					textLabel = new CTextLabel(size, desc->title, 0, 0);
					textLabel->setFont(kLabelFont);
					textLabel->setFontColor(kBlackCColor);
					textLabel->setTransparency(true);
					cntl = textLabel;
					break;
				}
				case 'larr':
				{
					CRockerSwitch *rockerSwitch;
					rockerSwitch = new CRockerSwitch(size, this, desc->id, rocker->getHeight() / 3, rocker, CPoint(0, 0), kVertical);
					cntl = rockerSwitch;
					break;
				}
				default:
					goto makeDummy;
					break;
			}
			break;
			
		default:
			goto makeDummy;
			break;
	}
	goto setupCntl;
	
makeDummy:
	cntl = new CDummyCntl(size);
	
setupCntl:
	if ( cntl )
	{
		//cntl->setMin(desc->minimum);
		//cntl->setMax(desc->maximum);
		//cntl->setValue(desc->value);
		cntl->setAttribute(kCViewTooltipAttribute,strlen(desc->title)+1,desc->title);
	}
	return cntl;
}

//-----------------------------------------------------------------------------
C700GUI::C700GUI(const CRect &inSize, CFrame *frame, CBitmap *pBackground)
: CViewContainer (inSize, frame, pBackground)
{
	bgKnob = new CBitmap("knobBack.png");
	sliderHandleBitmap = new CBitmap("sliderThumb.png");
	onOffButton = new CBitmap("bt_check.png");
	rocker = new CBitmap("rocker_sw.png");
	
	int	numCntls = sizeof(sCntl) / sizeof(ControlInstances);
	for ( int i=0; i<numCntls; i++ )
	{
		CControl	*cntl;
		cntl = makeControlFrom( &sCntl[i], frame );
		if ( cntl )
		{
			addView(cntl);
		}
	}
	
	bgKnob->forget();
	sliderHandleBitmap->forget();
	onOffButton->forget();
	rocker->forget();
	
	//以下テストコード
#if 0
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
	cWaveView = new CWaveView(size, frame, this, 606);
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
#endif
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
