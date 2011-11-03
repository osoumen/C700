// =============================================================================
//	TDictionary.h
// =============================================================================
//

#ifndef TDictionary_H_
#define TDictionary_H_

#include "TCFType.h"

typedef const void* DictionaryValue;
typedef const void*	DictionaryKey;

class TDictionary
	:	public TCFType
{
public:
					TDictionary();
					TDictionary(
						CFDictionaryRef				inDictionary,
						CFOwnership					inOwnership = kCFOwnershipTake );
					TDictionary(
						DictionaryKey*				inKeys,
						DictionaryValue*			inValues,
						CFIndex						inCount,
						const
						CFDictionaryKeyCallBacks*	inKeyCallbacks = &kCFTypeDictionaryKeyCallBacks,
						const
						CFDictionaryValueCallBacks*	inValueCallbacks = &kCFTypeDictionaryValueCallBacks );
	virtual			~TDictionary();

	operator		CFDictionaryRef() const
						{ return GetDictionaryRef(); }
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

private:
					// Disallow copy construction
					TDictionary( const TDictionary& );
	TDictionary&	operator =( const TDictionary& inTDictionary );
};

class TMutableDictionary
	:	public TDictionary
{
public:
					TMutableDictionary(
						CFDictionaryRef						inDictionary,
						CFIndex								inCapacity,
						CFOwnership							inOwnership );
					TMutableDictionary(
						CFIndex								inCapacity = 0,
						const CFDictionaryKeyCallBacks*		inKeyCallbacks = &kCFTypeDictionaryKeyCallBacks,
						const CFDictionaryValueCallBacks*	inValueCallbacks = &kCFTypeDictionaryValueCallBacks );
	virtual			~TMutableDictionary();

	operator		CFMutableDictionaryRef() const
						{ return GetMutableDictionaryRef(); }
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

private:
					// Disallow copy construction
					TMutableDictionary( const TMutableDictionary& );
	TMutableDictionary&
					operator =( const TMutableDictionary& inTMutableDictionary );
};

// -----------------------------------------------------------------------------
//	Contains
// -----------------------------------------------------------------------------
//
inline Boolean
TDictionary::Contains(
	DictionaryValue		inValue ) const
{
	return ::CFDictionaryContainsValue( GetDictionaryRef(), inValue );
}

// -----------------------------------------------------------------------------
//	ContainsKey
// -----------------------------------------------------------------------------
//
inline Boolean
TDictionary::ContainsKey(
	DictionaryKey		inKey ) const
{
	return ::CFDictionaryContainsKey( GetDictionaryRef(), inKey );
}

// -----------------------------------------------------------------------------
//	Get
// -----------------------------------------------------------------------------
//
inline DictionaryValue
TDictionary::Get(
	DictionaryKey		inKey ) const
{
	return ::CFDictionaryGetValue( GetDictionaryRef(), inKey );
}

// -----------------------------------------------------------------------------
//	GetCount
// -----------------------------------------------------------------------------
//
inline CFIndex
TDictionary::GetCount() const
{
	return ::CFDictionaryGetCount( GetDictionaryRef() );
}

// -----------------------------------------------------------------------------
//	GetCountOf
// -----------------------------------------------------------------------------
//
inline CFIndex
TDictionary::GetCountOf(
	DictionaryValue		inValue ) const
{
	return ::CFDictionaryGetCountOfValue( GetDictionaryRef(), inValue );
}

// -----------------------------------------------------------------------------
//	GetCountOfKey
// -----------------------------------------------------------------------------
//
inline CFIndex
TDictionary::GetCountOfKey(
	DictionaryKey		inKey ) const
{
	return ::CFDictionaryGetCountOfKey( GetDictionaryRef(), inKey );
}

// =============================================================================
//	MUTABLE
// =============================================================================

// -----------------------------------------------------------------------------
//	Extract
// -----------------------------------------------------------------------------
//
inline DictionaryValue
TMutableDictionary::Extract(
	DictionaryKey		inKey )
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
TMutableDictionary::RemoveAll()
{
	::CFDictionaryRemoveAllValues( GetMutableDictionaryRef() );
}

// -----------------------------------------------------------------------------
//	Remove
// -----------------------------------------------------------------------------
//
inline void
TMutableDictionary::Remove(
	DictionaryKey		inKey )
{
	::CFDictionaryRemoveValue( GetMutableDictionaryRef(), inKey );
}

// -----------------------------------------------------------------------------
//	Set
// -----------------------------------------------------------------------------
//
inline void
TMutableDictionary::Set(
	DictionaryKey		inKey,
	DictionaryValue		inValue )
{
	::CFDictionarySetValue( GetMutableDictionaryRef(), inKey, inValue );
}

#endif // TDictionary_H_
