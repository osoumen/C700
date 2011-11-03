/*
 *  DragTextControl.h
 *  MadShifta
 *
 *  Created by Stewart Moroney on Fri Jun 04 2004.
 *  Copyright (c) 2004 Airy ANDRE. All rights reserved.
 *
 */

#ifndef TDragTextControl_H_
#define TDragTextControl_H_

#include "TViewNoCompositingCompatible.h"

class TDragTextControl
: public TViewNoCompositingCompatible
{
	AUGUIDefineControl(TDragTextControl, TViewNoCompositingCompatible, "dragtext");
protected:
	// Constructor/Destructor
	TDragTextControl(HIViewRef inControl);
    virtual ~TDragTextControl();

    virtual ControlKind		GetKind();
	
	virtual bool UseNonblockingTracking() { return true; }

    virtual OSStatus		Initialize( TCarbonEvent& inEvent );
    virtual OSStatus		StillTracking(TCarbonEvent&inEvent, HIPoint& from);
    virtual void		DrawStaticBackground(RgnHandle	inLimitRgn, CGContextRef inContext, bool  inCompositing );
	virtual void		TitleChanged();
    virtual void		CompatibleDraw(RgnHandle	inLimitRgn, CGContextRef inContext,bool  inCompositing );
    virtual void		ValueChanged();

private:

	char fontName[100], unitStr[100];
	float fontSize, fontRColour, fontGColour, fontBColour, horizBorder, vertBorder, valueMultipler;
};

#endif // _TDragTextControl_h_