/*
 *  TMultiButton.h
 *  Automat
 *
 *  Created by Stefan Kirch on 12.04.05.
 *  Copyright 2005 Stefan Kirch. All rights reserved.
 *
 */

// -----------------------------------------------------------------------------

#ifndef TMULTIBUTTON_H_
#define TMULTIBUTTON_H_

#include "TViewNoCompositingCompatible.h"

// -----------------------------------------------------------------------------

class TMultiButton:public TViewNoCompositingCompatible
{
	AUGUIDefineControl(TMultiButton, TViewNoCompositingCompatible, "multibutton");

protected:

			 TMultiButton(HIViewRef inControl);
    virtual ~TMultiButton();
	
    virtual ControlKind		GetKind();
	
	virtual bool			UseNonblockingTracking(){return true;}
	
    virtual OSStatus		Initialize(TCarbonEvent& inEvent);
	
    virtual void			ValueChanged();
    virtual void			TitleChanged();

    virtual void			DrawStaticBackground(RgnHandle inLimitRgn, CGContextRef inContext, bool inCompositing);
    virtual void			CompatibleDraw(RgnHandle inLimitRgn, CGContextRef inContext, bool inCompositing);
	
	virtual OSStatus		StartTracking(TCarbonEvent&inEvent, HIPoint& from);
	virtual OSStatus		StopTracking(TCarbonEvent&inEvent, HIPoint& from);
	
private:
	
	CGImageRef				mBackImage;
	CGImageRef				mThumbImage;
	CGImageRef				mTopImage;
	
	int						mXposition;
	int						mYposition;
	int						mValue;
	
	bool					mDown;
};

#endif

// -----------------------------------------------------------------------------
