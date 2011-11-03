// =============================================================================
//	TBag.h
// =============================================================================
//

#ifndef TBag_H_
#define TBag_H_

#include "TCFType.h"

typedef const void* BagValue;

class TBag
	:	public TCFType
{
public:
					TBag();
					TBag(
						CFBagRef		inBag,
						CFOwnership		inOwnership = kCFOwnershipTake );
					TBag(
						BagValue*		inValues,
						CFIndex			inCount,
						const
						CFBagCallBacks*	inCallBacks = &kCFTypeBagCallBacks );
	virtual			~TBag();

	operator		CFBagRef() const
						{ return GetBagRef(); }
	inline CFBagRef	GetBagRef() const
						{ return (CFBagRef) GetCFRef(); }

	Boolean			Contains(
						BagValue	inValue ) const;
	BagValue		Get(
						BagValue	inValue ) const;
	CFIndex			GetCount() const;
	CFIndex			GetCountOf(
						BagValue	inValue ) const;

private:
					// Disallow copy construction
					TBag( const TBag& );
	TBag&			operator =( const TBag& inTBag );
};

class TMutableBag
	:	public TBag
{
public:
					TMutableBag(
						CFMutableBagRef			inBag,
						CFIndex					inCapacity,
						CFOwnership				inOwnership );
					TMutableBag(
						CFIndex					inCapacity = 0,
						const CFBagCallBacks*	inCallBacks = &kCFTypeBagCallBacks );
	virtual			~TMutableBag();

	operator		CFMutableBagRef() const
						{ return GetMutableBagRef(); }
	inline
	CFMutableBagRef	GetMutableBagRef() const
						{ return (CFMutableBagRef) GetCFRef(); }

	void			Add(
						BagValue	inValue );
	void			Remove(
						BagValue	inValue );
	void			RemoveAll();
	void			Replace(
						BagValue	inValue );
	void			Set(
						BagValue	inValue );

private:
					// Disallow copy construction
					TMutableBag( const TMutableBag& );
	TMutableBag&	operator =( const TMutableBag& inTMutableBag );
};

// -----------------------------------------------------------------------------
//	Get
// -----------------------------------------------------------------------------
//
Boolean
TBag::Contains(
	BagValue		inValue ) const
{
	return ::CFBagContainsValue( GetBagRef(), inValue );
}

// -----------------------------------------------------------------------------
//	Get
// -----------------------------------------------------------------------------
//
BagValue
TBag::Get(
	BagValue		inValue ) const
{
	return ::CFBagGetValue( GetBagRef(), inValue );
};

// -----------------------------------------------------------------------------
//	GetCount
// -----------------------------------------------------------------------------
//
CFIndex
TBag::GetCount() const
{
	return ::CFBagGetCount( GetBagRef() );
}

// -----------------------------------------------------------------------------
//	GetCountOf
// -----------------------------------------------------------------------------
//
CFIndex
TBag::GetCountOf(
	BagValue		inValue ) const
{
	return ::CFBagGetCountOfValue( GetBagRef(), inValue );
}

// =============================================================================
//	MUTABLE
// =============================================================================

// -----------------------------------------------------------------------------
//	Add
// -----------------------------------------------------------------------------
//
void
TMutableBag::Add(
	BagValue		inValue )
{
	::CFBagAddValue( GetMutableBagRef(), inValue );
}

// -----------------------------------------------------------------------------
//	Remove
// -----------------------------------------------------------------------------
//
void
TMutableBag::Remove(
	BagValue		inValue )
{
	::CFBagRemoveValue( GetMutableBagRef(), inValue );
}

// -----------------------------------------------------------------------------
//	RemoveAll
// -----------------------------------------------------------------------------
//
void
TMutableBag::RemoveAll()
{
	::CFBagRemoveAllValues( GetMutableBagRef() );
}

// -----------------------------------------------------------------------------
//	Replace
// -----------------------------------------------------------------------------
//
void
TMutableBag::Replace(
	BagValue		inValue )
{
	::CFBagReplaceValue( GetMutableBagRef(), inValue );
}

// -----------------------------------------------------------------------------
//	Set
// -----------------------------------------------------------------------------
//
void
TMutableBag::Set(
	BagValue		inValue )
{
	::CFBagSetValue( GetMutableBagRef(), inValue );
}

#endif // TBag_H_
