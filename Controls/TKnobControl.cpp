/*
 *  TKnobControl.cpp
 *  NeuSynth
 *
 *  Created by Airy André on Sun Oct 20 2002.
 *  Copyright (c) 2002 Airy André. All rights reserved.
 *
 */

#include "TKnobControl.h"
#include "TImageCache.h"
#include "AUGUIUtilities.h"

AUGUIProperties(TKnobControl) = {
	AUGUI::property_t('pict', CFSTR("picture"), CFSTR("Picture"), AUGUI::kPicture),
	AUGUI::property_t('maxA', CFSTR("max angle"), CFSTR("Max Angle"), AUGUI::kFloat),
	AUGUI::property_t('minA', CFSTR("min angle"), CFSTR("Min Angle"), AUGUI::kFloat),
	AUGUI::property_t('xOff', CFSTR("xoffset"), CFSTR("X Offset"), AUGUI::kFloat),
	AUGUI::property_t('yOff', CFSTR("yoffset"), CFSTR("Y Offset"), AUGUI::kFloat),
	AUGUI::property_t('endL', CFSTR("endless"), CFSTR("Endless"), AUGUI::kBool),
	AUGUI::property_t()
};
AUGUIInit(TKnobControl);


//#define EXPERIMENT

using namespace AUGUI;

// -----------------------------------------------------------------------------
//	macros
// -----------------------------------------------------------------------------
//

// -----------------------------------------------------------------------------
//	TKnobControl constructor
// -----------------------------------------------------------------------------
//
TKnobControl::TKnobControl(
						   HIViewRef inControl )
:	TViewNoCompositingCompatible( inControl ), mImage(0), mBackImage(0),
mXOffset(0), mYOffset(0), mMinAngle(-135), mMaxAngle(135), 	mEndless(false), mImageName(0)
{
}

// -----------------------------------------------------------------------------
//	TKnobControl destructor
// -----------------------------------------------------------------------------
//	Clean up after yourself.
//
TKnobControl::~TKnobControl()
{
    if (mImage)
		CGImageRelease(mImage);
    if (mBackImage)
		CGImageRelease(mBackImage);
	if (mImageName)
		CFRelease(mImageName);
}

// -----------------------------------------------------------------------------
//	GetKind
// -----------------------------------------------------------------------------
//
ControlKind TKnobControl::GetKind()
{
    const ControlKind kMyKind = { 'airy', 'knob' };
	
    return kMyKind;
}


//-----------------------------------------------------------------------------------
//	Initialize
//-----------------------------------------------------------------------------------
//	The control is set up.  Do the last minute stuff that needs to be done like
//	installing EventLoopTimers.
//
OSStatus TKnobControl::Initialize(
								  TCarbonEvent&		inEvent )
{
#pragma unused( inEvent )
    TitleChanged();
    return noErr;
}
//-----------------------------------------------------------------------------------
//	ValueChanged
//-----------------------------------------------------------------------------------
//
void TKnobControl::ValueChanged()
{
    Invalidate();
}
//-----------------------------------------------------------------------------------
//	TitleChanged
//	The title format is
//	 	filename minAngle maxAngle xOffset xOffset
//	With :
//		<filename>.pgn and <filename>Back.png are the pict used by the control
//		minAngle and maxAngle the angle for min and max values (-135,135 default values)
//		xOffset and yOffset the offset of the rotating part from the back (0,0 default values)
//		Only the filename is mandatory
//-----------------------------------------------------------------------------------
void TKnobControl::TitleChanged()
{
    if ( mBundleRef )
    {
		CFStringRef fileName;
		CopyControlTitleAsCFString(GetViewRef(), &fileName);
		CFShow(fileName);
		
		if ( fileName != NULL )
		{
			char file[100];
			char buffer[100] = "";
			
			float xOffset, yOffset, minAngle, maxAngle;
			CFStringGetCString(fileName, buffer, 100, kCFStringEncodingASCII);
			CFRelease( fileName );
			int nbVal = sscanf(buffer, "%s %f %f %f %f", file, &minAngle, &maxAngle, &xOffset, &yOffset);
			CFStringRef str = 0;
			if (nbVal >= 1)
				str = CFStringCreateWithCString(kCFAllocatorDefault, file, kCFStringEncodingUTF8);
			if (nbVal < 2) minAngle = -135.;
			if (nbVal < 3) maxAngle = 135.;
			if (nbVal < 4) xOffset = 0.;
			if (nbVal < 5) yOffset = 0.;
			if (str) SetProperty('pict', str);
			SetProperty('minA', minAngle);
			SetProperty('maxA', maxAngle);
			SetProperty('xOff', xOffset);
			SetProperty('yOff', yOffset);
			if (str)
				CFRelease(str);
		}
    }
}

//-----------------------------------------------------------------------------------
//	DrawStaticBackground
//-----------------------------------------------------------------------------------
//!	Draw the static part (background) of the control
//
void TKnobControl::DrawStaticBackground(
										RgnHandle				inLimitRgn,
										CGContextRef			inContext,
										bool  inCompositing)
{
#pragma unused( inLimitRgn )
    TRect					bounds = Bounds();
    CGContextRef			context = inContext;
	
    if (mBackImage)
		HIViewDrawCGImage( context, &bounds, mBackImage);
}

//-----------------------------------------------------------------------------------
//	CompatibleDraw
//-----------------------------------------------------------------------------------
//!	The fun part of the control
//
void TKnobControl::CompatibleDraw(
								  RgnHandle				inLimitRgn,
								  CGContextRef			inContext,
								  bool  inCompositing)
{
#pragma unused( inLimitRgn )
    TRect					bounds = Bounds();
    CGContextRef			context = inContext;
    TRect knobBounds = bounds;
	
    if (mBackImage)
		HIViewDrawCGImage( context, &bounds, mBackImage);
	
    if (mImage) {
		float angle = (((float) (GetValue() - GetMinimum())/ (float) (GetMaximum()-GetMinimum())) * (mMaxAngle-mMinAngle) + mMinAngle);
		if (mBackImage)
		{
			float scaleX = float(CGImageGetWidth(mImage)) / float(CGImageGetWidth(mBackImage));
			float scaleY = float(CGImageGetHeight(mImage)) / float(CGImageGetHeight(mBackImage));
			knobBounds.SetWidth(knobBounds.Width() * scaleX);
			knobBounds.SetHeight(knobBounds.Height() * scaleY);
		}
		float knobWidth = knobBounds.Width();
		float knobHeight = knobBounds.Height();
		
		CGContextTranslateCTM(context, mXOffset + (bounds.MinX() + bounds.MaxX())/2., mYOffset + (bounds.MinY() + bounds.MaxY())/2.);
		CGContextRotateCTM( context, DegreesToRadians(angle) );
		knobBounds.MoveTo(-knobWidth/2, -knobHeight/2);
		HIViewDrawCGImage( context, &knobBounds, mImage);
    }
}

// -----------------------------------------------------------------------------
//	StillTracking
// -----------------------------------------------------------------------------
//!	Called during tracking.
//
OSStatus TKnobControl::StillTracking(TCarbonEvent&inEvent, HIPoint& from)
{
#ifdef EXPERIMENT
	const double kRatio = 250.;
#else
	const double kRatio = 50.;
#endif
    HIPoint mouse;
    float deltaX, deltaY;
    SInt32 mini, maxi;
    UInt32 modifiers;
	
    mini = GetMinimum();
    maxi = GetMaximum();
	
    inEvent.GetParameter<HIPoint>( kEventParamWindowMouseLocation, typeHIPoint, &mouse );
    inEvent.GetParameter( kEventParamKeyModifiers, &modifiers );
    ConvertToLocal(mouse);
    deltaX = mouse.x-from.x;
    deltaY = mouse.y-from.y;
    SInt32 curVal = GetValue();
    SInt32 val;
#ifdef EXPERIMENT
    TRect knobBounds = Bounds();
	knobBounds.MoveTo(0.,0.);
	if (from.x > knobBounds.Width()/2) {
		deltaY = -deltaY;
	}
	if (from.y > knobBounds.Height()/2) {
		deltaX = -deltaX;
	}
#endif
	
#if SUPPORT_CONTROL_MODIFIERS
    if (modifiers & HIGHEST_PRECISION_MODIFIER) {
		val = SInt32(rint(curVal+(deltaX-deltaY)*(maxi-mini)/(kRatio*400.)));
    } else if (modifiers & HIGH_PRECISION_MODIFIER) {
		val = SInt32(rint(curVal+(deltaX-deltaY)*(maxi-mini)/(kRatio*20.)));
    } else {
#endif
		val = SInt32(rint(curVal+(deltaX-deltaY)*(maxi-mini)/kRatio));
#if SUPPORT_CONTROL_MODIFIERS
    }
#endif
	if (!mEndless) {
		if ( val > maxi) val = maxi;
		if ( val < mini) val = mini;
	} else {
		while (val > maxi)
			val -= (maxi - mini);
		while (val < mini)
			val += (maxi - mini);
	}
    if (val != curVal) {
		SetValue ( val );
		from = mouse;
    }
#ifdef EXPERIMENT
	from = mouse;
#endif
    return noErr;
}

//! This method is used to set the background image
OSStatus TKnobControl::SetData(OSType inTag, ControlPartCode inPart, Size inSize, const void* inPtr)
{
	OSStatus err = noErr;
	switch (inTag)
	{
		case kKnobControlBackgroundImageTag:
			if (inSize == sizeof(mBackImage)) {
				if (mBackImage)
					CFRelease(mBackImage);
				mBackImage = *(CGImageRef *)inPtr;
				if (mBackImage)
					CFRetain(mBackImage);
				FlushStaticBackground();
				ForceRedraw();
			} else {
				err = errDataSizeMismatch;
			}
			break;
			
		default:
			// unsupported data tag
			err = errDataNotSupported;
	}
	return err;
}

//-----------------------------------------------------------------------------------
void TKnobControl::SetProperty(OSType propID, CFStringRef s)
{
	switch(propID) {
		case 'pict':
			if (mImage)
				CGImageRelease(mImage);
			if (mBackImage)
				CGImageRelease(mBackImage);
			if (mImageName)
				CFRelease(mImageName);
					
			mImageName = s;
			CFRetain(s);
			if (mImageName) {
				CFRetain(mImageName);
				
				mImage = TImageCache::GetImage(mBundleRef, mImageName, CFSTR("png"), NULL);
				
				CFStringRef string	= CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%@Back"), mImageName);
				mBackImage  = TImageCache::GetImage(mBundleRef, string, CFSTR("png"), NULL);
				CFRelease(string);
			}				
			break;
		default:
			TViewNoCompositingCompatible::SetProperty(propID, s);
			break;
	};
}
//-----------------------------------------------------------------------------------
void TKnobControl::SetProperty(OSType propID, AUGUI::color_t& r)
{
	switch(propID) {
		default:
			TViewNoCompositingCompatible::SetProperty(propID, r);
			break;
	};
}
//-----------------------------------------------------------------------------------
void TKnobControl::SetProperty(OSType propID, int32_t val)
{
	switch(propID) {
		case 'endL':
			mEndless = val;
			break;			
		default:
			TViewNoCompositingCompatible::SetProperty(propID, val);
			break;
	};
}
//-----------------------------------------------------------------------------------
void TKnobControl::SetProperty(OSType propID, double val)
{
	switch(propID) {
		case 'minA':
			mMinAngle = val;
			break;
		case 'maxA':
			mMaxAngle = val;
			break;
		case 'xOff':
			mXOffset = val;
			break;
		case 'yOff':
			mYOffset = val;
			break;
		default:
			TViewNoCompositingCompatible::SetProperty(propID, val);
			break;
	};
}
//-----------------------------------------------------------------------------------
bool TKnobControl::GetProperty(OSType propID, AUGUI::color_t& r)
{
	bool res = true;
	switch(propID) {
		default:
			res = TViewNoCompositingCompatible::GetProperty(propID, r);
			break;
	};
	return res;
}
//-----------------------------------------------------------------------------------
bool TKnobControl::GetProperty(OSType propID, CFStringRef &s)
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
bool TKnobControl::GetProperty(OSType propID, double& val)
{
	bool res = true;
	switch(propID) {
		case 'minA':
			val = mMinAngle;
			break;
		case 'maxA':
			val = mMaxAngle;
			break;
		case 'xOff':
			val = mXOffset;
			break;
		case 'yOff':
			val = mYOffset;
			break;
			
		default:
			res = TViewNoCompositingCompatible::GetProperty(propID, val);
			break;
	};
	return res;
}
//-----------------------------------------------------------------------------------
bool TKnobControl::GetProperty(OSType propID, int32_t& val)
{
	bool res = true;
	switch(propID) {
		case 'endL':
			val = mEndless;
			break;			
		default:
			res = TViewNoCompositingCompatible::GetProperty(propID, val);
			break;
	};
	return res;
}	
