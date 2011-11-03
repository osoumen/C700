/*
 *  TViewNoCompositingCompatible.cpp
 *  NeuSynth
 *
 *  Created by Airy AndrŽ on Wed Nov 06 2002.
 *  Copyright (c) 2002 Airy AndrŽ. All rights reserved.
 *
 */

#include "TViewNoCompositingCompatible.h"
#include "TImageCache.h"
#include "TRect.h"
#include "AUGUIUtilities.h"
#include "AUProperty.h"

const AUGUI::property_t TViewNoCompositingCompatible::mPropertiesList[] = {
				AUGUI::property_t('titl', CFSTR("title"), CFSTR("Title"), AUGUI::kString),
				AUGUI::property_t('fram', CFSTR("frame"), CFSTR("Frame"), AUGUI::kRect),
				AUGUI::property_t('enab', CFSTR("enabled"), CFSTR("Enabled"), AUGUI::kBool),
				AUGUI::property_t('min ', CFSTR("min"), CFSTR("Minimum"), AUGUI::kInteger),
				AUGUI::property_t('max ', CFSTR("max"), CFSTR("Maximum"), AUGUI::kInteger),
				AUGUI::property_t('val ', CFSTR("val"), CFSTR("Value"), AUGUI::kInteger),
				AUGUI::property_t()
};
const size_t TViewNoCompositingCompatible::mPropertiesListSize = sizeof(mPropertiesList)/sizeof(AUGUI::property_t)-1;

//-----------------------------------------------------------------------------------
//	constants (needed for the register/unregister stuff)
//-----------------------------------------------------------------------------------
//
static const EventTypeSpec kHIObjectEvents[] =
{
{ kEventClassHIObject, kEventHIObjectConstruct },
{ kEventClassHIObject, kEventHIObjectInitialize },
{ kEventClassHIObject, kEventHIObjectDestruct },
{ kEventClassHIObject, kEventHIObjectPrintDebugInfo }
};

CFMutableDictionaryRef TViewNoCompositingCompatible::mRegisteryCache = 0;
bool TViewNoCompositingCompatible::sWindowReady = true;
CFBundleRef TViewNoCompositingCompatible::sStaticBundle = 0;

TViewNoCompositingCompatible::TViewNoCompositingCompatible( HIViewRef inControl ) : TView( inControl ), mMouseEventHandler(0), mStaticBackground(0)
{
	mSaveMinMaxVal = false;
	mCurrentFocusPart = kControlNoPart;
    mBundleRef = sStaticBundle;
    if (mBundleRef)
        CFRetain(mBundleRef);
};

//	Clean up after yourself.
TViewNoCompositingCompatible::~TViewNoCompositingCompatible()
{
    if (mBundleRef)
        CFRelease(mBundleRef);
    if (mStaticBackground)
        CFRelease(mStaticBackground);
}

void TViewNoCompositingCompatible::SetClassBundle(CFBundleRef sBundle)
{
    if (sStaticBundle)
        CFRelease(sBundle);
    
    if (sBundle)
        CFRetain(sBundle);
    sStaticBundle = sBundle;
}

OSStatus TViewNoCompositingCompatible::NewView(
													  HIViewRef*		outControl,
													  const HIRect*		inBounds,
													  HIViewRef			inView,
													  CFStringRef 	classID)
{
	OSStatus			err;
	EventRef			event = CreateInitializationEvent();
	
	err = HIObjectCreate( classID, event, (HIObjectRef*) outControl );
	
	ReleaseEvent( event );
	
	require_noerr( err, CantCreateHIObject );
	
	if ( inView != NULL )
		HIViewAddSubview( inView, *outControl );
	
	if ( inBounds != NULL )
		HIViewSetFrame( *outControl, inBounds );
	
CantCreateHIObject:
		
		return err;
}	

OSStatus TViewNoCompositingCompatible::NewViewForCurrentBundle(
											   HIViewRef*		outControl,
											   const HIRect*		inBounds,
											   HIViewRef			inView,
											   CFStringRef 	controlName)
{
	OSStatus			err;
	CFStringRef id = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%@.%@"), CFBundleGetIdentifier(sStaticBundle), controlName);
	err = NewView(outControl, inBounds, inView, id);
	CFRelease(id);
	return err;
}	



//-----------------------------------------------------------------------------------
//	FlushStaticBackground
//-----------------------------------------------------------------------------------
//! Force the static background to be rebuild
//
void TViewNoCompositingCompatible::FlushStaticBackground()
{
    if (mStaticBackground) {
        CFRelease(mStaticBackground);
        mStaticBackground = 0;
		ForceRedraw();
        // We should also flush all of our subviews static backgrounds
		int i = 1;
		HIViewRef ctrl;
		while (GetIndexedSubControl(GetViewRef(), i++, &ctrl) == noErr) {
			TViewNoCompositingCompatible *tview = 0;
			OSErr err = GetControlData(ctrl, 0, theViewTag, sizeof(tview), &tview, 0);
			if (err == noErr && tview) {
				tview->FlushStaticBackground();
			}
		}
    }
}

//-----------------------------------------------------------------------------------
//	mMouseEventHandler
//-----------------------------------------------------------------------------------
//!	Our static mouseDragged event handler proc.
//
OSStatus TViewNoCompositingCompatible::mouseDragged(TCarbonEvent&inEvent)
{
    return StillTracking(inEvent, mFromPoint);
}

//!	Our static mouseUp event handler proc.
OSStatus TViewNoCompositingCompatible::mouseUp(TCarbonEvent&inEvent)
{
    // Remove mouse handler...
    if (mMouseEventHandler) {
        RemoveEventHandler(mMouseEventHandler);
        mMouseEventHandler = 0;
    }
    
    OSStatus result = StopTracking(inEvent, mFromPoint);
    
    // create a control hit event to simulate a normal Track call
    if (result == noErr)
    {
        TCarbonEvent hitEvent(kEventClassControl, kEventControlHit);
        
        // get values from the in event to pass to the new event
        ControlPartCode hitPart = kControlNoPart;
        inEvent.GetParameter(kEventParamControlPart, &hitPart);
        UInt32 modifiers = 0;
        inEvent.GetParameter(kEventParamKeyModifiers, &modifiers);
        
        // set up parameters
        hitEvent.SetParameter<ControlRef>(kEventParamDirectObject, typeControlRef, GetViewRef());
        hitEvent.SetParameter(kEventParamControlPart, hitPart);
        hitEvent.SetParameter(kEventParamKeyModifiers, modifiers);
        
        // post to the current event queue
        hitEvent.PostToQueue(GetCurrentEventQueue());
    }
    return result;
}

static void releaseImageBuffer(void * info, const void* data, size_t dataSize)
{
    if (data)
        free(info); //const_cast<void*>(data));
}

//!	Draw background image into the context. Only the background part of the image hierarchy is taken in
//! account (we suppose no visible part of controls are overlapping)
void TViewNoCompositingCompatible::DrawBackgroundImage(CGContextRef context)
{
    
    HIViewRef superView = HIViewGetSuperview(GetViewRef());
    TViewNoCompositingCompatible *tview = 0;
    // Find our first superview which is a TViewNoCompositingCompatible
    while (superView && !tview) {
        OSErr err = GetControlData(superView, 0, theViewTag, sizeof(tview), &tview, 0);
        if (err != noErr) {
            superView = HIViewGetSuperview(superView);
        }
    }
    if (tview)
        tview->DrawBackgroundImage(context);
    DrawStaticBackground(NULL, context, IsCompositing());
}


//!	Create the offscreen CGImageRef containing the background of the control
//!	to speed up redraw. It does include the window background and the static
//!	part of the control, drawn by DrawStaticBackground method
void TViewNoCompositingCompatible::PrerenderStaticBackground()
{
    if (!sWindowReady) // Don't start compositing before the window is ready
        return;
    TRect bounds = Bounds();
    
    // get image info
    size_t width = (size_t)bounds.Width();
    size_t height = (size_t)bounds.Height();
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    size_t bitsPerComponent = 8;
    size_t bitsPerPixel = 32;
    size_t bytesPerRow = ((width * bitsPerPixel) + 7) / 8;
    CGImageAlphaInfo alphaInfo = kCGImageAlphaNoneSkipFirst;
    
    // create buffer and context
    size_t dataSize = bytesPerRow * height;
    void *buffer;
    void *data = AUGUI::malloc_aligned(dataSize, &buffer);
    if (!data)
        return;
    
    CGContextRef context = CGBitmapContextCreate(data, width, height, bitsPerComponent, bytesPerRow, colorSpace, alphaInfo);
    if (!context)
        return;
    
    // draw image into context
    CGContextTranslateCTM(context, 0, bounds.size.height);
    CGContextScaleCTM(context, 1.f, -1.f);
    CGContextTranslateCTM(context, -bounds.MinX(), -bounds.MinY());
    // We first fill the background with white color. This is for Controls not embedded in a pict control.
    // We suppose the host window is plain white.
    CGContextSetRGBFillColor(context, 1.f, 1.f, 1.f, 1.f);
    CGContextFillRect(context, bounds);
    DrawBackgroundImage(context);
    CGContextRelease(context);
    
    
    // create data provider for this image buffer
    CGDataProviderRef provider = CGDataProviderCreateWithData(buffer, data, dataSize, releaseImageBuffer);
    mStaticBackground = CGImageCreate(width, height, bitsPerComponent, bitsPerPixel, bytesPerRow, colorSpace, alphaInfo, provider, NULL, true,
                                      kCGRenderingIntentDefault);
    CGDataProviderRelease(provider);
    
    CFRelease(colorSpace);
}


//! This is the event handler that is registered for the MouseUp and
//! MouseDragged events by the \c SetUpTracking member function.
pascal OSStatus TViewNoCompositingCompatible::MouseEventHandler(EventHandlerCallRef	inCallRef,
                                                                EventRef			inEvent,
                                                                void*				inUserData)
{
    OSStatus			result = eventNotHandledErr;
    TViewNoCompositingCompatible*				view = (TViewNoCompositingCompatible*) inUserData;
    TCarbonEvent		event( inEvent );
    
    switch ( event.GetClass() )
    {
        case kEventClassMouse:
            switch ( event.GetKind() )
            {
                case kEventMouseUp:
                    result = view->mouseUp(event);
                    break;
                    
                case kEventMouseDragged:
                    result = view->mouseDragged(event);
                    break;
            }
            break;
    }
    return result;
}
//-----------------------------------------------------------------------------------
//	RegisterSubclass
//-----------------------------------------------------------------------------------
//!	This routine should be called by subclasses so they can be created as HIObjects.
//
OSStatus TViewNoCompositingCompatible::RegisterSubclass(CFStringRef		inID,
                                                        ConstructProc	inProc)
{
    HIObjectClassRef classRef;
    
	if (mRegisteryCache && CFDictionaryContainsKey(mRegisteryCache, inID))
		return noErr;

    OSStatus res = HIObjectRegisterSubclass( inID, kHIViewClassID, 0, ObjectEventHandler,
                                             GetEventTypeCount( kHIObjectEvents ), kHIObjectEvents, (void*) inProc, &classRef );
    if (res == noErr) {
        // Add (inId,classRef) in a dictionnary
        if (mRegisteryCache == 0) {
            mRegisteryCache = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        }
        CFNumberRef number = CFNumberCreate(NULL, kCFNumberLongLongType, &classRef);
        if (CFDictionaryContainsKey(mRegisteryCache, inID))
            CFDictionaryReplaceValue(mRegisteryCache, inID, number);
        else
            CFDictionaryAddValue(mRegisteryCache, inID, number);
        CFRelease(number);
    }
    return res;
}
//-----------------------------------------------------------------------------------
//	UnRegisterSubclass
//-----------------------------------------------------------------------------------
//!	This routine should be called by subclasses to unregister themself.
//
OSStatus TViewNoCompositingCompatible::UnRegisterSubclass(CFStringRef inID)
{
    OSStatus err = noErr;
    // Remove (inId,classRef) form the dictionnary and unregister classRef
    HIObjectClassRef classRef = 0;
    CFNumberRef number;
    if (CFDictionaryGetValueIfPresent(mRegisteryCache, inID, (const void **)&number)) {
        void* val;
        CFNumberGetValue(number, kCFNumberLongLongType, &val);
        classRef = HIObjectClassRef(val);
        if (classRef) err =  HIObjectUnregisterClass( classRef );
        CFDictionaryRemoveValue(mRegisteryCache, inID);
    }
    return err;
}

//-----------------------------------------------------------------------------------
//	SetUpTracking
//-----------------------------------------------------------------------------------
//!	Install handler for MouseUp and MouseDragged (for mouse tracking)
//
OSStatus TViewNoCompositingCompatible::SetUpTracking(TCarbonEvent& inEvent)
{
    static const EventTypeSpec kHIMouseEvents[] = {
    { kEventClassMouse, kEventMouseUp },
    { kEventClassMouse, kEventMouseDragged }
    };
    
    InstallEventHandler(
                        GetWindowEventTarget(GetOwner()),
                        MouseEventHandler,
                        GetEventTypeCount(kHIMouseEvents),
                        kHIMouseEvents,
                        this,
                        &mMouseEventHandler);
    inEvent.GetParameter<HIPoint>( kEventParamMouseLocation, typeHIPoint, &mFromPoint );
    if (!IsCompositing()) { // When called in a not compositing window : translate the coord to HIView local coords.
        TRect frame = Frame();
        mFromPoint.x -= frame.MinX();
        mFromPoint.y -= frame.MinY();
    }
    StartTracking(inEvent, mFromPoint);
    
    return noErr;
}

//-----------------------------------------------------------------------------------
//	Track
//-----------------------------------------------------------------------------------
//
OSStatus TViewNoCompositingCompatible::Track(TCarbonEvent& inEvent, ControlPartCode* outPart)
{
    *outPart = kControlNoPart;
    
    if (UseNonblockingTracking())
        return SetUpTracking(inEvent);
    else
        return eventNotHandledErr;
}


// To be overriden..
//-----------------------------------------------------------------------------------
//	StartTracking
//-----------------------------------------------------------------------------------
//!	Called at the beginning of tracking
//
OSStatus TViewNoCompositingCompatible::StartTracking(TCarbonEvent&inEvent, HIPoint& from)
{
    return noErr;
}

//-----------------------------------------------------------------------------------
//	StillTracking
//-----------------------------------------------------------------------------------
//!	Called whenever the mouse is moving during tracking
//
OSStatus TViewNoCompositingCompatible::StillTracking(TCarbonEvent&inEvent, HIPoint& from)
{
    return noErr;
}


//-----------------------------------------------------------------------------------
//	StopTracking
//-----------------------------------------------------------------------------------
//!	Called at the end of tracking. Our default implementation calls the HitTest()
//! method to determine which part of the control the mouse was over when the
//! button was released. This part code is set in \a inEvent.
//
OSStatus TViewNoCompositingCompatible::StopTracking(TCarbonEvent&inEvent, HIPoint& from)
{
    HIPoint mouse;
    inEvent.GetParameter<HIPoint>(kEventParamWindowMouseLocation, typeHIPoint, &mouse);
    ControlPartCode wherePart = HitTest(mouse);
    inEvent.SetParameter<ControlPartCode>(kEventParamControlPart, typeControlPartCode, wherePart);
    return noErr;
}

//-----------------------------------------------------------------------------------
//	Draw
//-----------------------------------------------------------------------------------
//!	Build a context for non-compositing window, draw the background and call the CompatibleDraw method
//
void TViewNoCompositingCompatible::Draw(RgnHandle		inLimitRgn,
                                        CGContextRef	inContext)
{
    if (!sWindowReady) // Don't start drawing before the window is ready
        return;
	if (!GetOwner())
		return;

    CGContextRef			context = inContext;
    GrafPtr graphPtr = 0;
    GrafPtr oldPort = 0;
    bool compositing = (inContext != 0);
	
    TRect bounds = Bounds();
    
    if (!compositing) {
        GetPort(&oldPort);
        SetPortWindowPort(GetOwner());
        Rect r = bounds;
        EraseRect(&r);
        if (mStaticBackground == 0) {
			PrerenderStaticBackground();
        }
    }
    if (!compositing) { // When called in a not compositing window
        graphPtr=GetWindowPort(GetOwner());
        
        Rect rect;
        GetPortBounds(graphPtr, &rect);
        
        QDBeginCGContext(graphPtr, &context);
        CGContextTranslateCTM(context, 0.f, float(rect.bottom-rect.top));
        CGContextScaleCTM(context, 1.f, -1.f);
        if (mStaticBackground)
            HIViewDrawCGImage(context, &bounds, mStaticBackground);
        else
            DrawStaticBackground(inLimitRgn, context, compositing);
	    CGContextClipToRect(context, bounds);
    } else {
        CGContextSaveGState(inContext);
        DrawStaticBackground(inLimitRgn, context, compositing);
        CGContextRestoreGState(inContext);
    }
    // Call the real draw routine
    CGContextSaveGState(context);
    CompatibleDraw(inLimitRgn, context, compositing);
    CGContextRestoreGState(context);
    
    if (!compositing) {
        CGContextSynchronize ( context );
        QDEndCGContext (graphPtr, &context );
        SetPort(oldPort);
    }
	
    // Draw embeded controls
    int i = 1;
    HIViewRef ctrl;
    while (GetIndexedSubControl(GetViewRef(), i++, &ctrl) == noErr) {
		if (!compositing) {
			Draw1Control(ctrl);
		} else {
			HIViewSetNeedsDisplay(ctrl, true);
		}
    }
}

//-----------------------------------------------------------------------------------
//	UnregisterAllClasses
//-----------------------------------------------------------------------------------
//!	Unregister all classes registred before
//
static void unregister(CFStringRef key, CFNumberRef value, void *context)
{
    void *val;
    CFNumberGetValue(value, kCFNumberLongLongType, &val);
    HIObjectClassRef classRef = HIObjectClassRef(val);
    if (classRef) {
        HIObjectUnregisterClass( classRef );
    }
}

void TViewNoCompositingCompatible::UnRegisterAllClasses()
{
	if (mRegisteryCache) {
		CFDictionaryApplyFunction(mRegisteryCache, (CFDictionaryApplierFunction)unregister, 0);
		CFDictionaryRemoveAllValues(mRegisteryCache);
	}
}

//-----------------------------------------------------------------------------------
//	IsCompositing
//-----------------------------------------------------------------------------------
//!	Return true if the view is in a compositing window.
//
bool TViewNoCompositingCompatible::IsCompositing()
{
    WindowAttributes attributes;
    GetWindowAttributes(GetOwner(), &attributes);
    return attributes & kWindowCompositingAttribute;
}

//-----------------------------------------------------------------------------------
//	ConvertToLocal
//-----------------------------------------------------------------------------------
//!	Convert the point to view local (x,y).
//
void TViewNoCompositingCompatible::ConvertToLocal(HIPoint& thePoint)
{
    if (!IsCompositing()) { // When called in a not compositing window : convert to HIView local coords
        TRect frame = Frame();
        thePoint.x -= frame.MinX();
        thePoint.y -= frame.MinY();
    }
    HIViewConvertPoint(&thePoint, NULL, GetViewRef());
}

//-----------------------------------------------------------------------------------
//	ConvertToGlobal
//-----------------------------------------------------------------------------------
//!	Convert the local point to window (x,y).
//
void TViewNoCompositingCompatible::ConvertToGlobal(HIPoint& thePoint)
{
    HIViewConvertPoint(&thePoint, GetViewRef(), NULL);
}

//-----------------------------------------------------------------------------------
//	GetData
//-----------------------------------------------------------------------------------
//!	Get some data about the view
//! Only 'TVew' is supported. It is returning the C++ object.
//
OSStatus TViewNoCompositingCompatible::GetData(OSType inTag, ControlPartCode inPart, Size inSize, Size* outSize, void* inPtr)
{
    OSStatus err = noErr;
    
    switch (inTag)
    {
        case theViewTag:
            if (inSize == sizeof(TViewNoCompositingCompatible *))
            {
                if (inPtr)
                    *(TViewNoCompositingCompatible **)inPtr = this;
                if (outSize)
                    *outSize = sizeof(TViewNoCompositingCompatible *);
            }
            else
                err = errDataSizeMismatch;
            break;
        default:
            err = TView::GetData(inTag, inPart, inSize, outSize, inPtr);
    }
    return err;
}

//-----------------------------------------------------------------------------------
//	ControlHit
//-----------------------------------------------------------------------------------
//	The was hit.  Subclasses can overide to care about what part was hit.
//
OSStatus TViewNoCompositingCompatible::ControlHit(ControlPartCode	inPart,
                                                  UInt32			inModifiers)
{
    OSStatus err = noErr;
    HICommandExtended theCommand;
    
    if (IsControlEnabled(GetViewRef())) {
        err = GetControlCommandID( GetViewRef(), &theCommand.commandID );
        if (err == noErr && theCommand.commandID != '\0\0\0\0') {
            EventRef commandEvent;
            err = CreateEvent( NULL, kEventClassCommand, kEventProcessCommand,
                               GetCurrentEventTime(), kEventAttributeNone, &commandEvent );
            require_noerr( err, CantCreateEvent );
            
            theCommand.attributes =  kHICommandFromControl;
            theCommand.source.control =  GetViewRef();
            err = SetEventParameter( commandEvent, kEventParamDirectObject,
                                     typeHICommand, sizeof( HICommand ), &theCommand );
            require_noerr( err, CantSetEventParameter );
            
            err = SendEventToEventTargetWithOptions( commandEvent, GetControlEventTarget(GetViewRef()) , kEventTargetSendToAllHandlers);
        }
		return noErr;
    }
	
	CantSetEventParameter:
	CantCreateEvent:
	
	return eventNotHandledErr;
}

// Asks your view to return what part of itself (if any) is hit by the point
// given to it. The point is in VIEW coordinates, so you should get the view
// srect to do bounds checking.
ControlPartCode TViewNoCompositingCompatible::HitTest(const HIPoint& inWhere)
{
    ControlPartCode part;
    // is the mouse on the button?
    if (CGRectContainsPoint(Bounds(), inWhere))
        part = kControlButtonPart;
    else
        part = kControlNoPart;
    
    return part;
}

//-----------------------------------------------------------------------------------
//	ForceRedraw : force a redraw of the control
//-----------------------------------------------------------------------------------
//
void TViewNoCompositingCompatible::ForceRedraw()
{
	if (!IsCompositing())
	{
		Hide();
		Show();
	}
	else
	{
		Invalidate(); // needed for compositing windows
	}
}

//-----------------------------------------------------------------------------------
//	ForceValueChange : post a value-changed message even if the value has not changed
//-----------------------------------------------------------------------------------
//
void TViewNoCompositingCompatible::ForceValueChange()
{
	TCarbonEvent event(kEventClassControl, kEventControlValueFieldChanged);
	
	event.SetParameter<ControlRef>(kEventParamDirectObject, typeControlRef, GetViewRef());
	event.PostToQueue(GetCurrentEventQueue());
	ControlHit(0, 0);
}

//-----------------------------------------------------------------------------------
//	WriteToDictionary : save properties to dictionary
//-----------------------------------------------------------------------------------
void TViewNoCompositingCompatible::WriteToDictionary(CACFDictionary &dict)
{
	// Write type
	dict.AddString(CFSTR("type"), GetType());
			
	UInt32 commandID;
	GetControlCommandID(GetViewRef(), &commandID);
	if (commandID) {
		CFStringRef str =  UTCreateStringForOSType(commandID);
		dict.AddString(CFSTR("callback"), str);
		CFRelease(str);
	}
	
	CACFDictionary idDict(true);
	HIViewID id = {0, 0};
	GetControlID(GetViewRef(), &id);
	if (id.signature)
	{
		CFStringRef str =  UTCreateStringForOSType(id.signature);
		idDict.AddString(CFSTR("signature"), str);
		CFRelease(str);
		idDict.AddSInt32(CFSTR("id"), id.id);
		dict.AddDictionary(CFSTR("id"), idDict.GetDict());
	}
	
	size_t s = GetPropertiesListSize();
	for (size_t i = 0; i < s; ++i) {
		AUGUI::property_t prop = GetPropertyDefinition(i);
		switch (prop.type()) {
			case AUGUI::kString:
			case AUGUI::kPicture:
				CFStringRef val;
				if (GetProperty(prop.tag(), val)) {
					dict.AddString(prop.name(), val);
					CFRelease(val);
				}
				break;
			case AUGUI::kFloat: {
				double val;
				if (GetProperty(prop.tag(), val)) {
					dict.AddFloat64(prop.name(), val);
				}
			}
					break;
			case AUGUI::kBool:
			case AUGUI::kInteger: {
				int32_t val;
				if (GetProperty(prop.tag(), val)) {
					SInt32 v = val;
					dict.AddSInt32(prop.name(), v);
				}
			}
					break;
			case AUGUI::kColor: {
				AUGUI::color_t val;
				if (GetProperty(prop.tag(), val)) {
					CACFDictionary element(true);
					element.AddFloat32(CFSTR("r"), val.red);
					element.AddFloat32(CFSTR("g"), val.green);
					element.AddFloat32(CFSTR("b"), val.blue);
					element.AddFloat32(CFSTR("a"), val.alpha);
					dict.AddDictionary(prop.name(), element.GetDict());
				}
			}
					break;
			case AUGUI::kFont: {
				AUGUI::font_t val;
				if (GetProperty(prop.tag(), val)) {
					CACFDictionary element(true);
					element.AddString(CFSTR("name"), val.name);
					element.AddSInt32(CFSTR("size"), val.size);
					dict.AddDictionary(prop.name(), element.GetDict());
					CFRelease(val.name);
				}
			}				
				break;
			case AUGUI::kPoint: {
				AUGUI::point_t val;
				if (GetProperty(prop.tag(), val)) {
					CACFDictionary element(true);
					element.AddFloat32(CFSTR("x"), val.x);
					element.AddFloat32(CFSTR("y"), val.y);
					dict.AddDictionary(prop.name(), element.GetDict());
				}
			}
					break;
			case AUGUI::kRect: {
				AUGUI::rect_t val;
				if (GetProperty(prop.tag(), val)) {
					CACFDictionary element(true);
					element.AddFloat32(CFSTR("x"), val.x);
					element.AddFloat32(CFSTR("y"), val.y);
					element.AddFloat32(CFSTR("w"), val.w);
					element.AddFloat32(CFSTR("h"), val.h);
					dict.AddDictionary(prop.name(), element.GetDict());
				}
			}
				break;
		}
	}
}

//-----------------------------------------------------------------------------------
//	ReadFromDictionary : read properties from dictionary
//-----------------------------------------------------------------------------------
void TViewNoCompositingCompatible::ReadFromDictionary(CACFDictionary &dict)
{
	// Read command
	CFStringRef cb;
	if (dict.GetString(CFSTR("callback"), cb)) {
		UInt32 commandID = UTGetOSTypeFromString(cb);
		SetControlCommandID(GetViewRef(), commandID);
	}
	
	// Read id
	CFDictionaryRef i = 0;
	if (dict.GetDictionary(CFSTR("id"), i)) {
		CACFDictionary element(i, true);

		HIViewID id = {0, 0};
		
		CFStringRef sig;
		element.GetString(CFSTR("signature"), sig);
		id.signature = UTGetOSTypeFromString(sig);
		
		element.GetSInt32(CFSTR("id"), id.id);
		
		SetControlID(GetViewRef(), &id);
	}	
	
	size_t s = GetPropertiesListSize();
	for (size_t i = 0; i < s; ++i) {
		AUGUI::property_t prop = GetPropertyDefinition(i);
		switch (prop.type()) {
			case AUGUI::kString:
			case AUGUI::kPicture: {
				CFStringRef val;
				if (dict.GetString(prop.name(), val)) {
					SetProperty(prop.tag(), val);
				}
			}
					break;
			case AUGUI::kFloat: {
				double val;
				if (dict.GetFloat64(prop.name(), val)) {
					SetProperty(prop.tag(), val);
				}
			}
					break;				
				break;
			case AUGUI::kBool:
			case AUGUI::kInteger: {
				SInt32 val;
				if (dict.GetSInt32(prop.name(), val)) {
					int32_t v = val;
					SetProperty(prop.tag(), v);
				}
			}
					break;				
				break;
			case AUGUI::kColor:{
				AUGUI::color_t c;
				CFDictionaryRef f = 0;
				if (dict.GetDictionary(prop.name(), f)) {
					CACFDictionary element(f, true);
					element.GetFloat32(CFSTR("a"), c.alpha);
					element.GetFloat32(CFSTR("r"), c.red);
					element.GetFloat32(CFSTR("b"), c.blue);
					element.GetFloat32(CFSTR("g"), c.green);					
					SetProperty(prop.tag(), c);
				}
			}
				break;
			case AUGUI::kFont:{
				AUGUI::font_t font;
				CFDictionaryRef f = 0;
				if (dict.GetDictionary(prop.name(), f)) {
					CACFDictionary element(f, true);
					element.GetString(CFSTR("name"), font.name);
					element.GetSInt32(CFSTR("size"), font.size);
					SetProperty(prop.tag(), font);
				}
			}
				break;
			case AUGUI::kPoint:{
				AUGUI::point_t p;
				CFDictionaryRef f = 0;
				if (dict.GetDictionary(prop.name(), f)) {
					CACFDictionary element(f, true);
					element.GetFloat32(CFSTR("x"), p.x);
					element.GetFloat32(CFSTR("y"), p.y);
					SetProperty(prop.tag(), p);
				}
			}
				break;
			case AUGUI::kRect:{
				AUGUI::rect_t r;
				CFDictionaryRef f = 0;
				if (dict.GetDictionary(prop.name(), f)) {
					CACFDictionary element(f, true);
					element.GetFloat32(CFSTR("x"), r.x);
					element.GetFloat32(CFSTR("y"), r.y);
					element.GetFloat32(CFSTR("w"), r.w);
					element.GetFloat32(CFSTR("h"), r.h);					
					SetProperty(prop.tag(), r);
				}
			}
				break;
		}
	}
	PropertiesChanged();	
}

OSStatus TViewNoCompositingCompatible::SetFocusPart(ControlPartCode inDesiredFocus, Boolean inFocusEverything, ControlPartCode* outActualFocus)
{
	OSStatus err = noErr;
	
	// resolve the desired focus
	switch( inDesiredFocus )
	{		
		case kControlFocusNextPart:
		case kControlFocusPrevPart: // toggle between focused or not
			if( (mCurrentFocusPart == kControlNoPart) && inFocusEverything )
				inDesiredFocus = kControlEditTextPart;
			else
				inDesiredFocus = kControlNoPart;
			break;
			
		default:  // we don't focus anything but the button
			if( inDesiredFocus != kControlEditTextPart )
				inDesiredFocus = kControlNoPart;
			break;
	}
	
	// handle invalidation if focus changed
	if( mCurrentFocusPart != inDesiredFocus )
	{
		mCurrentFocusPart = inDesiredFocus;
		
		err = Invalidate();
	}
	
	if( err == noErr )
		*outActualFocus = mCurrentFocusPart;
	return err;
}

//-----------------------------------------------------------------------------------
//	Properties management
//-----------------------------------------------------------------------------------
 void TViewNoCompositingCompatible::SetProperty(OSType propID, double value)
{
	switch(propID) {
		default:
			break;
	};
}

 void TViewNoCompositingCompatible::SetProperty(OSType propID, int32_t value)
{
	switch(propID) {
		case 'enab':
			if (value)
				EnableControl(GetViewRef());
			else
				DisableControl(GetViewRef());
			break;
		case 'min ':
			if (mSaveMinMaxVal)
				SetValue(value);
			break;
		case 'max ':
			if (mSaveMinMaxVal)
				SetMaximum(value);
			break;
		case 'val ':
			if (mSaveMinMaxVal)
				SetMinimum(value);
			break;
		default:
			break;
	};
}

 void TViewNoCompositingCompatible::SetProperty(OSType propID, CFStringRef value)
{
	switch(propID) {
		case 'titl':
			SetControlTitleWithCFString(GetViewRef(), value);
			TitleChanged();
			break;
		default:
			break;
	};
}

void TViewNoCompositingCompatible::SetProperty(OSType propID, AUGUI::font_t& font)
{
	switch(propID) {
		default:
			break;
	};
}

void TViewNoCompositingCompatible::SetProperty(OSType propID, AUGUI::rect_t& r)
{
	switch(propID) {
		case 'fram':
			HIRect frame;
			frame.origin.x = r.x;
			frame.origin.y = r.y;
			frame.size.width = r.w;
			frame.size.height = r.h;
			if (!IsCompositing()) {
				// Offset the frame by the parent origin
				HIViewRef parent = HIViewGetSuperview(GetViewRef());
				if (parent) {
					HIRect r;
					HIViewGetFrame(parent, &r);
					frame.origin.x += r.origin.x;
					frame.origin.y += r.origin.y;
				}
			}
			SetFrame(frame);
			break;			
		default:
			break;
	};
}

void TViewNoCompositingCompatible::SetProperty(OSType propID, AUGUI::point_t &p)
{
	switch(propID) {
		default:
			break;
	};
}

void TViewNoCompositingCompatible::SetProperty(OSType propID, AUGUI::color_t &c)
{
	switch(propID) {
		default:
			break;
	};
}

 bool TViewNoCompositingCompatible::GetProperty(OSType propID, double &value)
{
	switch(propID) {
		default:
			break;
	};
	return false;
}

bool TViewNoCompositingCompatible::GetProperty(OSType propID, int32_t &value)
{
	bool res = true;
	switch(propID) {
		case 'enab':
			value = IsEnabled();
			break;
		case 'min ':
			value = GetMinimum();
			break;
		case 'max ':
			value = GetMaximum();
			break;
		case 'val ':
			value = GetValue();
			break;
		default:
			res = false;
			break;
	};
	return res;
}

 bool TViewNoCompositingCompatible::GetProperty(OSType propID, CFStringRef &value)
{
	switch(propID) {
		case 'titl':
			CopyControlTitleAsCFString(GetViewRef(), &value);
			return true;
			break;
		default:
			break;
	};
	return false;
}

bool TViewNoCompositingCompatible::GetProperty(OSType propID, AUGUI::font_t &f)
{
	switch(propID) {
		default:
			break;
	};
	return false;
}

bool TViewNoCompositingCompatible::GetProperty(OSType propID, AUGUI::rect_t &r)
{
	bool result = true;
	switch(propID) {
		case 'fram':
			HIRect frame = Frame();
			if (!IsCompositing()) {
				// Offset the frame by the parent origin
				HIViewRef parent = HIViewGetSuperview(GetViewRef());
				if (parent) {
					HIRect r;
					HIViewGetFrame(parent, &r);
					frame.origin.x -= r.origin.x;
					frame.origin.y -= r.origin.y;
				}
			}
			r.x = frame.origin.x;
			r.y = frame.origin.y;
			r.w = frame.size.width ;
			r.h = frame.size.height;
			break;
		default:
			result = false;
			break;
	};
	return result;
}

bool TViewNoCompositingCompatible::GetProperty(OSType propID, AUGUI::point_t &p)
{
	switch(propID) {
		default:
			break;
	};
	return false;
}

bool TViewNoCompositingCompatible::GetProperty(OSType propID, AUGUI::color_t &c)
{
	switch(propID) {
		default:
			break;
	};
	return false;
}


