// =============================================================================
//	CFStringMethods.h
// =============================================================================
//

#ifndef CFStringMethods_H_
#define CFStringMethods_H_

#include "CFTypeMethods.h"

#define CFString	__CFString

struct CFString
	:	public CFType
{
	static
	CFStringRef		Create(
						CFStringRef				inString );
	static
	CFStringRef		Create(
						ConstStr255Param		inPStr,
						CFStringEncoding		inEncoding = kCFStringEncodingMacRoman );
	static
	CFStringRef		Create(
						const char*				inCStr,
						CFStringEncoding		inEncoding = kCFStringEncodingMacRoman );
	static
	CFStringRef		Create(
						const UniChar*			inUStr,
						CFIndex					inCount );
	static
	CFStringRef		Create(
						const UInt8*			inBytes,
						CFIndex					inCount,
						CFStringEncoding		inEncoding = kCFStringEncodingMacRoman,
						Boolean					inExternalRep = false );
	static CFStringRef
					Create(
						CFDataRef				inData,
						CFStringEncoding		inEncoding = kCFStringEncodingMacRoman );
	static CFStringRef
					Create(
						CFDictionaryRef			inOptions,
						CFStringRef				inFormat,
						... );
	static CFStringRef
					Create(
						CFStringRef				inFormat,
						CFRange					inRange );

	inline
	CFStringRef		GetStringRef() const
						{ return (CFStringRef) GetCFRef(); }

	CFComparisonResult
					Compare(
						CFStringRef				inCompareTo,
						CFOptionFlags			inOptions = 0 ) const;
	CFRange			Find(
						CFStringRef				inStringToFind,
						CFOptionFlags			inOptions = 0 ) const;

	UniChar			GetCharacter(
						CFIndex					inIndex ) const;
	void			GetCharacters(
						CFIndex					inIndex,
						CFIndex					inCount,
						UniChar*				outBuffer ) const;
	double			GetDoubleValue() const;
	SInt32			GetIntValue() const;
	CFIndex			GetLength() const;
	Boolean			GetString(
						StringPtr				outBuffer,
						CFIndex					inCount,
						CFStringEncoding		inEncoding = kCFStringEncodingMacRoman ) const;
	Boolean			GetString(
						char*					outBuffer,
						CFIndex					inCount,
						CFStringEncoding		inEncoding = kCFStringEncodingMacRoman ) const;
	Boolean			HasPrefix(
						CFStringRef				inPrefix ) const;
	Boolean			HasSuffix(
						CFStringRef				inSuffix ) const;

	// =========================================================================
	// Mutable
	// =========================================================================

	static CFMutableStringRef
					CreateMutable(
						CFIndex					inMaxLength = 0 );
	static CFMutableStringRef
					CreateMutable(
						CFStringRef				inString,
						CFIndex					inMaxLength = 0 );
	static CFMutableStringRef
					CreateMutable(
						ConstStr255Param		inPStr,
						CFStringEncoding		inEncoding = kCFStringEncodingMacRoman );
	static CFMutableStringRef
					CreateMutable(
						const char*				inCStr,
						CFStringEncoding		inEncoding = kCFStringEncodingMacRoman );
	static CFMutableStringRef
					CreateMutable(
						const UniChar*			inUStr,
						CFIndex					inCount );
	static CFMutableStringRef
					CreateMutable(
						const UInt8*			inBytes,
						CFIndex					inCount,
						CFStringEncoding		inEncoding = kCFStringEncodingMacRoman,
						Boolean					inExternalRep = false );
	static CFMutableStringRef
					CreateMutable(
						CFDataRef				inData,
						CFStringEncoding		inEncoding = kCFStringEncodingMacRoman );
	static CFMutableStringRef
					CreateMutable(
						CFDictionaryRef			inOptions,
						CFStringRef				inFormat,
						... );
	static CFMutableStringRef
					CreateMutable(
						CFStringRef				inString,
						CFRange					inRange );

	inline
	CFMutableStringRef
					GetMutableStringRef() const
						{ return (CFMutableStringRef) GetCFRef(); }

	void			Append(
						CFStringRef				inString );
	void			Append(
						const UniChar*			inUStr,
						CFIndex					inCount );
	void			Append(
						ConstStr255Param		inPStr,
						CFStringEncoding		inEncoding = kCFStringEncodingMacRoman );
	void			Append(
						const char*				inCStr,
						CFStringEncoding		inEncoding = kCFStringEncodingMacRoman );
	void			Append(
						CFDictionaryRef			inOptions,
						CFStringRef				inFormat,
						... );
	void			Capitalize(
						CFLocaleRef				inLocale = NULL );
	void			Delete(
						CFIndex					inIndex,
						CFIndex					inLength );
	void			Insert(
						CFIndex					inIndex,
						CFStringRef				inStringToInsert );
	void			Lowercase(
						CFLocaleRef				inLocale = NULL );
	void			Pad(
						CFStringRef				inPadString,
						CFIndex					inFinalCount,
						CFIndex					inPadStartIndex = 0 );
	void			Replace(
						CFIndex					inIndex,
						CFIndex					inLength,
						CFStringRef				inReplacement );
	void			Replace(
						CFStringRef				inReplacement );
	void			Trim(
						CFStringRef				inTrimString );
	void			TrimWhitespace();
	void			Uppercase(
						CFLocaleRef				inLocale = NULL );
};

// -----------------------------------------------------------------------------
//	Create
// -----------------------------------------------------------------------------
//
inline CFStringRef
CFString::Create(
	CFStringRef		inString )
{
	return ::CFStringCreateCopy( kCFAllocatorDefault, inString );
}

// -----------------------------------------------------------------------------
//	Create
// -----------------------------------------------------------------------------
//
inline CFStringRef
CFString::Create(
	ConstStr255Param		inPStr,
	CFStringEncoding		inEncoding )
{
	return ::CFStringCreateWithPascalString( kCFAllocatorDefault, inPStr, inEncoding );
}

// -----------------------------------------------------------------------------
//	Create
// -----------------------------------------------------------------------------
//
inline CFStringRef
CFString::Create(
	const char*				inCStr,
	CFStringEncoding		inEncoding )
{
	return ::CFStringCreateWithCString( kCFAllocatorDefault, inCStr, inEncoding );
}

// -----------------------------------------------------------------------------
//	Create
// -----------------------------------------------------------------------------
//
inline CFStringRef
CFString::Create(
	const UniChar*			inUStr,
	CFIndex					inCount )
{
	return ::CFStringCreateWithCharacters( kCFAllocatorDefault, inUStr, inCount );
}

// -----------------------------------------------------------------------------
//	Create
// -----------------------------------------------------------------------------
//
inline CFStringRef
CFString::Create(
	const UInt8*			inBytes,
	CFIndex					inCount,
	CFStringEncoding		inEncoding,
	Boolean					inExternalRep )
{
	return ::CFStringCreateWithBytes( kCFAllocatorDefault, inBytes, inCount, inEncoding, inExternalRep );
}

// -----------------------------------------------------------------------------
//	Create
// -----------------------------------------------------------------------------
//
inline CFStringRef
CFString::Create(
	CFDataRef				inData,
	CFStringEncoding		inEncoding )
{
	return ::CFStringCreateFromExternalRepresentation( kCFAllocatorDefault, inData, inEncoding );
}

// -----------------------------------------------------------------------------
//	Create
// -----------------------------------------------------------------------------
//
inline CFStringRef
CFString::Create(
	CFDictionaryRef			inOptions,
	CFStringRef				inFormat,
	... )
{
	CFStringRef				string;
	va_list					args;

	va_start( args, inFormat );
	string = ::CFStringCreateWithFormatAndArguments( kCFAllocatorDefault, inOptions, inFormat, args );
	va_end( args );

	return string;
}

// -----------------------------------------------------------------------------
//	Create
// -----------------------------------------------------------------------------
//
inline CFStringRef
CFString::Create(
	CFStringRef				inString,
	CFRange					inRange )
{
	return ::CFStringCreateWithSubstring( kCFAllocatorDefault, inString, inRange );
}

// -----------------------------------------------------------------------------
//	Compare
// -----------------------------------------------------------------------------
//
inline CFComparisonResult
CFString::Compare(
	CFStringRef				inCompareTo,
	CFOptionFlags			inOptions ) const
{
	return ::CFStringCompare( GetStringRef(), inCompareTo, inOptions );
}

// -----------------------------------------------------------------------------
//	Find
// -----------------------------------------------------------------------------
//
inline CFRange
CFString::Find(
	CFStringRef				inStringToFind,
	CFOptionFlags			inOptions ) const
{
	return ::CFStringFind( GetStringRef(), inStringToFind, inOptions );
}

// -----------------------------------------------------------------------------
//	GetCharacter
// -----------------------------------------------------------------------------
//
inline UniChar
CFString::GetCharacter(
	CFIndex					inIndex ) const
{
	return ::CFStringGetCharacterAtIndex( GetStringRef(), inIndex );
}

// -----------------------------------------------------------------------------
//	GetCharacters
// -----------------------------------------------------------------------------
//
inline void
CFString::GetCharacters(
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
inline double
CFString::GetDoubleValue() const
{
	return ::CFStringGetDoubleValue( GetStringRef() );
}

// -----------------------------------------------------------------------------
//	GetIntValue
// -----------------------------------------------------------------------------
//
inline SInt32
CFString::GetIntValue() const
{
	return ::CFStringGetIntValue( GetStringRef() );
}

// -----------------------------------------------------------------------------
//	GetLength
// -----------------------------------------------------------------------------
//
inline CFIndex
CFString::GetLength() const
{
	return ::CFStringGetLength( GetStringRef() );
}

// -----------------------------------------------------------------------------
//	GetString Pascal string
// -----------------------------------------------------------------------------
//
inline Boolean
CFString::GetString(
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
inline Boolean
CFString::GetString(
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
inline Boolean
CFString::HasPrefix(
	CFStringRef				inPrefix ) const
{
	return ::CFStringHasPrefix( GetStringRef(), inPrefix );
}

// -----------------------------------------------------------------------------
//	HasSuffix
// -----------------------------------------------------------------------------
//
inline Boolean
CFString::HasSuffix(
	CFStringRef				inSuffix ) const
{
	return ::CFStringHasSuffix( GetStringRef(), inSuffix );
}

// -----------------------------------------------------------------------------
//	CreateMutable
// -----------------------------------------------------------------------------
//
inline CFMutableStringRef
CFString::CreateMutable(
	CFIndex			inMaxLength )
{
	return ::CFStringCreateMutable( kCFAllocatorDefault, inMaxLength );
}

// -----------------------------------------------------------------------------
//	CreateMutable
// -----------------------------------------------------------------------------
//
inline CFMutableStringRef
CFString::CreateMutable(
	CFStringRef				inString,
	CFIndex					inMaxLength )
{
	return ::CFStringCreateMutableCopy( kCFAllocatorDefault, inMaxLength, inString );
}

// -----------------------------------------------------------------------------
//	CreateMutable
// -----------------------------------------------------------------------------
//
inline CFMutableStringRef
CFString::CreateMutable(
	ConstStr255Param		inPStr,
	CFStringEncoding		inEncoding )
{
	CFMutableStringRef	newString = CreateMutable();
	newString->Append( inPStr, inEncoding );
	return newString;
}

// -----------------------------------------------------------------------------
//	CreateMutable
// -----------------------------------------------------------------------------
//
inline CFMutableStringRef
CFString::CreateMutable(
	const char*				inCStr,
	CFStringEncoding		inEncoding )
{
	CFMutableStringRef	newString = CreateMutable();
	newString->Append( inCStr, inEncoding );
	return newString;
}

// -----------------------------------------------------------------------------
//	CreateMutable
// -----------------------------------------------------------------------------
//
inline CFMutableStringRef
CFString::CreateMutable(
	const UniChar*			inUStr,
	CFIndex					inCount )
{
	CFMutableStringRef	newString = CreateMutable();
	newString->Append( inUStr, inCount );
	return newString;
}

// -----------------------------------------------------------------------------
//	CreateMutable
// -----------------------------------------------------------------------------
//
inline CFMutableStringRef
CFString::CreateMutable(
	const UInt8*			inBytes,
	CFIndex					inCount,
	CFStringEncoding		inEncoding,
	Boolean					inExternalRep )
{
	CFStringRef				fillString = Create( inBytes, inCount, inEncoding, inExternalRep );
	CFMutableStringRef		newString = CreateMutable( fillString );
	fillString->Release();
	return newString;
}

// -----------------------------------------------------------------------------
//	CreateMutable
// -----------------------------------------------------------------------------
//
inline CFMutableStringRef
CFString::CreateMutable(
	CFDataRef				inData,
	CFStringEncoding		inEncoding )
{
	CFStringRef				fillString = Create( inData, inEncoding );
	CFMutableStringRef		newString = CreateMutable( fillString );
	fillString->Release();
	return newString;
}

// -----------------------------------------------------------------------------
//	CreateMutable
// -----------------------------------------------------------------------------
//
inline CFMutableStringRef
CFString::CreateMutable(
	CFDictionaryRef			inOptions,
	CFStringRef				inFormat,
	... )
{
	CFMutableStringRef		newString = CreateMutable();
	va_list					args;
	va_start( args, inFormat );

	newString->Append( inOptions, inFormat, args );

	va_end( args );
	return newString;
}

// -----------------------------------------------------------------------------
//	CreateMutable
// -----------------------------------------------------------------------------
//
inline CFMutableStringRef
CFString::CreateMutable(
	CFStringRef				inString,
	CFRange					inRange )
{
	CFStringRef				fillString = Create( inString, inRange );
	CFMutableStringRef		newString = CreateMutable( fillString );
	fillString->Release();
	return newString;
}

// -----------------------------------------------------------------------------
//	Append CFStringRef
// -----------------------------------------------------------------------------
//
inline void
CFString::Append(
	CFStringRef				inString )
{
	::CFStringAppend( GetMutableStringRef(), inString );
}

// -----------------------------------------------------------------------------
//	Append UniChar characters
// -----------------------------------------------------------------------------
//
inline void
CFString::Append(
	const UniChar*			inUStr,
	CFIndex					inCount )
{
	::CFStringAppendCharacters( GetMutableStringRef(), inUStr, inCount );
}

// -----------------------------------------------------------------------------
//	Append Pascal string
// -----------------------------------------------------------------------------
//
inline void
CFString::Append(
	ConstStr255Param		inPStr,
	CFStringEncoding		inEncoding )
{
	::CFStringAppendPascalString( GetMutableStringRef(), inPStr, inEncoding );
}

// -----------------------------------------------------------------------------
//	Append args
// -----------------------------------------------------------------------------
//
inline void
CFString::Append(
	CFDictionaryRef			inOptions,
	CFStringRef				inFormat,
	... )
{
	va_list					args;
	va_start( args, inFormat );

	::CFStringAppendFormat( GetMutableStringRef(), inOptions, inFormat, args );

	va_end( args );
}

// -----------------------------------------------------------------------------
//	Append C string
// -----------------------------------------------------------------------------
//
inline void
CFString::Append(
	const char*				inCStr,
	CFStringEncoding		inEncoding )
{
	::CFStringAppendCString( GetMutableStringRef(), inCStr, inEncoding );
}

// -----------------------------------------------------------------------------
//	Capitalize
// -----------------------------------------------------------------------------
//
inline void
CFString::Capitalize(
	CFLocaleRef				inLocale )
{
	::CFStringCapitalize( GetMutableStringRef(), inLocale );
}

// -----------------------------------------------------------------------------
//	Delete
// -----------------------------------------------------------------------------
//
inline void
CFString::Delete(
	CFIndex					inIndex,
	CFIndex					inLength )
{
	::CFStringDelete( GetMutableStringRef(), CFRangeMake( inIndex, inLength ) );
}

// -----------------------------------------------------------------------------
//	Insert
// -----------------------------------------------------------------------------
//
inline void
CFString::Insert(
	CFIndex					inIndex,
	CFStringRef				inStringToInsert )
{
	::CFStringInsert( GetMutableStringRef(), inIndex, inStringToInsert );
}

// -----------------------------------------------------------------------------
//	Lowercase
// -----------------------------------------------------------------------------
//
inline void
CFString::Lowercase(
	CFLocaleRef				inLocale )
{
	::CFStringLowercase( GetMutableStringRef(), inLocale );
}

// -----------------------------------------------------------------------------
//	Pad
// -----------------------------------------------------------------------------
//
inline void
CFString::Pad(
	CFStringRef				inPadString,
	CFIndex					inFinalCount,
	CFIndex					inPadStartIndex )
{
	::CFStringPad( GetMutableStringRef(), inPadString, inFinalCount, inPadStartIndex );
}

// -----------------------------------------------------------------------------
//	Replace
// -----------------------------------------------------------------------------
//
inline void
CFString::Replace(
	CFIndex					inIndex,
	CFIndex					inLength,
	CFStringRef				inReplacement )
{
	::CFStringReplace( GetMutableStringRef(), CFRangeMake( inIndex, inLength ), inReplacement );
}

// -----------------------------------------------------------------------------
//	Replace
// -----------------------------------------------------------------------------
//
inline void
CFString::Replace(
	CFStringRef				inReplacement )
{
	::CFStringReplaceAll( GetMutableStringRef(), inReplacement );
}

// -----------------------------------------------------------------------------
//	Trim
// -----------------------------------------------------------------------------
//
inline void
CFString::Trim(
	CFStringRef				inTrimString )
{
	::CFStringTrim( GetMutableStringRef(), inTrimString );
}

// -----------------------------------------------------------------------------
//	TrimWhitespace
// -----------------------------------------------------------------------------
//
inline void
CFString::TrimWhitespace()
{
	::CFStringTrimWhitespace( GetMutableStringRef() );
}

// -----------------------------------------------------------------------------
//	Uppercase
// -----------------------------------------------------------------------------
//
inline void
CFString::Uppercase(
	CFLocaleRef				inLocale )
{
	::CFStringUppercase( GetMutableStringRef(), inLocale );
}

#endif // CFStringMethods_H_
