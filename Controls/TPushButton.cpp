/*
 *  TPushButton.cpp
 *  Muso
 *
 *  Created by Airy ANDRE on 17/10/04.
 *  Copyright 2004 __MyCompanyName__. All rights reserved.
 *
 */

#include "TPushButton.h"

AUGUIProperties(TPushButton) = {
	AUGUI::property_t()
};
AUGUIInit(TPushButton);

using namespace AUGUI;

TPushButton::TPushButton(HIViewRef inControl)
: TValuePictButtonControl(inControl), mValue(-1)
{
}

//	Clean up after yourself.
TPushButton::~TPushButton()
{
}

// -----------------------------------------------------------------------------
//	GetKind
// -----------------------------------------------------------------------------
//
ControlKind TPushButton::GetKind()
{
    const ControlKind kMyKind = { 'airy', 'push' };
	
    return kMyKind;
}

// -----------------------------------------------------------------------------
//	ControlHit
// -----------------------------------------------------------------------------
//	ControlHit method.
//
OSStatus TPushButton::ControlHit(ControlPartCode inPart, UInt32 inModifiers )
{
    int val = GetValue();
    
    if (val < GetMinimum())
        val = GetMinimum();
    
    if (val >= GetMaximum()-1)
	SetValue(GetMinimum());
    else
	SetValue(val+1);
    
    return TViewNoCompositingCompatible::ControlHit(0, 0);
}

OSStatus TPushButton::StartTracking(TCarbonEvent&inEvent, HIPoint& from)
{
    mValue = GetValue();
    SetValue(GetMaximum());
    return noErr;
}

OSStatus TPushButton::StopTracking(TCarbonEvent&inEvent, HIPoint& from)
{
    SetValue(mValue);
    return noErr;
}

