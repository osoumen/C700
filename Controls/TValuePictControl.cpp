/*
 *  TValuePictControl.cpp
 *  NeuSynth
 *
 *  Created by Airy André on Sun Oct 20 2002.
 *  Copyright (c) 2002 Airy André. All rights reserved.
 *
 */

#include "TValuePictControl.h"
#include "TImageCache.h"

AUGUIProperties(TValuePictControl) = {
				AUGUI::property_t('pict', CFSTR("picture"), CFSTR("Picture"), AUGUI::kString),
				AUGUI::property_t()
};
AUGUIInit(TValuePictControl);

// -----------------------------------------------------------------------------
//	TValuePictControl constructor
// -----------------------------------------------------------------------------
//
TValuePictControl::TValuePictControl(
									 HIViewRef			inControl )
:	TViewNoCompositingCompatible( inControl )
{
	//    ChangeAutoInvalidateFlags( kAutoInvalidateOnActivate | kAutoInvalidateOnEnable, 0 );
    mImage = 0;
    mImageCache = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
}

// -----------------------------------------------------------------------------
//	TValuePictControl destructor
// -----------------------------------------------------------------------------
//	Clean up after yourself.
//
TValuePictControl::~TValuePictControl()
{
    if (mImage)
		CGImageRelease(mImage);
    if (mImageCache)
		CFRelease(mImageCache);
}

// -----------------------------------------------------------------------------
//	GetKind
// -----------------------------------------------------------------------------
//
ControlKind TValuePictControl::GetKind()
{
    const ControlKind kMyKind = { 'airy', 'valp' };

    return kMyKind;
}


//-----------------------------------------------------------------------------------
//	Initialize
//-----------------------------------------------------------------------------------
//	The control is set up.  Do the last minute stuff that needs to be done like
//	installing EventLoopTimers.
//
OSStatus TValuePictControl::Initialize(
									   TCarbonEvent&		inEvent )
{
#pragma unused( inEvent )
	ValueChanged();
    return noErr;
}
//-----------------------------------------------------------------------------------
//	TitleChanged
//-----------------------------------------------------------------------------------
void TValuePictControl::TitleChanged()
{
    CFDictionaryRemoveAllValues(mImageCache);
    ValueChanged();
}
//-----------------------------------------------------------------------------------
//	ValueChanged
//-----------------------------------------------------------------------------------
void TValuePictControl::ValueChanged()
{
    if (mImage)
		CGImageRelease(mImage);

    SInt32 value = GetValue();
    CFNumberRef number = CFNumberCreate(NULL, kCFNumberIntType, &value);
    if (CFDictionaryGetValueIfPresent(mImageCache, number, (const void **)&mImage)) {
		CFRetain(mImage);
    } else if (mBundleRef) {
		char buffer[100];
		char name[100];
		CFStringRef fileName;
		CopyControlTitleAsCFString(GetViewRef(), &fileName);
		CFStringGetCString(fileName, buffer, 100, kCFStringEncodingASCII);
		sprintf(name, "%s%ld.png", buffer, GetValue());
		CFStringRef pict = CFStringCreateWithCStringNoCopy(0, name, kCFStringEncodingASCII, 0);
		mImage = TImageCache::GetImage(mBundleRef, pict, NULL, NULL);
		if (mImage) {
			CFDictionaryAddValue(mImageCache, number, mImage);
		}
    }

    CFRelease(number);
    Invalidate();
}

//-----------------------------------------------------------------------------------
//	Draw
//-----------------------------------------------------------------------------------
//	The fun part of the control
//
void TValuePictControl::CompatibleDraw(
									   RgnHandle				inLimitRgn,
									   CGContextRef			inContext,
									   bool  inCompositing)
{
#pragma unused( inLimitRgn )
    if (mImage) {
		HIRect bounds = Bounds();
		HIViewDrawCGImage( inContext, &bounds, mImage);
    }
}

// -----------------------------------------------------------------------------
//	StillTracking
// -----------------------------------------------------------------------------
//	Called during tracking.
//
OSStatus TValuePictControl::StillTracking(TCarbonEvent&inEvent, HIPoint& from)
{
    HIPoint mouse;
    float deltaX, deltaY;
    SInt32 mini, maxi;

    mini = GetMinimum();
    maxi = GetMaximum();

    inEvent.GetParameter<HIPoint>( kEventParamWindowMouseLocation, typeHIPoint, &mouse );
    ConvertToLocal(mouse);

    SInt32 curVal = GetValue();
    deltaX = mouse.x-from.x;
    deltaY = mouse.y-from.y;

    SInt32 val = SInt32(rint(curVal + (deltaX - deltaY) * (maxi - mini) / 300.));
    if ( val > maxi) val = maxi;
    if ( val < mini) val = mini;
    if (val != curVal) {
		SetValue ( val );
		from = mouse;
    }
    return noErr;
}


