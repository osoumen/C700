/*
 *  AUGUIUtilities.h
 *  Ritmo
 *
 *  Created by Chris Reed on Tue Feb 04 2003.
 *  Copyright (c) 2003 Chris Reed. All rights reserved.
 *
 */

#include "TRect.h"

#if !defined(_AUGUIUtilities_h_)
#define _AUGUIUtilities_h_

namespace AUGUI {

// Rect utilities
void HIRectToQDRect(const HIRect* inHIRect, Rect* outQDRect);
TRect RectForCGImage(CGImageRef theImage);

//! Simple utility to calculate radians from degrees.
//! \param deg Input value in degrees, from 0.0 to 360.0
//! \result The output in radians.
inline float DegreesToRadians(float deg) { return (2.0 * M_PI * deg / 360.0); }

//! Converts a point from local window coordinates to global coordinates.
void ConvertWindowPointToGlobal(WindowRef theWindow, Point& thePoint);

//! Converts a point from global to local window coordinates.
void ConvertGlobalToWindowPoint(WindowRef theWindow, Point& thePoint);

//! Sets the font face, size, style, and color based on the fields and
//! flags of the \c rec argument. The arguments \c baseThemeFont and \c
//! baseThemeColor are used as the baseline font and color. The fields of
//! \c rec are applied to this baseline depending on which flags are set.
//! This can vary from simply modifying one attribute from the baseline, or
//! totally changing everything.
void SetFontFromFontStyleRec(CGContextRef context, ControlFontStyleRec& rec, ThemeFontID baseThemeFont, ThemeTextColor themeColor);

//! Allocates a block of memory that is 16-byte aligned and zeroed. This makes it
//! possible to use Altivec routines on the data in the buffer. The buffer
//! is allocated with \c malloc() and so must be disposed with \c free().
//! \param numBytes The number of bytes to allocate. The buffer will not
//! 	necessarily be this size, as it may need to be rounded up.
//! \param realPointer On exit, this contains a pointer to the actual
//! 	buffer that must be passed to free() to be disposed.
//! \result Pointer to aligned buffer at least \c numBytes in size.
void* malloc_aligned(UInt32 numBytes, void** realPointer);

}

#endif // _AUGUIUtilties_h_
