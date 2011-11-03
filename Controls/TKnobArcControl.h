/*
 *  TKnobArcControl.h
 *  Muso
 *
 *  Created by Airy ANDRE on 05/09/04.
 *  Copyright 2004 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef TKnobArcControl_H_
#define TKnobArcControl_H_

#include "TKnobControl.h"

// We should have some kind of TVirtualKnobControl, from which other knobs derive

// -----------------------------------------------------------------------------

enum
{
	kNoAlphaBlend				= 0,
	kAlphaBlend					= 1,
	kInverseAlphaBlend			= 2,
	
    kSmallWidthNoCenter			= 0,
    kSmallDynamicNoCenter		= 1,
	kSmallInvDynamicNoCenter	= 2,
    kSmallWidth					= 3,
    kSmallDynamic				= 4,
    kSmallRing					= 5,
		
	kFullWidth					= 100,
	kFullWidthNoCenter			= 101
};

// -----------------------------------------------------------------------------

class TKnobArcControl:public TKnobControl
{
	AUGUIDefineControl(TKnobArcControl, TKnobControl, "knobArc");

public:
	// Control properties management

	virtual void SetProperty(OSType propID, double value);
	virtual void SetProperty(OSType propID, int32_t value);
	virtual void SetProperty(OSType propID, AUGUI::color_t &value);
	
	virtual bool GetProperty(OSType propID, double &value);
	virtual bool GetProperty(OSType propID, int32_t &value);
	virtual bool GetProperty(OSType propID, AUGUI::color_t &value);
	
protected:
    // Constructor/Destructor
	
			 TKnobArcControl(HIViewRef inControl);
    virtual ~TKnobArcControl();
    
	virtual void	TitleChanged();
    virtual void	CompatibleDraw(RgnHandle inLimitRgn, CGContextRef inContext,bool inCompositing);	
	
private:
	
	void	DrawStrokeArc(CGContextRef context, float fillWidth, float fillRadius, float startAngle, float endAngle, float alphaBlend);
	void	DrawFillArc(CGContextRef context, float fillRadius, float startAngle, float endAngle, float alphaBlend);
	
	float   mFill_R, mFill_G, mFill_B, mFill_A;
	float   mFrame_R, mFrame_G, mFrame_B, mFrame_A;
	
	bool	mDrawFrame;
	bool	mHalfCircle;
	int		mFillMode;
	int		mAlphaBlend;
	
	float	mMiddleAngle;
	
};

#endif // TKnobArcControl_H_#endif

// -----------------------------------------------------------------------------
