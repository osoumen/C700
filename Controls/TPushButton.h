/*
 *  TPushButton.h
 *  Muso
 *
 *  Created by Airy ANDRE on 17/10/04.
 *  Copyright 2004 __MyCompanyName__. All rights reserved.
 *
 */

#if !defined(_TPushButton_h_)
#define _TPushButton_h_

#include "TValuePictButtonControl.h"

/*!
* \class TPushButton
 * \brief A pushbutton control with configurable images for each value.
 * 
 * This control is similar to the TValuePictButtonControl, except that it does change
 * its value to its max while it's being pushed.
 *
 */

class TPushButton : public TValuePictButtonControl
{
	AUGUIDefineControl(TPushButton, TValuePictButtonControl, "push");
protected:
    // Constructor/Destructor
    TPushButton(HIViewRef inControl);
    virtual ~TPushButton();
    
    virtual bool UseNonblockingTracking() { return true; }
    virtual OSStatus ControlHit(ControlPartCode inPart, UInt32 inModifiers );
    virtual OSStatus		StartTracking(TCarbonEvent&inEvent, HIPoint& from);
    virtual OSStatus		StopTracking(TCarbonEvent&inEvent, HIPoint& from);
    
    int mValue;
};

#endif _TPushButton_h_

