// =============================================================================
//	TString.cp
// =============================================================================
//

#include <AssertMacros.h>

#include "TString.h"

// -----------------------------------------------------------------------------
//	TString constructor
// -----------------------------------------------------------------------------
//
TString::TString()
{
}

// -----------------------------------------------------------------------------
//	TString Pascal string constructor
// -----------------------------------------------------------------------------
//
TString::TString(
	ConstStr255Param		inPStr,
	CFStringEncoding		inEncoding )
{
	SetCFRef( ::CFStringCreateWithPascalString( kCFAllocatorDefault, inPStr, inEncoding ) );
}

// -----------------------------------------------------------------------------
//	TString C string constructor
// -----------------------------------------------------------------------------
//
TString::TString(
	const char*				inCStr,
	CFStringEncoding		inEncoding )
{
	SetCFRef( ::CFStringCreateWithCString( kCFAllocatorDefault, inCStr, inEncoding ) );
}

// -----------------------------------------------------------------------------
//	TString UniChar string constructor
// -----------------------------------------------------------------------------
//
TString::TString(
	const UniChar*			inUStr,
	CFIndex					inCount )
{
	SetCFRef( ::CFStringCreateWithCharacters( kCFAllocatorDefault, inUStr, inCount ) );
}

// -----------------------------------------------------------------------------
//	TString bytes constructor
// -----------------------------------------------------------------------------
//
TString::TString(
	const UInt8*			inBytes,
	CFIndex					inCount,
	CFStringEncoding		inEncoding,
	Boolean					inExternalRepresentation )
{
	SetCFRef( ::CFStringCreateWithBytes( kCFAllocatorDefault, inBytes, inCount, inEncoding, inExternalRepresentation ) );
}

// -----------------------------------------------------------------------------
//	TString CFData constructor
// -----------------------------------------------------------------------------
//
TString::TString(
	CFDataRef				inData,
	CFStringEncoding		inEncoding )
{
	SetCFRef( ::CFStringCreateFromExternalRepresentation( kCFAllocatorDefault, inData, inEncoding ) );
}

// -----------------------------------------------------------------------------
//	TString CFStringRef constructor
// -----------------------------------------------------------------------------
//
TString::TString(
	CFStringRef				inString,
	CFOwnership				inOwnership )
{
	if ( inOwnership == kCFOwnershipCopy )
		SetCFRef( ::CFStringCreateCopy( kCFAllocatorDefault, inString ) );
	else
		SetCFRef( inString, inOwnership );
}

// -----------------------------------------------------------------------------
//	TString CFStringRef with format and arguments constructor
// -----------------------------------------------------------------------------
//
TString::TString(
	CFDictionaryRef			inOptions,
	CFStringRef				inFormat,
	... )
{
	va_list					args;
	va_start( args, inFormat );

	SetCFRef( ::CFStringCreateWithFormatAndArguments( kCFAllocatorDefault, inOptions, inFormat, args ) );

	va_end( args );
}

// -----------------------------------------------------------------------------
//	TString substring from CFStringRef
// -----------------------------------------------------------------------------
//
TString::TString(
	CFStringRef			inString,
	CFRange				inRange )
{
	SetCFRef( ::CFStringCreateWithSubstring( kCFAllocatorDefault, inString, inRange ), kCFOwnershipTake );
}

// -----------------------------------------------------------------------------
//	TString destructor
// -----------------------------------------------------------------------------
//
TString::~TString()
{
}

// -----------------------------------------------------------------------------
//	Compare
// -----------------------------------------------------------------------------
//
CFComparisonResult
TString::Compare(
	CFStringRef				inCompareTo,
	CFOptionFlags			inOptions ) const
{
	return ::CFStringCompare( GetStringRef(), inCompareTo, inOptions );
}

// -----------------------------------------------------------------------------
//	Find
// -----------------------------------------------------------------------------
//
CFRange
TString::Find(
	CFStringRef				inStringToFind,
	CFOptionFlags			inOptions ) const
{
	return ::CFStringFind( GetStringRef(), inStringToFind, inOptions );
}

// -----------------------------------------------------------------------------
//	GetCharacter
// -----------------------------------------------------------------------------
//
UniChar
TString::GetCharacter(
	CFIndex					inIndex ) const
{
	return ::CFStringGetCharacterAtIndex( GetStringRef(), inIndex );
}

// -----------------------------------------------------------------------------
//	GetCharacters
// -----------------------------------------------------------------------------
//
void
TString::GetCharacters(
	CFIndex					inIndex,
	CFIndex					inCount,
	UniChar*				outBuffer ) const
{
	::CFStringGetCharacters( GetStringRef(), CFRangeMake( inIndex, inCount ), outBuffer );
}

// -----------------------------------------------------------------------------
//	GetDoubleValue
// -----------------------------------------------------------------------------
//
double
TString::GetDoubleValue() const
{
	return ::CFStringGetDoubleValue( GetStringRef() );
}

// -----------------------------------------------------------------------------
//	GetIntValue
// -----------------------------------------------------------------------------
//
SInt32
TString::GetIntValue() const
{
	return ::CFStringGetIntValue( GetStringRef() );
}

// -----------------------------------------------------------------------------
//	GetLength
// -----------------------------------------------------------------------------
//
CFIndex
TString::GetLength() const
{
	return ::CFStringGetLength( GetStringRef() );
}

// -----------------------------------------------------------------------------
//	GetString Pascal string
// -----------------------------------------------------------------------------
//
Boolean
TString::GetString(
	StringPtr				outBuffer,
	CFIndex					inCount,
	CFStringEncoding		inEncoding ) const
{
	return ::CFStringGetPascalString( GetStringRef(), outBuffer, inCount, inEncoding );
}

// -----------------------------------------------------------------------------
//	GetString C string
// -----------------------------------------------------------------------------
//
Boolean
TString::GetString(
	char*					outBuffer,
	CFIndex					inCount,
	CFStringEncoding		inEncoding ) const
{
	return ::CFStringGetCString( GetStringRef(), outBuffer, inCount, inEncoding );
}

// -----------------------------------------------------------------------------
//	HasPrefix
// -----------------------------------------------------------------------------
//
Boolean
TString::HasPrefix(
	CFStringRef				inPrefix ) const
{
	return ::CFStringHasPrefix( GetStringRef(), inPrefix );
}

// -----------------------------------------------------------------------------
//	HasSuffix
// -----------------------------------------------------------------------------
//
Boolean
TString::HasSuffix(
	CFStringRef				inSuffix ) const
{
	return ::CFStringHasSuffix( GetStringRef(), inSuffix );
}

// =============================================================================
//	MUTABLE
// =============================================================================

// -----------------------------------------------------------------------------
//	TMutableString constructor
// -----------------------------------------------------------------------------
//
TMutableString::TMutableString(
	CFIndex					inMaxLength )
{
	SetCFRef( ::CFStringCreateMutable( kCFAllocatorDefault, inMaxLength ) );
}

// -----------------------------------------------------------------------------
//	TMutableString Pascal string constructor
// -----------------------------------------------------------------------------
//
TMutableString::TMutableString(
	ConstStr255Param		inPStr,
	CFIndex					inMaxLength,
	CFStringEncoding		inEncoding )
{
	SetCFRef( ::CFStringCreateMutable( kCFAllocatorDefault, inMaxLength ) );
	Append( inPStr, inEncoding );
}

// -----------------------------------------------------------------------------
//	TMutableString C string constructor
// -----------------------------------------------------------------------------
//
TMutableString::TMutableString(
	const char*				inCStr,
	CFIndex					inMaxLength,
	CFStringEncoding		inEncoding )
{
	SetCFRef( ::CFStringCreateMutable( kCFAllocatorDefault, inMaxLength ) );
	Append( inCStr, inEncoding );
}

// -----------------------------------------------------------------------------
//	TMutableString UniChar string constructor
// -----------------------------------------------------------------------------
//
TMutableString::TMutableString(
	const UniChar*			inUStr,
	CFIndex					inCount,
	CFIndex					inMaxLength )
{
	SetCFRef( ::CFStringCreateMutable( kCFAllocatorDefault, inMaxLength ) );
	Append( inUStr, inCount );
}

// -----------------------------------------------------------------------------
//	TMutableString bytes constructor
// -----------------------------------------------------------------------------
//
TMutableString::TMutableString(
	const UInt8*			inBytes,
	CFIndex					inCount,
	CFIndex					inMaxLength,
	CFStringEncoding		inEncoding,
	Boolean					inExternalRepresentation )
{
	TString	temp( inBytes, inCount, inEncoding, inExternalRepresentation );
	SetCFRef( ::CFStringCreateMutableCopy( kCFAllocatorDefault, inMaxLength, temp ) );
}

// -----------------------------------------------------------------------------
//	TMutableString CFData constructor
// -----------------------------------------------------------------------------
//
TMutableString::TMutableString(
	CFDataRef				inData,
	CFIndex					inMaxLength,
	CFStringEncoding		inEncoding )
{
	TString	temp( inData, inEncoding );
	SetCFRef( ::CFStringCreateMutableCopy( kCFAllocatorDefault, inMaxLength, temp ) );
}

// -----------------------------------------------------------------------------
//	TMutableString CFStringRef constructor
// -----------------------------------------------------------------------------
//
TMutableString::TMutableString(
	CFStringRef				inString,
	CFIndex					inMaxLength,
	CFOwnership	inOwnership )
{
	if ( inOwnership == kCFOwnershipCopy )
		SetCFRef( ::CFStringCreateMutableCopy( kCFAllocatorDefault, inMaxLength, inString ) );
	else
		SetCFRef( inString, inOwnership );
}

// -----------------------------------------------------------------------------
//	TMutableString CFStringRef with format and arguments constructor
// -----------------------------------------------------------------------------
//
TMutableString::TMutableString(
	CFDictionaryRef			inOptions,
	CFStringRef				inFormat,
	... )
{
	va_list					args;

	SetCFRef( ::CFStringCreateMutable( kCFAllocatorDefault, 0 ) );

	va_start( args, inFormat );

	::CFStringAppendFormatAndArguments( GetMutableStringRef(), inOptions, inFormat, args );

	va_end( args );
}

// -----------------------------------------------------------------------------
//	TMutableString substring from CFStringRef
// -----------------------------------------------------------------------------
//
TMutableString::TMutableString(
	CFStringRef			inString,
	CFRange				inRange,
	CFIndex				inMaxLength )
{
	TString	temp( inString, inRange );
	SetCFRef( ::CFStringCreateMutableCopy( kCFAllocatorDefault, inMaxLength, temp ) );
}

// -----------------------------------------------------------------------------
//	TMutableString destructor
// -----------------------------------------------------------------------------
//
TMutableString::~TMutableString()
{
}
