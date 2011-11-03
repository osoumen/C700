// =============================================================================
//	HIViewMethods.h
// =============================================================================
//

#ifndef HIViewMethods_H_
#define HIViewMethods_H_

#include "ControlMethods.h"

struct OpaqueHIViewRef
	:	public OpaqueControlRef
{
public:
	inline HIViewRef	GetHIViewRef()
							{ return this; }

	OSStatus			AddSubview(
							HIViewRef		inNewChild );
	OSStatus			AdvanceFocus(
							EventModifiers	inModifiers );
	OSStatus			ApplyLayout();
	OSStatus			ChangeAttributes(
							OptionBits		inAttrsToSet,
							OptionBits		inAttrsToClear );
	OSStatus			ChangeFeatures(
							HIViewFeatures	inFeaturesToSet,
							HIViewFeatures	inFeaturesToClear );
	OSStatus			Click(
							EventRef		inEvent );
	CFStringRef			CopyText();
	OSStatus			CopyShape(
							HIViewPartCode	inPart,
							HIShapeRef*		outShape );
	CFIndex				CountSubviews();
	OSStatus			CreateOffscreenImage(
							OptionBits		inOptions,
							HIRect*			outFrame,	/* can be NULL */
							CGImageRef*		outImage );
	OSStatus			FindByID(
							HIViewID		inID,
							HIViewRef*		outControl );
	OSStatus			GetAttributes(
							OptionBits*		outAttrs );
	OSStatus			GetBounds(
							HIRect*			outRect );
	OSStatus			GetCommandID(
							UInt32*			outCommandID );
	OSStatus			GetFeatures(
							HIViewFeatures*	outFeatures );
	HIViewRef			GetFirstSubview();
	OSStatus			GetFocusPart(
							HIViewPartCode*	outFocusPart );
	OSStatus			GetFrame(
							HIRect*			outRect );
	OSStatus			GetID(
							HIViewID*		outID );
	OSStatus			GetIndexedSubview(
							CFIndex			inSubviewIndex,
							HIViewRef*		outSubview );
	OSStatus			GetKind(
							HIViewKind*		outViewKind );
	HIViewRef			GetLastSubview();
	OSStatus			GetLayoutInfo(
							HILayoutInfo*	outLayoutInfo );
	SInt32				GetMaximum();
	SInt32				GetMinimum();
	Boolean				GetNeedsDisplay();
	HIViewRef			GetNextView();
	OSStatus			GetOptimalBounds(
							HIRect*			outBounds,				/* can be NULL */
							float*			outBaseLineOffset );	/* can be NULL */
	HIViewRef			GetPreviousView();
	OSStatus			GetSizeConstraints(
							HISize*			outMinSize,		/* can be NULL */
							HISize*			outMaxSize );	/* can be NULL */
	OSStatus			GetSubviewHit(
							const HIPoint*	inPoint,
							Boolean			inDeep,
							HIViewRef*		outView );
	HIViewRef			GetSuperview();
	SInt32				GetValue();
	OSStatus			GetViewForMouseEvent(
							EventRef		inEvent,
							HIViewRef*		outView );
	SInt32				GetViewSize();
	WindowRef			GetWindow();
	Boolean				IsActive(
							Boolean*		outIsLatentActive );	/* can be NULL */
	Boolean				IsDrawingEnabled();
	Boolean				IsEnabled(
							Boolean*		outIsLatentEnabled );	/* can be NULL */
	Boolean				IsLatentlyVisible();
	Boolean				IsLayoutActive();
	Boolean				IsValid();
	Boolean				IsVisible();
	OSStatus			MoveBy(
							float			inDX,
							float			inDY );
	OSStatus			PlaceInSuperviewAt(
							float			inX,
							float			inY );
	OSStatus			RegionChanged(
							HIViewPartCode	inRegionCode );
	OSStatus			RemoveFromSuperview();
	OSStatus			Render();
	OSStatus			ReshapeStructure();
	OSStatus			ResumeLayout();
	OSStatus			ScrollRect(
							const HIRect*	inRect,	/* can be NULL */
							float			inDX,
							float			inDY );
	OSStatus			SetActivated(
							Boolean			inSetActivated );
	OSStatus			SetBoundsOrigin(
							float			inX,
							float			inY );
	OSStatus			SetCommandID(
							UInt32			inCommandID );
	OSStatus			SetDrawingEnabled(
							Boolean			inEnabled );
	OSStatus			SetEnabled(
							Boolean			inSetEnabled );
	OSStatus			SetFirstSubViewFocus(
							HIViewRef		inSubView );		/* can be NULL */
	OSStatus			SetFrame(
							const HIRect*	inRect );
	OSStatus			SetHilite(
							HIViewPartCode	inHilitePart );
	OSStatus			SetID(
							HIViewID		inID );
	OSStatus			SetLayoutInfo(
							const HILayoutInfo*	inLayoutInfo );
	OSStatus			SetMaximum(
							SInt32			inMaximum );
	OSStatus			SetMinimum(
							SInt32			inMinimum );
	OSStatus			SetNeedsDisplay(
							Boolean			inNeedsDisplay );
	OSStatus			SetNeedsDisplayInRect(
							const HIRect*	inRect,
							Boolean			inNeedsDisplay );
	OSStatus			SetNeedsDisplayInRegion(
							RgnHandle		inRgn,
							Boolean			inNeedsDisplay );
	OSStatus			SetNeedsDisplayInShape(
							HIShapeRef		inArea,
							Boolean			inNeedsDisplay );
	OSStatus			SetNextFocus(
							HIViewRef		inNextFocus );	/* can be NULL */
	OSStatus			SetText(
							CFStringRef		inText );
	OSStatus			SetValue(
							SInt32			inValue );
	OSStatus			SetViewSize(
							SInt32			inViewSize );
	OSStatus			SetVisible(
							Boolean			inVisible );
	OSStatus			SetZOrder(
							HIViewZOrderOp	inOp,
							HIViewRef		inOther );	/* can be NULL */
	OSStatus			SimulateClick(
							HIViewPartCode	inPartToClick,
							UInt32			inModifiers,
							HIViewPartCode*	outPartClicked );	/* can be NULL */
	Boolean				SubtreeContainsFocus();
	OSStatus			SuspendLayout();
};

// -----------------------------------------------------------------------------
//	AddSubview
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::AddSubview(
	HIViewRef		inNewChild )
{
	return ::HIViewAddSubview( GetHIViewRef(), inNewChild );
}

// -----------------------------------------------------------------------------
//	AdvanceFocus
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::AdvanceFocus(
	EventModifiers	inModifiers )
{
	return ::HIViewAdvanceFocus( GetHIViewRef(), inModifiers );
}

// -----------------------------------------------------------------------------
//	ApplyLayout
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::ApplyLayout()
{
	return ::HIViewApplyLayout( GetHIViewRef() );
}

// -----------------------------------------------------------------------------
//	ChangeAttributes
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::ChangeAttributes(
	OptionBits		inAttrsToSet,
	OptionBits		inAttrsToClear )
{
	return ::HIViewChangeAttributes( GetHIViewRef(), inAttrsToSet, inAttrsToClear );
}

// -----------------------------------------------------------------------------
//	ChangeFeatures
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::ChangeFeatures(
	HIViewFeatures	inFeaturesToSet,
	HIViewFeatures	inFeaturesToClear )
{
	return ::HIViewChangeFeatures( GetHIViewRef(), inFeaturesToSet, inFeaturesToClear );
}

// -----------------------------------------------------------------------------
//	Click
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::Click(
	EventRef		inEvent )
{
	return ::HIViewClick( GetHIViewRef(), inEvent );
}

// -----------------------------------------------------------------------------
//	CopyText
// -----------------------------------------------------------------------------
//
inline CFStringRef
OpaqueHIViewRef::CopyText()
{
	return ::HIViewCopyText( GetHIViewRef() );
}

// -----------------------------------------------------------------------------
//	CopyShape
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::CopyShape(
	HIViewPartCode	inPart,
	HIShapeRef*		outShape )
{
	return ::HIViewCopyShape( GetHIViewRef(), inPart, outShape );
}

// -----------------------------------------------------------------------------
//	CountSubviews
// -----------------------------------------------------------------------------
//
inline CFIndex
OpaqueHIViewRef::CountSubviews()
{
	return ::HIViewCountSubviews( GetHIViewRef() );
}

// -----------------------------------------------------------------------------
//	CreateOffscreenImage
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::CreateOffscreenImage(
	OptionBits		inOptions,
	HIRect*			outFrame,	/* can be NULL */
	CGImageRef*		outImage )
{
	return ::HIViewCreateOffscreenImage( GetHIViewRef(), inOptions, outFrame, outImage );
}

// -----------------------------------------------------------------------------
//	FindByID
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::FindByID(
	HIViewID		inID,
	HIViewRef*		outControl )
{
	return ::HIViewFindByID( GetHIViewRef(), inID, outControl );
}

// -----------------------------------------------------------------------------
//	GetAttributes
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::GetAttributes(
	OptionBits*		outAttrs )
{
	return ::HIViewGetAttributes( GetHIViewRef(), outAttrs );
}

// -----------------------------------------------------------------------------
//	GetBounds
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::GetBounds(
	HIRect*			outRect )
{
	return ::HIViewGetBounds( GetHIViewRef(), outRect );
}

// -----------------------------------------------------------------------------
//	GetCommandID
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::GetCommandID(
	UInt32*			outCommandID )
{
	return ::HIViewGetCommandID( GetHIViewRef(), outCommandID );
}

// -----------------------------------------------------------------------------
//	GetFeatures
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::GetFeatures(
	HIViewFeatures*	outFeatures )
{
	return ::HIViewGetFeatures( GetHIViewRef(), outFeatures );
}

// -----------------------------------------------------------------------------
//	GetFirstSubview
// -----------------------------------------------------------------------------
//
inline HIViewRef
OpaqueHIViewRef::GetFirstSubview()
{
	return ::HIViewGetFirstSubview( GetHIViewRef() );
}

// -----------------------------------------------------------------------------
//	GetFocusPart
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::GetFocusPart(
	HIViewPartCode*	outFocusPart )
{
	return ::HIViewGetFocusPart( GetHIViewRef(), outFocusPart );
}

// -----------------------------------------------------------------------------
//	GetFrame
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::GetFrame(
	HIRect*			outRect )
{
	return ::HIViewGetFrame( GetHIViewRef(), outRect );
}

// -----------------------------------------------------------------------------
//	GetID
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::GetID(
	HIViewID*		outID )
{
	return ::HIViewGetID( GetHIViewRef(), outID );
}

// -----------------------------------------------------------------------------
//	GetIndexedSubview
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::GetIndexedSubview(
	CFIndex			inSubviewIndex,
	HIViewRef*		outSubview )
{
	return ::HIViewGetIndexedSubview( GetHIViewRef(), inSubviewIndex, outSubview );
}

// -----------------------------------------------------------------------------
//	GetKind
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::GetKind(
	HIViewKind*		outViewKind )
{
	return ::HIViewGetKind( GetHIViewRef(), outViewKind );
}

// -----------------------------------------------------------------------------
//	GetLastSubview
// -----------------------------------------------------------------------------
//
inline HIViewRef
OpaqueHIViewRef::GetLastSubview()
{
	return ::HIViewGetLastSubview( GetHIViewRef() );
}

// -----------------------------------------------------------------------------
//	GetLayoutInfo
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::GetLayoutInfo(
	HILayoutInfo*	outLayoutInfo )
{
	return ::HIViewGetLayoutInfo( GetHIViewRef(), outLayoutInfo );
}

// -----------------------------------------------------------------------------
//	GetMaximum
// -----------------------------------------------------------------------------
//
inline SInt32
OpaqueHIViewRef::GetMaximum()
{
	return ::HIViewGetMaximum( GetHIViewRef() );
}

// -----------------------------------------------------------------------------
//	GetMinimum
// -----------------------------------------------------------------------------
//
inline SInt32
OpaqueHIViewRef::GetMinimum()
{
	return ::HIViewGetMinimum( GetHIViewRef() );
}

// -----------------------------------------------------------------------------
//	GetNeedsDisplay
// -----------------------------------------------------------------------------
//
inline Boolean
OpaqueHIViewRef::GetNeedsDisplay()
{
	return ::HIViewGetNeedsDisplay( GetHIViewRef() );
}

// -----------------------------------------------------------------------------
//	GetNextView
// -----------------------------------------------------------------------------
//
inline HIViewRef
OpaqueHIViewRef::GetNextView()
{
	return ::HIViewGetNextView( GetHIViewRef() );
}

// -----------------------------------------------------------------------------
//	GetOptimalBounds
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::GetOptimalBounds(
	HIRect*			outBounds,			/* can be NULL */
	float*			outBaseLineOffset )	/* can be NULL */
{
	return ::HIViewGetOptimalBounds( GetHIViewRef(), outBounds, outBaseLineOffset );
}

// -----------------------------------------------------------------------------
//	GetPreviousView
// -----------------------------------------------------------------------------
//
inline HIViewRef
OpaqueHIViewRef::GetPreviousView()
{
	return ::HIViewGetPreviousView( GetHIViewRef() );
}

// -----------------------------------------------------------------------------
//	GetSizeConstraints
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::GetSizeConstraints(
	HISize*			outMinSize,		/* can be NULL */
	HISize*			outMaxSize )	/* can be NULL */
{
	return ::HIViewGetSizeConstraints( GetHIViewRef(), outMinSize, outMaxSize );
}

// -----------------------------------------------------------------------------
//	GetSubviewHit
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::GetSubviewHit(
	const HIPoint*	inPoint,
	Boolean			inDeep,
	HIViewRef*		outView )
{
	return ::HIViewGetSubviewHit( GetHIViewRef(), inPoint, inDeep, outView );
}

// -----------------------------------------------------------------------------
//	GetSuperview
// -----------------------------------------------------------------------------
//
inline HIViewRef
OpaqueHIViewRef::GetSuperview()
{
	return ::HIViewGetSuperview( GetHIViewRef() );
}

// -----------------------------------------------------------------------------
//	GetValue
// -----------------------------------------------------------------------------
//
inline SInt32
OpaqueHIViewRef::GetValue()
{
	return ::HIViewGetValue( GetHIViewRef() );
}

// -----------------------------------------------------------------------------
//	GetViewForMouseEvent
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::GetViewForMouseEvent(
	EventRef		inEvent,
	HIViewRef*		outView )
{
	return ::HIViewGetViewForMouseEvent( GetHIViewRef(), inEvent, outView );
}

// -----------------------------------------------------------------------------
//	GetViewSize
// -----------------------------------------------------------------------------
//
inline SInt32
OpaqueHIViewRef::GetViewSize()
{
	return ::HIViewGetViewSize( GetHIViewRef() );
}

// -----------------------------------------------------------------------------
//	GetWindow
// -----------------------------------------------------------------------------
//
inline WindowRef
OpaqueHIViewRef::GetWindow()
{
	return ::HIViewGetWindow( GetHIViewRef() );
}

// -----------------------------------------------------------------------------
//	IsActive
// -----------------------------------------------------------------------------
//
inline Boolean
OpaqueHIViewRef::IsActive(
	Boolean*		outIsLatentActive )	/* can be NULL */
{
	return ::HIViewIsActive( GetHIViewRef(), outIsLatentActive );
}

// -----------------------------------------------------------------------------
//	IsDrawingEnabled
// -----------------------------------------------------------------------------
//
inline Boolean
OpaqueHIViewRef::IsDrawingEnabled()
{
	return ::HIViewIsDrawingEnabled( GetHIViewRef() );
}

// -----------------------------------------------------------------------------
//	IsEnabled
// -----------------------------------------------------------------------------
//
inline Boolean
OpaqueHIViewRef::IsEnabled(
	Boolean*		outIsLatentEnabled )	/* can be NULL */
{
	return ::HIViewIsEnabled( GetHIViewRef(), outIsLatentEnabled );
}

// -----------------------------------------------------------------------------
//	IsLatentlyVisible
// -----------------------------------------------------------------------------
//
inline Boolean
OpaqueHIViewRef::IsLatentlyVisible()
{
	return ::HIViewIsLatentlyVisible( GetHIViewRef() );
}

// -----------------------------------------------------------------------------
//	IsLayoutActive
// -----------------------------------------------------------------------------
//
inline Boolean
OpaqueHIViewRef::IsLayoutActive()
{
	return ::HIViewIsLayoutActive( GetHIViewRef() );
}

// -----------------------------------------------------------------------------
//	IsValid
// -----------------------------------------------------------------------------
//
inline Boolean
OpaqueHIViewRef::IsValid()
{
	return ::HIViewIsValid( GetHIViewRef() );
}

// -----------------------------------------------------------------------------
//	IsVisible
// -----------------------------------------------------------------------------
//
inline Boolean
OpaqueHIViewRef::IsVisible()
{
	return ::HIViewIsVisible( GetHIViewRef() );
}

// -----------------------------------------------------------------------------
//	MoveBy
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::MoveBy(
	float			inDX,
	float			inDY )
{
	return ::HIViewMoveBy( GetHIViewRef(), inDX, inDY );
}

// -----------------------------------------------------------------------------
//	PlaceInSuperviewAt
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::PlaceInSuperviewAt(
	float			inX,
	float			inY )
{
	return ::HIViewPlaceInSuperviewAt( GetHIViewRef(), inX, inY );
}

// -----------------------------------------------------------------------------
//	RegionChanged
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::RegionChanged(
	HIViewPartCode	inRegionCode )
{
	return ::HIViewRegionChanged( GetHIViewRef(), inRegionCode );
}

// -----------------------------------------------------------------------------
//	RemoveFromSuperview
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::RemoveFromSuperview()
{
	return ::HIViewRemoveFromSuperview( GetHIViewRef() );
}

// -----------------------------------------------------------------------------
//	Render
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::Render()
{
	return ::HIViewRender( GetHIViewRef() );
}

// -----------------------------------------------------------------------------
//	ReshapeStructure
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::ReshapeStructure()
{
	return ::HIViewReshapeStructure( GetHIViewRef() );
}

// -----------------------------------------------------------------------------
//	ResumeLayout
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::ResumeLayout()
{
	return ::HIViewResumeLayout( GetHIViewRef() );
}

// -----------------------------------------------------------------------------
//	ScrollRect
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::ScrollRect(
	const HIRect*	inRect,	/* can be NULL */
	float			inDX,
	float			inDY )
{
	return ::HIViewScrollRect( GetHIViewRef(), inRect, inDX, inDY );
}

// -----------------------------------------------------------------------------
//	SetActivated
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::SetActivated(
	Boolean			inSetActivated )
{
	return ::HIViewSetActivated( GetHIViewRef(), inSetActivated );
}

// -----------------------------------------------------------------------------
//	SetBoundsOrigin
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::SetBoundsOrigin(
	float			inX,
	float			inY )
{
	return ::HIViewSetBoundsOrigin( GetHIViewRef(), inX, inY );
}

// -----------------------------------------------------------------------------
//	SetCommandID
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::SetCommandID(
	UInt32			inCommandID )
{
	return ::HIViewSetCommandID( GetHIViewRef(), inCommandID );
}

// -----------------------------------------------------------------------------
//	SetDrawingEnabled
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::SetDrawingEnabled(
	Boolean			inEnabled )
{
	return ::HIViewSetDrawingEnabled( GetHIViewRef(), inEnabled );
}

// -----------------------------------------------------------------------------
//	SetEnabled
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::SetEnabled(
	Boolean			inSetEnabled )
{
	return ::HIViewSetEnabled( GetHIViewRef(), inSetEnabled );
}

// -----------------------------------------------------------------------------
//	SetFirstSubViewFocus
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::SetFirstSubViewFocus(
	HIViewRef		inSubView )		/* can be NULL */
{
	return ::HIViewSetFirstSubViewFocus( GetHIViewRef(), inSubView );
}

// -----------------------------------------------------------------------------
//	SetFrame
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::SetFrame(
	const HIRect*	inRect )
{
	return ::HIViewSetFrame( GetHIViewRef(), inRect );
}

// -----------------------------------------------------------------------------
//	SetHilite
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::SetHilite(
	HIViewPartCode	inHilitePart )
{
	return ::HIViewSetHilite( GetHIViewRef(), inHilitePart );
}

// -----------------------------------------------------------------------------
//	SetID
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::SetID(
	HIViewID		inID )
{
	return ::HIViewSetID( GetHIViewRef(), inID );
}

// -----------------------------------------------------------------------------
//	SetLayoutInfo
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::SetLayoutInfo(
	const HILayoutInfo*	inLayoutInfo )
{
	return ::HIViewSetLayoutInfo( GetHIViewRef(), inLayoutInfo );
}

// -----------------------------------------------------------------------------
//	SetMaximum
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::SetMaximum(
	SInt32			inMaximum )
{
	return ::HIViewSetMaximum( GetHIViewRef(), inMaximum );
}

// -----------------------------------------------------------------------------
//	SetMinimum
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::SetMinimum(
	SInt32			inMinimum )
{
	return ::HIViewSetMinimum( GetHIViewRef(), inMinimum );
}

// -----------------------------------------------------------------------------
//	SetNeedsDisplay
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::SetNeedsDisplay(
	Boolean			inNeedsDisplay )
{
	return ::HIViewSetNeedsDisplay( GetHIViewRef(), inNeedsDisplay );
}

// -----------------------------------------------------------------------------
//	SetNeedsDisplayInRect
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::SetNeedsDisplayInRect(
	const HIRect*	inRect,
	Boolean			inNeedsDisplay )
{
	return ::HIViewSetNeedsDisplayInRect( GetHIViewRef(), inRect, inNeedsDisplay );
}

// -----------------------------------------------------------------------------
//	SetNeedsDisplayInRegion
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::SetNeedsDisplayInRegion(
	RgnHandle		inRgn,
	Boolean			inNeedsDisplay )
{
	return ::HIViewSetNeedsDisplayInRegion( GetHIViewRef(), inRgn, inNeedsDisplay );
}

// -----------------------------------------------------------------------------
//	SetNeedsDisplayInShape
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::SetNeedsDisplayInShape(
	HIShapeRef		inArea,
	Boolean			inNeedsDisplay )
{
	return ::HIViewSetNeedsDisplayInShape( GetHIViewRef(), inArea, inNeedsDisplay );
}

// -----------------------------------------------------------------------------
//	SetNextFocus
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::SetNextFocus(
	HIViewRef		inNextFocus )	/* can be NULL */
{
	return ::HIViewSetNextFocus( GetHIViewRef(), inNextFocus );
}

// -----------------------------------------------------------------------------
//	SetText
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::SetText(
	CFStringRef		inText )
{
	return ::HIViewSetText( GetHIViewRef(), inText );
}

// -----------------------------------------------------------------------------
//	SetValue
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::SetValue(
	SInt32			inValue )
{
	return ::HIViewSetValue( GetHIViewRef(), inValue );
}

// -----------------------------------------------------------------------------
//	SetViewSize
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::SetViewSize(
	SInt32			inViewSize )
{
	return ::HIViewSetViewSize( GetHIViewRef(), inViewSize );
}

// -----------------------------------------------------------------------------
//	SetVisible
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::SetVisible(
	Boolean			inVisible )
{
	return ::HIViewSetVisible( GetHIViewRef(), inVisible );
}

// -----------------------------------------------------------------------------
//	SetZOrder
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::SetZOrder(
	HIViewZOrderOp	inOp,
	HIViewRef		inOther )	/* can be NULL */
{
	return ::HIViewSetZOrder( GetHIViewRef(), inOp, inOther );
}

// -----------------------------------------------------------------------------
//	SimulateClick
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::SimulateClick(
	HIViewPartCode	inPartToClick,
	UInt32			inModifiers,
	HIViewPartCode*	outPartClicked )	/* can be NULL */
{
	return ::HIViewSimulateClick( GetHIViewRef(), inPartToClick, inModifiers, outPartClicked );
}

// -----------------------------------------------------------------------------
//	SubtreeContainsFocus
// -----------------------------------------------------------------------------
//
inline Boolean
OpaqueHIViewRef::SubtreeContainsFocus()
{
	return ::HIViewSubtreeContainsFocus( GetHIViewRef() );
}

// -----------------------------------------------------------------------------
//	SuspendLayout
// -----------------------------------------------------------------------------
//
inline OSStatus
OpaqueHIViewRef::SuspendLayout()
{
	return ::HIViewSuspendLayout( GetHIViewRef() );
}

#endif // HIViewMethods_H_