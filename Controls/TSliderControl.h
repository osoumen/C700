/*
 *  TSliderControl.h
 *  Ritmo
 *
 *  Created by Chris Reed on Mon Feb 03 2003.
 *  Copyright (c) 2003 Chris Reed. All rights reserved.
 *
 */

#if !defined(_TSliderControl_h_)
#define _TSliderControl_h_

#include "TViewNoCompositingCompatible.h"

//! Value at which center detent appears. The value is an SInt32.
#define kSliderControlDetentValueTag FOUR_CHAR_CODE('CDet')

//! The range the current value must be in for it to snap to the detent
//! value. This value is an SInt32.
#define kSliderControlDetentRangeTag FOUR_CHAR_CODE('RDet')

//! A boolean flag of type UInt32 indicating whether to use a center detent.
#define kSliderControlUseDetentTag FOUR_CHAR_CODE('UDet')

/*!
 * \class TSliderControl
 * \brief A slider control with configurable background and thumb images.
 *
 * This slider control operates very much like the standard Aqua slider
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
 * The control title specifies the base image file name used by the slider.
 * Currently the only file format supported in PNG. The filename given in
 * the control title has ".png" added to it to construct the background
 * image filename. The thumb's filename is the base name plus "Thumb.png".
 *
 * In addition to the base file name, the title can contain several other
 * fields, each separated by a single space character. These fields are,
 * in order: left inset, top inset, and bottom inset. These field names
 * are for a vertically oriented slider, so they become top inset, left
 * inset, and right inset for a horizontally oriented slider. This is also
 * true for when the slider is rotated due to the bounds orientation.
 *
 * Another optional feature of the slider is support for a center detent
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

class TSliderControl : public TViewNoCompositingCompatible
{
	AUGUIDefineControl(TSliderControl, TViewNoCompositingCompatible, "slider");
	
public:
	// Control properties management
	virtual void SetProperty(OSType propID, double value);
	virtual void SetProperty(OSType propID, CFStringRef value);
	virtual bool GetProperty(OSType propID, double &value);
	virtual bool GetProperty(OSType propID, CFStringRef &value);
	
protected:
	// Constructor/Destructor
	TSliderControl(HIViewRef inControl);
	virtual ~TSliderControl();
	
	virtual ControlKind GetKind();
	
	virtual bool UseNonblockingTracking() { return true; }
	
	virtual OSStatus Initialize(TCarbonEvent& inEvent);
	virtual OSStatus		StillTracking(TCarbonEvent&inEvent, HIPoint& from);
	virtual OSStatus		StartTracking(TCarbonEvent&inEvent, HIPoint& from);
    virtual void		DrawStaticBackground(RgnHandle	inLimitRgn, CGContextRef inContext, bool  inCompositing );
	virtual void CompatibleDraw(RgnHandle inLimitRgn, CGContextRef inContext, bool inCompositing);
	virtual void TitleChanged();
	virtual void ValueChanged();
	virtual UInt32 GetBehaviors() { return TViewNoCompositingCompatible::GetBehaviors() | kControlSupportsEmbedding ; }
	
	virtual OSStatus SetData(OSType inTag, ControlPartCode inPart, Size inSize, const void* inPtr);
	virtual OSStatus GetData(OSType inTag, ControlPartCode inPart, Size inSize, Size* outSize, void* inPtr);

protected:
	typedef enum {
		kHorizontalOrientation,
		kVerticalOrientation
	} SliderOrientation;
	
    CGImageRef mThumbImage;
    CGImageRef mBackImage;
	CFStringRef mImageName;
	float mXInset;
	float mTopInset;
	float mBottomInset;
	SliderOrientation mImageOrientation;
	bool mUseDetent;
	SInt32 mDetentValue;
	SInt32 mDetentRange;
	float mTotalDetentMoved;
	
	virtual float ScaleX();
	virtual float ScaleY();
	SliderOrientation OrientationForRect(const TRect& theRect);
	TRect ScaledThumbBounds();
};


#endif // _TSliderControl_h_
