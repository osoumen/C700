// =============================================================================
//	ControlMethods.h
// =============================================================================
//

#ifndef ControlMethods_H_
#define ControlMethods_H_

#include "HIObjectMethods.h"

struct OpaqueControlRef
	:	public OpaqueHIObjectRef
{

	inline	ControlRef	GetControlRef()
							{ return this; }

	OSStatus			Activate();
	OSStatus			AutoEmbed(
							WindowRef			inWindow );
	OSStatus			ChangePropertyAttributes(
							OSType				inPropertyCreator,
							OSType				inPropertyTag,
							UInt32				inAttributesToSet,
							UInt32				inAttributesToClear );
	OSStatus			CopyTitle(
							CFStringRef*		outString );
	OSStatus			CountSubs(
							UInt16*				outNumChildren );
	OSStatus			Deactivate();
	OSStatus			Disable();
	void				Dispose();
	void				Drag(
							Point				inStartPoint,
							const Rect*			inLimitRect,
							const Rect*			inSlopRect,
							DragConstraint		inAxis );
	void				Draw();
	void				DrawInCurrentPort();
	OSStatus			Embed(
							ControlRef			inContainer );
	OSStatus			Enable();
	ControlActionUPP	GetAction();
	OSStatus			GetBestRect(
							Rect*				outRect,
							SInt16*				outBaseLineOffset );
	OSStatus			GetClickActivation(
							Point					inWhere,
							EventModifiers			inModifiers,
							ClickActivationResult*	outResult );
	OSStatus			GetCommandID(
							UInt32*				outCommandID );
	OSStatus			GetID(
							ControlID*			outID );
	OSStatus			GetIndexedSub(
							UInt16				inIndex,
							ControlRef*			outSubControl );
	OSStatus			GetKind(
							ControlKind*		outControlKind );
	SInt32				GetMaximum();
	OSStatus			GetProperty(
							OSType				inPropertyCreator,
							OSType				inPropertyTag,
							UInt32				inBufferSize,
							UInt32*				outActualSize, /* can be NULL */
							void*				inPropertyBuffer );
	OSStatus			GetPropertyAttributes(
							OSType				inPropertyCreator,
							OSType				inPropertyTag,
							UInt32*				outAttributes );
	OSStatus			GetPropertySize(
							OSType				inPropertyCreator,
							OSType				inPropertyTag,
							UInt32*				outSize );
	SInt32				GetReference();
	OSStatus			GetRegion(
							ControlPartCode		inPart,
							RgnHandle			outRegion );
	OSStatus			GetSuper(
							ControlRef*			outParent );
	void				GetTitle(
							Str255				inTitle );
	SInt32				GetMinimum();
	SInt32				GetValue();
	ControlVariant			GetVariant();
	SInt32				GetViewSize();
	ControlPartCode			HandleClick(
							Point				inWhere,
							EventModifiers		inModifiers,
							ControlActionUPP	inAction ); /* can be NULL */
	OSStatus			HandleContextualMenuClick(
							Point				inWhere,
							Boolean*			outMenuDisplayed );
	ControlPartCode			HandleKey(
							SInt16				inKeyCode,
							SInt16				inCharCode,
							EventModifiers		inModifiers );
	OSStatus			HandleSetCursor(
							Point				inLocalPoint,
							EventModifiers		inModifiers,
							Boolean*			outCursorWasSet );
	void				Hide();
	void				Hilite(
							ControlPartCode		inHiliteState );
	Boolean				IsEnabled();
	Boolean				IsActive();
	Boolean				IsValidHandle();
	Boolean				IsVisible();
	void				Move(
							SInt16				inH,
							SInt16				inV );
	OSStatus			RemoveProperty(
							OSType				inPropertyCreator,
							OSType				inPropertyTag );
	SInt32				SendMessage(
							SInt16				inMessage,
							void*				inParam );
	void				SetAction(
							ControlActionUPP	inActionProc );
	OSStatus			SetColorProc(
							ControlColorUPP		inProc );
	OSStatus			SetCommandID(
							UInt32				inCommandID );
	OSStatus			SetFontStyle(
							const ControlFontStyleRec*	inStyle );
	OSStatus			SetID(
							const ControlID*	inID );
	void				SetMaximum(
							SInt32				inNewMaximum );
	void				SetMinimum(
							SInt32				inNewMinimum );
	OSStatus			SetProperty(
							OSType				inPropertyCreator,
							OSType				inPropertyTag,
							UInt32				inPropertySize,
							const void*			inPropertyData );
	void				SetReference(
							SInt32				inData );
	OSStatus			SetSupervisor(
							ControlRef			inSupervisor ); /* can be NULL */
	void				SetTitle(
							ConstStr255Param	inTitle );
	OSStatus			SetTitle(
							CFStringRef			inString );
	OSStatus			SetUpBackground();
	OSStatus			SetUpTextColor();
	void				SetValue(
							SInt32				inNewValue );
	void				SetViewSize(
							SInt32				inNewViewSize );
	OSStatus			SetVisibility(
							Boolean				inIsVisible,
							Boolean				inDoDraw );
	void				Size(
							SInt16				inW,
							SInt16				inH );
	void				Show();
	ControlPartCode			Test(
							Point				inTestPoint );
	ControlPartCode			Track(
							Point				inStartPoint,
							ControlActionUPP	inActionProc ); /* can be NULL */
};

// -----------------------------------------------------------------------------
//	Activate
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueControlRef::Activate()
{
	return ::ActivateControl( GetControlRef() );
}

// -----------------------------------------------------------------------------
//	AutoEmbed
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueControlRef::AutoEmbed(
	WindowRef			inWindow )
{
	return ::AutoEmbedControl( GetControlRef(), inWindow );
}

// -----------------------------------------------------------------------------
//	ChangePropertyAttributes
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueControlRef::ChangePropertyAttributes(
	OSType				inPropertyCreator,
	OSType				inPropertyTag,
	UInt32				inAttributesToSet,
	UInt32				inAttributesToClear )
{
	return ::ChangeControlPropertyAttributes( GetControlRef(), inPropertyCreator, inPropertyTag, inAttributesToSet, inAttributesToClear );
}

// -----------------------------------------------------------------------------
//	CopyTitle
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueControlRef::CopyTitle(
	CFStringRef*		outString )
{
	return ::CopyControlTitleAsCFString( GetControlRef(), outString );
}

// -----------------------------------------------------------------------------
//	CountSubs
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueControlRef::CountSubs(
	UInt16*				outNumChildren )
{
	return ::CountSubControls( GetControlRef(), outNumChildren );
}

// -----------------------------------------------------------------------------
//	Deactivate
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueControlRef::Deactivate()
{
	return ::DeactivateControl( GetControlRef() );
}

// -----------------------------------------------------------------------------
//	Disable
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueControlRef::Disable()
{
	return ::DisableControl( GetControlRef() );
}

// -----------------------------------------------------------------------------
//	Dispose
// -----------------------------------------------------------------------------
//
inline void
OpaqueControlRef::Dispose()
{
	::DisposeControl( GetControlRef() );
}

// -----------------------------------------------------------------------------
//	Drag
// -----------------------------------------------------------------------------
//
inline void
OpaqueControlRef::Drag(
	Point				inStartPoint,
	const Rect*			inLimitRect,
	const Rect*			inSlopRect,
	DragConstraint		inAxis )
{
	::DragControl( GetControlRef(), inStartPoint, inLimitRect, inSlopRect, inAxis );
}

// -----------------------------------------------------------------------------
//	Draw
// -----------------------------------------------------------------------------
//
inline void
OpaqueControlRef::Draw()
{
	::Draw1Control( GetControlRef() );
}

// -----------------------------------------------------------------------------
//	DrawInCurrentPort
// -----------------------------------------------------------------------------
//
inline void
OpaqueControlRef::DrawInCurrentPort()
{
	::DrawControlInCurrentPort( GetControlRef() );
}

// -----------------------------------------------------------------------------
//	Embed
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueControlRef::Embed(
	ControlRef			inContainer )
{
	return ::EmbedControl( GetControlRef(), inContainer );
}

// -----------------------------------------------------------------------------
//	Enable
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueControlRef::Enable()
{
	return ::EnableControl( GetControlRef() );
}

// -----------------------------------------------------------------------------
//	GetAction
// -----------------------------------------------------------------------------
//
inline ControlActionUPP
OpaqueControlRef::GetAction()
{
	return ::GetControlAction( GetControlRef() );
}

// -----------------------------------------------------------------------------
//	GetBestRect
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueControlRef::GetBestRect(
	Rect*				outRect,
	SInt16*				outBaseLineOffset )
{
	return ::GetBestControlRect( GetControlRef(), outRect, outBaseLineOffset );
}

// -----------------------------------------------------------------------------
//	GetClickActivation
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueControlRef::GetClickActivation(
	Point					inWhere,
	EventModifiers			inModifiers,
	ClickActivationResult*	outResult )
{
	return ::GetControlClickActivation( GetControlRef(), inWhere, inModifiers, outResult );
}

// -----------------------------------------------------------------------------
//	GetCommandID
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueControlRef::GetCommandID(
	UInt32*				outCommandID )
{
	return ::GetControlCommandID( GetControlRef(), outCommandID );
}

// -----------------------------------------------------------------------------
//	GetID
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueControlRef::GetID(
	ControlID*			outID )
{
	return ::GetControlID( GetControlRef(), outID );
}

// -----------------------------------------------------------------------------
//	GetIndexedSub
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueControlRef::GetIndexedSub(
	UInt16				inIndex,
	ControlRef*			outSubControl )
{
	return ::GetIndexedSubControl( GetControlRef(), inIndex, outSubControl );
}

// -----------------------------------------------------------------------------
//	GetKind
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueControlRef::GetKind(
	ControlKind*		outControlKind )
{
	return ::GetControlKind( GetControlRef(), outControlKind );
}

// -----------------------------------------------------------------------------
//	GetMaximum
// -----------------------------------------------------------------------------
//
inline SInt32
OpaqueControlRef::GetMaximum()
{
	return ::GetControl32BitMaximum( GetControlRef() );
}

// -----------------------------------------------------------------------------
//	GetProperty
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueControlRef::GetProperty(
	OSType				inPropertyCreator,
	OSType				inPropertyTag,
	UInt32				inBufferSize,
	UInt32*				outActualSize, /* can be NULL */
	void*				inPropertyBuffer )
{
	return ::GetControlProperty( GetControlRef(), inPropertyCreator, inPropertyTag, inBufferSize, outActualSize, inPropertyBuffer );
}

// -----------------------------------------------------------------------------
//	GetPropertyAttributes
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueControlRef::GetPropertyAttributes(
	OSType				inPropertyCreator,
	OSType				inPropertyTag,
	UInt32*				outAttributes )
{
	return ::GetControlPropertyAttributes( GetControlRef(), inPropertyCreator, inPropertyTag, outAttributes );
}

// -----------------------------------------------------------------------------
//	GetPropertySize
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueControlRef::GetPropertySize(
	OSType				inPropertyCreator,
	OSType				inPropertyTag,
	UInt32*				outSize )
{
	return ::GetControlPropertySize( GetControlRef(), inPropertyCreator, inPropertyTag, outSize );
}

// -----------------------------------------------------------------------------
//	GetReference
// -----------------------------------------------------------------------------
//
inline SInt32
OpaqueControlRef::GetReference()
{
	return ::GetControlReference( GetControlRef() );
}

// -----------------------------------------------------------------------------
//	GetRegion
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueControlRef::GetRegion(
	ControlPartCode		inPart,
	RgnHandle			outRegion )
{
	return ::GetControlRegion( GetControlRef(), inPart, outRegion );
}

// -----------------------------------------------------------------------------
//	GetSuper
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueControlRef::GetSuper(
	ControlRef*			outParent )
{
	return ::GetSuperControl( GetControlRef(), outParent );
}

// -----------------------------------------------------------------------------
//	GetTitle
// -----------------------------------------------------------------------------
//
inline void
OpaqueControlRef::GetTitle(
	Str255				inTitle )
{
	::GetControlTitle( GetControlRef(), inTitle );
}

// -----------------------------------------------------------------------------
//	GetMinimum
// -----------------------------------------------------------------------------
//
inline SInt32
OpaqueControlRef::GetMinimum()
{
	return ::GetControl32BitMinimum( GetControlRef() );
}

// -----------------------------------------------------------------------------
//	GetValue
// -----------------------------------------------------------------------------
//
inline SInt32
OpaqueControlRef::GetValue()
{
	return ::GetControl32BitValue( GetControlRef() );
}

// -----------------------------------------------------------------------------
//	GetVariant
// -----------------------------------------------------------------------------
//
inline ControlVariant
OpaqueControlRef::GetVariant()
{
	return ::GetControlVariant( GetControlRef() );
}

// -----------------------------------------------------------------------------
//	GetViewSize
// -----------------------------------------------------------------------------
//
inline SInt32
OpaqueControlRef::GetViewSize()
{
	return ::GetControlViewSize( GetControlRef() );
}

// -----------------------------------------------------------------------------
//	HandleClick
// -----------------------------------------------------------------------------
//
inline ControlPartCode
OpaqueControlRef::HandleClick(
	Point				inWhere,
	EventModifiers		inModifiers,
	ControlActionUPP	inAction ) /* can be NULL */
{
	return ::HandleControlClick( GetControlRef(), inWhere, inModifiers, inAction );
}

// -----------------------------------------------------------------------------
//	HandleContextualMenuClick
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueControlRef::HandleContextualMenuClick(
	Point				inWhere,
	Boolean*			outMenuDisplayed )
{
	return ::HandleControlContextualMenuClick( GetControlRef(), inWhere, outMenuDisplayed );
}

// -----------------------------------------------------------------------------
//	HandleKey
// -----------------------------------------------------------------------------
//
inline ControlPartCode
OpaqueControlRef::HandleKey(
	SInt16				inKeyCode,
	SInt16				inCharCode,
	EventModifiers		inModifiers )
{
	return ::HandleControlKey( GetControlRef(), inKeyCode, inCharCode, inModifiers );
}

// -----------------------------------------------------------------------------
//	HandleSetCursor
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueControlRef::HandleSetCursor(
	Point				inLocalPoint,
	EventModifiers		inModifiers,
	Boolean*			outCursorWasSet )
{
	return ::HandleControlSetCursor( GetControlRef(), inLocalPoint, inModifiers, outCursorWasSet );
}

// -----------------------------------------------------------------------------
//	Hide
// -----------------------------------------------------------------------------
//
inline void
OpaqueControlRef::Hide()
{
	::HideControl( GetControlRef() );
}

// -----------------------------------------------------------------------------
//	Hilite
// -----------------------------------------------------------------------------
//
inline void
OpaqueControlRef::Hilite(
	ControlPartCode		inHiliteState )
{
	::HiliteControl( GetControlRef(), inHiliteState );
}

// -----------------------------------------------------------------------------
//	IsEnabled
// -----------------------------------------------------------------------------
//
inline Boolean
OpaqueControlRef::IsEnabled()
{
	return ::IsControlEnabled( GetControlRef() );
}

// -----------------------------------------------------------------------------
//	IsActive
// -----------------------------------------------------------------------------
//
inline Boolean
OpaqueControlRef::IsActive()
{
	return ::IsControlActive( GetControlRef() );
}

// -----------------------------------------------------------------------------
//	IsValidHandle
// -----------------------------------------------------------------------------
//
inline Boolean
OpaqueControlRef::IsValidHandle()
{
	return ::IsValidControlHandle( GetControlRef() );
}

// -----------------------------------------------------------------------------
//	IsVisible
// -----------------------------------------------------------------------------
//
inline Boolean
OpaqueControlRef::IsVisible()
{
	return ::IsControlVisible( GetControlRef() );
}

// -----------------------------------------------------------------------------
//	Move
// -----------------------------------------------------------------------------
//
inline void
OpaqueControlRef::Move(
	SInt16				inH,
	SInt16				inV )
{
	::MoveControl( GetControlRef(), inH, inV );
}

// -----------------------------------------------------------------------------
//	RemoveProperty
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueControlRef::RemoveProperty(
	OSType				inPropertyCreator,
	OSType				inPropertyTag )
{
	return ::RemoveControlProperty( GetControlRef(), inPropertyCreator, inPropertyTag );
}

// -----------------------------------------------------------------------------
//	SendMessage
// -----------------------------------------------------------------------------
//
inline SInt32
OpaqueControlRef::SendMessage(
	SInt16				inMessage,
	void*				inParam )
{
	return ::SendControlMessage( GetControlRef(), inMessage, inParam );
}

// -----------------------------------------------------------------------------
//	SetAction
// -----------------------------------------------------------------------------
//
inline void
OpaqueControlRef::SetAction(
	ControlActionUPP	inActionProc )
{
	::SetControlAction( GetControlRef(), inActionProc );
}

// -----------------------------------------------------------------------------
//	SetColorProc
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueControlRef::SetColorProc(
	ControlColorUPP		inProc )
{
	return ::SetControlColorProc( GetControlRef(), inProc );
}

// -----------------------------------------------------------------------------
//	SetCommandID
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueControlRef::SetCommandID(
	UInt32				inCommandID )
{
	return ::SetControlCommandID( GetControlRef(), inCommandID );
}

// -----------------------------------------------------------------------------
//	SetFontStyle
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueControlRef::SetFontStyle(
	const ControlFontStyleRec*	inStyle )
{
	return ::SetControlFontStyle( GetControlRef(), inStyle );
}

// -----------------------------------------------------------------------------
//	SetID
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueControlRef::SetID(
	const ControlID*	inID )
{
	return ::SetControlID( GetControlRef(), inID );
}

// -----------------------------------------------------------------------------
//	SetMaximum
// -----------------------------------------------------------------------------
//
inline void
OpaqueControlRef::SetMaximum(
	SInt32				inNewMaximum )
{
	::SetControl32BitMaximum( GetControlRef(), inNewMaximum );
}

// -----------------------------------------------------------------------------
//	SetMinimum
// -----------------------------------------------------------------------------
//
inline void
OpaqueControlRef::SetMinimum(
	SInt32				inNewMinimum )
{
	::SetControl32BitMinimum( GetControlRef(), inNewMinimum );
}

// -----------------------------------------------------------------------------
//	SetProperty
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueControlRef::SetProperty(
	OSType				inPropertyCreator,
	OSType				inPropertyTag,
	UInt32				inPropertySize,
	const void*			inPropertyData )
{
	return ::SetControlProperty( GetControlRef(), inPropertyCreator, inPropertyTag, inPropertySize, inPropertyData );
}

// -----------------------------------------------------------------------------
//	SetReference
// -----------------------------------------------------------------------------
//
inline void
OpaqueControlRef::SetReference(
	SInt32				inData )
{
	::SetControlReference( GetControlRef(), inData );
}

// -----------------------------------------------------------------------------
//	SetSupervisor
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueControlRef::SetSupervisor(
	ControlRef			inSupervisor ) /* can be NULL */
{
	return ::SetControlSupervisor( GetControlRef(), inSupervisor );
}

// -----------------------------------------------------------------------------
//	SetTitle
// -----------------------------------------------------------------------------
//
inline void
OpaqueControlRef::SetTitle(
	ConstStr255Param	inTitle )
{
	::SetControlTitle( GetControlRef(), inTitle );
}

// -----------------------------------------------------------------------------
//	SetTitle
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueControlRef::SetTitle(
	CFStringRef			inString )
{
	return ::SetControlTitleWithCFString( GetControlRef(), inString );
}

// -----------------------------------------------------------------------------
//	SetUpBackground
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueControlRef::SetUpBackground()
{
	return ::SetUpControlBackground( GetControlRef(), 32, true );
}

// -----------------------------------------------------------------------------
//	SetUpTextColor
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueControlRef::SetUpTextColor()
{
	return ::SetUpControlTextColor( GetControlRef(), 32, true );
}

// -----------------------------------------------------------------------------
//	SetValue
// -----------------------------------------------------------------------------
//
inline void
OpaqueControlRef::SetValue(
	SInt32				inNewValue )
{
	::SetControl32BitValue( GetControlRef(), inNewValue );
}

// -----------------------------------------------------------------------------
//	SetViewSize
// -----------------------------------------------------------------------------
//
inline void
OpaqueControlRef::SetViewSize(
	SInt32				inNewViewSize )
{
	::SetControlViewSize( GetControlRef(), inNewViewSize );
}

// -----------------------------------------------------------------------------
//	SetVisibility
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueControlRef::SetVisibility(
	Boolean				inIsVisible,
	Boolean				inDoDraw )
{
	return ::SetControlVisibility( GetControlRef(), inIsVisible, inDoDraw );
}

// -----------------------------------------------------------------------------
//	Size
// -----------------------------------------------------------------------------
//
inline void
OpaqueControlRef::Size(
	SInt16				inW,
	SInt16				inH )
{
	::SizeControl( GetControlRef(), inW, inH );
}

// -----------------------------------------------------------------------------
//	Show
// -----------------------------------------------------------------------------
//
inline void
OpaqueControlRef::Show()
{
	::ShowControl( GetControlRef() );
}

// -----------------------------------------------------------------------------
//	Test
// -----------------------------------------------------------------------------
//
inline ControlPartCode
OpaqueControlRef::Test(
	Point				inTestPoint )
{
	return ::TestControl( GetControlRef(), inTestPoint );
}

// -----------------------------------------------------------------------------
//	Track
// -----------------------------------------------------------------------------
//
inline ControlPartCode
OpaqueControlRef::Track(
	Point				inStartPoint,
	ControlActionUPP	inActionProc ) /* can be NULL */
{
	return ::TrackControl( GetControlRef(), inStartPoint, inActionProc );
}

#endif // ControlMethods_H_