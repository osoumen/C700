/*
 *  TTransparentEditText.cpp
 *  Ritmo
 *
 *  Created by Chris Reed on Wed Feb 26 2003.
 *  Copyright (c) 2003 Chris Reed. All rights reserved.
 *
 */

#include "TTransparentEditText.h"

#include "TTransparentEditText.h"
#include "TImageCache.h"
#include "AUGUIUtilities.h"

using namespace AUGUI;

AUGUIProperties(TTransparentEditText) = {
	AUGUI::property_t()
};
AUGUIInit(TTransparentEditText);


// used to install the caret update timer on the main event run loop
extern "C" CFRunLoopRef CFRunLoopGetMain(void);

//! The control kind is the same as that for the unicode edit text control, with
//! obviously a different signature. This allows users to compare by the
//! control kind and see us as a regular unicode edit control.
const ControlKind kTransparentEditTextKind = { 'airy', 'eutx' };

//! How often the caret is updated. It would be nice to grab this value
//! from the OS somewhere, somewhen.
const CFTimeInterval kCaretUpdateInterval = 0.5;

//! Clicks must be within 5 pixels vertically and horizontally to be
//! considered a double click.
const float kMinDoubleClickDistance = 5.0;

//! Constructor. The real initialisation takes place in Initialize().
TTransparentEditText::TTransparentEditText(HIViewRef inControl)
	: TViewNoCompositingCompatible(inControl), mTextStyle(0), mTextLayout(0), mTextStorage(), mIsActive(false), mIsFocused(false), mDrawFrame(true), mSelectionStart(0), mSelectionEnd(0), mSelectionIsLeading(true), mCaretUpdateTimer(0), mLastClickTime(0), mFocusImage(0), mBackImage(0), mIsLocked(false), mKeyFilter(0), mNextFocusControl(0), mNextFocusPart(kControlEditTextPart)
{
    ChangeAutoInvalidateFlags(kAutoInvalidateOnActivate | kAutoInvalidateOnEnable, 0);
	
	// reserve some space so the buffer pointer is not null
	mTextStorage.ReserveSpace(100);
	
	// default inset
	mTextInset = 4.0f;
}

//! Clean up after ourself.
TTransparentEditText::~TTransparentEditText()
{
	ATSUDisposeStyle(mTextStyle);
	ATSUDisposeTextLayout(mTextLayout);
	
	// get rid of our caret update timer
	if (mCaretUpdateTimer)
	{
		if (CFRunLoopTimerIsValid(mCaretUpdateTimer))
			CFRunLoopTimerInvalidate(mCaretUpdateTimer);
		CFRelease(mCaretUpdateTimer);
	}

	// release focus image
	if (mFocusImage)
		CFRelease(mFocusImage);
	
	// release background image
	if (mBackImage)
		CFRelease(mBackImage);
}

ControlKind TTransparentEditText::GetKind()
{
    return kTransparentEditTextKind;
}

//! Creates the ATSUI text style and text layout objects. At this point,
//! the text storage will most likely be empty, so we're just setting things
//! up to be updated once text is set. We also activate keyboard focus
//! and unicode text entry events. Wouldn't be much use without those!
OSStatus TTransparentEditText::Initialize(TCarbonEvent& inEvent)
{
#pragma unused(inEvent)

	// we want keyboard focus events
	ActivateInterface(kKeyboardFocus);
    
//	printf("creating text object (control = %p)\n", GetViewRef());
	
	// create style object
	verify_noerr( ATSUCreateStyle(&mTextStyle));
	
	Fixed fontSize = X2Fix(10.0);
	ATSUAttributeTag theTags[] = { kATSUSizeTag };
	ByteCount theSizes[] = { sizeof(Fixed) };
	ATSUAttributeValuePtr theValues[] = { &fontSize };

	verify_noerr( ATSUSetAttributes(mTextStyle, 1, theTags, theSizes, theValues));
	
	// create the layout object
//	UniCharCount runLengths[] = { 0 };
//	verify_noerr( ATSUCreateTextLayoutWithTextPtr(mTextStorage, kATSUFromTextBeginning, kATSUToTextEnd, 0, 1, runLength, &mTextStyle, &mTextLayout));

	verify_noerr( ATSUCreateTextLayout(&mTextLayout));
	TitleChanged();
    return noErr;
}

void TTransparentEditText::TitleChanged()
{
	CFStringRef infoString;
	CopyControlTitleAsCFString(GetViewRef(), &infoString);
	mTextStorage = infoString;

	UpdateTextLayout();
	ForceRedraw();
	CFRelease(infoString);
}

//! Set \c mIsActive and force a redraw.
void TTransparentEditText::ActiveStateChanged()
{
	mIsActive = IsControlActive(GetViewRef());
	
	// the active state does not matter if locked
	if (!mIsLocked)
	{
		// adjust our update timer
		if (mIsActive && mIsFocused)
			InstallCaretUpdateTimer();
		else if (!mIsActive && mIsFocused)
			RemoveCaretUpdateTimer();
	}
}

//! Set the \c mIsFocused member and force a redraw.
OSStatus TTransparentEditText::SetFocusPart(ControlPartCode inDesiredFocus, Boolean inFocusEverything, ControlPartCode* outActualFocus)
{
	OSStatus err = TViewNoCompositingCompatible::SetFocusPart(inDesiredFocus, inFocusEverything, outActualFocus);
	
	mIsFocused = mCurrentFocusPart != kControlNoPart;
	// cannot focus if locked
	if (mIsLocked)
	{
		return errCouldntSetFocus;
	}
	
	// adjust update timer
	if (mIsFocused)
		InstallCaretUpdateTimer();
	else
		RemoveCaretUpdateTimer();
	ForceRedraw();

	return err;
}

//! Install caret update timer. If the timer does not exist, it is created.
//! Once created, it will be reused until it is released in the destructor.
void TTransparentEditText::InstallCaretUpdateTimer()
{
	CFRunLoopRef mainRunLoop = CFRunLoopGetMain();
	
	// create timer
	if (!mCaretUpdateTimer)
	{
		CFRunLoopTimerContext context = { 0, this, NULL, NULL, NULL };
		mCaretUpdateTimer = CFRunLoopTimerCreate(kCFAllocatorDefault, CFAbsoluteTimeGetCurrent(), kCaretUpdateInterval, 0, 0, CaretUpdateTimerCallback, &context);
	}
	
	// make the caret appear the next time the timer fires
	mCaretIsVisible = false;
	
	// add timer to run loop
	if (!CFRunLoopContainsTimer(mainRunLoop, mCaretUpdateTimer, kCFRunLoopCommonModes))
	{
		CFRunLoopAddTimer(mainRunLoop, mCaretUpdateTimer, kCFRunLoopCommonModes);
	}
}

//! Remove caret update timer. The timer will not be created if it does not
//! already exist.
void TTransparentEditText::RemoveCaretUpdateTimer()
{
	CFRunLoopRef mainRunLoop = CFRunLoopGetMain();
	if (mCaretUpdateTimer && CFRunLoopContainsTimer(mainRunLoop, mCaretUpdateTimer, kCFRunLoopCommonModes))
	{
		CFRunLoopRemoveTimer(mainRunLoop, mCaretUpdateTimer, kCFRunLoopCommonModes);
#if 1
		ForceValueChange();
#endif
	}
}

//! Here we call \c UpdateTextLayout() because is needs to update the soft
//! line breaks with the new control width. Then we force a redraw.
OSStatus TTransparentEditText::BoundsChanged(UInt32 inOptions, const HIRect& inOriginalBounds, const HIRect& inCurrentBounds, RgnHandle inInvalRgn)
{
	UpdateTextLayout();
	ForceRedraw();
	return noErr;
}

//! This method is used to set the text in the edit field, or attributes
//! of that text.
OSStatus TTransparentEditText::SetData(OSType inTag, ControlPartCode inPart, Size inSize, const void* inPtr)
{
	OSStatus err = noErr;
	switch (inTag)
	{
		case kTransparentEditTextCFStringTag:
			if (inSize == sizeof(CFStringRef))
			{
				CFStringRef theString = *(CFStringRef*)inPtr;
				if (theString == NULL)
					break;
				mTextStorage = theString;
				UpdateTextLayout();
				ForceRedraw();
			}
			else
			{
				err = errDataSizeMismatch;
			}
			break;

		case kTransparentEditTextFontDrawFrameTag:
			if (inSize == sizeof(int))
			{
				mDrawFrame = *(int*)inPtr;
				ForceRedraw();
			}
			else
			{
				err = errDataSizeMismatch;
			}
			break;
			
		case kTransparentEditTextFontStyleTag:
			if (inSize == sizeof(ControlFontStyleRec))
			{
				// update our style object
				ControlFontStyleRec textStyle = *(ControlFontStyleRec *)inPtr;

				ATSUAttributeTag  tags[10];
				ByteCount    tagValueSizes[10];
				ATSUAttributeValuePtr tagValuePtrs[10];
				int nbTags = 0;

				// set up the 3 tags that we are setting in the style
				ATSUFontID fontID;
				if (textStyle.flags & kControlUseFontMask) {
					tags[nbTags] = kATSUFontTag;
					tagValueSizes[nbTags] = sizeof( ATSUFontID );
					ATSUFONDtoFontID ( textStyle.font, normal, & fontID );
					tagValuePtrs[nbTags] = &fontID;
					nbTags++;
				}
				Fixed textSize;
				if (textStyle.flags & kControlUseSizeMask) {
					tags[nbTags] = kATSUSizeTag;
					tagValueSizes[nbTags] = sizeof( Fixed );
					textSize = Long2Fix(textStyle.size);
					tagValuePtrs[nbTags] = &textSize;
					nbTags++;
				}
				
				RGBColor color;
				if (textStyle.flags & kControlUseForeColorMask) {
					tags[nbTags] = kATSUColorTag;
					color = textStyle.foreColor;
					tagValueSizes[nbTags] = sizeof( RGBColor );
					tagValuePtrs[nbTags] = &color;
					nbTags++;
				}

				ATSUSetAttributes( mTextStyle, nbTags, tags, tagValueSizes, tagValuePtrs );

				if (textStyle.flags & kControlUseJustMask) 
				{	
					Fixed size;
					Fract just;
					Fract just2;
					ATSUAttributeTag  theTags[] =  {
						kATSULineWidthTag,
						kATSULineFlushFactorTag,  
						kATSULineJustificationFactorTag};
					ByteCount   theSizes[] = {sizeof( Fixed ), sizeof(Fract), sizeof(Fract)};
					
					size = Long2Fix(long(Bounds().size.width-2.f*mTextInset));
					if (textStyle.just == teJustCenter)
						just = kATSUCenterAlignment;
					else if (textStyle.just == teJustRight)
						just = kATSUEndAlignment;
					else
						just = kATSUStartAlignment;
					just2 = kATSUNoJustification;
					ATSUAttributeValuePtr theValues[] = {&size, &just, &just2};
					ATSUSetLayoutControls (mTextLayout,
													 3, 
													 theTags, 
													 theSizes, 
													 theValues);
				}
				UpdateTextLayout();
				ForceRedraw();
			}
			else
			{
				err = errDataSizeMismatch;
			}
			break;

		case kTransparentEditTextFontATSUStyleTag:
			if (inSize == sizeof(ATSUStyle))
			{
				// synthesize a style rec
				ATSUDisposeStyle (mTextStyle);
				ATSUCreateAndCopyStyle (*(ATSUStyle *)inPtr, &mTextStyle);
				UpdateTextLayout();
				ForceRedraw();
			}
			else
				err = errDataSizeMismatch;
			break;

		case kTransparentEditTextFocusCGImageRefTag:
			if (inSize == sizeof(CGImageRef))
			{
				if (mFocusImage)
					CFRelease(mFocusImage);
				mFocusImage = *(CGImageRef*)inPtr;
				CFRetain(mFocusImage);
				
				if (mIsFocused)
					ForceRedraw();
			}
			else
			{
				err = errDataSizeMismatch;
			}
			break;
		
		case kTransparentEditTextFocusImageURLTag:
			if (inSize == sizeof(CFStringRef))
			{
				if (mFocusImage)
					CFRelease(mFocusImage);
				CFURLRef url = *(CFURLRef*)inPtr;
				
				mFocusImage = TImageCache::GetImage(url);
				CFRetain(mFocusImage);
			}
			else
			{
				err = errDataSizeMismatch;
			}
			break;
		
		case kTransparentEditTextKeyFilterTag:
			if (inSize == sizeof(ControlKeyFilterProcPtr))
				mKeyFilter = *(ControlKeyFilterProcPtr*)inPtr;
			else
				err = errDataSizeMismatch;
			break;
		
		case kTransparentEditTextSelectionTag:
			if (inSize == sizeof(ControlEditTextSelectionRec))
			{
				ControlEditTextSelectionRec& sel = *(ControlEditTextSelectionRec*)inPtr;
				
				uint32_t maxi = mTextStorage.Count();
				mSelectionStart = sel.selStart;
				if (mSelectionStart < 0)
					mSelectionStart = 0;
				if (mSelectionStart > maxi)
					mSelectionStart = maxi;
				
				mSelectionEnd = sel.selEnd;
				if (mSelectionEnd < mSelectionStart)
					mSelectionEnd = mSelectionStart;
				if (mSelectionEnd > maxi)
					mSelectionEnd = maxi;
				
				// update
				ForceRedraw();
			}
			else
			{
				err = errDataSizeMismatch;
			}
			break;
			
		case kTransparentEditTextLockedTag:
			if (inSize == sizeof(Boolean))
				mIsLocked = *(Boolean*)inPtr;
			else
				err = errDataSizeMismatch;
			break;
		
		case kTransparentEditTextNextKeyboardFocusControlIDTag:
			if (inSize == sizeof(ControlID))
			{
				mNextFocusControl = NULL;
				GetControlByID(GetWindowRef(), (const ControlID *)inPtr, &mNextFocusControl);
			}
			else
			{
				err = errDataSizeMismatch;
			}
			break;
		
		case kTransparentEditTextNextKeyboardFocusControlRefTag:
			if (inSize == sizeof(ControlRef))
				mNextFocusControl = *(ControlRef *)inPtr;
			else
				err = errDataSizeMismatch;
			break;
		
		case kTransparentEditTextNextKeyboardFocusPartTag:
			if (inSize == sizeof(ControlPartCode))
				mNextFocusPart = *(ControlPartCode *)inPtr;
			else
				err = errDataSizeMismatch;
			break;

		case kTransparentEditTextBackgroundImageTag:
			if (inSize == sizeof(mBackImage)) {
				if (mBackImage)
					CFRelease(mBackImage);
				mBackImage = *(CGImageRef *)inPtr;
				if (mBackImage)
					CFRetain(mBackImage);
				FlushStaticBackground();
				ForceRedraw();
			} else {
				err = errDataSizeMismatch;
			}
			break;
			
		default:
			// unsupported data tag
			err = errDataNotSupported;
	}
	return err;
}

//! This method can return the text or various information about it and the
//! current settings.
OSStatus TTransparentEditText::GetData(OSType inTag, ControlPartCode inPart, Size inSize, Size* outSize, void* inPtr)
{
	OSStatus err = noErr;
	switch (inTag)
	{
		case kTransparentEditTextCFStringTag:
			if (inSize == sizeof(CFStringRef))
				*(CFStringRef*)inPtr = mTextStorage;
			else
				err = errDataSizeMismatch;
			break;

		case kTransparentEditTextFontStyleTag:
			if (inSize == sizeof(ControlFontStyleRec))
			{
				// synthesize a style rec
				// TO DO
			}
			else
				err = errDataSizeMismatch;
			break;

		case kTransparentEditTextFontATSUStyleTag:
			if (inSize == sizeof(ATSUStyle))
			{
				// synthesize a style rec 
				ATSUCreateAndCopyStyle (mTextStyle, (ATSUStyle *)inPtr);
			}
			else
				err = errDataSizeMismatch;
			break;
			
		case kTransparentEditTextFocusCGImageRefTag:
			if (inSize == sizeof(CGImageRef))
				*(CGImageRef*)inPtr = mFocusImage;
			else
				err = errDataSizeMismatch;
			break;
		
		case kTransparentEditTextKeyFilterTag:
			if (inSize == sizeof(ControlKeyFilterProcPtr))
				*(ControlKeyFilterProcPtr*)inPtr = mKeyFilter;
			else
				err = errDataSizeMismatch;
			break;
		
		case kTransparentEditTextSelectionTag:
			if (inSize == sizeof(ControlEditTextSelectionRec))
			{
				ControlEditTextSelectionRec& sel = *(ControlEditTextSelectionRec*)inPtr;
				sel.selStart = mSelectionStart;
				sel.selEnd = mSelectionEnd;
			}
			else
			{
				err = errDataSizeMismatch;
			}
			break;
			
		case kTransparentEditTextLockedTag:
			if (inSize == sizeof(Boolean))
				*(Boolean*)inPtr = static_cast<Boolean>(mIsLocked);
			else
				err = errDataSizeMismatch;
			break;
		
		case kTransparentEditTextNextKeyboardFocusControlRefTag:
			if (inSize == sizeof(ControlRef))
				*(ControlRef *)inPtr = mNextFocusControl;
			else
				err = errDataSizeMismatch;
			break;

		case kTransparentEditTextNextKeyboardFocusPartTag:
			if (inSize == sizeof(ControlPartCode))
				*(ControlPartCode *)inPtr = mNextFocusPart;
			else
				err = errDataSizeMismatch;
			break;
			
		default:
			err = TViewNoCompositingCompatible::GetData(inTag, inPart, inSize, outSize, inPtr);
	}
	return err;
}

//-----------------------------------------------------------------------------------
//	DrawStaticBackground
//-----------------------------------------------------------------------------------
// Draw the text background
void TTransparentEditText::DrawStaticBackground(
										   RgnHandle				inLimitRgn,
										   CGContextRef			inContext,
										   bool  inCompositing)
{
#pragma unused( inLimitRgn )
	if (mBackImage) {
		HIRect bounds = Bounds();
		HIViewDrawCGImage( inContext, &bounds, mBackImage);
    }
}

//! The fun part of the control. Because the HIView coordinates have 0 of the
//! \em y axis at the upper left, we must flip the context by using a \em
//! y scale value of -1.0. Then coordinates are translated so that 0, 0
//! is at the bottom left. The context graphics state is saved and restored.
void TTransparentEditText::CompatibleDraw(RgnHandle inLimitRgn, CGContextRef inContext, bool inCompositing)
{
#pragma unused(inLimitRgn)
	TRect bounds = Bounds();
	
	CGContextSaveGState(inContext);
	
	//CGContextSetShouldAntialias(inContext,false);
	
	// draw a box around the edit field
	if (mDrawFrame)
	{
#if 0
		if (mIsFocused)
		{
			// get primary highlight colour
			RGBColor highlightColor;
			GetThemeBrushAsColor(kThemeBrushPrimaryHighlightColor, 32, true, &highlightColor);
			float r = float(highlightColor.red) / 65535.0f;
			float g = float(highlightColor.green) / 65535.0f;
			float b = float(highlightColor.blue) / 65535.0f;
			
			CGContextSetRGBStrokeColor(inContext, r, g, b, 0.7);
			
			TRect focusRect = bounds;
			focusRect.Inset(2.0f, 2.0f);
			CGContextSetLineWidth(inContext, 2.0f);
			CGContextStrokeRect(inContext, focusRect);
			
			CGContextSetRGBStrokeColor(inContext, 0.0, 0.0, 0.0, 1.0);
		}
		CGContextStrokeRect(inContext, bounds);
#else
		CGContextSaveGState(inContext);	
		ThemeDrawState	state = kThemeStateDisabled;
		if( IsActive() )
			state = kThemeStatePressed;
		
		CGContextSetRGBFillColor(inContext, 0., 0., 0., 1.);
		//CGContextSetRGBFillColor(inContext, 1., 1., 1., 1.);
		CGContextFillRect(inContext, bounds);
		CGContextSetRGBFillColor(inContext, 0., 0., 0., 1.);
		Rect r;
		GetControlBounds(GetViewRef(), &r);
		r.right -= r.left;
		r.bottom -= r.top;
		r.top = r.left = 0;
		//DrawThemeEditTextFrame(&r, state);
		InsetRect(&r, 3, 3);
		//DrawThemeFocusRect(&r, mIsFocused);
		CGContextRestoreGState(inContext);	
#endif
	}
	
	// flip the context
	CGContextScaleCTM(inContext, 1.0f, -1.0f);
	
	// translate coordinates to the bottom left of the pane is x=0,y=0
	bounds.Inset(mTextInset, mTextInset);
	CGContextTranslateCTM(inContext, bounds.MinX(), -bounds.MaxY());
	bounds.MoveBy(-bounds.MinX(), -bounds.MinY());
	
	// set context in layout object
	ATSUAttributeTag theTags[] = { kATSUCGContextTag };
	ByteCount theSizes[] = { sizeof(CGContextRef) };
	ATSUAttributeValuePtr theValues[] = { &inContext };
		
	ATSUSetLayoutControls(mTextLayout, 1, theTags, theSizes, theValues);
	
	// draw the text
	float baseline = GetBaselineForLine(kATSUFromTextBeginning)-3;
	
	Fixed baselineX = X2Fix(0.0);
	Fixed baselineY = X2Fix(baseline);
	ATSUDrawText(mTextLayout, kATSUFromTextBeginning, kATSUToTextEnd, baselineX, baselineY);
	
	// draw caret or highlight selection if we have focus
	if (!mIsLocked)
	{
		if (mIsFocused)
		{
			if (mSelectionStart == mSelectionEnd && mCaretIsVisible)
			{
				// caret
				ATSUCaret primaryCaret, secondaryCaret;
				Boolean isCaretSplit;
				verify_noerr( ATSUOffsetToPosition(mTextLayout, mSelectionStart, mSelectionIsLeading, &primaryCaret, &secondaryCaret, &isCaretSplit));
				
				// draw the caret or carets
				CGContextSetRGBStrokeColor(inContext, 1., 1., 1., 1.);
				
				CGContextBeginPath(inContext);
				
				CGContextMoveToPoint(inContext, Fix2X(primaryCaret.fX), baseline - Fix2X(primaryCaret.fY));
				CGContextAddLineToPoint(inContext, Fix2X(primaryCaret.fDeltaX), baseline - Fix2X(primaryCaret.fDeltaY));
				
				// support split carets at a bidi boundary
				if (isCaretSplit)
				{
					CGContextMoveToPoint(inContext, Fix2X(secondaryCaret.fX), Fix2X(secondaryCaret.fY) + baseline);
					CGContextAddLineToPoint(inContext, Fix2X(secondaryCaret.fDeltaX), Fix2X(secondaryCaret.fDeltaY) + baseline);
				}
				
				CGContextClosePath(inContext);
				CGContextStrokePath(inContext);
			}
			else
			{
				// highlight selection. we can't use ATSUI's highlight
				// method here because we're in a CGContext and QD calls are
				// ignored.
				
				// calculate highlight rectangle; note that this won't work
				// correctly for discontinuous highlighting
				ATSUCaret startPrimaryCaret, startSecondaryCaret;
				Boolean isCaretSplit;
				verify_noerr( ATSUOffsetToPosition(mTextLayout, mSelectionStart, mSelectionIsLeading, &startPrimaryCaret, &startSecondaryCaret, &isCaretSplit));
				
				ATSUCaret endPrimaryCaret, endSecondaryCaret;
				verify_noerr( ATSUOffsetToPosition(mTextLayout, mSelectionEnd, mSelectionIsLeading, &endPrimaryCaret, &endSecondaryCaret, &isCaretSplit));
				
				TRect highlightRect;
				highlightRect.SetOrigin(Fix2X(startPrimaryCaret.fX), baseline - Fix2X(startPrimaryCaret.fDeltaY));
				highlightRect.SetWidth(Fix2X(endPrimaryCaret.fX) - Fix2X(startPrimaryCaret.fX));
				highlightRect.SetHeight(fabsf(Fix2X(startPrimaryCaret.fDeltaY)) + fabsf(Fix2X(startPrimaryCaret.fY)));
				
				// get primary highlight colour
				RGBColor highlightColor;
				GetThemeBrushAsColor(kThemeBrushPrimaryHighlightColor, 32, true, &highlightColor);
				float r = float(highlightColor.red) / 65535.0f;
				float g = float(highlightColor.green) / 65535.0f;
				float b = float(highlightColor.blue) / 65535.0f;
				
				
				// draw the highlight rect
				CGContextSetRGBFillColor(inContext, r, g, b, 0.7);
				CGContextFillRect(inContext, highlightRect);
				
				// redraw the highlighted text on top
				CGContextSetRGBFillColor(inContext, 0.0, 0.0, 0.0, 1.0);
				ATSUDrawText(mTextLayout, mSelectionStart, mSelectionEnd - mSelectionStart, baselineX, baselineY);
			}
		}
	}
	
	CGContextRestoreGState(inContext);
}

//!	Asks your view to return what part of itself (if any) is hit by the point
//!	given to it. The point is in \em view coordinates, so you should get the view
//!	rect to do bounds checking. All points within the view are considered
//! to be in the \c kControlEditTextPart control part.
ControlPartCode TTransparentEditText::HitTest(const HIPoint& inWhere)
{
    ControlPartCode part;

    // is the mouse in our frame?
    if (CGRectContainsPoint(Bounds(), inWhere))
		part = kControlEditTextPart;
    else
		part = kControlNoPart;
    return part;
}

//! Handle the initial mouse down event.
OSStatus TTransparentEditText::StartTracking(TCarbonEvent& inEvent, HIPoint& from)
{
	// ignore mouse events if we're locked
	if (mIsLocked)
		return noErr;
		
	UniCharArrayOffset primaryOffset, secondaryOffset;
	Boolean isLeading;
	UInt32 modifiers;
	
	// are we focused?
	if (!mIsFocused)
	{
		// clicking on us when not focused switches focus to our view
		SetKeyboardFocus(GetWindowRef(), GetViewRef(), kControlEditTextPart);
	}
	
	// find double clicks
	EventTime thisEventTime = GetEventTime(inEvent);
	double dblTime = double(GetDblTime()) / 60.0;
	if (thisEventTime - mLastClickTime < dblTime && fabsf(from.x - mLastClickPoint.x) < kMinDoubleClickDistance && fabsf(from.y - mLastClickPoint.y) < kMinDoubleClickDistance)
	{
		mClickCount++;
	}
	else
	{
		mClickCount = 1;
	}
	mLastClickTime = thisEventTime;
	mLastClickPoint = from;
	
	// get modifier keys
	inEvent.GetParameter(kEventParamKeyModifiers, &modifiers);
	
	// convert the click position to line relative coordinates
	float baseline = GetBaselineForLine(kATSUFromTextBeginning);
	
	Fixed x, y;
	x = X2Fix(from.x - mTextInset); // x is already line relative
	y = X2Fix(from.y - baseline - mTextInset);
	
	primaryOffset = kATSUFromTextBeginning;
	verify_noerr( ATSUPositionToOffset(mTextLayout, x, y, &primaryOffset, &isLeading, &secondaryOffset));
	
	// set caret position
	if (mClickCount >= 3)
	{
		// select all text
		mSelectionStart = 0;
		mSelectionEnd = mTextStorage.Count();
		mSelectionIsLeading = isLeading;
		mSelectionAnchor = 0;
	}
	else if (mClickCount == 2)
	{
		// select the word under the mouse
		FindWordAtOffset(primaryOffset, &mSelectionStart, &mSelectionEnd);
		mSelectionIsLeading = true;
		
		// this is used to refind the original word that was double-clicked
		mSelectionAnchor = primaryOffset;
	}
	else if (primaryOffset != mSelectionStart || mSelectionEnd != mSelectionStart)
	{
		// single click
		mSelectionStart = primaryOffset;
		mSelectionEnd = primaryOffset;
		mSelectionIsLeading = isLeading;
		
		// set anchor, used in StillTracking
		mSelectionAnchor = primaryOffset;
	}
	
	// update
	ForceRedraw();
	
	return noErr;
}

//! Handle the mouse moving from one location to another while the button
//! is still held down.
OSStatus TTransparentEditText::StillTracking(TCarbonEvent& inEvent, HIPoint& from)
{
	// ignore mouse events if we're locked
	if (mIsLocked)
		return noErr;
		
	HIPoint mouse;
	HIPoint delta;
	UInt32 modifiers;
	
	// nothing more to do for triple-clicks
	if (mClickCount >= 3)
		return noErr;
	
	// get mouse location
	inEvent.GetParameter<HIPoint>(kEventParamWindowMouseLocation, typeHIPoint, &mouse);
	ConvertToLocal(mouse);

    inEvent.GetParameter<HIPoint>(kEventParamMouseDelta, typeHIPoint, &delta);
	inEvent.GetParameter(kEventParamKeyModifiers, &modifiers);

	// convert the click position to line relative coordinates
	float baseline = GetBaselineForLine(kATSUFromTextBeginning);
	
	Fixed x, y;
	x = X2Fix(mouse.x - mTextInset); // x is already line relative
	y = X2Fix(mouse.y - baseline - mTextInset);
//	printf("line relative: x=%g, y=%g\n", Fix2X(x), Fix2X(y));
	
	UniCharArrayOffset primaryOffset, secondaryOffset;
	Boolean isLeading;
	primaryOffset = kATSUFromTextBeginning;
	verify_noerr( ATSUPositionToOffset(mTextLayout, x, y, &primaryOffset, &isLeading, &secondaryOffset));
	
//	printf("primaryOffset = %d, secondaryOffset = %d\n", (int)primaryOffset, (int)secondaryOffset);
	
	// update selection
	if (mClickCount == 2)
	{
		// user has double clicked on a word and is now dragging. so she
		// will expect the selection to snap to words. this is accomplished
		// by re-finding the original word that was selected, finding the
		// word the mouse is currently over, and setting the selection to
		// the smallest start char and the greatest end char.
		uint32_t anchorStart, anchorEnd;
		FindWordAtOffset(mSelectionAnchor, &anchorStart, &anchorEnd);
		
		uint32_t wordStart, wordEnd;
		FindWordAtOffset(primaryOffset, &wordStart, &wordEnd);
		
		if (wordStart > anchorStart)
			mSelectionStart = anchorStart;
		else
			mSelectionStart = wordStart;
		
		if (wordEnd > anchorEnd)
			mSelectionEnd = wordEnd;
		else
			mSelectionEnd = anchorEnd;
	}
	else if (primaryOffset != mSelectionAnchor)
	{
		if (primaryOffset < mSelectionAnchor)
		{
			mSelectionStart = primaryOffset;
			mSelectionEnd = mSelectionAnchor;
			mSelectionIsLeading = isLeading;
		}
		else
		{
			mSelectionStart = mSelectionAnchor;
			mSelectionEnd = primaryOffset;
		}
	}
	
	// force a redraw
	ForceRedraw();
	
	return noErr;
}

// Unicode command characters
const UniChar kTabChar = 0x0009;
const UniChar kDeleteChar = 0x0008;
const UniChar kReturnChar = 0x000d;
const UniChar kEnterChar = 0x0003;
const UniChar kLeftArrowChar = 0x001c;
const UniChar kRightArrowChar = 0x001d;
const UniChar kUpArrowChar = 0x001e;
const UniChar kDownArrowChar = 0x001f;

//! Handle all key events.
OSStatus TTransparentEditText::TextInput(TCarbonEvent& inEvent)
{
	// ignore key events if we're locked
	if (mIsLocked)
		return noErr;
	
	// get the text that was entered
	UInt32 textSize;
	inEvent.GetParameterSize(kEventParamTextInputSendText, &textSize);
	
	if (textSize == 0)
		return noErr;
	
	UInt32 count = textSize / sizeof(UniChar);
	UniChar text[count];
	inEvent.GetParameter<UniChar>(kEventParamTextInputSendText, typeUnicodeText, text);
	
	// get modifiers
	UInt32 modifiers = 0;
	EventRef rawKeyboardEventRef;
	inEvent.GetParameter<EventRef>(kEventParamTextInputSendKeyboardEvent, typeEventRef, &rawKeyboardEventRef);
	TCarbonEvent rawKeyboardEvent(rawKeyboardEventRef);
	rawKeyboardEvent.GetParameter<UInt32>(kEventParamKeyModifiers, typeUInt32, &modifiers);
	
	// get key code
	UInt32 keyCode;
	rawKeyboardEvent.GetParameter<UInt32>(kEventParamKeyCode, typeUInt32, &keyCode);
	
	// pass the event to the key filter
	if (mKeyFilter)
	{
		// pass each unicode character through the key filter
		for (unsigned i=0; i < count; ++i)
		{
			SInt16 filterKeyCode = keyCode;	// we ignore this on return
			EventModifiers filterModifiers = modifiers;
			ControlKeyFilterResult filterResult = (*mKeyFilter)(GetViewRef(), &filterKeyCode, (SInt16*)&text[i], &filterModifiers);
			if (filterResult == kControlKeyFilterPassKey)
			{
				ProcessTextInput(&text[i], 1, filterModifiers);
			}
		}
	}
	else
	{
		// handle it as one big chunk
		ProcessTextInput(text, count, modifiers);
	}
	
	return noErr;
}
	
//! This function is called after the text has been pulled out of the event
//! and any filtering has been performed. It may be called once for a group of
//! multiple characters, or it may be called repeatedly with a single character
//! at a time.
//! 
//! Both command and data characters are handled. Only the first character is
//! looked at as a possible command (e.g., arrow key, enter, tab, etc).
void TTransparentEditText::ProcessTextInput(UniChar* text, uint32_t count, uint32_t modifiers)
{
	// only the first character is considered for key commands
	if (text[0] <= kDownArrowChar && text[0] != kDeleteChar)
	{
		bool forceRedraw = false;

		// a command character has been entered, so process it
		switch (text[0])
		{
			case kTabChar:
				// move to next key target
				if (mNextFocusControl)
					verify_noerr( SetKeyboardFocus(GetWindowRef(), mNextFocusControl, kControlFocusNextPart));
				else
					verify_noerr( HIViewAdvanceFocus(HIViewGetRoot(GetWindowRef()), modifiers));
				break;
			case kReturnChar:
			case kEnterChar:
				// end editing
				ClearKeyboardFocus(GetWindowRef());
				break;
			case kLeftArrowChar:
			case kUpArrowChar:
				if (mSelectionStart > 0)
				{
					if (text[0] == kLeftArrowChar)
					{
						if (mSelectionStart == mSelectionEnd || (modifiers & shiftKey))
							mSelectionStart -= 1;
					}
					else
					{
						mSelectionStart = 0;
					}
					
					if ((modifiers & shiftKey) == 0)
						mSelectionEnd = mSelectionStart;
					
					forceRedraw = true;
				}
				break;
			case kRightArrowChar:
			case kDownArrowChar:
				if (mSelectionEnd < mTextStorage.Count())
				{
					if (text[0] == kRightArrowChar)
					{
						if (mSelectionStart == mSelectionEnd || (modifiers & shiftKey))
							mSelectionEnd += 1;
					}
					else
					{
						mSelectionEnd = mTextStorage.Count();
					}
					
					if ((modifiers & shiftKey) == 0)
						mSelectionStart = mSelectionEnd;
					
					forceRedraw = true;
				}
				break;
		}
		
		// force an update after arrow key
		if (forceRedraw)
		{
			ForceRedraw();
		}
	}
	else
	{
		// update our text storage and update selection
		if (text[0] == kDeleteChar)
		{
			if (mSelectionStart == mSelectionEnd)
			{
				// deleting a single character
				if (mSelectionStart != 0)
				{
					mTextStorage.Delete(mSelectionStart-1);
					
					mSelectionStart -= 1;
					mSelectionEnd = mSelectionStart;
				}
			}
			else
			{
				// deleting multiple characters
				mTextStorage.Delete(mSelectionStart, mSelectionEnd - mSelectionStart);
				
				mSelectionEnd = mSelectionStart;
			}
		}
		else
		{
			if (mSelectionStart == mSelectionEnd)
			{
				// inserting characters
				mTextStorage.InsertText(mSelectionStart, text, count);
				
				mSelectionStart += 1;
				mSelectionEnd = mSelectionStart;
			}
			else
			{
				// replacing the selection
				mTextStorage.Delete(mSelectionStart, mSelectionEnd - mSelectionStart);
				mTextStorage.InsertText(mSelectionStart, text, count);
				
				mSelectionStart += count;
				mSelectionEnd = mSelectionStart;
			}
		}
		
		// update the layout and redraw
		UpdateTextLayout();
		ForceRedraw();
	}
}

//! Reset the pointer to the text for the text layout to the whole storage
//! buffer, and update the length of the single style run. This routine
//! also updates the soft line breaks at the width of the control.
//!
//! \em Note: Line breaking is commented out until support is added for
//! more than one line of text.
void TTransparentEditText::UpdateTextLayout()
{
	UniCharCount count = mTextStorage.Count();
	
	// update text pointer and length
	verify_noerr( ATSUSetTextPointerLocation(mTextLayout, mTextStorage, kATSUFromTextBeginning, count, count));
	verify_noerr( ATSUSetRunStyle(mTextLayout, mTextStyle, kATSUFromTextBeginning, count));
	
	// update soft line breaks
//	TRect bounds = Bounds();
//	Fixed lineWidth = X2Fix(bounds.Width() - mTextInset * 2.0f);
//	
//	verify_noerr( ATSUClearSoftLineBreaks(mTextLayout, kATSUFromTextBeginning, count));
//	verify_noerr( ATSUBatchBreakLines(mTextLayout, kATSUFromTextBeginning, count, lineWidth, NULL));
}

//! The leading is included as part of the descent.
void TTransparentEditText::GetLineHeight(uint32_t lineStartOffset, float* outAscent, float* outDescent)
{
	ATSUTextMeasurement fixedAscent, fixedDescent;
	ByteCount actualSize;
	
	// these two calls will return kATSUNotSetErr but the resulting line 
	// heights are still valid
	ATSUGetLineControl(mTextLayout, lineStartOffset, kATSULineAscentTag, sizeof(ATSUTextMeasurement), &fixedAscent, &actualSize);
	ATSUGetLineControl(mTextLayout, lineStartOffset, kATSULineDescentTag, sizeof(ATSUTextMeasurement), &fixedDescent, &actualSize);
	
	*outAscent = float(Fix2X(fixedAscent));
	*outDescent = float(Fix2X(fixedDescent));
}

//! This function currently assumes there is only one line of text and will
//! center the baseline within the control bounds.
float TTransparentEditText::GetBaselineForLine(uint32_t lineStartOffset)
{
	TRect bounds = Bounds();
	
	// get line height
	float ascent, descent;
	GetLineHeight(lineStartOffset, &ascent, &descent);
	
	// center the line in the view
	return (ascent + descent - bounds.Height()) / 2.0f + descent;
}

//! Returns the start and end offset of the word centered on \c offset.
void TTransparentEditText::FindWordAtOffset(uint32_t offset, uint32_t* wordStart, uint32_t* wordEnd)
{
	verify_noerr( ATSUPreviousCursorPosition(mTextLayout, offset, kATSUByWord, reinterpret_cast<UniCharArrayOffset*>(wordStart)));
	verify_noerr( ATSUNextCursorPosition(mTextLayout, offset, kATSUByWord, reinterpret_cast<UniCharArrayOffset*>(wordEnd)));
}

//! Just be lazy and force a redraw.
void TTransparentEditText::UpdateCaret()
{
	if (mSelectionStart != mSelectionEnd)
		return;
	mCaretIsVisible = !mCaretIsVisible;
	ForceRedraw();
}

//! Simply passes the message along to the object referred to by \c info.
void TTransparentEditText::CaretUpdateTimerCallback(CFRunLoopTimerRef timer, void *info)
{
	TTransparentEditText *This = reinterpret_cast<TTransparentEditText*>(info);
	This->UpdateCaret();
}


