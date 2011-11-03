/*
 *  TMultiPane.cpp
 *  NeuSynth
 *
 *  Created by Airy on Wed Mar 12 2003.
 *  Copyright (c) 2003 Airy ANDRE. All rights reserved.
 *
 */

#include "TMultiPane.h"

AUGUIProperties(TMultiPane) = {
	AUGUI::property_t()
};
AUGUIInit(TMultiPane);

// -----------------------------------------------------------------------------
//	TMultiPane constructor
// -----------------------------------------------------------------------------
//
TMultiPane::TMultiPane(
									 HIViewRef			inControl )
:	TViewNoCompositingCompatible( inControl), mID(-1)
{
}

// -----------------------------------------------------------------------------
//	TMultiPane destructor
// -----------------------------------------------------------------------------
//	Clean up after yourself.
//
TMultiPane::~TMultiPane()
{
}

// -----------------------------------------------------------------------------
//	GetKind
// -----------------------------------------------------------------------------
//
ControlKind TMultiPane::GetKind()
{
    const ControlKind kMyKind = { 'airy', kControlKind };
    return kMyKind;
}


//-----------------------------------------------------------------------------------
//	Initialize
//-----------------------------------------------------------------------------------
//	The control is set up.  Do the last minute stuff that needs to be done like
//	installing EventLoopTimers.
//
OSStatus TMultiPane::Initialize(
									   TCarbonEvent&		inEvent )
{
#pragma unused( inEvent )
	return noErr;
}

//-----------------------------------------------------------------------------------
//	ValueChanged
//-----------------------------------------------------------------------------------
void TMultiPane::ValueChanged()
{
	HIViewRef subPane;
	SInt32 value = GetValue();
	SInt32 min = GetMinimum();
	SInt32 max = GetMaximum();

	for (SInt32 i = min; i <= max; ++i) {
		HIViewID id = { kSubPanelSignature + mID, i };
		OSStatus result = HIViewFindByID(GetViewRef(), id, &subPane);
		if (result == noErr) {
			HIViewSetVisible( subPane, false);
                }
	}
	HIViewID id = { kSubPanelSignature + mID, value };
	OSStatus result = HIViewFindByID(GetViewRef(), id, &subPane);
	if (result == noErr) {
		HIViewSetVisible( subPane, true);
        }  
	Invalidate();
}

//-----------------------------------------------------------------------------------
//	Draw
//-----------------------------------------------------------------------------------
//	The fun part of the control
//
void TMultiPane::Draw(
					  RgnHandle				inLimitRgn,
					  CGContextRef			inContext )
{
    bool compositing = (inContext != 0);
	if (!compositing) {
            HIViewRef subPane;
            TViewNoCompositingCompatible::Draw(inLimitRgn,inContext); // For background handling
		// First look for 9999 : this is our background if we have one
		HIViewID id = { kSubPanelSignature + mID, 9999 };
		OSStatus result = HIViewFindByID(GetViewRef(), id, &subPane);
		if (result == noErr) {
			Draw1Control( subPane);
		} else {
			// Else, redraw just the subpane
			HIViewID id = { kSubPanelSignature + mID, GetValue() };
			OSStatus result = HIViewFindByID(GetViewRef(), id, &subPane);
			if (result == noErr) {
				Draw1Control( subPane);
			}
		}
	}
}

// Nothing to do here
void TMultiPane::CompatibleDraw(
									   RgnHandle				inLimitRgn,
									   CGContextRef			inContext,
									   bool  inCompositing)
{
}


OSStatus TMultiPane::SetData(OSType inTag, ControlPartCode inPart, Size inSize, const void* inPtr)
{
    OSStatus err = noErr;
    switch (inTag)
    {
        case kSubPanelID:
            if (inSize == sizeof(int))
            {
                mID = *(int *)inPtr;
                Invalidate();
            }
            else
                err = errDataSizeMismatch;
            break;
        default:
            err = TViewNoCompositingCompatible::SetData(inTag, inPart, inSize, inPtr);
    }
    return err;
}

OSStatus TMultiPane::GetData(OSType inTag, ControlPartCode inPart, Size inSize, Size* outSize, void* inPtr)
{
    OSStatus err = noErr;
    switch (inTag)
    {
        case kSubPanelID:
            if (inSize == sizeof(int))
            {
                if (inPtr)
                    *(int *)inPtr = mID;
                if (outSize)
                    *outSize = sizeof(int);
            }
            else
                err = errDataSizeMismatch;
            break;
            
        default:
            err = TViewNoCompositingCompatible::GetData(inTag, inPart, inSize, outSize, inPtr);
    }
    return err;
}

