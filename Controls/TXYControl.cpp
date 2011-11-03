/*
 *  TXYControl.cpp
 *  CAUGuiDemo
 *
 *  Created by Airy on Sat May 24 2003.
 *  Copyright (c) 2003 Airy ANDRE. All rights reserved.
 *
 */

#include "TXYControl.h"
#include "TImageCache.h"
#include "AUGUIUtilities.h"

using namespace AUGUI;

AUGUIProperties(TXYControl) = {
//				AUGUI::property_t('pict', CFSTR("picture"), CFSTR("Picture"), AUGUI::kPicture),
				AUGUI::property_t()
};
AUGUIInit(TXYControl);

// -----------------------------------------------------------------------------
//	macros
// -----------------------------------------------------------------------------
//

// -----------------------------------------------------------------------------
//	TXYControl constructor
// -----------------------------------------------------------------------------
//
TXYControl::TXYControl(HIViewRef inControl):TViewNoCompositingCompatible(inControl), mBackImage(0), mMarker(0), mX(0), mY(0)
{	
	mMarker_Size	= 6.0f;

	mMarker_R		= 0.0f;
	mMarker_G		= 0.0f;
	mMarker_B		= 0.0f;
	mMarker_A		= 1.0f;
}

// -----------------------------------------------------------------------------
//	TXYControl destructor
// -----------------------------------------------------------------------------
//	Clean up after yourself.
//
TXYControl::~TXYControl()
{
    if (mBackImage)
		CGImageRelease(mBackImage);
    if (mMarker)
		CGImageRelease(mMarker);
}

// -----------------------------------------------------------------------------
//	GetKind
// -----------------------------------------------------------------------------
//
ControlKind TXYControl::GetKind()
{
    const ControlKind kMyKind = { 'airy', 'xypn' };
    return kMyKind;
}


//-----------------------------------------------------------------------------------
//	Initialize
//-----------------------------------------------------------------------------------
//	The control is set up.  Do the last minute stuff that needs to be done like
//	installing EventLoopTimers.
//
OSStatus TXYControl::Initialize(
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
void TXYControl::ValueChanged()
{
    Invalidate();
}
//-----------------------------------------------------------------------------------
//	TitleChanged
//	The title format is
//	 	filename
//	With :
//		<filename>.pgn and <filename>Back.png are the pict used by the control
//-----------------------------------------------------------------------------------
void TXYControl::TitleChanged()
{
    if (!mBundleRef)
		return;
	
	CFStringRef infoString;
	CopyControlTitleAsCFString(GetViewRef(), &infoString);
	
	if (infoString == NULL)
		return;
	
	CFArrayRef info = CFStringCreateArrayBySeparatingStrings(kCFAllocatorDefault, infoString, CFSTR(","));
	
	if (info == NULL)
		return;
	
	CFIndex		count   = CFArrayGetCount(info);
	CFStringRef string  = NULL;
	
	//  background image and marker
	
	if (count >= 1)
	{
		if (mMarker)
			CGImageRelease(mMarker);
			
		if (mBackImage)
			CGImageRelease(mBackImage);
		
		string		= (CFStringRef)CFArrayGetValueAtIndex(info, 0);
		mMarker		= TImageCache::GetImage(mBundleRef, string, CFSTR("png"), NULL);
		
		string		= CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%@Back"), string);
		mBackImage  = TImageCache::GetImage(mBundleRef, string, CFSTR("png"), NULL);
	}
	
	//  marker color
	
	if (count >= 2)
	{
		string = (CFStringRef)CFArrayGetValueAtIndex(info, 1);
		char	buffer[9];
		int		color;
		
		CFStringGetCString(string, buffer, 9, CFStringGetSystemEncoding());
		sscanf(buffer, "%x", &color);
		
		mMarker_A = (color >> 24 & 0xff) / 255.0f;
		mMarker_R = (color >> 16 & 0xff) / 255.0f;
		mMarker_G = (color >> 8	 & 0xff) / 255.0f;
		mMarker_B = (color		 & 0xff) / 255.0f;

		if (mMarker_A == 0.0f)
			mMarker_A = 1.0f;
	}
	
	//  marker size
	
	if (count >= 3)
		mMarker_Size = CFStringGetIntValue((CFStringRef)CFArrayGetValueAtIndex(info, 2));
	
	//  clean up
	
	CFRelease(info);
	CFRelease(infoString);	
}

//-----------------------------------------------------------------------------------
//	DrawStaticBackground
//-----------------------------------------------------------------------------------
//!	Draw the static part (background) of the control
//
void TXYControl::DrawStaticBackground(
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
void TXYControl::CompatibleDraw(RgnHandle		inLimitRgn,
								CGContextRef	inContext,
								bool			inCompositing)
{
#pragma unused(inLimitRgn)

	TRect	bounds = Bounds();
	float	x, y;
	
    if (mMarker)
	{
		TRect markerBounds = RectForCGImage(mMarker);
		
		x = bounds.MinX() + mX * (bounds.MaxX() - bounds.MinX()) - markerBounds.Width() / 2.0f;
		y = bounds.MaxY() - mY * (bounds.MaxY() - bounds.MinY()) - markerBounds.Height() / 2.0f;
		
		markerBounds.MoveTo(x, y);
		HIViewDrawCGImage(inContext, &markerBounds, mMarker);
    }
	else
	{
		CGContextSaveGState(inContext);

		x = bounds.MinX() + mX * (bounds.MaxX() - bounds.MinX());
		y = bounds.MaxY() - mY * (bounds.MaxY() - bounds.MinY());

		CGContextBeginPath(inContext);    
		CGContextAddArc(inContext, x, y, mMarker_Size * 0.5f, 0, 2.0f * M_PI, 0);
   
		CGContextSetRGBFillColor(inContext, mMarker_R, mMarker_G, mMarker_B, mMarker_A);
		CGContextFillPath(inContext);
		
		CGContextRestoreGState(inContext);	
	}
}

// -----------------------------------------------------------------------------
//	StartTracking
// -----------------------------------------------------------------------------
//!	Called before tracking.
//
OSStatus TXYControl::StartTracking(TCarbonEvent& inEvent, HIPoint& from)
{
    TRect bounds = Bounds();
	mX = from.x / bounds.Width();
	mY = 1. - from.y / bounds.Height();
	SetValue(!GetValue());
    return noErr;
}

// -----------------------------------------------------------------------------
//	StillTracking
// -----------------------------------------------------------------------------
//!	Called during tracking.
//
OSStatus TXYControl::StillTracking(TCarbonEvent&inEvent, HIPoint& from)
{
    TRect bounds = Bounds();
	HIPoint mouse;
    inEvent.GetParameter<HIPoint>( kEventParamWindowMouseLocation, typeHIPoint, &mouse );
    ConvertToLocal(mouse);
	
    UInt32 modifiers;
    inEvent.GetParameter( kEventParamKeyModifiers, &modifiers );
	if ((modifiers == controlKey || modifiers == optionKey))
	{
		// value depends on how much the mouse moved
		double ratio = 1.;
		if (modifiers & controlKey)
			ratio = 50.;
		else if (modifiers & optionKey)
			ratio = 10.;
		mX += (mouse.x-from.x) / (bounds.Width() * ratio);
		mY -= (mouse.y-from.y) / (bounds.Height() * ratio);
	}
	else
	{
		// value is directly proportional to position
		mX = mouse.x / bounds.Width();
		mY = 1. - mouse.y / bounds.Height();
	}
	if (mX < 0.) mX = 0.;
	if (mX > 1.) mX = 1.;
	if (mY < 0.) mY = 0.;
	if (mY > 1.) mY = 1.;
	SetValue(!GetValue());
	from = mouse;
    return noErr;
}

OSStatus TXYControl::SetData(OSType inTag, ControlPartCode inPart, Size inSize, const void* inPtr)
{
	OSStatus err = noErr;
	switch (inTag)
	{
		case selectorX:
			if (inSize == sizeof(mX))
			{
				mX = *(double *)inPtr;
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
		case selectorY:
			if (inSize == sizeof(mY))
			{
				mY = *(double *)inPtr;
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

OSStatus TXYControl::GetData(OSType inTag, ControlPartCode inPart, Size inSize, Size* outSize, void* inPtr)
{
	OSStatus err = noErr;
	switch (inTag)
	{
		case selectorX:
			if (inSize == sizeof(mX))
				*(double*)inPtr = mX;
			else
				err = errDataSizeMismatch;
			break;
		case selectorY:
			if (inSize == sizeof(mY))
				*(double*)inPtr = mY;
			else
				err = errDataSizeMismatch;
			break;
			
		default:
			err = TViewNoCompositingCompatible::GetData(inTag, inPart, inSize, outSize, inPtr);
	}
	return err;
}

