// =============================================================================
//	TURL.cp
// =============================================================================
//

#include <AssertMacros.h>

#include "TURL.h"

// -----------------------------------------------------------------------------
//	TURL constructor
// -----------------------------------------------------------------------------
//
TURL::TURL()
{
}

// -----------------------------------------------------------------------------
//	TURL CFURL constructor
// -----------------------------------------------------------------------------
//
TURL::TURL(
	CFURLRef				inURL,
	CFOwnership				inOwnership )
{
	// CFURLs are immutable so a Copy is covered by retaining.
	if ( inOwnership == kCFOwnershipCopy )
		inOwnership = kCFOwnershipRetain;
	SetCFRef( inURL, inOwnership );
}

// -----------------------------------------------------------------------------
//	TURL bytes constructor
// -----------------------------------------------------------------------------
//
TURL::TURL(
	const UInt8*			inBytes,
	CFIndex					inLength,
	CFStringEncoding		inEncoding,
	CFURLRef				inBaseURL )
{
	SetCFRef( ::CFURLCreateWithBytes( kCFAllocatorDefault, inBytes, inLength, inEncoding, inBaseURL ) );
}

// -----------------------------------------------------------------------------
//	TURL CFStringRef constructor
// -----------------------------------------------------------------------------
//
TURL::TURL(
	CFStringRef				inString,
	CFURLRef				inBaseURL )
{
	SetCFRef( ::CFURLCreateWithString( kCFAllocatorDefault, inString, inBaseURL ) );
}

// -----------------------------------------------------------------------------
//	TURL CFStringRef file system path constructor
// -----------------------------------------------------------------------------
//
TURL::TURL(
	CFStringRef				inPath,
	CFURLPathStyle			inPathStyle,
	Boolean					inIsDirectory,
	CFURLRef				inBaseURL )
{
	SetCFRef( ::CFURLCreateWithFileSystemPathRelativeToBase( kCFAllocatorDefault, inPath, inPathStyle, inIsDirectory, inBaseURL ) );
}

// -----------------------------------------------------------------------------
//	TURL file system rep constructor
// -----------------------------------------------------------------------------
//
TURL::TURL(
	const UInt8*			inBytes,
	CFIndex					inLength,
	Boolean					inIsDirectory,
	CFURLRef				inBaseURL )
{
	SetCFRef( ::CFURLCreateFromFileSystemRepresentation( kCFAllocatorDefault, inBytes, inLength,  inIsDirectory ) );
}

// -----------------------------------------------------------------------------
//	TURL FSRef constructor
// -----------------------------------------------------------------------------
//
TURL::TURL(
	const FSRef*			inFSRef )
{
	SetCFRef( ::CFURLCreateFromFSRef( kCFAllocatorDefault, inFSRef ) );
}

// -----------------------------------------------------------------------------
//	TURL destructor
// -----------------------------------------------------------------------------
//
TURL::~TURL()
{
}
