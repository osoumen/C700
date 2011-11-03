/*
 *  TPMTexturedCell.h
 *
 *  Created by Peter Mark on Mon Dec 03 2004.
 *  Copyright (c) 2004 Peter Mark. All rights reserved.
 *
 */

#if !defined(_TPMTexturedCell_h_)
#define _TPMTexturedCell_h_

#include "TViewNoCompositingCompatible.h"

/*!
 * \class TPMTexturedCell
 * \brief A buttoncell control for use within a radiogroup,
 * that looks like a Cocoa Textured ButtonCell
 */

class TPMTexturedCell : public TViewNoCompositingCompatible
{
	friend class TViewNoCompositingCompatible;
public:
	static CFStringRef GetControlName() { return CFSTR("TextureCell"); }
protected:
	// Contstructor/Destructor
	TPMTexturedCell(HIViewRef inControl);
	virtual ~TPMTexturedCell();
	
	virtual ControlKind GetKind();
	
	virtual OSStatus Initialize(TCarbonEvent& inEvent);
	virtual void CompatibleDraw(RgnHandle inLimitRgn, CGContextRef inContext, bool inCompositing);
	virtual void TitleChanged();
	virtual void ValueChanged();
	
	virtual OSStatus SetData(OSType inTag, ControlPartCode inPart, Size inSize, const void* inPtr);
	virtual OSStatus GetData(OSType inTag, ControlPartCode inPart, Size inSize, Size* outSize, void* inPtr);
	
private:
	CFArrayRef mImages;
	CFStringRef mTitle;
	ControlFontStyleRec mTextStyle;
	float mJustInset;
};

#endif _TPMTexturedCell_h_
