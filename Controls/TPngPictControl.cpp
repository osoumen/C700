/*
 *  TPngPictControl.cpp
 *  NeuSynth
 *
 *  Created by Airy André on Sun Oct 20 2002.
 *  Copyright (c) 2002 Airy André. All rights reserved.
 *
 */

#include "TPngPictControl.h"
#include "TImageCache.h"

AUGUIProperties(TPngPictControl) = {
	AUGUI::property_t()
};
AUGUIInit(TPngPictControl);

// -----------------------------------------------------------------------------
//	TPngPictControl constructor
// -----------------------------------------------------------------------------
//
TPngPictControl::TPngPictControl(
								 HIViewRef			inControl )
:	TViewNoCompositingCompatible( inControl )
{
	//    ChangeAutoInvalidateFlags( kAutoInvalidateOnActivate | kAutoInvalidateOnEnable, 0 );
    mImage = 0;
}

// -----------------------------------------------------------------------------
//	TPngPictControl destructor
// -----------------------------------------------------------------------------
//	Clean up after yourself.
//
TPngPictControl::~TPngPictControl()
{
    if (mImage)
		CGImageRelease(mImage);
}

// -----------------------------------------------------------------------------
//	GetKind
// -----------------------------------------------------------------------------
//
ControlKind TPngPictControl::GetKind()
{
    const ControlKind kMyKind = { 'airy', 'png ' };
    return kMyKind;
}

//-----------------------------------------------------------------------------------
//	Initialize
//-----------------------------------------------------------------------------------
//	The control is set up.  Do the last minute stuff that needs to be done like
//	installing EventLoopTimers.
//
OSStatus TPngPictControl::Initialize(
									 TCarbonEvent&		inEvent )
{
#pragma unused( inEvent )
#if 0
    UInt32 val;
    OSStatus err =    inEvent.GetParameter('dumm', &val);
    printf("%d : dumm = %d\n", err, val);
#endif
    TitleChanged();
    return noErr;
}
//-----------------------------------------------------------------------------------
//	TitleChanged
//-----------------------------------------------------------------------------------
void TPngPictControl::TitleChanged()
{
    if (mImage)
		CGImageRelease(mImage);
    if ( mBundleRef )
    {
		CFStringRef fileName;
		CopyControlTitleAsCFString(GetViewRef(), &fileName);
                char buffer[100];
                CFStringGetCString(fileName, buffer, 100, kCFStringEncodingASCII);
                if ( fileName != NULL )
		{
			mImage = TImageCache::GetImage( mBundleRef, fileName, NULL, NULL );
			CFRelease( fileName );
			
			FlushStaticBackground();
		}
    }
}

//-----------------------------------------------------------------------------------
//	Draw
//-----------------------------------------------------------------------------------
//	The fun part of the control
//
void TPngPictControl::DrawStaticBackground(
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
//-----------------------------------------------------------------------------------
//	Draw
//-----------------------------------------------------------------------------------
//	The fun part of the control
//
void TPngPictControl::CompatibleDraw(
									 RgnHandle				inLimitRgn,
									 CGContextRef			inContext,
									 bool  inCompositing)
{
	// Nothing to do : only has a static part, drawn in DrawStaticBackground
}

UInt32 TPngPictControl::GetBehaviors()
{
    return TViewNoCompositingCompatible::GetBehaviors() | kControlSupportsEmbedding;
}

OSStatus TPngPictControl::SetData(OSType inTag, ControlPartCode inPart, Size inSize, const void* inPtr)
{
	OSStatus err = noErr;
	switch (inTag)
	{
		case kPngPictControlCGImageTag:
			if (inSize == sizeof(CGImageRef))
			{
				if (mImage != *(CGImageRef *)inPtr) {
					if (mImage)
						CGImageRelease(mImage);
					mImage = *(CGImageRef *)inPtr;
					if (mImage)
						CGImageRetain(mImage);
					FlushStaticBackground();
				}
			}
			else
			{
				err = errDataSizeMismatch;
			}
			break;

		default:
			err = TViewNoCompositingCompatible::SetData(inTag, inPart, inSize, inPtr);
	}
	return err;
}

OSStatus TPngPictControl::GetData(OSType inTag, ControlPartCode inPart, Size inSize, Size* outSize, void* inPtr)
{
	OSStatus err = noErr;
	switch (inTag)
	{
		case kPngPictControlCGImageTag:
			if (inSize == sizeof(CGImageRef))
			{
				if (inPtr)
					*(CGImageRef *)inPtr = mImage;
				if (outSize)
					*outSize = sizeof(CGImageRef);
			}
			else
				err = errDataSizeMismatch;
			break;

		default:
			err = TViewNoCompositingCompatible::GetData(inTag, inPart, inSize, outSize, inPtr);
	}
	return err;
}

