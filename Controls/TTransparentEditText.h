/*
 *  TTransparentEditText.h
 *  Ritmo
 *
 *  Created by Chris Reed on Wed Feb 26 2003.
 *  Copyright (c) 2003 Chris Reed. All rights reserved.
 *
 */

#if !defined(_TTransparentEditText_h_)
#define _TTransparentEditText_h_

#include "TViewNoCompositingCompatible.h"
#include "TUnicodeTextStorage.h"

enum {
	//! Alias of the standard data tag. This sets the contents of the edit
	//! field to the CFString passed in.
	kTransparentEditTextCFStringTag = kControlEditTextCFStringTag,
	
	//! Alias of the standard data tag. This changes the edit field's font,
	//! size, style, and color settings.
	kTransparentEditTextFontStyleTag = kControlFontStyleTag,
	
	//! Also an alias of a standard data tag. This sets a key filter
	//! function that uses the same prototype as the standard one, and
	//! works exactly the same. The value is of type ControlKeyFilterProcPtr.
	kTransparentEditTextKeyFilterTag = kControlEditTextKeyFilterTag,
	
	//! Alias. The value is a ControlEditTextSelectionRec structure.
	kTransparentEditTextSelectionTag = kControlEditTextSelectionTag,
	
	//! Another alias of a standard data tag. This value is a Boolean type
	//! that indicates whether editing and selecting is allowed.
	kTransparentEditTextLockedTag = kControlEditTextLockedTag,
	
	//! With this data tag you can set an image that will be drawn in place
	//! of the normal keyboard focus ring. The value is a CGImageRef. The
	//! image will be retained by the control, so you do not have to keep
	//! a reference to it.
	//!
	//! When reading this data, the image is not retained an extra time, so
	//! do not release it when you are through (unless you retain it
	//! yourself, of course).
	kTransparentEditTextFocusCGImageRefTag = FOUR_CHAR_CODE('Fimg'),
	
	//! Like \c kTransparentEditTextFocusCGImageRefTag, this data tag can
	//! be used to set an image that will be drawn instead of the normal
	//! keyboard focus ring. But instead of a CGImageRef, a reference to
	//! a ".png" format image file is given as a CFURLRef.
	//!
	//! This data is write only; you cannot read back the URL, only actual
	//! CGImageRef using kTransparentEditTextFocusCGImageRefTag.
	kTransparentEditTextFocusImageURLTag = FOUR_CHAR_CODE('Furl'),
	
	//! Write only. Sets which control the tab key will advance focus to by
	//! the control's ControlID.
	kTransparentEditTextNextKeyboardFocusControlIDTag = FOUR_CHAR_CODE('NFID'),
	
	//! Sets the control which the tab key will move keyboard focus to by
	//! passing the ControlRef.
	kTransparentEditTextNextKeyboardFocusControlRefTag = FOUR_CHAR_CODE('NFCR'),

	//! Sets the ControlPartCode of the next focus control to advance focus to.
	kTransparentEditTextNextKeyboardFocusPartTag = FOUR_CHAR_CODE('NFPC'),

	//! Sets the background image of the control.
	kTransparentEditTextBackgroundImageTag = FOUR_CHAR_CODE('BKIM'),

	//! Sets the ATSUStyle property
	kTransparentEditTextFontATSUStyleTag = FOUR_CHAR_CODE('ATSU'),
	
	//! Sets the "draw frame' property
	kTransparentEditTextFontDrawFrameTag = FOUR_CHAR_CODE('FRAM')
	
};

/*!
 * \brief Custom edit text control that does not erase its background.
 *
 * This control subclass works like a standard unicode edit text control,
 * except for how it draws. It will never erase the background with a solid
 * color, and it does not draw any sort of frame or box around the text.
 * The focus ring may be turned off.
 */
class TTransparentEditText : public TViewNoCompositingCompatible
{
	AUGUIDefineControl(TTransparentEditText, TViewNoCompositingCompatible, "edittext");
protected:
	// Constructor/Destructor
	TTransparentEditText(HIViewRef inControl);
	virtual ~TTransparentEditText();
	
	virtual ControlKind GetKind();
	
	virtual bool UseNonblockingTracking() { return true; }
	
	//! \name Event handlers
	//@{
	virtual OSStatus Initialize(TCarbonEvent& inEvent);
	virtual void TitleChanged();
	virtual void ActiveStateChanged();
	virtual OSStatus SetFocusPart(ControlPartCode inDesiredFocus, Boolean inFocusEverything, ControlPartCode* outActualFocus);
	virtual OSStatus BoundsChanged(UInt32 inOptions, const HIRect& inOriginalBounds, const HIRect& inCurrentBounds, RgnHandle inInvalRgn);
	virtual ControlPartCode HitTest(const HIPoint&	inWhere);
	virtual OSStatus StillTracking(TCarbonEvent& inEvent, HIPoint& from);
	virtual OSStatus StartTracking(TCarbonEvent& inEvent, HIPoint& from);
    virtual void		DrawStaticBackground(RgnHandle	inLimitRgn, CGContextRef inContext, bool  inCompositing );
	virtual void CompatibleDraw(RgnHandle inLimitRgn, CGContextRef inContext, bool inCompositing);
	virtual UInt32 GetBehaviors() { return TViewNoCompositingCompatible::GetBehaviors() | kControlSupportsEmbedding | kControlSupportsFocus | kControlGetsFocusOnClick; }
	virtual OSStatus SetData(OSType inTag, ControlPartCode inPart, Size inSize, const void* inPtr);
	virtual OSStatus GetData(OSType inTag, ControlPartCode inPart, Size inSize, Size* outSize, void* inPtr);
	virtual OSStatus TextInput(TCarbonEvent& inEvent);
	//@}
	
	//! Has the same effect as typing whatever characters are passed in \a text.
	virtual void ProcessTextInput(UniChar* text, uint32_t count, uint32_t modifiers);
	
	virtual void SetDrawFrame(bool draw) { mDrawFrame = draw; };

private:
	ATSUStyle mTextStyle;	//!< Our single style object for all of the text.
	ATSUTextLayout mTextLayout;	//!< The text layout object we reuse each time we draw.
	TUnicodeTextStorage mTextStorage;	//!< The actual storage for the text.
	float mTextInset;	//!< The amount the text is inset from frame on all sides.
	bool mIsActive;	//!< Are we active?
	bool mIsFocused;	//!< Are we focused?
	bool mDrawFrame;	//!< Does the user want a box around the edit field?
	uint32_t mSelectionStart;
	uint32_t mSelectionEnd;
	bool mSelectionIsLeading;
	CFRunLoopTimerRef mCaretUpdateTimer;	//!< The timer that draws the insertion caret.
	bool mCaretIsVisible;	//!< Flag used by the caret update timer to keep track of which state the caret is currently in.
	uint32_t mSelectionAnchor;	//!< The anchor point during selection tracking.
	EventTime mLastClickTime;	//!< Used for double-click determination.
	HIPoint mLastClickPoint;	//!< Used for double-click determination.
	int mClickCount;	//!< The number of clicks that we're tracking.
	CGImageRef mFocusImage;	//!< The image drawn for the focus ring.
	CGImageRef mBackImage;	//!< The image drawn for the background.
	bool mIsLocked;	//!< Is editing and selecting allowed?
	ControlKeyFilterProcPtr mKeyFilter;	//!< Standard control key filter function.
	ControlRef mNextFocusControl;	//!< The control tab will take keyboard focus to.
	ControlPartCode mNextFocusPart;	//!< The part of the next focus control to focus on.
	
	void UpdateTextLayout();
	void GetLineHeight(uint32_t lineStartOffset, float* outAscent, float* outDescent);
	float GetBaselineForLine(uint32_t lineStartOffset);
	void FindWordAtOffset(uint32_t offset, uint32_t* wordStart, uint32_t* wordEnd);
	
	void InstallCaretUpdateTimer();
	void RemoveCaretUpdateTimer();
	
	void UpdateCaret(); //!< Called by CaretUpdateTimerCallback().
	
	static void CaretUpdateTimerCallback(CFRunLoopTimerRef timer, void *info);
};

#endif //_TTransparentEditText_h_
