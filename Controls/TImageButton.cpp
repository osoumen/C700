/*
 *  TImageButton.cpp
 *  Ritmo
 *
 *  Created by Chris Reed on Mon Feb 03 2003.
 *  Copyright (c) 2003 Chris Reed. All rights reserved.
 *
 */

#include "TImageButton.h"
#include "TImageCache.h"
#include "AUGUIUtilities.h"

AUGUIProperties(TImageButton) = {
	AUGUI::property_t()
};
AUGUIInit(TImageButton);

using namespace AUGUI;

const ControlKind kImageButtonKind = { 'airy', 'butn' };

TImageButton::TImageButton(HIViewRef inControl)
: TViewNoCompositingCompatible(inControl), mImages(0), mTitle(0), mJustInset(5.0f)
{
    ChangeAutoInvalidateFlags(kAutoInvalidateOnActivate | kAutoInvalidateOnEnable | kAutoInvalidateOnHilite, 0);
    
    // init text style
    memset(&mTextStyle, 0, sizeof(mTextStyle));
    mTextStyle.font = kControlFontSmallSystemFont;
    mTextStyle.just = teJustCenter;
}

//	Clean up after yourself.
TImageButton::~TImageButton()
{
    if (mImages)
        CFRelease(mImages);
    if (mTitle)
        CFRelease(mTitle);
}

ControlKind TImageButton::GetKind()
{
    return kImageButtonKind;
}

// The control is set up.  Do the last minute stuff that needs to be done
// like installing EventLoopTimers.
OSStatus TImageButton::Initialize(TCarbonEvent& inEvent)
{
#pragma unused(inEvent)
    TitleChanged();
    return noErr;
}

// -----------------------------------------------------------------------------
//	ControlHit
// -----------------------------------------------------------------------------
//	ControlHit method.
//
OSStatus TImageButton::ControlHit(
                                  ControlPartCode		inPart,
                                  UInt32				inModifiers )
{
    int val = GetValue();
    if (val == GetMaximum())
	SetValue(GetMinimum());
    else
	SetValue(val+1);
    return TViewNoCompositingCompatible::ControlHit(0, 0);
}


void TImageButton::ValueChanged()
{
    Invalidate();
}

//	The title format is
//	 	filename realTitle justInset
//
// The filename is mandatory
// Integer values are appended to the filename and image files are read
// until a file cannot be found with that name. For example, if the
// filename is "foo", then "foo1", "foo2", etc will be attempted. Once a
// file cannot be found with that name, the search is canceled.
void TImageButton::TitleChanged()
{
    if (!mBundleRef)
        return;
    
    CFStringRef infoString;
    CopyControlTitleAsCFString(GetViewRef(), &infoString);
    
    if (infoString == NULL)
        return;
    
    CFArrayRef info = CFStringCreateArrayBySeparatingStrings(kCFAllocatorDefault, infoString, CFSTR(" "));
    CFIndex count = CFArrayGetCount(info);
    
    // filename
    if (count >= 1)
    {
        if (mImages)
            CFRelease(mImages);
        
        CFMutableArrayRef images = CFArrayCreateMutable(kCFAllocatorDefault, 10, &kCFTypeArrayCallBacks);
        mImages = images;
        
        CFStringRef filename = (CFStringRef)CFArrayGetValueAtIndex(info, 0);
        CFURLRef url = CFBundleCopyResourceURL(mBundleRef, filename, CFSTR("png"), NULL);
        if (url)
        {
            CGImageRef thisImage = TImageCache::GetImage(url);
            CFRelease(url);
            
            if (thisImage)
                CFArrayAppendValue(images, thisImage);
        }
        
        for (int n=1;; ++n)
        {
            CFStringRef searchFilename = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%@%d"), filename, n);
            CFURLRef searchUrl = CFBundleCopyResourceURL(mBundleRef, searchFilename, CFSTR("png"), NULL);
            CFRelease(searchFilename);
            if (!searchUrl)
                break;
            
            CGImageRef searchImage = TImageCache::GetImage(searchUrl);
            CFRelease(searchUrl);
            
            if (searchImage)
                CFArrayAppendValue(images, searchImage);
        }
    }
    
    // title
    if (count >= 2)
    {
        CFMutableStringRef title = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, (CFStringRef)CFArrayGetValueAtIndex(info, 1));
        CFStringFindAndReplace(title, CFSTR("+"), CFSTR(" "), CFRangeMake(0, CFStringGetLength(title)), 0);
        
        if (mTitle)
        {
            CFRelease(mTitle);
            mTitle = NULL;
        }
        
        // only use the title if it's not empty
        if (CFStringGetLength(title) > 0)
            mTitle = title;
        else
            CFRelease(title);
    }
    
    // inset
    if (count >= 3)
    {
        mJustInset = CFStringGetDoubleValue((CFStringRef)CFArrayGetValueAtIndex(info, 2));
    }
    
    // clean up
    CFRelease(info);
    CFRelease(infoString);
    
    // force a redraw
    Invalidate();
}

OSStatus TImageButton::SetData(OSType inTag, ControlPartCode inPart, Size inSize, const void* inPtr)
{
    OSStatus err = noErr;
    switch (inTag)
    {
        case kControlEditTextCFStringTag:
            if (inSize == sizeof(CFStringRef))
            {
                if (mTitle)
                {
                    CFRelease(mTitle);
                    mTitle = NULL;
                }
                mTitle = *(CFStringRef *)inPtr;
                CFRetain(mTitle);
                Invalidate();
            }
            else
                err = errDataSizeMismatch;
            break;
            
        case kControlFontStyleTag:
            if (inSize == sizeof(ControlFontStyleRec))
            {
                memcpy(&mTextStyle, inPtr, inSize);
                Invalidate();
            }
            else
                err = errDataSizeMismatch;
            break;
            
        default:
            // unsupported data tag
            err = errDataNotSupported;
    }
    return err;
}

OSStatus TImageButton::GetData(OSType inTag, ControlPartCode inPart, Size inSize, Size* outSize, void* inPtr)
{
    OSStatus err = noErr;
    switch (inTag)
    {
        case kControlEditTextCFStringTag:
            if (inSize == sizeof(CFStringRef))
            {
                if (inPtr)
                    *(CFStringRef *)inPtr = mTitle;
                if (outSize)
                    *outSize = sizeof(CFStringRef);
            }
            else
                err = errDataSizeMismatch;
            break;
            
        case kControlFontStyleTag:
            if (inSize == sizeof(ControlFontStyleRec))
            {
                if (inPtr)
                    memcpy(inPtr, &mTextStyle, inSize);
                if (outSize)
                    *outSize = sizeof(ControlFontStyleRec);
            }
            else
                err = errDataSizeMismatch;
            break;
            
        default:
            err = errDataNotSupported;
    }
    return err;
}

//! The fun part of the control
void TImageButton::CompatibleDraw(RgnHandle inLimitRgn, CGContextRef inContext, bool inCompositing)
{
#pragma unused(inLimitRgn)
    TRect bounds = Bounds();
    int value = GetValue();
    ControlPartCode hilite = GetHilite();
    
    if (hilite == kControlButtonPart)
        value = 0;
    
    if (mImages)
    {
        if (value < 0 || value >= CFArrayGetCount(mImages))
            value = 0;
        
        CGImageRef valueImage = 0;
        if (CFArrayGetCount(mImages))
            valueImage = (CGImageRef)CFArrayGetValueAtIndex(mImages, value);
        if (valueImage)
        {
            HIViewDrawCGImage(inContext, &bounds, valueImage);
        }
    }
    
    // draw title text
    if (mTitle)
    {
        ThemeDrawState state = kThemeStateActive;
        if (hilite == kControlInactivePart || !IsEnabled())
            state = kThemeStateInactive;
        
        CGContextSetRGBStrokeColor(inContext, 0.0f, 0.0f, 0.0f, 1.0f);
        CGContextSetRGBFillColor(inContext, 0.0f, 0.0f, 0.0f, 1.0f);
        
        SetFontFromFontStyleRec(inContext, mTextStyle, kThemeSmallSystemFont, kThemeTextColorPushButtonActive);
        
        // get string to draw
        int textLength = CFStringGetLength(mTitle);
        const char *textPtr = CFStringGetCStringPtr(mTitle, CFStringGetSystemEncoding());
        char *textBuffer = NULL;
        if (textPtr == NULL)
        {
            textPtr = textBuffer = new char[textLength + 1];
            CFStringGetCString(mTitle, textBuffer, textLength+1, CFStringGetSystemEncoding());
        }
        
        // measure string in the font
        Rect qdTextBounds;
        FontInfo fontInfo;
        QDTextBounds(textLength, textPtr, &qdTextBounds);
        GetFontInfo(&fontInfo);
        
        // get text bounding rect
        float textWidth = float(qdTextBounds.right - qdTextBounds.left);
        float textHeight = float(qdTextBounds.bottom - qdTextBounds.top);
        TRect textBounds = Bounds();
        textBounds.SetHeight(textHeight);
        textBounds.MoveBy(0.0f, (bounds.Height() - textBounds.Height()) / 2.0f - fontInfo.descent);
        
        //		printf("text w = %g; h = %g\n", textWidth, textHeight);
        
        // apply justification
        bool useJust = mTextStyle.flags & kControlUseJustMask;
        int theJust = useJust ? mTextStyle.just : teJustCenter;
        switch (theJust)
        {
            case teJustLeft:
                textBounds.MoveBy(mJustInset, 0.0f);
                break;
            case teJustRight:
                textBounds.MoveBy(textBounds.Width() - mJustInset - textWidth, 0.0f);
                break;
            case teJustCenter:
            default:
                textBounds.MoveBy(mJustInset + (textBounds.Width() - mJustInset * 2.0f - textWidth) / 2.0f, 0.0f);
                break;
        }
        
        // draw text
        //		Rect qdBounds = textBounds;
        //		MoveTo(qdBounds.left, qdBounds.bottom);
        //		DrawText(textPtr, 0, textLength);
        
        //		CGContextSelectFont(inContext, "Silkscreen", 11.0, kCGEncodingMacRoman);
        //
        //		CGAffineTransform textMatrix = CGAffineTransformMakeTranslation(bounds.MinX(), (bounds.MinY() + bounds.MaxY()) / 2.0f);
        //		textMatrix = CGAffineTransformRotate(textMatrix, 180.0f);
        //		
        CGAffineTransform textMatrix = CGContextGetTextMatrix(inContext);
        textMatrix = CGAffineTransformScale(textMatrix, 1.0, -1.0);
        CGContextSetTextMatrix(inContext, textMatrix);
        
        CGContextSetTextDrawingMode(inContext, kCGTextFill);
        CGContextShowTextAtPoint(inContext, textBounds.MinX(), textBounds.MaxY(), textPtr, textLength);
        //		CGContextShowTextAtPoint(inContext, (bounds.MinX() + bounds.MaxX()) / 2.0f - textWidth / 2.0f, (bounds.MinY() + bounds.MaxY()) / 2.0f + textHeight / 2.0f - fontInfo.descent, textPtr, textLength);
        
        // clean up
        if (textBuffer)
            delete [] textBuffer;
    }
}

// Just force a redraw.
OSStatus TImageButton::HiliteChanged(ControlPartCode inOriginalPart, ControlPartCode inCurrentPart, RgnHandle inInvalRgn)
{
    Invalidate();
    return noErr;
}

OSStatus TImageButton::Track(TCarbonEvent& inEvent, ControlPartCode* outPart)
{
    return eventNotHandledErr;
}
