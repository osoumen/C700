/*
 *  CAUCarbonViewNib.h
 *  CAUCarbonViewNib
 
 *
 *  Created by Airy André on Sun Oct 06 2002.
 *  Copyright (c) 2002 Airy André. All rights reserved.
 *
 */

/*! \mainpage AUGUIFramework home page
*
* \section intro Introduction
*
* The AUGUI framework is a free set of classes to be used to ease development of custom Carbon edit views for Mac OS X Audio Units.
* The classes are based on the HIFramework and on the AudioUnit SDK provided by Apple.
*
* The AUGUI framework is opensource and hosted on Sourceforge ( http://sourceforge.net/projects/augui )
                                                               *
                                                               * Project CVS repository :
                                                               * http://sourceforge.net/cvs/?group_id=73514
                                                               *
                                                               *
                                                               * \section doc Documentation
                                                               *
                                                               * This is a very premilary online documentation about AUGUIFramework.
                                                               *
                                                               * This documentation has been automatically extracted from the code, using doxygen
                                                               *
                                                               *
                                                               */

#ifndef __AUCARBONVIEWNIB_H__
#define __AUCARBONVIEWNIB_H__

#include <map>
#include <set>

// Plugin views are based on this class
#include "AUCarbonViewBase.h"
#include "CACFDictionary.h"

#include "AUOid.h"

//! The control signature that must be used for each of the controls in
//! your .nib file in order for them to be located during initialisation.
//! \sa CAUCarbonViewNib
#define kAUNibControlSignature 'AUid'

//! The control signature of a user pane that can change its content dynamically
#define kAUPanelControlSignature 'AUpn'

//! The control ID of the user pane that will be relocated as a subview of
//! mCarbonPane, the root pane of the audio unit edit view.
//! \sa CAUCarbonViewNib
#define kAUNibUserPaneControlID 9999

#define kDefaultWindowNibName CFSTR("window")
#define kDefaultCompositingWindowNibName CFSTR("windowCompositing")
#define kDefaultNoCompositingWindowNibName kDefaultWindowNibName
#define kDefaultBackgroundImageFileName CFSTR("AUBackground.png")
#define kDefaultAboutImageFileName CFSTR("about.png")

#define kProperty_MIDILearn 100000

/*!
* \class CAUCarbonViewNib
 * \brief This class implements a GUI pane initialized from a nib file
 * 
 * Your GUI class must inherit from CAUCarbonViewNib in order to automatically load your ".nib" file.
 * Then a few methods have to or can be overridden to customize the standard behaviors.
 * 
 * There are some rules to respect when you are contructing your ".nib" window :
 * - All controls must be contained in a UserPane with a control ID = {'AUid', 9999}
 * - All controls must have a control ID = {'AUid', n+1000*j}, where n is the id of the parameter to bind to the control, a j an integer starting at
 *  0. So it's better to keep all of your parameter ids in a 1000 range...
 * 	For example, if you want to bind a UnicodeTextControl and a Slider to parameter 5, set their control ID to {'AUid', 5} and {'AUid', 1005}
 * 	The maximum, minimum and current values will be automatically set by the SDK.
 * - Control with control ID = {'AUid', -1} is binded to all parameters. Of course, this control should be "read-only". Typically, this is a static
 * 	text which is used to display the value of the currently tracked knob.
 */

class CAUCarbonViewEditor;

class CAUCarbonViewNib: public AUCarbonViewBase
{
public:
    // Construction ------------------------------
    CAUCarbonViewNib(AudioUnitCarbonView inInstance);
    virtual ~CAUCarbonViewNib();
    
    // Configuration Methods -----------------------
    //! Must be overriden to return the AU bundle ID
    virtual CFStringRef GetBundleID() = 0;
    //! Name of main nib file (default = "window")
    virtual CFStringRef GetWindowNibName() { return kDefaultWindowNibName; }
    //! Name of window for compositing window (default = "windowCompositing")
    virtual CFStringRef GetWindowCompositingName() { return kDefaultCompositingWindowNibName; }
    //! Name of window for non compositing window (default = "window")
    virtual CFStringRef GetWindowNoCompositingName() { return kDefaultNoCompositingWindowNibName; }
    
    //! Override to return false if the GUI has no background
    virtual bool HasBackgroundPict() { return true; }
    //! Name of the background  file (default = "AUBackground.png")
    virtual CFStringRef GetBackgroundFilename() { return kDefaultBackgroundImageFileName; }
    //! Name of the about file (default = "about.png")
    virtual CFStringRef GetAboutFilename() { return kDefaultAboutImageFileName; }
    //! Name of the background image to use for text field (default = 0 = no image)
    virtual CFStringRef GetTextBackgroundFilename() { return 0; }
    //! Name of the font to use for text field (default = 0 = default font)
    virtual CFStringRef GetFontname() { return 0; }
    //! Size of the font to use for text field (default = 12)
    virtual int GetFontsize() { return 12; }
    //! Texts justification (default = right)
    virtual int GetTextJustification() { return teJustLeft; }

    //! Override to return false if the GUI has a background and no Apple standard control
    virtual bool PatchEraseRect() { return true; }
    
    //! Override to return false if you don't need the "idle" method
    virtual bool HasTimer() { return true;};
    //! Timer resolution in ms (default = 50)
    virtual float TimerResolution() { return 50.;};
    //! virtual method called every "TimerResolution()" ms
    virtual void Idle() {};
    
    //! Override to return true if you want to be able to use the online GUI editor
    virtual bool SupportEditMode() const { return false; }

    // Baseclass overrides -----------------------
    OSStatus CreateUI(Float32, 		// x position within parent window
                      Float32);		// y position within parent window
    
    //! Override to implement special action before the GUI pane is constructed (like registering custom controls)
    virtual void InitWindow(CFBundleRef sBundle); // Register custom controls...
                                                  //! Override to implement special action after the GUI pane is constructed
    virtual void FinishWindow(CFBundleRef sBundle); // Called after window creation
                                                    //! Handle application commands
    virtual bool HandleCommand(EventRef	inEvent, HICommandExtended &cmd) { return false;};
    
    //! Override if you want to do special processing on some events
    virtual bool HandleEventForView(EventRef event, HIViewRef view);

    //! Override if you want to do special processing on some events
    virtual bool HandleEventForContextualMenu(EventRef event, HIViewRef view) { return false; }

    virtual bool HandleEvent(EventHandlerCallRef inHandlerRef, EventRef event);
    
    // Used by QD patch for controls to have the window background (for non compositing windows)
    virtual CGImageRef GetBackgroundImage() { return mBackgroundImage; }
    virtual void GetPaneBounds(Rect *rect);
    virtual HIViewRef GetCarbonPane() const { return mCarbonPane; }
    virtual HIViewRef GetRootPane() const { return mRootUserPane; }
    
    virtual void PropertyHasChanged(AudioUnitPropertyID inPropertyID, AudioUnitScope inScope,  
                                    AudioUnitElement inElement);
    
    CFBundleRef GetBundleRef() { return mBundleRef; }
    
	void InitXML();
	void SaveXML();

	void updateXMLForControl(HIViewRef control);
	void updateControlFromXML(HIViewRef control);

protected:
	virtual	HIViewRef BuildViewFromOid(AUGUI::oid_t oid);
	virtual bool BuildFromXML();
    virtual int SwitchPane(int paneID, int paneNumber);
    virtual void BindPane(HIViewRef thePane, AUGUI::oid_t oid);
    virtual void BuildMultiPane(HIViewRef control, AUGUI::oid_t parent);
    virtual void BuildDynamicViews(HIViewRef control, AUGUI::oid_t parent);
    virtual void BindView(HIViewRef control, AUGUI::oid_t oid);
    
    virtual void RegisterPropertyChanges(AudioUnitPropertyID inPropertyID);
    virtual void UnRegisterPropertyChanges(AudioUnitPropertyID inPropertyID);
    virtual void UnRegisterAllPropertiesChanges();

    virtual bool HandleEventPlayMode(EventRef event);
    
	virtual void DisplayOverlay(CFStringRef pictName);
    
    static void PropertyChangedProc(void* inUserData, AudioUnit inComponentInstance,  
                                    AudioUnitPropertyID inPropertyID, AudioUnitScope inScope,  
                                    AudioUnitElement inElement);
    
    void SetEditMode(bool mode);
    bool EditMode() const { return mEditMode; };
    
	bool GetDictionaryForOid(CACFDictionary &element, AUGUI::oid_t);
	bool AddDictionaryForOid(CACFDictionary &element, AUGUI::oid_t oid);
	
	void AddUnknownControls(HIViewRef view, AUGUI::oid_t fromOid);
	
	// Baseclass overrides -----------------------
//    virtual void RespondToEventTimer (EventLoopTimerRef inTimer) { Idle(); }

    CQDProcs mProcs;
    CQDProcs* mCurrentProcsPtr;
    CGImageRef mBackgroundImage;
    CFStringRef mBundleID;
    CFBundleRef mBundleRef;
    HIViewRef mRootUserPane;
    
    int mPaneID;

    bool mEditMode;
    CAUCarbonViewEditor *mEditor;

	CACFDictionary mXml;
	
    EventLoopTimerRef mTimer;
	
    std::set<AudioUnitPropertyID> mPropertiesRegistered;
	std::map<HIViewRef, AUGUI::oid_t> mOids;
	std::map<AUGUI::oid_t, HIViewRef> mViews;
};
#endif
// -----------------------------------------------------------------------------
// End of file.
// -----------------------------------------------------------------------------
