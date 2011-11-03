/*
 *  TMeterControl.cpp
 *  AUStk
 *
 *  Created by Airy on Wed Mar 19 2003.
 *  Copyright (c) 2003 Airy ANDRE. All rights reserved.
 *
 */

#include "TMeterControl.h"
#include "AUGUIUtilities.h"

AUGUIProperties(TMeterControl) = {
	AUGUI::property_t()
};
AUGUIInit(TMeterControl);

using namespace AUGUI;

TMeterControl::TMeterControl(HIViewRef inControl)
: TSliderControl(inControl)
{
}

TMeterControl::~TMeterControl()
{
}


// -----------------------------------------------------------------------------
//	StartTracking
// -----------------------------------------------------------------------------
//!	Called before tracking. The new control value is calculated so that the
//! slider thumb ends up centered on the mouse. This takes into account all
//! of the inset and size values, and also considers the background image
//! and thumb orientation compared to the slider orientation.
//
OSStatus TMeterControl::StartTracking(TCarbonEvent& inEvent, HIPoint& from)
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
        distance = orientedThumbBounds.Width();
        pos = (from.x - bounds.Width()/2.0 - mTopInset + orientedThumbBounds.Width()/2.0) / distance;
    }
    else
    {
        distance = orientedThumbBounds.Height();
        pos = 1.f - (from.y - bounds.Height()/2.0 - mTopInset + orientedThumbBounds.Height()/2.0) / distance;
    }
    
    SInt32 curVal = GetValue();
    SInt32 val = SInt32(rint(mini+pos*(maxi-mini+1)));
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
OSStatus TMeterControl::StillTracking(TCarbonEvent&inEvent, HIPoint& from)
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
        distance = orientedThumbBounds.Width();
        pos = (mouse.x - bounds.Width()/2.0 - mTopInset + orientedThumbBounds.Width()/2.0) / distance;
        d = delta.x / distance;
    }
    else
    {
        distance = orientedThumbBounds.Height();
        pos = 1.f - (mouse.y - bounds.Height()/2.0 - mTopInset + orientedThumbBounds.Height()/2.0) / distance;
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
    if ((modifiers == controlKey || modifiers == optionKey))
    {
        // value depends on how much the mouse moved
        SInt32 ratio = 1;
        if (modifiers & controlKey)
            ratio = 50;
        else if (modifiers & optionKey)
            ratio = 10;
        val = SInt32(rint(curVal + d * (1 + maxi - mini) / ratio));
    }
    else
    {
        // value is directly proportional to position
        val = SInt32(rint(mini + pos * (1 + maxi - mini)));
    }
    if ( val > maxi) val = maxi;
    if ( val < mini) val = mini;
    if (val != curVal) {
        SetValue ( val );
        from = mouse;
    }
    return noErr;
}

//! The fun part of the control
//!
//!	XXX need to test with vertically oriented images
void TMeterControl::CompatibleDraw(RgnHandle inLimitRgn, CGContextRef inContext, bool inCompositing)
{
#pragma unused(inLimitRgn)
    TRect bounds = Bounds();
    CGContextRef context = inContext;
    SliderOrientation boundsOrientation = OrientationForRect(bounds);
    TRect backBounds = RectForCGImage(mBackImage);
    
    float scaleY = ScaleY();
    // draw slider thumb
    if (mThumbImage)
    {
        TRect thumbBounds = ScaledThumbBounds();
        
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
            slideDistance = orientedThumbBounds.Height();
        else
            slideDistance = orientedThumbBounds.Width();
        slideDistance -= (mTopInset + mBottomInset) * scaleY;
        float position = (float(GetValue() - GetMinimum()) / float(GetMaximum() - GetMinimum())) * slideDistance;
        
        orientedThumbBounds.SetAroundCenter(bounds.CenterX(), bounds.CenterY(),
                                            orientedThumbBounds.Width(), orientedThumbBounds.Height());
#if 0
        float xOffset, yOffset;
        if (boundsOrientation == TSliderControl::kVerticalOrientation)
        {
            xOffset = bounds.CenterX() + mXInset - orientedThumbBounds.Width()/2.0;
            yOffset = bounds.MinY() + mTopInset + (bounds.Height() + slideDistance) / 2.0;
        }
        else
        {
            xOffset = bounds.MinX() + mTopInset + (bounds.Width() + slideDistance) / 2.0;
            yOffset = bounds.CenterY() + mXInset - - orientedThumbBounds.Height()/2.0;;
        }
#else
        float xOffset, yOffset;
        if (boundsOrientation == TSliderControl::kVerticalOrientation)
        {
            xOffset = bounds.CenterX() + mXInset - orientedThumbBounds.Width()/2.0;
            yOffset = bounds.CenterY() + mTopInset - slideDistance / 2.0;
        }
        else
        {
            yOffset = bounds.CenterY() + mXInset - orientedThumbBounds.Height()/2.0;;
            xOffset = bounds.CenterX() + mTopInset - slideDistance / 2.0;
        }
#endif
        CGContextTranslateCTM(context, xOffset, yOffset);
        orientedThumbBounds.SetOrigin(thumbBounds.Origin());
        if (boundsOrientation == TSliderControl::kHorizontalOrientation) {
            orientedThumbBounds.SetWidth(position);
        } else {
            orientedThumbBounds.MoveBy(0, orientedThumbBounds.Height()-position);
            orientedThumbBounds.SetHeight(position);
        }
        CGContextClipToRect(context, orientedThumbBounds);
        
        if (mImageOrientation != boundsOrientation)
        {
            CGContextRotateCTM(context, DegreesToRadians(90.0));
        }
        HIViewDrawCGImage(context, &thumbBounds, mThumbImage);
    }
}

float TMeterControl::ScaleY()
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
                scale = bounds.Height() / backBounds.Width();
            else
                scale = bounds.Width() / backBounds.Height();
        }
        else
        {
            if (boundsOrientation == TSliderControl::kVerticalOrientation)
                scale = bounds.Height() / backBounds.Height();
            else
                scale = bounds.Width() / backBounds.Width();
        }
    }
    return 1.f/scale;
}
