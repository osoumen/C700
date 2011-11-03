/*
 *  TKnobArcControl.cpp
 *  Muso
 *
 *  Created by Airy ANDRE on 05/09/04.
 *  Copyright 2004 __MyCompanyName__. All rights reserved.
 *
 */

#include "TKnobArcControl.h"
#include "AUGUIUtilities.h"
#include "TImageCache.h"

using namespace AUGUI;

AUGUIProperties(TKnobArcControl) =
{
	AUGUI::property_t('filc', CFSTR("fill"), CFSTR("Fill Color"), AUGUI::kColor),
	AUGUI::property_t('frmc', CFSTR("frame color"), CFSTR("Frame Color"), AUGUI::kColor),
	AUGUI::property_t('drfr', CFSTR("draw frame"), CFSTR("Draw Frame"), AUGUI::kBool),
	AUGUI::property_t('flmd', CFSTR("fill mode"), CFSTR("Fill Mode"), AUGUI::kInteger),
	AUGUI::property_t('alph', CFSTR("alpha blend"), CFSTR("Alpha Blend"), AUGUI::kInteger),
	AUGUI::property_t('midA', CFSTR("mid angle"), CFSTR("Middle Angle"), AUGUI::kFloat),
	AUGUI::property_t('half', CFSTR("half circle"), CFSTR("Half Circle"), AUGUI::kBool),
	AUGUI::property_t()
};

AUGUIInit(TKnobArcControl);

// -----------------------------------------------------------------------------
//	TKnobArcControl constructor
// -----------------------------------------------------------------------------
//
TKnobArcControl::TKnobArcControl(HIViewRef inControl):TKnobControl(inControl), mMiddleAngle(-135.f)
{
	//  original settings from Airy AndrŽ
	
	mFill_R			= 0.5f;
	mFill_G			= 0.5f;
	mFill_B			= 0.5f;
	mFill_A			= 0.3f;
	
	mFrame_R		= 0.0f;
	mFrame_G		= 0.0f;
	mFrame_B		= 0.0f;
	mFrame_A		= 1.0f;
	
	mDrawFrame		= true;
	mFillMode		= kSmallWidthNoCenter;
	mAlphaBlend		= kNoAlphaBlend;
	
	mMiddleAngle	= mMinAngle;
	
	mHalfCircle		= false;

}

// -----------------------------------------------------------------------------
//	TKnobArcControl destructor
// -----------------------------------------------------------------------------
//	Clean up after yourself.
//
TKnobArcControl::~TKnobArcControl()
{
}

//-----------------------------------------------------------------------------------

//  Comma separated title format.
//  bgImage(string), fillColor(hex), frameColor(hex), drawFrame(bool), fillMode(int), alphaBlend(int)

void TKnobArcControl::TitleChanged()
{
    if (!mBundleRef)
		return;
	CFStringRef infoString;
	CopyControlTitleAsCFString(GetViewRef(), &infoString);
	
	if (infoString == NULL)
		return;
	
	CFArrayRef info	= CFStringCreateArrayBySeparatingStrings(kCFAllocatorDefault, infoString, CFSTR(","));

	if (info == NULL)
		return;
	
	CFIndex		count	= CFArrayGetCount(info);
	CFStringRef string  = NULL;
	
	//  background image
	
	if (count >= 1)
		TKnobControl::SetProperty('pict', (CFStringRef)CFArrayGetValueAtIndex(info, 0));
	
	//  fill color
	
	if (count >= 2)
	{
		string = (CFStringRef)CFArrayGetValueAtIndex(info, 1);
		char	buffer[9];
		int		color;
		
		CFStringGetCString(string, buffer, 9, CFStringGetSystemEncoding());
		sscanf(buffer, "%x", &color);
		
		AUGUI::color_t c;
		
		c.alpha	= (color >> 24  & 0xff) / 255.0f;
		c.red	= (color >> 16  & 0xff) / 255.0f;
		c.green	= (color >>  8	& 0xff) / 255.0f;
		c.blue	= (color		& 0xff) / 255.0f;
		
		if (c.alpha == 0.0f)
			c.alpha = 1.0f;

		SetProperty('filc', c);
	}
	
	//  frame color
	if (count >= 3)
	{
		string = (CFStringRef)CFArrayGetValueAtIndex(info, 2);
		char	buffer[9];
		int		color;
		
		CFStringGetCString(string, buffer, 9, CFStringGetSystemEncoding());
		sscanf(buffer, "%x", &color);
		
		AUGUI::color_t c;
		
		c.alpha	= (color >> 24  & 0xff) / 255.0f;
		c.red	= (color >> 16  & 0xff) / 255.0f;
		c.green	= (color >>  8	& 0xff) / 255.0f;
		c.blue	= (color		& 0xff) / 255.0f;
		
		if (c.alpha == 0.0f)
			c.alpha = 1.0f;
		
		SetProperty('frmc', c);
	}
	
	// draw frame
	if (count >= 4)
		SetProperty('drfr', (int32_t)CFStringGetIntValue((CFStringRef)CFArrayGetValueAtIndex(info, 3)) == 1);
	
	//  mode
	if (count >= 5)
		SetProperty('flmd', (int32_t)CFStringGetIntValue((CFStringRef)CFArrayGetValueAtIndex(info, 4)));
	
	//  alpha blend
	if (count >= 6)
		SetProperty('alph', (int32_t)CFStringGetIntValue((CFStringRef)CFArrayGetValueAtIndex(info, 5)));
	
	// min angle
	if (count >= 7)
		SetProperty('minA', (double)CFStringGetDoubleValue((CFStringRef)CFArrayGetValueAtIndex(info, 6)));
	
	// max angle
	if (count >= 8)
		SetProperty('maxA', (double)CFStringGetDoubleValue((CFStringRef)CFArrayGetValueAtIndex(info, 7)));
	
	// mid angle
	if (count >= 9)
		SetProperty('midA', (double)CFStringGetDoubleValue((CFStringRef)CFArrayGetValueAtIndex(info, 8)));
	else
		SetProperty('midA', (double)mMinAngle);
	
	//  clean up
		
	CFRelease(info);
	CFRelease(infoString);
}

//-----------------------------------------------------------------------------------
//	CompatibleDraw
//-----------------------------------------------------------------------------------
//	The fun part of the control
//
void TKnobArcControl::CompatibleDraw(RgnHandle inLimitRgn, CGContextRef inContext, bool inCompositing)
{
#pragma unused(inLimitRgn)
    TRect	bounds		= Bounds();
    TRect	knobBounds  = bounds;
	
	float   value		= (float)(GetValue() - GetMinimum()) / (float)(GetMaximum() - GetMinimum());
	
    float   angle		= value * (mMaxAngle - mMinAngle) + mMinAngle;
	float	midAngle	= (float)DegreesToRadians(mMiddleAngle - 90.0f);
	float   startAngle;
	float   endAngle;
	
	if (mHalfCircle) {
		startAngle = DegreesToRadians(mMiddleAngle - (mMiddleAngle-mMinAngle)*value - 90.f);
		endAngle = DegreesToRadians(mMiddleAngle + (mMaxAngle-mMiddleAngle)*value - 90.f);
	} else {	
		if (angle < mMiddleAngle)
		{
			startAngle	= (float)DegreesToRadians(angle - 90.0f);
			endAngle	= midAngle;
		} 
		else
		{
			startAngle	= midAngle;
			endAngle	= (float)DegreesToRadians(angle - 90.0f);
		}
	}
	
    float   knobWidth   = knobBounds.Width();
    float   knobHeight  = knobBounds.Height();
	
	float   unit		= knobHeight / 8.0f;
    
	CGContextSaveGState(inContext);
    CGContextTranslateCTM(inContext, mXOffset + 0.5f * (bounds.MinX() + bounds.MaxX()), mYOffset + 0.5f * (bounds.MinY() + bounds.MaxY()));
	
    /*
     * We have to do some scaling to deal with ovals
     */
	
    float   sx  = knobWidth / knobHeight;
    float   sy  = 1.0f;
	
    CGContextScaleCTM(inContext, sx, sy);
    
	//  Do the fill stuff
	
	float fillWidth, fillRadius, frameRadius, alphaBlend = 0.0f;
	
	switch(mAlphaBlend)
	{
		default:
		case kNoAlphaBlend:
			alphaBlend = mFill_A;
			break;
			
		case kAlphaBlend:
			alphaBlend = mFill_A + value * (1.0f - mFill_A);
			break;
			
		case kInverseAlphaBlend:
			alphaBlend = mFill_A + (1.0f - value) * (1.0f - mFill_A);
			break;
	}

	switch(mFillMode)
	{
		default:
		case kSmallWidthNoCenter:
			fillWidth   = unit * 2.0f + (mDrawFrame ? 0.0f : 1.0f);
			fillRadius  = unit * 2.0f;
			frameRadius	= unit * 3.0f;
			
			DrawStrokeArc(inContext, fillWidth, fillRadius, startAngle, endAngle, alphaBlend);
		break;
			
		case kSmallDynamicNoCenter:
			fillWidth   = unit * value + unit + (mDrawFrame ? 0.0f : 1.0f);
			fillRadius  = unit * 2.0f + ((unit * 0.5f) * (1.0f - value));
			frameRadius	= unit * 3.0f;
			
			DrawStrokeArc(inContext, fillWidth, fillRadius, startAngle, endAngle, alphaBlend);
			break;
			
		case kSmallInvDynamicNoCenter:
			fillWidth   = unit * (1.0f - value) + unit + (mDrawFrame ? 0.0f : 1.0f);
			fillRadius  = unit * 2.0f + ((unit * 0.5f) * value);
			frameRadius	= unit * 3.0f;
			
			DrawStrokeArc(inContext, fillWidth, fillRadius, startAngle, endAngle, alphaBlend);
			break;
			
		case kSmallWidth:
			fillRadius  = unit * 3.0f + (mDrawFrame ? 0.0f : 0.5f);
			frameRadius	= fillRadius;
			
			DrawFillArc(inContext, fillRadius, startAngle, endAngle, alphaBlend);
			break;
			
		case kSmallDynamic:
			fillRadius  = unit + (unit * 2.0f * value) + (mDrawFrame ? 0.0f : 0.5f);
			frameRadius	= fillRadius;
			
			DrawFillArc(inContext, fillRadius, startAngle, endAngle, alphaBlend);
			break;

		case kSmallRing:
			fillWidth   = 4.0f;
			fillRadius  = unit * 3.0f - 1.5f;
			frameRadius	= unit * 3.0f;
			
			DrawStrokeArc(inContext, fillWidth, fillRadius, startAngle, endAngle, alphaBlend);
			break;

		case kFullWidth:
			fillRadius  = unit * 4.0f - (mDrawFrame ? 0.5f : 0.0f);
			frameRadius	= fillRadius;
			
			DrawFillArc(inContext, fillRadius, startAngle, endAngle, alphaBlend);
			break;			

		case kFullWidthNoCenter:
			fillWidth   = unit * 2.0f;
			fillRadius  = unit * 3.0f - (mDrawFrame ? 0.5f : 0.0f);
			frameRadius	= unit * 4.0f - 0.5f;
			
			DrawStrokeArc(inContext, fillWidth, fillRadius, startAngle, endAngle, alphaBlend);
			break;
	}
	
	//  Do the frame stuff
	
	if (mDrawFrame)
	{
		CGContextBeginPath(inContext);    
		CGContextSetLineWidth(inContext, 1.0f);
		CGContextMoveToPoint(inContext, 0, 0);
		CGContextAddArc(inContext, 0, 0, frameRadius, startAngle, endAngle, 0);
		CGContextAddLineToPoint(inContext, 0, 0);
		CGContextSetRGBStrokeColor(inContext, mFrame_R, mFrame_G, mFrame_B, mFrame_A);
		CGContextStrokePath(inContext);
	}
	
	CGContextRestoreGState(inContext);	

    if (mImage)
		HIViewDrawCGImage(inContext, &bounds, mImage);
}

//-----------------------------------------------------------------------------------

void TKnobArcControl::DrawStrokeArc(CGContextRef context, float fillWidth, float fillRadius, float startAngle, float endAngle, float alphaBlend)
{
	CGContextBeginPath(context);    
	CGContextSetLineWidth(context, fillWidth);   
	CGContextAddArc(context, 0, 0, fillRadius, startAngle, endAngle, 0);
	CGContextSetRGBStrokeColor(context, mFill_R, mFill_G, mFill_B, alphaBlend);    
	CGContextStrokePath(context);
}

//-----------------------------------------------------------------------------------

void TKnobArcControl::DrawFillArc(CGContextRef context, float fillRadius, float startAngle, float endAngle, float alphaBlend)
{
	CGContextBeginPath(context);    
	CGContextMoveToPoint(context, 0, 0);
	CGContextAddArc(context, 0, 0, fillRadius, startAngle, endAngle, 0);
	CGContextAddLineToPoint(context, 0, 0);
	CGContextSetRGBFillColor(context, mFill_R, mFill_G, mFill_B, alphaBlend);
	CGContextFillPath(context);
}

//-----------------------------------------------------------------------------------

void TKnobArcControl::SetProperty(OSType propID, AUGUI::color_t& r)
{
	switch(propID) {
		case 'frmc':
			mFrame_A = r.alpha;
			mFrame_R = r.red;
			mFrame_G = r.green;
			mFrame_B = r.blue;
			break;
		case 'filc':
			mFill_A = r.alpha;
			mFill_R = r.red;
			mFill_G = r.green;
			mFill_B = r.blue;
			break;			
		default:
			TKnobControl::SetProperty(propID, r);
			break;
	};
}

//-----------------------------------------------------------------------------------

void TKnobArcControl::SetProperty(OSType propID, int32_t val)
{
	switch(propID) {
		case 'drfr':
			mDrawFrame = val;
			break;
		case 'flmd':
			mFillMode = val;
			break;
		case 'alph':
			mAlphaBlend = val;
			break;			
		case 'half':
			mHalfCircle = val;
			break;			
		default:
			TKnobControl::SetProperty(propID, val);
			break;
	};
}

//-----------------------------------------------------------------------------------

void TKnobArcControl::SetProperty(OSType propID, double val)
{
	switch(propID) {
		case 'midA':
			mMiddleAngle = val;
			break;			
		default:
			TKnobControl::SetProperty(propID, val);
			break;
	};
}

//-----------------------------------------------------------------------------------

bool TKnobArcControl::GetProperty(OSType propID, AUGUI::color_t& r)
{
	bool res = true;
	switch(propID) {
		case 'frmc':
			r.alpha	= mFrame_A;
			r.red	= mFrame_R;
			r.green = mFrame_G;
			r.blue	= mFrame_B;
			break;
		case 'filc':
			r.alpha = mFill_A;
			r.red	= mFill_R;
			r.green = mFill_G;
			r.blue	= mFill_B;
			break;			
		default:
			res = TKnobControl::GetProperty(propID, r);
			break;
	};
	return res;
}

//-----------------------------------------------------------------------------------

bool TKnobArcControl::GetProperty(OSType propID, double& val)
{
	bool res = true;
	switch(propID) {
		case 'midA':
			val = mMiddleAngle;
			break;
		default:
			res = TKnobControl::GetProperty(propID, val);
			break;
	};
	return res;
}

//-----------------------------------------------------------------------------------

bool TKnobArcControl::GetProperty(OSType propID, int32_t& val)
{
	bool res = true;
	switch(propID) {
		case 'drfr':
			val = mDrawFrame;
			break;
		case 'half':
			val = mHalfCircle;
			break;
		case 'flmd':
			val = mFillMode;
			break;
		case 'alph':
			val = mAlphaBlend;
			break;
		default:
			res = TKnobControl::GetProperty(propID, val);
			break;
	};
	return res;
}

//-----------------------------------------------------------------------------------
