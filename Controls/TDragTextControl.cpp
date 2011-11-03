/*
 *  TDragTextControl.cpp
 *  MadShifta
 *
 *  Created by Stewart Moroney on Fri Jun 04 2004.
 *  Copyright (c) 2004 Airy ANDRE. All rights reserved.
 *
 */

#include "TDragTextControl.h"
#include "TImageCache.h"
#include "AUGUIUtilities.h"

AUGUIProperties(TDragTextControl) = {
	AUGUI::property_t()
};
AUGUIInit(TDragTextControl);

using namespace AUGUI;

// -----------------------------------------------------------------------------
//	macros
// -----------------------------------------------------------------------------
//

// -----------------------------------------------------------------------------
//	TDragTextControl constructor
// -----------------------------------------------------------------------------
//
TDragTextControl::TDragTextControl(
			   HIViewRef inControl )
:	TViewNoCompositingCompatible( inControl ) 
{
}

// -----------------------------------------------------------------------------
//	TDragTextControl destructor
// -----------------------------------------------------------------------------
//	Clean up after yourself.
//
TDragTextControl::~TDragTextControl()
{
}

// -----------------------------------------------------------------------------
//	GetKind
// -----------------------------------------------------------------------------
//
ControlKind TDragTextControl::GetKind()
{
    const ControlKind kMyKind = { 'airy', 'dtxt' };

    return kMyKind;
}


//-----------------------------------------------------------------------------------
//	Initialize
//-----------------------------------------------------------------------------------
//	The control is set up.  Do the last minute stuff that needs to be done like
//	installing EventLoopTimers.
//
OSStatus TDragTextControl::Initialize(
				  TCarbonEvent&		inEvent )
{
#pragma unused( inEvent )

    TitleChanged();
    return noErr;
}
//-----------------------------------------------------------------------------------
//	ValueChanged
//-----------------------------------------------------------------------------------
//
void TDragTextControl::ValueChanged()
{
    Invalidate();
}
//-----------------------------------------------------------------------------------
//	DrawStaticBackground
//-----------------------------------------------------------------------------------
//!	Draw the static part (background) of the control
//
void TDragTextControl::DrawStaticBackground(
										RgnHandle				inLimitRgn,
										CGContextRef			inContext,
										bool  inCompositing)
{
#pragma unused( inLimitRgn )
}
//-----------------------------------------------------------------------------------
//	TitleChanged
//	The title format is
//	 	fontname fontsize fontColourR fontColourG fontColourB horizontalBorder verticalBorder
//-----------------------------------------------------------------------------------
void TDragTextControl::TitleChanged()
{
    if (mBundleRef)
    {
		CFStringRef fileName;
		CopyControlTitleAsCFString(GetViewRef(), &fileName);
		if ( fileName != NULL )
		{
			char buffer[100];
			
			CFStringGetCString(fileName, buffer, 100, kCFStringEncodingASCII);
			sscanf(buffer, "%s %f %f %f %f %f %f %f %s", fontName, &fontSize, &fontRColour, &fontGColour, &fontBColour, &horizBorder, &vertBorder, &valueMultipler, unitStr);
			if (valueMultipler == 0) valueMultipler=1;

			CFRelease( fileName );
		}
    }
}
//-----------------------------------------------------------------------------------
//	CompatibleDraw
//-----------------------------------------------------------------------------------
//!	The fun part of the control
//
void TDragTextControl::CompatibleDraw(
				  RgnHandle				inLimitRgn,
				  CGContextRef			inContext,
				  bool  inCompositing)
{
#pragma unused(inLimitRgn)

	TRect bounds = Bounds();
	CGContextSaveGState(inContext);

	//CGContextSetShouldAntialias(inContext,false);
	CGContextSetRGBFillColor(inContext, 0, 0, 0, 1.0f);
	CGContextFillRect(inContext, bounds);
	
	// flip the context
	CGContextScaleCTM(inContext, 1.0f, -1.0f);	
	CGContextTranslateCTM(inContext, bounds.MinX(), -bounds.MaxY());
	bounds.MoveBy(-bounds.MinX(), -bounds.MinY());


//get bounds outline	
    CGRect 		outlineRect;
    outlineRect.origin.x = bounds.Origin().x;
    outlineRect.origin.y = bounds.Origin().y;
    outlineRect.size.width = bounds.Width();
    outlineRect.size.height = bounds.Height();

//draw text

//convert current value to a string
//1st create a CFSting

	CFStringRef val;

	if (unitStr[0] == '0') {
		val = CFStringCreateWithFormat(NULL, NULL, CFSTR("%ld"), (SInt32)(GetValue()*valueMultipler));
	}
	else {
		val = CFStringCreateWithFormat(NULL, NULL, CFSTR("%ld%s"), (SInt32)(GetValue()*valueMultipler), unitStr);
	}
	
	//then get pointer to C String from it
	char kSampleText[100];
	CFStringGetCString(val,kSampleText,100,kCFStringEncodingMacRoman);

    // set the font attributes
    CGContextSelectFont(
        inContext, 
        fontName,
        fontSize, 
        kCGEncodingMacRoman
    );

	CFRelease(val);	
	
	//Measuring Text
	CGPoint	beginpt,endpt;
	beginpt=CGContextGetTextPosition(inContext);
	CGContextSetTextDrawingMode(inContext, kCGTextInvisible);
	CGContextShowText(inContext,kSampleText,strlen(kSampleText));
	endpt=CGContextGetTextPosition(inContext);
	
	//Stroke the text and clip to its interior at the same time
    CGContextSetTextDrawingMode(inContext, kCGTextFill);

    // Fill the text with a color
	CGContextSetRGBFillColor(inContext, fontRColour/255.0f, fontGColour/255.0f, fontBColour/255.0f, 1.0f);
    CGContextShowTextAtPoint(
        inContext, 
        outlineRect.origin.x + (outlineRect.size.width-(endpt.x-beginpt.x))/2, 
        outlineRect.origin.y + vertBorder, 
        kSampleText, 
        strlen(kSampleText)
    );

	CGContextRestoreGState(inContext);
}

// -----------------------------------------------------------------------------
//	StillTracking
// -----------------------------------------------------------------------------
//!	Called during tracking.
//
OSStatus TDragTextControl::StillTracking(TCarbonEvent&inEvent, HIPoint& from)
{
#ifdef EXPERIMENT
	const double kRatio = 250.;
#else
	const double kRatio = 50.;
#endif
    HIPoint mouse;
    float deltaX, deltaY;
    SInt32 mini, maxi;
    UInt32 modifiers;

    mini = GetMinimum();
    maxi = GetMaximum();

    inEvent.GetParameter<HIPoint>( kEventParamWindowMouseLocation, typeHIPoint, &mouse );
    inEvent.GetParameter( kEventParamKeyModifiers, &modifiers );
    ConvertToLocal(mouse);
    deltaX = mouse.x-from.x;
    deltaY = mouse.y-from.y;
    SInt32 curVal = GetValue();
    SInt32 val;
#ifdef EXPERIMENT
    TRect knobBounds = Bounds();
	knobBounds.MoveTo(0.,0.);
	if (from.x > knobBounds.Width()/2) {
		deltaY = -deltaY;
	}
	if (from.y > knobBounds.Height()/2) {
		deltaX = -deltaX;
	}
#endif

#if SUPPORT_CONTROL_MODIFIERS
//    if (modifiers & HIGHEST_PRECISION_MODIFIER) {
//		val = SInt32(rint(curVal+(deltaX-deltaY)*(maxi-mini)/(kRatio*400.)));
//    } else if (modifiers & HIGH_PRECISION_MODIFIER) {
//		val = SInt32(rint(curVal+(deltaX-deltaY)*(maxi-mini)/(kRatio*20.)));
//    } else {
#endif
		val = SInt32(rint(curVal+(deltaX-deltaY)/10.0));
#if SUPPORT_CONTROL_MODIFIERS
//    }
#endif
    if ( val > maxi) val = maxi;
    if ( val < mini) val = mini;
    if (val != curVal) {
		SetValue ( val );
		from = mouse;
    }
#ifdef EXPERIMENT
	from = mouse;
#endif
    return noErr;
}