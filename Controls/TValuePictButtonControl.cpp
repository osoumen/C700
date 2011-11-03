/*
 *  TValuePictButtonControl.cpp
 *  NeuSynth
 *
 *  Created by Airy André on Sun Oct 20 2002.
 *  Copyright (c) 2002 Airy André. All rights reserved.
 *
 */

#include "TValuePictButtonControl.h"

AUGUIProperties(TValuePictButtonControl) = {
	AUGUI::property_t()
};
AUGUIInit(TValuePictButtonControl);

// -----------------------------------------------------------------------------
//	TValuePictButtonControl constructor
// -----------------------------------------------------------------------------
//
TValuePictButtonControl::TValuePictButtonControl(
			 HIViewRef			inControl )
:	TValuePictControl( inControl )
{
}

// -----------------------------------------------------------------------------
//	TValuePictButtonControl destructor
// -----------------------------------------------------------------------------
//	Clean up after yourself.
//
TValuePictButtonControl::~TValuePictButtonControl()
{
}
// -----------------------------------------------------------------------------
//	GetKind
// -----------------------------------------------------------------------------
//
ControlKind TValuePictButtonControl::GetKind()
{
    const ControlKind kMyKind = { 'airy', 'bttn' };

    return kMyKind;
}


// -----------------------------------------------------------------------------
// tracking method.
//
OSStatus TValuePictButtonControl::StartTracking(TCarbonEvent&	inEvent, HIPoint& from)
{
    int val = GetValue();
    if (val == GetMaximum())
		SetValue(GetMinimum());
    else
		SetValue(val+1);
    return noErr;
}
// tracking method.
//
OSStatus TValuePictButtonControl::StopTracking(TCarbonEvent&	inEvent, HIPoint& from)
{
    return noErr;
}
