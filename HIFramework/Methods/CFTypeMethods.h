// =============================================================================
//	CFTypeMethods.h
// =============================================================================
//

#ifndef CFTypeMethods_H_
#define CFTypeMethods_H_

#include <CoreFoundation/CoreFoundation.h>

#define CFType	__CFType

struct CFType
{
public:
	inline CFTypeRef	GetCFRef() const
							{ return this; }

	CFStringRef			CopyDescription() const;
	CFStringRef			CopyTypeDescription() const;
	CFHashCode			GetHashCode() const;
	CFIndex				GetRetainCount() const;
	CFTypeID			GetTypeID() const;
	Boolean				IsEqual(
							CFTypeRef	inCompareTo ) const;
	void				Release() const;
	void				Retain() const;
	void				Show() const;
};

// -----------------------------------------------------------------------------
//	CopyDescription
// -----------------------------------------------------------------------------
//
inline CFStringRef		
CFType::CopyDescription() const
{
	return ::CFCopyDescription( GetCFRef() );
}

// -----------------------------------------------------------------------------
//	CopyTypeDescription
// -----------------------------------------------------------------------------
//
inline CFStringRef		
CFType::CopyTypeDescription() const
{
	return ::CFCopyTypeIDDescription( ::CFGetTypeID( GetCFRef() ) );
}

// -----------------------------------------------------------------------------
//	GetHashCode
// -----------------------------------------------------------------------------
//
inline CFHashCode			
CFType::GetHashCode() const
{
	return ::CFHash( GetCFRef() );
}

// -----------------------------------------------------------------------------
//	GetRetainCount
// -----------------------------------------------------------------------------
//
inline CFIndex			
CFType::GetRetainCount() const
{
	return ::CFGetRetainCount( GetCFRef() );
}

// -----------------------------------------------------------------------------
//	GetTypeID
// -----------------------------------------------------------------------------
//
inline CFTypeID		
CFType::GetTypeID() const
{
	return ::CFGetTypeID( GetCFRef() );
}

// -----------------------------------------------------------------------------
//	IsEqual
// -----------------------------------------------------------------------------
//
inline Boolean		
CFType::IsEqual(
	CFTypeRef		inCompareTo ) const
{
	return ::CFEqual( GetCFRef(), inCompareTo );
}

// -----------------------------------------------------------------------------
//	Release
// -----------------------------------------------------------------------------
//
inline void			
CFType::Release() const
{
	check( GetCFRef() != NULL );
	check( GetRetainCount() > 1 );
	::CFRelease( GetCFRef() );
}

// -----------------------------------------------------------------------------
//	Retain
// -----------------------------------------------------------------------------
//
inline void			
CFType::Retain() const
{
	check( GetCFRef() != NULL );
	::CFRetain( GetCFRef() );
}

// -----------------------------------------------------------------------------
//	Show
// -----------------------------------------------------------------------------
//
inline void			
CFType::Show() const
{
	::CFShow( GetCFRef() );
}

#endif // CFTypeMethods_H_