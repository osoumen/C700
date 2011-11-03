/*
 *  TWaveView.cpp
 *
 */

#include "TWaveView.h"

AUGUIProperties(TWaveView) = {
	AUGUI::property_t()
};
AUGUIInit(TWaveView);

// -----------------------------------------------------------------------------
//	TWaveView constructor
// -----------------------------------------------------------------------------
//
TWaveView::TWaveView(
								 HIViewRef			inControl )
:	TViewNoCompositingCompatible( inControl )
{
	//    ChangeAutoInvalidateFlags( kAutoInvalidateOnActivate | kAutoInvalidateOnEnable, 0 );
	path = CGPathCreateMutable();
	looppoint = 0;
	loaded = false;
	converting = 0;
}

// -----------------------------------------------------------------------------
//	TWaveView destructor
// -----------------------------------------------------------------------------
//	Clean up after yourself.
//
TWaveView::~TWaveView()
{
	CGPathRelease(path);
}

// -----------------------------------------------------------------------------
//	GetKind
// -----------------------------------------------------------------------------
//
ControlKind TWaveView::GetKind()
{
    const ControlKind kMyKind = { 'VeMa', 'wave' };
    return kMyKind;
}

//-----------------------------------------------------------------------------------
//	Initialize
//-----------------------------------------------------------------------------------
//	The control is set up.  Do the last minute stuff that needs to be done like
//	installing EventLoopTimers.
//
OSStatus TWaveView::Initialize(
									 TCarbonEvent&		inEvent )
{
#pragma unused( inEvent )
	
    TitleChanged();
    return noErr;
}
//-----------------------------------------------------------------------------------
//	TitleChanged
//-----------------------------------------------------------------------------------
void TWaveView::TitleChanged()
{
}

//-----------------------------------------------------------------------------------
//	Draw
//-----------------------------------------------------------------------------------
//	The fun part of the control
//
void TWaveView::DrawStaticBackground(
									 RgnHandle				inLimitRgn,
									 CGContextRef			inContext,
									 bool  inCompositing)
{
#pragma unused( inLimitRgn )
}
//-----------------------------------------------------------------------------------
//	Draw
//-----------------------------------------------------------------------------------
//	The fun part of the control
//
void TWaveView::CompatibleDraw(
									 RgnHandle				inLimitRgn,
									 CGContextRef			inContext,
									 bool  inCompositing)
{
	TRect bounds = Bounds();
	CGContextSaveGState(inContext);
	
	CGContextSetRGBFillColor(inContext,67/255.0,75/255.0,88/255.0,1);
	CGContextFillRect(inContext, bounds);
	
	if (converting) {
		CGContextSetRGBStrokeColor(inContext,0,0,0,1.0);
		CGContextAddRect(inContext,bounds);
		CGContextDrawPath(inContext,kCGPathStroke);
		
		CGContextScaleCTM(inContext, 1.0f, -1.0f);
		CGContextTranslateCTM(inContext, bounds.MinX(), -bounds.MaxY());
		CGContextSelectFont(inContext, "Courier",18, kCGEncodingMacRoman);
		CGContextSetTextDrawingMode(inContext, kCGTextFill);
		CGContextSetRGBFillColor(inContext,180/255.0,248/255.0,1,1);
		CGContextShowTextAtPoint(inContext,10,10,"Converting to ADPCM...",22);
	}
	else {
		if (loaded) {
			CGContextSetRGBStrokeColor(inContext,180/255.0,248/255.0,1,1.0);
			CGContextAddPath(inContext,path);
			CGContextDrawPath(inContext,kCGPathStroke);
			
			if (looppoint > 0) {
				CGContextSetRGBStrokeColor(inContext,1,1,0,1.0);
				CGContextMoveToPoint(inContext,bounds.MinX()+bounds.Width()*looppoint/datanum,bounds.MinY());
				CGContextAddLineToPoint(inContext,bounds.MinX()+bounds.Width()*looppoint/datanum,bounds.MaxY());
				CGContextDrawPath(inContext,kCGPathStroke);
			}
			CGContextSetRGBStrokeColor(inContext,0,0,0,1.0);
			CGContextAddRect(inContext,bounds);
			CGContextDrawPath(inContext,kCGPathStroke);
		}
		else {
			CGContextSetRGBStrokeColor(inContext,0,0,0,1.0);
			CGContextAddRect(inContext,bounds);
			CGContextDrawPath(inContext,kCGPathStroke);

			CGContextScaleCTM(inContext, 1.0f, -1.0f);
			CGContextTranslateCTM(inContext, bounds.MinX(), -bounds.MaxY());
			CGContextSelectFont(inContext, "Courier",18, kCGEncodingMacRoman);
			CGContextSetTextDrawingMode(inContext, kCGTextFill);
			CGContextSetRGBFillColor(inContext,180/255.0,248/255.0,1,1);
			CGContextShowTextAtPoint(inContext,10,10,"Drop audio file here.",21);
		}
	}
	CGContextRestoreGState(inContext);
}

UInt32 TWaveView::GetBehaviors()
{
    return TViewNoCompositingCompatible::GetBehaviors() | kControlSupportsEmbedding;
}

OSStatus TWaveView::SetData(OSType inTag, ControlPartCode inPart, Size inSize, const void* inPtr)
{
	OSStatus err = noErr;
	TRect	bounds = Bounds();
	int		width=bounds.Width();
	int		height=bounds.Height()/2;
	int		center=bounds.Height()/2;
	float	temp;
	short	*dataptr=(short*)inPtr,*dataend;
	float	pixelPerFrame=(float)width/(float)inSize;
	dataend=dataptr+inSize;
	
	switch (inTag)
	{
		case kWaveDataTag:
			if (!CGPathIsEmpty(path)) {
				CGPathRelease(path);
				path = CGPathCreateMutable();
			}
			datanum = 0;
			loaded = false;
			converting = false;
			
			if (inPtr) {
				float x=1,y=0,xLoc=1.0f,max=0,min=0;
				
				while (dataptr < dataend) {
					temp = *dataptr * height / 32768.0;
					if (max==.0f) max=temp;
					if (min==.0f) min=temp;
					if (temp > max) max = temp;
					if (temp < min) min = temp;
					if (xLoc >= 1.0f) {
						float	xadd;
						xLoc = modff(xLoc, &xadd);
						y = center-min;
						if (x == 1)
							CGPathMoveToPoint(path,NULL,x+bounds.MinX(),y+bounds.MinY());
						else
							CGPathAddLineToPoint(path,NULL,x+bounds.MinX(),y+bounds.MinY());
						if (max != min) {
							y = center-max;
							CGPathAddLineToPoint(path,NULL,x+bounds.MinX(),y+bounds.MinY());
						}
						max=0.0f;
						min=0.0f;
						
						x += xadd;
					}
					dataptr++;
					xLoc += pixelPerFrame;
				}
				if (max != 0.0f) {
					y = center-min;
					CGPathAddLineToPoint(path,NULL,x+bounds.MinX(),y+bounds.MinY());
					if (max != min) {
						y = center-max;
						CGPathAddLineToPoint(path,NULL,x+bounds.MinX(),y+bounds.MinY());
					}
				}
				datanum = inSize;
				loaded = true;
			}
			ForceRedraw();
			break;
			
		case kLoopPointTag:
			if (inSize == sizeof(int))
			{
				looppoint = *(int *)inPtr;
				ForceRedraw();
			}
			else
				err = errDataSizeMismatch;
			break;
			
		case kShowMsgTag:
			converting = true;
			ForceRedraw();
			break;
			
		default:
			err = TViewNoCompositingCompatible::SetData(inTag, inPart, inSize, inPtr);
	}
	return err;
}

OSStatus TWaveView::GetData(OSType inTag, ControlPartCode inPart, Size inSize, Size* outSize, void* inPtr)
{
	OSStatus err = noErr;
	switch (inTag)
	{
		case kWaveDataTag:
			/*
			if (inSize == sizeof(CGImageRef))
			{
				if (inPtr)
					*(CGImageRef *)inPtr = mImage;
				if (outSize)
					*outSize = sizeof(CGImageRef);
			}
			else
				err = errDataSizeMismatch;
			 */
			break;

		default:
			err = TViewNoCompositingCompatible::GetData(inTag, inPart, inSize, outSize, inPtr);
	}
	return err;
}

