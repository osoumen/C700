// =============================================================================
//	TBag.cp
// =============================================================================
//

#include <AssertMacros.h>

#include "TBag.h"

// -----------------------------------------------------------------------------
//	TBag constructor
// -----------------------------------------------------------------------------
//
TBag::TBag()
{
}

// -----------------------------------------------------------------------------
//	TBag CFBagRef constructor
// -----------------------------------------------------------------------------
//
TBag::TBag(
	CFBagRef					inBag,
	CFOwnership					inOwnership )
{
	if ( inOwnership == kCFOwnershipCopy )
		SetCFRef( ::CFBagCreateCopy( kCFAllocatorDefault, inBag ) );
	else
		SetCFRef( inBag, inOwnership );
}

// -----------------------------------------------------------------------------
//	TBag constructor
// -----------------------------------------------------------------------------
//
TBag::TBag(
	BagValue*					inValues,
	CFIndex						inCount,
	const CFBagCallBacks*		inCallBacks )
{
	SetCFRef( ::CFBagCreateMutable( kCFAllocatorDefault, 0, inCallBacks ) );
}

// -----------------------------------------------------------------------------
//	TBag destructor
// -----------------------------------------------------------------------------
//
TBag::~TBag()
{
}

// =============================================================================
//	MUTABLE
// =============================================================================

// -----------------------------------------------------------------------------
//	TMutableBag CFMutableBagRef constructor
// -----------------------------------------------------------------------------
//
TMutableBag::TMutableBag(
	CFMutableBagRef				inBag,
	CFIndex						inCapacity,
	CFOwnership					inOwnership )
{
	if ( inOwnership == kCFOwnershipCopy )
		SetCFRef( ::CFBagCreateMutableCopy( kCFAllocatorDefault, inCapacity, inBag ) );
	else
		SetCFRef( inBag, inOwnership );
}

// -----------------------------------------------------------------------------
//	TMutableBag constructor
// -----------------------------------------------------------------------------
//
TMutableBag::TMutableBag(
	CFIndex						inCapacity,
	const CFBagCallBacks*		inCallBacks )
{
	SetCFRef( ::CFBagCreateMutable( kCFAllocatorDefault, inCapacity, inCallBacks ) );
}

// -----------------------------------------------------------------------------
//	TMutableBag destructor
// -----------------------------------------------------------------------------
//
TMutableBag::~TMutableBag()
{
}
