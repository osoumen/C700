/*
 *  TTextPopup.cpp
 *  Automat
 *
 *  Created by Stefan Kirch on Sun Jun 06 2004.
 *  Copyright (c) 2004 Stefan Kirch. All rights reserved.
 *
 */

//  based on TImagePopup created by Chris Reed

// -----------------------------------------------------------------------------

#include "TTextPopup.h"
#include "TImageCache.h"
#include "AUGUIUtilities.h"

using namespace AUGUI;

AUGUIProperties(TTextPopup) =
{
	AUGUI::property_t('pict', CFSTR("picture"), CFSTR("Back Image"), AUGUI::kPicture),
	AUGUI::property_t('colr', CFSTR("color"), CFSTR("Color"), AUGUI::kColor),
	AUGUI::property_t('just', CFSTR("justify"), CFSTR("Justification"), AUGUI::kInteger),
	AUGUI::property_t('xOff', CFSTR("xoffset"), CFSTR("X Offset"), AUGUI::kFloat),
	AUGUI::property_t('yOff', CFSTR("yoffset"), CFSTR("Y Offset"), AUGUI::kFloat),
	AUGUI::property_t('font', CFSTR("font"), CFSTR("Font"), AUGUI::kFont),
	AUGUI::property_t('fram', CFSTR("drawframe"), CFSTR("Draw Frame"), AUGUI::kBool),
	AUGUI::property_t('smfo', CFSTR("smoothfont"), CFSTR("Smooth Font"), AUGUI::kBool),
	AUGUI::property_t()
};

AUGUIInit(TTextPopup);

const ControlKind kTextPopupControlKind = {'airy', 'popb'};

// -----------------------------------------------------------------------------

TTextPopup::TTextPopup(HIViewRef inControl):TViewNoCompositingCompatible(inControl)
{
	mBackImage			= 0;
	mPopupMenu			= 0;
	mSelectedItemText   = 0;
	mFilenameBase		= 0;
	mDrawFrame			= false;
	
	mXoffset			= 0.0f;
	mYoffset			= 0.0f;
	
	mJustify			= teCenter;
	mSmoothFont			= true;
	
	mFontSize			= 9.0f;
	
	mAlpha				= 1.0f;
	mRed				= 0.0f;
	mGreen				= 0.0f;
	mBlue				= 0.0f;
	
	sprintf(mFontName, "Arial");

    ChangeAutoInvalidateFlags(kAutoInvalidateOnActivate | kAutoInvalidateOnEnable | kAutoInvalidateOnHilite, 0);	
}

// -----------------------------------------------------------------------------

TTextPopup::~TTextPopup()
{
    if (mBackImage)
		CFRelease(mBackImage);
	
	if (mPopupMenu)
		ReleaseMenu(mPopupMenu);
	
	if (mSelectedItemText)
		CFRelease(mSelectedItemText);
	
	if (mFilenameBase)
		CFRelease(mFilenameBase);
}

// -----------------------------------------------------------------------------

ControlKind TTextPopup::GetKind()
{
    return kTextPopupControlKind;
}

// -----------------------------------------------------------------------------

OSStatus TTextPopup::Initialize(TCarbonEvent& inEvent)
{
	#pragma unused(inEvent)
	
    TitleChanged();
    return noErr;
}

// -----------------------------------------------------------------------------

void TTextPopup::ValueChanged()
{
	if (mSelectedItemText)
		CFRelease(mSelectedItemText);
	
	mSelectedItemText = 0;
	
	if (mPopupMenu)
		CopyMenuItemTextAsCFString(mPopupMenu, (MenuItemIndex)GetValue(), &mSelectedItemText);

    Invalidate();
}

// -----------------------------------------------------------------------------

//  Comma separated title format. (needed for fontNames with spaces like 'Helvetica Neue Condensed Bold') ;)
//  bgImage (string), fontName (string), fontSize (float), justification (int), fontColor (hex), xOffset (float), yOffset (float), smoothfont (bool)

void TTextPopup::TitleChanged()
{
    if (!mBundleRef)
		return;

	CFStringRef infoString;
	CopyControlTitleAsCFString(GetViewRef(), &infoString);
	
	if (infoString == NULL)
		return;
		
	CFArrayRef info = CFStringCreateArrayBySeparatingStrings(kCFAllocatorDefault, infoString, CFSTR(","));

	if (info == NULL)
		return;

	CFIndex		count   = CFArrayGetCount(info);
	CFStringRef string  = NULL;
	
	//  background image
	
	if (count >= 1)
	{
		SetProperty('pict', (CFStringRef)CFArrayGetValueAtIndex(info, 0));
	}
	
	//  text font
	
	if (count >= 2)
	{
		string = (CFStringRef)CFArrayGetValueAtIndex(info, 1);
		CFStringGetCString(string, mFontName, 50, CFStringGetSystemEncoding());
	}
	
	//  text font size
	
	if (count >= 3)
		mFontSize = CFStringGetDoubleValue((CFStringRef)CFArrayGetValueAtIndex(info, 2));

	//  text justification
	
	if (count >= 4)
		SetProperty('just', 
					(int32_t)CFStringGetIntValue((CFStringRef)CFArrayGetValueAtIndex(info, 3)));

	//  text color
	
	if (count >= 5)
	{
		string = (CFStringRef)CFArrayGetValueAtIndex(info, 4);
		char	buffer[10];
		int		color;
			
		CFStringGetCString(string, buffer, 10, CFStringGetSystemEncoding());
		sscanf(buffer, "%x", &color);
		
		AUGUI::color_t c;
		
		c.alpha	= (color >> 24  & 0xff) / 255.0f;
		c.red	= (color >> 16  & 0xff) / 255.0f;
		c.green	= (color >> 8   & 0xff) / 255.0f;
		c.blue	= (color		& 0xff) / 255.0f;

		if (c.alpha == 0)
			c.alpha = 1.0f;

		SetProperty('colr', c);
	}
	
	//  text x offset
	
	if (count >= 6)
		SetProperty('xOff', 
					CFStringGetDoubleValue((CFStringRef)CFArrayGetValueAtIndex(info, 5)));
		
	//  text y offset
	
	if (count >= 7)
		SetProperty('yOff', 
					CFStringGetDoubleValue((CFStringRef)CFArrayGetValueAtIndex(info, 6)));

	//	antialias
	
	if (count >= 8)
		SetProperty('smfo', 
					(int32_t)CFStringGetIntValue((CFStringRef)CFArrayGetValueAtIndex(info, 7)) == 1);
					
	//  clean up
	
	CFRelease(infoString);
	CFRelease(info);
}

// -----------------------------------------------------------------------------

void TTextPopup::DrawStaticBackground(RgnHandle inLimitRgn, CGContextRef inContext, bool inCompositing)
{
	#pragma unused(inLimitRgn)
    TRect bounds = Bounds();
	
	if (mDrawFrame)
	{
		CGContextSaveGState(inContext);	
		ThemeDrawState	state = kThemeStateDisabled;
		
		CGContextSetRGBFillColor(inContext, 1., 1., 1., 1.);
		CGContextFillRect(inContext, bounds);
		CGContextSetRGBFillColor(inContext, 0., 0., 0., 1.);
		Rect r;
		GetControlBounds(GetViewRef(), &r);
		r.right -= r.left;
		r.bottom -= r.top;
		r.top = r.left = 0;
		DrawThemeEditTextFrame(&r, state);
		CGContextRestoreGState(inContext);	
	}
	
	if (mBackImage)
		HIViewDrawCGImage(inContext, &bounds, mBackImage);
}

// -----------------------------------------------------------------------------

void TTextPopup::CompatibleDraw(RgnHandle inLimitRgn, CGContextRef inContext, bool inCompositing)
{
	#pragma unused(inLimitRgn)
	
	TRect bounds = Bounds();
	if (mSelectedItemText)
	{
		CGContextSaveGState(inContext);
		
		CGContextScaleCTM(inContext, 1.0f, -1.0f);	
		CGContextTranslateCTM(inContext, bounds.MinX(), -bounds.MaxY());
		bounds.MoveBy(-bounds.MinX(), -bounds.MinY());
		
		const char* text = CFStringGetCStringPtr(mSelectedItemText, 0);
		CGContextSelectFont(inContext, mFontName, mFontSize, kCGEncodingMacRoman);		
		CGContextSetRGBFillColor(inContext, mRed, mGreen, mBlue, mAlpha);
		
		if (mJustify != teJustLeft)
		{
			CGContextSetTextDrawingMode(inContext , kCGTextInvisible);
			CGContextShowTextAtPoint(inContext, 0, 0, text, strlen(text));
			
			CGPoint pt = CGContextGetTextPosition(inContext);
				
			if (mJustify == teCenter)
				bounds.origin.x += bounds.size.width / 2.0f - pt.x / 2.0f - mXoffset;
			else
				bounds.origin.x += bounds.size.width - pt.x - mXoffset;
		}
		else
			bounds.origin.x += mXoffset;

		CGContextSetShouldAntialias(inContext, mSmoothFont);
		CGContextSetShouldSmoothFonts(inContext, mSmoothFont);
		CGContextSetTextDrawingMode(inContext, kCGTextFill);
		CGContextShowTextAtPoint(inContext, roundf(bounds.origin.x), roundf(bounds.origin.y + mYoffset), text, strlen(text));
		
		CGContextRestoreGState(inContext);
	}
}

// -----------------------------------------------------------------------------

OSStatus TTextPopup::HiliteChanged(ControlPartCode inOriginalPart, ControlPartCode inCurrentPart, RgnHandle inInvalRgn)
{
	Invalidate();
	
	return noErr;
}

// -----------------------------------------------------------------------------

OSStatus TTextPopup::Track(TCarbonEvent& inEvent, ControlPartCode* outPart)
{
	TRect   bounds  = Bounds();
	int		mini	= GetMinimum();
	int		maxi	= GetMaximum();
	
	if (mPopupMenu)
	{
		HIPoint pt;
		pt.x = lrint(bounds.MinX());
		pt.y = lrint(bounds.MinY());
		ConvertToGlobal(pt);
		Point p;
		p.h = int(pt.x);
		p.v = int(pt.y);
		ConvertWindowPointToGlobal(GetOwner(), p);
		SInt16 saveFontID;
		UInt16 saveFontSize;
		GetMenuFont(mPopupMenu, &saveFontID, &saveFontSize);
		
		Str255 themeFontName;
		SInt16 themeFontID;
		SInt16 themeFontSize;
		Style themeFontStyle;
		GetThemeFont(kThemeSmallSystemFont, smSystemScript, themeFontName, &themeFontSize, &themeFontStyle);
		GetFNum(themeFontName, &themeFontID);
		SetMenuFont(mPopupMenu, themeFontID, themeFontSize);
		long result = PopUpMenuSelect(mPopupMenu, p.v-themeFontSize, p.h, GetValue());
		SetMenuFont(mPopupMenu, saveFontID, saveFontSize);

		if (result != 0)
		{
			int selectedItem = result & 0xffff;
			
			if (selectedItem < mini)
				selectedItem = mini;
			else if (selectedItem > maxi)
				selectedItem = maxi;
			
			if (selectedItem != GetValue())
				SetValue(selectedItem);
		}
		
		*outPart = kControlMenuPart;
	}
	else
		*outPart = kControlNoPart;
	
	return noErr;
}

// -----------------------------------------------------------------------------

ControlPartCode TTextPopup::HitTest(const HIPoint& inWhere)
{
	ControlPartCode part;
	
	if (CGRectContainsPoint(Bounds(), inWhere))
		part = kControlMenuPart;
	else
		part = kControlNoPart;
	
    return part;
}

// -----------------------------------------------------------------------------

OSStatus TTextPopup::SetData(OSType inTag, ControlPartCode inPart, Size inSize, const void* inPtr)
{
	OSStatus err = noErr;
	
	switch (inTag)
	{
		case kControlPopupButtonMenuRefTag:
			if (inSize == sizeof(MenuRef))
			{
				MenuRef theMenu = *(MenuRef*)inPtr;
				
				if (theMenu != mPopupMenu)
				{
					if (mPopupMenu)
						ReleaseMenu(mPopupMenu);
					
					mPopupMenu = theMenu;	
					InsertMenu(mPopupMenu, hierMenu);
					ValueChanged();
				}
			}
			else
				err = errDataSizeMismatch;
			break;
			
		default:
			err = errDataNotSupported;
	}
	
	return err;
}

// -----------------------------------------------------------------------------

OSStatus TTextPopup::GetData(OSType inTag, ControlPartCode inPart, Size inSize, Size* outSize, void* inPtr)
{
	OSStatus err = noErr;
	
	switch (inTag)
	{
		case kControlEditTextCFStringTag:
			if (inSize == sizeof(CFStringRef))
			{
				if (inPtr) {
					*(CFStringRef*)inPtr = mSelectedItemText;
					CFRetain(mSelectedItemText);
				}
				
				if (outSize)
					*outSize = sizeof(CFStringRef);
			}
			else
				err = errDataSizeMismatch;
			break;
		case kControlPopupButtonMenuRefTag:
			if (inSize == sizeof(MenuRef))
			{
				if (inPtr) {
					*(MenuRef*)inPtr = mPopupMenu;
				}
				if (outSize)
					*outSize = sizeof(MenuRef);
			}
			else
				err = errDataSizeMismatch;
			break;
			
		default:
			err = TViewNoCompositingCompatible::GetData(inTag, inPart, inSize, outSize, inPtr);
	}
	
	return err;
}

// -----------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
//	Properties management
//-----------------------------------------------------------------------------------

void TTextPopup::SetProperty(OSType propID, double value)
{
	switch(propID) {
		case 'xOff':
			mXoffset = value;
			break;
		case 'yOff':
			mYoffset = value;
			break;			
		default:
			TViewNoCompositingCompatible::SetProperty(propID, value);
			break;
	};
}

void TTextPopup::SetProperty(OSType propID, int32_t value)
{
	switch(propID) {
		case 'just':
			mJustify = value;
			break;
		case 'fram':
			mDrawFrame = value > 0;
			break;
		case 'smfo':
			mSmoothFont = value > 0;
		default:
			TViewNoCompositingCompatible::SetProperty(propID, value);
			break;
	};
}

void TTextPopup::SetProperty(OSType propID, CFStringRef value)
{
	switch(propID) {
		case 'pict':
			if (mFilenameBase)
				CFRelease(mFilenameBase);
			mFilenameBase = value;
			if (mFilenameBase)
				CFRetain(mFilenameBase);
			if (mBackImage)
				CGImageRelease(mBackImage);
			mBackImage  = TImageCache::GetImage(mBundleRef, mFilenameBase, CFSTR("png"), NULL);
			FlushStaticBackground();
			
			break;
		default:
			TViewNoCompositingCompatible::SetProperty(propID, value);
			break;
	};
}

void TTextPopup::SetProperty(OSType propID, AUGUI::font_t& f)
{
	switch(propID) {
		case 'font':
			mFontSize = f.size;
			CFStringGetCString(f.name, mFontName, 100, kCFStringEncodingASCII);
			break;
		default:
			TViewNoCompositingCompatible::SetProperty(propID, f);
			break;
	};
}

void TTextPopup::SetProperty(OSType propID, AUGUI::color_t &c)
{
	switch(propID) {
		case 'colr':
			mRed	= c.red;
			mGreen	= c.green;
			mBlue	= c.blue;
			mAlpha	= c.alpha;
			break;
		default:
			TViewNoCompositingCompatible::SetProperty(propID, c);
			break;
	};
}

bool TTextPopup::GetProperty(OSType propID, double &value)
{
	switch(propID) {
		case 'xOff':
			value = mXoffset;
			break;
		case 'yOff':
			value = mYoffset;
			break;
			
		default:
			return TViewNoCompositingCompatible::GetProperty(propID, value);
			break;
	};
	return true;
}

bool TTextPopup::GetProperty(OSType propID, int32_t &value)
{
	switch(propID) {
		case 'just':
			value = mJustify;
			break;
		case 'fram':
			value = mDrawFrame ? 1 : 0;
			break;
		case 'smfo':
			value = mSmoothFont ? 1 : 0;
			break;
		default:
			return TViewNoCompositingCompatible::GetProperty(propID, value);
			break;
	};
	return true;
}

bool TTextPopup::GetProperty(OSType propID, CFStringRef &value)
{
	switch(propID) {
		case 'pict':
			value = mFilenameBase;
			if (value)
				CFRetain(value);
			break;
		default:
			return TViewNoCompositingCompatible::GetProperty(propID, value);
			break;
	};
	return true;
}

bool TTextPopup::GetProperty(OSType propID, AUGUI::font_t &f)
{
	switch(propID) {
		case 'font':
			f.size = (int)mFontSize;
			f.name = CFStringCreateWithCString(NULL, mFontName, kCFStringEncodingASCII);
			break;
		default:
			return TViewNoCompositingCompatible::GetProperty(propID, f);
			break;
	};
	return true;
}

bool TTextPopup::GetProperty(OSType propID, AUGUI::color_t &c)
{
	switch(propID) {
		case 'colr':
			c.alpha	= mAlpha;
			c.red	= mRed;
			c.green = mGreen;
			c.blue	= mBlue;
			break;
		default:
			return TViewNoCompositingCompatible::GetProperty(propID, c);
			break;
	};
	return true;
}



