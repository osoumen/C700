// =============================================================================
//	CFDataMethods.h
// =============================================================================
//

#ifndef CFDataMethods_H_
#define CFDataMethods_H_

#include "TCFType.h"

#define	CFData	__CFData

struct CFData
	:	public CFType
{
	static CFDataRef
					Create(
						UInt8*			inBytes,
						CFIndex			inLength );

	inline
	CFDataRef		GetDataRef() const
						{ return (CFDataRef) GetCFRef(); }
	operator		const UInt8*();

	void			GetBytes(
						CFIndex			inStart,
						CFIndex			inLength,	// Zero length means all of the data
						UInt8*			outBuffer ) const;
	const UInt8*	GetBytePtr() const;
	CFIndex			GetLength() const;

	// =========================================================================
	//	Mutable
	// =========================================================================

	static
	CFMutableDataRef
					CreateMutable(
						CFIndex					inCapacity = 0 );
	static
	CFMutableDataRef
					CreateMutable(
						CFDataRef				inDataToCopy,
						CFIndex					inCapacity );
	static
	CFMutableDataRef
					CreateMutable(
						UInt8*					inBytes,
						CFIndex					inLength,
						CFIndex					inCapacity = 0 );

	inline
	CFMutableDataRef
					GetMutableDataRef() const
						{ return (CFMutableDataRef) GetCFRef(); }
	operator		UInt8*();

	void			AppendBytes(
						const UInt8*	inBytes,
						CFIndex			inLength );
	UInt8*			GetMutableBytePtr() const;
	void			IncreaseLength(
						CFIndex			inExtraLength );
	void			RemoveBytes(
						CFIndex			inStart,
						CFIndex			inLength );	// Zero length means all of the data
	void			ReplaceBytes(
						CFIndex			inStart,
						CFIndex			inLength,	// Zero length means all of the data
						UInt8*			inNewBytes,
						CFIndex			inNewLength );
	void			SetLength(
						CFIndex			inLength );
};

// -----------------------------------------------------------------------------
//	Create
// -----------------------------------------------------------------------------
//
inline CFDataRef
CFData::Create(
	UInt8*			inBytes,
	CFIndex			inLength )
{
	return ::CFDataCreate( kCFAllocatorDefault, inBytes, inLength );
}

// -----------------------------------------------------------------------------
//	UInt8*
// -----------------------------------------------------------------------------
//
inline CFData::operator const UInt8*()
{
	return GetBytePtr();
}

// -----------------------------------------------------------------------------
//	GetBytes
// -----------------------------------------------------------------------------
//
inline void
CFData::GetBytes(
	CFIndex			inStart,
	CFIndex			inLength,
	UInt8*			outBuffer ) const
{
	// Zero length means all of the data
	if ( inLength == 0 )
		inLength = GetLength();
	return ::CFDataGetBytes( GetDataRef(), CFRangeMake( inStart, inLength ), outBuffer );
}

// -----------------------------------------------------------------------------
//	GetBytePtr
// -----------------------------------------------------------------------------
//
inline const UInt8*
CFData::GetBytePtr() const
{
	return ::CFDataGetBytePtr( GetDataRef() );
}

// -----------------------------------------------------------------------------
//	GetLength
// -----------------------------------------------------------------------------
//
inline CFIndex
CFData::GetLength() const
{
	return ::CFDataGetLength( GetDataRef() );
}

// -----------------------------------------------------------------------------
//	CreateMutable
// -----------------------------------------------------------------------------
//
inline CFMutableDataRef
CFData::CreateMutable(
	CFIndex					inCapacity )
{
	return ::CFDataCreateMutable( kCFAllocatorDefault, inCapacity );
}

// -----------------------------------------------------------------------------
//	CreateMutable
// -----------------------------------------------------------------------------
//
inline CFMutableDataRef
CFData::CreateMutable(
	CFDataRef				inDataToCopy,
	CFIndex					inCapacity )
{
	return ::CFDataCreateMutableCopy( kCFAllocatorDefault, inCapacity, inDataToCopy );
}

// -----------------------------------------------------------------------------
//	CreateMutable
// -----------------------------------------------------------------------------
//
inline CFMutableDataRef
CFData::CreateMutable(
	UInt8*					inBytes,
	CFIndex					inLength,
	CFIndex					inCapacity )
{
	CFMutableDataRef		newData;
	newData = ::CFDataCreateMutable( kCFAllocatorDefault, inCapacity );
	::CFDataAppendBytes( newData, inBytes, inLength );
	return newData;
}

// -----------------------------------------------------------------------------
//	UInt8*
// -----------------------------------------------------------------------------
//
inline CFData::operator UInt8*()
{
	return GetMutableBytePtr();
}

// -----------------------------------------------------------------------------
//	AppendBytes
// -----------------------------------------------------------------------------
//
inline void
CFData::AppendBytes(
	const UInt8*	inBytes,
	CFIndex			inLength )
{
	::CFDataAppendBytes( GetMutableDataRef(), inBytes, inLength );
}

// -----------------------------------------------------------------------------
//	GetMutableBytePtr
// -----------------------------------------------------------------------------
//
inline UInt8*
CFData::GetMutableBytePtr() const
{
	return ::CFDataGetMutableBytePtr( GetMutableDataRef() );
}

// -----------------------------------------------------------------------------
//	IncreaseLength
// -----------------------------------------------------------------------------
//
inline void
CFData::IncreaseLength(
	CFIndex			inExtraLength )
{
	::CFDataIncreaseLength( GetMutableDataRef(), inExtraLength );
}

// -----------------------------------------------------------------------------
//	RemoveBytes
// -----------------------------------------------------------------------------
//
inline void
CFData::RemoveBytes(
	CFIndex			inStart,
	CFIndex			inLength )
{
	// Zero length means all of the data
	if ( inLength == 0 )
		inLength = GetLength();
	::CFDataDeleteBytes( GetMutableDataRef(), CFRangeMake( inStart, inLength ) );
}

// -----------------------------------------------------------------------------
//	ReplaceBytes
// -----------------------------------------------------------------------------
//
inline void
CFData::ReplaceBytes(
	CFIndex			inStart,
	CFIndex			inLength,
	UInt8*			inNewBytes,
	CFIndex			inNewLength )
{
	// Zero length means all of the data
	if ( inLength == 0 )
		inLength = GetLength();
	::CFDataReplaceBytes( GetMutableDataRef(), CFRangeMake( inStart, inLength ), inNewBytes, inNewLength );
}

// -----------------------------------------------------------------------------
//	SetLength
// -----------------------------------------------------------------------------
//
inline void
CFData::SetLength(
	CFIndex			inLength )
{
	::CFDataSetLength( GetMutableDataRef(), inLength );
}

#endif // CFDataMethods_H_
