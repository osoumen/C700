// =============================================================================
//	TType.h
// =============================================================================
//

#ifndef TType_H_
#define TType_H_

#include "TBase.h"

class TType
	:	public TBase
{
public:
				TType();
	virtual		~TType();

	CFStringRef	CopyDescription() const;
	CFIndex		GetRetainCount() const;
	void		Release();
	void		Retain();

private:
	CFIndex		fRetainCount;

				// Disallow copy construction
				TType( const TType& );
	TType&		operator =( const TType& inTType );
};

#endif // TType_H_