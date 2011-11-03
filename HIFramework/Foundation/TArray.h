// =============================================================================
//	TArray.h
// =============================================================================
//

#ifndef TArray_H_
#define TArray_H_

#include "TCFType.h"

typedef const void* ArrayValue;

#define kNoArrayCallbacks	((CFArrayCallBacks*) NULL)

class TArray
	:	public TCFType
{
public:
						TArray();
						TArray(
							CFArrayRef				inArray,
							CFOwnership				inOwnership );
						TArray(
							ArrayValue*				inValues,
							CFIndex					inCount,
							const CFArrayCallBacks*	inCallBacks = &kCFTypeArrayCallBacks );
	virtual				~TArray();

	operator			CFArrayRef() const
							{ return GetArrayRef(); }
	inline
	CFArrayRef			GetArrayRef() const
							{ return (CFArrayRef) GetCFRef(); };

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

private:
						// Disallow copy construction
						TArray( const TArray& );
	TArray&				operator =( const TArray& inTArray );
};

class TMutableArray
	:	public TArray
{
public:
						TMutableArray(
							CFMutableArrayRef		inArray,
							CFIndex					inCapacity,
							CFOwnership				inOwnership );
						TMutableArray(
							CFIndex					inCapacity = 0,
							const CFArrayCallBacks* inCallBacks = &kCFTypeArrayCallBacks );
						TMutableArray(
							ArrayValue*				inValues,
							CFIndex					inCount,
							CFIndex					inCapacity,
							const CFArrayCallBacks*	inCallBacks = &kCFTypeArrayCallBacks );
	virtual				~TMutableArray();

	operator			CFMutableArrayRef() const
							{ return GetMutableArrayRef(); }
	inline
	CFMutableArrayRef	GetMutableArrayRef() const
							{ return (CFMutableArrayRef) GetCFRef(); }

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

private:
						// Disallow copy construction
						TMutableArray( const TMutableArray& );
	TMutableArray&		operator =( const TMutableArray& inTMutableArray );
};

// -----------------------------------------------------------------------------
//	Contains
// -----------------------------------------------------------------------------
//
inline Boolean
TArray::Contains(
	ArrayValue		inValue ) const
{
	return ::CFArrayContainsValue( GetArrayRef(), CFRangeMake( 0, GetCount() ), inValue );
}

// -----------------------------------------------------------------------------
//	Contains
// -----------------------------------------------------------------------------
//
inline Boolean
TArray::Contains(
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
TArray::Get(
	CFIndex			inIndex ) const
{
	check( inIndex < GetCount() );
	return ::CFArrayGetValueAtIndex( GetArrayRef(), inIndex );
}

// -----------------------------------------------------------------------------
//	GetCount
// -----------------------------------------------------------------------------
//
inline CFIndex
TArray::GetCount() const
{
	return ::CFArrayGetCount( GetArrayRef() );
}

// -----------------------------------------------------------------------------
//	GetCountOf
// -----------------------------------------------------------------------------
//
inline CFIndex
TArray::GetCountOf(
	ArrayValue		inValue ) const
{
	return ::CFArrayGetCountOfValue( GetArrayRef(), CFRangeMake( 0, GetCount() ), inValue );
}

// -----------------------------------------------------------------------------
//	GetCountOf
// -----------------------------------------------------------------------------
//
inline CFIndex
TArray::GetCountOf(
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
TArray::GetFirst() const
{
	return Get( 0 );
}

// -----------------------------------------------------------------------------
//	GetFirstIndexOf
// -----------------------------------------------------------------------------
//
inline CFIndex
TArray::GetFirstIndexOf(
	ArrayValue		inValue ) const
{
	return ::CFArrayGetFirstIndexOfValue( GetArrayRef(), CFRangeMake( 0, GetCount() ), inValue );
}

// -----------------------------------------------------------------------------
//	GetFirstIndexOf
// -----------------------------------------------------------------------------
//
inline CFIndex
TArray::GetFirstIndexOf(
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
TArray::GetLast() const
{
	return Get( GetCount() - 1 );
}

// -----------------------------------------------------------------------------
//	GetLastIndexOf
// -----------------------------------------------------------------------------
//
inline CFIndex
TArray::GetLastIndexOf(
	ArrayValue		inValue ) const
{
	return ::CFArrayGetLastIndexOfValue( GetArrayRef(), CFRangeMake( 0, GetCount() ), inValue );
}

// -----------------------------------------------------------------------------
//	GetLastIndexOf
// -----------------------------------------------------------------------------
//
inline CFIndex
TArray::GetLastIndexOf(
	ArrayValue		inValue,
	CFIndex			inIndex,
	CFIndex			inLength ) const
{
	return ::CFArrayGetLastIndexOfValue( GetArrayRef(), CFRangeMake( inIndex, inLength ), inValue );
}

// =============================================================================
//	TMutableArray
// =============================================================================

// -----------------------------------------------------------------------------
//	Append
// -----------------------------------------------------------------------------
//
inline void
TMutableArray::Append(
	ArrayValue		inValue )
{
	Set( inValue, GetCount() );
}

// -----------------------------------------------------------------------------
//	Exchange
// -----------------------------------------------------------------------------
//
inline void
TMutableArray::Exchange(
	CFIndex			inIndexA,
	CFIndex			inIndexB )
{
	::CFArrayExchangeValuesAtIndices( GetMutableArrayRef(), inIndexA, inIndexB );
}

// -----------------------------------------------------------------------------
//	Extract
// -----------------------------------------------------------------------------
//
inline ArrayValue
TMutableArray::Extract(
	CFIndex			inIndex )
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
TMutableArray::ExtractFirst()
{
	return Extract( 0 );
}

// -----------------------------------------------------------------------------
//	ExtractLast
// -----------------------------------------------------------------------------
//
inline ArrayValue
TMutableArray::ExtractLast()
{
	return Extract( GetCount() - 1 );
}

// -----------------------------------------------------------------------------
//	Insert
// -----------------------------------------------------------------------------
//
inline void
TMutableArray::Insert(
	ArrayValue		inValue,
	CFIndex			inIndex )
{
	::CFArrayInsertValueAtIndex( GetMutableArrayRef(), inIndex, inValue );
}

// -----------------------------------------------------------------------------
//	Prepend
// -----------------------------------------------------------------------------
//
inline void
TMutableArray::Prepend(
	ArrayValue		inValue )
{
	Insert( inValue, 0 );
}

// -----------------------------------------------------------------------------
//	Remove
// -----------------------------------------------------------------------------
//
inline void
TMutableArray::Remove(
	CFIndex			inIndex )
{
	::CFArrayRemoveValueAtIndex( GetMutableArrayRef(), inIndex );
}

// -----------------------------------------------------------------------------
//	RemoveAll
// -----------------------------------------------------------------------------
//
inline void
TMutableArray::RemoveAll()
{
	::CFArrayRemoveAllValues( GetMutableArrayRef() );
}

// -----------------------------------------------------------------------------
//	Set
// -----------------------------------------------------------------------------
//
inline void
TMutableArray::Set(
	ArrayValue		inValue,
	CFIndex			inIndex )
{
	::CFArraySetValueAtIndex( GetMutableArrayRef(), inIndex, inValue );
}

#endif // TArray_H_
