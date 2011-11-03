/*
 *  TPngPictControl.h
 *  AUStk
 *
 *  Created by Airy André on Sun Oct 20 2002.
 *  Copyright (c) 2002 Airy André. All rights reserved.
 *
 */

#ifndef TPngPictControl_H_
#define TPngPictControl_H_

#include "TViewNoCompositingCompatible.h"

enum {
	//! A CGImageRef that you wish displayed in the control.
	kPngPictControlCGImageTag = 'Img.'
};

class TPngPictControl
: public TViewNoCompositingCompatible
{
	AUGUIDefineControl(TPngPictControl, TViewNoCompositingCompatible, "pict");
protected:
	// Constructor/Destructor
	TPngPictControl(HIViewRef inControl);
    virtual ~TPngPictControl();

    virtual ControlKind		GetKind();

    virtual OSStatus		Initialize( TCarbonEvent& inEvent );
    virtual void		DrawStaticBackground(RgnHandle	inLimitRgn, CGContextRef inContext, bool  inCompositing );
    virtual void		CompatibleDraw(RgnHandle	inLimitRgn, CGContextRef inContext,bool  inCompositing );
    virtual void		TitleChanged();
    virtual UInt32			GetBehaviors();

    virtual OSStatus SetData(OSType inTag, ControlPartCode inPart, Size inSize, const void* inPtr);
    virtual OSStatus GetData(OSType inTag, ControlPartCode inPart, Size inSize, Size* outSize, void* inPtr);
#if 0
	OSStatus
		HandleEvent(
					EventHandlerCallRef	inCallRef,
					TCarbonEvent&		inEvent );
#endif
private:
    CGImageRef mImage;
};

#endif // 