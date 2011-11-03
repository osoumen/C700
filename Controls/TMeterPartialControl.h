/*
 *  TMeterPartialControl.h
 *  AU303
 *
 *  Created by Airy on Tue Apr 15 2003.
 *  Copyright (c) 2003 Airy ANDRE. All rights reserved.
 *
 */
#pragma once

#include <TMeterControl.h>

//! A TMeterControl which only draws the clipped part of the thumb corresponding to the current value
class TMeterPartialControl : public TMeterControl
{
	AUGUIDefineControl(TMeterPartialControl, TMeterControl, "meterp");
protected:
	// Constructor/Destructor
	TMeterPartialControl(HIViewRef inControl);
	virtual ~TMeterPartialControl();

	virtual void CompatibleDraw(RgnHandle inLimitRgn, CGContextRef inContext, bool inCompositing);
};
