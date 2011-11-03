/*
 *  TArcSliderControl.h
 *  Musolomo
 *
 *  Copyright (c) 2004 Airy André. All rights reserved.
 *
 */

#if !defined(_TArcSliderControl_h_)
#define _TArcSliderControl_h_

#include "TSliderControl.h"

class TArcSliderControl : public TSliderControl
{
	AUGUIDefineControl(TArcSliderControl, TSliderControl, "arcslider");
protected:
	// Constructor/Destructor
	TArcSliderControl(HIViewRef inControl);
	virtual ~TArcSliderControl();
		
    virtual void		DrawStaticBackground(RgnHandle	inLimitRgn, CGContextRef inContext, bool  inCompositing );
	virtual void CompatibleDraw(RgnHandle inLimitRgn, CGContextRef inContext, bool inCompositing);
	virtual void TitleChanged();
protected:
		
		float mCenterX, mCenterY, mRadius, mAngle;
};


#endif // _TArcSliderControl_h_
