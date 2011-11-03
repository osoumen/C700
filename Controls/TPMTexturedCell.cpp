/*
 *  TPMTexturedCell.cpp
 *
 *  Created by Peter Mark on Mon Dec 03 2004.
 *  Copyright (c) 2004 Peter Mark. All rights reserved.
 *
 */

#include "TPMTexturedCell.h"
#include "TImageCache.h"
#include "AUGUIUtilities.h"

using namespace AUGUI;

const ControlKind kImageButtonKind = { '  PM', 'txbt' };

TPMTexturedCell::TPMTexturedCell(HIViewRef inControl)
	: TViewNoCompositingCompatible(inControl), mImages(0), mTitle(0), mJustInset(5.0f)
{
    ChangeAutoInvalidateFlags(kAutoInvalidateOnActivate | kAutoInvalidateOnEnable | kAutoInvalidateOnHilite, 0);

	// init text style
	memset(&mTextStyle, 0, sizeof(mTextStyle));
	mTextStyle.font = kControlFontSmallSystemFont;
	mTextStyle.just = teJustCenter;
}

//	Clean up after yourself.
TPMTexturedCell::~TPMTexturedCell()
{
    if (mImages)
		CFRelease(mImages);
	if (mTitle)
		CFRelease(mTitle);
}

ControlKind TPMTexturedCell::GetKind()
{
    return kImageButtonKind;
}

// The control is set up.  Do the last minute stuff that needs to be done
// like installing EventLoopTimers.
OSStatus TPMTexturedCell::Initialize(TCarbonEvent& inEvent)
{
#pragma unused(inEvent)
    if (!mBundleRef)
		return 1;
	
	CFStringRef infoString;
	CopyControlTitleAsCFString(GetViewRef(), &infoString);
	
	if (!infoString)
		return 1;
	
	if (mImages)
		CFRelease(mImages);
	
	CFMutableArrayRef images = CFArrayCreateMutable(kCFAllocatorDefault, 12, &kCFTypeArrayCallBacks);
	mImages = images;
	
	CGImageRef thisImage;
	thisImage = TImageCache::GetImage(mBundleRef, CFSTR("MetalNorL"), CFSTR("png"), NULL);
	if (thisImage) CFArrayAppendValue(images, thisImage);
	CFRelease(thisImage);
	thisImage = TImageCache::GetImage(mBundleRef, CFSTR("MetalNorC"), CFSTR("png"), NULL);
	if (thisImage) CFArrayAppendValue(images, thisImage);
	CFRelease(thisImage);
	thisImage = TImageCache::GetImage(mBundleRef, CFSTR("MetalNorR"), CFSTR("png"), NULL);
	if (thisImage) CFArrayAppendValue(images, thisImage);
	CFRelease(thisImage);
	
	thisImage = TImageCache::GetImage(mBundleRef, CFSTR("MetalSelL"), CFSTR("png"), NULL);
	if (thisImage) CFArrayAppendValue(images, thisImage);
	CFRelease(thisImage);
	thisImage = TImageCache::GetImage(mBundleRef, CFSTR("MetalSelC"), CFSTR("png"), NULL);
	if (thisImage) CFArrayAppendValue(images, thisImage);
	CFRelease(thisImage);
	thisImage = TImageCache::GetImage(mBundleRef, CFSTR("MetalSelR"), CFSTR("png"), NULL);
	if (thisImage) CFArrayAppendValue(images, thisImage);
	CFRelease(thisImage);

	thisImage = TImageCache::GetImage(mBundleRef, CFSTR("SmlMetNorL"), CFSTR("png"), NULL);
	if (thisImage) CFArrayAppendValue(images, thisImage);
	CFRelease(thisImage);
	thisImage = TImageCache::GetImage(mBundleRef, CFSTR("SmlMetNorC"), CFSTR("png"), NULL);
	if (thisImage) CFArrayAppendValue(images, thisImage);
	CFRelease(thisImage);
	thisImage = TImageCache::GetImage(mBundleRef, CFSTR("SmlMetNorR"), CFSTR("png"), NULL);
	if (thisImage) CFArrayAppendValue(images, thisImage);
	CFRelease(thisImage);

	thisImage = TImageCache::GetImage(mBundleRef, CFSTR("SmlMetSelL"), CFSTR("png"), NULL);
	if (thisImage) CFArrayAppendValue(images, thisImage);
	CFRelease(thisImage);
	thisImage = TImageCache::GetImage(mBundleRef, CFSTR("SmlMetSelC"), CFSTR("png"), NULL);
	if (thisImage) CFArrayAppendValue(images, thisImage);
	CFRelease(thisImage);
	thisImage = TImageCache::GetImage(mBundleRef, CFSTR("SmlMetSelR"), CFSTR("png"), NULL);
	if (thisImage) CFArrayAppendValue(images, thisImage);
	CFRelease(thisImage);

    TitleChanged();
    return noErr;
}

void TPMTexturedCell::ValueChanged()
{
    Invalidate();
}

//	The title format is
//	 	filename realTitle justInset
//
// The filename is mandatory
// Integer values are appended to the filename and image files are read
// until a file cannot be found with that name. For example, if the
// filename is "foo", then "foo1", "foo2", etc will be attempted. Once a
// file cannot be found with that name, the search is canceled.
void TPMTexturedCell::TitleChanged()
{
		
	
	CFStringRef infoString;
	CopyControlTitleAsCFString(GetViewRef(), &infoString);

	// title
		CFMutableStringRef title = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, infoString);

		if (mTitle)
		{
			CFRelease(mTitle);
			mTitle = NULL;
		}
		
		// only use the title if it's not empty
		if (CFStringGetLength(title) > 0)
			mTitle = title;
		else
			CFRelease(title);
	
	
	// inset
		mJustInset = 0;

	// clean up
	CFRelease(infoString);

	// force a redraw
	Invalidate();
}

OSStatus TPMTexturedCell::SetData(OSType inTag, ControlPartCode inPart, Size inSize, const void* inPtr)
{
	OSStatus err = noErr;
	switch (inTag)
	{
		case kControlEditTextCFStringTag:
			if (inSize == sizeof(CFStringRef))
			{
				if (mTitle)
				{
					CFRelease(mTitle);
					mTitle = NULL;
				}
				mTitle = *(CFStringRef *)inPtr;
                                CFRetain(mTitle);
				Invalidate();
			}
			else
				err = errDataSizeMismatch;
			break;

		case kControlFontStyleTag:
			if (inSize == sizeof(ControlFontStyleRec))
			{
				memcpy(&mTextStyle, inPtr, inSize);
				Invalidate();
			}
			else
				err = errDataSizeMismatch;
			break;

		default:
			// unsupported data tag
			err = errDataNotSupported;
	}
	return err;
}

OSStatus TPMTexturedCell::GetData(OSType inTag, ControlPartCode inPart, Size inSize, Size* outSize, void* inPtr)
{
	OSStatus err = noErr;
	switch (inTag)
	{
		case kControlEditTextCFStringTag:
			if (inSize == sizeof(CFStringRef))
			{
				if (inPtr)
					*(CFStringRef *)inPtr = mTitle;
				if (outSize)
					*outSize = sizeof(CFStringRef);
			}
			else
				err = errDataSizeMismatch;
			break;

		case kControlFontStyleTag:
			if (inSize == sizeof(ControlFontStyleRec))
			{
				if (inPtr)
					memcpy(inPtr, &mTextStyle, inSize);
				if (outSize)
					*outSize = sizeof(ControlFontStyleRec);
			}
			else
				err = errDataSizeMismatch;
			break;

		default:
			err = errDataNotSupported;
	}
	return err;
}

//! The fun part of the control
void TPMTexturedCell::CompatibleDraw(RgnHandle inLimitRgn, CGContextRef inContext, bool inCompositing)
{
#pragma unused(inLimitRgn)
	TRect bounds = Bounds();
	int value = GetValue();
	ControlPartCode hilite = GetHilite();

	CGRect aRect=Bounds();
	if(aRect.size.height == 18){
		if(hilite == kControlButtonPart || value){
			aRect.size.width = 6;
			aRect.size.height = 18;
			HIViewDrawCGImage(inContext, &aRect, (CGImageRef)CFArrayGetValueAtIndex(mImages, 9));
			
			aRect.origin.x += 6;
			aRect.size.width=Bounds().size.width - 12;
			HIViewDrawCGImage(inContext, &aRect, (CGImageRef)CFArrayGetValueAtIndex(mImages, 10));

			aRect.origin.x = CGRectGetMaxX(Bounds()) - 6;
			aRect.size.width = 6;
			HIViewDrawCGImage(inContext, &aRect, (CGImageRef)CFArrayGetValueAtIndex(mImages, 11));

		}else{
			aRect.size.width = 6;
			aRect.size.height = 18;
			HIViewDrawCGImage(inContext, &aRect, (CGImageRef)CFArrayGetValueAtIndex(mImages, 6));
			
			aRect.origin.x += 6;
			aRect.size.width=Bounds().size.width - 12;
			HIViewDrawCGImage(inContext, &aRect, (CGImageRef)CFArrayGetValueAtIndex(mImages, 7));
			aRect.origin.x = CGRectGetMaxX(Bounds()) - 6;
			aRect.size.width = 6;
			HIViewDrawCGImage(inContext, &aRect, (CGImageRef)CFArrayGetValueAtIndex(mImages, 8));			
		}
	}else{
		if(hilite == kControlButtonPart || value){
			aRect.size.width = 10;
			aRect.size.height = 25;
			HIViewDrawCGImage(inContext, &aRect, (CGImageRef)CFArrayGetValueAtIndex(mImages, 3));
			
			aRect.origin.x += 10;
			aRect.size.width=Bounds().size.width - 20;
			HIViewDrawCGImage(inContext, &aRect, (CGImageRef)CFArrayGetValueAtIndex(mImages, 4));
			aRect.origin.x = CGRectGetMaxX(Bounds()) - 10;
			aRect.size.width = 10;
			HIViewDrawCGImage(inContext, &aRect, (CGImageRef)CFArrayGetValueAtIndex(mImages, 5));
		}else{
			aRect.size.width = 10;
			aRect.size.height = 25;
			HIViewDrawCGImage(inContext, &aRect, (CGImageRef)CFArrayGetValueAtIndex(mImages, 0));
			
			aRect.origin.x += 10;
			aRect.size.width=Bounds().size.width - 20;
			HIViewDrawCGImage(inContext, &aRect, (CGImageRef)CFArrayGetValueAtIndex(mImages, 1));
			aRect.origin.x = CGRectGetMaxX(Bounds()) - 10;
			aRect.size.width = 10;
			HIViewDrawCGImage(inContext, &aRect, (CGImageRef)CFArrayGetValueAtIndex(mImages, 2));
			
		}		
	}

	// draw title text
	if (mTitle)
	{
		ThemeDrawState state = kThemeStateActive;
		if (hilite == kControlInactivePart || !IsEnabled())
			state = kThemeStateInactive;

		CGContextSetRGBStrokeColor(inContext, 1.0f, 1.0f, 1.0f, 1.0f);
		CGContextSetRGBFillColor(inContext, 1.0f, 1.0f, 1.0f, 1.0f);
		
		SetFontFromFontStyleRec(inContext, mTextStyle, kThemeSmallSystemFont, kThemeTextColorPushButtonActive);
		
		// get string to draw
		int textLength = CFStringGetLength(mTitle);
		const char *textPtr = CFStringGetCStringPtr(mTitle, CFStringGetSystemEncoding());
		char *textBuffer = NULL;
		if (textPtr == NULL)
		{
			textBuffer = new char[textLength + 1];
			CFStringGetCString(mTitle, textBuffer, textLength+1, CFStringGetSystemEncoding());
			textPtr = textBuffer;
		}
		
		// measure string in the font
		Rect qdTextBounds;
		FontInfo fontInfo;
		QDTextBounds(textLength, textPtr, &qdTextBounds);
		GetFontInfo(&fontInfo);
		
		// get text bounding rect
		float textWidth = float(qdTextBounds.right - qdTextBounds.left);
		float textHeight = float(qdTextBounds.bottom - qdTextBounds.top);
		TRect textBounds = Bounds();
		textBounds.SetHeight(textHeight);
		textBounds.MoveBy(0.0f, (bounds.Height() - textBounds.Height()) / 2.0f - fontInfo.descent);
		
//		printf("text w = %g; h = %g\n", textWidth, textHeight);
		
		// apply justification
		bool useJust = mTextStyle.flags & kControlUseJustMask;
		int theJust = useJust ? mTextStyle.just : teJustCenter;
		switch (theJust)
		{
			case teJustLeft:
				textBounds.MoveBy(mJustInset, 0.0f);
				break;
			case teJustRight:
				textBounds.MoveBy(textBounds.Width() - mJustInset - textWidth, 0.0f);
				break;
			case teJustCenter:
			default:
				textBounds.MoveBy(mJustInset + (textBounds.Width() - mJustInset * 2.0f - textWidth) / 2.0f, 0.0f);
				break;
		}
		
		// draw text
		CGAffineTransform textMatrix = CGContextGetTextMatrix(inContext);
		textMatrix = CGAffineTransformScale(textMatrix, 1.0, -1.0);
		CGContextSetTextMatrix(inContext, textMatrix);
		
		CGContextSetTextDrawingMode(inContext, kCGTextFill);
		CGContextShowTextAtPoint(inContext, textBounds.MinX(), textBounds.MaxY(), textPtr, textLength);
		CGContextSetRGBStrokeColor(inContext, 0.0f, 0.0f, 0.0f, 1.0f);
		CGContextSetRGBFillColor(inContext, 0.0f, 0.0f, 0.0f, 1.0f);
		CGContextShowTextAtPoint(inContext, textBounds.MinX(), textBounds.MaxY()-1, textPtr, textLength);
		
		// clean up
		if (textBuffer)
			delete [] textBuffer;
	}
}
