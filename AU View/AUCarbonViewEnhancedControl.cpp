/*
 *  AUCarbonViewEnhancedControl.cpp
 *  CAUGuiDemo
 *
 *  Created by Airy on Sat May 24 2003.
 *  Copyright (c) 2003 Airy ANDRE. All rights reserved.
 *
 */

#include "AUCarbonViewEnhancedControl.h"

AUCarbonViewEnhancedControl::AUCarbonViewEnhancedControl(AUCarbonViewBase *ownerView, AUParameterListenerRef listener, ControlType type, const CAAUParameter &param, ControlRef control, ControlPartCode inPart, OSType inTag, int inDigits) :
AUCarbonViewControl(ownerView, listener, type, param, control),
mPart(inPart),
mTag(inTag),
mDigits(inDigits)
{
    if (type == kTypeText && inTag == 0)
        mTag = kControlEditTextCFStringTag; // Text control values are not a special case anymore
}

void	AUCarbonViewEnhancedControl::ParameterToControl(Float32 paramValue)
{
    switch (mType) {
        case kTypeText:
        {
            // Get the string for the value
            ++mInControlInitialization;
            if (mParam.HasNamedParams()) {
                SetTextValue(mParam.GetParamName(int(paramValue-mParam.ParamInfo().minValue+.5)));
            } else {
                CFStringRef cfstr = 0;
                char valstr[32];
                AUParameterFormatValue(	paramValue, &mParam, valstr, mDigits);
                cfstr = CFStringCreateWithCString(NULL, valstr, kCFStringEncodingASCII);
                SetTextValue(cfstr);
                CFRelease (cfstr);
            }
            --mInControlInitialization;
            break;
        }
        default:
            AUCarbonViewControl::ParameterToControl(paramValue);
    }
}

void	AUCarbonViewEnhancedControl::SetValueFract(double value)
{
    if (mTag) {
        SetControlData(mControl, mPart, mTag, sizeof(value), &value);
    } else {
        AUCarbonViewControl::SetValueFract(value);
    }
}

double	AUCarbonViewEnhancedControl::GetValueFract()
{
    double result;
    if (mTag) {
        GetControlData(mControl, mPart, mTag, sizeof(result), &result, 0);
    } else {
        result = AUCarbonViewControl::GetValueFract();
    }
    return result;
}

void	AUCarbonViewEnhancedControl::SetTextValue(CFStringRef cfstr)
{
    verify_noerr(SetControlData(mControl, mPart, mTag, sizeof(CFStringRef), &cfstr));
    DrawOneControl(mControl);	// !!msh-- This needs to be changed to HIViewSetNeedsDisplay()
}

CFStringRef	AUCarbonViewEnhancedControl::GetTextValue()
{
    CFStringRef cfstr;
    // Should ask the text string to the AU
    verify_noerr(GetControlData(mControl, mPart, mTag, sizeof(CFStringRef), &cfstr, NULL));
    return cfstr;
}

void	AUCarbonViewEnhancedControl::SetValue(long value)
{
    if (mTag)
        SetControlData(mControl, mPart, mTag, sizeof(value), &value);
    else
        AUCarbonViewControl::SetValue(value);
}

long	AUCarbonViewEnhancedControl::GetValue()
{
    long result;
    if (mTag)
        GetControlData(mControl, mPart, mTag, sizeof(result), &result, 0);
    else
        result = AUCarbonViewControl::GetValue();
    return result;
}

