/*
 *  TUnicodeTextStorage.h
 *  Ritmo
 *
 *  Created by Chris Reed on Sun Mar 02 2003.
 *  Copyright (c) 2003 Chris Reed. All rights reserved.
 *
 */
#if !defined(_TUnicodeTextStorage_h_)
#define _TUnicodeTextStorage_h_

#include <Carbon/Carbon.h>

/*!
 * \brief Manages storage for dynamic array of unicode characters.
 */
class TUnicodeTextStorage
{
public:
	//! Default constructor;
	TUnicodeTextStorage();
	virtual ~TUnicodeTextStorage();
	
	bool ReserveSpace(uint32_t totalCount);
	
	//! Returns the number of characters actually in the buffer.
	unsigned Count() const { return mCount; }
	
	//! \name Setters
	//@{
	void SetString(CFStringRef text);
	void SetText(const UniChar* text, uint32_t count);
	//@}
	
	//! \name Getters
	//@{
	CFStringRef GetString();
//	UniChar* GetText() { return mStorage; }
	const UniChar* GetText() { return mStorage; }
	//@}
	
	//! \name Operators
	//@{
	TUnicodeTextStorage& operator = (CFStringRef text) { SetString(text); return *this; }
	operator CFStringRef () { return GetString(); }
	
	operator UniChar* () { return mStorage; }
	operator const UniChar* () { return mStorage; }
	UniChar& operator [] (int index) { return mStorage[index]; }
	//@}
	
	//! \name Edit operations
	//@{
	void Delete(uint32_t start, uint32_t count=1);
	
	void InsertString(uint32_t position, CFStringRef text);
	void InsertText(uint32_t position, const UniChar* text, uint32_t count);
	
	void AppendString(CFStringRef text) { InsertString(mCount, text); }
	void AppendText(const UniChar* text, uint32_t count) { InsertText(mCount, text, count); }
	//@}

protected:
	UniChar* mStorage;	//!< The buffer where characters are stored.
	uint32_t mCount;	//!< Actual number of characters in storage buffer.
	uint32_t mSize;	//!< Total number of characters that have been allocated.
	
	void Allocate(uint32_t characters);
};

#endif // _TUnicodeTextStorage_h_
