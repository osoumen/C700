/*
	File:		TObject.h

    Version:	1.1

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	Copyright © 2000-2005 Apple Computer, Inc., All Rights Reserved
*/

#ifndef TObject_H_
#define TObject_H_

#include <Carbon/Carbon.h>

#include "HIFramework.h"
#include "TCarbonEvent.h"
#include "TEventHandler.h"

class TObject
	: public TEventHandler
{
public:
	HIObjectRef				GetObjectRef()
								{ return fObjectRef; }

	void					PrintDebugInfo()
								{ return HIObjectPrintDebugInfo( fObjectRef ); }
								
	void					Retain()
								{ CFRetain( fObjectRef ); }
	void					Release()
								{ CFRelease( fObjectRef ); }

protected:
	// Types
	typedef OSStatus		(*ConstructProc)(
								HIObjectRef			inBaseObject,
								TObject**			outInstance );

	// Construction/Destruction
							TObject(
								HIObjectRef			inObject );
	virtual					~TObject();

	static OSStatus			RegisterSubclass(
								CFStringRef			inID,
								CFStringRef			inBaseID,
								ConstructProc		inProc );
	static EventRef			CreateInitializationEvent();

	virtual OSStatus		Initialize(
								TCarbonEvent&		inEvent );

	// Interfaces
	enum 	Interface		{
								kAccessibility		= 'acce',
								kCommands			= 'cmds'
							};
	virtual OSStatus		ActivateInterface(
								UInt32				inInterface );

	// Accessibility
	virtual OSStatus		CopyAccessibleChildAtPoint(
								EventHandlerCallRef	inCallRef,
								EventRef			inEvent,
								AXUIElementRef		inElement,
								UInt64				inIdentifier,
								const HIPoint&		inWhere,
								CFTypeRef*			outChild );
	virtual OSStatus		CopyAccessibleFocusedChild(
								EventHandlerCallRef	inCallRef,
								EventRef			inEvent,
								AXUIElementRef		inElement,
								UInt64				inIdentifier,
								CFTypeRef*			outChild );
	virtual OSStatus		GetAccessibleAttributeNames(
								EventHandlerCallRef	inCallRef,
								EventRef			inEvent,
								AXUIElementRef		inElement,
								UInt64				inIdentifier,
								CFMutableArrayRef	outNames );
	virtual OSStatus		GetAccessibleParameterizedAttributeNames(
								EventHandlerCallRef	inCallRef,
								EventRef			inEvent,
								AXUIElementRef		inElement,
								UInt64				inIdentifier,
								CFMutableArrayRef	outNames );
	virtual OSStatus		GetAccessibleNamedAttribute(
								EventHandlerCallRef	inCallRef,
								EventRef			inEvent,
								AXUIElementRef		inElement,
								UInt64				inIdentifier,
								CFStringRef			inAttribute );
	virtual OSStatus		SetAccessibleNamedAttribute(
								EventHandlerCallRef	inCallRef,
								EventRef			inEvent,
								AXUIElementRef		inElement,
								UInt64				inIdentifier,
								CFStringRef			inAttribute );
	virtual OSStatus		IsAccessibleNamedAttributeSettable(
								EventHandlerCallRef	inCallRef,
								EventRef			inEvent,
								AXUIElementRef		inElement,
								UInt64				inIdentifier,
								CFStringRef			inName,
								Boolean*			outIsSettable );
	virtual OSStatus		GetAccessibleActionNames(
								EventHandlerCallRef	inCallRef,
								EventRef			inEvent,
								AXUIElementRef		inElement,
								UInt64				inIdentifier,
								CFMutableArrayRef	outNames );
	virtual OSStatus		CopyAccessibleNamedActionDescription(
								EventHandlerCallRef	inCallRef,
								EventRef			inEvent,
								AXUIElementRef		inElement,
								UInt64				inIdentifier,
								CFStringRef			inName,
								CFStringRef*		outDescription );
	virtual OSStatus		PerformAccessibleNamedAction(
								EventHandlerCallRef	inCallRef,
								EventRef			inEvent,
								AXUIElementRef		inElement,
								UInt64				inIdentifier,
								CFStringRef			inName );

	// Debugging
	virtual void			PrintDebugInfoSelf();

	// Event Handling
	virtual OSStatus		HandleEvent(
								EventHandlerCallRef	inCallRef,
								TCarbonEvent&		inEvent );
								
	virtual OSStatus		UpdateCommandStatus(
								EventHandlerCallRef	inCallRef,
								HICommandExtended&	inCommand );
								
	virtual OSStatus		ProcessCommand(
								EventHandlerCallRef	inCallRef,
								HICommandExtended&	inCommand );

	EventTargetRef			GetEventTarget()
								{ return HIObjectGetEventTarget( fObjectRef ); }

protected:
	static OSStatus			ObjectEventHandler(
								EventHandlerCallRef	inCallRef,
								EventRef			inEvent,
								void*				inUserData );
private:
	HIObjectRef				fObjectRef;
};


#endif // TObject_H_
