/*
 *  TImagePopup.cpp
 *  Ritmo
 *
 *  Created by Chris Reed on Mon Feb 17 2003.
 *  Copyright (c) 2003 Chris Reed. All rights reserved.
 *
 */

#include "TImagePopup.h"
#include "TImageCache.h"
#include "AUGUIUtilities.h"

AUGUIProperties(TImagePopup) = {
	AUGUI::property_t()
};
AUGUIInit(TImagePopup);


using namespace AUGUI;

const ControlKind kImagePopupControlKind = { 'airy', 'popb' };

TImagePopup::TImagePopup(HIViewRef inControl)
	: TViewNoCompositingCompatible(inControl), mBackImage(0), mPopupMenu(0), mSelectedItemText(0), mLeftInset(0.0), mRightInset(0.0), mFilenameBase(0), mUseValueImages(false), mValueImageCache(0)
{
    ChangeAutoInvalidateFlags(kAutoInvalidateOnActivate | kAutoInvalidateOnEnable | kAutoInvalidateOnHilite, 0);
	
	// init text style
	memset(&mTextStyle, 0, sizeof(mTextStyle));
	mTextStyle.font = kControlFontSmallSystemFont;
	mTextStyle.just = teJustLeft; // default anyway, since it's equal to 0
}

//	Clean up after yourself.
TImagePopup::~TImagePopup()
{
    if (mBackImage)
		CFRelease(mBackImage);
	if (mPopupMenu)
		ReleaseMenu(mPopupMenu);
	if (mSelectedItemText)
		CFRelease(mSelectedItemText);
	if (mValueImageCache)
		CFRelease(mValueImageCache);
	if (mFilenameBase)
		CFRelease(mFilenameBase);
}

ControlKind TImagePopup::GetKind()
{
    return kImagePopupControlKind;
}

// The control is set up.  Do the last minute stuff that needs to be done
// like installing EventLoopTimers.
OSStatus TImagePopup::Initialize(TCarbonEvent& inEvent)
{
#pragma unused(inEvent)
    TitleChanged();
    return noErr;
}

//! Search for the image file to use for the current control value, using
//! the image cache dictionary to speed up the operation. The filename used
//! is the base name specified in the control title plus the value, like
//! "<base>5.png".
CGImageRef TImagePopup::GetImageForValue()
{
	// create the cache if it is not already in existence
	if (!mValueImageCache)
	{
		mValueImageCache = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		if (!mValueImageCache)
			return NULL;
	}
	
	// create a key for the current control value
	int value = GetValue() - 1;
	CFNumberRef valueKey = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &value);
	if (!valueKey)
		return NULL;
	
	// look in the cache dictionary
	CGImageRef valueImage = (CGImageRef)CFDictionaryGetValue(mValueImageCache, valueKey);
	if (!valueImage)
	{
		// image not in dictionary, so load it from disk
		CFStringRef searchFilename = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%@%d"), mFilenameBase, value);
		CGImageRef searchImage = TImageCache::GetImage(mBundleRef, searchFilename, CFSTR("png"), NULL);
		CFRelease(searchFilename);
		
		if (searchImage)
		{
			CFDictionarySetValue(mValueImageCache, valueKey, searchImage);
			valueImage = searchImage;
		}
	}
	
	CFRelease(valueKey);
	return valueImage;
}

//! Here we update the \c mSelectedItemText member variable and then force
//! the whole control to be redrawn the next time through the event loop.
void TImagePopup::ValueChanged()
{
	// release old value
	if (mSelectedItemText)
		CFRelease(mSelectedItemText);
	mSelectedItemText = 0;
	
	if (mPopupMenu)
	{
		CopyMenuItemTextAsCFString(mPopupMenu, (MenuItemIndex)GetValue(), &mSelectedItemText);
	}
	
	// force a redraw
    Invalidate();
}

//	The title format is
//	 	filename leftInset rightInset
//
// The filename is mandatory
void TImagePopup::TitleChanged()
{
    if (!mBundleRef)
		return;
		
	CFStringRef infoString;
	CopyControlTitleAsCFString(GetViewRef(), &infoString);
	
	if (infoString == NULL)
		return;
		
	CFArrayRef info = CFStringCreateArrayBySeparatingStrings(kCFAllocatorDefault, infoString, CFSTR(" "));
	CFIndex count = CFArrayGetCount(info);

	// filename
	if (count >= 1)
	{
		CFStringRef filename = (CFStringRef)CFArrayGetValueAtIndex(info, 0);
		if (CFStringGetLength(filename)) {
			mBackImage = TImageCache::GetImage(mBundleRef, filename, CFSTR("png"), NULL);
			mFilenameBase = filename;
			CFRetain(mFilenameBase);
		}
	}
	
	// left inset
	if (count >= 2)
		mLeftInset = CFStringGetDoubleValue((CFStringRef)CFArrayGetValueAtIndex(info, 1));
	else
		mLeftInset = 0.0;

	// right inset
	if (count >= 3)
		mRightInset = CFStringGetDoubleValue((CFStringRef)CFArrayGetValueAtIndex(info, 2));
	else
		mRightInset = 0.0;

	// text justification
	if (count >= 4)
		mTextStyle.just = CFStringGetIntValue((CFStringRef)CFArrayGetValueAtIndex(info, 3));

	// clean up
	CFRelease(info);
	CFRelease(infoString);
	
	// force a redraw
	Invalidate();
}

OSStatus TImagePopup::SetData(OSType inTag, ControlPartCode inPart, Size inSize, const void* inPtr)
{
	OSStatus err = noErr;
	switch (inTag)
	{
		case kControlPopupButtonMenuRefTag:
			if (inSize == sizeof(MenuRef))
			{
				MenuRef theMenu = *(MenuRef *)inPtr;
				if (theMenu != mPopupMenu)
				{
					if (mPopupMenu)
						ReleaseMenu(mPopupMenu);
					mPopupMenu = theMenu;
				
					// add the menu to hierarchical part of the menu list
					InsertMenu(mPopupMenu, hierMenu);
					
					// re-read the value
					ValueChanged();
				}
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
		
		case kImagePopupControlUseImagesNotTextTag:
			if (inSize == sizeof(UInt32))
			{
				bool useIt = *(UInt32 *)inPtr;
				if (useIt != mUseValueImages)
				{
					mUseValueImages = useIt;
					Invalidate();
				}
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

OSStatus TImagePopup::GetData(OSType inTag, ControlPartCode inPart, Size inSize, Size* outSize, void* inPtr)
{
	OSStatus err = noErr;
	if (outSize)
		*outSize = 0;
	switch (inTag)
	{
		case kControlPopupButtonMenuRefTag:
			if (inSize == sizeof(MenuRef))
			{
				if (inPtr)
					*(MenuRef *)inPtr = mPopupMenu;
				if (outSize)
					*outSize = sizeof(MenuRef);
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
			
		case kImagePopupControlUseImagesNotTextTag:
			if (inSize == sizeof(UInt32))
			{
				if (inPtr)
					*(UInt32 *)inPtr = mUseValueImages;
				if (outSize)
					*outSize = sizeof(UInt32);
			}
			else
				err = errDataSizeMismatch;
			break;
			
		default:
			err = TViewNoCompositingCompatible::GetData(inTag, inPart, inSize, outSize, inPtr);
	}
	return err;
}


// The fun part of the control
void TImagePopup::CompatibleDraw(RgnHandle inLimitRgn, CGContextRef inContext, bool inCompositing)
{
#pragma unused(inLimitRgn)
	TRect bounds = Bounds();
	ControlPartCode hilite = GetHilite();
	
	if (mBackImage)
	{
		HIViewDrawCGImage(inContext, &bounds, mBackImage);
	}
	
	if (mUseValueImages)
	{
		// composite an image for the value
		CGImageRef valueImage = GetImageForValue();
		if (valueImage)
			HIViewDrawCGImage(inContext, &bounds, valueImage);
	}
	else if (mSelectedItemText)
	{
		// draw text for the value
		ThemeDrawState state = kThemeStateActive;
		if (hilite == kControlInactivePart || !IsEnabled())
			state = kThemeStateInactive;

		RGBColor textColor;
		GetThemeTextColor(kThemeTextColorPushButtonInactive, 32, true, &textColor);
		RGBForeColor(&textColor);
		
		Point textDimensions;
		SInt16 baseline;
		GetThemeTextDimensions(mSelectedItemText, kThemeSmallSystemFont, state, false, &textDimensions, &baseline);
		
		TRect textBounds = Bounds();
		textBounds.SetHeight(textDimensions.v);
		textBounds.MoveBy(mLeftInset, (bounds.Height() - textBounds.Height()) / 2.0);

		textBounds.SetSize(textBounds.Width() - mLeftInset - mRightInset, textBounds.Height());

		Rect qdBounds = textBounds;
		DrawThemeTextBox(mSelectedItemText, kThemeSmallSystemFont, state, false, &qdBounds, mTextStyle.just, inContext);
	}
}

//! Just force a redraw.
OSStatus TImagePopup::HiliteChanged(ControlPartCode inOriginalPart, ControlPartCode inCurrentPart, RgnHandle inInvalRgn)
{
	Invalidate();
	return noErr;
}

OSStatus TImagePopup::Track(TCarbonEvent& inEvent, ControlPartCode* outPart)
{
	TRect bounds = Bounds();
	int mini = GetMinimum();
	int maxi = GetMaximum();
	if (mPopupMenu)
	{
		// convert the top left for the popup menu to global coordinates
		HIPoint pt;
		pt.x = lrint(bounds.MinX() + mLeftInset);
		pt.y = lrint(bounds.MinY());
		ConvertToGlobal(pt);
		Point p;
		p.h = int(pt.x);
		p.v = int(pt.y);
		ConvertWindowPointToGlobal(GetOwner(), p);
		
		// save menu's previous font
		SInt16 saveFontID;
		UInt16 saveFontSize;
		GetMenuFont(mPopupMenu, &saveFontID, &saveFontSize);
		
		// set to theme font
		Str255 themeFontName;
		SInt16 themeFontID;
		SInt16 themeFontSize;
		Style themeFontStyle;
		GetThemeFont(kThemeSmallSystemFont, smSystemScript, themeFontName, &themeFontSize, &themeFontStyle);
		GetFNum(themeFontName, &themeFontID);
		SetMenuFont(mPopupMenu, themeFontID, themeFontSize);
		
		// display the popup menu
		int result = PopUpMenuSelect(mPopupMenu, p.v-themeFontSize, p.h, GetValue());

		// restore menu's previous font
		SetMenuFont(mPopupMenu, saveFontID, saveFontSize);
		
		// was an item selected?
		if (result != 0)
		{
			int selectedItem = result & 0xffff;
			if (selectedItem < mini)
				selectedItem = mini;
			else if (selectedItem > maxi)
				selectedItem = maxi;
			
			// only set value if it's different
			if (selectedItem != GetValue())
				SetValue(selectedItem);
		}
		*outPart = kControlMenuPart;
	} else  {
		*outPart = kControlNoPart;
	}
	return noErr;
}

//! Asks your view to return what part of itself (if any) is hit by the point
//! given to it. The point is in VIEW coordinates, so you should get the view
//! srect to do bounds checking.
ControlPartCode TImagePopup::HitTest(const HIPoint& inWhere)
{
	ControlPartCode part;

    // is the mouse on the button?
	if (CGRectContainsPoint(Bounds(), inWhere))
		part = kControlMenuPart;
	else
		part = kControlNoPart;

    return part;
}
