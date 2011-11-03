/*
 *  TTextPopup.h
 *  Automat
 *
 *  Created by Stefan Kirch on Sun Jun 06 2004.
 *  Copyright (c) 2004 Stefan Kirch. All rights reserved.
 *
 */

//  based on TImagePopup created by Chris Reed

// -----------------------------------------------------------------------------

#ifndef TTextPopup_H_
#define TTextPopup_H_

#include "TViewNoCompositingCompatible.h"

// -----------------------------------------------------------------------------

class TTextPopup:public TViewNoCompositingCompatible
{
	AUGUIDefineControl(TTextPopup, TViewNoCompositingCompatible, "txtpop");

public:
	// Control properties management
	virtual void SetProperty(OSType propID, double value);
	virtual void SetProperty(OSType propID, int32_t value);
	virtual void SetProperty(OSType propID, AUGUI::color_t &value);
	virtual void SetProperty(OSType propID, CFStringRef value);
	virtual void SetProperty(OSType propID, AUGUI::font_t & value);
	
	virtual bool GetProperty(OSType propID, double &value);
	virtual bool GetProperty(OSType propID, int32_t &value);
	virtual bool GetProperty(OSType propID, AUGUI::color_t &value);
	virtual bool GetProperty(OSType propID, CFStringRef &value);
	virtual bool GetProperty(OSType propID, AUGUI::font_t &value);
	
protected:
	
	TTextPopup(HIViewRef inControl);
	virtual ~TTextPopup();
	
	virtual ControlKind		GetKind();
	
	virtual OSStatus		Initialize(TCarbonEvent& inEvent);
	
    virtual void			DrawStaticBackground(RgnHandle inLimitRgn, CGContextRef inContext, bool inCompositing);
	virtual void			CompatibleDraw(RgnHandle inLimitRgn, CGContextRef inContext, bool inCompositing);
	
	virtual ControlPartCode HitTest(const HIPoint&	inWhere);
	virtual OSStatus		HiliteChanged(ControlPartCode inOriginalPart, ControlPartCode inCurrentPart, RgnHandle inInvalRgn);
	virtual OSStatus		Track(TCarbonEvent& inEvent, ControlPartCode* outPart);
	
	virtual void			TitleChanged();
	virtual void			ValueChanged();
		
	virtual OSStatus		SetData(OSType inTag, ControlPartCode inPart, Size inSize, const void* inPtr);
	virtual OSStatus		GetData(OSType inTag, ControlPartCode inPart, Size inSize, Size* outSize, void* inPtr);
	
private:
	
	CGImageRef			mBackImage;
	MenuRef				mPopupMenu;
	CFStringRef			mSelectedItemText;
	CFStringRef			mFilenameBase;
	
	bool				mDrawFrame;
	
	float				mXoffset, mYoffset, mFontSize, mRed, mGreen, mBlue, mAlpha;
	int					mJustify;
	bool				mSmoothFont;
	char				mFontName[100];
};

#endif

// -----------------------------------------------------------------------------

