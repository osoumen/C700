/*
 *  AUGUIUtilities.cpp
 *  Ritmo
 *
 *  Created by Chris Reed on Tue Feb 04 2003.
 *  Copyright (c) 2003 Chris Reed. All rights reserved.
 *
 */

#include "AUGUIUtilities.h"
#include <ApplicationServices/ApplicationServices.h>

// Converts an HIRect to a Quickdraw rectangle, snipping off non-integral bits.
void AUGUI::HIRectToQDRect(const HIRect* inHIRect, Rect* outQDRect)
{
    outQDRect->top = (SInt16)CGRectGetMinY(*inHIRect);
    outQDRect->left = (SInt16)CGRectGetMinX(*inHIRect);
    outQDRect->bottom = (SInt16)CGRectGetMaxY(*inHIRect);
    outQDRect->right = (SInt16)CGRectGetMaxX(*inHIRect);
}

TRect AUGUI::RectForCGImage(CGImageRef theImage)
{
	if (theImage == NULL)
		return TRect();
	return TRect(0, 0, CGImageGetWidth(theImage), CGImageGetHeight(theImage));
}

//! This function works by saving the current port, setting the port to
//! the given window's content port, and then converting the point. This
//! would be easy enough to do by simply calling \c LocalToGlobal(), but
//! you have to guarantee that the port is properly set. So we have this
//! utility.
void AUGUI::ConvertWindowPointToGlobal(WindowRef theWindow, Point& thePoint)
{
    GrafPtr savePort;
    GetPort(&savePort);
    SetPort(GetWindowPort(theWindow));
    LocalToGlobal(&thePoint);
    SetPort(savePort);
}

//! This function works by saving the current port, setting the port to
//! the given window's content port, and then converting the point. This
//! would be easy enough to do by simply calling \c GlobalToLocal(), but
//! you have to guarantee that the port is properly set. So we have this
//! utility.
void AUGUI::ConvertGlobalToWindowPoint(WindowRef theWindow, Point& thePoint)
{
    GrafPtr savePort;
    GetPort(&savePort);
    SetPort(GetWindowPort(theWindow));
    GlobalToLocal(&thePoint);
    SetPort(savePort);
}

//! Both the QuickDraw and CoreGraphics fonts are set. This allows you to
//! call the QuickDraw Text routines to measure text since there is no way
//! to measure text in CoreGraphics (unbelievable!). Well, OK. You can draw
//! a string invisibly and subtract, but it's not the same. For one thing,
//! you have to have a context to draw into. You can't pre-compute sizes
//! this way.
void AUGUI::SetFontFromFontStyleRec(CGContextRef context, ControlFontStyleRec& rec, ThemeFontID baseThemeFont, ThemeTextColor baseThemeColor)
{
	Str255 fontName;
	SInt16 fontID;
	SInt16 fontSize;
	Style fontStyle;
	ThemeFontID themeFont = baseThemeFont;
	bool useThemeFont = false;
	
	if (rec.font >= kThemeViewsFont && rec.font <= kThemeSystemFont)
	{
		switch (rec.font)
		{
			case kControlFontBigSystemFont:
				themeFont = kThemeSystemFont;
				break;
			case kControlFontSmallSystemFont:
				themeFont = kThemeSmallSystemFont;
				break;
			case kControlFontSmallBoldSystemFont:
				themeFont = kThemeSmallEmphasizedSystemFont;
				break;
			case kControlFontViewSystemFont:
				themeFont = kThemeViewsFont;
				break;
		}
		useThemeFont = true;
	}
	else if (rec.flags & kControlUseThemeFontIDMask)
	{
		themeFont = rec.font;
		useThemeFont = true;
	}
	
	// always start off with the theme font
	GetThemeFont(themeFont, smSystemScript, fontName, &fontSize, &fontStyle);
	
	if (!useThemeFont || rec.flags & kControlAddToMetaFontMask)
	{
		if (rec.flags & kControlUseFontMask)
			GetFontName(rec.font, fontName);
	}
	
	ATSFontFamilyRef family = ATSFontFamilyFindFromQuickDrawName(fontName);
//	printf("family = %p\n", family);
	
	FMFont theFMFont;
	FMFontStyle intrinsicStyle;
	/*OSStatus status =*/ FMGetFontFromFontFamilyInstance(family, fontStyle, &theFMFont, &intrinsicStyle);
//	printf("FMGetFontFromFontFamilyInstance status = %d\n", status);
	
	ATSFontRef atsFont =  FMGetATSFontRefFromFont(theFMFont);
//	printf("atsFont = %p\n", atsFont);
//	CGFontRef cgFont = CGFontCreateWithPlatformFont(&atsFont);
	
	GetFNum(fontName, &fontID);
//	printf("fontID = %d\n", (int)fontID);
	
	// then apply components of rec based on which flags are set
	if (!useThemeFont || rec.flags & kControlAddToMetaFontMask)
	{
		if (rec.flags & kControlUseFontMask)
			fontID = rec.font;
		if (rec.flags & kControlUseSizeMask)
			fontSize = rec.size;
		if (rec.flags & kControlUseFaceMask)
			fontStyle = rec.style;
	}
	
	// set the font
	TextFont(fontID);
	TextSize(fontSize);
	TextFace(fontStyle);
	
	// XXX this doesn't work right, but using the postscript name does
//	printf("cgFont = %p; size = %d\n", cgFont, (int)fontSize);
//	CGContextSetFont(context, cgFont);
//	CGContextSetFontSize(context, float(fontSize));
	
	if (context)
	{
		CFStringRef postscriptName;
		ATSFontGetPostScriptName(atsFont, 0, &postscriptName);
		
		const char *postscriptNameString = CFStringGetCStringPtr(postscriptName, CFStringGetSystemEncoding());
//		printf("postscript name = %s\n", postscriptNameString);
		CGContextSelectFont(context, postscriptNameString, float(fontSize), kCGEncodingMacRoman);
		
		CFRelease(postscriptName);
	}
	
	// colour is set differently when we have a context
	if (context && rec.flags & kControlUseForeColorMask)
	{
		float r = float(rec.foreColor.red) / 65535.0f;
		float g = float(rec.foreColor.green) / 65535.0f;
		float b = float(rec.foreColor.blue) / 65535.0f;
		CGContextSetRGBFillColor(context, r, g, b, 1.0);
	}
	else if (!context)
	{
		// QuickDraw only; start off with theme color and then apply custom colors
		SetThemeTextColor(baseThemeColor, 32, true);
		
		if (rec.flags & kControlUseForeColorMask)
			RGBForeColor(&rec.foreColor);
		if (rec.flags & kControlUseBackColorMask)
			RGBBackColor(&rec.backColor);
	}
}

#define kAlignInterval 0x10
#define kAlignMask (kAlignInterval-1)
//! Allocate a zeroed 16 bytes aligned buffer
void* AUGUI::malloc_aligned(UInt32 numBytes, void** realPointer)
{
	void* memObject;
	void* alignedBuffer;
	
	*realPointer = 0;
	
	memObject = malloc(numBytes);
	if (!memObject)
		return 0;
	UInt32 misalign = UInt32(memObject) & kAlignMask;
	if (misalign)
	{
		memObject = realloc(memObject, numBytes + kAlignMask);
		if (!memObject)
			return 0;
		alignedBuffer = ((char *)memObject + kAlignInterval - misalign);
	}
	else
	{
		alignedBuffer = memObject;
	}
	bzero(alignedBuffer, numBytes);
	*realPointer = memObject;
	return alignedBuffer;
}


