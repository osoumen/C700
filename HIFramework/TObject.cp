/*
    File:		TObject.cp
    
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

#include "TObject.h"
#include "TCarbonEvent.h"

//-----------------------------------------------------------------------------------
//	constants
//-----------------------------------------------------------------------------------
//
static const EventTypeSpec kHIObjectEvents[] =
{
	{ kEventClassHIObject, kEventHIObjectConstruct },
	{ kEventClassHIObject, kEventHIObjectInitialize },
	{ kEventClassHIObject, kEventHIObjectDestruct },
	{ kEventClassHIObject, kEventHIObjectPrintDebugInfo }
};

//-----------------------------------------------------------------------------------
//	TObject HIObjectRef constructor
//-----------------------------------------------------------------------------------
//
TObject::TObject(
	HIObjectRef			inObject )
{
	fObjectRef = inObject;
}

//-----------------------------------------------------------------------------------
//	TObject destructor
//-----------------------------------------------------------------------------------
//
TObject::~TObject()
{
}

//-----------------------------------------------------------------------------------
//	Initialize
//-----------------------------------------------------------------------------------
//	Perform any initialization work.
//
OSStatus
TObject::Initialize(
	TCarbonEvent&		inEvent )
{
	return noErr;
}

//-----------------------------------------------------------------------------------
//	PrintDebugInfoSelf
//-----------------------------------------------------------------------------------
//	Prints debugging info specific to this object. Subclasses should call the
//	inherited method at the end of handling this so that superclass debugging info
//	also gets printed out.
//
void
TObject::PrintDebugInfoSelf()
{
	fprintf( stdout, "TObject 0x%08X\n", (unsigned int) GetObjectRef() );
}


//-----------------------------------------------------------------------------------
//	RegisterSubclass
//-----------------------------------------------------------------------------------
//	This routine should be called by subclasses so they can be created as HIObjects.
//
OSStatus
TObject::RegisterSubclass(
	CFStringRef			inID,
	CFStringRef			inBaseID,
	ConstructProc		inProc )
{
	return HIObjectRegisterSubclass( inID, inBaseID, 0, ObjectEventHandler,
			GetEventTypeCount( kHIObjectEvents ), kHIObjectEvents, (void*) inProc, NULL );
}

//-----------------------------------------------------------------------------------
//	CreateInitializationEvent
//-----------------------------------------------------------------------------------
// 	Create a basic intialization event.
//
EventRef
TObject::CreateInitializationEvent()
{
	OSStatus		result = noErr;
	EventRef		event;

	result = CreateEvent( NULL, kEventClassHIObject, kEventHIObjectInitialize, 0, 0, &event );
	require_noerr_action( result, CantCreateEvent, event = NULL );
		
CantCreateEvent:
	return event;
}

//-----------------------------------------------------------------------------------
//	ObjectEventHandler
//-----------------------------------------------------------------------------------
//	Our static event handler proc. We handle any HIObject based events directly in
// 	here at present. This handler is only called by the non-Event system dispatch
//	of HIObject creation/destruction.
//
OSStatus
TObject::ObjectEventHandler(
	EventHandlerCallRef	inCallRef,
	EventRef			inEvent,
	void*				inUserData )
{
	OSStatus			result = eventNotHandledErr;
	TObject*			instance = (TObject*) inUserData;
	TCarbonEvent		event( inEvent );
	
	switch ( event.GetClass() )
	{
		case kEventClassHIObject:
		{
			//
			// Get the object from the event (will be present for all HIObject events except Initialize
			// on Tiger; for Panther and earlier, will not be present in PrintDebugInfo and Destruct).
			// In case this object is a delegate of another object and is receiving that object's
			// HIObject event, we will only act on the event if it is meant for us.
			//
			HIObjectRef	object = NULL;
			OSStatus	err = event.GetParameter<HIObjectRef>( kEventParamHIObjectInstance,
															   typeHIObjectRef, &object );
			
			switch ( event.GetKind() )
			{
				case kEventHIObjectConstruct:
				{
					require_noerr_action( err, ParameterMissing, result = err );
					
					// on entry for our construct event, we're passed the
					// creation proc we registered with for this class.
					// we use it now to create the instance, and then we
					// replace the instance parameter data with said instance
					// as type void.

					result = (*(ConstructProc)inUserData)( object, &instance );

					if ( result == noErr )
					{
						event.SetParameter<TObject*>( kEventParamHIObjectInstance,
								typeVoidPtr, instance ); 
					}
					break;
				}
				
				case kEventHIObjectInitialize:
					result = CallNextEventHandler( inCallRef, inEvent );
					if ( result == noErr )
						result = instance->Initialize( event );
					break;
				
				case kEventHIObjectPrintDebugInfo:
					if ( object == NULL || object == instance->GetObjectRef() )
						instance->PrintDebugInfoSelf();
					// don't set result, let it propagate
					break;

				case kEventHIObjectDestruct:
					if ( object == NULL || object == instance->GetObjectRef() )
						delete instance;
					// result is unimportant
					break;
			}
			break;
		}
	}

ParameterMissing:
	return result;
}

//-----------------------------------------------------------------------------------
//	HandleEvent
//-----------------------------------------------------------------------------------
//	Per-instance event handler. Override this method to handle additional events.
//	From the overridden method, be sure to call this superclass method.
//
OSStatus
TObject::HandleEvent(
	EventHandlerCallRef	inCallRef,
	TCarbonEvent&		inEvent )
{
	OSStatus			result = eventNotHandledErr;
	
	switch ( inEvent.GetClass() )
	{
		case kEventClassAccessibility:
		{
			AXUIElementRef		element;
			UInt64				identifier;

			result = inEvent.GetParameter<AXUIElementRef>( kEventParamAccessibleObject, typeCFTypeRef, &element );
			require_noerr( result, ParameterMissing );
			
			AXUIElementGetIdentifier( element, &identifier );

			switch ( inEvent.GetKind() )
			{
				case kEventAccessibleGetChildAtPoint:
				{
					CFTypeRef	child = NULL;
					HIPoint		where;

					result = inEvent.GetParameter<HIPoint>( kEventParamMouseLocation, typeHIPoint, &where );
					require_noerr( result, ParameterMissing );

					result = CopyAccessibleChildAtPoint( inCallRef, inEvent, element, identifier, where, &child );
					if ( result == noErr && child != NULL )
					{
						result = inEvent.SetParameter<CFTypeRef>( kEventParamAccessibleChild, typeCFTypeRef, child );
						CFRelease( child );
					}
					break;
				}

				case kEventAccessibleGetFocusedChild:
				{
					CFTypeRef	child = NULL;

					result = CopyAccessibleFocusedChild( inCallRef, inEvent, element, identifier, &child );
					if ( result == noErr && child != NULL )
					{
						result = inEvent.SetParameter<CFTypeRef>( kEventParamAccessibleChild, typeCFTypeRef, child );
						CFRelease( child );
					}
					break;
				}

				case kEventAccessibleGetAllAttributeNames:
				{
					CFMutableArrayRef	namesArray;

					result = inEvent.GetParameter<CFMutableArrayRef>( kEventParamAccessibleAttributeNames,
							typeCFMutableArrayRef, &namesArray );
					require_noerr( result, ParameterMissing );

					result = GetAccessibleAttributeNames( inCallRef, inEvent, element, identifier, namesArray );
					break;
				}
					
				case kEventAccessibleGetAllParameterizedAttributeNames:
				{
					CFMutableArrayRef	namesArray;

					result = inEvent.GetParameter<CFMutableArrayRef>( kEventParamAccessibleAttributeNames,
							typeCFMutableArrayRef, &namesArray );
					require_noerr( result, ParameterMissing );

					result = GetAccessibleParameterizedAttributeNames( inCallRef, inEvent, element, identifier, namesArray );
					break;
				}
					
				case kEventAccessibleGetNamedAttribute:
				{
					CFStringRef			attribute;

					result = inEvent.GetParameter<CFStringRef>( kEventParamAccessibleAttributeName,
						typeCFStringRef, &attribute );
					require_noerr( result, ParameterMissing );

					result = GetAccessibleNamedAttribute( inCallRef, inEvent, element, identifier, attribute );
					break;
				}
					
				case kEventAccessibleSetNamedAttribute:
				{
					CFStringRef			attribute;

					result = inEvent.GetParameter<CFStringRef>( kEventParamAccessibleAttributeName,
						typeCFStringRef, &attribute );
					require_noerr( result, ParameterMissing );

					result = SetAccessibleNamedAttribute( inCallRef, inEvent, element, identifier, attribute );
					break;
				}
					
				case kEventAccessibleIsNamedAttributeSettable:
				{
					CFStringRef			attribute;
					Boolean				isSettable;
					
					result = inEvent.GetParameter<CFStringRef>( kEventParamAccessibleAttributeName,
						typeCFStringRef, &attribute );
					require_noerr( result, ParameterMissing );

					result = IsAccessibleNamedAttributeSettable( inCallRef, inEvent, element, identifier, attribute, &isSettable );

					inEvent.SetParameter( kEventParamAccessibleAttributeSettable, isSettable );
					break;
				}
					
				case kEventAccessibleGetAllActionNames:
				{
					CFMutableArrayRef	array;

					result = inEvent.GetParameter<CFMutableArrayRef>( kEventParamAccessibleActionNames,
							typeCFMutableArrayRef, &array );
					require_noerr( result, ParameterMissing );

					result = GetAccessibleActionNames( inCallRef, inEvent, element, identifier, array );
					break;
				}
					
				case kEventAccessibleGetNamedActionDescription:
				{
					CFStringRef				action;
					CFMutableStringRef		desc;
					CFStringRef				selfDesc = NULL;
					
					result = inEvent.GetParameter<CFStringRef>( kEventParamAccessibleActionName,
						typeCFStringRef, &action );
					require_noerr( result, ParameterMissing );
					result = inEvent.GetParameter<CFMutableStringRef>( kEventParamAccessibleActionDescription,
						typeCFMutableStringRef, &desc );
					require_noerr( result, ParameterMissing );

					result = CopyAccessibleNamedActionDescription( inCallRef, inEvent, element, identifier, action, &selfDesc );

					CFStringReplaceAll( desc, selfDesc );
					CFRelease( selfDesc );
					break;
				}
				
				case kEventAccessiblePerformNamedAction:
				{
					CFStringRef		action;
					
					result = inEvent.GetParameter<CFStringRef>( kEventParamAccessibleActionName,
						typeCFStringRef, &action );
					require_noerr( result, ParameterMissing );

					result = PerformAccessibleNamedAction( inCallRef, inEvent, element, identifier, action );
					break;
				}
			}
			break;
		}
		
		case kEventClassCommand:
		{
			switch ( inEvent.GetKind() )
			{
				case kEventCommandUpdateStatus:
				{
					HICommandExtended command;
					inEvent.GetParameter( kEventParamDirectObject, typeHICommand, &command );
					result = UpdateCommandStatus( inCallRef, command );
					break;
				}
					
				case kEventCommandProcess:
				{
					HICommandExtended command;
					inEvent.GetParameter( kEventParamDirectObject, typeHICommand, &command );
					result = ProcessCommand( inCallRef, command );
					break;
				}
				
				default:
					break;
			}
			break;
		}
		
		default:
			break;
	}

ParameterMissing:
	return result;
}

//-----------------------------------------------------------------------------------
//	UpdateCommandStatus
//-----------------------------------------------------------------------------------
//	Per-instance command status update handler. Override this method to handle
//	additional commands.
//
OSStatus
TObject::UpdateCommandStatus(
	EventHandlerCallRef	inCallRef,
	HICommandExtended&	inCommand )
{
	return eventNotHandledErr;
}

//-----------------------------------------------------------------------------------
//	ProcessCommand
//-----------------------------------------------------------------------------------
//	Per-instance command handler. Override this method to handle additional commands.
//
OSStatus
TObject::ProcessCommand(
	EventHandlerCallRef	inCallRef,
	HICommandExtended&	inCommand )
{
	return eventNotHandledErr;
}

//-----------------------------------------------------------------------------------
//	ActivateInterface
//-----------------------------------------------------------------------------------
//	This routine is used to allow a subclass to turn on a specific event or suite of
//	events. This allows us to keep event traffic down if we are	not interested, but
//	register for the events if we are.
//
OSStatus
TObject::ActivateInterface(
	UInt32	inInterface )
{
	OSStatus		result = paramErr;
	
	switch( inInterface )
	{
		case kAccessibility:
		{
			static const EventTypeSpec kAccessibilityEvents[] =
			{
				{ kEventClassAccessibility, kEventAccessibleGetChildAtPoint },
				{ kEventClassAccessibility, kEventAccessibleGetFocusedChild },
				{ kEventClassAccessibility, kEventAccessibleGetAllAttributeNames },
				{ kEventClassAccessibility, kEventAccessibleGetAllParameterizedAttributeNames },
				{ kEventClassAccessibility, kEventAccessibleGetNamedAttribute },
				{ kEventClassAccessibility, kEventAccessibleSetNamedAttribute },
				{ kEventClassAccessibility, kEventAccessibleIsNamedAttributeSettable },
				{ kEventClassAccessibility, kEventAccessibleGetAllActionNames },
				{ kEventClassAccessibility, kEventAccessibleGetNamedActionDescription },
				{ kEventClassAccessibility, kEventAccessiblePerformNamedAction },
			};
			
			result = AddEventTypesToHandler( GetEventHandler(), GetEventTypeCount( kAccessibilityEvents ),
											 kAccessibilityEvents );
			break;
		}
		
		case kCommands:
		{
			static const EventTypeSpec kCommandEvents[] =
			{
				{ kEventClassCommand, kEventCommandUpdateStatus },
				{ kEventClassCommand, kEventCommandProcess }
			};
			
			result = AddEventTypesToHandler( GetEventHandler(), GetEventTypeCount( kCommandEvents ),
											 kCommandEvents );
			break;
		}
	}
	
	return result;
}

//-----------------------------------------------------------------------------------
//	CopyAccessibleChildAtPoint
//-----------------------------------------------------------------------------------
//	Read CarbonEvents.h in the kEventAccessibleGetChildAtPoint section to
//	see how to override this method.
//
OSStatus
TObject::CopyAccessibleChildAtPoint(
	EventHandlerCallRef	inCallRef,
	EventRef			inEvent,
	AXUIElementRef		inElement,
	UInt64				inIdentifier,
	const HIPoint&		inWhere,
	CFTypeRef*			outChild )
{
	return eventNotHandledErr;
}

//-----------------------------------------------------------------------------------
//	CopyAccessibleFocusedChild
//-----------------------------------------------------------------------------------
//	Read CarbonEvents.h in the kEventAccessibleGetFocusedChild section to
//	see how to override this method.
//
OSStatus
TObject::CopyAccessibleFocusedChild(
	EventHandlerCallRef	inCallRef,
	EventRef			inEvent,
	AXUIElementRef		inElement,
	UInt64				inIdentifier,
	CFTypeRef*			outChild )
{
	return eventNotHandledErr;
}

//-----------------------------------------------------------------------------------
//	GetAccessibleAttributeNames
//-----------------------------------------------------------------------------------
//	Read CarbonEvents.h in the kEventAccessibleGetAllAttributeNames section to
//	see how to override this method.
//
OSStatus
TObject::GetAccessibleAttributeNames(
	EventHandlerCallRef	inCallRef,
	EventRef			inEvent,
	AXUIElementRef		inElement,
	UInt64				inIdentifier,
	CFMutableArrayRef	outNames )
{
	return eventNotHandledErr;
}

//-----------------------------------------------------------------------------------
//	GetAccessibleParameterizedAttributeNames
//-----------------------------------------------------------------------------------
//	Read CarbonEvents.h in the kEventAccessibleGetAllParameterizedAttributeNames
//	section to see how to override this method.
//
OSStatus
TObject::GetAccessibleParameterizedAttributeNames(
	EventHandlerCallRef	inCallRef,
	EventRef			inEvent,
	AXUIElementRef		inElement,
	UInt64				inIdentifier,
	CFMutableArrayRef	outNames )
{
	return eventNotHandledErr;
}

//-----------------------------------------------------------------------------------
//	GetAccessibleNamedAttribute
//-----------------------------------------------------------------------------------
//	Read CarbonEvents.h in the kEventAccessibleGetNamedAttribute section to
//	see how to override this method.
//
OSStatus
TObject::GetAccessibleNamedAttribute(
	EventHandlerCallRef	inCallRef,
	EventRef			inEvent,
	AXUIElementRef		inElement,
	UInt64				inIdentifier,
	CFStringRef			inAttribute )
{
	return eventNotHandledErr;
}

//-----------------------------------------------------------------------------------
//	SetAccessibleNamedAttribute
//-----------------------------------------------------------------------------------
//	Read CarbonEvents.h in the kEventAccessibleSetNamedAttribute section to
//	see how to override this method.
//
OSStatus
TObject::SetAccessibleNamedAttribute(
	EventHandlerCallRef	inCallRef,
	EventRef			inEvent,
	AXUIElementRef		inElement,
	UInt64				inIdentifier,
	CFStringRef			inAttribute )
{
	return eventNotHandledErr;
}

//-----------------------------------------------------------------------------------
//	IsAccessibleNamedAttributeSettable
//-----------------------------------------------------------------------------------
//	Read CarbonEvents.h in the kEventAccessibleIsNamedAttributeSettable section to
//	see how to override this method.
//
OSStatus
TObject::IsAccessibleNamedAttributeSettable(
	EventHandlerCallRef	inCallRef,
	EventRef			inEvent,
	AXUIElementRef		inElement,
	UInt64				inIdentifier,
	CFStringRef			inName,
	Boolean*			outIsSettable )
{
	return eventNotHandledErr;
}

//-----------------------------------------------------------------------------------
//	GetAccessibleActionNames
//-----------------------------------------------------------------------------------
//	Read CarbonEvents.h in the kEventAccessibleGetAllActionNames section to
//	see how to override this method.
//
OSStatus
TObject::GetAccessibleActionNames(
	EventHandlerCallRef	inCallRef,
	EventRef			inEvent,
	AXUIElementRef		inElement,
	UInt64				inIdentifier,
	CFMutableArrayRef	outNames )
{
	return eventNotHandledErr;
}

//-----------------------------------------------------------------------------------
//	CopyAccessibleNamedActionDescription
//-----------------------------------------------------------------------------------
//	Read CarbonEvents.h in the kEventAccessibleGetNamedActionDescription section to
//	see how to override this method.
//
OSStatus
TObject::CopyAccessibleNamedActionDescription(
	EventHandlerCallRef	inCallRef,
	EventRef			inEvent,
	AXUIElementRef		inElement,
	UInt64				inIdentifier,
	CFStringRef			inName,
	CFStringRef*		outDescription )
{
	return eventNotHandledErr;
}

//-----------------------------------------------------------------------------------
//	PerformAccessibleNamedAction
//-----------------------------------------------------------------------------------
//	Read CarbonEvents.h in the kEventAccessiblePerformNamedAction section to
//	see how to override this method.
//
OSStatus
TObject::PerformAccessibleNamedAction(
	EventHandlerCallRef	inCallRef,
	EventRef			inEvent,
	AXUIElementRef		inElement,
	UInt64				inIdentifier,
	CFStringRef			inName )
{
	return eventNotHandledErr;
}
