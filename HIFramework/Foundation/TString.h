// =============================================================================
//	TString.h
// =============================================================================
//

#ifndef TString_H_
#define TString_H_

#include "TCFType.h"

class TString
	:	public TCFType
{
public:
					TString();
					TString(
						ConstStr255Param		inPStr,
						CFStringEncoding		inEncoding = kCFStringEncodingMacRoman );
					TString(
						const char*				inCStr,
						CFStringEncoding		inEncoding = kCFStringEncodingMacRoman );
					TString(
						const UniChar*			inUStr,
						CFIndex					inCount );
					TString(
						const UInt8*			inBytes,
						CFIndex					inCount,
						CFStringEncoding		inEncoding = kCFStringEncodingMacRoman,
						Boolean					inExternalRep = false );
					TString(
						CFDataRef				inData,
						CFStringEncoding		inEncoding = kCFStringEncodingMacRoman );
					TString(
						CFStringRef				inString,
						CFOwnership				inOwnership = kCFOwnershipTake );
					TString(
						CFDictionaryRef			inOptions,
						CFStringRef				inFormat,
						... );
					TString(
						CFStringRef				inString,
						CFRange					inRange );
	virtual			~TString();

	operator		CFStringRef() const
						{ return GetStringRef(); }
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

private:
					// Disallow copy construction
					TString( const TString& );
	TString&		operator =( const TString& inTString );
};

class TMutableString
	:	public TString
{
public:
					TMutableString(
						CFIndex					inMaxLength = 0 );
					TMutableString(
						ConstStr255Param		inPStr,
						CFIndex					inMaxLength = 0,
						CFStringEncoding		inEncoding = kCFStringEncodingMacRoman );
					TMutableString(
						const char*				inCStr,
						CFIndex					inMaxLength = 0,
						CFStringEncoding		inEncoding = kCFStringEncodingMacRoman );
					TMutableString(
						const UniChar*			inUStr,
						CFIndex					inCount,
						CFIndex					inMaxLength = 0 );
					TMutableString(
						const UInt8*			inBytes,
						CFIndex					inCount,
						CFIndex					inMaxLength = 0,
						CFStringEncoding		inEncoding = kCFStringEncodingMacRoman,
						Boolean					inExternalRep = false );
					TMutableString(
						CFDataRef				inData,
						CFIndex					inMaxLength = 0,
						CFStringEncoding		inEncoding = kCFStringEncodingMacRoman );
					TMutableString(
						CFStringRef				inStringRef,
						CFIndex					inMaxLength,
						CFOwnership				inOwnership );
					TMutableString(
						CFDictionaryRef			inOptions,
						CFStringRef				inFormat,
						... );
					TMutableString(
						CFStringRef				inString,
						CFRange					inRange,
						CFIndex					inMaxLength = 0 );
	virtual			~TMutableString();

	operator		CFMutableStringRef() const
						{ return GetMutableStringRef(); }
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

private:
					// Disallow copy construction
					TMutableString( const TMutableString& );
	TMutableString&	operator =( const TMutableString& inTMutableString );
};

// -----------------------------------------------------------------------------
//	Append CFStringRef
// -----------------------------------------------------------------------------
//
inline void
TMutableString::Append(
	CFStringRef				inString )
{
	::CFStringAppend( GetMutableStringRef(), inString );
}

// -----------------------------------------------------------------------------
//	Append UniChar characters
// -----------------------------------------------------------------------------
//
inline void
TMutableString::Append(
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
TMutableString::Append(
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
TMutableString::Append(
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
TMutableString::Append(
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
TMutableString::Capitalize(
	CFLocaleRef				inLocale )
{
	::CFStringCapitalize( GetMutableStringRef(), inLocale );
}

// -----------------------------------------------------------------------------
//	Delete
// -----------------------------------------------------------------------------
//
inline void
TMutableString::Delete(
	CFIndex					inIndex,
	CFIndex					inCount )
{
	::CFStringDelete( GetMutableStringRef(), CFRangeMake( inIndex, inCount ) );
}

// -----------------------------------------------------------------------------
//	Insert
// -----------------------------------------------------------------------------
//
inline void
TMutableString::Insert(
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
TMutableString::Lowercase(
	CFLocaleRef				inLocale )
{
	::CFStringLowercase( GetMutableStringRef(), inLocale );
}

// -----------------------------------------------------------------------------
//	Pad
// -----------------------------------------------------------------------------
//
inline void
TMutableString::Pad(
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
TMutableString::Replace(
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
TMutableString::Replace(
	CFStringRef				inReplacement )
{
	::CFStringReplaceAll( GetMutableStringRef(), inReplacement );
}

// -----------------------------------------------------------------------------
//	Trim
// -----------------------------------------------------------------------------
//
inline void
TMutableString::Trim(
	CFStringRef				inTrimString )
{
	::CFStringTrim( GetMutableStringRef(), inTrimString );
}

// -----------------------------------------------------------------------------
//	TrimWhitespace
// -----------------------------------------------------------------------------
//
inline void
TMutableString::TrimWhitespace()
{
	::CFStringTrimWhitespace( GetMutableStringRef() );
}

// -----------------------------------------------------------------------------
//	Uppercase
// -----------------------------------------------------------------------------
//
inline void
TMutableString::Uppercase(
	CFLocaleRef				inLocale )
{
	::CFStringUppercase( GetMutableStringRef(), inLocale );
}

#endif // TString_H_
