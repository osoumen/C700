/*
 *  TSwitch.cpp
 *  Automat
 *
 *  Created by Stefan Kirch on 27.01.05.
 *  Copyright 2005 Stefan Kirch. All rights reserved.
 *
 */

// -----------------------------------------------------------------------------

#include "TSwitch.h"
#include "TImageCache.h"
#include "AUGUIUtilities.h"

using namespace AUGUI;

AUGUIProperties(TSwitch) =
{
	AUGUI::property_t()
};

AUGUIInit(TSwitch);

const ControlKind kSwitchControlKind = {'airy', 'swit'};

// -----------------------------------------------------------------------------

TSwitch::TSwitch(HIViewRef inControl):TViewNoCompositingCompatible(inControl)
{
	mBackImage		= 0;
	mThumbImage		= 0;
	mTopImage		= 0;
	mOrientation	= kHorizontalOrientation;
}

// -----------------------------------------------------------------------------

TSwitch::~TSwitch()
{	
    if (mBackImage)
		CGImageRelease(mBackImage);
		
	if (mThumbImage)
		CGImageRelease(mThumbImage);
		
    if (mTopImage)
		CGImageRelease(mTopImage);
}

// -----------------------------------------------------------------------------

ControlKind TSwitch::GetKind()
{
    return kSwitchControlKind;
}

// -----------------------------------------------------------------------------

OSStatus TSwitch::Initialize(TCarbonEvent& inEvent)
{
	#pragma unused(inEvent)
	
    TitleChanged();
    return noErr;
}

// -----------------------------------------------------------------------------

void TSwitch::ValueChanged()
{
    Invalidate();
}

// -----------------------------------------------------------------------------

void TSwitch::TitleChanged()
{
    if (!mBundleRef)
		return;

	CFStringRef infoString;
	CopyControlTitleAsCFString(GetViewRef(), &infoString);
	
	if (infoString != NULL)
	{
		CFArrayRef	info	= CFStringCreateArrayBySeparatingStrings(kCFAllocatorDefault, infoString, CFSTR(" "));
		CFIndex		count	= CFArrayGetCount(info);
		
		if (count >= 1)
		{
			if (mBackImage)
				CGImageRelease(mBackImage);
			
			if (mThumbImage)
				CGImageRelease(mThumbImage);
			
			if (mTopImage)
				CGImageRelease(mTopImage);
			
			CFStringRef fileName	= (CFStringRef)CFArrayGetValueAtIndex(info, 0);
			CFStringRef	tempName	= NULL;
			
			mBackImage = TImageCache::GetImage(mBundleRef, fileName, CFSTR("png"), NULL);
			
			if (mBackImage)
			{
				TRect bounds = RectForCGImage(mBackImage);
			    mOrientation = bounds.Height() > bounds.Width() ? kVerticalOrientation : kHorizontalOrientation;
				
				FlushStaticBackground();
			}

			tempName	= CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%@Thumb"), fileName);
			mThumbImage	= TImageCache::GetImage(mBundleRef, tempName, CFSTR("png"), NULL);

			tempName	= CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%@Top"), fileName);
			mTopImage	= TImageCache::GetImage(mBundleRef, tempName, CFSTR("png"), NULL);
			
			CFRelease(tempName);
		}

		CFRelease(info);
		CFRelease(infoString);
	}
}

// -----------------------------------------------------------------------------

void TSwitch::DrawStaticBackground(RgnHandle inLimitRgn, CGContextRef inContext, bool inCompositing)
{
	#pragma unused(inLimitRgn)
	
    TRect bounds = Bounds();
	
    if (mBackImage)
		HIViewDrawCGImage(inContext, &bounds, mBackImage);
}

// -----------------------------------------------------------------------------

void TSwitch::CompatibleDraw(RgnHandle inLimitRgn, CGContextRef inContext, bool inCompositing)
{
	#pragma unused(inLimitRgn)
	
	TRect bounds = Bounds();

    if (mThumbImage)
    {
		CGContextSaveGState(inContext);
		
		TRect	thumbBounds	= RectForCGImage(mThumbImage);
		float	distance, position;
		
		if (mOrientation == kHorizontalOrientation)
			distance = bounds.Width() - thumbBounds.Width();
		else
			distance = bounds.Height() - thumbBounds.Height();

		position = (float(GetValue() - GetMinimum()) / float(GetMaximum() - GetMinimum()) - 0.5f) * distance;

		float xOffset, yOffset;
		
		if (mOrientation == kHorizontalOrientation)
		{
			xOffset = bounds.MinX() + (thumbBounds.Width() + distance) / 2.0f;
			yOffset = bounds.MinY() + thumbBounds.Height() / 2.0f;
		}
		else
		{
			xOffset = bounds.MinX() + thumbBounds.Width() / 2.0f;
			yOffset = bounds.MinY() + (thumbBounds.Height() + distance) / 2.0f;
		}
		
		CGContextTranslateCTM(inContext, xOffset, yOffset);

		if (mOrientation == kHorizontalOrientation)
			CGContextTranslateCTM(inContext, position, 0.0f);
		else
			CGContextTranslateCTM(inContext, 0.0f, -position);

		thumbBounds.MoveTo(-thumbBounds.Width() / 2.0f, -thumbBounds.Height() / 2.0f);
		HIViewDrawCGImage(inContext, &thumbBounds, mThumbImage);
		
		CGContextRestoreGState(inContext);
    }

    if (mTopImage)
		HIViewDrawCGImage(inContext, &bounds, mTopImage);
}

// -----------------------------------------------------------------------------

OSStatus TSwitch::StartTracking(TCarbonEvent& inEvent, HIPoint& from)
{
    float	distance, position;
    TRect	bounds		= Bounds();
	TRect	thumbBounds	= RectForCGImage(mThumbImage);
	
    SInt32	mini		= GetMinimum();
    SInt32	maxi		= GetMaximum();
    SInt32	curVal		= GetValue();

    if (mOrientation == kHorizontalOrientation)
	{
		distance	= bounds.Width() - thumbBounds.Width();
		position	= (from.x - thumbBounds.Width() / 2.0f) / distance;
    }
	else
	{
		distance	= bounds.Height() - thumbBounds.Height();
		position	= 1.0f - (from.y - thumbBounds.Height() / 2.f) / distance;
    }

    SInt32 val = SInt32(rint(mini + position * (maxi - mini)));
	
    if (val > maxi)
		val = maxi;
	
    if (val < mini)
		val = mini;
	
    if (val != curVal)
		SetValue(val);
	
    return noErr;
}

// -----------------------------------------------------------------------------
