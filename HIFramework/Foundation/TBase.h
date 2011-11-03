// =============================================================================
//	TBase.h
// =============================================================================
//

#ifndef TBase_H_
#define TBase_H_

#include <CoreFoundation/CoreFoundation.h>

#include "HIFramework.h"

class TBase
{
public:
	virtual CFStringRef	CopyDescription() const = PURE_VIRTUAL;
	virtual CFIndex		GetRetainCount() const = PURE_VIRTUAL;
	virtual void		Release() = PURE_VIRTUAL;
	virtual void		Retain() = PURE_VIRTUAL;
};

#endif // TBase_H_