/*
 *  TUnicodeTextStorage.cpp
 *  Ritmo
 *
 *  Created by Chris Reed on Sun Mar 02 2003.
 *  Copyright (c) 2003 Chris Reed. All rights reserved.
 *
 */

#include "TUnicodeTextStorage.h"

TUnicodeTextStorage::TUnicodeTextStorage()
	: mStorage(0), mCount(0), mSize(0)
{
}

//! Disposes of the text storage buffer.
TUnicodeTextStorage::~TUnicodeTextStorage()
{
	if (mStorage)
		free(mStorage);
}

//! Sets the characters in the buffer. Writes over any previous buffer
//! contents; no data is preserved.
void TUnicodeTextStorage::SetString(CFStringRef text)
{
	// get the unicode characters from the string
	CFIndex count = CFStringGetLength(text);
	if (!ReserveSpace(count))
		return;
	
	CFStringGetCharacters(text, CFRangeMake(0, count), mStorage);
	mCount = count;
}

//! Sets 
void TUnicodeTextStorage::SetText(const UniChar* text, uint32_t count)
{
	if (!ReserveSpace(count))
		return;
	memcpy(mStorage, text, count * sizeof(UniChar));
	mCount = count;
}

CFStringRef TUnicodeTextStorage::GetString()
{
	return CFStringCreateWithCharacters(kCFAllocatorDefault, mStorage, mCount);
}

//! If this routine returns true, then you can be assured that there
//! is room in the buffer for \c totalCount number of characters. The
//! previous contents of the buffer will be preserved. If \c totalCount
//! is smaller than the current number of characters in the buffer,
//! the contents will be truncated.
bool TUnicodeTextStorage::ReserveSpace(uint32_t totalCount)
{
	if (mSize >= totalCount)
		return true;
	
	Allocate(totalCount);
	
	return mSize == totalCount;
}

//! Delets
void TUnicodeTextStorage::Delete(uint32_t start, uint32_t count)
{
	if (start >= mCount || count == 0)
		return;
	
	// deleting to the end
	if (start + count >= mCount)
	{
		mCount = start;
		return;
	}
	
	// deleting in the middle, move down
	memcpy(&mStorage[start], &mStorage[start + count], (mCount - start - count) * sizeof(UniChar));
	mCount -= count;
}

//! Inserts the given CFStringRef.
//! \sa InsertText()
void TUnicodeTextStorage::InsertString(uint32_t position, CFStringRef text)
{
	CFIndex count = CFStringGetLength(text);
	
	UniChar buffer[count];
	CFStringGetCharacters(text, CFRangeMake(0, count), buffer);
	
	InsertText(position, buffer, count);
}

//! Inserts the given text \em before the character at \c position. So to
//! append text, \c position must equal the current number of characters,
//! the value returned by Count().
void TUnicodeTextStorage::InsertText(uint32_t position, const UniChar* text, uint32_t count)
{
	if (count == 0)
		return;
	
	if (position > mCount)
		position = mCount;
	
	// make room
	ReserveSpace(mCount + count);
	
	// move text after insertion point
	if (position < mCount)
	{
		memcpy(&mStorage[position + count], &mStorage[position], (mCount - position) * sizeof(UniChar));
	}
	
	// copy text into place and update count
	memcpy(&mStorage[position], text, count * sizeof(UniChar));
	mCount += count;
}
	
//! Internal method to allocat or reallocate the buffer. The number of
//! characters can be less than the current number of characters in the
//! buffer, in which case they are truncated.
void TUnicodeTextStorage::Allocate(uint32_t characters)
{
	uint32_t bytes = characters * sizeof(UniChar);
	mStorage = reinterpret_cast<UniChar*>(realloc(mStorage, bytes));
	if (mStorage)
	{
		mSize = characters;
		if (mSize < mCount)
			mCount = mSize;
	}
}

