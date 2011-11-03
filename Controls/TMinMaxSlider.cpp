/*
 *  TMinMaxSlider.cpp
 *  Muso
 *
 *  Created by Airy André on 20/03/05.
 *  Copyright 2005 __mMaxCompanyName__. All rights reserved.
 *
 */

#include "TMinMaxSlider.h"

/*
 *  TMinMaxSlider.cpp
 *  CAUGuiDemo
 *
 *  Created by Airy on Sat May 24 2003.
 *  Copyright (c) 2003 Airy ANDRE. All rights reserved.
 *
 */

#include "TMinMaxSlider.h"
#include "TImageCache.h"
#include "AUGUIUtilities.h"

AUGUIProperties(TMinMaxSlider) = {
				AUGUI::property_t()
};
AUGUIInit(TMinMaxSlider);

using namespace AUGUI;

// -----------------------------------------------------------------------------
//	macros
// -----------------------------------------------------------------------------
//

// -----------------------------------------------------------------------------
//	TMinMaxSlider constructor
// -----------------------------------------------------------------------------
//
TMinMaxSlider::TMinMaxSlider(
							 HIViewRef			inControl )
:	TSliderControl( inControl ), mMin(0.f), mMax(.2f)
{
}

// -----------------------------------------------------------------------------
//	TMinMaxSlider destructor
// -----------------------------------------------------------------------------
//	Clean up after yourself.
//
TMinMaxSlider::~TMinMaxSlider()
{
}

// -----------------------------------------------------------------------------
//	GetKind
// -----------------------------------------------------------------------------
//
ControlKind TMinMaxSlider::GetKind()
{
    const ControlKind kmMaxKind = { 'airy', 'mmax' };
    return kmMaxKind;
}

//-----------------------------------------------------------------------------------
//	CompatibleDraw
//-----------------------------------------------------------------------------------
//!	The fun part of the control
//
void TMinMaxSlider::CompatibleDraw(
								   RgnHandle				inLimitRgn,
								   CGContextRef			inContext,
								   bool  inCompositing)
{
/*
    float scaleX = ScaleX();
    float scaleY = ScaleY();
*/	
    // draw slider thumb
    if (mThumbImage) {
		TRect thumbRect = ScaledThumbBounds();
		TRect	bounds = Bounds();
		thumbRect.SetOrigin(bounds.Origin());
		thumbRect.MoveBy(mMin*bounds.Width(), (bounds.Height()-thumbRect.Height()));
		float length = (mMax-mMin)*bounds.Width();
		if (length < 6.f)
			length = 6.f;
		thumbRect.SetWidth(length);
		HIViewDrawCGImage( inContext, &thumbRect, mThumbImage);
	}
}

// -----------------------------------------------------------------------------
//	StartTracking
// -----------------------------------------------------------------------------
//!	Called before tracking.
//
OSStatus TMinMaxSlider::StartTracking(TCarbonEvent& inEvent, HIPoint& from)
{
    TRect bounds = Bounds();
	float x = from.x / bounds.Width();
	float delta = 1.f / bounds.Width();
	if (x <= mMin + delta && x >= mMin - 4.f*delta)
		mMoveType = kStart;
	else if (x >= mMax - delta && x <= mMax + 4.f*delta)
		mMoveType = kEnd;
	else {
		mMoveType = kMove;
	}
	mStartTracking = from.x;
	float length = mMax - mMin;
	switch (mMoveType) {
		case kStart:
			mMin = x;
			if (mMin > mMax) 
				mMax = mMin;
			break;
		case kEnd:
			mMax = x;
			if (mMin > mMax) 
				mMin = mMax;
			break;
		default:
			if (x >= mMin && x <= mMax) {
				mStartTracking = x - mMin;
			} else {
				mMin = x - length * .5f;
				if (mMin < 0.f)
					mMin = 0.f;
				mMax = mMin + length;
				if (mMax > 1.f) {
					mMax = 1.f;
					mMin = mMax - length;
				}
				mStartTracking = length*.5f;
			}
			break;
	}
	if (mMin < 0.f)
		mMin = 0.f;
	if (mMin > 1.f) 
		mMin = 1.f;
	if (mMax < 0.f) 
		mMax = 0.f;
	if (mMax > 1.f) 
		mMax = 1.f;
	
	SetValue(!GetValue());
    return noErr;}

// -----------------------------------------------------------------------------
//	StillTracking
// -----------------------------------------------------------------------------
//!	Called during tracking.
//
OSStatus TMinMaxSlider::StillTracking(TCarbonEvent&inEvent, HIPoint& from)
{
    TRect bounds = Bounds();
	HIPoint mouse;
    inEvent.GetParameter<HIPoint>( kEventParamWindowMouseLocation, typeHIPoint, &mouse );
    ConvertToLocal(mouse);
	
    UInt32 modifiers;
    inEvent.GetParameter( kEventParamKeyModifiers, &modifiers );
	float length = mMax - mMin;
	if ((modifiers == controlKey || modifiers == optionKey))
	{
		// value depends on how much the mouse moved
		double ratio = 1.;
		if (modifiers & controlKey)
			ratio = 50.;
		else if (modifiers & optionKey)
			ratio = 10.;
		mMin += (mouse.x-from.x) / (bounds.Width() * ratio);
	}
	else
	{
		// value is directly proportional to position
		switch (mMoveType) {
			case kStart:
				mMin = mouse.x / bounds.Width();
				if (mMin > mMax) 
					mMin = mMax;
					if (mMin < 0.f) 
						mMin = 0.f;
						if (mMin > 1.f) mMin = 1.f;
							if (mMax < 0.f) mMax = 0.f;
								if (mMax > 1.f) mMax = 1.f;
									break;
			case kEnd:
				mMax = mouse.x / bounds.Width();
				if (mMin > mMax) 
					mMax = mMin;
					if (mMin < 0.f) mMin = 0.f;
						if (mMin > 1.f) mMin = 1.f;
							if (mMax < 0.f) mMax = 0.f;
								if (mMax > 1.f) mMax = 1.f;
									break;
			default:
				mMin = mouse.x / bounds.Width();
				mMin -= mStartTracking;
				if (mMin < 0.f)
					mMin = 0.f;
				mMax = mMin + length;
				if (mMax > 1.f) {
					mMax = 1.f;
					mMin = mMax - length;
				}
					break;
		}
	}
	SetValue(!GetValue());
	from = mouse;
    return noErr;
}

OSStatus TMinMaxSlider::SetData(OSType inTag, ControlPartCode inPart, Size inSize, const void* inPtr)
{
	OSStatus err = noErr;
	switch (inTag)
	{
		case minTag:
			if (inSize == sizeof(mMin))
			{
				double length = mMax - mMin;
				mMin = *(double *)inPtr;
				mMax = mMin + length;
				if (mMax > 1.f) {
					mMax = 1.f;
					mMin = mMax - length;
				}				
			}
			else
			{
				err = errDataSizeMismatch;
			}
			if (IsCompositing())
				Invalidate();
			else
				if (IsVisible()) Draw(0,0);
				break;
		case maxTag:
			if (inSize == sizeof(mMax))
			{
				mMax = *(double *)inPtr;
			}
			else
			{
				err = errDataSizeMismatch;
			}
			if (IsCompositing())
				Invalidate();
			else
				if (IsVisible()) Draw(0,0);
				break;
		case lenTag:
			if (inSize == sizeof(double))
			{
				double length = *(double *)inPtr;
				mMax = mMin + length;
				if (mMax > 1.f) {
					mMax = 1.f;
					mMin = mMax - length;
				}
			}
			else
			{
				err = errDataSizeMismatch;
			}
			if (IsCompositing())
				Invalidate();
			else
				if (IsVisible()) Draw(0,0);
				break;
		default:
			// unsupported data tag
			err = errDataNotSupported;
	}
	return err;
}

OSStatus TMinMaxSlider::GetData(OSType inTag, ControlPartCode inPart, Size inSize, Size* outSize, void* inPtr)
{
	OSStatus err = noErr;
	switch (inTag)
	{
		case minTag:
			if (inSize == sizeof(mMin)) {
				*(double*)inPtr = mMin;
			} else
				err = errDataSizeMismatch;
			break;
		case maxTag:
			if (inSize == sizeof(mMax))
				*(double*)inPtr = mMax;
			else
				err = errDataSizeMismatch;
			break;
		case lenTag:
			if (inSize == sizeof(double)) {
				*(double*)inPtr = mMax - mMin;
				
			} else
				err = errDataSizeMismatch;
			break;
		default:
			err = TViewNoCompositingCompatible::GetData(inTag, inPart, inSize, outSize, inPtr);
	}
	return err;
}

