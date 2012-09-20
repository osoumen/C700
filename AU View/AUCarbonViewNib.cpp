/*
 *  AUCarbonViewNib.cpp
 *  AUNib
 *
 *  Created by Airy André on Sun Oct 06 2002.
 *  Copyright (c) 2002-2003 Airy André. All rights reserved.
 *
 */

#include "AUCarbonViewNib.h"
#include "AUGUIUtilities.h"
#include "AUCarbonViewEnhancedControl.h"
#include "TPngPictControl.h"
//#include "TValuePictControl.h"
#include "TValuePictButtonControl.h"
#include "TKnobControl.h"
//#include "TKnobArcControl.h"
#include "TSliderControl.h"
//#include "TMeterControl.h"
//#include "TMeterPartialControl.h"
#include "TValueTextControl.h"
//#include "TImageButton.h"
//#include "TImagePopup.h"
#include "TTransparentEditText.h"
#include "TMultiPane.h"
#include "TPushButton.h"

#include "TImageCache.h"

#include "AUCarbonViewEditor.h"

#include "CACFDictionary.h"

#include "AUOid.h"

#define kMaxNumberOfControlsForEachParameter 10
#define kMaxNumberOfMultipanesForEachPane 16

static void dumpHierarchy(HIViewRef from)
{	
	static int level = -1;
	level++;
	for (int i = 0; i < level; ++i)
		printf("  ");
	CFShow(from);
	
	// Add embedded controls
	HIViewRef view = HIViewGetFirstSubview(from);
	while (view) {
		dumpHierarchy(view);
		view = HIViewGetNextView(view);
	}
	level--;
}

// Workaround for Intel title bug
#if defined(__i386__)
static void fixTitles(HIViewRef from)
{	
	// Add embedded controls
	HIViewRef view = HIViewGetFirstSubview(from);
	while (view) {
		CFStringRef title;
		CopyControlTitleAsCFString(view, &title);
		//- If the title is not NULL, then grab its first character with CFStringGetCharacterAtIndex
		//- If the character is like 0x7300 instead of 0x0073, meaning that the hi-byte is non-NULL and the lo-byte is NULL (you can also look at the other characters by the way), 
		//	then grab all characters with CFStringGetCharacters, byte-swap them, create a new CFString with those, set the title control with it, and clean-up.
		if (title && (CFStringGetCharacterAtIndex(title, 0) & 0xFF00) && !(CFStringGetCharacterAtIndex(title, 0) & 0xFF)) {
			int len = CFStringGetLength(title)	;
			UniChar buffer[len];
			CFRange range = { 0, len };
			CFStringGetCharacters(title, range, buffer);
			for (int i = 0; i < len; ++i) {
				UniChar c = buffer[i];
				buffer[i] = ((c<<8)&0xFF00) | ((c>>8)&0xFF);
				CFStringRef newTitle = CFStringCreateWithCharacters(NULL, buffer, len);
				SetControlTitleWithCFString(view, newTitle);
				CFRelease(newTitle);
			}
		}
		CFRelease(title);
		
		fixTitles(view);
		view = HIViewGetNextView(view);
	}
}
#endif

// ---------------------------------
// save a property list into an XML file:
static SInt32 SavePropertiesToXMLFile(
									  const CFPropertyListRef pCFPRef, 
									  const CFURLRef pCFURLRef)
{
    CFDataRef xmlCFDataRef;
    SInt32 errorCode = coreFoundationUnknownErr;
	
    // Convert the property list into XML data.
    xmlCFDataRef = CFPropertyListCreateXMLData(
											   kCFAllocatorDefault, pCFPRef );
    if (NULL != xmlCFDataRef)
    {
        // Write the XML data to the file.
        (void) CFURLWriteDataAndPropertiesToResource( 
													  pCFURLRef, xmlCFDataRef, NULL, &errorCode);
		
        // Release the XML data
        CFRelease(xmlCFDataRef);
    }	
    return errorCode;    
}

// ---------------------------------
// load a property list from an XML file
static CFPropertyListRef CreatePropertiesFromXMLFile(const CFURLRef pCFURLRef)
{
    CFDataRef xmlCFDataRef;
    CFPropertyListRef myCFPropertyListRef = NULL;
    Boolean status;
	
    // Read the XML file.
    status = CFURLCreateDataAndPropertiesFromResource( 
													   kCFAllocatorDefault, pCFURLRef, 
													   &xmlCFDataRef, NULL, NULL, NULL);
    if (status)
    {
        // Reconstitute the dictionary using the XML data.
        myCFPropertyListRef = CFPropertyListCreateFromXMLData(
															  kCFAllocatorDefault, xmlCFDataRef, 
															  kCFPropertyListImmutable, NULL);
        // Release the XML data
        CFRelease(xmlCFDataRef);
    }
	
    return myCFPropertyListRef;
}

//	The timer proc
//static "C"  void TimerProc ( EventLoopTimerRef inTimer, void *inUserData );

static void TimerProc ( EventLoopTimerRef inTimer, void *inUserData )
{
    if ( inUserData != NULL ) {
        CAUCarbonViewNib *view = (CAUCarbonViewNib *)inUserData;
        view->Idle();
    }
}

// -----------------------------------------------------------------------------
// Class CAUCarbonViewNib
//
// !This class generates a custom view of the plugin from a nib window.
// -----------------------------------------------------------------------------

static pascal void BackgroundEraseRectProc( GrafVerb verb, const Rect* r )
{
    if( verb != kQDGrafVerbErase ) {
        // not interested
        StdRect( verb, r );
    } else {
        CAUCarbonViewNib *view = 0;
        
        StdRect( verb, r );
        CGrafPtr currPort;
        GetPort( &currPort);
        WindowRef theWin = GetWindowFromPort(currPort);
        if (theWin) {
            GetWindowProperty(theWin, 'AGUI', 'CVNB', sizeof(CAUCarbonViewNib *), 0, &view);
        }
        if (view && view->GetBackgroundImage()) {
            CGContextRef context;
            Rect rect;
            
            QDBeginCGContext(currPort, &context);
            GetPortBounds(currPort, &rect);
            CGContextTranslateCTM(context, 0, float(rect.bottom-rect.top));
            CGContextScaleCTM(context, 1, -1);
            
            TRect trect = *r;
            CGContextClipToRect(context, trect);
            
            Rect paneRect;
            view->GetPaneBounds(&paneRect);
            trect = paneRect;
            HIViewDrawCGImage(context, &trect, view->GetBackgroundImage());
            
            QDEndCGContext (currPort, &context );
        }
    }
}
// -----------------------------------------------------------------------------
CAUCarbonViewNib::CAUCarbonViewNib(AudioUnitCarbonView tView):
AUCarbonViewBase(tView), mCurrentProcsPtr(0), mBackgroundImage(0), mBundleRef(0), mRootUserPane(0), mPaneID(0),
mEditMode(false), mEditor(0), mXml(true)
{
}
// -----------------------------------------------------------------------------
CAUCarbonViewNib::~CAUCarbonViewNib()
{
    UnRegisterAllPropertiesChanges();
    
    if (HasTimer())
        RemoveEventLoopTimer(mTimer);
    
    if (mBackgroundImage) {
        SetPortGrafProcs( GetWindowPort( mCarbonWindow ), mCurrentProcsPtr);
        DisposeRoutineDescriptor (mProcs.rectProc);
        CGImageRelease(mBackgroundImage);
    }
    // Delete our userPane so we can unregister our TView classes
    ControlID controlID = { 'AUid', kAUNibUserPaneControlID }; // My user pane
    ControlRef control;
    if (HIViewFindByID (mCarbonPane, controlID, &control) == noErr) {
        if (HasBackgroundPict()) {
            // My user pane is embededed in the background pane
            HIViewRef superview = HIViewGetSuperview(control);
			if (superview) {
				TViewNoCompositingCompatible *tview = 0;
				//  our  superview should be a TViewNoCompositingCompatible
				OSErr err = GetControlData(superview, 0, 
										   TViewNoCompositingCompatible::theViewTag, 
										   sizeof(tview), &tview, 0);
				if (err == noErr && tview) {
					control = superview;
				}
			}
        }
		DisposeControl(control);
    }
    // UnRegister standard SDK controls
    // Note : this fails with MacOS X < 10.2.4 (bug in MacOS X)
    TViewNoCompositingCompatible::UnRegisterAllClasses();
    TImageCache::Reset();
    
    // Deactivate all the fonts from our bundle
	if (mBundleRef) {
		CFURLRef myResourcesURL = CFBundleCopyResourcesDirectoryURL(mBundleRef);
		if (myResourcesURL != NULL)
		{
			FSRef myResourceDirRef;
			FSSpec bundleResourceDirFSSpec;
			CFURLGetFSRef(myResourcesURL, &myResourceDirRef);
			OSStatus status = FSGetCatalogInfo(&myResourceDirRef, kFSCatInfoNone, NULL, NULL, &bundleResourceDirFSSpec, NULL);
			if (status == noErr)
				FMDeactivateFonts(&bundleResourceDirFSSpec, NULL, NULL, kFMDefaultOptions);
			CFRelease( myResourcesURL );
		}
		
		CFRelease(mBundleRef);
	}
    if (mEditor)
        delete mEditor;
}
// -----------------------------------------------------------------------------
//! Called before the GUI has been created
void CAUCarbonViewNib::InitWindow(CFBundleRef sBundle) // Register custom controls...
{
    HIViewRef		view;
    // From Apple's Code :
    // Workaround: Unfortunately, at the time we call RegisterClass below,
    // the HIView base class isn't registered. It's supposed to be automatically
    // registered, but something is going wrong in HIToolbox. We can force
    // it to register by creating any arbitrary view. Here, we simply create
    // and release a scroll view. That's enough to make sure the HIView base
    // class is registered. Sorry folks.
    
    // From me (AA) : Is it still needed ???
    HIScrollViewCreate( kHIScrollViewOptionsVertScroll, &view );
    CFRelease( view );
}
// -----------------------------------------------------------------------------
//! Called after the GUI has been created
void CAUCarbonViewNib::FinishWindow(CFBundleRef sBundle)
{
}
// -----------------------------------------------------------------------------
//! Create the AU GUI
OSStatus CAUCarbonViewNib::CreateUI(Float32 x, Float32 y)
{
    Rect controlFrame;
    WindowRef win;
    IBNibRef      inNibRef;
        
	mBundleID = GetBundleID();
    mBundleRef = CFBundleGetBundleWithIdentifier (GetBundleID());
    CFRetain (mBundleRef);
    InitWindow(mBundleRef);
    
	InitXML();
    
    // Activate all the fonts from our bundle
    CFURLRef myResourcesURL = CFBundleCopyResourcesDirectoryURL(mBundleRef);
    if (myResourcesURL != NULL)
    {
        FSRef myResourceDirRef;
        FSSpec bundleResourceDirFSSpec;
        CFURLGetFSRef(myResourcesURL, &myResourceDirRef);
        OSStatus status = FSGetCatalogInfo(&myResourceDirRef, kFSCatInfoNone, NULL, NULL, &bundleResourceDirFSSpec, NULL);
        if (status == noErr)
            status = FMActivateFonts(&bundleResourceDirFSSpec, NULL, NULL, kFMLocalActivationContext);
        CFRelease( myResourcesURL );
    }	
    
    HideControl(mCarbonPane);
    
    // Register standard SDK controls - other classes must be registered in the application view class
    TViewNoCompositingCompatible::SetClassBundle(mBundleRef);
    TViewNoCompositingCompatible::RegisterClassForBundleID<TValuePictControl>(mBundleID);
    TViewNoCompositingCompatible::RegisterClassForBundleID<TPngPictControl>(mBundleID);
    TViewNoCompositingCompatible::RegisterClassForBundleID<TValuePictButtonControl>(mBundleID);
    TViewNoCompositingCompatible::RegisterClassForBundleID<TKnobControl>(mBundleID);
    //TViewNoCompositingCompatible::RegisterClassForBundleID<TKnobArcControl>(mBundleID);
    TViewNoCompositingCompatible::RegisterClassForBundleID<TSliderControl>(mBundleID);
    //TViewNoCompositingCompatible::RegisterClassForBundleID<TMeterControl>(mBundleID);
    //TViewNoCompositingCompatible::RegisterClassForBundleID<TMeterPartialControl>(mBundleID);
    TViewNoCompositingCompatible::RegisterClassForBundleID<TValueTextControl>(mBundleID);
    //TViewNoCompositingCompatible::RegisterClassForBundleID<TImageButton>(mBundleID);
    //TViewNoCompositingCompatible::RegisterClassForBundleID<TImagePopup>(mBundleID);
    TViewNoCompositingCompatible::RegisterClassForBundleID<TTransparentEditText>(mBundleID);
    TViewNoCompositingCompatible::RegisterClassForBundleID<TMultiPane>(mBundleID);
    TViewNoCompositingCompatible::RegisterClassForBundleID<TPushButton>(mBundleID);
    
    bool hasCompositing = IsCompositWindow();
    if (HasBackgroundPict() && !hasCompositing) { // Patching QD is only needed with non compositing windows
        mBackgroundImage = TImageCache::GetImage(mBundleRef, GetBackgroundFilename(), NULL, NULL);
        if (mBackgroundImage) {
            if (PatchEraseRect()) {
                // Add a property to the Window so anybody with a window ptr can get the AUCarbonViewNib
                CAUCarbonViewNib *ptr = this;
                SetWindowProperty(mCarbonWindow, 'AGUI', 'CVNB', sizeof(CAUCarbonViewNib *), &ptr);
                
                mCurrentProcsPtr = GetPortGrafProcs( GetWindowPort( mCarbonWindow ) );
                if ( mCurrentProcsPtr != NULL )
                    mProcs = *mCurrentProcsPtr;
                else
                    SetStdCProcs( &mProcs );
                mProcs.rectProc = NewQDRectUPP( BackgroundEraseRectProc );
                SetPortGrafProcs( GetWindowPort( mCarbonWindow ), &mProcs );
            }
        }
    }
    
	if (!BuildFromXML()) {
		CreateNibReferenceWithCFBundle(mBundleRef, GetWindowNibName(), &inNibRef);
		TViewNoCompositingCompatible::SetWindowReady(false);
		if (!hasCompositing || (CreateWindowFromNib( inNibRef, GetWindowCompositingName(), &win) != noErr)) {
			CreateWindowFromNib( inNibRef, GetWindowNoCompositingName(), &win);
		}
		OSStatus result = noErr;
		ControlRef control;
		
		ControlID controlID = { kAUNibControlSignature, kAUNibUserPaneControlID }; // The user pane to embed
		result = GetControlByID (win, &controlID, &control);
		
		AUGUI::oid_t rootOid = AUGUI::kRootPane;
		AUGUI::oid_t parentOid = rootOid;
		
		if (result == noErr) {
#if defined(__i386__)
			fixTitles(control);
#endif
			GetControlBounds (control, &controlFrame);
			OffsetRect (&controlFrame, (short)x, (short)y);
			SetControlBounds (control, &controlFrame);
			
			if (HasBackgroundPict()) {
				HIViewRef backCtrl;
				CFStringRef reg = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%@.%@"), mBundleID,
														   TPngPictControl::GetControlName());
				TViewNoCompositingCompatible::Create<TPngPictControl>(&backCtrl, 0, 0, reg);
				CFRelease(reg);
				SetControlBounds (backCtrl, &controlFrame);
				SetControlTitleWithCFString(backCtrl, GetBackgroundFilename());
				
				// Describing the new object in the xml
				parentOid = AUGUI::newOid(parentOid, 0x8000);
				mOids[backCtrl] = parentOid;
				/* Updating the gui description */
				CACFDictionary element(true);
				
				TViewNoCompositingCompatible *tview = 0;
				//  our  object should be a TViewNoCompositingCompatible
				OSErr err = GetControlData(backCtrl, 0, 
										   TViewNoCompositingCompatible::theViewTag, 
										   sizeof(tview), &tview, 0);
				if (err == noErr && tview) {
					// Looking for the element description in the dictionary
					if (GetDictionaryForOid(element, parentOid)) {
						tview->ReadFromDictionary(element);
					} else {
						tview->WriteToDictionary(element);
					}
					// Adding parent info
					element.AddUInt32(CFSTR("parent"), rootOid);
					AddDictionaryForOid(element, parentOid);
				}
				EmbedControl( backCtrl );
				if (hasCompositing) {
					::HIViewAddSubview( backCtrl, control );
				} else {
					::EmbedControl( control, backCtrl );
				}
				ShowControl( backCtrl);
				
			} else {
				EmbedControl( control);
			}
		}
		TViewNoCompositingCompatible::SetWindowReady(true);
		mRootUserPane = control;
		
		// now automatically build all the multipane (mpan, x) in the root user pane
		BuildDynamicViews(mRootUserPane, parentOid);
		
		// Bind all controls to their parameters
		BindPane(mRootUserPane, parentOid);
		
		// Updating the root description
		CACFDictionary element(true);
		
		// Looking for the element description in the dictionary
		GetDictionaryForOid(element, rootOid);
		{
			// Write Frame
			HIRect frame;
			HIViewGetFrame(mRootUserPane, &frame);
			CACFDictionary dict(true);
			CFDictionaryRef f = 0;
			if (element.GetDictionary(CFSTR("frame"), f)) {
				dict.SetCFMutableDictionaryFromCopy(f,true);
				//CFRelease(dict);
				dict.GetFloat32(CFSTR("x"), frame.origin.x);
				dict.GetFloat32(CFSTR("y"), frame.origin.y);
				dict.GetFloat32(CFSTR("w"), frame.size.width);
				dict.GetFloat32(CFSTR("h"), frame.size.height);
				HIViewSetFrame(mRootUserPane, &frame);
			} else {
				dict.AddFloat32(CFSTR("x"), frame.origin.x);
				dict.AddFloat32(CFSTR("y"), frame.origin.y);
				dict.AddFloat32(CFSTR("w"), frame.size.width);
				dict.AddFloat32(CFSTR("h"), frame.size.height);
				element.AddDictionary(CFSTR("frame"), dict.GetDict());
			}
		}
		AddDictionaryForOid(element, rootOid);

		AddUnknownControls(mCarbonPane, rootOid);
		
		ReleaseWindow(win);
		DisposeNibReference(inNibRef);
    }

	// register for clicks in our root pane since we completely cover up
	// the actual root pane, mCarbonPane
	// We also register for kEventCommandProcess, to handle plug-in level commands and
	// kEventWindowContextualMenuSelect to handle MIDI learn on ctrl-click on controls
	EventTypeSpec events[] = { {kEventClassCommand, kEventCommandProcess} };
	WantEventTypes(GetControlEventTarget(mCarbonPane), GetEventTypeCount(events), events);
	EventTypeSpec events2[] = { {kEventClassWindow, kEventWindowContextualMenuSelect} };
	WantEventTypes(GetWindowEventTarget(mCarbonWindow), GetEventTypeCount(events2), events2);
	
    FinishWindow(mBundleRef);
    if (HasTimer())		
        InstallEventLoopTimer(
                              GetCurrentEventLoop(),
                              0,
                              kEventDurationMillisecond * TimerResolution(),
                              NewEventLoopTimerUPP( TimerProc ),
                              this,
                              &mTimer );
    
    ShowControl(mCarbonPane);
    return noErr;
}
// -----------------------------------------------------------------------------
HIViewRef CAUCarbonViewNib::BuildViewFromOid(AUGUI::oid_t oid)
{
	HIViewRef view = 0;
	CACFDictionary desc(true);
	if (!GetDictionaryForOid(desc, oid)) {
		return 0;
	}				
	// Let's find our parent...
	HIViewRef parentView = 0;
	UInt32 id;
	if (desc.GetUInt32(CFSTR("parent"), id)) {
		AUGUI::oid_t parentOid = id;
		if (mViews.find(parentOid) == mViews.end()) {
			CFStringRef parent =  CFStringCreateWithFormat(kCFAllocatorDefault, 0, CFSTR("%08X"), parentOid);
			parentView = BuildViewFromOid(parentOid);
			CFRelease(parent);
		} else {
			parentView = mViews[parentOid];
		}
	}
	if (parentView) {
		// Let's create the view
		CFStringRef className;
		if (desc.GetString(CFSTR("type"), className)) {
			if (TViewNoCompositingCompatible::NewViewForCurrentBundle(&view, 0, 0, className) == noErr) {
				TViewNoCompositingCompatible *tview;
				GetControlData(view, 0, TViewNoCompositingCompatible::theViewTag, sizeof(tview), &tview, 0);
				HIViewAddSubview (parentView, view); 
				tview->ReadFromDictionary(desc);
				HIViewSetVisible(view, 1);
			}
		}
	}
	if (view) {
		mViews[oid] = view;
		mOids[view] = oid;
	}
	if (mRootUserPane == 0 && view) {
		mRootUserPane = view;
		Rect r;
		GetControlBounds(view, &r);
		SizeControl(mCarbonPane, r.right - r.left, r.bottom-r.top);
		HIViewID id = {'AUid', 9999};
		SetControlID(mRootUserPane, &id);
	}
	return view;
}
// -----------------------------------------------------------------------------
bool CAUCarbonViewNib::BuildFromXML()
{
//	return false;
	
	mViews[AUGUI::kRootPane] = mCarbonPane;
	mOids[mCarbonPane] = AUGUI::kRootPane;
	
	TViewNoCompositingCompatible::SetWindowReady(false);
	mRootUserPane = 0;

	// We should first resize mCarbonPane to the size of our root view in the XML
	// For each view of the dictionary :
	//		build the view
	int count = mXml.Size();
	if (count) {
	CFStringRef *keys = new CFStringRef[count];
	mXml.GetKeys((const void **)keys);
	for (int i = 0; i < count; ++i) {
		AUGUI::oid_t oid;
		char	buffer[10];
		CFStringGetCString(keys[i], buffer, 10, CFStringGetSystemEncoding());
		sscanf(buffer, "%X", &oid);
		if (mViews.find(oid) == mViews.end()) {
			BuildViewFromOid(oid);
		}		
	}
	delete[] keys;
	}
	
	TViewNoCompositingCompatible::SetWindowReady(true);
	BindPane(mRootUserPane, AUGUI::kRootPane);

	return count > 0;
}
// -----------------------------------------------------------------------------
void CAUCarbonViewNib::GetPaneBounds(Rect *r)
{
    GetControlBounds (mCarbonPane, r);
}

// -----------------------------------------------------------------------------
//! Bind the control to its parameter
void CAUCarbonViewNib::BindView(HIViewRef control, AUGUI::oid_t oid)
{
    int id;
    if (GetControlProperty(control, 'AGUI', 'auid', sizeof(id), NULL, &id) == noErr) {   
        ControlKind kind;
        GetControlKind(control, &kind);
        CAAUParameter tParam(mEditAudioUnit, id, kAudioUnitScope_Global, 0);
        int nbDigit = 5;
        AUCarbonViewControl::ControlType controlType = AUCarbonViewControl::kTypeContinuous;
        
        switch (kind.kind) {
            case kControlKindCheckBox:
            case kControlKindRadioGroup: {
                controlType = AUCarbonViewControl::kTypeDiscrete;
                break;
            }
            case kControlKindEditText:
            case kControlKindStaticText:
            case kControlKindEditUnicodeText: {
                CGImageRef textBackground = 0;
                if (GetTextBackgroundFilename())
                    textBackground = TImageCache::GetImage(mBundleRef, GetTextBackgroundFilename(), NULL, NULL);
                
                controlType = AUCarbonViewControl::kTypeText;
                if (kind.signature == 'airy') {
                    SetControlData(control,0, kTransparentEditTextBackgroundImageTag, sizeof(CGImageRef), &textBackground);
                }
                if (tParam.IsIndexedParam())
                    nbDigit = (int)log10(tParam.ParamInfo().maxValue)+1;
                else if (tParam.ParamInfo().unit == kAudioUnitParameterUnit_Boolean)
                    nbDigit = 1;
                
                if (textBackground)
                    CFRelease(textBackground);
                break;
            }
            case TMultiPane::kControlKind: {
                int mini, maxi, val;
                mini = int(tParam.ParamInfo().minValue);
                maxi = int(tParam.ParamInfo().maxValue);
                val = int(tParam.ParamInfo().defaultValue);
                SetControl32BitMinimum(control, mini);
                SetControl32BitMaximum(control, maxi);
                SetControl32BitValue(control, val);
				// We should create/get a dictionary with the TMultiPane title
                BuildMultiPane(control, oid); 
                break;
            }
            case kControlKindPopupArrow:
            case kControlKindPopupButton:
                if (tParam.HasNamedParams()) {
                    MenuRef menuRef;
                    CreateNewMenu( 1, 0, &menuRef);
                    for (int i = 0; i < tParam.GetNumIndexedParams(); ++i)
                    {
                        AppendMenuItemTextWithCFString (menuRef, tParam.GetParamName(i), 0, 0, 0);
                    }
                    
                    SetControlData(control, 0, kControlPopupButtonMenuRefTag, sizeof(menuRef), &menuRef);
                    SetControl32BitMaximum(control, tParam.GetNumIndexedParams());
                    controlType = AUCarbonViewControl::kTypeDiscrete;
                }
                break;
            default: {
                int mini, maxi;
                
                if (tParam.IsIndexedParam()) {
                    controlType = AUCarbonViewControl::kTypeDiscrete;
                    mini = int(tParam.ParamInfo().minValue);
                    maxi = int(tParam.ParamInfo().maxValue);
                } else if (tParam.ParamInfo().unit == kAudioUnitParameterUnit_Boolean) {
                    mini = 0;
                    maxi = 1;
                } else {
                    controlType = AUCarbonViewControl::kTypeContinuous;
                    mini = 0;
                    maxi = 1000000000;
                }
                SetControl32BitMinimum(control, mini);
                SetControl32BitMaximum(control, maxi);
                if (kind.kind == kControlKindSlider)
                    SetControlAction(control, AUCarbonViewControl::SliderTrackProc);                                                
                break;
            }
        }
        AUCarbonViewControl *auvc = new AUCarbonViewEnhancedControl(this, mParameterListener,
                                                                    controlType, tParam, control, 0, 0, nbDigit);
        //				auvc->Update(true); // To be sure the control has the right value
        auvc->Bind();
        AddControl(auvc);
    }
}

// -----------------------------------------------------------------------------
//! Bind all the controls embedded in the pane to their associated parameters
void CAUCarbonViewNib::BindPane(HIViewRef thePane, AUGUI::oid_t parent)
{
    // Built the Font Styte for text controls
    OSStatus styleOK = 0;
    ControlFontStyleRec textStyle = { 0 };
    if (GetFontname()) {
        char fname[255];
        CFStringGetCString(GetFontname(), fname+1, sizeof(fname)-1, kCFStringEncodingASCII);
        *fname = strlen(fname+1);
        textStyle.flags = kControlUseFontMask | kControlUseSizeMask | kControlUseForeColorMask | kControlUseJustMask;
        textStyle.font = FMGetFontFamilyFromName((const unsigned char *)fname);
        textStyle.size = GetFontsize();
        textStyle.just = GetTextJustification(); // Should we have a GetSomething for that ?
        //textStyle.foreColor.red = textStyle.foreColor.green = textStyle.foreColor.blue = (unsigned short)0; // Same here...
        textStyle.foreColor.red = 180*256;
		textStyle.foreColor.green = 248*256;
		textStyle.foreColor.blue = 255*256;
        styleOK = 1;
    }
    
    UInt32 size;
    Boolean writable;
    AudioUnitParameterID *ids;
    AudioUnitGetPropertyInfo(mEditAudioUnit,
                             kAudioUnitProperty_ParameterList,
                             kAudioUnitScope_Global,
                             0,
                             &size,
                             &writable);
    ids = new AudioUnitParameterID[size/sizeof(AudioUnitParameterID)];
    AudioUnitGetProperty(mEditAudioUnit,
                         kAudioUnitProperty_ParameterList,
                         kAudioUnitScope_Global,
                         0,
                         ids,
                         &size);
    for (unsigned int i = 0; i <  size/sizeof(AudioUnitParameterID); ++i) {
		CACFDictionary element(true);
		
        CAAUParameter tParam(mEditAudioUnit, ids[i], kAudioUnitScope_Global, 0);
        ControlRef control;
        for (int j = -1; j < kMaxNumberOfControlsForEachParameter; ++j) {
            HIViewID hiID = { kAUNibControlSignature, j*1000+ids[i] };
            if (j == -1)
                hiID.id = -1;
            if (HIViewFindByID(thePane, hiID, &control) == noErr) {
				int id;
				// Bind views only once
				if (GetControlProperty(control, 'AGUI', 'auid', sizeof(ids[i]), 0, &id) != noErr) {
					// Set a property so we can find the ParamID from the control
					SetControlProperty(control, 'AGUI', 'auid', sizeof(ids[i]), &ids[i]);
					if (styleOK)
						SetControlData(control,0, kControlFontStyleTag, sizeof(textStyle), &textStyle);    
					AUGUI::oid_t myoid = AUGUI::newOid(parent, hiID.id);
					BindView(control, myoid);
					
					/* Updating the gui description */
					CACFDictionary element(true);
					
					TViewNoCompositingCompatible *tview = 0;
					if (mOids.find(control) == mOids.end()) {
					//  our  object should be a TViewNoCompositingCompatible
					OSErr err = GetControlData(control, 0, 
											   TViewNoCompositingCompatible::theViewTag, 
											   sizeof(tview), &tview, 0);
					if (err == noErr && tview) {
						// Looking for the element description in the dictionary
						if (GetDictionaryForOid(element, myoid)) {
							tview->ReadFromDictionary(element);
						} else {
							tview->WriteToDictionary(element);
						}
						// Adding parameter name info
						element.AddString(CFSTR("parameter"), tParam.ParamInfo().cfNameString);
						
						// Adding parent info
						element.AddUInt32(CFSTR("parent"), parent);
						
						AddDictionaryForOid(element, myoid);
						mOids[control] = myoid;
					}
					}
				}
            } else if (j >= 0) {
                // No more control for this parameter
                break;
            }
        }
    }
    delete[] ids;
	
#ifdef ATSU
    if (styleOK == noErr)
        ATSUDisposeStyle( textStyle );
#endif
}
// -----------------------------------------------------------------------------
//! Build all multipanes (kAUPanelControlSignature, x) contained in the pane
void CAUCarbonViewNib::BuildDynamicViews(HIViewRef thePane, AUGUI::oid_t parent)
{
    for (int i = 0; i < kMaxNumberOfMultipanesForEachPane; ++i) {
        for (int j = 0; j < kMaxNumberOfMultipanesForEachPane; ++j) {
            HIViewID hiID = { kAUPanelControlSignature, i+j*1000 };
            ControlRef control;
            if (HIViewFindByID(thePane, hiID, &control) == noErr) {
				AUGUI::oid_t oid = AUGUI::newOid(parent, hiID.id | 0xF000);
				
				/* Updating the gui description */
				CACFDictionary element(true);
				
				TViewNoCompositingCompatible *tview = 0;
				//  our  object should be a TViewNoCompositingCompatible
				OSErr err = GetControlData(control, 0, 
										   TViewNoCompositingCompatible::theViewTag, 
										   sizeof(tview), &tview, 0);
				if (err == noErr && tview) {					
					// Looking for the element description in the dictionary
					if (GetDictionaryForOid(element, oid)) {
						tview->ReadFromDictionary(element);
					} else {
						tview->WriteToDictionary(element);
					}
					// Adding parameter name
					element.AddUInt32(CFSTR("parent"), parent);
					AddDictionaryForOid(element, oid);
					mOids[control] = oid;
				}
				
                BuildMultiPane(control, oid);
            } else {
                break;
            }
        }
    }
}

void CAUCarbonViewNib::InitXML()
{
	CFURLRef urldir=CFBundleCopyResourcesDirectoryURL(mBundleRef);
	CFURLRef url=CFURLCreateCopyAppendingPathComponent(kCFAllocatorDefault, urldir, CFSTR("gui.xml"), 0);
	CFDictionaryRef dict = (CFDictionaryRef)CreatePropertiesFromXMLFile(url);
	if (dict) {
		mXml.SetCFMutableDictionaryFromCopy(dict, true);
	}
	CFRelease(url);
	CFRelease(urldir);
}

void CAUCarbonViewNib::SaveXML()
{
	CFURLRef urldir=CFBundleCopyResourcesDirectoryURL(mBundleRef);
	CFURLRef url=CFURLCreateCopyAppendingPathComponent(kCFAllocatorDefault, urldir, CFSTR("gui.xml"), 0);
	SavePropertiesToXMLFile(mXml.GetDict(), url);
	CFRelease(url);
	CFRelease(urldir);
}


//! Populate the multipane "control", according to its title and its min,max
void CAUCarbonViewNib::BuildMultiPane(HIViewRef control, AUGUI::oid_t parent)
{
    OSErr result;
    CFStringRef panelName;
    int id;
    if (GetControlData(control,0, TMultiPane::kSubPanelID, sizeof(id), &id, 0)
		== noErr) {
		if (id > -1) {
			// Hey ! I've already built this one
			return;
		}
	}
    SetControlData(control,0, TMultiPane::kSubPanelID, sizeof(mPaneID), &mPaneID);
    mPaneID++;
    
    int paneID;
    GetControlData(control,0, TMultiPane::kSubPanelID, sizeof(paneID), &paneID, 0);
    
    CopyControlTitleAsCFString(control, &panelName);
    
    int mini = GetControl32BitMinimum(control);
    int maxi = GetControl32BitMaximum(control);
    int val = GetControl32BitValue(control);
    
    // Here, we can have three levels of backgrounds :
    // - the one for the AU window
    // - one for the multipane (<title>.png)
    // - one for the subpane (<title><value>.png)
    // All all these can have some transparency, making possibilities quite nice...
    HIViewRef backCtrl;
    CFStringRef backImageName = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%@.png"), panelName);
    CGImageRef image = TImageCache::GetImage(mBundleRef, backImageName, NULL, NULL);
    bool imageFound = image != NULL;
    if (imageFound) {
        // Create a new default background
        CFStringRef reg = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%@.%@"), mBundleID,
                                                   TPngPictControl::GetControlName());
        TViewNoCompositingCompatible::Create<TPngPictControl>(&backCtrl, 0, 0, reg);
        CFRelease(reg);
        Rect controlFrame;
        GetControlBounds (control, &controlFrame);
        SetControlBounds (backCtrl, &controlFrame);
        SetControlTitleWithCFString(backCtrl, backImageName) ;
        if (!IsCompositWindow()) {
            result = ::EmbedControl( backCtrl, control);
        } else {
            TRect r = controlFrame;
            r.SetOrigin(0.f, 0.f);
            controlFrame = r;
            SetControlBounds (backCtrl, &controlFrame);
            result = ::HIViewAddSubview( control, backCtrl);
        }
        CFRelease(image);
        
        // Set its signature (mpan, 9999)
        ControlID controlID = {TMultiPane::kSubPanelSignature+paneID, 9999};
        SetControlID(backCtrl, &controlID);
    } else {
        backCtrl = control;
    }
    CFRelease(backImageName);
    
    IBNibRef      inNibRef = 0;
    result = CreateNibReferenceWithCFBundle(mBundleRef, panelName, &inNibRef);
	if (result == noErr) 
	{
		for (int i = mini; i <= maxi; ++i) {
			ControlRef pane;
			WindowRef win;
			
			// Search for a local background for this pane
			HIViewRef back;
			CFStringRef backImageName = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%@%d.png"), panelName, i);
			CGImageRef image = TImageCache::GetImage(mBundleRef, backImageName, NULL, NULL);
			bool imageFound = image != NULL;
			if (imageFound) {
				// Create a new default background
				CFStringRef reg = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%@.%@"), mBundleID,
														   TPngPictControl::GetControlName());
				TViewNoCompositingCompatible::Create<TPngPictControl>(&back, 0, 0, reg);
				CFRelease(reg);
				Rect controlFrame;
				GetControlBounds (backCtrl, &controlFrame);
				SetControlBounds (back, &controlFrame);
				SetControlTitleWithCFString(back, backImageName) ;
				if (!IsCompositWindow()) {
					result = ::EmbedControl( back, backCtrl);
				} else {
					TRect r = controlFrame;
					r.SetOrigin(0.f, 0.f);
					controlFrame = r;
					SetControlBounds (back, &controlFrame);
					result = ::HIViewAddSubview( backCtrl, back);
				}
				CFRelease(image);
			} else {
				back = backCtrl;
			}
			CFRelease(backImageName);
			
			// Get the window
			CFStringRef winName = 0;
			OSStatus res = noErr;
			if (IsCompositWindow()) {
				winName = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%dCompositing"), i);
				res = CreateWindowFromNib( inNibRef, winName, &win);
				CFRelease(winName);
			}
			if (res != noErr || !IsCompositWindow()) {
				winName = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%d"), i);
				res = CreateWindowFromNib( inNibRef, winName, &win);
				CFRelease(winName);
			}
			
			if (res == noErr) {
				// We should create a new entry in the dictionary 
				AUGUI::oid_t myoid = AUGUI::newPaneOid(parent);
				CACFDictionary element(true);
				GetDictionaryForOid(element, myoid);
				element.AddString(CFSTR("type"), CFSTR("subpane"));
				element.AddString(CFSTR("title"), panelName);
				element.AddUInt32(CFSTR("index"), i);
				element.AddUInt32(CFSTR("parent"), parent);
				AddDictionaryForOid(element, myoid);
				
				// Get its root control
				ControlID controlID = { kAUNibControlSignature, kAUNibUserPaneControlID }; // The user pane to embed
				result = GetControlByID (win, &controlID, &pane);
				if (result == noErr) {
#if defined(__i386__)
					fixTitles(pane);
#endif
					// Embed it into control
					if (IsCompositWindow()) {
						result = ::HIViewAddSubview( back, pane );
					} else {
						Rect controlFrame;
						GetControlBounds (back, &controlFrame);
						SetControlBounds (pane, &controlFrame);
						result = ::EmbedControl( pane, back);
					}
					
					if (imageFound) {
						pane = back;
					}
					HIViewSetVisible( pane, i == val);
					// Set its signature (pane,i)
					controlID.signature = TMultiPane::kSubPanelSignature+paneID;
					controlID.id = i;
					result = SetControlID(pane, &controlID);
					
					// Bind it
					BindPane(pane, myoid);
					
					// Build multipanes contained in the pane.
					BuildDynamicViews(pane, myoid);
					
					mOids[pane] = myoid;
				}
				// Release the window
				ReleaseWindow(win);
			}
			// That's all...
		}
	}
    ShowControl(backCtrl);
    
    CFRelease(panelName);
    if (inNibRef)
		DisposeNibReference(inNibRef);
}

//! Display the pict has an overlay on the current GUI
void CAUCarbonViewNib::DisplayOverlay(CFStringRef pictName)
{
	HIViewRef about;
	CGImageRef image = TImageCache::GetImage(mBundleRef, pictName, NULL, NULL);
	if (image != NULL) {
		
		CFStringRef reg = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%@.%@"), mBundleID,
												   TPngPictControl::GetControlName());
		TViewNoCompositingCompatible::Create<TPngPictControl>(&about, 0, 0, reg);
		CFRelease(reg);
		Rect controlFrame;
		GetControlBounds (mRootUserPane, &controlFrame);
		SetControlBounds (about, &controlFrame);
		SetControlTitleWithCFString(about, pictName) ;
		if (!IsCompositWindow()) {
			::EmbedControl( about, mCarbonPane);
			if (mCarbonPane != HIViewGetSuperview(mRootUserPane))
				HideControl(HIViewGetSuperview(mRootUserPane));
		} else {
			TRect r = controlFrame;
			r.SetOrigin(0.f, 0.f);
			controlFrame = r;
			SetControlBounds (about, &controlFrame);
			::HIViewAddSubview( mCarbonPane, about);
		}
		CFRelease(image);
		SetControlCommandID(about, 'Rmve');
		HIViewSetZOrder(about, kHIViewZOrderAbove, NULL);
		ShowControl(about);
	}	
}

//! For special handling of events on a control
bool CAUCarbonViewNib::HandleEventForView(EventRef event, HIViewRef view)
{
	return false; // Event no handled
}

//! We handle any clicks in the root user pane just like they are handled
//! in the AUCarbonViewBase class, namely by resetting keyboard focus. This
//! is done because our user pane completely covers up the actual root pane,
//! mCarbonPane, which AUCarbonViewBase is watching for clicks on.

bool CAUCarbonViewNib::HandleEventPlayMode(EventRef event)
{
    bool res = false;
    
    TCarbonEvent theEvent = event;
    UInt32 eclass = theEvent.GetClass();
    UInt32 ekind = theEvent.GetKind();
	
    // handle user handling
    if ( (eclass == kEventClassControl) )
    {
        ControlRef control;
        GetEventParameter(event, kEventParamDirectObject, typeControlRef, NULL, sizeof(ControlRef), NULL, &control);
		if (HandleEventForView(event, control))
			return true;
    }
	
    if ( (eclass == kEventClassWindow) && (ekind == kEventWindowContextualMenuSelect) ) 
    {
        Boolean res = false;
        ControlRef theControl = 0;
        HIPoint thePoint;
        Point testPoint;
        GetEventParameter( event, kEventParamMouseLocation, typeHIPoint, NULL, sizeof( thePoint ), NULL, &thePoint );
        testPoint.h = int(thePoint.x);
        testPoint.v = int(thePoint.y);
        AUGUI::ConvertGlobalToWindowPoint(mCarbonWindow, testPoint);
        FindControl(testPoint, mCarbonWindow, &theControl);
        if (theControl) {
			if (!HandleEventForContextualMenu(event, theControl)) {
				HandleControlContextualMenuClick(theControl, testPoint, &res);
				int id;
				if (GetControlProperty(theControl, 'AGUI', 'auid', sizeof(id), NULL, &id) == noErr && id != -1) {
					AudioUnitSetProperty(mEditAudioUnit,
										 kProperty_MIDILearn,
										 kAudioUnitScope_Global,
										 0,
										 &id,
										 sizeof(id));
					res = true;
				}
			}
        }
        return res;
    }
    
    // catch any clicks on the embedding pane in order to clear focus
    if ( (eclass == kEventClassControl) && (ekind == kEventControlClick) )
    {
        ControlRef clickedControl;
        GetEventParameter(event, kEventParamDirectObject, typeControlRef, NULL, sizeof(ControlRef), NULL, &clickedControl);
        if (clickedControl == mRootUserPane)
        {
            // reset focus
            ClearKeyboardFocus(GetControlOwner(mCarbonPane));
            res = true;
        } else {
			UInt32 modifiers;
            theEvent.GetParameter( kEventParamKeyModifiers, &modifiers );
            if ((modifiers & (optionKey | cmdKey)) == (optionKey)) {
				int id;
				if (GetControlProperty(clickedControl, 'AGUI', 'auid', sizeof(id), NULL, &id) == noErr && id != -1) {
					CAAUParameter tParam(mEditAudioUnit, id, kAudioUnitScope_Global, 0);
					tParam.SetValue(0, 0, tParam.ParamInfo().defaultValue);
					res = true;
				} 
			}
		}
    }
    if (!res && (eclass == kEventClassCommand) && (ekind == kEventCommandProcess))
    {
        HICommandExtended cmd;
        GetEventParameter( event, kEventParamDirectObject, typeHICommand, NULL, sizeof( cmd ), NULL, &cmd );
        switch (cmd.commandID) {
            case kAUPanelControlSignature:
                if (cmd.attributes & kHICommandFromControl)
                {
                    HIViewRef source = cmd.source.control;
                    // Get the control value
                    SInt32 value = GetControl32BitValue(source);
                    // Get the control ID
                    HIViewID id;
                    GetControlID(source, &id);
                    res = SwitchPane(id.id % 1000, value);
                }
                break;
            case 'Abou': { // show "About" pict
                           // Create a new 'PngPict' with the "about" title, and associate the "Rmve" command to it
                
				DisplayOverlay(GetAboutFilename());
				res = true;
                break;
            }
            case 'Rmve': // remove "About" pict
                if (cmd.attributes & kHICommandFromControl)
                {
                    HIViewRef ctrl = cmd.source.control;
                    DisposeControl(ctrl);
                    
                    if (mCarbonPane != HIViewGetSuperview(mRootUserPane))
                        ShowControl(HIViewGetSuperview(mRootUserPane));
                    res = true;
                }
                break;
            default:
                res = HandleCommand(event, cmd);
                break;
        }
    }
    return res || AUCarbonViewBase::HandleEvent(NULL,event);
}

bool CAUCarbonViewNib::HandleEvent(EventHandlerCallRef inHandlerRef, EventRef event)
{
    TCarbonEvent theEvent = event;
    
    if (SupportEditMode()) {
        // catch any clicks on background with alt+cmd modifiers to enter/leave edit mode
        if ((theEvent.GetClass() == kEventClassControl) && (theEvent.GetKind() == kEventControlClick))
        {
            UInt32 modifiers;
            theEvent.GetParameter( kEventParamKeyModifiers, &modifiers );
            if ((modifiers & (optionKey | cmdKey)) == (optionKey | cmdKey)) {
                ControlRef clickedControl = 0;
                theEvent.GetParameter(kEventParamDirectObject, &clickedControl);
                if ((clickedControl == mCarbonPane) || (clickedControl == mRootUserPane))
                {
                    SetEditMode(!EditMode());
                    return true;
                }
			}
        }
    }
    if (!EditMode())
        return HandleEventPlayMode(event);
    else
        return false;
}

// switch pane (kAUPanelControlSignature,<paneID>) to paneNumber subpanel
int CAUCarbonViewNib::SwitchPane(int paneID, int paneNumber)
{
    HIViewID id = { kAUPanelControlSignature, paneID };
    HIViewRef pane;
    OSStatus result = HIViewFindByID(mRootUserPane, id, &pane);
    if (result == noErr) {
        SetControl32BitValue(pane, paneNumber);
    }
    return true;
}

void CAUCarbonViewNib::RegisterPropertyChanges(AudioUnitPropertyID inPropertyID)
{
    if (!mPropertiesRegistered.count(inPropertyID)) {
        AudioUnitAddPropertyListener(mEditAudioUnit, inPropertyID,  
                                     PropertyChangedProc, this);
        mPropertiesRegistered.insert(inPropertyID);
    }
}

void CAUCarbonViewNib::UnRegisterPropertyChanges(AudioUnitPropertyID inPropertyID)
{
    if (mPropertiesRegistered.count(inPropertyID)) {
        AudioUnitRemovePropertyListener(mEditAudioUnit, inPropertyID,  
                                        PropertyChangedProc);
        mPropertiesRegistered.erase(inPropertyID);
    }
}

void CAUCarbonViewNib::UnRegisterAllPropertiesChanges()
{
    std::set<AudioUnitPropertyID>::iterator i;    
    for (std::set<AudioUnitPropertyID>::iterator i = mPropertiesRegistered.begin(); 
         i != mPropertiesRegistered.end();
         ++i)   
    {
        AudioUnitRemovePropertyListener(mEditAudioUnit, *i,  
                                        PropertyChangedProc);
        mPropertiesRegistered.erase(*i);
    }
}

void CAUCarbonViewNib::PropertyHasChanged(AudioUnitPropertyID inPropertyID, AudioUnitScope inScope,  
                                          AudioUnitElement inElement)
{
}

void CAUCarbonViewNib::PropertyChangedProc(void* inUserData, AudioUnit inComponentInstance,  
                                           AudioUnitPropertyID inPropertyID, AudioUnitScope inScope,  
                                           AudioUnitElement inElement)
{
    CAUCarbonViewNib *This = (CAUCarbonViewNib *)inUserData;
    This->PropertyHasChanged(inPropertyID, inScope, inElement);
}

void CAUCarbonViewNib::SetEditMode(bool mode)
{ 
    if (mode == false) {
		// We should save here all the xml files
        delete mEditor;
        mEditor = 0;
        mEditMode = false;
    } else {
        mEditor = new CAUCarbonViewEditor(this);
        if (mEditor->IsValid()) {
            mEditor->ShowHide(true);
            mEditMode = true; 
        }
    }
}

void CAUCarbonViewNib::updateXMLForControl(HIViewRef control)
{
	if (mOids.find(control) != mOids.end()) {
		AUGUI::oid_t myoid = mOids[control];
		
		/* Updating the gui description */
		CACFDictionary element(true);
		
		TViewNoCompositingCompatible *tview = 0;
		//  our  object should be a TViewNoCompositingCompatible
		OSErr err = GetControlData(control, 0, 
								   TViewNoCompositingCompatible::theViewTag, 
								   sizeof(tview), &tview, 0);
		if (err == noErr && tview) {
			// Looking for the element description in the dictionary
			if (GetDictionaryForOid(element, myoid)) {
				tview->WriteToDictionary(element);
				AddDictionaryForOid(element, myoid);
			}
		}
	}
}


void CAUCarbonViewNib::updateControlFromXML(HIViewRef control)
{
	if (mOids.find(control) != mOids.end()) {
		AUGUI::oid_t myoid = mOids[control];
		
		/* Updating the gui description */
		CACFDictionary element(true);
		
		TViewNoCompositingCompatible *tview = 0;
		//  our  object should be a TViewNoCompositingCompatible
		OSErr err = GetControlData(control, 0, 
								   TViewNoCompositingCompatible::theViewTag, 
								   sizeof(tview), &tview, 0);
		if (err == noErr && tview) {
			// Looking for the element description in the dictionary
			if (GetDictionaryForOid(element, myoid)) {
				tview->ReadFromDictionary(element);
			}
		}
	}
}

bool CAUCarbonViewNib::GetDictionaryForOid(CACFDictionary &element, AUGUI::oid_t oid)
{
	bool found;
	/* Find the gui description */
	CFStringRef str =  CFStringCreateWithFormat(kCFAllocatorDefault, 0, CFSTR("%08X"), oid);
	
	CFDictionaryRef dict = 0;
	found = mXml.GetDictionary(str, dict);	
	if (found) {
		element.SetCFMutableDictionaryFromCopy(dict,false);
	}
	
	CFRelease(str);	
	return found;
}

bool CAUCarbonViewNib::AddDictionaryForOid(CACFDictionary &element, AUGUI::oid_t oid)
{
	CFStringRef str =  CFStringCreateWithFormat(kCFAllocatorDefault, 0, CFSTR("%08X"), oid);
	mXml.AddDictionary(str, element.GetDict());
	CFRelease(str);
	return true;
}

// Add all controls not bound to any parameter to the XML
void CAUCarbonViewNib::AddUnknownControls(HIViewRef from, AUGUI::oid_t parent)
{	
	AUGUI::oid_t fromOid = parent;
	if (mOids.find(from) != mOids.end()) {
		fromOid = mOids[from];
	} else {
		TViewNoCompositingCompatible *tview = 0;
		OSErr err = GetControlData(from, 0, 
								   TViewNoCompositingCompatible::theViewTag, 
								   sizeof(tview), &tview, 0);
		if (err == noErr && tview) {
			fromOid = AUGUI::newOid(parent);
			mOids[from] = fromOid;

			tview->SaveMinMaxVal(true);
			
			// If the object is not already in the XML description, let's add it
			CACFDictionary element(true);
			if (GetDictionaryForOid(element, fromOid)) {
				tview->ReadFromDictionary(element);
			} else {
				tview->WriteToDictionary(element);
			}
			element.AddUInt32(CFSTR("parent"), parent);
			AddDictionaryForOid(element, fromOid);
		}
	}

	if (fromOid != parent) {
		// Adding parent info
		CACFDictionary element(false);
		GetDictionaryForOid(element, fromOid);
		element.AddUInt32(CFSTR("parent"), parent);
		AddDictionaryForOid(element, fromOid);
	}
	
	// Add embedded controls
	HIViewRef view = HIViewGetFirstSubview(from);
	while (view) {
		AddUnknownControls(view, fromOid);
		view = HIViewGetNextView(view);
	}
}
// -----------------------------------------------------------------------------
// End of file.
// -----------------------------------------------------------------------------
