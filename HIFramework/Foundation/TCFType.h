// =============================================================================
//	TCFType.h
// =============================================================================
//

#ifndef TCFType_H_
#define TCFType_H_

#include "TBase.h"

typedef enum
{
	kCFOwnershipTake = 0,
	kCFOwnershipRetain,
	kCFOwnershipCopy,
} CFOwnership;

class TCFType
	:	public TBase
{
public:
						TCFType(
							CFTypeRef		inTypeRef,
							CFOwnership		inOwnership = kCFOwnershipTake );
	virtual				~TCFType();

	operator			CFTypeRef()
							{ return (CFPropertyListRef) GetCFRef(); }
	inline CFTypeRef	GetCFRef() const
							{ return fCFRef; }

	CFStringRef			CopyDescription() const;
	CFStringRef			CopyTypeDescription() const;
	CFHashCode			GetHashCode() const;
	CFIndex				GetRetainCount() const;
	CFTypeID			GetTypeID() const;
	Boolean				IsEqual(
							CFTypeRef	inCompareTo ) const;
	void				Release();
	void				Retain();
	void				Show() const;

protected:
						TCFType();
	void				SetCFRef(
							CFTypeRef		inTypeRef,
							CFOwnership		inOwnership = kCFOwnershipTake );

private:
						// Disallow copy construction
						TCFType( const TCFType& );
	TCFType&			operator =( const TCFType& inTCFType );

	CFTypeRef			fCFRef;
};

// -----------------------------------------------------------------------------
//	CopyDescription
// -----------------------------------------------------------------------------
//
inline CFStringRef
TCFType::CopyDescription() const
{
	return ::CFCopyDescription( GetCFRef() );
}

// -----------------------------------------------------------------------------
//	CopyTypeDescription
// -----------------------------------------------------------------------------
//
inline CFStringRef
TCFType::CopyTypeDescription() const
{
	return ::CFCopyTypeIDDescription( ::CFGetTypeID( GetCFRef() ) );
}

// -----------------------------------------------------------------------------
//	GetHashCode
// -----------------------------------------------------------------------------
//
inline CFHashCode
TCFType::GetHashCode() const
{
	return ::CFHash( GetCFRef() );
}

// -----------------------------------------------------------------------------
//	GetRetainCount
// -----------------------------------------------------------------------------
//
inline CFIndex
TCFType::GetRetainCount() const
{
	return ::CFGetRetainCount( GetCFRef() );
}

// -----------------------------------------------------------------------------
//	GetTypeID
// -----------------------------------------------------------------------------
//
inline CFTypeID
TCFType::GetTypeID() const
{
	return ::CFGetTypeID( GetCFRef() );
}

// -----------------------------------------------------------------------------
//	IsEqual
// -----------------------------------------------------------------------------
//
inline Boolean
TCFType::IsEqual(
	CFTypeRef		inCompareTo ) const
{
	return ::CFEqual( GetCFRef(), inCompareTo );
}

// -----------------------------------------------------------------------------
//	Release
// -----------------------------------------------------------------------------
//
inline void
TCFType::Release()
{
	if ( GetCFRef() != NULL );
		::CFRelease( GetCFRef() );
}

// -----------------------------------------------------------------------------
//	Retain
// -----------------------------------------------------------------------------
//
inline void
TCFType::Retain()
{
	check( GetCFRef() != NULL );
	::CFRetain( GetCFRef() );
}

// -----------------------------------------------------------------------------
//	Show
// -----------------------------------------------------------------------------
//
inline void
TCFType::Show() const
{
	::CFShow( GetCFRef() );
}

#endif // TCFType_H_