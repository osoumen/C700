// =============================================================================
//	TStream.h
// =============================================================================
//

#ifndef TStream_H_
#define TStream_H_

#include "TCFType.h"

class TStream
	: public TCFType
{
public:
	virtual void		Close() const = PURE_VIRTUAL;
	virtual CFTypeRef	CopyProperty(
							CFStringRef				inPropertyName ) const = PURE_VIRTUAL;
	virtual
	CFStreamError		GetError() const = PURE_VIRTUAL;
	virtual
	CFStreamStatus		GetStatus() const = PURE_VIRTUAL;
	virtual Boolean		Open() const = PURE_VIRTUAL;
	virtual Boolean		SetProperty(
							CFStringRef				inPropertyName,
							CFTypeRef				inProperty ) = PURE_VIRTUAL;
};

class TReadStream
	:	public TStream
{
public:
					TReadStream();
					TReadStream(
						UInt8*					inBytes,
						CFIndex					inLength );
					TReadStream(
						CFURLRef				inURL );
	virtual			~TReadStream();

	operator		CFReadStreamRef() const
						{ return GetReadStreamRef(); }
	inline
	CFReadStreamRef	GetReadStreamRef() const
						{ return (CFReadStreamRef) GetCFRef(); }

	void			Close() const;
	CFTypeRef		CopyProperty(
						CFStringRef				inPropertyName ) const;
	const UInt8*	GetBuffer(
						CFIndex					inMaxToRead,
						CFIndex*				outAmountRead ) const;
	CFStreamError	GetError() const;
	CFStreamStatus	GetStatus() const;
	Boolean			HasBytesAvailable() const;
	Boolean			Open() const;
	CFIndex			Read(
						UInt8*					inBuffer,
						CFIndex					inLenfth );
	Boolean			SetProperty(
						CFStringRef				inPropertyName,
						CFTypeRef				inProperty );
	
private:
					// Disallow copy construction
					TReadStream( const TReadStream& );
	TReadStream&	operator =( const TReadStream& inTReadStream );
};

class TWriteStream
	:	public TStream
{
public:
					TWriteStream();
					TWriteStream(
						UInt8*					inBuffer,
						CFIndex					inLength );
					TWriteStream(
						CFURLRef				inURL );
	virtual			~TWriteStream();

	operator		CFWriteStreamRef() const
						{ return GetWriteStreamRef(); }
	inline
	CFWriteStreamRef
					GetWriteStreamRef() const
						{ return (CFWriteStreamRef) GetCFRef(); }

	Boolean			CanAcceptBytes() const;
	void			Close() const;
	CFTypeRef		CopyProperty(
						CFStringRef				inPropertyName ) const;
	CFStreamError	GetError() const;
	CFStreamStatus	GetStatus() const;
	Boolean			Open() const;
	Boolean			SetProperty(
						CFStringRef				inPropertyName,
						CFTypeRef				inProperty );
	CFIndex			Write(
						const UInt8*			inBuffer,
						CFIndex					inLength );

private:
					// Disallow copy construction
					TWriteStream( const TWriteStream& );
	TWriteStream&	operator =( const TWriteStream& inTWriteStream );
};

// -----------------------------------------------------------------------------
//	Close
// -----------------------------------------------------------------------------
//
inline void
TReadStream::Close() const
{
	::CFReadStreamClose( GetReadStreamRef() );
}

// -----------------------------------------------------------------------------
//	CopyProperty
// -----------------------------------------------------------------------------
//
inline CFTypeRef
TReadStream::CopyProperty(
	CFStringRef				inPropertyName ) const
{
	return ::CFReadStreamCopyProperty( GetReadStreamRef(), inPropertyName );
}

// -----------------------------------------------------------------------------
//	GetBuffer
// -----------------------------------------------------------------------------
//
inline const UInt8*
TReadStream::GetBuffer(
	CFIndex					inMaxToRead,
	CFIndex*				outAmountRead ) const
{
	return ::CFReadStreamGetBuffer( GetReadStreamRef(), inMaxToRead, outAmountRead );
}

// -----------------------------------------------------------------------------
//	GetError
// -----------------------------------------------------------------------------
//
inline CFStreamError
TReadStream::GetError() const
{
	return ::CFReadStreamGetError( GetReadStreamRef() );
}

// -----------------------------------------------------------------------------
//	GetStatus
// -----------------------------------------------------------------------------
//
inline CFStreamStatus
TReadStream::GetStatus() const
{
	return ::CFReadStreamGetStatus( GetReadStreamRef() );
}

// -----------------------------------------------------------------------------
//	HasBytesAvailable
// -----------------------------------------------------------------------------
//
inline Boolean
TReadStream::HasBytesAvailable() const
{
	return ::CFReadStreamHasBytesAvailable( GetReadStreamRef() );
}

// -----------------------------------------------------------------------------
//	Open
// -----------------------------------------------------------------------------
//
inline Boolean
TReadStream::Open() const
{
	return ::CFReadStreamOpen( GetReadStreamRef() );
}

// -----------------------------------------------------------------------------
//	Read
// -----------------------------------------------------------------------------
//
inline CFIndex
TReadStream::Read(
	UInt8*					inBuffer,
	CFIndex					inLength )
{
	return ::CFReadStreamRead( GetReadStreamRef(), inBuffer, inLength );
}

// -----------------------------------------------------------------------------
//	SetProperty
// -----------------------------------------------------------------------------
//
inline Boolean
TReadStream::SetProperty(
	CFStringRef				inPropertyName,
	CFTypeRef				inProperty )
{
	return ::CFReadStreamSetProperty( GetReadStreamRef(), inPropertyName, inProperty );
}

// =============================================================================
//	WRITE
// =============================================================================

// -----------------------------------------------------------------------------
//	CanAcceptBytes
// -----------------------------------------------------------------------------
//
inline Boolean
TWriteStream::CanAcceptBytes() const
{
	return ::CFWriteStreamCanAcceptBytes( GetWriteStreamRef() );
}

// -----------------------------------------------------------------------------
//	Close
// -----------------------------------------------------------------------------
//
inline void
TWriteStream::Close() const
{
	::CFWriteStreamClose( GetWriteStreamRef() );
}

// -----------------------------------------------------------------------------
//	CopyProperty
// -----------------------------------------------------------------------------
//
inline CFTypeRef
TWriteStream::CopyProperty(
	CFStringRef				inPropertyName ) const
{
	return ::CFWriteStreamCopyProperty( GetWriteStreamRef(), inPropertyName );
}

// -----------------------------------------------------------------------------
//	GetError
// -----------------------------------------------------------------------------
//
inline CFStreamError
TWriteStream::GetError() const
{
	return ::CFWriteStreamGetError( GetWriteStreamRef() );
}

// -----------------------------------------------------------------------------
//	GetStatus
// -----------------------------------------------------------------------------
//
inline CFStreamStatus
TWriteStream::GetStatus() const
{
	return ::CFWriteStreamGetStatus( GetWriteStreamRef() );
}

// -----------------------------------------------------------------------------
//	Open
// -----------------------------------------------------------------------------
//
inline Boolean
TWriteStream::Open() const
{
	return ::CFWriteStreamOpen( GetWriteStreamRef() );
}

// -----------------------------------------------------------------------------
//	Write
// -----------------------------------------------------------------------------
//
inline CFIndex
TWriteStream::Write(
	const UInt8*			inBuffer,
	CFIndex					inLength )
{
	return ::CFWriteStreamWrite( GetWriteStreamRef(), inBuffer, inLength );
}

// -----------------------------------------------------------------------------
//	SetProperty
// -----------------------------------------------------------------------------
//
inline Boolean
TWriteStream::SetProperty(
	CFStringRef				inPropertyName,
	CFTypeRef				inProperty )
{
	return ::CFWriteStreamSetProperty( GetWriteStreamRef(), inPropertyName, inProperty );
}

#endif // TStream_H_
