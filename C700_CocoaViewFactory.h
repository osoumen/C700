//
//  C700_CocoaViewFactory.h
//  Chip700
//
//  Created by “¡“c ‹§•F on 12/10/01.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <AudioUnit/AUCocoaUIView.h>

#if AU
#include "plugguieditor.h"
#else
#include "aeffguieditor.h"
#endif

@interface C700_CocoaViewFactory :NSObject <AUCocoaUIBase>
{
}

- (NSString *) description;	// string description of the view

@end

@interface C700_CocoaView : NSView
{
	AEffGUIEditor	*editor;
	NSTimer			*timer;
}

- (C700_CocoaView *)initWithFrame:(NSRect)frameRect audioUnit:(AudioUnit)inAU;
- (void)setEditorFrame;

@end
