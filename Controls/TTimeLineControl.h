/*
 *  TTimeLineControl.h
 *  Automat
 *
 *  Created by Stefan Kirch on Sun Jun 06 2004.
 *  Copyright (c) 2004 Stefan Kirch. All rights reserved.
 *
 */

// -----------------------------------------------------------------------------

#ifndef TTimeLineControl_H_
#define TTimeLineControl_H_

#include "TViewNoCompositingCompatible.h"

// -----------------------------------------------------------------------------

enum
{	
    kTlc_SelectSyncOffset	= 0,
    kTlc_SelectMarkerLeft	= 1,
    kTlc_SelectMarkerRight  = 2,
    kTlc_SelectLoop			= 3,
    kTlc_SelectPoint		= 4,
	kTlc_SelectNone			= 5,
	
	kTlc_NumberOfPresets	= 16,
	kTlc_NumberOfPoints		= 64
};

typedef struct TLPoint
{
	float   time;
	float   value;
}
TLPoint;

typedef struct TLData
{
	float   syncOffsetTime;
	float   leftMarkerTime;
	float   rightMarkerTime;
	
	int		syncOffsetIndex;
	int		leftMarkerIndex;
	int		rightMarkerIndex;

	float   markerDelta;
	
	int		numOfActivePoints;
	int		activeIndex;
	
    TLPoint points[kTlc_NumberOfPoints];
	float   timeDelta[kTlc_NumberOfPoints - 1];
}
TLData;

// -----------------------------------------------------------------------------

class TTimeLineControl:public TViewNoCompositingCompatible
{
	AUGUIDefineControl(TTimeLineControl, TViewNoCompositingCompatible, "timeline");

public:
	static  const			OSType copyData = 'cpyD';
	static	void			initTLData(TLData* data);
	static	void			updateTLData(TLData* data);

protected:

	TTimeLineControl(HIViewRef inControl);
    virtual ~TTimeLineControl();
	
    virtual ControlKind		GetKind();
	
	virtual bool			UseNonblockingTracking(){return true;}
	
    virtual OSStatus		Initialize(TCarbonEvent& inEvent);
	
	virtual OSStatus		StartTracking(TCarbonEvent&inEvent, HIPoint& from);
    virtual OSStatus		StillTracking(TCarbonEvent&inEvent, HIPoint& from);
	
    virtual void			DrawStaticBackground(RgnHandle inLimitRgn, CGContextRef inContext, bool inCompositing);
    virtual void			CompatibleDraw(RgnHandle inLimitRgn, CGContextRef inContext, bool inCompositing);
	
    virtual void			TitleChanged();
    virtual void			ValueChanged();
	
	virtual OSStatus 		SetData(OSType inTag, ControlPartCode inPart, Size inSize, const void* inPtr);
	virtual OSStatus 		GetData(OSType inTag, ControlPartCode inPart, Size inSize, Size* outSize, void* inPtr);

private:
	
	void					printTLData();
	
	CGImageRef				mBackImage;
	float					mX, mY, mMemo, mTopValue, mTopNormalize;
	int						mMode;
	
	float					mLoopHandleOffset, mLoopHandleHeight, mLoopAreaHeight;
	float					mLineWidth, mRed, mGreen, mBlue, mAlpha;
	
	TLData					mData;
};

#endif

// -----------------------------------------------------------------------------
