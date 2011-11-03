/*
 *  TValuePictButtonControl.h
 *  AUStk
 *
 *  Created by Airy André on Sun Oct 20 2002.
 *  Copyright (c) 2002 Airy André. All rights reserved.
 *
 */

#ifndef TValuePictButtonControl_H_
#define TValuePictButtonControl_H_

#include "TValuePictControl.h"

class TValuePictButtonControl
: public TValuePictControl
{
	AUGUIDefineControl(TValuePictButtonControl, TViewNoCompositingCompatible, "pbutton");
protected:
	// Constructor/Destructor
	virtual ~TValuePictButtonControl();
    TValuePictButtonControl(HIViewRef inControl);

    virtual ControlKind		GetKind();
	virtual OSStatus StartTracking(TCarbonEvent&inEvent, HIPoint& from);
	virtual OSStatus StopTracking(TCarbonEvent&inEvent, HIPoint& from);
};

#endif // 
