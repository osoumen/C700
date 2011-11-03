// =============================================================================
//	CFBagMethods.h
// =============================================================================
//

#ifndef CFBagMethods_H_
#define CFBagMethods_H_

#include "CFTypeMethods.h"

typedef const void* BagValue;

#define CFBag	__CFBag

struct CFBag
	:	public CFType
{
public:
	static CFBagRef	Create(
						BagValue*				inValues,
						CFIndex					inCount,
						const CFBagCallBacks*	inCallBacks = &kCFTypeBagCallBacks );

	inline CFBagRef	GetBagRef() const
						{ return (CFBagRef) this; }

	Boolean			Contains(
						BagValue	inValue ) const;
	BagValue		Get(
						BagValue	inValue ) const;
	CFIndex			GetCount() const;
	CFIndex			GetCountOf(
						BagValue	inValue ) const;

	// =========================================================================
	//	Mutable
	// =========================================================================

	static
	CFMutableBagRef	CreateMutable(
						CFBagRef				inBagToCopy,
						CFIndex					inCapacity = 0 );
	static
	CFMutableBagRef	CreateMutable(
						CFIndex					inCapacity = 0,
						const CFBagCallBacks*	inCallBacks = &kCFTypeBagCallBacks );

	inline
	CFMutableBagRef	GetMutableBagRef() const
						{ return (CFMutableBagRef) this; }

	void			Add(
						BagValue	inValue );
	void			Remove(
						BagValue	inValue );
	void			RemoveAll();
	void			Replace(
						BagValue	inValue );
	void			Set(
						BagValue	inValue );
};

// -----------------------------------------------------------------------------
//	Create
// -----------------------------------------------------------------------------
//
inline CFBagRef
CFBag::Create(
	BagValue*				inValues,
	CFIndex					inCount,
	const CFBagCallBacks*	inCallBacks )
{
	return ::CFBagCreate( kCFAllocatorDefault, inValues, inCount, inCallBacks );
}

// -----------------------------------------------------------------------------
//	Contains
// -----------------------------------------------------------------------------
//
inline Boolean
CFBag::Contains(
	BagValue	inValue ) const
{
	return ::CFBagContainsValue( GetBagRef(), inValue );
}

// -----------------------------------------------------------------------------
//	Get
// -----------------------------------------------------------------------------
//
inline BagValue
CFBag::Get(
	BagValue	inValue ) const
{
	return ::CFBagGetValue( GetBagRef(), inValue );
}

// -----------------------------------------------------------------------------
//	GetCount
// -----------------------------------------------------------------------------
//
inline CFIndex
CFBag::GetCount() const
{
	return ::CFBagGetCount( GetBagRef() );
}

// -----------------------------------------------------------------------------
//	GetCountOf
// -----------------------------------------------------------------------------
//
inline CFIndex
CFBag::GetCountOf(
	BagValue	inValue ) const
{
	return ::CFBagGetCountOfValue( GetBagRef(), inValue );
}

// -----------------------------------------------------------------------------
//	CreateMutable
// -----------------------------------------------------------------------------
//
inline CFMutableBagRef
CFBag::CreateMutable(
	CFBagRef			inBagToCopy,
	CFIndex				inCapacity )
{
	return ::CFBagCreateMutableCopy( kCFAllocatorDefault, inCapacity, inBagToCopy );
}

// -----------------------------------------------------------------------------
//	CreateMutable
// -----------------------------------------------------------------------------
//
inline CFMutableBagRef
CFBag::CreateMutable(
	CFIndex					inCapacity,
	const CFBagCallBacks*	inCallBacks )
{
	return ::CFBagCreateMutable( kCFAllocatorDefault, inCapacity, inCallBacks );
}

// -----------------------------------------------------------------------------
//	Add
// -----------------------------------------------------------------------------
//
inline void
CFBag::Add(
	BagValue	inValue )
{
	::CFBagAddValue( GetMutableBagRef(), inValue );
}

// -----------------------------------------------------------------------------
//	Remove
// -----------------------------------------------------------------------------
//
inline void
CFBag::Remove(
	BagValue	inValue )
{
	::CFBagRemoveValue( GetMutableBagRef(), inValue );
}

// -----------------------------------------------------------------------------
//	RemoveAll
// -----------------------------------------------------------------------------
//
inline void
CFBag::RemoveAll()
{
	::CFBagRemoveAllValues( GetMutableBagRef() );
}

// -----------------------------------------------------------------------------
//	Replace
// -----------------------------------------------------------------------------
//
inline void
CFBag::Replace(
	BagValue	inValue )
{
	::CFBagReplaceValue( GetMutableBagRef(), inValue );
}

// -----------------------------------------------------------------------------
//	Set
// -----------------------------------------------------------------------------
//
inline void
CFBag::Set(
	BagValue	inValue )
{
	::CFBagSetValue( GetMutableBagRef(), inValue );
}

#endif // CFBagMethods_H_
