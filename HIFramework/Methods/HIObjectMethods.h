// =============================================================================
//	HIObjectMethods.h
// =============================================================================
//

#ifndef HIObjectMethods_H_
#define HIObjectMethods_H_

#include <Carbon/Carbon.h>

struct OpaqueHIObjectRef
{
	inline HIObjectRef	GetHIObjectRef()
							{ return this; }

	CFStringRef			CopyClassID();
	OSStatus			CopyCustomArchiveData(
							CFDictionaryRef*	outCustomData );
	void*				DynamicCast(
							CFStringRef			inClassID );
	EventTargetRef		GetEventTarget();
	Boolean				IsArchivingIgnored();
	Boolean				IsAccessibilityIgnored();
	Boolean				IsOfClass(
							CFStringRef			inObjectClassID );
	void				OverrideAccessibilityContainment(
							AXUIElementRef		inDesiredParent,
							AXUIElementRef		inDesiredWindow,
							AXUIElementRef		inDesiredTopLevelUIElement );
	void				PrintDebugInfo();
	OSStatus			SetAccessibilityIgnored(
							Boolean				inSetIgnored );
	OSStatus			SetArchivingIgnored(
							Boolean				inSetIgnored );
	OSStatus			SetAuxiliaryAccessibilityAttribute(
							UInt64				inIdentifier,
							CFStringRef			inAttributeName,
							CFTypeRef			inAttributeData );
	OSStatus			SetCustomArchiveData(
							CFDictionaryRef		inCustomData );
};

// -----------------------------------------------------------------------------
//	CopyClassID
// -----------------------------------------------------------------------------
//
CFStringRef
OpaqueHIObjectRef::CopyClassID()
{
	return ::HIObjectCopyClassID( GetHIObjectRef() );
}

// -----------------------------------------------------------------------------
//	CopyCustomArchiveData
// -----------------------------------------------------------------------------
//
OSStatus
OpaqueHIObjectRef::CopyCustomArchiveData(
	CFDictionaryRef*	outCustomData )
{
	return ::HIObjectCopyCustomArchiveData( GetHIObjectRef(), outCustomData );
}

// -----------------------------------------------------------------------------
//	DynamicCast
// -----------------------------------------------------------------------------
//
void*
OpaqueHIObjectRef::DynamicCast(
	CFStringRef			inClassID )
{
	return ::HIObjectDynamicCast( GetHIObjectRef(), inClassID );
}

// -----------------------------------------------------------------------------
//	IsAccessibilityIgnored
// -----------------------------------------------------------------------------
//
Boolean
OpaqueHIObjectRef::IsAccessibilityIgnored()
{
	return ::HIObjectIsAccessibilityIgnored( GetHIObjectRef() );
}

// -----------------------------------------------------------------------------
//	IsArchivingIgnored
// -----------------------------------------------------------------------------
//
Boolean
OpaqueHIObjectRef::IsArchivingIgnored()
{
	return ::HIObjectIsArchivingIgnored( GetHIObjectRef() );
}

// -----------------------------------------------------------------------------
//	IsOfClass
// -----------------------------------------------------------------------------
//
Boolean
OpaqueHIObjectRef::IsOfClass(
	CFStringRef			inObjectClassID )
{
	return ::HIObjectIsOfClass( GetHIObjectRef(), inObjectClassID );
}

// -----------------------------------------------------------------------------
//	GetEventTarget
// -----------------------------------------------------------------------------
//
EventTargetRef
OpaqueHIObjectRef::GetEventTarget()
{
	return ::HIObjectGetEventTarget( GetHIObjectRef() );
}

// -----------------------------------------------------------------------------
//	PrintDebugInfo
// -----------------------------------------------------------------------------
//
void
OpaqueHIObjectRef::PrintDebugInfo()
{
	::HIObjectPrintDebugInfo( GetHIObjectRef() );
}

// -----------------------------------------------------------------------------
//	OverrideAccessibilityContainment
// -----------------------------------------------------------------------------
//
void
OpaqueHIObjectRef::OverrideAccessibilityContainment(
	AXUIElementRef		inDesiredParent,
	AXUIElementRef		inDesiredWindow,
	AXUIElementRef		inDesiredTopLevelUIElement )
{
	::HIObjectOverrideAccessibilityContainment( GetHIObjectRef(), inDesiredParent, inDesiredWindow, inDesiredTopLevelUIElement );
}

// -----------------------------------------------------------------------------
//	SetAccessibilityIgnored
// -----------------------------------------------------------------------------
//
OSStatus
OpaqueHIObjectRef::SetAccessibilityIgnored(
	Boolean				inSetIgnored )
{
	return ::HIObjectSetAccessibilityIgnored( GetHIObjectRef(), inSetIgnored );
}

// -----------------------------------------------------------------------------
//	SetArchivingIgnored
// -----------------------------------------------------------------------------
//
OSStatus
OpaqueHIObjectRef::SetArchivingIgnored(
	Boolean				inSetIgnored )
{
	return ::HIObjectSetArchivingIgnored( GetHIObjectRef(), inSetIgnored );
}

// -----------------------------------------------------------------------------
//	SetAuxiliaryAccessibilityAttribute
// -----------------------------------------------------------------------------
//
OSStatus
OpaqueHIObjectRef::SetAuxiliaryAccessibilityAttribute(
	UInt64				inIdentifier,
	CFStringRef			inAttributeName,
	CFTypeRef			inAttributeData )
{
	return ::HIObjectSetAuxiliaryAccessibilityAttribute( GetHIObjectRef(), inIdentifier, inAttributeName, inAttributeData );
}

// -----------------------------------------------------------------------------
//	SetCustomArchiveData
// -----------------------------------------------------------------------------
//
OSStatus
OpaqueHIObjectRef::SetCustomArchiveData(
	CFDictionaryRef			inCustomData )
{
	return ::HIObjectSetCustomArchiveData( GetHIObjectRef(), inCustomData );
}

#endif // HIObjectMethods_H_