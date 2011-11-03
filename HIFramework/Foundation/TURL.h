// =============================================================================
//	TURL.h
// =============================================================================
//

#ifndef TURL_H_
#define TURL_H_

#include <CoreFoundation/CoreFoundation.h>

#include "TCFType.h"

class TURL
	:	public TCFType
{
public:
					TURL();
					TURL(
						CFURLRef				inURL,
						CFOwnership				inOwnership = kCFOwnershipTake );
					TURL(
						const UInt8*			inBytes,
						CFIndex					inLength,
						CFStringEncoding		inEncoding = kCFStringEncodingMacRoman,
						CFURLRef				inBaseURL = NULL );
					TURL(
						CFStringRef				inString,
						CFURLRef				inBaseURL = NULL );
					TURL(
						CFStringRef				inPath,
						CFURLPathStyle			inPathStyle,
						Boolean					inIsDirectory,
						CFURLRef				inBaseURL = NULL );
#if 1
					TURL(
						const UInt8*			inBytes,
						CFIndex					inLength,
						Boolean					inIsDirectory,
						CFURLRef				inBaseURL = NULL );
#endif
					TURL(
						const FSRef*			inFSRef );

	virtual			~TURL();

	operator		CFURLRef() const
						{ return GetURLRef(); }
	inline CFURLRef	GetURLRef() const
						{ return (CFURLRef) GetCFRef(); }

	Boolean			CanBeDecomposed() const;
	CFURLRef		CopyAbsoluteURL() const;
	CFDataRef		CopyAsData(
						CFStringEncoding		inEncoding = kCFStringEncodingMacRoman,
						Boolean					inEscapeWhiteSpace = true ) const;
	CFStringRef		CopyFileSystemPath(
						CFURLPathStyle			inPathStyle ) const;
	CFStringRef		CopyFragment(
						CFStringRef				inCharactersToLeaveEscaped = NULL ) const;
	CFStringRef		CopyHostName() const;
	CFStringRef		CopyLastPathComponent() const;
	CFStringRef		CopyNetLocation() const;
	CFStringRef		CopyPath() const;
	CFStringRef		CopyPathExtension() const;
	CFStringRef		CopyParameterString(
						CFStringRef				inCharactersToLeaveEscaped = NULL ) const;
	CFStringRef		CopyPassword() const;
	CFStringRef		CopyQueryString(
						CFStringRef				inCharactersToLeaveEscaped = NULL ) const;
	CFStringRef		CopyScheme() const;
	CFStringRef		CopyStrictPath(
						Boolean*				outIsAbsolute = NULL ) const;
	CFStringRef		CopyResourceSpecifier() const;
	CFStringRef		CopyUserName() const;
	CFURLRef		GetBaseURL() const;
	Boolean			GetFSRef(
						FSRef*					outRef ) const;
	SInt32			GetPortNumber() const;
	CFStringRef		GetString() const;

private:
					// Disallow copy construction
					TURL( const TURL& );
	TURL&			operator =( const TURL& inTURL );
};

// -----------------------------------------------------------------------------
//	CanBeDecomposed
// -----------------------------------------------------------------------------
//
inline Boolean		
TURL::CanBeDecomposed() const
{
	return ::CFURLCanBeDecomposed( GetURLRef() );
}

// -----------------------------------------------------------------------------
//	CopyAbsoluteURL
// -----------------------------------------------------------------------------
//
inline CFURLRef	
TURL::CopyAbsoluteURL() const
{
	return ::CFURLCopyAbsoluteURL( GetURLRef() );
}

// -----------------------------------------------------------------------------
//	CopyAsData
// -----------------------------------------------------------------------------
//
inline CFDataRef	
TURL::CopyAsData(
	CFStringEncoding		inEncoding,
	Boolean					inEscapeWhiteSpace ) const
{
	return ::CFURLCreateData( kCFAllocatorDefault, GetURLRef(), inEncoding, inEscapeWhiteSpace );
}

// -----------------------------------------------------------------------------
//	CopyFileSystemPath
// -----------------------------------------------------------------------------
//
inline CFStringRef	
TURL::CopyFileSystemPath(
	CFURLPathStyle			inPathStyle ) const
{
	return ::CFURLCopyFileSystemPath( GetURLRef(), inPathStyle );
}

// -----------------------------------------------------------------------------
//	CopyFragment
// -----------------------------------------------------------------------------
//
inline CFStringRef	
TURL::CopyFragment(
	CFStringRef				inCharactersToLeaveEscaped ) const
{
	return ::CFURLCopyFragment( GetURLRef(), inCharactersToLeaveEscaped );
}

// -----------------------------------------------------------------------------
//	CopyHostName
// -----------------------------------------------------------------------------
//
inline CFStringRef	
TURL::CopyHostName() const
{
	return ::CFURLCopyHostName( GetURLRef() );
}

// -----------------------------------------------------------------------------
//	CopyLastPathComponent
// -----------------------------------------------------------------------------
//
inline CFStringRef	
TURL::CopyLastPathComponent() const
{
	return ::CFURLCopyLastPathComponent( GetURLRef() );
}

// -----------------------------------------------------------------------------
//	CopyNetLocation
// -----------------------------------------------------------------------------
//
inline CFStringRef	
TURL::CopyNetLocation() const
{
	return ::CFURLCopyNetLocation( GetURLRef() );
}

// -----------------------------------------------------------------------------
//	CopyPath
// -----------------------------------------------------------------------------
//
inline CFStringRef	
TURL::CopyPath() const
{
	return ::CFURLCopyPath( GetURLRef() );
}

// -----------------------------------------------------------------------------
//	CopyPathExtension
// -----------------------------------------------------------------------------
//
inline CFStringRef	
TURL::CopyPathExtension() const
{
	return ::CFURLCopyPathExtension( GetURLRef() );
}

// -----------------------------------------------------------------------------
//	CopyParameterString
// -----------------------------------------------------------------------------
//
inline CFStringRef	
TURL::CopyParameterString(
	CFStringRef				inCharactersToLeaveEscaped ) const
{
	return ::CFURLCopyParameterString( GetURLRef(), inCharactersToLeaveEscaped );
}

// -----------------------------------------------------------------------------
//	CopyPassword
// -----------------------------------------------------------------------------
//
inline CFStringRef	
TURL::CopyPassword() const
{
	return ::CFURLCopyPassword( GetURLRef() );
}

// -----------------------------------------------------------------------------
//	CopyQueryString
// -----------------------------------------------------------------------------
//
inline CFStringRef	
TURL::CopyQueryString(
	CFStringRef				inCharactersToLeaveEscaped ) const
{
	return ::CFURLCopyQueryString( GetURLRef(), inCharactersToLeaveEscaped );
}

// -----------------------------------------------------------------------------
//	CopyScheme
// -----------------------------------------------------------------------------
//
inline CFStringRef	
TURL::CopyScheme() const
{
	return ::CFURLCopyScheme( GetURLRef() );
}

// -----------------------------------------------------------------------------
//	CopyStrictPath
// -----------------------------------------------------------------------------
//
inline CFStringRef	
TURL::CopyStrictPath(
	Boolean*				outIsAbsolute ) const
{
	return ::CFURLCopyStrictPath( GetURLRef(), outIsAbsolute );
}

// -----------------------------------------------------------------------------
//	CopyResourceSpecifier
// -----------------------------------------------------------------------------
//
inline CFStringRef	
TURL::CopyResourceSpecifier() const
{
	return ::CFURLCopyResourceSpecifier( GetURLRef() );
}

// -----------------------------------------------------------------------------
//	CopyUserName
// -----------------------------------------------------------------------------
//
inline CFStringRef	
TURL::CopyUserName() const
{
	return ::CFURLCopyUserName( GetURLRef() );
}

// -----------------------------------------------------------------------------
//	GetBaseURL
// -----------------------------------------------------------------------------
//
inline CFURLRef	
TURL::GetBaseURL() const
{
	return ::CFURLGetBaseURL( GetURLRef() );
}

// -----------------------------------------------------------------------------
//	GetFSRef
// -----------------------------------------------------------------------------
//
inline Boolean		
TURL::GetFSRef(
	FSRef*					outRef ) const
{
	return ::CFURLGetFSRef( GetURLRef(), outRef );
}

// -----------------------------------------------------------------------------
//	GetPortNumber
// -----------------------------------------------------------------------------
//
inline SInt32		
TURL::GetPortNumber() const
{
	return ::CFURLGetPortNumber( GetURLRef() );
}

// -----------------------------------------------------------------------------
//	GetString
// -----------------------------------------------------------------------------
//
inline CFStringRef	
TURL::GetString() const
{
	return ::CFURLGetString( GetURLRef() );
}

#endif // TURL_H_
