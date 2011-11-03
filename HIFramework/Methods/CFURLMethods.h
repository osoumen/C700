// =============================================================================
//	CFURLMethods.h
// =============================================================================
//

#ifndef CFURLMethods_H_
#define CFURLMethods_H_

#include "CFTypeMethods.h"

#define CFURL	__CFURL

struct CFURL
	:	public CFType
{
	static CFURLRef	Create(
						const UInt8*			inBytes,
						CFIndex					inLength,
						CFStringEncoding		inEncoding = kCFStringEncodingMacRoman,
						CFURLRef				inBaseURL = NULL );
	static CFURLRef	Create(
						CFStringRef				inString,
						CFURLRef				inBaseURL = NULL );
	static CFURLRef	Create(
						CFStringRef				inPath,
						CFURLPathStyle			inPathStyle,
						Boolean					inIsDirectory,
						CFURLRef				inBaseURL = NULL );
	static CFURLRef	Create(
						const UInt8*			inBytes,
						CFIndex					inLength,
						Boolean					inIsDirectory,
						CFURLRef				inBaseURL = NULL );
	static CFURLRef	Create(
						const FSRef*			inFSRef );

	inline CFURLRef	GetURLRef() const
						{ return (CFURLRef) this; }

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
};

// -----------------------------------------------------------------------------
//	Create
// -----------------------------------------------------------------------------
//
inline CFURLRef
CFURL::Create(
	const UInt8*			inBytes,
	CFIndex					inLength,
	CFStringEncoding		inEncoding,
	CFURLRef				inBaseURL )
{
	return ::CFURLCreateWithBytes( kCFAllocatorDefault, inBytes, inLength, inEncoding, inBaseURL );
}

// -----------------------------------------------------------------------------
//	Create
// -----------------------------------------------------------------------------
//
inline CFURLRef
CFURL::Create(
	CFStringRef				inString,
	CFURLRef				inBaseURL )
{
	return ::CFURLCreateWithString( kCFAllocatorDefault, inString, inBaseURL );
}

// -----------------------------------------------------------------------------
//	Create
// -----------------------------------------------------------------------------
//
inline CFURLRef
CFURL::Create(
	CFStringRef				inPath,
	CFURLPathStyle			inPathStyle,
	Boolean					inIsDirectory,
	CFURLRef				inBaseURL )
{
	return ::CFURLCreateWithFileSystemPath( kCFAllocatorDefault, inPath, inPathStyle, inIsDirectory );
}

// -----------------------------------------------------------------------------
//	Create
// -----------------------------------------------------------------------------
//
inline CFURLRef
CFURL::Create(
	const UInt8*			inBytes,
	CFIndex					inLength,
	Boolean					inIsDirectory,
	CFURLRef				inBaseURL )
{
	return ::CFURLCreateFromFileSystemRepresentation( kCFAllocatorDefault, inBytes, inLength,  inIsDirectory );
}

// -----------------------------------------------------------------------------
//	Create
// -----------------------------------------------------------------------------
//
inline CFURLRef
CFURL::Create(
	const FSRef*			inFSRef )
{
	return ::CFURLCreateFromFSRef( kCFAllocatorDefault, inFSRef );
}

// -----------------------------------------------------------------------------
//	CanBeDecomposed
// -----------------------------------------------------------------------------
//
inline Boolean		
CFURL::CanBeDecomposed() const
{
	return ::CFURLCanBeDecomposed( GetURLRef() );
}

// -----------------------------------------------------------------------------
//	CopyAbsoluteURL
// -----------------------------------------------------------------------------
//
inline CFURLRef	
CFURL::CopyAbsoluteURL() const
{
	return ::CFURLCopyAbsoluteURL( GetURLRef() );
}

// -----------------------------------------------------------------------------
//	CopyAsData
// -----------------------------------------------------------------------------
//
inline CFDataRef	
CFURL::CopyAsData(
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
CFURL::CopyFileSystemPath(
	CFURLPathStyle			inPathStyle ) const
{
	return ::CFURLCopyFileSystemPath( GetURLRef(), inPathStyle );
}

// -----------------------------------------------------------------------------
//	CopyFragment
// -----------------------------------------------------------------------------
//
inline CFStringRef	
CFURL::CopyFragment(
	CFStringRef				inCharactersToLeaveEscaped ) const
{
	return ::CFURLCopyFragment( GetURLRef(), inCharactersToLeaveEscaped );
}

// -----------------------------------------------------------------------------
//	CopyHostName
// -----------------------------------------------------------------------------
//
inline CFStringRef	
CFURL::CopyHostName() const
{
	return ::CFURLCopyHostName( GetURLRef() );
}

// -----------------------------------------------------------------------------
//	CopyLastPathComponent
// -----------------------------------------------------------------------------
//
inline CFStringRef	
CFURL::CopyLastPathComponent() const
{
	return ::CFURLCopyLastPathComponent( GetURLRef() );
}

// -----------------------------------------------------------------------------
//	CopyNetLocation
// -----------------------------------------------------------------------------
//
inline CFStringRef	
CFURL::CopyNetLocation() const
{
	return ::CFURLCopyNetLocation( GetURLRef() );
}

// -----------------------------------------------------------------------------
//	CopyPath
// -----------------------------------------------------------------------------
//
inline CFStringRef	
CFURL::CopyPath() const
{
	return ::CFURLCopyPath( GetURLRef() );
}

// -----------------------------------------------------------------------------
//	CopyPathExtension
// -----------------------------------------------------------------------------
//
inline CFStringRef	
CFURL::CopyPathExtension() const
{
	return ::CFURLCopyPathExtension( GetURLRef() );
}

// -----------------------------------------------------------------------------
//	CopyParameterString
// -----------------------------------------------------------------------------
//
inline CFStringRef	
CFURL::CopyParameterString(
	CFStringRef				inCharactersToLeaveEscaped ) const
{
	return ::CFURLCopyParameterString( GetURLRef(), inCharactersToLeaveEscaped );
}

// -----------------------------------------------------------------------------
//	CopyPassword
// -----------------------------------------------------------------------------
//
inline CFStringRef	
CFURL::CopyPassword() const
{
	return ::CFURLCopyPassword( GetURLRef() );
}

// -----------------------------------------------------------------------------
//	CopyQueryString
// -----------------------------------------------------------------------------
//
inline CFStringRef	
CFURL::CopyQueryString(
	CFStringRef				inCharactersToLeaveEscaped ) const
{
	return ::CFURLCopyQueryString( GetURLRef(), inCharactersToLeaveEscaped );
}

// -----------------------------------------------------------------------------
//	CopyScheme
// -----------------------------------------------------------------------------
//
inline CFStringRef	
CFURL::CopyScheme() const
{
	return ::CFURLCopyScheme( GetURLRef() );
}

// -----------------------------------------------------------------------------
//	CopyStrictPath
// -----------------------------------------------------------------------------
//
inline CFStringRef	
CFURL::CopyStrictPath(
	Boolean*				outIsAbsolute ) const
{
	return ::CFURLCopyStrictPath( GetURLRef(), outIsAbsolute );
}

// -----------------------------------------------------------------------------
//	CopyResourceSpecifier
// -----------------------------------------------------------------------------
//
inline CFStringRef	
CFURL::CopyResourceSpecifier() const
{
	return ::CFURLCopyResourceSpecifier( GetURLRef() );
}

// -----------------------------------------------------------------------------
//	CopyUserName
// -----------------------------------------------------------------------------
//
inline CFStringRef	
CFURL::CopyUserName() const
{
	return ::CFURLCopyUserName( GetURLRef() );
}

// -----------------------------------------------------------------------------
//	GetBaseURL
// -----------------------------------------------------------------------------
//
inline CFURLRef	
CFURL::GetBaseURL() const
{
	return ::CFURLGetBaseURL( GetURLRef() );
}

// -----------------------------------------------------------------------------
//	GetFSRef
// -----------------------------------------------------------------------------
//
inline Boolean		
CFURL::GetFSRef(
	FSRef*					outRef ) const
{
	return ::CFURLGetFSRef( GetURLRef(), outRef );
}

// -----------------------------------------------------------------------------
//	GetPortNumber
// -----------------------------------------------------------------------------
//
inline SInt32		
CFURL::GetPortNumber() const
{
	return ::CFURLGetPortNumber( GetURLRef() );
}

// -----------------------------------------------------------------------------
//	GetString
// -----------------------------------------------------------------------------
//
inline CFStringRef	
CFURL::GetString() const
{
	return ::CFURLGetString( GetURLRef() );
}

#endif // CFURLMethods_H_
