// =============================================================================
//	TNumber.cp
// =============================================================================
//

#include <AssertMacros.h>

#include "TNumber.h"

// -----------------------------------------------------------------------------
//	TNumber CFNumberRef constructor
// -----------------------------------------------------------------------------
//
TNumber::TNumber(
	CFNumberRef				inNumberRef,
	CFOwnership				inOwnership )
{
	// CFNumbers are immutable so a Copy is covered by retaining.
	if ( inOwnership == kCFOwnershipCopy )
		inOwnership = kCFOwnershipRetain;
	SetCFRef( inNumberRef, inOwnership );
}

// -----------------------------------------------------------------------------
//	TNumber constructor
// -----------------------------------------------------------------------------
//
TNumber::TNumber(
	SInt8					inValue )
{
	SetCFRef( ::CFNumberCreate( kCFAllocatorDefault, kCFNumberSInt8Type, &inValue ) );
}

// -----------------------------------------------------------------------------
//	TNumber constructor
// -----------------------------------------------------------------------------
//
TNumber::TNumber(
	SInt16					inValue )
{
	SetCFRef( ::CFNumberCreate( kCFAllocatorDefault, kCFNumberSInt16Type, &inValue ) );
}

// -----------------------------------------------------------------------------
//	TNumber constructor
// -----------------------------------------------------------------------------
//
TNumber::TNumber(
	SInt32					inValue )
{
	SetCFRef( ::CFNumberCreate( kCFAllocatorDefault, kCFNumberSInt32Type, &inValue ) );
}

// -----------------------------------------------------------------------------
//	TNumber constructor
// -----------------------------------------------------------------------------
//
TNumber::TNumber(
	SInt64					inValue )
{
	SetCFRef( ::CFNumberCreate( kCFAllocatorDefault, kCFNumberSInt64Type, &inValue ) );
}

// -----------------------------------------------------------------------------
//	TNumber constructor
// -----------------------------------------------------------------------------
//
TNumber::TNumber(
	float					inValue )
{
	SetCFRef( ::CFNumberCreate( kCFAllocatorDefault, kCFNumberFloatType, &inValue ) );
}

// -----------------------------------------------------------------------------
//	TNumber constructor
// -----------------------------------------------------------------------------
//
TNumber::TNumber(
	double					inValue )
{
	SetCFRef( ::CFNumberCreate( kCFAllocatorDefault, kCFNumberDoubleType, &inValue ) );
}

// -----------------------------------------------------------------------------
//	TNumber destructor
// -----------------------------------------------------------------------------
//
TNumber::~TNumber()
{
}
