/*
 *  TImagePopup.h
 *  Ritmo
 *
 *  Created by Chris Reed on Mon Feb 17 2003.
 *  Copyright (c) 2003 Chris Reed. All rights reserved.
 *
 */

#include <Carbon/Carbon.h>

#if !defined(_TImagePopup_h_)
#define _TImagePopup_h_

#include "TViewNoCompositingCompatible.h"

//! A CFArrayRef array of CGImageRef images to use for the values instead
//! of drawing text strings.
#define kImagePopupControlValueImageDictTag FOUR_CHAR_CODE('VImg')

//! A CFArrayRef containing CFStringRefs that are the filenames of image
//! files 
#define kImagePopupControlValueImageFilenameDictTag FOUR_CHAR_CODE('VImF')

//! An integer boolean value of type UInt32 indicating that we want to
//! draw images for each distinct value and not draw text over a background.
#define kImagePopupControlUseImagesNotTextTag FOUR_CHAR_CODE('I!Tx')

/*!
 * This HIView control subclass is basically a popup menu button with
 * support for a background image read from a file. The string value of the
 * selected menu item will be drawn over the background image.
 *
 * The text will be drawn in the small system font by default. You can
 * change the text style by setting the \c kControlFontStyleTag data, a
 * structure of type \c ControlFontStyleRec.
 */

class TImagePopup : public TViewNoCompositingCompatible
{
	AUGUIDefineControl(TImagePopup, TViewNoCompositingCompatible, "popup");
protected:
	// Constructor/Destructor
	TImagePopup(HIViewRef inControl);
	virtual ~TImagePopup();
	
	virtual ControlKind GetKind();
	
	virtual OSStatus Initialize(TCarbonEvent& inEvent);
	virtual ControlPartCode HitTest(const HIPoint&	inWhere);
	virtual void CompatibleDraw(RgnHandle inLimitRgn, CGContextRef inContext, bool inCompositing);
	virtual OSStatus HiliteChanged(ControlPartCode inOriginalPart, ControlPartCode inCurrentPart, RgnHandle inInvalRgn);
	virtual OSStatus Track(TCarbonEvent& inEvent, ControlPartCode* outPart);
	virtual void TitleChanged();
	virtual void ValueChanged();
	
	virtual OSStatus SetData(OSType inTag, ControlPartCode inPart, Size inSize, const void* inPtr);
	virtual OSStatus GetData(OSType inTag, ControlPartCode inPart, Size inSize, Size* outSize, void* inPtr);

private:
	CGImageRef mBackImage;
	MenuRef mPopupMenu;
	CFStringRef mSelectedItemText;
	ControlFontStyleRec mTextStyle;
	float mLeftInset;
	float mRightInset;
	CFStringRef mFilenameBase;
	bool mUseValueImages;
	CFMutableDictionaryRef mValueImageCache;
	
	CGImageRef GetImageForValue();
};

#endif _TImagePopup_h_
