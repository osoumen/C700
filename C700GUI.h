/*
 *  C700GUI.h
 *  Chip700
 *
 *  Created by osoumen on 12/10/01.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "vstgui.h"
#include "Chip700defines.h"
#include "EfxAccess.h"

#include "DummyCntl.h"
#include "MyKnob.h"
#include "MySlider.h"
#include "LabelOnOffButton.h"
#include "WaveView.h"
#include "SeparatorLine.h"
#include "MyParamDisplay.h"
#include "MyTextEdit.h"

#define GUITEST 0

class C700GUI : public CViewContainer, public CControlListener
{
public:
	C700GUI(const CRect &size, CFrame *pParent, CBitmap *pBackground = 0);
	~C700GUI();
	
	CControl*	FindControlByTag( long tag );
	void		SetEfxAccess(EfxAccess* efxacc) { efxAcc = efxacc; }
	
	// CViewÇÊÇË
	virtual void	valueChanged(CControl* control);
	bool			attached(CView* view);
	bool			removed(CView* parent);
	CMessageResult	notify(CBaseObject* sender, const char* message);
	
private:
	CControl		*makeControlFrom( const ControlInstances *desc, CFrame *frame );
	void			copyFIRParamToClipBoard();
	void			loadToCurrentProgram();
	void			saveFromCurrentProgram();
	void			saveFromCurrentProgramToXI();
	void			autocalcCurrentProgramSampleRate();
	void			autocalcCurrentProgramBaseKey();
	
	int						mNumCntls;
	CControl				**mCntl;
	
	EfxAccess				*efxAcc;
	
	//èâä˙âªéûÇ…ÇÃÇ›égóp
	CBitmap					*sliderHandleBitmap;
	CBitmap					*onOffButton;
	CBitmap					*bgKnob;
	CBitmap					*rocker;
	
	//Testóp
#if GUITEST
	CMyKnob				*cKnob;
	CMySlider			*cVerticalSlider;
	CLabelOnOffButton	*cCheckBox;
	CRockerSwitch		*cRockerSwitch;
	CWaveView			*cWaveView;
	CDummyCntl			*cDummyTest;
#endif
};
