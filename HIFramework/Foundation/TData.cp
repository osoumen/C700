// =============================================================================
//	TData.cp
// =============================================================================
//

#include <AssertMacros.h>

#include "TData.h"

// -----------------------------------------------------------------------------
//	TData constructor
// -----------------------------------------------------------------------------
//
TData::TData()
{
}

// -----------------------------------------------------------------------------
//	TData CFDataRef constructor
// -----------------------------------------------------------------------------
//
TData::TData(
	CFDataRef				inData,
	CFOwnership				inOwnership )
{
	if ( inOwnership == kCFOwnershipCopy )
		SetCFRef( ::CFDataCreateCopy( kCFAllocatorDefault, inData ) );
	else
		SetCFRef( inData, inOwnership );
}

// -----------------------------------------------------------------------------
//	TData bytes constructor
// -----------------------------------------------------------------------------
//
TData::TData(
	UInt8*					inBytes,
	CFIndex					inLength )
{
	SetCFRef( ::CFDataCreateMutable( kCFAllocatorDefault, 0 ) );
}

// -----------------------------------------------------------------------------
//	TData destructor
// -----------------------------------------------------------------------------
//
TData::~TData()
{
}

// =============================================================================
//	MUTABLE
// =============================================================================

// -----------------------------------------------------------------------------
//	TMutableData CFMutableDataRef constructor
// -----------------------------------------------------------------------------
//
TMutableData::TMutableData(
	CFDataRef				inData,
	CFIndex					inCapacity,
	CFOwnership				inOwnership )
{
	if ( inOwnership == kCFOwnershipCopy )
		SetCFRef( ::CFDataCreateMutableCopy( kCFAllocatorDefault, inCapacity, inData ) );
	else
		SetCFRef( inData, inOwnership );
}

// -----------------------------------------------------------------------------
//	TMutableData constructor
// -----------------------------------------------------------------------------
//
TMutableData::TMutableData(
	CFIndex					inCapacity )
{
	SetCFRef( ::CFDataCreateMutable( kCFAllocatorDefault, inCapacity ) );
}

// -----------------------------------------------------------------------------
//	TMutableData bytes constructor
// -----------------------------------------------------------------------------
//
TMutableData::TMutableData(
	UInt8*					inBytes,
	CFIndex					inLength,
	CFIndex					inCapacity )
{
	CFMutableDataRef		dataRef = ::CFDataCreateMutable( kCFAllocatorDefault, inCapacity );
	check( dataRef );
	SetCFRef( dataRef );
	AppendBytes( inBytes, inLength );
}

// -----------------------------------------------------------------------------
//	TMutableData destructor
// -----------------------------------------------------------------------------
//
TMutableData::~TMutableData()
{
}
