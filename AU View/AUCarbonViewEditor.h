/*
 *  AUCarbonViewEditor.h
 *  Muso
 *
 *  Created by Airy ANDRE on 20/09/04.
 *  Copyright 2004 __MyCompanyName__. All rights reserved.
 *
 */

#include "TCarbonEvent.h"

class CAUCarbonViewNib;

class CAUCarbonViewEditor {
public:
    CAUCarbonViewEditor(CAUCarbonViewNib *view);
    virtual ~CAUCarbonViewEditor();
    
    bool HandleEvent(EventHandlerCallRef inHandlerRef, EventRef event);
    bool HandleEvent(EventRef event) { return HandleEvent(0, event); };
    void ShowHide(bool show);
    
    bool IsValid() const;

	void		UpdatePropertiesList();
protected:
    virtual void	WantEventTypes(EventTargetRef target, UInt32 inNumTypes, const EventTypeSpec *inList);
    virtual void	RemoveEventTypes(EventTargetRef target, UInt32 inNumTypes, const EventTypeSpec *inList);

    virtual OSStatus		SetUpTracking(TCarbonEvent& inEvent);
    virtual OSStatus		StillTracking(TCarbonEvent&inEvent);
    virtual OSStatus		StopTracking(TCarbonEvent&inEvent);
    
    virtual OSStatus		WindowContextual(TCarbonEvent&inEvent, EventHandlerCallRef inHandlerRef);
    virtual OSStatus		CommandProcess(TCarbonEvent&inEvent, EventHandlerCallRef inHandlerRef);
    virtual OSStatus		DrawControl(TCarbonEvent&inEvent, EventHandlerCallRef inHandlerRef);
    virtual OSStatus		ClickControl(TCarbonEvent&inEvent, EventHandlerCallRef inHandlerRef);
    
private:
        bool IsCompositing();
    
    /*! @var mHandlers */
    CFMutableDictionaryRef mHandlers;

    CAUCarbonViewNib *mAUView;
    HIViewRef mCurrentControl;
    WindowRef mInfos;
    MenuRef mMenu, mControlsMenu;

    HIPoint mFromPoint; // Current mouse pos for dragging

};