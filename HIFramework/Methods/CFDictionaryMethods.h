// =============================================================================
//	CFDictionaryMethods.h
// =============================================================================
//

#ifndef CFDictionaryMethods_H_
#define CFDictionaryMethods_H_

#include "CFTypeMethods.h"

typedef const void* DictionaryValue;
typedef const void*	DictionaryKey;

#define CFDictionary	__CFDictionary

class CFDictionary
	:	public CFType
{
public:
	static
	CFDictionaryRef	Create(
						DictionaryKey*						inKeys,
						DictionaryValue*					inValues,
						CFIndex								inCount,
						const CFDictionaryKeyCallBacks*		inKeyCallBacks = &kCFTypeDictionaryKeyCallBacks,
						const CFDictionaryValueCallBacks*	inValueCallBacks = &kCFTypeDictionaryValueCallBacks );

	inline
	CFDictionaryRef GetDictionaryRef() const
						{ return (CFDictionaryRef) GetCFRef(); }

	Boolean			Contains(
						DictionaryValue	inValue ) const;
	Boolean			ContainsKey(
						DictionaryKey	inKey ) const;
	CFIndex			GetCount() const;
	CFIndex			GetCountOfKey(
						DictionaryKey	inKey ) const;
	CFIndex			GetCountOf(
						DictionaryValue	inValue ) const;
	DictionaryValue	Get(
						DictionaryKey	inKey ) const;

	// =========================================================================
	//	Mutable
	// =========================================================================

	static
	CFMutableDictionaryRef
					CreateMutable(
						CFIndex								inCapacity = 0,
						const CFDictionaryKeyCallBacks*		inKeyCallBacks = &kCFTypeDictionaryKeyCallBacks,
						const CFDictionaryValueCallBacks*	inValueCallBacks = &kCFTypeDictionaryValueCallBacks );
	static
	CFMutableDictionaryRef
					CreateMutable(
						CFDictionaryRef						inDictionaryToCopy,
						CFIndex								inCapacity = 0 );

	inline
	CFMutableDictionaryRef
					GetMutableDictionaryRef() const
						{ return (CFMutableDictionaryRef) GetCFRef(); }

	DictionaryValue	Extract(
						DictionaryKey	inKey );
	void			RemoveAll();
	void			Remove(
						DictionaryKey	inKey );
	void			Set(
						DictionaryKey	inKey,
						DictionaryValue	inValue );
};

// -----------------------------------------------------------------------------
//	Create
// -----------------------------------------------------------------------------
//
inline CFDictionaryRef
CFDictionary::Create(
	DictionaryKey*						inKeys,
	DictionaryValue*					inValues,
	CFIndex								inCount,
	const CFDictionaryKeyCallBacks*		inKeyCallBacks,
	const CFDictionaryValueCallBacks*	inValueCallBacks )
{
	return ::CFDictionaryCreate( kCFAllocatorDefault, inKeys, inValues, inCount, inKeyCallBacks, inValueCallBacks );
}

// -----------------------------------------------------------------------------
//	Contains
// -----------------------------------------------------------------------------
//
inline Boolean
CFDictionary::Contains(
	DictionaryValue	inValue ) const
{
	return ::CFDictionaryContainsValue( GetDictionaryRef(), inValue );
}

// -----------------------------------------------------------------------------
//	ContainsKey
// -----------------------------------------------------------------------------
//
inline Boolean
CFDictionary::ContainsKey(
	DictionaryKey	inKey ) const
{
	return ::CFDictionaryContainsKey( GetDictionaryRef(), inKey );
}

// -----------------------------------------------------------------------------
//	Get
// -----------------------------------------------------------------------------
//
inline DictionaryValue
CFDictionary::Get(
	DictionaryKey	inKey ) const
{
	return ::CFDictionaryGetValue( GetDictionaryRef(), inKey );
}

// -----------------------------------------------------------------------------
//	GetCount
// -----------------------------------------------------------------------------
//
inline CFIndex
CFDictionary::GetCount() const
{
	return ::CFDictionaryGetCount( GetDictionaryRef() );
}

// -----------------------------------------------------------------------------
//	GetCountOf
// -----------------------------------------------------------------------------
//
inline CFIndex
CFDictionary::GetCountOf(
	DictionaryValue	inValue ) const
{
	return ::CFDictionaryGetCountOfValue( GetDictionaryRef(), inValue );
}

// -----------------------------------------------------------------------------
//	GetCountOfKey
// -----------------------------------------------------------------------------
//
inline CFIndex
CFDictionary::GetCountOfKey(
	DictionaryKey	inKey ) const
{
	return ::CFDictionaryGetCountOfKey( GetDictionaryRef(), inKey );
}

// -----------------------------------------------------------------------------
//	CreateMutable
// -----------------------------------------------------------------------------
//
inline CFMutableDictionaryRef
CFDictionary::CreateMutable(
	CFIndex								inCapacity,
	const CFDictionaryKeyCallBacks*		inKeyCallBacks,
	const CFDictionaryValueCallBacks*	inValueCallBacks )
{
	return ::CFDictionaryCreateMutable( kCFAllocatorDefault, inCapacity, inKeyCallBacks, inValueCallBacks );
}

// -----------------------------------------------------------------------------
//	CreateMutable
// -----------------------------------------------------------------------------
//
inline CFMutableDictionaryRef
CFDictionary::CreateMutable(
	CFDictionaryRef						inDictionaryToCopy,
	CFIndex								inCapacity )
{
	return ::CFDictionaryCreateMutableCopy( kCFAllocatorDefault, inCapacity, inDictionaryToCopy );
}

// -----------------------------------------------------------------------------
//	Extract
// -----------------------------------------------------------------------------
//
inline DictionaryValue
CFDictionary::Extract(
	DictionaryKey	inKey )
{
	DictionaryValue  value = Get( inKey );
	Remove( inKey );
	return value;
}

// -----------------------------------------------------------------------------
//	RemoveAll
// -----------------------------------------------------------------------------
//
inline void
CFDictionary::RemoveAll()
{
	::CFDictionaryRemoveAllValues( GetMutableDictionaryRef() );
}

// -----------------------------------------------------------------------------
//	Remove
// -----------------------------------------------------------------------------
//
inline void
CFDictionary::Remove(
	DictionaryKey	inKey )
{
	::CFDictionaryRemoveValue( GetMutableDictionaryRef(), inKey );
}

// -----------------------------------------------------------------------------
//	Set
// -----------------------------------------------------------------------------
//
inline void
CFDictionary::Set(
	DictionaryKey	inKey,
	DictionaryValue	inValue )
{
	::CFDictionarySetValue( GetMutableDictionaryRef(), inKey, inValue );
}

#endif // CFDictionaryMethods_H_
