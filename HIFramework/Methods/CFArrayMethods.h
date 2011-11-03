// =============================================================================
//	CFArrayMethods.h
// =============================================================================
//

#ifndef CFArrayMethods_H_
#define CFArrayMethods_H_

#include "CFTypeMethods.h"

typedef const void* ArrayValue;

#define kNoArrayCallbacks	((CFArrayCallBacks*) NULL)

#define	CFArray	__CFArray

struct CFArray
	:	public CFType
{
public:
	static CFArrayRef	Create(
							CFArrayRef	inArrayToCopy );
	static CFArrayRef	Create(
							ArrayValue*	inValues,
							CFIndex		inCount,
							const
							CFArrayCallBacks*	inCallBacks = &kCFTypeArrayCallBacks );

	inline
	CFArrayRef			GetArrayRef() const
							{ return (CFArrayRef) this; };

	Boolean				Contains(
							ArrayValue	inValue ) const;
	Boolean				Contains(
							ArrayValue	inValue,
							CFIndex		inIndex,
							CFIndex		inLength ) const;
	ArrayValue			Get(
							CFIndex		inIndex ) const;
	CFIndex				GetCount() const;
	CFIndex				GetCountOf(
							ArrayValue	inValue ) const;
	CFIndex				GetCountOf(
							ArrayValue	inValue,
							CFIndex		inIndex,
							CFIndex		inLength ) const;
	ArrayValue			GetFirst() const;
	CFIndex				GetFirstIndexOf(
							ArrayValue	inValue ) const;
	CFIndex				GetFirstIndexOf(
							ArrayValue	inValue,
							CFIndex		inIndex,
							CFIndex		inLength ) const;
	ArrayValue			GetLast() const;
	CFIndex				GetLastIndexOf(
							ArrayValue	inValue ) const;
	CFIndex				GetLastIndexOf(
							ArrayValue	inValue,
							CFIndex		inIndex,
							CFIndex		inLength ) const;

	// =========================================================================
	//	Mutable
	// =========================================================================

	static
	CFMutableArrayRef	CreateMutable(
							CFArrayRef				inArrayToCopy,
							CFIndex					inCapacity = 0 );
	static
	CFMutableArrayRef	CreateMutable(
							CFIndex					inCapacity = 0,
							const CFArrayCallBacks* inCallBacks = &kCFTypeArrayCallBacks );

	inline
	CFMutableArrayRef	GetMutableArrayRef() const
							{ return (CFMutableArrayRef) this; }

	void				Append(
							ArrayValue	inValue );
	void				Exchange(
							CFIndex		inIndexA,
							CFIndex		inIndexB );
	ArrayValue			Extract(
							CFIndex		inIndex );
	ArrayValue			ExtractFirst();
	ArrayValue			ExtractLast();
	void				Insert(
							ArrayValue	inValue,
							CFIndex		inIndex );
	void				Prepend(
							ArrayValue	inValue );
	void				Remove(
							CFIndex		inIndex );
	void				RemoveAll();
	void				Set(
							ArrayValue	inValue,
							CFIndex		inIndex );
};

// -----------------------------------------------------------------------------
//	Create
// -----------------------------------------------------------------------------
//
inline CFArrayRef
CFArray::Create(
	CFArrayRef			inArrayToCopy )
{
	return ::CFArrayCreateCopy( kCFAllocatorDefault, inArrayToCopy );
}

// -----------------------------------------------------------------------------
//	Create
// -----------------------------------------------------------------------------
//
inline CFArrayRef
CFArray::Create(
	ArrayValue*				inValues,
	CFIndex					inCount,
	const CFArrayCallBacks*	inCallBacks )
{
	return ::CFArrayCreate( kCFAllocatorDefault, inValues, inCount, inCallBacks );
}

// -----------------------------------------------------------------------------
//	Contains
// -----------------------------------------------------------------------------
//
inline Boolean
CFArray::Contains(
	ArrayValue	inValue ) const
{
	return ::CFArrayContainsValue( GetArrayRef(), CFRangeMake( 0, GetCount() ), inValue );
}

// -----------------------------------------------------------------------------
//	Contains
// -----------------------------------------------------------------------------
//
inline Boolean
CFArray::Contains(
	ArrayValue		inValue,
	CFIndex			inIndex,
	CFIndex			inLength ) const
{
	return ::CFArrayContainsValue( GetArrayRef(), CFRangeMake( inIndex, inLength ), inValue );
}

// -----------------------------------------------------------------------------
//	Get
// -----------------------------------------------------------------------------
//
inline ArrayValue
CFArray::Get(
	CFIndex		inIndex ) const
{
	check( inIndex < GetCount() );
	return ::CFArrayGetValueAtIndex( GetArrayRef(), inIndex );
}

// -----------------------------------------------------------------------------
//	GetCount
// -----------------------------------------------------------------------------
//
inline CFIndex
CFArray::GetCount() const
{
	return ::CFArrayGetCount( GetArrayRef() );
}

// -----------------------------------------------------------------------------
//	GetCountOf
// -----------------------------------------------------------------------------
//
inline CFIndex
CFArray::GetCountOf(
	ArrayValue	inValue ) const
{
	return ::CFArrayGetCountOfValue( GetArrayRef(), CFRangeMake( 0, GetCount() ), inValue );
}

// -----------------------------------------------------------------------------
//	GetCountOf
// -----------------------------------------------------------------------------
//
inline CFIndex
CFArray::GetCountOf(
	ArrayValue		inValue,
	CFIndex			inIndex,
	CFIndex			inLength ) const
{
	return ::CFArrayGetCountOfValue( GetArrayRef(), CFRangeMake( inIndex, inLength ), inValue );
}

// -----------------------------------------------------------------------------
//	GetFirst
// -----------------------------------------------------------------------------
//
inline ArrayValue
CFArray::GetFirst() const
{
	return Get( 0 );
}

// -----------------------------------------------------------------------------
//	GetFirstIndexOf
// -----------------------------------------------------------------------------
//
inline CFIndex
CFArray::GetFirstIndexOf(
	ArrayValue	inValue ) const
{
	return ::CFArrayGetFirstIndexOfValue( GetArrayRef(), CFRangeMake( 0, GetCount() ), inValue );
}

// -----------------------------------------------------------------------------
//	GetFirstIndexOf
// -----------------------------------------------------------------------------
//
inline CFIndex
CFArray::GetFirstIndexOf(
	ArrayValue		inValue,
	CFIndex			inIndex,
	CFIndex			inLength ) const
{
	return ::CFArrayGetFirstIndexOfValue( GetArrayRef(), CFRangeMake( inIndex, inLength ), inValue );
}

// -----------------------------------------------------------------------------
//	GetLast
// -----------------------------------------------------------------------------
//
inline ArrayValue
CFArray::GetLast() const
{
	return Get( GetCount() - 1 );
}

// -----------------------------------------------------------------------------
//	GetLastIndexOf
// -----------------------------------------------------------------------------
//
inline CFIndex
CFArray::GetLastIndexOf(
	ArrayValue	inValue ) const
{
	return ::CFArrayGetLastIndexOfValue( GetArrayRef(), CFRangeMake( 0, GetCount() ), inValue );
}

// -----------------------------------------------------------------------------
//	GetLastIndexOf
// -----------------------------------------------------------------------------
//
inline CFIndex
CFArray::GetLastIndexOf(
	ArrayValue		inValue,
	CFIndex			inIndex,
	CFIndex			inLength ) const
{
	return ::CFArrayGetLastIndexOfValue( GetMutableArrayRef(), CFRangeMake( inIndex, inLength ), inValue );
}

// =============================================================================
//	CFMutableArray wrapper
// =============================================================================

// -----------------------------------------------------------------------------
//	CreateMutable
// -----------------------------------------------------------------------------
//
inline CFMutableArrayRef
CFArray::CreateMutable(
	CFArrayRef				inArrayToCopy,
	CFIndex					inCapacity )
{
	return ::CFArrayCreateMutableCopy( kCFAllocatorDefault, inCapacity, inArrayToCopy );
}

// -----------------------------------------------------------------------------
//	CreateMutable
// -----------------------------------------------------------------------------
//
inline CFMutableArrayRef
CFArray::CreateMutable(
	CFIndex					inCapacity,
	const CFArrayCallBacks* inCallBacks )
{
	return ::CFArrayCreateMutable( kCFAllocatorDefault, inCapacity, inCallBacks );
}

// -----------------------------------------------------------------------------
//	Append
// -----------------------------------------------------------------------------
//
inline void
CFArray::Append(
	ArrayValue	inValue )
{
	Set( inValue, GetCount() );
}

// -----------------------------------------------------------------------------
//	Exchange
// -----------------------------------------------------------------------------
//
inline void
CFArray::Exchange(
	CFIndex		inIndexA,
	CFIndex		inIndexB )
{
	::CFArrayExchangeValuesAtIndices( GetMutableArrayRef(), inIndexA, inIndexB );
}

// -----------------------------------------------------------------------------
//	Extract
// -----------------------------------------------------------------------------
//
inline ArrayValue
CFArray::Extract(
	CFIndex		inIndex )
{
	ArrayValue  value = Get( inIndex );
	::CFArrayRemoveValueAtIndex( GetMutableArrayRef(), inIndex );
	return value;
}

// -----------------------------------------------------------------------------
//	ExtractFirst
// -----------------------------------------------------------------------------
//
inline ArrayValue
CFArray::ExtractFirst()
{
	return Extract( 0 );
}

// -----------------------------------------------------------------------------
//	ExtractLast
// -----------------------------------------------------------------------------
//
inline ArrayValue
CFArray::ExtractLast()
{
	return Extract( GetCount() - 1 );
}

// -----------------------------------------------------------------------------
//	Insert
// -----------------------------------------------------------------------------
//
inline void
CFArray::Insert(
	ArrayValue	inValue,
	CFIndex		inIndex )
{
	::CFArrayInsertValueAtIndex( GetMutableArrayRef(), inIndex, inValue );
}

// -----------------------------------------------------------------------------
//	Prepend
// -----------------------------------------------------------------------------
//
inline void
CFArray::Prepend(
	ArrayValue	inValue )
{
	Insert( inValue, 0 );
}

// -----------------------------------------------------------------------------
//	Remove
// -----------------------------------------------------------------------------
//
inline void
CFArray::Remove(
	CFIndex		inIndex )
{
	::CFArrayRemoveValueAtIndex( GetMutableArrayRef(), inIndex );
}

// -----------------------------------------------------------------------------
//	RemoveAll
// -----------------------------------------------------------------------------
//
inline void
CFArray::RemoveAll()
{
	::CFArrayRemoveAllValues( GetMutableArrayRef() );
}

// -----------------------------------------------------------------------------
//	Set
// -----------------------------------------------------------------------------
//
inline void
CFArray::Set(
	ArrayValue	inValue,
	CFIndex		inIndex )
{
	::CFArraySetValueAtIndex( GetMutableArrayRef(), inIndex, inValue );
}

#endif // CFArrayMethods_H_
