/*
 *  TViewNoCompositingCompatible.h
 *  NeuSynth
 *
 *  Created by Airy André on Wed Nov 06 2002.
 *  Copyright (c) 2002 Airy André. All rights reserved.
 *
 */

#ifndef TViewNoCompositingCompatible_H_
#define TViewNoCompositingCompatible_H_

#include "TView.h"
#include "CACFDictionary.h"
#include "AUProperty.h"

// Using modifier keys with controls is supported by default.
#if !defined(SUPPORT_CONTROL_MODIFIERS)
#define SUPPORT_CONTROL_MODIFIERS 1
#endif

// Definitions of which actual modifiers to use.
#if !defined(HIGH_PRECISION_MODIFIER)
#define HIGH_PRECISION_MODIFIER optionKey
#endif
#if !defined(HIGHEST_PRECISION_MODIFIER)
#define HIGHEST_PRECISION_MODIFIER controlKey
#endif

/*!
* \class TViewNoCompositingCompatible
 * \brief Base class for all the controls of the SDK
 *
 * The custom controls classes you to use non standard controls for your GUI, like knobs,
 * customs sliders or others.
 *
 * One of the easiest way for developing custom controls, is to use Apple's HIFramework,
 * available on Apple sample code site.
 *
 * Unfortunately, it not compatible for controls embedded in windows with compositing
 * flag.
 *
 * In order to solve this problem, the SDK is offering a \c TViewNoCompositingCompatible
 * class.
 *
 * Just inherit from this class instead of TView, and replace your \c Draw( RgnHandle
																			* inLimitRgn, CGContextRef inContext) method by \c CompatibleDraw( RgnHandle  * inLimitRgn,
																																			   * CGContextRef inContext,bool  inCompositing ). The additional "inCompositing" flag let
 * your control knows the kind of windows it does belongs to, if it does need it. The
 * TViewNoCompositingCompatible class automatically build a context and release it if it
 * is needed.
 *
 * If you control needs mouse tracking (slider, knob...), instead of overriding \c Track
 * and using \c TrackMouseLocation, you should override \c StillTracking, which is called each
 * time the mouse is dragged, in a non-blocking way. That makes your host happier,
 * since it still run while you are tracking. You can add additionnal code in
 * \c StartTracking, called at the beginning of the tracking, and \c StopTracking, called at
 * the end.
 *
 * All custom control classes should provide a  \c GetControlName() method that return the
 * unique ID of the class. And a non static GetType method that just return GetControlName
 *
 * For example, if you choose to define a "slider" class, you could define :
 * \c static \c  CFStringRef mySliderClass::GetControlName() { return CFSTR("myslider");};
 * \c virtual \c  CFStringRef mySliderClass::GetType() { return GetControlName();};
 * 
 * To register your class, the \c RegisterClassForBundleID template method should be called
 * in your \c InitWindows GUI class :
 * \c TViewNoCompositingCompatible::RegisterClassForBundleID<mySliderClass>(mBundleID);
 * 
 * And you should set the classID of your sliders in you .nib file to "your_AU_bundle_id.myslider"
 */

// All control class names should call this macro with some unique name
#define AUGUIDefineControl(classname, parent, name) \
	friend class TViewNoCompositingCompatible; \
protected: \
	static  const AUGUI::property_t mPropertiesList[];\
	static  const size_t mPropertiesListSize;\
public: \
	static  CFStringRef GetControlName() { return CFSTR(name);}; \
	virtual	CFStringRef GetType() { return GetControlName(); }; \
	virtual const AUGUI::property_t& GetPropertyDefinition(unsigned int i) const \
	{ \
		if (i < parent::GetPropertiesListSize())\
			return parent::GetPropertyDefinition(i);\
		else\
			return mPropertiesList[i-parent::GetPropertiesListSize()]; \
	}\
	virtual size_t GetPropertiesListSize() const { return parent::GetPropertiesListSize() + mPropertiesListSize; }

#define AUGUIProperties(CLASS) const AUGUI::property_t CLASS::mPropertiesList[]
#define AUGUIInit(CLASS) const size_t CLASS::mPropertiesListSize = sizeof(mPropertiesList)/sizeof(AUGUI::property_t)-1

class TViewNoCompositingCompatible
: public TView
{
protected:
	static  const AUGUI::property_t mPropertiesList[];
	static  const size_t mPropertiesListSize;
public:    
    typedef enum {
        kPropertyTypeInt = 0,
        kPropertyTypeDouble,
        kPropertyTypeString
    } tPropertyType;
    
	static const OSType theViewTag = 'TVew';
	
    static OSStatus			RegisterSubclass(CFStringRef inID, ConstructProc inProc );
    static OSStatus			UnRegisterSubclass( CFStringRef inID);
	
    TViewNoCompositingCompatible( HIViewRef inControl ); 
    virtual ~TViewNoCompositingCompatible();
	
	virtual	CFStringRef GetType() = 0;

	void ForceRedraw();
	void ForceValueChange();
	
	static void SetClassBundle(CFBundleRef sBundle);
	
    virtual void		FlushStaticBackground();
	
	virtual OSStatus SetFocusPart(ControlPartCode inDesiredFocus, Boolean inFocusEverything, ControlPartCode* outActualFocus);
    virtual void		Draw(RgnHandle	inLimitRgn, CGContextRef inContext );
    virtual void		DrawStaticBackground(RgnHandle	inLimitRgn, CGContextRef inContext, bool  inCompositing ) {};
    virtual void		CompatibleDraw(RgnHandle inLimitRgn, CGContextRef inContext, bool  inCompositing ) = 0;
	
	//! This method will be called by \c Track() if a subclass has overridden
	//! \c UseNonblockingTracking() to return false. Subclasses that do not call
	//! this will inherit the default tracking behaviour. Handlers for the MouseUp
	//! and MouseDragged events are installed. The control subclass must
	//! then override \c StillTracking instead of \c Track in order to get
	//! this behavior.
	//! \sa UseDefaultTracking()
    virtual OSStatus		SetUpTracking(TCarbonEvent& inEvent);
	
    virtual OSStatus		StartTracking(TCarbonEvent&inEvent, HIPoint& from);
    virtual OSStatus		StillTracking(TCarbonEvent&inEvent, HIPoint& from);
    virtual OSStatus		StopTracking(TCarbonEvent&inEvent, HIPoint& from);
	
	//! Will automatically call \c SetUpTracking() if the result of \c
	//! UseNonblockingTracking() is true. If false, this method will return
	//! the eventNotHandledErr error code to invoke default mouse tracking.
    virtual OSStatus		Track(TCarbonEvent& inEvent, ControlPartCode* outPartHit);
	
	virtual OSStatus GetData(OSType inTag, ControlPartCode inPart, Size inSize, Size* outSize, void* inPtr);
	virtual ControlPartCode	HitTest(
									const HIPoint&		inWhere );
	virtual OSStatus		ControlHit(
									   ControlPartCode		inPart,
									   UInt32				inModifiers );
	
	/*! @method GetBehaviors */
	virtual UInt32 GetBehaviors() { return (TView::GetBehaviors() | kControlSupportsEmbedding); };
	
	//! To enable the non-blocking tracking a subclass must override this
	//! method to return true.
    virtual bool UseNonblockingTracking() { return false; }
	
	//! See AUGUI::CopyClassForBundleID()
	template <class V> static CFStringRef CopyClassIDForBundleID(CFStringRef bundleID);
	//! See AUGUI::CopyCanonicalClassID()
	template <class V> static CFStringRef CopyCanonicalClassID();
    
	//! Register the given class.
    template <class V>  static void RegisterClassForBundleID(CFStringRef str);
	
    // The following is only needed if you need to unregister a single class
    // UnRegisterAllClasses is usually used.
    template <class V>  static void UnRegisterClassForBundleID(CFStringRef str);
	
	static OSStatus TViewNoCompositingCompatible::NewView(
														  HIViewRef*		outControl,
														  const HIRect*		inBounds,
														  HIViewRef			inView,
														  CFStringRef 	classID);

	static OSStatus TViewNoCompositingCompatible::NewViewForCurrentBundle(
														  HIViewRef*		outControl,
														  const HIRect*		inBounds,
														  HIViewRef			inView,
														  CFStringRef 	className);
	
    template <class V>  static OSStatus Construct(
                                                  HIObjectRef		inBaseObject,
                                                  TObject**			outInstance );
	
    template <class V>  static OSStatus CreateInView(
													 HIViewRef*			outControl,
													 const HIRect*		inBounds,
													 HIViewRef			inRoot,
													 CFStringRef 	classID);
	
    template <class V>  static OSStatus Create(
											   HIViewRef*			outControl,
											   const HIRect*		inBounds,
											   WindowRef			inWindow,
											   CFStringRef 	classID);
	
    template <class V>  static TViewNoCompositingCompatible* CreateForCurrentBundle (
											   const HIRect*		inBounds,
											   HIViewRef			inRoot);
	
    static void UnRegisterAllClasses();
	
    // Must be false while the window is building
    static void SetWindowReady(bool ready) { sWindowReady = ready; };
	
	virtual void DrawBackgroundImage(CGContextRef context);
	
	// utilities
	bool IsCompositing();
	void ConvertToGlobal(HIPoint& thePoint);
	void ConvertToLocal(HIPoint& thePoint);
	
	virtual void WriteToDictionary(CACFDictionary &dict);
	virtual void ReadFromDictionary(CACFDictionary &dict);

	// Control properties management
	virtual void SetProperty(OSType propID, double value);
	virtual void SetProperty(OSType propID, int32_t value);
	virtual void SetProperty(OSType propID, CFStringRef value);
	virtual void SetProperty(OSType propID, AUGUI::font_t &value);
	virtual void SetProperty(OSType propID, AUGUI::rect_t &value);
	virtual void SetProperty(OSType propID, AUGUI::color_t &value);
	virtual void SetProperty(OSType propID, AUGUI::point_t &value);
	virtual bool GetProperty(OSType propID, double &value);
	virtual bool GetProperty(OSType propID, int32_t &value);
	virtual bool GetProperty(OSType propID, CFStringRef &value);
	virtual bool GetProperty(OSType propID, AUGUI::font_t &value);
	virtual bool GetProperty(OSType propID, AUGUI::rect_t &value);
	virtual bool GetProperty(OSType propID, AUGUI::color_t &value);
	virtual bool GetProperty(OSType propID, AUGUI::point_t &value);
	
	virtual void PropertiesChanged() {};

	virtual const AUGUI::property_t& GetPropertyDefinition(unsigned int i) const { return mPropertiesList[i];}
	virtual size_t GetPropertiesListSize() const { return mSaveMinMaxVal?mPropertiesListSize:(mPropertiesListSize-3); }

	void SaveMinMaxVal(bool save) { mSaveMinMaxVal = save; }
	// We have to redefine Initialize and ObjectEventHandler because that are "private" in TView and we need them for
	// the unregistering stuff... Too bad
protected:
	
	virtual OSStatus		mouseDragged(TCarbonEvent&inEvent);
    virtual OSStatus		mouseUp(TCarbonEvent&inEvent);
	void PrerenderStaticBackground();
	CFBundleRef mBundleRef;
	
	int mCurrentFocusPart;	
private:
	HIPoint mFromPoint;
    EventHandlerRef      mMouseEventHandler;
	
	CGImageRef mStaticBackground; // The composited background
	
	bool	mSaveMinMaxVal;
	
    static CFBundleRef      sStaticBundle;
    
    static bool sWindowReady; 
	
    static pascal OSStatus MouseEventHandler(
											 EventHandlerCallRef	inCallRef,
											 EventRef			inEvent,
											 void*				inUserData );
	
    static CFMutableDictionaryRef mRegisteryCache;
};
typedef TViewNoCompositingCompatible*				TViewNoCompositingCompatiblePtr;

template <class V>
CFStringRef TViewNoCompositingCompatible::CopyCanonicalClassID()
{
    return TViewNoCompositingCompatible::CopyClassIDForBundleID<V>(CFBundleGetIdentifier(V::sStaticBundle));
}

//! Use this method to create a string for the class ID of the template argument
//! using the bundle ID passed in \a bundleID.
template <class V>
CFStringRef TViewNoCompositingCompatible::CopyClassIDForBundleID(CFStringRef bundleID)
{
    return CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%@.%@"), bundleID, V::GetControlName());
}

template <class V>
void TViewNoCompositingCompatible::RegisterClassForBundleID(CFStringRef str)
{
    CFStringRef reg = CopyClassIDForBundleID<V>(str);
    RegisterSubclass( reg, (ConstructProc)Construct<V>);
    CFRelease(reg);
}

// The following is only needed if you need to unregister a single class
// UnRegisterAllClasses is usually used.
template <class V>
void TViewNoCompositingCompatible::UnRegisterClassForBundleID(CFStringRef str)
{
    CFStringRef reg = CopyClassIDForBundleID<V>(str);
    UnRegisterSubclass( reg);
    CFRelease(reg);
}

template <class V>
OSStatus TViewNoCompositingCompatible::Construct(
                                                 HIObjectRef		inBaseObject,
                                                 TObject**			outInstance )
{
    *outInstance = new V( (HIViewRef)inBaseObject );
    return noErr;
}


template <class V>
OSStatus TViewNoCompositingCompatible::CreateInView(
											  HIViewRef*		outControl,
											  const HIRect*		inBounds,
											  HIViewRef			inView,
											  CFStringRef 	classID)
{
    RegisterSubclass(classID, TViewNoCompositingCompatible::Construct<V>);
    return NewView( outControl, inBounds, inView, classID );
}

template <class V>
OSStatus TViewNoCompositingCompatible::Create(
											  HIViewRef*			outControl,
											  const HIRect*		inBounds,
											  WindowRef			inWindow,
											  CFStringRef 	classID)
{
	HIViewRef root = 0;
    if ( inWindow != NULL )
        GetRootControl( inWindow, &root );
	return CreateInView<V>(outControl, inBounds, root, classID);
}

template <class V> 
TViewNoCompositingCompatible* TViewNoCompositingCompatible::CreateForCurrentBundle (
														const HIRect*		inBounds,
														HIViewRef			root)
{
	HIViewRef			ctrl = 0;
	TViewNoCompositingCompatible *tview = 0;
    CFStringRef		   classID = TViewNoCompositingCompatible::CopyCanonicalClassID<V>();
	TViewNoCompositingCompatible::CreateInView<V>(&ctrl, inBounds, root, classID);
	CFRelease(classID);
    if (ctrl)
		GetControlData(ctrl, 0, TViewNoCompositingCompatible::theViewTag, sizeof(tview), &tview, 0);
	return tview;
}

namespace AUGUI {
    //@{
    //! These 3 functions are inline duplicates of the members of TViewNoCompositingCompatible.
    //! They are here because they are extremely useful and pulling them out of the
    //! class makes them a little easier to use (much less to type).
    //!
	template <class V>  
	inline TViewNoCompositingCompatible* CreateForCurrentBundle (const HIRect* inBounds, HIViewRef root)
	{
		return TViewNoCompositingCompatible::CreateForCurrentBundle<V>(inBounds, root);
	}
	
	
    //! CopyCanonicalClassID() will use the bundle stored as a static member of the class
    //! that you pass as the template argument.
    template <class V>
    inline CFStringRef CopyCanonicalClassID()
    {
        return TViewNoCompositingCompatible::CopyCanonicalClassID<V>();
    }
    
    template <class V>
    inline CFStringRef CopyClassIDForBundleID(CFStringRef bundleID)
    {
        return TViewNoCompositingCompatible::CopyClassIDForBundleID<V>(bundleID);
    }
    //@}

    template <class V>
    inline void AddPropertyInfo(uint32_t tag, std::string name, std::string label, property_type_t type)
    {
        V::mPropertiesList[tag] = property_t(tag, name, label, type);
    }
    
} // end namespace


#endif
