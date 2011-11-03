/*
 *  TValueTextControl.cpp
 *  Ritmo
 *
 *  Created by Chris Reed on Mon Feb 03 2003.
 *  Copyright (c) 2003 Chris Reed. All rights reserved.
 *
 */

#include "TValueTextControl.h"
#include "TImageCache.h"
#include "AUGUIUtilities.h"

AUGUIProperties(TValueTextControl) = {
	AUGUI::property_t()
};
AUGUIInit(TValueTextControl);

using namespace AUGUI;

TValueTextControl::TValueTextControl(HIViewRef inControl)
: TViewNoCompositingCompatible(inControl), mThumbImage(0), mBackImage(0), mXInset(0), mTopInset(0), mBottomInset(0)
{
	//    ChangeAutoInvalidateFlags(kAutoInvalidateOnActivate | kAutoInvalidateOnEnable, 0);
}


//	Clean up after yourself.
TValueTextControl::~TValueTextControl()
{
    if (mThumbImage)
		CGImageRelease(mThumbImage);
    if (mBackImage)
		CGImageRelease(mBackImage);
}

ControlKind TValueTextControl::GetKind()
{
    const ControlKind kMyKind = { 'airy', 'valt' };

    return kMyKind;
}

// The control is set up.  Do the last minute stuff that needs to be done
// like installing EventLoopTimers.
OSStatus TValueTextControl::Initialize(TCarbonEvent& inEvent)
{
#pragma unused(inEvent)
    TitleChanged();
    return noErr;
}

void TValueTextControl::ValueChanged()
{
    Invalidate();
}

//	The title format is
//	 	filename xInset topInset bottomInset
//	With :
//		<filename>.png and <filename>Back.png are the pict used by the
//			control
//		xOffset and yOffset the offset of the sliding part from the back
//			(0,0 default values)
//		Only the filename is mandatory
void TValueTextControl::TitleChanged()
{
    if (mThumbImage)
		CGImageRelease(mThumbImage);
    if (mBackImage)
		CGImageRelease(mBackImage);
    if (mBundleRef)
    {
		CFStringRef infoString;
		CopyControlTitleAsCFString(GetViewRef(), &infoString);

		if (infoString != NULL)
		{
			CFArrayRef info = CFStringCreateArrayBySeparatingStrings(kCFAllocatorDefault, infoString, CFSTR(" "));
			CFIndex count = CFArrayGetCount(info);

			// filename
			if (count >= 1)
			{
				CFStringRef filename = (CFStringRef)CFArrayGetValueAtIndex(info, 0);

				CFURLRef url = CFBundleCopyResourceURL(mBundleRef, filename, CFSTR("png"), NULL);
				if (url)
				{
					mBackImage = TImageCache::GetImage(url);
					CFRelease(url);
				}

				filename = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%@Thumb"), filename);
				url = CFBundleCopyResourceURL(mBundleRef, filename, CFSTR("png"), NULL);
				if (url)
				{
					mThumbImage = TImageCache::GetImage(url);
					CFRelease(url);
					CFRelease(filename);
				}
			}

			// x inset
			if (count >= 2)
				mXInset = CFStringGetDoubleValue((CFStringRef)CFArrayGetValueAtIndex(info, 1));
			else
				mXInset = 0.0;

			// top inset
			if (count >= 3)
				mTopInset = CFStringGetDoubleValue((CFStringRef)CFArrayGetValueAtIndex(info, 2));
			else
				mTopInset = 0.0;

			// bottom inset
			if (count >= 3)
				mBottomInset = CFStringGetDoubleValue((CFStringRef)CFArrayGetValueAtIndex(info, 3));
			else
				mBottomInset = 0.0;

			// clean up
			CFRelease(info);
			CFRelease(infoString);
		}
    }
}

// The fun part of the control
//
//	XXX need to add scaling
//	XXX need to test with horizontally oriented images
void TValueTextControl::CompatibleDraw(RgnHandle inLimitRgn, CGContextRef inContext, bool inCompositing)
{
#pragma unused(inLimitRgn)
	TRect bounds = Bounds();
	CGContextRef context = inContext;
	TRect backBounds(RectForCGImage(mBackImage));

	// draw background
    if (mBackImage)
	{
		CGContextSaveGState(context);

		// rotate 90 degrees CW
		HIViewDrawCGImage(context, &backBounds, mBackImage);

		CGContextRestoreGState(context);
	}

	// draw slider thumb
    if (mThumbImage)
	{
		TRect thumbBounds(RectForCGImage(mThumbImage));
		HIViewDrawCGImage(context, &thumbBounds, mThumbImage);
    }
}

// -----------------------------------------------------------------------------
//	StillTracking
// -----------------------------------------------------------------------------
//	Called during tracking.
//
OSStatus TValueTextControl::StillTracking(TCarbonEvent&inEvent, HIPoint& from)
{
    HIPoint mouse;
    float deltaX, deltaY;
    SInt32 mini, maxi;
    UInt32 modifiers;

    mini = GetMinimum();
    maxi = GetMaximum();

    inEvent.GetParameter<HIPoint>( kEventParamWindowMouseLocation, typeHIPoint, &mouse );
    inEvent.GetParameter( kEventParamKeyModifiers, &modifiers );
    HIViewConvertPoint(&mouse, NULL, GetViewRef());

    deltaX = mouse.x-from.x;
    deltaY = mouse.y-from.y;
    SInt32 curVal = GetValue();
    SInt32 val;
#if SUPPORT_CONTROL_MODIFIERS
    if (modifiers & HIGHEST_PRECISION_MODIFIER) {
		val = SInt32(rint(curVal+(deltaX-deltaY)*(maxi-mini)/20000.));
    } else if (modifiers & HIGH_PRECISION_MODIFIER) {
		val = SInt32(rint(curVal+(deltaX-deltaY)*(maxi-mini)/1000.));
    } else {
#endif
		val = SInt32(rint(curVal+(deltaX-deltaY)*(maxi-mini)/50.));
#if SUPPORT_CONTROL_MODIFIERS
    }
#endif
    if ( val > maxi) val = maxi;
    if ( val < mini) val = mini;
    if (val != curVal) {
		SetValue ( val );
		from = mouse;
    }
    return noErr;
}
