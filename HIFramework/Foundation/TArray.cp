// =============================================================================
//	TArray.cp
// =============================================================================
//

#include <AssertMacros.h>

#include "TArray.h"

// -----------------------------------------------------------------------------
//	TArray constructor
// -----------------------------------------------------------------------------
//
TArray::TArray()
{
}

// -----------------------------------------------------------------------------
//	TArray CFArray constructor
// -----------------------------------------------------------------------------
//
TArray::TArray(
	CFArrayRef					inArray,
	CFOwnership					inOwnership )
{
	if ( inOwnership == kCFOwnershipCopy )
		SetCFRef( ::CFArrayCreateCopy( kCFAllocatorDefault, inArray ) );
	else
		SetCFRef( inArray, inOwnership );
}

// -----------------------------------------------------------------------------
//	TArray constructor
// -----------------------------------------------------------------------------
//
TArray::TArray(
	ArrayValue*					inValues,
	CFIndex						inCount,
	const CFArrayCallBacks*		inCallBacks )
	:	TCFType()
{
	SetCFRef( ::CFArrayCreate( kCFAllocatorDefault, inValues, inCount, inCallBacks ) );
}

// -----------------------------------------------------------------------------
//	TArray destructor
// -----------------------------------------------------------------------------
//
TArray::~TArray()
{
}

// =============================================================================
//	TMutableArray
// =============================================================================

// -----------------------------------------------------------------------------
//	TMutableArray CFMutableArray constructor
// -----------------------------------------------------------------------------
//
TMutableArray::TMutableArray(
	CFMutableArrayRef		inArray,
	CFIndex					inCapacity,
	CFOwnership				inOwnership )
{
	if ( inOwnership == kCFOwnershipCopy )
		SetCFRef( ::CFArrayCreateMutableCopy( kCFAllocatorDefault, inCapacity, inArray ) );
	else
		SetCFRef( inArray, inOwnership );
}

// -----------------------------------------------------------------------------
//	TMutableArray constructor
// -----------------------------------------------------------------------------
//
TMutableArray::TMutableArray(
	CFIndex						inCapacity,
	const CFArrayCallBacks*		inCallBacks )
{
	SetCFRef( ::CFArrayCreateMutable( kCFAllocatorDefault, inCapacity, inCallBacks ) );
}

// -----------------------------------------------------------------------------
//	TArray constructor
// -----------------------------------------------------------------------------
//
TMutableArray::TMutableArray(
	ArrayValue*					inValues,
	CFIndex						inCount,
	CFIndex						inCapacity,
	const CFArrayCallBacks*		inCallBacks )
{
	SetCFRef( ::CFArrayCreateMutable( kCFAllocatorDefault, inCapacity, inCallBacks ) );
	::CFArrayReplaceValues( GetMutableArrayRef(), CFRangeMake( 0, 0 ), inValues, inCount );
}

// -----------------------------------------------------------------------------
//	TMutableArray destructor
// -----------------------------------------------------------------------------
//
TMutableArray::~TMutableArray()
{
}
