/*
 *  TMultiButton.cpp
 *  Automat
 *
 *  Created by Stefan Kirch on 12.04.05.
 *  Copyright 2005 Stefan Kirch. All rights reserved.
 *
 */

// -----------------------------------------------------------------------------

#include "TMultiButton.h"
#include "TImageCache.h"
#include "AUGUIUtilities.h"

using namespace AUGUI;

AUGUIProperties(TMultiButton) =
{
	AUGUI::property_t()
};

AUGUIInit(TMultiButton);

const ControlKind kMultiButtonControlKind = {'airy', 'mbut'};

// -----------------------------------------------------------------------------

TMultiButton::TMultiButton(HIViewRef inControl):TViewNoCompositingCompatible(inControl)
{
	mBackImage		= 0;
	mThumbImage		= 0;
	mTopImage		= 0;
	
	mXposition		= 0;
	mYposition		= 0;
	mValue			= 0;
	
	mDown			= false;
}

// -----------------------------------------------------------------------------

TMultiButton::~TMultiButton()
{	
    if (mBackImage)
		CGImageRelease(mBackImage);
		
	if (mThumbImage)
		CGImageRelease(mThumbImage);
		
    if (mTopImage)
		CGImageRelease(mTopImage);
}

// -----------------------------------------------------------------------------

ControlKind TMultiButton::GetKind()
{
    return kMultiButtonControlKind;
}

// -----------------------------------------------------------------------------

OSStatus TMultiButton::Initialize(TCarbonEvent& inEvent)
{
	#pragma unused(inEvent)
	
    TitleChanged();
    return noErr;
}

// -----------------------------------------------------------------------------

void TMultiButton::ValueChanged()
{
    Invalidate();
}

// -----------------------------------------------------------------------------

void TMultiButton::TitleChanged()
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
			
			FlushStaticBackground();

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

void TMultiButton::DrawStaticBackground(RgnHandle inLimitRgn, CGContextRef inContext, bool inCompositing)
{
	#pragma unused(inLimitRgn)
	
    TRect bounds = Bounds();
	
    if (mBackImage)
		HIViewDrawCGImage(inContext, &bounds, mBackImage);
}

// -----------------------------------------------------------------------------

void TMultiButton::CompatibleDraw(RgnHandle inLimitRgn, CGContextRef inContext, bool inCompositing)
{
	#pragma unused(inLimitRgn)
	
	TRect bounds = Bounds();

    if (mThumbImage && mDown)
    {
		CGContextSaveGState(inContext);
		
		TRect thumbBounds = RectForCGImage(mThumbImage);
		
		CGContextTranslateCTM(inContext, bounds.MinX(), bounds.MinY());
		
		thumbBounds.MoveTo(mXposition * thumbBounds.Width(), mYposition * thumbBounds.Height());
		HIViewDrawCGImage(inContext, &thumbBounds, mThumbImage);
		
		CGContextRestoreGState(inContext);
    }

    if (mTopImage)
		HIViewDrawCGImage(inContext, &bounds, mTopImage);
}

// -----------------------------------------------------------------------------

OSStatus TMultiButton::StartTracking(TCarbonEvent& inEvent, HIPoint& from)
{
    TRect	bounds		= Bounds();
	TRect	thumbBounds	= RectForCGImage(mThumbImage);
	
	int	xTile			= (int)(bounds.Width() / thumbBounds.Width());
	int	yTile			= (int)(bounds.Height() / thumbBounds.Height());

	mXposition			= (int)((from.x / bounds.Width()) * xTile);
	mYposition			= (int)((from.y / bounds.Height()) * yTile);
	
    SInt32	min			= GetMinimum();
    SInt32	max			= GetMaximum();
	
	int		pos			= mYposition * xTile + mXposition;
	
    if (pos > max)
		pos = max;
	
    if (pos < min)
		pos = min;

	mValue	= pos;
	mDown	= true;
	
	ForceRedraw();
		
    return noErr;
}

// -----------------------------------------------------------------------------

OSStatus TMultiButton::StopTracking(TCarbonEvent&inEvent, HIPoint& from)
{
	mDown = false;

	if (GetValue() != mValue)
		SetValue(mValue);
	else
	{
		ForceRedraw();
		ForceValueChange();
	}
	
    return noErr;
}

//-----------------------------------------------------------------------------------
