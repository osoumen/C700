// =============================================================================
//	TData.h
// =============================================================================
//

#ifndef TData_H_
#define TData_H_

#include "TCFType.h"

class TData
	:	public TCFType
{
public:
					TData();
					TData(
						CFDataRef		inData,
						CFOwnership		inOwnership = kCFOwnershipTake );
					TData(
						UInt8*			inBytes,
						CFIndex			inLength );
	virtual			~TData();

	operator		CFDataRef() const
						{ return GetDataRef(); }
	inline
	CFDataRef		GetDataRef() const
						{ return (CFDataRef) GetCFRef(); }
	operator		const UInt8*() const
						{ return GetBytePtr(); }

	void			GetBytes(
						CFIndex			inStart,
						CFIndex			inLength,	// Zero length means all of the data
						UInt8*			outBuffer ) const;
	const UInt8*	GetBytePtr() const;
	CFIndex			GetLength() const;

private:
					// Disallow copy construction
					TData( const TData& );
	TData&			operator =( const TData& inTData );
};

class TMutableData
	:	public TData
{
public:
					TMutableData(
						CFIndex				inCapacity = 0 );
					TMutableData(
						CFDataRef			inData,
						CFIndex				inCapacity,
						CFOwnership			inOwnership );
					TMutableData(
						UInt8*				inBytes,
						CFIndex				inLength,
						CFIndex				inCapacity = 0);
	virtual			~TMutableData();

	operator		CFMutableDataRef() const
						{ return GetMutableDataRef(); }
	inline
	CFMutableDataRef
					GetMutableDataRef() const
						{ return (CFMutableDataRef) GetCFRef(); }
	operator		UInt8*()
						{ return GetMutableBytePtr(); }

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

private:
					// Disallow copy construction
					TMutableData( const TMutableData& );
	TMutableData&	operator =( const TMutableData& inTMutableData );
};

// -----------------------------------------------------------------------------
//	GetBytes
// -----------------------------------------------------------------------------
//
inline void
TData::GetBytes(
	CFIndex			inStart,
	CFIndex			inLength,
	UInt8*			outBuffer ) const
{
	// Zero length means all of the data
	if ( inLength == 0 )
		inLength = GetLength();
	::CFDataGetBytes( GetDataRef(), CFRangeMake( inStart, inLength ), outBuffer );
}

// -----------------------------------------------------------------------------
//	GetBytePtr
// -----------------------------------------------------------------------------
//
inline const UInt8*
TData::GetBytePtr() const
{
	return ::CFDataGetBytePtr( GetDataRef() );
}

// -----------------------------------------------------------------------------
//	GetLength
// -----------------------------------------------------------------------------
//
inline CFIndex
TData::GetLength() const
{
	return ::CFDataGetLength( GetDataRef() );
}

// =============================================================================
//	MUTABLE
// =============================================================================

// -----------------------------------------------------------------------------
//	AppendBytes
// -----------------------------------------------------------------------------
//
inline void
TMutableData::AppendBytes(
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
TMutableData::GetMutableBytePtr() const
{
	return ::CFDataGetMutableBytePtr( GetMutableDataRef() );
}

// -----------------------------------------------------------------------------
//	IncreaseLength
// -----------------------------------------------------------------------------
//
inline void
TMutableData::IncreaseLength(
	CFIndex			inExtraLength )
{
	::CFDataIncreaseLength( GetMutableDataRef(), inExtraLength );
}

// -----------------------------------------------------------------------------
//	RemoveBytes
// -----------------------------------------------------------------------------
//
inline void
TMutableData::RemoveBytes(
	CFIndex			inStart,
	CFIndex			inLength  )
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
TMutableData::ReplaceBytes(
	CFIndex			inStart,
	CFIndex			inLength,
	UInt8*			inNewBytes,
	CFIndex			inNewLength  )
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
TMutableData::SetLength(
	CFIndex			inLength )
{
	::CFDataSetLength( GetMutableDataRef(), inLength );
}

#endif // TData_H_
