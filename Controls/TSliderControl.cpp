/*
 *  TSliderControl.cpp
 *  Ritmo
 *
 *  Created by Chris Reed on Mon Feb 03 2003.
 *  Copyright (c) 2003 Chris Reed. All rights reserved.
 *
 */

#include "TSliderControl.h"
#include "TImageCache.h"
#include "AUGUIUtilities.h"

AUGUIProperties(TSliderControl) = {
	AUGUI::property_t('pict', CFSTR("picture"), CFSTR("Picture"), AUGUI::kPicture),
	AUGUI::property_t('xIns', CFSTR("xinset"), CFSTR("X Inset"), AUGUI::kFloat),
	AUGUI::property_t('tIns', CFSTR("topinset"), CFSTR("Top Inset"), AUGUI::kFloat),
	AUGUI::property_t('bIns', CFSTR("bottominset"), CFSTR("Bottom Inset"), AUGUI::kFloat),
	AUGUI::property_t()
};
AUGUIInit(TSliderControl);


using namespace AUGUI;

const ControlKind kSliderControlKind = { 'airy', 'slid' };

TSliderControl::TSliderControl(HIViewRef inControl)
: TViewNoCompositingCompatible(inControl), mThumbImage(0), mBackImage(0), mImageName(0), mXInset(0), mTopInset(0), mBottomInset(0), mImageOrientation(TSliderControl::kVerticalOrientation), mUseDetent(false), mDetentValue(0), mDetentRange(100)
{
//    ChangeAutoInvalidateFlags(kAutoInvalidateOnActivate | kAutoInvalidateOnEnable, 0);
}

//	Clean up after yourself.
TSliderControl::~TSliderControl()
{
    if (mThumbImage)
		CGImageRelease(mThumbImage);
    if (mBackImage)
		CGImageRelease(mBackImage);
    if (mImageName)
		CFRelease(mImageName);
}

ControlKind TSliderControl::GetKind()
{
    return kSliderControlKind;
}

// The control is set up.  Do the last minute stuff that needs to be done
// like installing EventLoopTimers.
OSStatus TSliderControl::Initialize(TCarbonEvent& inEvent)
{
#pragma unused(inEvent)
    TitleChanged();
    return noErr;
}

void TSliderControl::ValueChanged()
{
    Invalidate();
}

//	The title format is
//	 	filename xInset topInset bottomInset
//	With :
//		<filename>.png and <filename>Thumb.png are the pict used by the
//			control
//		xOffset and yOffset the offset of the sliding part from the back
//			(0,0 default values)
//		Only the filename is mandatory
void TSliderControl::TitleChanged()
{
    if (mThumbImage)
		CGImageRelease(mThumbImage);
	mThumbImage = 0;
    if (mBackImage)
		CGImageRelease(mBackImage);
	mBackImage = 0;
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
				SetProperty('pict', filename);
			}

			// x inset
			SetProperty('xIns', 
						(count >= 2)?CFStringGetDoubleValue((CFStringRef)CFArrayGetValueAtIndex(info, 1)):0.);

			// top inset
			SetProperty('tIns', 
						(count >= 3)?CFStringGetDoubleValue((CFStringRef)CFArrayGetValueAtIndex(info, 2)):0.);

			// bottom inset
			SetProperty('bIns', 
						(count >= 4)?CFStringGetDoubleValue((CFStringRef)CFArrayGetValueAtIndex(info, 3)):0.);

			// clean up
			CFRelease(info);
			CFRelease(infoString);
		}
    }
}

OSStatus TSliderControl::SetData(OSType inTag, ControlPartCode inPart, Size inSize, const void* inPtr)
{
	OSStatus err = noErr;
	switch (inTag)
	{
		case kSliderControlDetentValueTag:
			if (inSize == sizeof(SInt32))
			{
				mDetentValue = *(SInt32 *)inPtr;
			}
			else
				err = errDataSizeMismatch;
			break;

		case kSliderControlDetentRangeTag:
			if (inSize == sizeof(SInt32))
			{
				mDetentRange = *(SInt32 *)inPtr;
			}
			else
				err = errDataSizeMismatch;
			break;

		case kSliderControlUseDetentTag:
			if (inSize == sizeof(UInt32))
			{
				mUseDetent = *(UInt32 *)inPtr;
			}
			else
				err = errDataSizeMismatch;
			break;

		default:
			err = errDataNotSupported;
			break;
	}
	return err;
}

OSStatus TSliderControl::GetData(OSType inTag, ControlPartCode inPart, Size inSize, Size* outSize, void* inPtr)
{
	OSStatus err = noErr;
	switch (inTag)
	{
		case kSliderControlDetentValueTag:
			if (inSize == sizeof(SInt32))
			{
				if (inPtr)
					*(UInt32 *)inPtr = mDetentValue;
				if (outSize)
					*outSize = sizeof(SInt32);
			}
			else
				err = errDataSizeMismatch;
			break;

		case kSliderControlDetentRangeTag:
			if (inSize == sizeof(SInt32))
			{
				if (inPtr)
					*(SInt32 *)inPtr = mDetentRange;
				if (outSize)
					*outSize = sizeof(SInt32);
			}
			else
				err = errDataSizeMismatch;
			break;

		case kSliderControlUseDetentTag:
			if (inSize == sizeof(UInt32))
			{
				if (inPtr)
					*(UInt32 *)inPtr = mUseDetent;
				if (outSize)
					*outSize = sizeof(UInt32);
			}
			else
				err = errDataSizeMismatch;
			break;

		default:
			err = TViewNoCompositingCompatible::GetData(inTag, inPart, inSize, outSize, inPtr);
			break;
	}
	return err;
}

float TSliderControl::ScaleX()
{
	float scale = 1.0f;

	if (mBackImage)
	{
		TRect bounds = Bounds();
		TRect backBounds = RectForCGImage(mBackImage);
		SliderOrientation boundsOrientation = OrientationForRect(bounds);

		if (mImageOrientation != boundsOrientation)
		{
			if (boundsOrientation == TSliderControl::kVerticalOrientation)
				scale = bounds.Width() / backBounds.Height();
			else
				scale = bounds.Height() / backBounds.Width();
		}
		else
		{
			if (boundsOrientation == TSliderControl::kVerticalOrientation)
				scale = bounds.Width() / backBounds.Width();
			else
				scale = bounds.Height() / backBounds.Height();
		}
	}
	return scale;
}

float TSliderControl::ScaleY()
{
	return ScaleX();
}

//! The fun part of the control
//!
//!	XXX need to test with vertically oriented images
void TSliderControl::DrawStaticBackground(RgnHandle inLimitRgn, CGContextRef inContext, bool inCompositing)
{
    TRect bounds = Bounds();
    CGContextRef context = inContext;
    SliderOrientation boundsOrientation = OrientationForRect(bounds);
    TRect backBounds = RectForCGImage(mBackImage);

    // draw background
    if (mBackImage)
    {
		CGContextSaveGState(context);

		CGContextTranslateCTM(context, (bounds.MinX() + bounds.MaxX()) / 2.0, (bounds.MinY() + bounds.MaxY()) / 2.0);
		if (mImageOrientation != boundsOrientation)
		{
			// rotate 90 degrees CW
			CGContextRotateCTM(context, DegreesToRadians(90.0));

			backBounds.SetWidth(bounds.Height());
			backBounds.SetHeight(bounds.Width());
		}
		else
		{
			backBounds.SetWidth(bounds.Width());
			backBounds.SetHeight(bounds.Height());
		}
		backBounds.MoveTo(-backBounds.Width() / 2.0, -backBounds.Height() / 2.0);

		HIViewDrawCGImage(context, &backBounds, mBackImage);

		CGContextRestoreGState(context);
    }
}

//! The fun part of the control
//!
//!	XXX need to test with vertically oriented images
void TSliderControl::CompatibleDraw(RgnHandle inLimitRgn, CGContextRef inContext, bool inCompositing)
{
#pragma unused(inLimitRgn)
    TRect bounds = Bounds();
    CGContextRef context = inContext;
    SliderOrientation boundsOrientation = OrientationForRect(bounds);
    TRect backBounds = RectForCGImage(mBackImage);

    float scaleX = ScaleX();
    float scaleY = ScaleY();

    // draw slider thumb
    if (mThumbImage)
    {
		TRect thumbBounds = RectForCGImage(mThumbImage);

		thumbBounds.SetWidth(thumbBounds.Width() * scaleX);
		thumbBounds.SetHeight(thumbBounds.Height() * scaleY);

		// compute a thumb bounds oriented the same as the slider
		TRect orientedThumbBounds = thumbBounds;
		if (boundsOrientation != mImageOrientation)
		{
			float temp = orientedThumbBounds.Width();
			orientedThumbBounds.SetWidth(orientedThumbBounds.Height());
			orientedThumbBounds.SetHeight(temp);
		}

		// the thumb image is assumed to be of the same orientation as the
		// background image
		float slideDistance;
		if (boundsOrientation == TSliderControl::kVerticalOrientation)
			slideDistance = bounds.Height() - orientedThumbBounds.Height();
		else
			slideDistance = bounds.Width() - orientedThumbBounds.Width();
		slideDistance -= (mTopInset + mBottomInset) * scaleY;

		// position is in -slideDistance/2,+slideDistance/2 from the middle of the slider
		float position = (float(GetValue() - GetMinimum()) / float(GetMaximum() - GetMinimum())-.5) * slideDistance;

		float xOffset, yOffset;
		if (boundsOrientation == TSliderControl::kVerticalOrientation)
		{
			xOffset = bounds.MinX() + mXInset + orientedThumbBounds.Width() / 2.0;
			yOffset = bounds.MinY() + mTopInset + (orientedThumbBounds.Height() + slideDistance) / 2.0;
		}
		else
		{
			xOffset = bounds.MinX() + mTopInset + (orientedThumbBounds.Width() + slideDistance) / 2.0;
			yOffset = bounds.MinY() + mXInset + orientedThumbBounds.Height() / 2.0;
		}
		CGContextTranslateCTM(context, xOffset, yOffset);

		if (boundsOrientation == TSliderControl::kHorizontalOrientation)
			CGContextTranslateCTM(context, position, 0.0);
		else
			CGContextTranslateCTM(context, 0.0, -position); // min is at the bottom

		if (mImageOrientation != boundsOrientation)
		{
			CGContextRotateCTM(context, DegreesToRadians(90.0));
		}

		thumbBounds.MoveTo(-thumbBounds.Width()/2.0, -thumbBounds.Height()/2.0);
		HIViewDrawCGImage(context, &thumbBounds, mThumbImage);
    }
}

// -----------------------------------------------------------------------------
//	StartTracking
// -----------------------------------------------------------------------------
//!	Called before tracking. The new control value is calculated so that the
//! slider thumb ends up centered on the mouse. This takes into account all
//! of the inset and size values, and also considers the background image
//! and thumb orientation compared to the slider orientation.
//
OSStatus TSliderControl::StartTracking(TCarbonEvent& inEvent, HIPoint& from)
{
    float pos;
    SInt32 mini, maxi;
    TRect bounds = Bounds();
	TRect thumbBounds = ScaledThumbBounds();
	SliderOrientation boundsOrientation = OrientationForRect(bounds);

    mini = GetMinimum();
    maxi = GetMaximum();

	mTotalDetentMoved = 0;

	// compute a thumb bounds oriented the same as the slider
	TRect orientedThumbBounds = thumbBounds;
	if (boundsOrientation != mImageOrientation)
	{
		float temp = orientedThumbBounds.Width();
		orientedThumbBounds.SetWidth(orientedThumbBounds.Height());
		orientedThumbBounds.SetHeight(temp);
	}

	float distance;
    if (boundsOrientation == TSliderControl::kHorizontalOrientation)
	{
		distance = bounds.Width() - mTopInset - mBottomInset - thumbBounds.Width();
		pos = (from.x - mTopInset - thumbBounds.Width() / 2.f) / distance;
    }
	else
	{
		distance = bounds.Height() - mTopInset - mBottomInset - thumbBounds.Height();
		pos = 1.f - (from.y - mTopInset - thumbBounds.Height() / 2.f) / distance;
    }

    SInt32 curVal = GetValue();
    SInt32 val = SInt32(rint(mini+pos*(maxi-mini)));
    if ( val > maxi) val = maxi;
    if ( val < mini) val = mini;
    if (val != curVal) {
		SetValue ( val );
    }
	
    return noErr;
}

// -----------------------------------------------------------------------------
//	StillTracking
// -----------------------------------------------------------------------------
//!	Called during mouse tracking. Works basically the same as the \c
//! StartTracking() member function, except that thus function also
//! calculates the amount of control value change based on how much the
//! mouse position has changed since the last call to \c StillTracking().
//! This delta value is used for the control and option key modifiers,
//! for higher precision slider control.
//!
OSStatus TSliderControl::StillTracking(TCarbonEvent&inEvent, HIPoint& from)
{
    HIPoint mouse,delta;
    float d, pos;
    SInt32 mini, maxi;
    UInt32 modifiers;
    TRect bounds = Bounds();
	TRect thumbBounds = ScaledThumbBounds();
	SliderOrientation boundsOrientation = OrientationForRect(bounds);

    mini = GetMinimum();
    maxi = GetMaximum();

    inEvent.GetParameter<HIPoint>( kEventParamWindowMouseLocation, typeHIPoint, &mouse );
	ConvertToLocal(mouse);

	delta.x = mouse.x - from.x;
	delta.y = mouse.y - from.y;
	
    inEvent.GetParameter( kEventParamKeyModifiers, &modifiers );

	// compute a thumb bounds oriented the same as the slider
	TRect orientedThumbBounds = thumbBounds;
	if (boundsOrientation != mImageOrientation)
	{
		float temp = orientedThumbBounds.Width();
		orientedThumbBounds.SetWidth(orientedThumbBounds.Height());
		orientedThumbBounds.SetHeight(temp);
	}

	float distance;
    if (boundsOrientation == TSliderControl::kHorizontalOrientation)
	{
		distance = bounds.Width() - mTopInset - mBottomInset - thumbBounds.Width();
		pos = (mouse.x - mTopInset - thumbBounds.Width() / 2.f) / distance;
		d = delta.x / distance;
    }
	else
	{
		distance = bounds.Height() - mTopInset - mBottomInset - thumbBounds.Height();
		pos = 1.f - (mouse.y - mTopInset - thumbBounds.Height() / 2.f) / distance;
		d = -delta.y / distance;
    }

    SInt32 curVal = GetValue();

	if (mUseDetent && (curVal == mDetentValue || abs(curVal - mDetentValue) <= mDetentRange))
	{
		if (boundsOrientation == TSliderControl::kHorizontalOrientation)
			mTotalDetentMoved += delta.x;
		else
			mTotalDetentMoved += delta.y;
		if (fabsf(mTotalDetentMoved) < 15.0)
		{
			if (curVal != mDetentValue)
				SetValue(mDetentValue);
			from = mouse;
			return noErr;
		}
	}
	mTotalDetentMoved = 0;

    SInt32 val;
#if SUPPORT_CONTROL_MODIFIERS
	if ((modifiers == HIGHEST_PRECISION_MODIFIER || modifiers == HIGH_PRECISION_MODIFIER))
	{
		// value depends on how much the mouse moved
		float ratio = 1.f;
		if (modifiers & HIGHEST_PRECISION_MODIFIER)
			ratio = 50.f;
		else if (modifiers & HIGH_PRECISION_MODIFIER)
			ratio = 10.f;
		val = SInt32(rint(curVal + d * (maxi - mini) / ratio));
	}
	else
	{
#endif
		// value is directly proportional to position
		val = SInt32(rint(mini + pos * (maxi - mini)));
#if SUPPORT_CONTROL_MODIFIERS
	}
#endif
#if 0
        long dd = val - mini;
        long unit = (maxi - mini)/10;
        val = mini + unit*(dd/unit + .5);
#endif
    if ( val > maxi) val = maxi;
    if ( val < mini) val = mini;
    if (val != curVal) {
		SetValue ( val );
		from = mouse;
    }
    return noErr;
}

TSliderControl::SliderOrientation TSliderControl::OrientationForRect(const TRect& theRect)
{
    if (theRect.Height() > theRect.Width())
		return TSliderControl::kVerticalOrientation;
    else
		return TSliderControl::kHorizontalOrientation;
}

TRect TSliderControl::ScaledThumbBounds()
{
	TRect thumbBounds = RectForCGImage(mThumbImage);
	thumbBounds.SetWidth(thumbBounds.Width() * ScaleX());
	thumbBounds.SetHeight(thumbBounds.Height() * ScaleY());
	return thumbBounds;
}

//-----------------------------------------------------------------------------------
void TSliderControl::SetProperty(OSType propID, CFStringRef s)
{
	switch(propID) {
		case 'pict':			
			if (mThumbImage)
				CGImageRelease(mThumbImage);
			mThumbImage = 0;
			if (mBackImage)
				CGImageRelease(mBackImage);
			mBackImage = 0;
			if (mImageName)
				CFRelease(mImageName);
					
			mImageName = s;
			
			if (mImageName) {
				CFRetain(mImageName);
				mBackImage = TImageCache::GetImage(mBundleRef, mImageName, CFSTR("png"), NULL);
				if (mBackImage) {
					mImageOrientation = OrientationForRect(RectForCGImage(mBackImage));
					FlushStaticBackground();
				}
				
				CFStringRef string	= CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%@Thumb"), mImageName);
				mThumbImage = TImageCache::GetImage(mBundleRef, string, CFSTR("png"), NULL);
				CFRelease(string);
			}				
				break;
		default:
			TViewNoCompositingCompatible::SetProperty(propID, s);
			break;
	};
}
//-----------------------------------------------------------------------------------
void TSliderControl::SetProperty(OSType propID, double val)
{
	switch(propID) {
		case 'xIns':
			mXInset = val;
			break;
		case 'tIns':
			mTopInset = val;
			break;
		case 'bIns':
			mBottomInset = val;
			break;
		default:
			TViewNoCompositingCompatible::SetProperty(propID, val);
			break;
	};
}
//-----------------------------------------------------------------------------------
bool TSliderControl::GetProperty(OSType propID, CFStringRef &s)
{
	bool res = true;
	switch(propID) {
		case 'pict':
			if (mImageName) {
				s = mImageName;
				CFRetain(s);
			} else {
				res = false;
			}
			break;
		default:
			res = TViewNoCompositingCompatible::GetProperty(propID, s);
			break;
	};
	
	return res;
}
//-----------------------------------------------------------------------------------
bool TSliderControl::GetProperty(OSType propID, double& val)
{
	bool res = true;
	switch(propID) {
		case 'xIns':
			val = mXInset;
			break;
		case 'tIns':
			val = mTopInset;
			break;
		case 'bIns':
			val = mBottomInset;
			break;
			
		default:
			res = TViewNoCompositingCompatible::GetProperty(propID, val);
			break;
	};
	return res;
}
