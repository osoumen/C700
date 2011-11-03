/*
 *  TKnobControl.h
 *  AUStk
 *
 *  Created by Airy André on Sun Oct 20 2002.
 *  Copyright (c) 2002 Airy André. All rights reserved.
 *
 */

#ifndef TKnobControl_H_
#define TKnobControl_H_

#include "TViewNoCompositingCompatible.h"

class TKnobControl
: public TViewNoCompositingCompatible
{
	AUGUIDefineControl(TKnobControl, TViewNoCompositingCompatible, "knob");
public:
	// Control properties management
	virtual void SetProperty(OSType propID, double value);
	virtual void SetProperty(OSType propID, int32_t value);
	virtual void SetProperty(OSType propID, AUGUI::color_t &value);
	virtual void SetProperty(OSType propID, CFStringRef value);
	virtual bool GetProperty(OSType propID, double &value);
	virtual bool GetProperty(OSType propID, int32_t &value);
	virtual bool GetProperty(OSType propID, AUGUI::color_t &value);
	virtual bool GetProperty(OSType propID, CFStringRef &value);

	static const OSType kKnobControlBackgroundImageTag = 'BkIm';
protected:
	// Constructor/Destructor
	TKnobControl(HIViewRef inControl);
    virtual ~TKnobControl();

    virtual ControlKind		GetKind();
	
	virtual bool UseNonblockingTracking() { return true; }

    virtual OSStatus		Initialize( TCarbonEvent& inEvent );
    virtual OSStatus		StillTracking(TCarbonEvent&inEvent, HIPoint& from);
    virtual void		DrawStaticBackground(RgnHandle	inLimitRgn, CGContextRef inContext, bool  inCompositing );
    virtual void		CompatibleDraw(RgnHandle	inLimitRgn, CGContextRef inContext,bool  inCompositing );
    virtual void		TitleChanged();
    virtual void		ValueChanged();

	virtual OSStatus		SetData(OSType inTag, ControlPartCode inPart, Size inSize, const void* inPtr);

protected:
    CGImageRef mImage;
    CGImageRef mBackImage;
    float mXOffset, mYOffset; // offset for knob on background
    float mMinAngle, mMaxAngle;
	bool mEndless;
	
	CFStringRef mImageName;
};

#endif // TKnobControl
