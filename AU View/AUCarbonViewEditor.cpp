/*
 *  AUCarbonViewEditor.cpp
 *  Muso
 *
 *  Created by Airy ANDRE on 20/09/04.
 *  Copyright 2004 __MyCompanyName__. All rights reserved.
 *
 */

#include "AUCarbonViewEditor.h"
#include "AUGUIUtilities.h"
#include "AUCarbonViewNib.h"
#include "TViewNoCompositingCompatible.h"
#include "TTransparentEditText.h"
#include "TTextPopup.h"
#include "ColorSwatch.h"
#include "AUProperty.h"

//#define USE_TTEXTPOPUP

typedef enum {
    kX = 0,
    kY,
    kWidth,
    kHeight,
    kTitle,
    kEnabled,
    kSignature,
    kSignatureID,
    kCommandID,
    kParameter,
	kList
} tInfoFieldID;

// --------------------------------------------------------------------
CAUCarbonViewEditor::CAUCarbonViewEditor(CAUCarbonViewNib *view) :
mHandlers(0), mAUView(view), mCurrentControl(0), mInfos(0)
{
    IBNibRef      inNibRef;
    TViewNoCompositingCompatible::RegisterClassForBundleID<ColorSwatch>(mAUView->GetBundleID());
    if (CreateNibReferenceWithCFBundle(view->GetBundleRef(), CFSTR("editor"), &inNibRef) == noErr) {
        if (CreateWindowFromNib( inNibRef, CFSTR("infos"), &mInfos) == noErr) {
            EventTypeSpec events[] = { {kEventClassCommand, kEventCommandProcess}};
            WantEventTypes(GetWindowEventTarget(mInfos), GetEventTypeCount(events), events);
            EventTypeSpec paneEvents[] = { { kEventClassControl, kEventControlClick } };
            WantEventTypes(GetControlEventTarget(view->GetCarbonPane()), GetEventTypeCount(paneEvents), paneEvents);
            EventTypeSpec events2[] = { {kEventClassWindow, kEventWindowContextualMenuSelect} };
            WantEventTypes(GetWindowEventTarget(view->GetCarbonWindow()), GetEventTypeCount(events2), events2);
        }
        if (CreateMenuFromNib(inNibRef, CFSTR("Editor"), &mMenu) == noErr) {
            if (CreateMenuFromNib(inNibRef, CFSTR("Controls"), &mControlsMenu) == noErr) {
                SetMenuItemHierarchicalMenu(mMenu, 2, mControlsMenu);
				Str255 themeFontName;
				SInt16 themeFontID;
				SInt16 themeFontSize;
				Style themeFontStyle;
				GetThemeFont(kThemeSmallSystemFont, smSystemScript, themeFontName, &themeFontSize, &themeFontStyle);
				GetFNum(themeFontName, &themeFontID);
				SetMenuFont(mMenu, themeFontID, themeFontSize);
				SetMenuFont(mControlsMenu, themeFontID, themeFontSize);
            }
        }
        DisposeNibReference(inNibRef);
	}        
}

CAUCarbonViewEditor::~CAUCarbonViewEditor()
{
    if (mHandlers != NULL) {
        int count = CFDictionaryGetCount(mHandlers);
        EventHandlerRef *theHandlers = (EventHandlerRef*) malloc(count * sizeof(EventHandlerRef));
        CFDictionaryGetKeysAndValues(mHandlers, NULL, (const void **)theHandlers);
        
        for (int i = 0; i < count; i++)
            RemoveEventHandler(theHandlers[i]);
        CFDictionaryRemoveAllValues(mHandlers);
        CFRelease (mHandlers);
        free(theHandlers);
    }	
    
    if (mInfos)
        ReleaseWindow(mInfos);
    if (mMenu)
        ReleaseMenu(mMenu);
    if (mControlsMenu)
        ReleaseMenu(mControlsMenu);
    Draw1Control(mAUView->GetCarbonPane());
	
	// Here, we should save our XML file
	if (mCurrentControl)
		mAUView->updateXMLForControl(mCurrentControl);
	mAUView->SaveXML();
	
	mCurrentControl = 0;
	UpdatePropertiesList();
}

bool CAUCarbonViewEditor::IsValid() const
{
    return mInfos != 0;
}

void CAUCarbonViewEditor::ShowHide(bool show)
{
    if (mInfos)
        ::ShowHide(mInfos, show);
}

static pascal OSStatus TheEventHandler(EventHandlerCallRef inHandlerRef, EventRef inEvent, void *inUserData)
{
    CAUCarbonViewEditor *handler = (CAUCarbonViewEditor *)inUserData;

    if (handler->HandleEvent(inHandlerRef, inEvent)) {
        return noErr;
    } else  {
		return eventNotHandledErr;
	}
}

void	CAUCarbonViewEditor::WantEventTypes(EventTargetRef target, UInt32 inNumTypes, const EventTypeSpec *inList)
{
    if (mHandlers == NULL)
        mHandlers = CFDictionaryCreateMutable(NULL, 0, NULL, NULL);
    
    EventHandlerRef handler;
    
    if (CFDictionaryGetValueIfPresent (mHandlers, target, (const void **)&handler))	// if there is already a handler for the target, add the type
        verify_noerr(AddEventTypesToHandler(handler, inNumTypes, inList));
    else {
        verify_noerr(InstallEventHandler(target, TheEventHandler, inNumTypes, inList, this, &handler));
        CFDictionaryAddValue(mHandlers, target, handler);
    }
}

void	CAUCarbonViewEditor::RemoveEventTypes(EventTargetRef target, UInt32 inNumTypes, const EventTypeSpec *inList)
{
    if (mHandlers == NULL)
        mHandlers = CFDictionaryCreateMutable(NULL, 0, NULL, NULL);
    
    EventHandlerRef handler;
    
    if (CFDictionaryGetValueIfPresent (mHandlers, target, (const void **)&handler))	// if there is already a handler for the target, add the type
        verify_noerr(RemoveEventTypesFromHandler(handler, inNumTypes, inList));
}

//-----------------------------------------------------------------------------------
//	WindowContextual
//-----------------------------------------------------------------------------------
//
OSStatus CAUCarbonViewEditor::WindowContextual(TCarbonEvent&theEvent, EventHandlerCallRef inHandlerRef)
{
    ControlRef theControl = 0;
    HIPoint thePoint;
    Point testPoint;
    theEvent.GetParameter<HIPoint>( kEventParamMouseLocation, typeHIPoint, &thePoint );
    testPoint.h = int(thePoint.x);
    testPoint.v = int(thePoint.y);
    AUGUI::ConvertGlobalToWindowPoint(mAUView->GetCarbonWindow(), testPoint);
    FindControl(testPoint, mAUView->GetCarbonWindow(), &theControl);
    int result;
    if (theControl == mCurrentControl) {
        result = PopUpMenuSelect(mMenu, int(thePoint.y), int(thePoint.x), 1);
    } else {
        result = PopUpMenuSelect(mControlsMenu, int(thePoint.y), int(thePoint.x), 1);
    }
    return noErr;
}    
//-----------------------------------------------------------------------------------
//	CommandProcess
//-----------------------------------------------------------------------------------
//
OSStatus CAUCarbonViewEditor::CommandProcess(TCarbonEvent&theEvent, EventHandlerCallRef inHandlerRef)
{
    int err = noErr;
    HICommandExtended cmd;
    GetEventParameter( theEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof( cmd ), NULL, &cmd );
    HIViewRef source = cmd.source.control;
    TViewNoCompositingCompatible *tview = 0;
    GetControlData(mCurrentControl, 0, TViewNoCompositingCompatible::theViewTag, sizeof(tview), &tview, 0);
	
    switch (cmd.commandID) {
        case 'SAVE': { // Save XML file
			if (mCurrentControl)
				mAUView->updateXMLForControl(mCurrentControl);
			mAUView->SaveXML();
			break;
		}
		case 'PROP': { // Prop Edit mode
					   // "source" value has to be copied to corresponding property
			if (tview) {
				// Get property id
				HIViewID id;
				GetControlID(source, &id);
				AUGUI::property_t prop = tview->GetPropertyDefinition(id.id);
				switch (prop.type()) {
					case AUGUI::kFont: {
						HIViewID fontID = { 'PROP', id.id };
						HIViewID sizeID = { 'SIZE', id.id };
						HIViewRef ctrl, ctrl2;
						HIViewFindByID(HIViewGetRoot(mInfos), fontID, &ctrl);
						HIViewFindByID(HIViewGetRoot(mInfos), sizeID, &ctrl2);
						AUGUI::font_t f;
						Size size;
#ifdef USE_TTEXTPOPUP
						GetControlData(ctrl, kControlNoPart, kControlEditTextCFStringTag, 
									   sizeof(CFStringRef), &f.name, &size);
#else
						int n = GetControlValue(ctrl);
						MenuRef menuRef;
						Size actualSize;
						GetControlData(ctrl,kControlEntireControl,kControlPopupButtonMenuHandleTag,
									   sizeof(menuRef),&menuRef,&actualSize);
						CopyMenuItemTextAsCFString(menuRef,n,&f.name);
#endif
						CFStringRef v;
						GetControlData(ctrl2, kControlNoPart, kControlEditTextCFStringTag, 
									   sizeof(CFStringRef), &v, &size);						
						f.size = CFStringGetIntValue(v);
						CFRelease(v);
						tview->SetProperty(prop.tag(), f);
						CFRelease(f.name);

					}
						break;
					case AUGUI::kString:
					case AUGUI::kPicture: {
						Size size;
						CFStringRef v;
						GetControlData(source, kControlNoPart, kControlEditTextCFStringTag, 
									   sizeof(CFStringRef), &v, &size);
						tview->SetProperty(prop.tag(), v);
						CFRelease(v);

						
					}
						break;
					case AUGUI::kFloat: {
						Size size;
						CFStringRef v;
						GetControlData(source, kControlNoPart, kControlEditTextCFStringTag, 
									   sizeof(CFStringRef), &v, &size);						
						double val = CFStringGetDoubleValue(v);
						CFRelease(v);
						tview->SetProperty(prop.tag(), val);
						
					}
						break;
					case AUGUI::kBool: {
						int val = GetControlValue(source);
						tview->SetProperty(prop.tag(), val);
					}
						break;
					case AUGUI::kInteger: {
						Size size;
						CFStringRef v;
						GetControlData(source, kControlNoPart, kControlEditTextCFStringTag, 
									   sizeof(CFStringRef), &v, &size);						
						int32_t val = CFStringGetIntValue(v);
						CFRelease(v);
						tview->SetProperty(prop.tag(), val);
						
					}
						break;
					case AUGUI::kColor: {
						RGBColor rgb;
						HIViewID alpha = { 'ALPH', id.id };
						HIViewID colorSwatch = { 'RGB ', id.id };
						HIViewRef ctrl, ctrl2;
						HIViewFindByID(HIViewGetRoot(mInfos), colorSwatch, &ctrl);
						HIViewFindByID(HIViewGetRoot(mInfos), alpha, &ctrl2);
						int a = GetControl32BitValue(ctrl2);
						
						HIColorSwatchGetColor(ctrl, &rgb);
						AUGUI::color_t val;
						val.red = rgb.red/65535.;
						val.green = rgb.green/65535.;
						val.blue = rgb.blue/65535.;
						val.alpha = a/10000.;
						tview->SetProperty(prop.tag(), val);
					}
						break;
					default: 
						break;
				}
				tview->FlushStaticBackground();
                Draw1Control(mAUView->GetCarbonPane());
				mAUView->updateXMLForControl(mCurrentControl);				
			}
			break;
		}
		case 'INFO': { // Edit mode
            if (GetControlOwner(source) == mInfos) {
                HIRect frame;
                HIViewGetFrame(mCurrentControl, &frame);
                
                Size size;
                CFStringRef cfstr; 
                
                {
                    HIViewID id = { 'info', kX };
                    HIViewRef ctrl;
                    HIViewFindByID(HIViewGetRoot(mInfos), id, &ctrl);
                    
                    GetControlData(ctrl, kControlNoPart, kControlEditTextCFStringTag, 
                                   sizeof(CFStringRef), &cfstr, &size);
                    frame.origin.x = CFStringGetDoubleValue(cfstr);
                }
                {
                    HIViewID id = { 'info', kY };
                    HIViewRef ctrl;
                    HIViewFindByID(HIViewGetRoot(mInfos), id, &ctrl);
                    
                    GetControlData(ctrl, kControlNoPart, kControlEditTextCFStringTag, 
                                   sizeof(CFStringRef), &cfstr, &size);
                    frame.origin.y = CFStringGetDoubleValue(cfstr);
                }
                {
                    HIViewID id = { 'info', kWidth };
                    HIViewRef ctrl;
                    HIViewFindByID(HIViewGetRoot(mInfos), id, &ctrl);
                    
                    GetControlData(ctrl, kControlNoPart, kControlEditTextCFStringTag, 
                                   sizeof(CFStringRef), &cfstr, &size);
                    frame.size.width = CFStringGetDoubleValue(cfstr);
                }
                {
                    HIViewID id = { 'info', kHeight };
                    HIViewRef ctrl;
                    HIViewFindByID(HIViewGetRoot(mInfos), id, &ctrl);
                    
                    GetControlData(ctrl, kControlNoPart, kControlEditTextCFStringTag, 
                                   sizeof(CFStringRef), &cfstr, &size);
                    frame.size.height = CFStringGetDoubleValue(cfstr);
                }
                if (tview->GetPropertiesListSize() <= tview->TViewNoCompositingCompatible::GetPropertiesListSize()) {
                    HIViewID id = { 'info', kTitle };
                    HIViewRef ctrl;
                    HIViewFindByID(HIViewGetRoot(mInfos), id, &ctrl);
                    
                    GetControlData(ctrl, kControlNoPart, kControlEditTextCFStringTag, 
                                   sizeof(CFStringRef), &cfstr, &size);
                    SetControlTitleWithCFString(mCurrentControl, cfstr);
                }
                HIViewSetFrame(mCurrentControl, &frame);
                if (tview)
                    tview->FlushStaticBackground();
                Draw1Control(mAUView->GetCarbonPane());
				if (mCurrentControl)
					mAUView->updateXMLForControl(mCurrentControl);
				UpdatePropertiesList();
                
            }
			default:
				err = eventNotHandledErr;
        }
    }
    return err;
}

//-----------------------------------------------------------------------------------
//	DrawControl
//-----------------------------------------------------------------------------------
//
OSStatus CAUCarbonViewEditor::DrawControl(TCarbonEvent&theEvent, EventHandlerCallRef inHandlerRef)
{
    CGContextRef		inContext = NULL;
    HIViewRef   control = NULL;
    theEvent.GetParameter<CGContextRef>( kEventParamCGContextRef, typeCGContextRef, &inContext );
    theEvent.GetParameter<HIViewRef>( kEventParamDirectObject, typeControlRef, &control );
    
    // Call standard draw method
    CallNextEventHandler(inHandlerRef, theEvent);
    HIRect frame;
    {
        CGContextRef			context = inContext;
        GrafPtr graphPtr = 0;
        bool compositing = (inContext != 0);
        
        HIViewGetBounds(control, &frame);
        TRect bounds = frame;
        
        if (!compositing) {
            SetPortWindowPort(GetControlOwner(control));
            Rect r = bounds;
            //EraseRect(&r);
        }
        if (!compositing) { // When called in a not compositing window
            graphPtr=GetWindowPort(GetControlOwner(control));
            
            Rect rect;
            GetPortBounds(graphPtr, &rect);
            
            QDBeginCGContext(graphPtr, &context);
            CGContextTranslateCTM(context, 0.f, float(rect.bottom-rect.top));
            CGContextScaleCTM(context, 1.f, -1.f);
            // CGContextClipToRect(context, bounds);
        } 
        // Call the real draw routine
        HIViewGetFrame(control, &frame);
        frame.origin.x += .5;
        frame.origin.y += .5;
        frame.size.width -= 1.;
        frame.size.height -= 1.;
        CGContextBeginPath(context);    
        CGContextSetRGBFillColor(context, 1, 0, 0, .2);    
        CGContextFillRect(context, frame);
        
        CGContextSetLineWidth(context, 1);   
        CGContextSetRGBStrokeColor(context, 0, 0, 0, .8);    
        CGContextStrokeRect(context, frame);
        CGContextSynchronize ( context );
        
        if (!compositing) {
            CGContextSynchronize ( context );
            QDEndCGContext (graphPtr, &context );
        }
    }
    return noErr;
}
//-----------------------------------------------------------------------------------
//	ClickControl
//-----------------------------------------------------------------------------------
//!	Called at the beginning of tracking
//
OSStatus CAUCarbonViewEditor::ClickControl(TCarbonEvent&theEvent, EventHandlerCallRef inHandlerRef)
{
    OSStatus res = -1;
    
    ControlRef clickedControl = 0;
    theEvent.GetParameter(kEventParamDirectObject, &clickedControl);
    if (clickedControl != mCurrentControl) {
        if (mCurrentControl) {
            EventTypeSpec paneEvents[] = { { kEventClassControl, kEventControlDraw },
            { kEventClassControl, kEventControlTrack }};
            RemoveEventTypes(GetControlEventTarget(mCurrentControl), GetEventTypeCount(paneEvents), paneEvents);
			if (mCurrentControl)
				mAUView->updateXMLForControl(mCurrentControl);
            mCurrentControl = 0;
            Draw1Control(mAUView->GetCarbonPane());
        }
		UpdatePropertiesList();
	}                
    if (clickedControl && clickedControl != mAUView->GetCarbonPane() && 
        clickedControl != mAUView->GetRootPane())
    {
        if (mCurrentControl != clickedControl) {
            EventTypeSpec paneEvents[] = { { kEventClassControl, kEventControlDraw },
            { kEventClassControl, kEventControlTrack }};
            WantEventTypes(GetControlEventTarget(clickedControl), GetEventTypeCount(paneEvents), paneEvents);
            if (mCurrentControl)
				mAUView->updateXMLForControl(mCurrentControl);
			mCurrentControl = clickedControl;
			UpdatePropertiesList();
        }
        
        // Move control to top
        HIViewSetZOrder(clickedControl, kHIViewZOrderAbove, NULL);
        
        HIRect frame;
        HIViewGetFrame(clickedControl, &frame);
        for (int i = kX; i <= kHeight; ++i) {
            float val = frame.origin.x;
            HIViewID id = { 'info', i };
            HIViewRef ctrl;
            OSStatus result = HIViewFindByID(HIViewGetRoot(mInfos), id, &ctrl);
            if (result == noErr) {
                switch (i) {
                    case kX:
                        val = frame.origin.x;
                        break;
                    case kY:
                        val = frame.origin.y;
                        break;
                    case kWidth:
                        val = frame.size.width;
                        break;
                    case kHeight:
                        val = frame.size.height;
                        break;
                }
				CFStringRef cfstr = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%.0f"), val);
                SetControlData(ctrl, kControlNoPart, kControlEditTextCFStringTag, sizeof(CFStringRef), &cfstr);
                CFRelease (cfstr);
            }
        }
        {
            HIViewID id = { 'info', kTitle };
            HIViewRef ctrl;
            OSStatus result = HIViewFindByID(HIViewGetRoot(mInfos), id, &ctrl);
            if (result == noErr) {
                CFStringRef title;
                CopyControlTitleAsCFString(clickedControl, &title);
                SetControlData(ctrl, kControlNoPart, kControlEditTextCFStringTag, sizeof(CFStringRef), &title);
                CFRelease(title);
            }
        }                        
        HIViewID signature;
        GetControlID(clickedControl, &signature);
        {
            HIViewID id = { 'info', kSignature };
            HIViewRef ctrl;
            OSStatus result = HIViewFindByID(HIViewGetRoot(mInfos), id, &ctrl);
            if (result == noErr) {
				CFStringRef cfstr =  UTCreateStringForOSType(signature.signature);
                SetControlData(ctrl, kControlNoPart, kControlEditTextCFStringTag, sizeof(CFStringRef), &cfstr);
                CFRelease (cfstr);
            }
        }
        {
            HIViewID id = { 'info', kSignatureID };
            HIViewRef ctrl;
            OSStatus result = HIViewFindByID(HIViewGetRoot(mInfos), id, &ctrl);
            if (result == noErr) {
				CFStringRef cfstr = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%d"), signature.id);
                SetControlData(ctrl, kControlNoPart, kControlEditTextCFStringTag, sizeof(CFStringRef), &cfstr);
                CFRelease (cfstr);
                
                HIViewID id = { 'info', kParameter };
                OSStatus result = HIViewFindByID(HIViewGetRoot(mInfos), id, &ctrl);
                if (result == noErr) {
                    if (signature.signature == 'AUid'  && signature.id > 0 && signature.id != 9999) {
                        int paramID = signature.id % 1000;
                        CAAUParameter tParam(mAUView->GetEditAudioUnit(), paramID, kAudioUnitScope_Global, 0);
                        CFStringRef cfstr = tParam.GetName();
                        SetControlData(ctrl, kControlNoPart, kControlEditTextCFStringTag, sizeof(CFStringRef), &cfstr);
                    } else {
                        CFStringRef cfstr = CFSTR("(none)");
                        SetControlData(ctrl, kControlNoPart, kControlEditTextCFStringTag, sizeof(CFStringRef), &cfstr);
                    }
                }
            }
        }
        {
            HIViewID id = { 'info', kCommandID };
            HIViewRef ctrl;
            OSStatus result = HIViewFindByID(HIViewGetRoot(mInfos), id, &ctrl);
            if (result == noErr) {
                UInt32 commandID = 0;
                GetControlCommandID(clickedControl, &commandID);
                
				CFStringRef cfstr =  UTCreateStringForOSType(commandID);
                SetControlData(ctrl, kControlNoPart, kControlEditTextCFStringTag, sizeof(CFStringRef), &cfstr);
                CFRelease (cfstr);
            }
        }
        
        Draw1Control(mAUView->GetCarbonPane());
        SetUpTracking(theEvent);
        res = noErr;
    }
    return res;
}

//-----------------------------------------------------------------------------------
//	StillTracking
//-----------------------------------------------------------------------------------
//!	Called whenever the mouse is moving during tracking
//
OSStatus CAUCarbonViewEditor::StillTracking(TCarbonEvent&inEvent)
{
    ControlRef clickedControl = 0;
    UInt32 modifiers;
    HIPoint to;
    inEvent.GetParameter<HIPoint>( kEventParamMouseLocation, typeHIPoint, &to );
    clickedControl = mCurrentControl;
    
    inEvent.GetParameter( kEventParamKeyModifiers, &modifiers );
    {
        
        float deltaX, deltaY;
        
        TViewNoCompositingCompatible *tview = 0;
        GetControlData(clickedControl, 0, TViewNoCompositingCompatible::theViewTag, sizeof(tview), &tview, 0);
        {
            deltaX = (to.x - mFromPoint.x);
            deltaY = (to.y - mFromPoint.y);
            mFromPoint = to;
            
            HIRect frame;
            HIViewGetFrame(clickedControl, &frame);
            
            if (modifiers & cmdKey) {
                frame.size.width += deltaX;
                frame.size.height += deltaY;
                if (frame.size.width < 0)
                    frame.size.width = 1;
                if (frame.size.height < 0)
                    frame.size.height = 1;
            } else {
                frame.origin.x += deltaX;
                frame.origin.y += deltaY;
            }
            HideControl(clickedControl);
            HIViewSetFrame(clickedControl, &frame);
            ShowControl(clickedControl);
            Draw1Control(mCurrentControl);
            
            // Update info window
            for (int i = kX; i <= kHeight; ++i) {
                float val = frame.origin.x;
                HIViewID id = { 'info', i };
                HIViewRef ctrl;
                OSStatus result = HIViewFindByID(HIViewGetRoot(mInfos), id, &ctrl);
                if (result == noErr) {
                    switch (i) {
                        case kX:
                            val = frame.origin.x;
                            break;
                        case kY:
                            val = frame.origin.y;
                            break;
                        case kWidth:
                            val = frame.size.width;
                            break;
                        case kHeight:
                            val = frame.size.height;
                            break;
                    }
					CFStringRef cfstr = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%.0f"), val);
                    SetControlData(ctrl, kControlNoPart, kControlEditTextCFStringTag, sizeof(CFStringRef), &cfstr);
                    CFRelease (cfstr);
                }
				if (tview)
                    tview->FlushStaticBackground();
            }
        }
    }
    return noErr;
}


//-----------------------------------------------------------------------------------
//	StopTracking
//-----------------------------------------------------------------------------------
//!	Called at the end of tracking. Our default implementation calls the HitTest()
//! method to determine which part of the control the mouse was over when the
//! button was released. This part code is set in \a inEvent.
//
OSStatus CAUCarbonViewEditor::StopTracking(TCarbonEvent&inEvent)
{
    // Remove mouse handler...
    EventTypeSpec kHIMouseEvents[] = {
    { kEventClassMouse, kEventMouseUp },
    { kEventClassMouse, kEventMouseDragged }
    };
    RemoveEventTypes(GetWindowEventTarget(GetControlOwner(mAUView->GetCarbonPane())), 
                     GetEventTypeCount(kHIMouseEvents), 
                     kHIMouseEvents);
    Draw1Control(mAUView->GetCarbonPane());
    return noErr;
}


//-----------------------------------------------------------------------------------
//	SetUpTracking
//-----------------------------------------------------------------------------------
//!	Install handler for MouseUp and MouseDragged (for mouse tracking)
//
OSStatus CAUCarbonViewEditor::SetUpTracking(TCarbonEvent& inEvent)
{
    EventTypeSpec kHIMouseEvents[] = {
    { kEventClassMouse, kEventMouseUp },
    { kEventClassMouse, kEventMouseDragged }
    };
    WantEventTypes(GetWindowEventTarget(GetControlOwner(mAUView->GetCarbonPane())), 
                   GetEventTypeCount(kHIMouseEvents), 
                   kHIMouseEvents);
    
    inEvent.GetParameter<HIPoint>( kEventParamMouseLocation, typeHIPoint, &mFromPoint );
    
    return noErr;
}

//-----------------------------------------------------------------------------------
//	IsCompositing
//-----------------------------------------------------------------------------------
//!	Return true if the view is in a compositing window.
//
bool CAUCarbonViewEditor::IsCompositing()
{
    WindowAttributes attributes;
    GetWindowAttributes(GetControlOwner(mAUView->GetCarbonPane()), &attributes);
    return attributes & kWindowCompositingAttribute;
}

//! In Edit Mode, we can move and resize controls
bool CAUCarbonViewEditor::HandleEvent(EventHandlerCallRef inHandlerRef, EventRef event)
{
    OSStatus res = eventNotHandledErr;
    
    TCarbonEvent theEvent = event;
    
    if ( (theEvent.GetClass() == kEventClassWindow) && (theEvent.GetKind() == kEventWindowContextualMenuSelect) ) 
    {
        res = WindowContextual(theEvent, inHandlerRef);
    }
    
    if ( (theEvent.GetClass() == kEventClassMouse) && (theEvent.GetKind() == kEventMouseUp) ) 
    {
        res = StopTracking(theEvent);
    }
    
    if ( (theEvent.GetClass() == kEventClassMouse) && (theEvent.GetKind() == kEventMouseDragged) ) 
    {
        res = StillTracking(theEvent);
    }
    
    if ( (theEvent.GetClass() == kEventClassControl) && (theEvent.GetKind() == kEventControlDraw) ) 
    {
        res = DrawControl(theEvent, inHandlerRef);
    }
    
    // catch any clicks with ctrl+alt+cmd modifiers to move the control
    if ( (theEvent.GetClass() == kEventClassControl) && (theEvent.GetKind() == kEventControlTrack) )
    {        
        res =  true;    
    } 
    
    if ( (theEvent.GetClass() == kEventClassControl) && (theEvent.GetKind() == kEventControlClick) )
    {        
        res = ClickControl(theEvent, inHandlerRef);
    } 
    
    if ((res == eventNotHandledErr) && (theEvent.GetClass() == kEventClassCommand) &&
        (theEvent.GetKind() == kEventCommandProcess) && mCurrentControl)
    {
        res = CommandProcess(theEvent, inHandlerRef);
    }
    return res == noErr;
}

void CAUCarbonViewEditor::UpdatePropertiesList()
{
	HIViewID id = { 'info', kList };
	HIViewRef list;
	OSStatus result = HIViewFindByID(HIViewGetRoot(mInfos), id, &list);
	if (result != noErr) 
		return;
	
	// First, let's empty the current list
	HIViewRef view = HIViewGetFirstSubview(list);
	while (view) {
		HIViewRef oldView = view;
		view = HIViewGetNextView(oldView);
		HIViewRemoveFromSuperview(oldView);
		DisposeControl(oldView);
	}
	HIViewID idTitle = { 'info', kTitle };
	HIViewRef ctrl;
	HIViewFindByID(HIViewGetRoot(mInfos), idTitle, &ctrl);
	if (ctrl)
		HIViewSetVisible(ctrl, 1);
	
	if (mCurrentControl) {
		// Populate it with all the properties of the control
		TViewNoCompositingCompatible *tview = 0;
		GetControlData(mCurrentControl, 0, TViewNoCompositingCompatible::theViewTag, sizeof(tview), &tview, 0);
		if (tview) {
			HIRect r;
			HIRect r2;
			r.origin.x = 5;
			r.origin.y = 5;
			r.size.width = 60;
			r.size.height = 15;
			size_t s = tview->GetPropertiesListSize();
			
			if (ctrl) {
				HIViewSetVisible(ctrl, s <= tview->TViewNoCompositingCompatible::GetPropertiesListSize());
			}
			int start = 2; // Don't edit here title and frame
			for (size_t i = start; i < s; ++i) { // We ignore frame and title
				HIViewRef label;
				HIViewRef ctrl = 0;
				AUGUI::property_t prop = tview->GetPropertyDefinition(i);
				Rect pos;
				AUGUI::HIRectToQDRect(&r, &pos);
				ControlFontStyleRec style;
				style.flags = kControlUseFontMask | kControlUseJustMask | kControlUseSizeMask;
				style.font = kFontIDGeneva;
				style.size = 9;
				style.just = teJustRight; 
				CreateStaticTextControl(0, &pos, prop.label(), &style, &label);
				HIViewAddSubview (list, label); 
				HIViewSetVisible(label, 1);
				r2 = r; r2.origin.x = r.origin.x + r.size.width + 10;
				r2.size.height = 16;
				AUGUI::HIRectToQDRect(&r2, &pos);
				switch (prop.type()) {
					case AUGUI::kFont: {
						AUGUI::font_t f = {CFSTR(""), 0};
						tview->GetProperty(prop.tag(), f);

						r2.size.width += 100;
#ifdef USE_TTEXTPOPUP
						TTextPopup *popup = static_cast<TTextPopup *>(AUGUI::CreateForCurrentBundle<TTextPopup>(&r2, 0));
						ctrl = popup->GetViewRef();
						AUGUI::font_t font = { CFSTR("Geneva"), 9 };
						popup->SetProperty('yOff', 5.); // 5 pixels offeset
						popup->SetProperty('fram', 1);  // Draw frame
						popup->SetProperty('font', font);  // font
						popup->SetProperty('smfo', 0); // Don't smooth
#else
						pos.right += 100;
						::CreatePopupButtonControl( 0, &pos, CFSTR(""), -12345, true, -1, 0, 0, &ctrl );
						ControlSize controlSize = kControlSizeSmall;
						SetControlData( ctrl, kControlEntireControl, kControlSizeTag, sizeof( ControlSize ), &controlSize );
						SInt16 baseLine;
						GetBestControlRect( ctrl, &pos, &baseLine );
						SetControlBounds( ctrl, &pos );			
						ControlFontStyleRec fontStyle = { 0 };
						
						fontStyle.flags = kControlUseFontMask;
						fontStyle.font = kControlFontSmallSystemFont;
						SetControlData( ctrl, kControlEntireControl, kControlFontStyleTag, sizeof( fontStyle ), &fontStyle );						
#endif
						MenuRef menuRef;
						CreateNewMenu('AUid', 0, &menuRef);
						// set to theme font
						Str255 themeFontName;
						SInt16 themeFontID;
						SInt16 themeFontSize;
						Style themeFontStyle;
						GetThemeFont(kThemeSmallSystemFont, smSystemScript, themeFontName, &themeFontSize, &themeFontStyle);
						GetFNum(themeFontName, &themeFontID);
						SetMenuFont(menuRef, themeFontID, themeFontSize);
						SetControlData(ctrl, 0, kControlPopupButtonMenuRefTag, sizeof(menuRef), &menuRef);
						
						CreateStandardFontMenu (menuRef, 0, 0, 0, 0);
						SetControlMinimum(ctrl, 1);
						SetControlMaximum(ctrl, CountMenuItems(menuRef));
						
						// We  set the control value to current font
						int numItems = CountMenuItems( menuRef ); 
						for (int item = 1; item <= numItems; item++ )
						{         
							CFStringRef fontName;
							CopyMenuItemTextAsCFString( menuRef, item, &fontName );
							if (CFStringCompare(f.name, fontName, 0) == kCFCompareEqualTo)
							{
								SetControlValue(ctrl, item);
								numItems = 0;
							}
							CFRelease(fontName);
						}                       
												
						HIViewAddSubview (list, ctrl); 
						HIViewSetVisible(ctrl, 1);
						HIViewID controlID = { 'PROP', i };
						SetControlID(ctrl, &controlID);
						SetControlCommandID(ctrl, 'PROP');

						// Create size control
						style.just = teJustRight;
						r2.origin.x += r2.size.width + 5;
						r2.size.width = 40;
						TTransparentEditText *text = 
							(TTransparentEditText *)AUGUI::CreateForCurrentBundle<TTransparentEditText>(&r2, 0);
						ctrl = text->GetViewRef();
						int drawFrame = 1;
						SetControlData(ctrl, 0, kTransparentEditTextFontDrawFrameTag, sizeof(drawFrame), &drawFrame);
						SetControlData(ctrl, 0, kTransparentEditTextFontStyleTag, sizeof(style), &style);
						CFStringRef v = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%d"), 
																 f.size);
						SetControlData(ctrl, 0, kTransparentEditTextCFStringTag, sizeof(v), &v);
						CFRelease(v);
						HIViewAddSubview (list, ctrl); 
						HIViewSetVisible(ctrl, 1);
						controlID.signature = 'SIZE';
						controlID.id = i;
						SetControlID(ctrl, &controlID);
						SetControlCommandID(ctrl, 'PROP');						
					}
						break;
					case AUGUI::kString:
					case AUGUI::kPicture: {
						CFStringRef val = CFSTR("");
						tview->GetProperty(prop.tag(), val);
						style.just = teJustLeft;
						r2.size.width += 145;
						TTransparentEditText *text = 
							(TTransparentEditText *)AUGUI::CreateForCurrentBundle<TTransparentEditText>(&r2, 0);
						ctrl = text->GetViewRef();
						SetControlData(ctrl, 0, kTransparentEditTextCFStringTag, sizeof(val), &val);
						int drawFrame = 1;
						SetControlData(ctrl, 0, kTransparentEditTextFontDrawFrameTag, sizeof(drawFrame), &drawFrame);
						SetControlData(ctrl, 0, kTransparentEditTextFontStyleTag, sizeof(style), &style);
						HIViewAddSubview (list, ctrl); 
						HIViewSetVisible(ctrl, 1);
						HIViewID controlID = { 'PROP', i };
						SetControlID(ctrl, &controlID);
						SetControlCommandID(ctrl, 'PROP');
						
						CFRelease(val);
					}
						break;
					case AUGUI::kFloat: {
						double val;
						tview->GetProperty(prop.tag(), val);
						style.just = teJustRight;
						CFStringRef v = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%lg"), 
																 val);
						r2.size.width = 70;
						TTransparentEditText *text = 
							(TTransparentEditText *)AUGUI::CreateForCurrentBundle<TTransparentEditText>(&r2, 0);
						ctrl = text->GetViewRef();
						SetControlData(ctrl, 0, kTransparentEditTextCFStringTag, sizeof(v), &v);
						int drawFrame = 1;
						SetControlData(ctrl, 0, kTransparentEditTextFontDrawFrameTag, sizeof(drawFrame), &drawFrame);
						SetControlData(ctrl, 0, kTransparentEditTextFontStyleTag, sizeof(style), &style);
						HIViewAddSubview (list, ctrl); 
						HIViewSetVisible(ctrl, 1);
						HIViewID controlID = { 'PROP', i };;
						SetControlID(ctrl, &controlID);
						SetControlCommandID(ctrl, 'PROP');
						
						CFRelease(v);
					}
						break;
					case AUGUI::kBool: {
						int32_t val;
						tview->GetProperty(prop.tag(), val);
						CreateCheckBoxControl(0, &pos, CFSTR(""), val != 0, 1, &ctrl);
						ControlSize controlSize = kControlSizeSmall;
						SetControlData( ctrl, 0, kControlSizeTag, sizeof( ControlSize ), &controlSize );
						SInt16 baseLine;
						GetBestControlRect( ctrl, &pos, &baseLine );
						SetControlBounds( ctrl, &pos );
						HIViewAddSubview (list, ctrl); 
						HIViewSetVisible(ctrl, 1);
						HIViewID controlID = { 'PROP', i };;
						SetControlID(ctrl, &controlID);
						SetControlCommandID(ctrl, 'PROP');
					}
						break;
					case AUGUI::kInteger: {
						int32_t val;
						tview->GetProperty(prop.tag(), val);
						style.just = teJustRight;
						CFStringRef v = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%d"), 
																 val);
						r2.size.width = 70;
						TTransparentEditText *text = 
							(TTransparentEditText *)AUGUI::CreateForCurrentBundle<TTransparentEditText>(&r2, 0);
						ctrl = text->GetViewRef();
						SetControlData(ctrl, 0, kTransparentEditTextCFStringTag, sizeof(v), &v);
						int drawFrame = 1;
						SetControlData(ctrl, 0, kTransparentEditTextFontDrawFrameTag, sizeof(drawFrame), &drawFrame);
						SetControlData(ctrl, 0, kTransparentEditTextFontStyleTag, sizeof(style), &style);
						HIViewAddSubview (list, ctrl); 
						HIViewSetVisible(ctrl, 1);
						HIViewID controlID = { 'PROP', i };;
						SetControlID(ctrl, &controlID);
						SetControlCommandID(ctrl, 'PROP');
						
						CFRelease(v);
					}
						break;
					case AUGUI::kColor: {
						AUGUI::color_t val;
						tview->GetProperty(prop.tag(), val);
						r2.size.width = 50;
						r2.size.height += 3;
						ColorSwatch *swatch = 
							(ColorSwatch *)AUGUI::CreateForCurrentBundle<ColorSwatch>(&r2, 0);
						ctrl = swatch->GetViewRef();
						RGBColor rgb;
						rgb.red = (unsigned short)(val.red*65535.f);
						rgb.green = (unsigned short)(val.green*65535.f);
						rgb.blue = (unsigned short)(val.blue*65535.f);
						HIColorSwatchSetColor(ctrl, rgb);
						HIViewSetVisible(ctrl, 1);
						
						HIViewID controlID = { 'RGB ', i };
						SetControlID(ctrl, &controlID);
						SetControlCommandID(ctrl, 'PROP');
						
						HIViewAddSubview (list, ctrl);

						// Create the "alpha" slider
						r2.origin.x += r2.size.width + 10;
						r2.size.width = 100;
						r2.size.height = 1;
						AUGUI::HIRectToQDRect(&r2, &pos);
						CreateSliderControl(0, &pos, int(10000*val.alpha), 0, 10000, kControlSliderDoesNotPoint, 0, 0, 0, &ctrl);
						ControlSize controlSize = kControlSizeSmall;
						SetControlData( ctrl, 0, kControlSizeTag, sizeof( ControlSize ), &controlSize );
						SInt16 baseLine;
						GetBestControlRect( ctrl, &pos, &baseLine );
						SetControlBounds( ctrl, &pos );
						HIViewAddSubview (list, ctrl); 
						HIViewSetVisible(ctrl, 1);
						
						controlID.signature = 'ALPH';
						SetControlID(ctrl, &controlID);
						SetControlCommandID(ctrl, 'PROP');
						
					}
						break;
					default: 
						break;
				}
				r.origin.y += r.size.height + 4;
			}
		}
	}
}

