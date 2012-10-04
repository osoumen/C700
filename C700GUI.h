/*
 *  C700GUI.h
 *  Chip700
 *
 *  Created by “¡“c ‹§•F on 12/10/01.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "vstgui.h"
#include "MyKnob.h"
#include "MySlider.h"
#include "LabelOnOffButton.h"
#include "WaveView.h"

class C700GUI : public CViewContainer, public CControlListener
{
public:
	C700GUI(const CRect &size, CFrame *pParent, CBitmap *pBackground = 0);
	~C700GUI();
	
	// CView‚æ‚è
	virtual void	valueChanged(CControl* control);
	bool			attached(CView* view);
	bool			removed(CView* parent);
	CMessageResult	notify(CBaseObject* sender, const char* message);
	
private:
	
	//Test
	CMyKnob				*cKnob;
	CMySlider			*cVerticalSlider;
	CLabelOnOffButton	*cCheckBox;
	CRockerSwitch		*cRockerSwitch;
	CWaveView			*cWaveView;
};
