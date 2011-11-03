/*
 *  AUCarbonViewEnhancedControl.h
 *  CAUGuiDemo
 *
 *  Created by Airy on Sat May 24 2003.
 *  Copyright (c) 2003 Airy ANDRE. All rights reserved.
 *
 */

#include <AUCarbonViewControl.h>
#include <CAAUParameter.h>

class AUCarbonViewEnhancedControl : public AUCarbonViewControl
{
public:
	AUCarbonViewEnhancedControl(AUCarbonViewBase *ownerView, AUParameterListenerRef listener, ControlType type, const CAAUParameter &param,
							 ControlRef control, ControlPartCode inPart = kControlNoPart, OSType inTag = 0, int inDigits = 3);
	/*! @method ParameterToControl */
	virtual void		ParameterToControl(Float32 newValue);
	/*! @method SetValueFract */
	virtual void		SetValueFract(double value);
	/*! @method GetValueFract */
	virtual double		GetValueFract();
	/*! @method SetTextValue */
	virtual void		SetTextValue(CFStringRef str);
	/*! @method GetTextValue */
	virtual CFStringRef	GetTextValue();
	/*! @method SetValue */
	virtual void		SetValue(long value);
	/*! @method GetValue */
	virtual long		GetValue();

private:
	/*! @var mPart */
	ControlPartCode mPart;

	/*! @var mTag */
	OSType					mTag;

	/*! @var mDigits */
	int						mDigits;
};


