/*
 *  TImageButton.h
 *  Ritmo
 *
 *  Created by Chris Reed on Mon Feb 03 2003.
 *  Copyright (c) 2003 Chris Reed. All rights reserved.
 *
 */

#if !defined(_TImageButton_h_)
#define _TImageButton_h_

#include "TViewNoCompositingCompatible.h"

/*!
 * \class TImageButton
 * \brief A pushbutton control with configurable images for each value.
 * 
 * This HIView control subclass works like a standard pushbutton, but
 * supports the use of any image or images for its various states. In
 * addition, it supports drawing a text string over the image for a title.
 *
 * Because the control's actual title is used to configure the image file
 * names, the text to be drawn as the visible title must be set using
 * SetControlData with the data tag kControlEditTextCFStringTag or
 * kControlStaticTextCFStringTag.
 *
 * The text will be drawn in the small system font by default. You can
 * change the text style by setting the kControlFontStyleTag data, a
 * structure of type ControlFontStyleRec.
 */

class TImageButton : public TViewNoCompositingCompatible
{
	AUGUIDefineControl(TImageButton, TViewNoCompositingCompatible, "button");
protected:
	// Constructor/Destructor
	TImageButton(HIViewRef inControl);
	virtual ~TImageButton();
	
	virtual ControlKind GetKind();
	
	virtual OSStatus Initialize(TCarbonEvent& inEvent);
	virtual void CompatibleDraw(RgnHandle inLimitRgn, CGContextRef inContext, bool inCompositing);
        virtual OSStatus ControlHit(ControlPartCode inPart, UInt32 inModifiers );
            virtual OSStatus Track(TCarbonEvent& inEvent, ControlPartCode* outPart);
	virtual OSStatus HiliteChanged(ControlPartCode inOriginalPart, ControlPartCode inCurrentPart, RgnHandle inInvalRgn);
	virtual void TitleChanged();
	virtual void ValueChanged();
        virtual UInt32 GetBehaviors() { return TViewNoCompositingCompatible::GetBehaviors() | kControlSupportsEmbedding ; };
	
	virtual OSStatus SetData(OSType inTag, ControlPartCode inPart, Size inSize, const void* inPtr);
	virtual OSStatus GetData(OSType inTag, ControlPartCode inPart, Size inSize, Size* outSize, void* inPtr);

private:
	CFArrayRef mImages;
	CFStringRef mTitle;
	ControlFontStyleRec mTextStyle;
	float mJustInset;
};

#endif _TImageButton_h_
