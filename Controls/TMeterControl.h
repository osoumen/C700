/*
 *  TMeterControl.h
 *  AUStk
 *
 *  Created by Airy on Wed Mar 19 2003.
 *  Copyright (c) 2003 Airy ANDRE. All rights reserved.
 *
 */

#if !defined(_TMeterControl_h_)

#define _TMeterControl_h_
#include <TSliderControl.h>

/*!
* \class TMeterControl
 * \brief A meter control with configurable background and moving part.
 *
 * This meter control operates very much like the standard Aqua meter
 * provided by Apple. The major improvements are configurable background
 * and thumb images, and high precision modes accessed by the option and
 * control keys.
 *
 * Another feature is that the control can be used either vertically or
 * horizontally, regardless of the original image orientation. The original
 * orientation is determined by the height and width of the background
 * image. If the image is higher than it is wide, the orientation is
 * vertical, otherwise the orientation is horizontal. The thumb is assumed
 * to have the same orientation as the background, but can be any size.
 * So it is possible to have a thumb wider than it is tall when the
 * orientation is vertical.
 *
 * Like the image orientation, the control orientation is determined by the
 * height and width of the control's bounding rectangle. If its height is
 * greater than its width, the control has a vertical orientation. When
 * the orientation of the background image does not match that of the
 * control, the graphics, both background and thumb, are rotated 90 degrees
 * clockwise.
 *
 * The control title specifies the base image file name used by the meter.
 * Currently the only file format supported in PNG. The filename given in
 * the control title has ".png" added to it to construct the background
 * image filename. The thumb's filename is the base name plus "Thumb.png".
 *
 * In addition to the base file name, the title can contain several other
 * fields, each separated by a single space character. These fields are,
 * in order: left inset, top inset, and bottom inset. These field names
 * are for a vertically oriented meter, so they become top inset, left
 * inset, and right inset for a horizontally oriented meter. This is also
 * true for when the meter is rotated due to the bounds orientation.
 *
 * Another optional feature of the meter is support for a center detent
 * value. If this is used, the cursor will "stick" when the control hits
 * this value and will have to move more pixels than normal to continue
 * sliding. Turn on center detent support by writing a \c UInt32 value of
 * 1 to the tagged data \c kSliderControlUseDetentTag, using \c
 * SetControlData(). Set the value at which the center detent appears
 * (which is 0 by default) using the data tag \c kSliderControlDetentValueTag.
 * Finally, you can control the range that the value will "fall into" the
 * center detent by setting the \c kSliderControlDetentRangeTag tagged data.
 * If the range is 100, then the difference between the current value and
 * the center detent value must be less than 100 for the mouse to stick.
 */

class TMeterControl : public TSliderControl
{
	AUGUIDefineControl(TMeterControl, TSliderControl, "meter");
protected:
	// Constructor/Destructor
	TMeterControl(HIViewRef inControl);
	virtual ~TMeterControl();

	virtual OSStatus StillTracking(TCarbonEvent&inEvent, HIPoint& from);
	virtual OSStatus StartTracking(TCarbonEvent&inEvent, HIPoint& from);
	virtual void CompatibleDraw(RgnHandle inLimitRgn, CGContextRef inContext, bool inCompositing);
	virtual float ScaleY();
};


#endif // _TMeterControl_h_
