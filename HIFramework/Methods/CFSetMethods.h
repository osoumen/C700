// =============================================================================
//	CFSetMethods.h
// =============================================================================
//

#ifndef CFSetMethods_H_
#define CFSetMethods_H_

#include "CFTypeMethods.h"

typedef const void* SetValue;

#define CFSet	__CFSet

struct CFSet
	:	public CFType
{
	static CFSetRef	Create(
						SetValue*				inValues,
						CFIndex					inCount,
						const CFSetCallBacks*	inCallBacks = &kCFTypeSetCallBacks );

	inline CFSetRef	GetSetRef() const
						{ return (CFSetRef) GetCFRef(); }

	Boolean			Contains(
						SetValue	inValue ) const;
	SetValue		Get(
						SetValue	inValue ) const;
	CFIndex			GetCount() const;
	CFIndex			GetCountOf(
						SetValue	inValue ) const;

	// =========================================================================
	// Mutable
	// =========================================================================

	static CFMutableSetRef
					CreateMutable(
						CFIndex					inCapacity = 0,
						const CFSetCallBacks*	inCallBacks = &kCFTypeSetCallBacks );
	static CFMutableSetRef
					CreateMutable(
						CFSetRef				inSetToCopy,
						CFIndex					inCapacity = 0 );

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
};

// -----------------------------------------------------------------------------
//	Create
// -----------------------------------------------------------------------------
//
inline CFSetRef
CFSet::Create(
	SetValue*				inValues,
	CFIndex					inCount,
	const CFSetCallBacks*	inCallBacks )
{
	return ::CFSetCreate( kCFAllocatorDefault, inValues, inCount, inCallBacks );
}

// -----------------------------------------------------------------------------
//	Contains
// -----------------------------------------------------------------------------
//
inline Boolean
CFSet::Contains(
	SetValue	inValue ) const
{
	return ::CFSetContainsValue( GetSetRef(), inValue );
}

// -----------------------------------------------------------------------------
//	Get
// -----------------------------------------------------------------------------
//
inline SetValue
CFSet::Get(
	SetValue	inValue ) const
{
	return ::CFSetGetValue( GetSetRef(), inValue );
}

// -----------------------------------------------------------------------------
//	GetCount
// -----------------------------------------------------------------------------
//
inline CFIndex
CFSet::GetCount() const
{
	return ::CFSetGetCount( GetSetRef() );
}

// -----------------------------------------------------------------------------
//	GetCountOf
// -----------------------------------------------------------------------------
//
inline CFIndex
CFSet::GetCountOf(
	SetValue	inValue ) const
{
	return ::CFSetGetCountOfValue( GetSetRef(), inValue );
}

// -----------------------------------------------------------------------------
//	CreateMutable
// -----------------------------------------------------------------------------
//
inline CFMutableSetRef
CFSet::CreateMutable(
	CFIndex					inCapacity,
	const CFSetCallBacks*	inCallBacks )
{
	return ::CFSetCreateMutable( kCFAllocatorDefault, inCapacity, inCallBacks );
}

// -----------------------------------------------------------------------------
//	CreateMutable
// -----------------------------------------------------------------------------
//
inline CFMutableSetRef
CFSet::CreateMutable(
	CFSetRef				inSetToCopy,
	CFIndex					inCapacity )
{
	return ::CFSetCreateMutableCopy( kCFAllocatorDefault, inCapacity, inSetToCopy );
}

// -----------------------------------------------------------------------------
//	Add
// -----------------------------------------------------------------------------
//
inline void
CFSet::Add(
	SetValue	inValue )
{
	::CFSetAddValue( GetMutableSetRef(), inValue );
}

// -----------------------------------------------------------------------------
//	Remove
// -----------------------------------------------------------------------------
//
inline void
CFSet::Remove(
	SetValue	inValue )
{
	::CFSetRemoveValue( GetMutableSetRef(), inValue );
}

// -----------------------------------------------------------------------------
//	RemoveAll
// -----------------------------------------------------------------------------
//
inline void
CFSet::RemoveAll()
{
	::CFSetRemoveAllValues( GetMutableSetRef() );
}

// -----------------------------------------------------------------------------
//	Replace
// -----------------------------------------------------------------------------
//
inline void
CFSet::Replace(
	SetValue	inValue )
{
	::CFSetReplaceValue( GetMutableSetRef(), inValue );
}

// -----------------------------------------------------------------------------
//	Set
// -----------------------------------------------------------------------------
//
inline void
CFSet::Set(
	SetValue	inValue )
{
	::CFSetSetValue( GetMutableSetRef(), inValue );
}

#endif // CFSetMethods_H_
