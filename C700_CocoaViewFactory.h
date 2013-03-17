//
//  C700_CocoaViewFactory.h
//  C700
//
//  Created by osoumen on 12/10/01.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <AudioUnit/AUCocoaUIView.h>
#import <AudioToolbox/AudioToolbox.h>

#if AU
#include "plugguieditor.h"
#else
#include "aeffguieditor.h"
#endif

#include "C700Edit.h"
#include "EfxAccess.h"

@interface C700_CocoaViewFactory :NSObject <AUCocoaUIBase>
{
}

- (NSString *) description;	// string description of the view

@end

@interface C700_CocoaView : NSView
{
	C700Edit		*editor;
	NSTimer			*timer;
	AudioUnit 		mAU;
	AUEventListenerRef	mEventListener;
	
	EfxAccess		*efxAcc;
}

- (C700_CocoaView *)initWithFrame:(NSRect)frameRect audioUnit:(AudioUnit)inAU;
- (void)setEditorFrame;
- (void)_addListeners;
- (void)_removeListeners;
- (void)_synchronizeUIWithParameterValues;

@end
