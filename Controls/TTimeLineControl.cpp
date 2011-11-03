/*
 *  TTimeLineControl.cpp
 *  Automat
 *
 *  Created by Stefan Kirch on Sun Jun 06 2004.
 *  Copyright (c) 2004 Stefan Kirch. All rights reserved.
 *
 */

#include "TTimeLineControl.h"
#include "TImageCache.h"
#include "AUGUIUtilities.h"

using namespace AUGUI;

AUGUIProperties(TTimeLineControl) =
{
	AUGUI::property_t()
};

AUGUIInit(TTimeLineControl);

const ControlKind kTimeLineControlControlKind = {'airy', 'tili'};

// -----------------------------------------------------------------------------

TTimeLineControl::TTimeLineControl(HIViewRef inControl):TViewNoCompositingCompatible(inControl)
{
	mBackImage				= 0;

	mX						= 0.0f;
	mY						= 0.0f;
	mMemo					= 0.0f;
	
	mTopValue				= 1.0f;
	mTopNormalize			= 1.0f;

	mMode					= kTlc_SelectNone;
	
	mLoopHandleOffset		= 0.0f;
	mLoopHandleHeight		= 5.0f;
	mLoopAreaHeight			= 5.0f;
	
	mLineWidth				= 1.0f;
	mRed					= 1.0f;
	mGreen					= 1.0f;
	mBlue					= 1.0f;
	mAlpha					= 1.0f;
	
	initTLData(&mData);
}

// -----------------------------------------------------------------------------

TTimeLineControl::~TTimeLineControl()
{	
    if (mBackImage)
		CGImageRelease(mBackImage);
}

// -----------------------------------------------------------------------------

ControlKind TTimeLineControl::GetKind()
{
    return kTimeLineControlControlKind;
}

// -----------------------------------------------------------------------------

OSStatus TTimeLineControl::Initialize(TCarbonEvent& inEvent)
{
	#pragma unused(inEvent)
	
    TitleChanged();
    return noErr;
}

// -----------------------------------------------------------------------------

void TTimeLineControl::ValueChanged()
{
    Invalidate();
}

// -----------------------------------------------------------------------------

//  Comma separated title format.
//  bgImage (string), loopHandleOffset (int), loopHandleHeight (int), loopAreaHeight (int), lineWidth (int), lineColor (hex)

void TTimeLineControl::TitleChanged()
{
    if (mBackImage)
		CGImageRelease(mBackImage);
	
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
		if (mBackImage)
			CGImageRelease(mBackImage);
	
		string		= (CFStringRef)CFArrayGetValueAtIndex(info, 0);
		mBackImage  = TImageCache::GetImage(mBundleRef, string, CFSTR("png"), NULL);
		
		FlushStaticBackground();
	}
	
	//	loop handle offset
	
	TRect bounds = Bounds();

	if (count >= 2)
	{
		float height = CFStringGetDoubleValue((CFStringRef)CFArrayGetValueAtIndex(info, 1));

		if (height > 0.0f && height < bounds.Height())
			mLoopHandleOffset = height;
	}
	
	//	loop handle height
	
	if (count >= 3)
	{
		float height = CFStringGetDoubleValue((CFStringRef)CFArrayGetValueAtIndex(info, 2));
		
		if (height > 1.0f && height < bounds.Height())
			mLoopHandleHeight = height;
	}

	//  loop area height
	
	if (count >= 4)
	{
		float	height = CFStringGetDoubleValue((CFStringRef)CFArrayGetValueAtIndex(info, 3));
		float	offset = mLoopHandleHeight + mLoopHandleOffset;
		
		if (height > offset && height < bounds.Height())
			mLoopAreaHeight = height;
		else
			mLoopAreaHeight = offset;
	}
	
	mTopValue		= (bounds.Height() - mLoopAreaHeight) / bounds.Height();
	mTopNormalize	= 1.0f / mTopValue;
	
	//  line width
	
	if (count >= 5)
		mLineWidth = CFStringGetDoubleValue((CFStringRef)CFArrayGetValueAtIndex(info, 4));

	//  line color
	
	if (count >= 6)
	{
		string = (CFStringRef)CFArrayGetValueAtIndex(info, 5);
		char	buffer[9];
		int		color;
			
		CFStringGetCString(string, buffer, 9, CFStringGetSystemEncoding());
		sscanf(buffer, "%x", &color);
		
		mAlpha	= (color >> 24  & 0xff) / 255.0f;
		mRed	= (color >> 16  & 0xff) / 255.0f;
		mGreen  = (color >> 8   & 0xff) / 255.0f;
		mBlue   = (color		& 0xff) / 255.0f;
		
		if (mAlpha == 0.0f)
			mAlpha = 1.0f;
	}

	//  clean up

	CFRelease(infoString);
	CFRelease(info);
}

// -----------------------------------------------------------------------------

void TTimeLineControl::DrawStaticBackground(RgnHandle inLimitRgn, CGContextRef inContext, bool inCompositing)
{
	#pragma unused(inLimitRgn)
	
    TRect bounds = Bounds();

    if (mBackImage)
		HIViewDrawCGImage(inContext, &bounds, mBackImage);
}

// -----------------------------------------------------------------------------

void TTimeLineControl::CompatibleDraw(RgnHandle inLimitRgn, CGContextRef inContext, bool inCompositing)
{
	#pragma unused(inLimitRgn)
	
	CGContextSaveGState(inContext);
   
	TRect	bounds = Bounds();
	CGRect  rect;

	float	top		= roundf(bounds.MinY() + mLoopHandleOffset) + 0.5f;
	float	bottom	= roundf(bounds.MaxY()) - 0.5f;
	
	float	left	= roundf(bounds.MinX() + mData.leftMarkerTime * bounds.Width()) + 0.5f;
	float	right	= roundf(bounds.MinX() + mData.rightMarkerTime * bounds.Width()) - 0.5f;
	float	sync	= roundf(bounds.MinX() + mData.syncOffsetTime * bounds.Width()) + 0.5f;
	
	float	height	= mLoopHandleHeight - mLineWidth;
	float	start	= top + height;
	
	//  draw loop area
	
	CGContextBeginPath(inContext);

	/*
	rect = CGRectMake(left, top, right - left, height);
	CGContextAddRect(inContext, rect);
	*/
	
	CGContextMoveToPoint(inContext, left, top);
	CGContextAddLineToPoint(inContext, right, top);

	CGContextMoveToPoint(inContext, left, start);
	CGContextAddLineToPoint(inContext, right, start);
	
	//  draw leftLoop bounds

	rect = CGRectMake(left, top, 4.0f, height);
	
	CGContextAddRect(inContext, rect);
	CGContextMoveToPoint(inContext, left, start);
	CGContextAddLineToPoint(inContext, left, bottom);

	//  draw rightLoop bounds

	rect = CGRectMake(right - 4.0f, top, 4.0f, height);
	
	CGContextAddRect(inContext, rect);
	CGContextMoveToPoint(inContext, right, start);
	CGContextAddLineToPoint(inContext, right, bottom);
	
	//	draw syncOffset marker
		
	rect = CGRectMake(sync - 2.0f, top, 4.0f, height);
	
	CGContextAddRect(inContext, rect);
	CGContextMoveToPoint(inContext, sync, start);
	CGContextAddLineToPoint(inContext, sync, bottom);	
	
	//  draw path and points
	
	float x, y;
	
	for (int i = 0; i < mData.numOfActivePoints; i++)
	{
		x   = roundf(bounds.MinX() + mData.points[i].time  * bounds.Width()) - 0.5f;
		y   = roundf(bounds.MaxY() - mData.points[i].value * bounds.Height() * mTopValue) - 0.5f;
		
		if (i == 0)
			CGContextMoveToPoint(inContext, x, y);
		else
			CGContextAddLineToPoint(inContext, x, y);
		
		rect = CGRectMake(x - 1.0f, y - 1.0f, 2.0f, 2.0f);
		CGContextAddRect(inContext, rect);
		
		CGContextMoveToPoint(inContext, x, y);
	}
	
	CGContextSetRGBStrokeColor(inContext, mRed, mGreen, mBlue, mAlpha);
	CGContextSetLineWidth(inContext, mLineWidth);
	CGContextSetRGBFillColor(inContext, mRed, mGreen, mBlue, mAlpha);
	
	CGContextDrawPath(inContext, kCGPathFillStroke);
	
	CGContextRestoreGState(inContext);
}

// -----------------------------------------------------------------------------

OSStatus TTimeLineControl::StartTracking(TCarbonEvent& inEvent, HIPoint& from)
{
	TRect   bounds		= Bounds();
	float   fuzzy		= 1.0f / kTlc_NumberOfPoints;
	int		maxIndex	= mData.numOfActivePoints - 1;
	
	mX  = from.x / bounds.Width();
	mY  = 1.0f - from.y / bounds.Height();
							
	if (from.y > mLoopAreaHeight)
	{
		//  select points
		
		mMode = kTlc_SelectNone;
		bool found = false;
		
		for (int i = 0; i < mData.numOfActivePoints; i++)
		{
			//  check if old point
			
			float timeDelta	= fabs(mX - mData.points[i].time);
				
			if (timeDelta < fuzzy)
			{
				//  look for nearest point
				
				mData.activeIndex		= i;
				mMode					= kTlc_SelectPoint;
				fuzzy					= timeDelta;
				
				found					= true;
			}
			else if (found)
				break;
		}
				
		if (mMode == kTlc_SelectPoint)
		{
			//  check if key down
			
			UInt32  modifiers;
			inEvent.GetParameter(kEventParamKeyModifiers, &modifiers);
			
			if (modifiers == optionKey && (mData.activeIndex > 0 && mData.activeIndex < maxIndex))
			{
				//  delete old point
				
				for (int j = mData.activeIndex; j < maxIndex; j++)
				{
					int k = j + 1;
					
					mData.points[j].time	= mData.points[k].time;
					mData.points[j].value   = mData.points[k].value;
				}

				mData.numOfActivePoints--;
			}
			else
			{
				//  move old point
				
				if (mData.activeIndex > 0 && mData.activeIndex < maxIndex)
				{
					if (mX > mData.points[mData.activeIndex - 1].time && mX < mData.points[mData.activeIndex + 1].time)
						mData.points[mData.activeIndex].time = mX;
				}
				
				mData.points[mData.activeIndex].value = mY * mTopNormalize;
			}
		}
		else if (mMode == kTlc_SelectNone && mData.numOfActivePoints < kTlc_NumberOfPoints)
		{			
			//  new point
			
			for (int i = 1; i < mData.numOfActivePoints; i++)
			{
				//  find position in array
				
				if (mX < mData.points[i].time)
				{
					//  create space for new point
					
					for (int j = maxIndex; j >= i; j--)
					{
						int k = j + 1;
						
						mData.points[k].time   = mData.points[j].time;
						mData.points[k].value  = mData.points[j].value;
					}
					
					//  set new point
					
					mData.points[i].time	= mX;
					mData.points[i].value   = mY * mTopNormalize;

					mData.activeIndex		= i;
					mMode					= kTlc_SelectPoint;

					mData.numOfActivePoints++;
						
					break;
				}
			}
		}
	}
	else if (from.y <= (mLoopHandleOffset + mLoopHandleHeight))
	{		
		//  select marker
		
		if (fabs(mX - mData.syncOffsetTime) < fuzzy)
		{
			mMode					= kTlc_SelectSyncOffset;
			mData.syncOffsetTime	= mX;
		}
		else if (fabs(mX - mData.leftMarkerTime) < fuzzy)
		{
			mMode					= kTlc_SelectMarkerLeft;
			mData.leftMarkerTime	= mX;
		}
		else if (fabs(mX - mData.rightMarkerTime) < fuzzy)
		{
			mMode					= kTlc_SelectMarkerRight;
			mData.rightMarkerTime	= mX;
		}
		else if (mData.leftMarkerTime < mX && mX < mData.rightMarkerTime)
		{
			mMode	= kTlc_SelectLoop;
			mMemo	= mX - mData.leftMarkerTime;
		}
		else
			mMode = kTlc_SelectNone;
	}
	else
		mMode = kTlc_SelectNone;
	
	//	printf("TTimeLineControl::StartTracking id %i\n", mMode);

	SetValue(!GetValue());

    return noErr;
}

// -----------------------------------------------------------------------------

OSStatus TTimeLineControl::StillTracking(TCarbonEvent& inEvent, HIPoint& from)
{
    TRect   bounds = Bounds();
	HIPoint mouse;
	
    inEvent.GetParameter<HIPoint>(kEventParamWindowMouseLocation, typeHIPoint, &mouse);
    ConvertToLocal(mouse);
	
    UInt32 modifiers;
    inEvent.GetParameter(kEventParamKeyModifiers, &modifiers);
	
#if SUPPORT_CONTROL_MODIFIERS
	
	if (modifiers == (cmdKey | HIGHEST_PRECISION_MODIFIER))
	{
		mX += (mouse.x - from.x) / (bounds.Width() * 50.0f);
		mY -= (mouse.y - from.y) / (bounds.Height() * 50.0f);
	}
	else if (modifiers == (cmdKey | HIGH_PRECISION_MODIFIER))
	{
		mX += (mouse.x - from.x) / (bounds.Width() * 10.0f);
		mY -= (mouse.y - from.y) / (bounds.Height() * 10.0f);
	}
	else
	{		

#endif
		mX = mouse.x / bounds.Width();
		mY = 1.0f - mouse.y / bounds.Height();

#if SUPPORT_CONTROL_MODIFIERS

	}
		
#endif
			
	if (mX < 0.0f)
		mX = 0.0f;
		
	if (mX > 1.0f)
		mX = 1.0f;
		
	if (mY < 0.0f)
		mY = 0.0f;
		
	if (mY > mTopValue)
		mY = mTopValue;
	
	//  update data
	
	switch(mMode)
	{
		case kTlc_SelectSyncOffset:
			mData.syncOffsetTime = mX;
			break;

		case kTlc_SelectMarkerLeft:
			if (mX < mData.rightMarkerTime)
				mData.leftMarkerTime = mX;
			break;
			
		case kTlc_SelectMarkerRight:
			if (mX > mData.leftMarkerTime)
				mData.rightMarkerTime = mX;
			break;
			
		case kTlc_SelectLoop:
		{
			float	left	= mX - mMemo;
			float	right	= left + (mData.rightMarkerTime - mData.leftMarkerTime);
			
			if (left >= 0.0f && right <= 1.0f)
			{
				mData.leftMarkerTime = left;
				mData.rightMarkerTime = right;
			}
						
			break;
		}
			
		case kTlc_SelectPoint:
		{
			if (mData.activeIndex > 0 && mData.activeIndex < mData.numOfActivePoints - 1)
			{
				if (mX > mData.points[mData.activeIndex - 1].time && mX < mData.points[mData.activeIndex + 1].time)
					mData.points[mData.activeIndex].time = mX;
			}
			
			mData.points[mData.activeIndex].value = mY * mTopNormalize;
			break;
		}
			
		case kTlc_SelectNone:
			break;
	}
		
	SetValue(!GetValue());
	from = mouse;
	
	//	printPresetTLData();
	//	printTLData();
	
    return noErr;
}

// -----------------------------------------------------------------------------

OSStatus TTimeLineControl::SetData(OSType inTag, ControlPartCode inPart, Size inSize, const void* inPtr)
{
	OSStatus err = noErr;
	
	switch (inTag)
	{
		case copyData:
		{
			if (inSize == sizeof(TLData))
				mData = *(TLData*)inPtr;
			else
				err = errDataSizeMismatch;

			if (IsCompositing())
				Invalidate();
			else
				if (IsVisible())
					Draw(0, 0);
		}
		break;

		default:
			err = TViewNoCompositingCompatible::SetData(inTag, inPart, inSize, inPtr);
	}
	
	return err;
}

// -----------------------------------------------------------------------------

OSStatus TTimeLineControl::GetData(OSType inTag, ControlPartCode inPart, Size inSize, Size* outSize, void* inPtr)
{
	OSStatus err = noErr;
	
	switch (inTag)
	{
		case copyData:
		{
			updateTLData(&mData);
			
			if (inSize == sizeof(TLData))
				*(TLData*)inPtr = mData;
			else
				err = errDataSizeMismatch;
		}
		break;

		default:
			err = TViewNoCompositingCompatible::GetData(inTag, inPart, inSize, outSize, inPtr);
	}
	return err;
}

// -----------------------------------------------------------------------------

void TTimeLineControl::initTLData(TLData* data)
{
	//	default sine 
	
	data->numOfActivePoints	= 17;
	
	data->syncOffsetTime	= 0.000000f;
	data->leftMarkerTime	= 0.000000f;
	data->rightMarkerTime	= 1.000000f;
	
	float scale = 1.0f / (data->numOfActivePoints - 1);
	
	for (int i = 0; i < data->numOfActivePoints; i++)
	{
		data->points[i].time	= i * scale;
		data->points[i].value	= 0.5f + sin(2.0f * M_PI * data->points[i].time) * 0.5f;				
	}
	
	updateTLData(data);
}

// -----------------------------------------------------------------------------

void TTimeLineControl::updateTLData(TLData* data)
{
	//  update timeDelta between points
	
	for (int i = 0; i < data->numOfActivePoints - 1; i++)
		data->timeDelta[i] = data->points[i + 1].time - data->points[i].time;

	
	//  update index to last point before syncOffsetPos
	
	for (int i = 1; i < data->numOfActivePoints; i++)
	{
		if (data->points[i].time >= data->syncOffsetTime)
		{
			data->syncOffsetIndex = i - 1;
			break;
		}
	}
	
	//  update index to last point before leftMarkerPos

	for (int i = 1; i < data->numOfActivePoints; i++)
	{
		if (data->points[i].time >= data->leftMarkerTime)
		{
			data->leftMarkerIndex = i - 1;
			break;
		}
	}
	
	//  update index to last point before rightMarkerPos
	
	for (int i = data->leftMarkerIndex; i < data->numOfActivePoints; i++)
	{
		if (data->points[i].time >= data->rightMarkerTime)
		{
			data->rightMarkerIndex = i - 1;
			break;
		}
	}
		
	//  update timeDelta between marker
	data->markerDelta = data->rightMarkerTime - data->leftMarkerTime;
}

// -----------------------------------------------------------------------------

void TTimeLineControl::printTLData()
{
	printf("data->syncOffsetTime	= %ff;\n", mData.syncOffsetTime);
	printf("data->rightMarkerTime	= %ff;\n", mData.leftMarkerTime);
	printf("data->rightMarkerTime	= %ff;\n\n", mData.rightMarkerTime);
	printf("data->numOfActivePoints	= %i;\n\n", mData.numOfActivePoints);

	for (int i = 0; i < mData.numOfActivePoints; i++)
	{
		printf("data->points[%i].time	= %ff;\n", i, mData.points[i].time);
		printf("data->points[%i].value	= %ff;\n", i, mData.points[i].value);
	}

	printf("\n");
}

// -----------------------------------------------------------------------------
