/*
 *  TMeterPartialControl.cpp
 *  AU303
 *
 *  Created by Airy on Thu Apr 15 2003.
 *  Copyright (c) 2003 Airy ANDRE. All rights reserved.
 *
 */

#include "TMeterPartialControl.h"
#include "AUGUIUtilities.h"

AUGUIProperties(TMeterPartialControl) = {
	AUGUI::property_t()
};
AUGUIInit(TMeterPartialControl);

using namespace AUGUI;

TMeterPartialControl::TMeterPartialControl(HIViewRef inControl)
: TMeterControl(inControl)
{
}

TMeterPartialControl::~TMeterPartialControl()
{
}



//! The fun part of the control
//!
void TMeterPartialControl::CompatibleDraw(RgnHandle inLimitRgn, CGContextRef inContext, bool inCompositing)
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
		float position = (float(GetValue() - GetMinimum()+1) / float(GetMaximum() - GetMinimum() + 1)) * slideDistance;

		orientedThumbBounds.SetAroundCenter(bounds.CenterX(), bounds.CenterY(),
									  orientedThumbBounds.Width(), orientedThumbBounds.Height());

		float xOffset, yOffset;
		if (boundsOrientation == TSliderControl::kVerticalOrientation)
		{
			xOffset = bounds.CenterX() + mXInset - orientedThumbBounds.Width()/2.0;
			yOffset = bounds.CenterY() + mTopInset - slideDistance / 2.0;
		}
		else
		{
			yOffset = bounds.CenterY() + mXInset - - orientedThumbBounds.Height()/2.0;;
			xOffset = bounds.CenterX() + mTopInset + slideDistance / 2.0;
		}

		CGContextTranslateCTM(context, xOffset, yOffset);
		orientedThumbBounds.SetOrigin(thumbBounds.Origin());
		if (boundsOrientation == TSliderControl::kHorizontalOrientation) {
			orientedThumbBounds.SetWidth(slideDistance/(GetMaximum() - GetMinimum()+1));
		} else {
			orientedThumbBounds.MoveBy(0, orientedThumbBounds.Height()-position);
			orientedThumbBounds.SetHeight(slideDistance/(GetMaximum() - GetMinimum()+1));
		}
		CGContextClipToRect(context, orientedThumbBounds);

		if (mImageOrientation != boundsOrientation)
		{
			CGContextRotateCTM(context, DegreesToRadians(90.0));
		}
		HIViewDrawCGImage(context, &thumbBounds, mThumbImage);
    }
}
