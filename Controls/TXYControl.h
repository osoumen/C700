/*
 *  TXYControl.h
 *  CAUGuiDemo
 *
 *  Created by Airy on Sat May 24 2003.
 *  Copyright (c) 2003 Airy ANDRE. All rights reserved.
 *
 */


#ifndef TXYControl_H_
#define TXYControl_H_

#include "TViewNoCompositingCompatible.h"

class TXYControl:public TViewNoCompositingCompatible
{
	AUGUIDefineControl(TXYControl, TViewNoCompositingCompatible, "xy");
	
public:

	static const OSType selectorY = 'posX';
	static const OSType selectorX = 'posY';
	
protected:

	// Constructor/Destructor
	
			TXYControl(HIViewRef inControl);
    virtual ~TXYControl();
	
    virtual ControlKind	GetKind();
	
	virtual bool		UseNonblockingTracking() {return true;}
	
    virtual OSStatus	Initialize(TCarbonEvent& inEvent);
	virtual OSStatus	StartTracking(TCarbonEvent&inEvent, HIPoint& from);
    virtual OSStatus	StillTracking(TCarbonEvent&inEvent, HIPoint& from);
    virtual void		DrawStaticBackground(RgnHandle	inLimitRgn, CGContextRef inContext, bool  inCompositing);
    virtual void		CompatibleDraw(RgnHandle inLimitRgn, CGContextRef inContext,bool inCompositing);
    virtual void		TitleChanged();
    virtual void		ValueChanged();
	virtual OSStatus	SetData(OSType inTag, ControlPartCode inPart, Size inSize, const void* inPtr);
	virtual OSStatus	GetData(OSType inTag, ControlPartCode inPart, Size inSize, Size* outSize, void* inPtr);
	
private:

	CGImageRef	mBackImage;
	CGImageRef	mMarker;
	double		mX, mY;
	
	float		mMarker_Size;
	float		mMarker_A, mMarker_R, mMarker_G, mMarker_B;
};

#endif // 