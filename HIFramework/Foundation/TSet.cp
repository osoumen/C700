// =============================================================================
//	TSet.cp
// =============================================================================
//

#include <AssertMacros.h>

#include "TSet.h"

// -----------------------------------------------------------------------------
//	TSet constructor
// -----------------------------------------------------------------------------
//
TSet::TSet()
	:	TCFType( NULL )
{
}

// -----------------------------------------------------------------------------
//	TSet CFSetRef constructor
// -----------------------------------------------------------------------------
//
TSet::TSet(
	CFSetRef				inSet,
	CFOwnership				inOwnership )
{
	if ( inOwnership == kCFOwnershipCopy )
		SetCFRef( ::CFSetCreateCopy( kCFAllocatorDefault, inSet ) );
	else
		SetCFRef( inSet, inOwnership );
}

// -----------------------------------------------------------------------------
//	TSet constructor
// -----------------------------------------------------------------------------
//
TSet::TSet(
	SetValue*					inValues,
	CFIndex						inCount,
	const CFSetCallBacks*		inCallBacks )
{
	SetCFRef( ::CFSetCreate( kCFAllocatorDefault, inValues, inCount, inCallBacks ) );
}

// -----------------------------------------------------------------------------
//	TSet destructor
// -----------------------------------------------------------------------------
//
TSet::~TSet()
{
}

// =============================================================================
//	MUTABLE
// =============================================================================

// -----------------------------------------------------------------------------
//	TMutableSet CFMutableSetRef constructor
// -----------------------------------------------------------------------------
//
TMutableSet::TMutableSet(
	CFSetRef				inSet,
	CFIndex					inCapacity,
	CFOwnership				inOwnership )
{
	if ( inOwnership == kCFOwnershipCopy )
		SetCFRef( ::CFSetCreateMutableCopy( kCFAllocatorDefault, inCapacity, inSet ) );
	else
		SetCFRef( inSet, inOwnership );
}

// -----------------------------------------------------------------------------
//	TMutableSet constructor
// -----------------------------------------------------------------------------
//
TMutableSet::TMutableSet(
	CFIndex					inCapacity,
	const CFSetCallBacks*	inCallBacks )
{
	SetCFRef( ::CFSetCreateMutable( kCFAllocatorDefault, inCapacity, inCallBacks ) );
}

// -----------------------------------------------------------------------------
//	TMutableSet destructor
// -----------------------------------------------------------------------------
//
TMutableSet::~TMutableSet()
{
}
