/*
 *  TValuePictControl.h
 *  AUStk
 *
 *  Created by Airy André on Sun Oct 20 2002.
 *  Copyright (c) 2002 Airy André. All rights reserved.
 *
 */

#ifndef TValuePictControl_H_
#define TValuePictControl_H_

#include "TViewNoCompositingCompatible.h"

class TValuePictControl
: public TViewNoCompositingCompatible
{
	AUGUIDefineControl(TValuePictControl, TViewNoCompositingCompatible, "valpict");
	
protected:
	// Constructor/Destructor
	TValuePictControl(HIViewRef inControl);
    virtual ~TValuePictControl();

    virtual ControlKind		GetKind();

	virtual bool UseNonblockingTracking() { return true; }

    virtual OSStatus		Initialize( TCarbonEvent& inEvent );
    virtual OSStatus		StillTracking(TCarbonEvent&inEvent, HIPoint& from);
    virtual void		CompatibleDraw(RgnHandle	inLimitRgn, CGContextRef inContext,bool  inCompositing );
    virtual void		TitleChanged();
    virtual void		ValueChanged();
    virtual UInt32 GetBehaviors() { return TViewNoCompositingCompatible::GetBehaviors() | kControlSupportsEmbedding ; }
    
private:
    CGImageRef mImage;
    CFMutableDictionaryRef mImageCache;
};

#endif // 
