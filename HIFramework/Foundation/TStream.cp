// =============================================================================
//	TStream.cp
// =============================================================================
//

#include <AssertMacros.h>

#include "TStream.h"

// -----------------------------------------------------------------------------
//	TReadStream constructor
// -----------------------------------------------------------------------------
//
TReadStream::TReadStream()
{
}

// -----------------------------------------------------------------------------
//	TReadStream bytes constructor
// -----------------------------------------------------------------------------
//
TReadStream::TReadStream(
	UInt8*					inBytes,
	CFIndex					inLength )
{
	SetCFRef( ::CFReadStreamCreateWithBytesNoCopy( kCFAllocatorDefault, inBytes, inLength, kCFAllocatorDefault ) );
}

// -----------------------------------------------------------------------------
//	TReadStream constructor
// -----------------------------------------------------------------------------
//
TReadStream::TReadStream(
	CFURLRef				inURL )
{
	SetCFRef( ::CFReadStreamCreateWithFile( kCFAllocatorDefault, inURL ) );
}

// -----------------------------------------------------------------------------
//	TReadStream destructor
// -----------------------------------------------------------------------------
//
TReadStream::~TReadStream()
{
}

// -----------------------------------------------------------------------------
//	TWriteStream bytes constructor
// -----------------------------------------------------------------------------
//
TWriteStream::TWriteStream(
	UInt8*					inBuffer,
	CFIndex					inLength )
{
	SetCFRef( ::CFWriteStreamCreateWithBuffer( kCFAllocatorDefault, inBuffer, inLength ) );
}

// -----------------------------------------------------------------------------
//	TWriteStream constructor
// -----------------------------------------------------------------------------
//
TWriteStream::TWriteStream(
	CFURLRef				inURL )
{
	SetCFRef( ::CFWriteStreamCreateWithFile( kCFAllocatorDefault, inURL ) );
}

// -----------------------------------------------------------------------------
//	TWriteStream destructor
// -----------------------------------------------------------------------------
//
TWriteStream::~TWriteStream()
{
}
