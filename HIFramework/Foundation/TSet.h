// =============================================================================
//	TSet.h
// =============================================================================
//

#ifndef TSet_H_
#define TSet_H_

#include "TCFType.h"

typedef const void* SetValue;

class TSet
	:	public TCFType
{
public:
					TSet();
					TSet(
						CFSetRef				inSet,
						CFOwnership				inOwnership = kCFOwnershipTake );
					TSet(
						SetValue*				inValues,
						CFIndex					inCount,
						const CFSetCallBacks*	inCallBacks = &kCFTypeSetCallBacks );
	virtual			~TSet();

	operator		CFSetRef() const
						{ return GetSetRef(); }
	inline CFSetRef	GetSetRef() const
						{ return (CFSetRef) GetCFRef(); }

	Boolean			Contains(
						SetValue	inValue ) const;
	SetValue		Get(
						SetValue	inValue ) const;
	CFIndex			GetCount() const;
	CFIndex			GetCountOf(
						SetValue	inValue ) const;

private:
					// Disallow copy construction
					TSet( const TSet& );
	TSet&			operator =( const TSet& inTSet );
};

class TMutableSet
	:	public TSet
{
public:
					TMutableSet(
						CFSetRef				inSet,
						CFIndex					inCapacity,
						CFOwnership				inOwnership );
					TMutableSet(
						CFIndex					inCapacity = 0,
						const CFSetCallBacks*	inCallBacks = &kCFTypeSetCallBacks );
	virtual			~TMutableSet();

	operator		CFMutableSetRef() const
						{ return GetMutableSetRef(); }
	inline
	CFMutableSetRef	GetMutableSetRef() const
						{ return (CFMutableSetRef) GetCFRef(); }

	void			Add(
						SetValue	inValue );
	void			Remove(
						SetValue	inValue );
	void			RemoveAll();
	void			Replace(
						SetValue	inValue );
	void			Set(
						SetValue	inValue );

private:
					// Disallow copy construction
					TMutableSet( const TMutableSet& );
	TMutableSet&	operator =( const TMutableSet& inTMutableSet );
};

// -----------------------------------------------------------------------------
//	Contains
// -----------------------------------------------------------------------------
//
inline Boolean
TSet::Contains(
	SetValue	inValue ) const
{
	return ::CFSetContainsValue( GetSetRef(), inValue );
}

// -----------------------------------------------------------------------------
//	Get
// -----------------------------------------------------------------------------
//
inline SetValue
TSet::Get(
	SetValue		inValue ) const
{
	return ::CFSetGetValue( GetSetRef(), inValue );
}

// -----------------------------------------------------------------------------
//	GetCount
// -----------------------------------------------------------------------------
//
inline CFIndex
TSet::GetCount() const
{
	return ::CFSetGetCount( GetSetRef() );
}

// -----------------------------------------------------------------------------
//	GetCountOf
// -----------------------------------------------------------------------------
//
inline CFIndex
TSet::GetCountOf(
	SetValue		inValue ) const
{
	return ::CFSetGetCountOfValue( GetSetRef(), inValue );
}

// =============================================================================
//	MUTABLE
// =============================================================================

// -----------------------------------------------------------------------------
//	Add
// -----------------------------------------------------------------------------
//
inline void
TMutableSet::Add(
	SetValue		inValue )
{
	::CFSetAddValue( GetMutableSetRef(), inValue );
}

// -----------------------------------------------------------------------------
//	Remove
// -----------------------------------------------------------------------------
//
inline void
TMutableSet::Remove(
	SetValue		inValue )
{
	::CFSetRemoveValue( GetMutableSetRef(), inValue );
}

// -----------------------------------------------------------------------------
//	RemoveAll
// -----------------------------------------------------------------------------
//
inline void
TMutableSet::RemoveAll()
{
	::CFSetRemoveAllValues( GetMutableSetRef() );
}

// -----------------------------------------------------------------------------
//	Replace
// -----------------------------------------------------------------------------
//
inline void
TMutableSet::Replace(
	SetValue		inValue )
{
	::CFSetReplaceValue( GetMutableSetRef(), inValue );
}

// -----------------------------------------------------------------------------
//	Set
// -----------------------------------------------------------------------------
//
inline void
TMutableSet::Set(
	SetValue		inValue )
{
	::CFSetSetValue( GetMutableSetRef(), inValue );
}

#endif // TSet_H_
