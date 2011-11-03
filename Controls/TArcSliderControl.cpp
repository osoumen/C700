/*
 *  TArcSliderControl.cpp
 *  Ritmo
 *
 *  Created by Chris Reed on Mon Feb 03 2003.
 *  Copyright (c) 2003 Chris Reed. All rights reserved.
 *
 */

#include "TArcSliderControl.h"
#include "TImageCache.h"
#include "AUGUIUtilities.h"

AUGUIProperties(TArcSliderControl) = {
	AUGUI::property_t()
};
AUGUIInit(TArcSliderControl);

using namespace AUGUI;

TArcSliderControl::TArcSliderControl(HIViewRef inControl)
: TSliderControl(inControl)
{
	//    ChangeAutoInvalidateFlags(kAutoInvalidateOnActivate | kAutoInvalidateOnEnable, 0);
}

//	Clean up after yourself.
TArcSliderControl::~TArcSliderControl()
{
}

//	The title format is
//	 	filename xInset topInset bottomInset
//	With :
//		<filename>.png and <filename>Thumb.png are the pict used by the
//			control
//		xOffset and yOffset the offset of the sliding part from the back
//			(0,0 default values)
//		Only the filename is mandatory
void TArcSliderControl::TitleChanged()
{
	TSliderControl::TitleChanged();
	mBottomInset = mTopInset = -5;
}

//! The fun part of the control
//!
//!	XXX need to test with vertically oriented images
void TArcSliderControl::DrawStaticBackground(RgnHandle inLimitRgn, CGContextRef inContext, bool inCompositing)
{
	float delta = 5.f;
	if (mThumbImage) {
		float knobWidth = float(CGImageGetWidth(mThumbImage));
		delta = 2.f + knobWidth*.5f;
	}
    TRect bounds = Bounds();
	float centerX = bounds.MinX()+delta;
	float centerY = bounds.CenterY();
	float maxX = bounds.MaxX()-delta;
	float minY = bounds.MinY()+delta;
	float maxY = bounds.MaxY()-delta;
	float bx = centerX;
	float by = minY;
	float cx = maxX;
	float cy = centerY;
	float dx = centerX;
	float dy = maxY;
	float temp = cx*cx+cy*cy;
	float bc = (bx*bx + by*by - temp)*.5f;
	float cd = (temp - dx*dx - dy*dy)*.5f;
	float det = (bx-cx)*(cy-dy)-(cx-dx)*(by-cy);
	det = 1.f/det;
	float centerx = (bc*(cy-dy)-cd*(by-cy))*det;
	float centery = ((bx-cx)*cd-(cx-dx)*bc)*det;
	float radius = sqrt((centerx-bx)*(centerx-bx)+(centery-by)*(centery-by));
	
	CGContextBeginPath(inContext);    
	CGContextSetLineWidth(inContext, 4);
	float angle = atan2((maxY-centerY),(centerX - centerx));
	CGContextAddArc (inContext, centerx, centery, radius, -angle, angle, 0);
	
	CGContextSetRGBStrokeColor(inContext, .80, .80, .80, .9);    
	CGContextStrokePath(inContext);
	
	mCenterX = centerx;
	mCenterY = centery;
	mRadius = radius;
	mAngle = angle;
}

//! The fun part of the control
//!
//!	XXX need to test with vertically oriented images
void TArcSliderControl::CompatibleDraw(RgnHandle inLimitRgn, CGContextRef inContext, bool inCompositing)
{
#pragma unused(inLimitRgn)
	if (mThumbImage) {

    TRect bounds = Bounds();
	float   value		= (float)(GetValue() - GetMinimum()) / (float)(GetMaximum() - GetMinimum());
	float angle = mAngle - value*2.f*mAngle;
	
	CGContextSaveGState(inContext);
	
		float knobWidth = float(CGImageGetWidth(mThumbImage));
		float knobHeight = float(CGImageGetHeight(mThumbImage));

		TRect knobBounds(-knobWidth*.5f, -knobHeight*.5f, knobWidth, knobHeight);
		CGContextTranslateCTM(inContext, mCenterX + mRadius*cos(angle), mCenterY + mRadius*sin(angle));
		CGContextRotateCTM( inContext, angle);
		HIViewDrawCGImage( inContext, &knobBounds, mThumbImage);

		CGContextRestoreGState(inContext);
    }
}
