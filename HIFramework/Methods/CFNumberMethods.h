// =============================================================================
//	CFNumberMethods.h
// =============================================================================
//

#ifndef CFNumberMethods_H_
#define CFNumberMethods_H_

#include "CFTypeMethods.h"

typedef const void* NumberValue;

#define CFNumber	__CFNumber

struct CFNumber
	:	public CFType
{
	static CFNumberRef	Create(
							SInt8					inValue );
	static CFNumberRef	Create(
							SInt16					inValue );
	static CFNumberRef	Create(
							SInt32					inValue );
	static CFNumberRef	Create(
							SInt64					inValue );
	static CFNumberRef	Create(
							char					inValue );
	static CFNumberRef	Create(
							float					inValue );
	static CFNumberRef	Create(
							double					inValue );

	inline
	CFNumberRef			GetNumberRef() const
							{ return (CFNumberRef) GetCFRef(); }

	CFComparisonResult	Compare(
							CFNumberRef				inCompareTo,
							void*					inContext = NULL ) const;

	Boolean				Get(
							SInt8*					outValue ) const;
	Boolean				Get(
							SInt16*					outValue ) const;
	Boolean				Get(
							SInt32*					outValue ) const;
	Boolean				Get(
							SInt64*					outValue ) const;
	Boolean				Get(
							float*					outValue ) const;
	Boolean				Get(
							double*					outValue ) const;
	CFNumberType		GetType() const;
	CFIndex				GetSize() const;
	Boolean				IsFloatType() const;
};

// -----------------------------------------------------------------------------
//	Create
// -----------------------------------------------------------------------------
//
inline CFNumberRef
CFNumber::Create(
	SInt8					inValue )
{
	return ::CFNumberCreate( kCFAllocatorDefault, kCFNumberSInt8Type, &inValue );
}

// -----------------------------------------------------------------------------
//	Create
// -----------------------------------------------------------------------------
//
inline CFNumberRef
CFNumber::Create(
	SInt16					inValue )
{
	return ::CFNumberCreate( kCFAllocatorDefault, kCFNumberSInt16Type, &inValue );
}

// -----------------------------------------------------------------------------
//	Create
// -----------------------------------------------------------------------------
//
inline CFNumberRef
CFNumber::Create(
	SInt32					inValue )
{
	return ::CFNumberCreate( kCFAllocatorDefault, kCFNumberSInt32Type, &inValue );
}

// -----------------------------------------------------------------------------
//	Create
// -----------------------------------------------------------------------------
//
inline CFNumberRef
CFNumber::Create(
	SInt64					inValue )
{
	return ::CFNumberCreate( kCFAllocatorDefault, kCFNumberSInt64Type, &inValue );
}

// -----------------------------------------------------------------------------
//	Create
// -----------------------------------------------------------------------------
//
inline CFNumberRef
CFNumber::Create(
	char					inValue )
{
	return ::CFNumberCreate( kCFAllocatorDefault, kCFNumberCharType, &inValue );
}

// -----------------------------------------------------------------------------
//	Create
// -----------------------------------------------------------------------------
//
inline CFNumberRef
CFNumber::Create(
	float					inValue )
{
	return ::CFNumberCreate( kCFAllocatorDefault, kCFNumberFloatType, &inValue );
}

// -----------------------------------------------------------------------------
//	Create
// -----------------------------------------------------------------------------
//
inline CFNumberRef
CFNumber::Create(
	double					inValue )
{
	return ::CFNumberCreate( kCFAllocatorDefault, kCFNumberDoubleType, &inValue );
}

// -----------------------------------------------------------------------------
//	Compare
// -----------------------------------------------------------------------------
//
inline CFComparisonResult
CFNumber::Compare(
	CFNumberRef				inCompareTo,
	void*					inContext ) const
{
	return ::CFNumberCompare( GetNumberRef(), inCompareTo, inContext );
}

// -----------------------------------------------------------------------------
//	Get
// -----------------------------------------------------------------------------
//
inline Boolean
CFNumber::Get(
	SInt8*					outValue ) const
{
	return ::CFNumberGetValue( GetNumberRef(), kCFNumberSInt8Type, outValue );
}

// -----------------------------------------------------------------------------
//	Get
// -----------------------------------------------------------------------------
//
inline Boolean
CFNumber::Get(
	SInt16*					outValue ) const
{
	return ::CFNumberGetValue( GetNumberRef(), kCFNumberSInt16Type, outValue );
}

// -----------------------------------------------------------------------------
//	Get
// -----------------------------------------------------------------------------
//
inline Boolean
CFNumber::Get(
	SInt32*					outValue ) const
{
	return ::CFNumberGetValue( GetNumberRef(), kCFNumberSInt32Type, outValue );
}

// -----------------------------------------------------------------------------
//	Get
// -----------------------------------------------------------------------------
//
inline Boolean
CFNumber::Get(
	SInt64*					outValue ) const
{
	return ::CFNumberGetValue( GetNumberRef(), kCFNumberSInt64Type, outValue );
}

// -----------------------------------------------------------------------------
//	Get
// -----------------------------------------------------------------------------
//
inline Boolean
CFNumber::Get(
	float*					outValue ) const
{
	return ::CFNumberGetValue( GetNumberRef(), kCFNumberFloatType, outValue );
}

// -----------------------------------------------------------------------------
//	Get
// -----------------------------------------------------------------------------
//
inline Boolean
CFNumber::Get(
	double*					outValue ) const
{
	return ::CFNumberGetValue( GetNumberRef(), kCFNumberDoubleType, outValue );
}

// -----------------------------------------------------------------------------
//	GetType
// -----------------------------------------------------------------------------
//
inline CFNumberType
CFNumber::GetType() const
{
	return ::CFNumberGetType( GetNumberRef() );
}

// -----------------------------------------------------------------------------
//	GetSize
// -----------------------------------------------------------------------------
//
inline CFIndex
CFNumber::GetSize() const
{
	return ::CFNumberGetByteSize( GetNumberRef() );
}

// -----------------------------------------------------------------------------
//	IsFloatType
// -----------------------------------------------------------------------------
//
inline Boolean
CFNumber::IsFloatType() const
{
	return ::CFNumberIsFloatType( GetNumberRef() );
}

#endif // CFNumberMethods_H_
