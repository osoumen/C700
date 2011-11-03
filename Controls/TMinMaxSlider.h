/*
 *  TMinMaxSlider.h
 *  Muso
 *
 *  Created by Airy André on 20/03/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#if !defined(_TMinMaxSlider_h_)
#define _TMinMaxSlider_h_

#include "TSliderControl.h"

/*!
* \class TMinMaxSlider
 * \brief A slider control with configurable background and thumb images,
 * to define a min and a max value
 */

class TMinMaxSlider : public TSliderControl
{
	AUGUIDefineControl(TMinMaxSlider, TSliderControl, "minmax");
	
public:
	static const OSType maxTag = 'max ';
	static const OSType minTag = 'min ';
	static const OSType lenTag = 'len ';
protected:
	enum { kStart, kMove, kEnd };
	
	// Constructor/Destructor
	TMinMaxSlider(HIViewRef inControl);
	virtual ~TMinMaxSlider();
	
	virtual ControlKind GetKind();
	
	virtual OSStatus		StillTracking(TCarbonEvent&inEvent, HIPoint& from);
	virtual OSStatus		StartTracking(TCarbonEvent&inEvent, HIPoint& from);
	virtual void CompatibleDraw(RgnHandle inLimitRgn, CGContextRef inContext, bool inCompositing);
	
	virtual OSStatus SetData(OSType inTag, ControlPartCode inPart, Size inSize, const void* inPtr);
	virtual OSStatus GetData(OSType inTag, ControlPartCode inPart, Size inSize, Size* outSize, void* inPtr);
	
protected:
	double mMin, mMax;
	float mStartTracking;
	int mMoveType;
};


#endif // _TMinMaxSlider_h_
