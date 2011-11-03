// =============================================================================
//	TDictionary.cp
// =============================================================================
//

#include <AssertMacros.h>

#include "TDictionary.h"

// -----------------------------------------------------------------------------
//	TDictionary constructor
// -----------------------------------------------------------------------------
//
TDictionary::TDictionary()
{
}

// -----------------------------------------------------------------------------
//	TDictionary CFDictionaryRef constructor
// -----------------------------------------------------------------------------
//
TDictionary::TDictionary(
	CFDictionaryRef						inDictionary,
	CFOwnership							inOwnership )
{
	if ( inOwnership == kCFOwnershipCopy )
		SetCFRef( ::CFDictionaryCreateCopy( kCFAllocatorDefault, inDictionary ) );
	else
		SetCFRef( inDictionary, inOwnership );
}

// -----------------------------------------------------------------------------
//	TDictionary constructor
// -----------------------------------------------------------------------------
//
TDictionary::TDictionary(
	DictionaryKey*						inKeys,
	DictionaryValue*					inValues,
	CFIndex								inCount,
	const CFDictionaryKeyCallBacks*		inKeyCallbacks,
	const CFDictionaryValueCallBacks*	inValueCallbacks )
{
	SetCFRef( ::CFDictionaryCreate( kCFAllocatorDefault, inKeys, inValues, inCount, inKeyCallbacks, inValueCallbacks ) );
}

// -----------------------------------------------------------------------------
//	TDictionary destructor
// -----------------------------------------------------------------------------
//
TDictionary::~TDictionary()
{
}

// =============================================================================
//	MUTABLE
// =============================================================================

// -----------------------------------------------------------------------------
//	TMutableDictionary CFMutableDictionaryRef constructor
// -----------------------------------------------------------------------------
//
TMutableDictionary::TMutableDictionary(
	CFDictionaryRef					inDictionary,
	CFIndex							inCapacity,
	CFOwnership						inOwnership )
{
	if ( inOwnership == kCFOwnershipCopy )
		SetCFRef( ::CFDictionaryCreateMutableCopy( kCFAllocatorDefault, inCapacity, inDictionary ) );
	else
		SetCFRef( inDictionary, inOwnership );
}

// -----------------------------------------------------------------------------
//	TMutableDictionary constructor
// -----------------------------------------------------------------------------
//
TMutableDictionary::TMutableDictionary(
	CFIndex								inCapacity,
	const CFDictionaryKeyCallBacks*		inKeyCallbacks,
	const CFDictionaryValueCallBacks*	inValueCallbacks )
{
	SetCFRef( ::CFDictionaryCreateMutable( kCFAllocatorDefault, inCapacity, inKeyCallbacks, inValueCallbacks ) );
}

// -----------------------------------------------------------------------------
//	TMutableDictionary destructor
// -----------------------------------------------------------------------------
//
TMutableDictionary::~TMutableDictionary()
{
}
