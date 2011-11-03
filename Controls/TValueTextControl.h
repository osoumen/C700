/*
 *  TValueTextControl.h
 *  Ritmo
 *
 *  Created by Chris Reed on Mon Feb 03 2003.
 *  Copyright (c) 2003 Chris Reed. All rights reserved.
 *
 */

#if !defined(_TValueTextControl_h_)
#define _TValueTextControl_h_

#include "TViewNoCompositingCompatible.h"

/**
* @class TValueTextControl
 *
 * This HIView control class works like the TValuePictControl except that
 * it uses a single background image and draws a text string for each
 * distinct value. The text strings come from a CFArray of CFStrings set
 * through the SetData method.
 *
 * The title contains the name of the background image, without the file
 * type extension of .png. It can also have optional configuration values
 * such as the offset for the baseline of the text. Its format is as
 * follows:
 *
 *		filename [leftInset] [bottomInset]
 *
 * Where leftInset is the inset in pixels from the left side of the image
 * that the text should be drawn and bottomInset in the inset from the
 * bottom edge for the baseline of the text. Values are separated by a
 * single space character.
 */

class TValueTextControl : public TViewNoCompositingCompatible
{
	AUGUIDefineControl(TValueTextControl, TViewNoCompositingCompatible, "valtext");
protected:
	// Constructor/Destructor
	TValueTextControl(HIViewRef inControl);
	virtual ~TValueTextControl();

	virtual ControlKind GetKind();

	virtual bool UseNonblockingTracking() { return false; }

	virtual OSStatus Initialize(TCarbonEvent& inEvent);
	virtual OSStatus		StillTracking(TCarbonEvent&inEvent, HIPoint& from);
	virtual void CompatibleDraw(RgnHandle inLimitRgn, CGContextRef inContext, bool inCompositing);
	virtual void TitleChanged();
	virtual void ValueChanged();
	virtual UInt32 GetBehaviors() { return TViewNoCompositingCompatible::GetBehaviors() | kControlSupportsEmbedding ; }

private:
	CGImageRef mThumbImage;
    CGImageRef mBackImage;
	float mXInset;
	float mTopInset;
	float mBottomInset;
};

#endif _TValueTextControl_h_
