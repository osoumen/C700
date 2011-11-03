// =============================================================================
//	TCFType.cp
// =============================================================================
//

#include <AssertMacros.h>

#include "TCFType.h"

// -----------------------------------------------------------------------------
//	TCFType constructor
// -----------------------------------------------------------------------------
//
TCFType::TCFType()
{
	fCFRef = NULL;
}

// -----------------------------------------------------------------------------
//	TCFType CFTypeRef constructor
// -----------------------------------------------------------------------------
//
TCFType::TCFType(
	CFTypeRef		inTypeRef,
	CFOwnership		inOwnership )
{
	SetCFRef( inTypeRef, inOwnership );
}

// -----------------------------------------------------------------------------
//	TCFType destructor
// -----------------------------------------------------------------------------
//
TCFType::~TCFType()
{
	if ( GetCFRef() != NULL )
		::CFRelease( GetCFRef() );
}

// -----------------------------------------------------------------------------
//	SetCFRef
// -----------------------------------------------------------------------------
//
void
TCFType::SetCFRef(
	CFTypeRef		inTypeRef,
	CFOwnership		inOwnership )
{
	// Release the existing ref
	if ( fCFRef != NULL )
		::CFRelease( fCFRef );

	switch ( inOwnership )
	{
		case kCFOwnershipCopy:
			assert( "Base CF types can't be copied!\n" );
			break;
		case kCFOwnershipTake:
			// Just copy the ref. This instance is now responsible of releasing it!
			fCFRef = inTypeRef;
			break;
		case kCFOwnershipRetain:
			if ( inTypeRef != NULL )
				fCFRef = ::CFRetain( inTypeRef );
			else
				fCFRef = inTypeRef;
			break;
		default:
			assert( "Invalid CFOwnership!" );
			break;
	}
}
