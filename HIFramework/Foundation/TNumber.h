// =============================================================================
//	TNumber.h
// =============================================================================
//

#ifndef TNumber_H_
#define TNumber_H_

#include "TCFType.h"

typedef const void* NumberValue;

class TNumber
	:	public TCFType
{
public:
						TNumber(
							CFNumberRef				inNumber,
							CFOwnership				inOwnership = kCFOwnershipTake );
						TNumber(
							SInt8					inValue );
						TNumber(
							SInt16					inValue );
						TNumber(
							SInt32					inValue );
						TNumber(
							SInt64					inValue );
						TNumber(
							char					inValue );
						TNumber(
							float					inValue );
						TNumber(
							double					inValue );
	virtual				~TNumber();

	operator			CFNumberRef() const
							{ return GetNumberRef(); }
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

private:
						// Disallow copy construction
						TNumber( const TNumber& );
	TNumber&			operator =( const TNumber& inTNumber );
};

// -----------------------------------------------------------------------------
//	Compare
// -----------------------------------------------------------------------------
//
inline CFComparisonResult
TNumber::Compare(
	CFNumberRef		inCompareTo,
	void*			inContext ) const
{
	return ::CFNumberCompare( GetNumberRef(), inCompareTo, inContext );
}

// -----------------------------------------------------------------------------
//	Get
// -----------------------------------------------------------------------------
//
inline Boolean
TNumber::Get(
	SInt8*					outValue ) const
{
	return ::CFNumberGetValue( GetNumberRef(), kCFNumberSInt8Type, outValue );
}

// -----------------------------------------------------------------------------
//	Get
// -----------------------------------------------------------------------------
//
inline Boolean
TNumber::Get(
	SInt16*					outValue ) const
{
	return ::CFNumberGetValue( GetNumberRef(), kCFNumberSInt16Type, outValue );
}

// -----------------------------------------------------------------------------
//	Get
// -----------------------------------------------------------------------------
//
inline Boolean
TNumber::Get(
	SInt32*					outValue ) const
{
	return ::CFNumberGetValue( GetNumberRef(), kCFNumberSInt32Type, outValue );
}

// -----------------------------------------------------------------------------
//	Get
// -----------------------------------------------------------------------------
//
inline Boolean
TNumber::Get(
	SInt64*					outValue ) const
{
	return ::CFNumberGetValue( GetNumberRef(), kCFNumberSInt64Type, outValue );
}

// -----------------------------------------------------------------------------
//	Get
// -----------------------------------------------------------------------------
//
inline Boolean
TNumber::Get(
	float*					outValue ) const
{
	return ::CFNumberGetValue( GetNumberRef(), kCFNumberFloatType, outValue );
}

// -----------------------------------------------------------------------------
//	Get
// -----------------------------------------------------------------------------
//
inline Boolean
TNumber::Get(
	double*					outValue ) const
{
	return ::CFNumberGetValue( GetNumberRef(), kCFNumberDoubleType, outValue );
}

// -----------------------------------------------------------------------------
//	GetType
// -----------------------------------------------------------------------------
//
inline CFNumberType
TNumber::GetType() const
{
	return ::CFNumberGetType( GetNumberRef() );
}

// -----------------------------------------------------------------------------
//	GetSize
// -----------------------------------------------------------------------------
//
inline CFIndex
TNumber::GetSize() const
{
	return ::CFNumberGetByteSize( GetNumberRef() );
}

// -----------------------------------------------------------------------------
//	IsFloatType
// -----------------------------------------------------------------------------
//
inline Boolean
TNumber::IsFloatType() const
{
	return ::CFNumberIsFloatType( GetNumberRef() );
}

#endif // TNumber_H_
