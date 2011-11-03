/*
 *  TSwitch.h
 *  Automat
 *
 *  Created by Stefan Kirch on 27.01.05.
 *  Copyright 2005 Stefan Kirch. All rights reserved.
 *
 */

// -----------------------------------------------------------------------------

#ifndef TSWITCH_H_
#define TSWITCH_H_

#include "TViewNoCompositingCompatible.h"

// -----------------------------------------------------------------------------

enum
{
	kVerticalOrientation	= 0,
	kHorizontalOrientation	= 1
};

// -----------------------------------------------------------------------------

class TSwitch:public TViewNoCompositingCompatible
{
	AUGUIDefineControl(TSwitch, TViewNoCompositingCompatible, "switch");

protected:

			 TSwitch(HIViewRef inControl);
    virtual ~TSwitch();
	
    virtual ControlKind		GetKind();
	
	virtual bool			UseNonblockingTracking(){return true;}
	
    virtual OSStatus		Initialize(TCarbonEvent& inEvent);
	
    virtual void			ValueChanged();
    virtual void			TitleChanged();

    virtual void			DrawStaticBackground(RgnHandle inLimitRgn, CGContextRef inContext, bool inCompositing);
    virtual void			CompatibleDraw(RgnHandle inLimitRgn, CGContextRef inContext, bool inCompositing);
	
	virtual OSStatus		StartTracking(TCarbonEvent&inEvent, HIPoint& from);
			
private:
	
	CGImageRef				mBackImage;
	CGImageRef				mThumbImage;
	CGImageRef				mTopImage;
	
	int						mOrientation;
};

#endif

// -----------------------------------------------------------------------------
