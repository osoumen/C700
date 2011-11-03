/*
    File:		TView.cp
    
    Version:	1.1

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	Copyright © 2000-2005 Apple Computer, Inc., All Rights Reserved
*/

/*
 NOTE: This is NOWHERE near a completely exhaustive implementation of a view. There are
       many more carbon events one could intercept and hook into this.
*/

#include "TView.h"

//-----------------------------------------------------------------------------------
//	constants
//-----------------------------------------------------------------------------------
//
const EventTypeSpec kHIViewEvents[] =
{	{ kEventClassCommand, kEventCommandProcess },
	{ kEventClassCommand, kEventCommandUpdateStatus },
	
	{ kEventClassControl, kEventControlInitialize },
	{ kEventClassControl, kEventControlDraw },
	{ kEventClassControl, kEventControlHitTest },
	{ kEventClassControl, kEventControlGetPartRegion },
	{ kEventClassControl, kEventControlGetData },
	{ kEventClassControl, kEventControlSetData },
	{ kEventClassControl, kEventControlGetOptimalBounds },
	{ kEventClassControl, kEventControlBoundsChanged },
	{ kEventClassControl, kEventControlTrack },
	{ kEventClassControl, kEventControlGetSizeConstraints },
	{ kEventClassControl, kEventControlHit },
	
	{ kEventClassControl, kEventControlHiliteChanged },
	{ kEventClassControl, kEventControlActivate },
	{ kEventClassControl, kEventControlDeactivate },
	{ kEventClassControl, kEventControlValueFieldChanged },
	{ kEventClassControl, kEventControlTitleChanged },
	{ kEventClassControl, kEventControlEnabledStateChanged },
	{ kEventClassControl, kEventControlOwningWindowChanged },
	{ kEventClassControl, kEventControlVisibilityChanged },
};

// This param name was accidentally left unexported for
// the release of Jaguar.
const EventParamName kEventParamControlLikesDrag = 'cldg';

//-----------------------------------------------------------------------------------
//	TView constructor
//-----------------------------------------------------------------------------------
//
TView::TView(
	HIViewRef			inView )
	:	TObject( (HIObjectRef) inView )
{
	verify_noerr( AddEventTypesToHandler( GetEventHandler(), GetEventTypeCount( kHIViewEvents ),
			kHIViewEvents ) );
}

//-----------------------------------------------------------------------------------
//	TView destructor
//-----------------------------------------------------------------------------------
//
TView::~TView()
{
}

//-----------------------------------------------------------------------------------
//	Initialize
//-----------------------------------------------------------------------------------
//
OSStatus
TView::Initialize(
	TCarbonEvent&		inEvent )
{
	return noErr;
}

//-----------------------------------------------------------------------------------
//	GetBehaviors
//-----------------------------------------------------------------------------------
//	Returns our behaviors. Any subclass that overrides this should OR in its behaviors
//	into the inherited behaviors.
//
UInt32
TView::GetBehaviors()
{
	return kControlSupportsDataAccess | kControlSupportsGetRegion;
}

//-----------------------------------------------------------------------------------
//	Draw
//-----------------------------------------------------------------------------------
//	Draw your view. You should draw based on VIEW coordinates, not frame coordinates.
//
void
TView::Draw(
	RgnHandle			inLimitRgn,
	CGContextRef		inContext )
{
	#pragma unused( inLimitRgn, inContext )
}

//-----------------------------------------------------------------------------------
//	HitTest
//-----------------------------------------------------------------------------------
//	Asks your view to return what part of itself (if any) is hit by the point given
//	to it. The point is in VIEW coordinates, so you should get the view rect to do
//	bounds checking.
//
ControlPartCode
TView::HitTest(
	const HIPoint&		inWhere )
{
	#pragma unused( inWhere )

	return kControlNoPart;
}

//-----------------------------------------------------------------------------------
//	GetRegion
//-----------------------------------------------------------------------------------
//	This is called when someone wants to know certain metrics regarding this view.
//	The base class does nothing. Subclasses should handle their own parts, such as
//	the content region by overriding this method. The structure region is, by default,
//	the view's bounds. If a subclass does not have a region for a given part, it 
//	should always call the inherited method.
//
OSStatus
TView::GetRegion(
	ControlPartCode		inPart,
	RgnHandle			outRgn )
{
	#pragma unused( inPart, outRgn )

	return eventNotHandledErr;
}

//-----------------------------------------------------------------------------------
//	GetData
//-----------------------------------------------------------------------------------
//	Gets some data from our view. Subclasses should override to handle their own
//	defined data tags. If a tag is not understood by the subclass, it should call the
//	inherited method. As a convienience, we map the request for ControlKind into our
//	GetKind method.
//
OSStatus
TView::GetData(
	OSType				inTag,
	ControlPartCode		inPart,
	Size				inSize,
	Size*				outSize,
	void*				inPtr )
{
	#pragma unused( inPart )

	OSStatus			err = noErr;
	
	switch( inTag )
	{
		case kControlKindTag:
			if ( inPtr )
			{
				if ( inSize != sizeof( ControlKind ) )
					err = errDataSizeMismatch;
				else
					( *(ControlKind *) inPtr ) = GetKind();
			}
			*outSize = sizeof( ControlKind );
			break;
		
		default:
			err = eventNotHandledErr;
			break;
	}
	
	return err;
}

//-----------------------------------------------------------------------------------
//	SetData
//-----------------------------------------------------------------------------------
//	Sets some data on our control. Subclasses should override to handle their own
//	defined data tags. If a tag is not understood by the subclass, it should call the
//	inherited method.
//
OSStatus
TView::SetData(
	OSType				inTag,
	ControlPartCode		inPart,
	Size				inSize,
	const void*			inPtr )
{
	#pragma unused( inTag, inPart, inSize, inPtr )

	return eventNotHandledErr;
}

//-----------------------------------------------------------------------------------
//	GetOptimalSize
//-----------------------------------------------------------------------------------
//	Public call to get the optimal size of this view.
//
void
TView::GetOptimalSize(
	HISize*				outSize,
	float*				outBaseLine )
{
	Rect		bounds;
	SInt16		baseLine;
	TRect		frame( Frame() );
	
	bounds = frame;
	
	GetBestControlRect( GetViewRef(), &bounds, &baseLine );
	
	outSize->height = bounds.bottom - bounds.top;
	outSize->width = bounds.right - bounds.left;
	
	if ( outBaseLine )
		*outBaseLine = (float)baseLine;
}

//-----------------------------------------------------------------------------------
//	GetOptimalSizeSelf
//-----------------------------------------------------------------------------------
//	Someone wants to know this view's optimal size and text baseline, probably to help
//	do some type of layout. The base class does nothing, but subclasses should
//	override and do something meaningful here.
//
OSStatus
TView::GetOptimalSizeSelf(
	HISize*				outSize,
	float*				outBaseLine )
{
	#pragma unused( outSize, outBaseLine )

	return eventNotHandledErr;
}

//-----------------------------------------------------------------------------------
//	GetSizeConstraints
//-----------------------------------------------------------------------------------
//	Someone wants to know this view's minimum and maximum sizes, probably to help
//	do some type of layout. The base class does nothing, but subclasses should
//	override and do something meaningful here.
//
OSStatus
TView::GetSizeConstraints(
	HISize*				outMin,
	HISize*				outMax )
{
	#pragma unused( outMin, outMax )

	return eventNotHandledErr;
}

//-----------------------------------------------------------------------------------
//	BoundsChanged
//-----------------------------------------------------------------------------------
//	The bounds of our view have changed. Subclasses can override here to make note
//	of it and flush caches, etc. The base class does nothing.
//
void
TView::BoundsChanged(
	UInt32 				inOptions,
	const HIRect& 		inOriginalBounds,
	const HIRect& 		inCurrentBounds )
{
	#pragma unused( inOptions, inOriginalBounds, inCurrentBounds, inInvalRgn )
}

//-----------------------------------------------------------------------------------
//	ControlHit
//-----------------------------------------------------------------------------------
//	The was hit.  Subclasses can overide to care about what part was hit.
//
OSStatus
TView::ControlHit(
	ControlPartCode		inPart,
	UInt32				inModifiers )
{
	#pragma unused( inPart, inModifiers )

	return eventNotHandledErr;
}

//-----------------------------------------------------------------------------------
//	HiliteChanged
//-----------------------------------------------------------------------------------
//	The hilite of our view has changed. Subclasses can override here to make note
//	of it and flush caches, etc. The base class does nothing.
//
void
TView::HiliteChanged(
	ControlPartCode		inOriginalPart,
	ControlPartCode		inCurrentPart )
{
	#pragma unused( inOriginalPart, inCurrentPart, inInvalRgn )
}

//-----------------------------------------------------------------------------------
//	DragEnter
//-----------------------------------------------------------------------------------
//	A drag has entered our bounds. The Drag and Drop interface also should have been
//	activated or else this method will NOT be called. If true is returned, this view
//	likes the drag and will receive drag within/leave/receive messages as appropriate.
//	If false is returned, it is assumed the drag is not valid for this view, and no
//	further drag activity will flow into this view unless the drag leaves and is
//	re-entered.
//
bool
TView::DragEnter(
	DragRef				inDrag )
{
	#pragma unused( inDrag )

	return false;
}

//-----------------------------------------------------------------------------------
//	DragWithin
//-----------------------------------------------------------------------------------
//	A drag has moved within our bounds. In order for this method to be called, the
//	view must have signaled the drag as being desirable in the DragEnter method. The
//	Drag and Drop interface also should have been activated.
//
bool
TView::DragWithin(
	DragRef				inDrag )
{
	#pragma unused( inDrag )

	return false;
}

//-----------------------------------------------------------------------------------
//	DragLeave
//-----------------------------------------------------------------------------------
//	A drag has left. Deal with it. Subclasses should override as necessary. The
//	Drag and Drop interface should be activated in order for this method to be valid.
//	The drag must have also been accepted in the DragEnter method, else this method
//	will NOT be called.
//
bool
TView::DragLeave(
	DragRef				inDrag )
{
	#pragma unused( inDrag )

	return false;
}

//-----------------------------------------------------------------------------------
//	DragReceive
//-----------------------------------------------------------------------------------
//	Deal with receiving a drag. By default we return dragNotAcceptedErr. I'm not sure
//	if this is correct, or eventNotHandledErr. Time will tell...
//
OSStatus
TView::DragReceive(
	DragRef				inDrag )
{
	#pragma unused( inDrag )

	return dragNotAcceptedErr;
}

//-----------------------------------------------------------------------------------
//	Track
//-----------------------------------------------------------------------------------
//	Default tracking method. Subclasses should override as necessary. We do nothing
//	here in the base class, so we return eventNotHandledErr.
//
OSStatus
TView::Track(
	TCarbonEvent&		inEvent,
	ControlPartCode*	outPart )
{
	#pragma unused( inEvent, outPart )

	return eventNotHandledErr;
}

//-----------------------------------------------------------------------------------
//	SetFocusPart
//-----------------------------------------------------------------------------------
//	Handle focusing. Our base behavior is to punt.
//
OSStatus
TView::SetFocusPart(
	ControlPartCode		inDesiredFocus,
	Boolean				inFocusEverything,
	ControlPartCode*	outActualFocus )
{
	#pragma unused( inDesiredFocus, inFocusEverything, outActualFocus )

	return eventNotHandledErr;
}

//-----------------------------------------------------------------------------------
//	ProcessCommand
//-----------------------------------------------------------------------------------
//	Process a command. Subclasses should override as necessary.
//
OSStatus
TView::ProcessCommand(
	const HICommandExtended&	inCommand )
{
	#pragma unused( inCommand )

	return eventNotHandledErr;
}

//-----------------------------------------------------------------------------------
//	UpdateCommandStatus
//-----------------------------------------------------------------------------------
//	Update the status for a command. Subclasses should override as necessary.
//
OSStatus
TView::UpdateCommandStatus(
	const HICommandExtended&	inCommand )
{
	#pragma unused( inCommand )

	return eventNotHandledErr;
}

//-----------------------------------------------------------------------------------
//	ActivateInterface
//-----------------------------------------------------------------------------------
//	This routine is used to allow a subclass to turn on a specific event or suite of
//	events, like Drag and Drop. This allows us to keep event traffic down if we are
//	not interested, but register for the events if we are.
//
OSStatus
TView::ActivateInterface(
	UInt32			inInterface )
{
	OSStatus		result = noErr;
	
	switch( inInterface )
	{
		case kDragAndDrop:
			{
				static const EventTypeSpec kDragEvents[] =
				{
					{ kEventClassControl, kEventControlDragEnter },			
					{ kEventClassControl, kEventControlDragLeave },			
					{ kEventClassControl, kEventControlDragWithin },			
					{ kEventClassControl, kEventControlDragReceive }
				};
				
				result = AddEventTypesToHandler( GetEventHandler(), GetEventTypeCount( kDragEvents ),
						kDragEvents );
			}
			break;
			
		case kKeyboardFocus:
			{
				static const EventTypeSpec kKeyboardFocusEvents[] =
				{
					{ kEventClassControl, kEventControlSetFocusPart },
					{ kEventClassTextInput, kEventTextInputUnicodeForKeyEvent }
				};
				
				result = AddEventTypesToHandler( GetEventHandler(), GetEventTypeCount( kKeyboardFocusEvents ),
						kKeyboardFocusEvents );
			}
			break;

		case kScrolling:
			{
				static const EventTypeSpec kScrollingEvents[] =
				{
					{ kEventClassScrollable, kEventScrollableGetInfo },
					{ kEventClassScrollable, kEventScrollableScrollTo },
					{ kEventClassControl, kEventControlGetSizeConstraints },
				};
				
				result = AddEventTypesToHandler( GetEventHandler(), GetEventTypeCount( kScrollingEvents ),
						kScrollingEvents );
			}
			break;

		default:
			result = TObject::ActivateInterface( inInterface );
			break;
	}
	
	return result;
}

//-----------------------------------------------------------------------------------
//	HandleEvent
//-----------------------------------------------------------------------------------
//	Per-instance event handler. Override this method to handle additional events.
//	From the overridden method, be sure to call the superclass method to give it a
//	chance to handle events or event classes not handled by this override.
//
OSStatus
TView::HandleEvent(
	EventHandlerCallRef	inCallRef,
	TCarbonEvent&		inEvent )
{
	#pragma unused( inCallRef )

	OSStatus			result = eventNotHandledErr;
	HIPoint				where;
	OSType				tag;
	void *				ptr;
	Size				size, outSize;
	UInt32				features;
	RgnHandle			region = NULL;
	ControlPartCode		part;
	
	switch ( inEvent.GetClass() )
	{
		case kEventClassCommand:
			{
				HICommandExtended		command;
				
				result = inEvent.GetParameter( kEventParamDirectObject, &command );
				require_noerr( result, MissingParameter );
				
				switch ( inEvent.GetKind() )
				{
					case kEventCommandProcess:
						result = ProcessCommand( command );
						break;
					
					case kEventCommandUpdateStatus:
						result = UpdateCommandStatus( command );
						break;
				}
			}
			break;

		case kEventClassControl:
			switch ( inEvent.GetKind() )
			{
				case kEventControlInitialize:
					features = GetBehaviors();
					inEvent.SetParameter( kEventParamControlFeatures, features );
					result = noErr;
					break;
					
				case kEventControlDraw:
					{
						CGContextRef		context = NULL;
						
						inEvent.GetParameter( kEventParamRgnHandle, &region );
						inEvent.GetParameter<CGContextRef>( kEventParamCGContextRef, typeCGContextRef, &context );

						Draw( region, context );
						result = noErr;
					}
					break;
				
				case kEventControlHitTest:
					inEvent.GetParameter<HIPoint>( kEventParamMouseLocation, typeHIPoint, &where );
					part = HitTest( where );
					inEvent.SetParameter<ControlPartCode>( kEventParamControlPart, typeControlPartCode, part );
					result = noErr;
					break;
					
				case kEventControlGetPartRegion:
					inEvent.GetParameter<ControlPartCode>( kEventParamControlPart, typeControlPartCode, &part );
					inEvent.GetParameter( kEventParamControlRegion, &region );
					result = GetRegion( part, region );
					break;
				
				case kEventControlGetData:
					inEvent.GetParameter<ControlPartCode>( kEventParamControlPart, typeControlPartCode, &part );
					inEvent.GetParameter<OSType>( kEventParamControlDataTag, typeEnumeration, &tag );
					inEvent.GetParameter<Ptr>( kEventParamControlDataBuffer, typePtr, (Ptr*)&ptr );
					inEvent.GetParameter<Size>( kEventParamControlDataBufferSize, typeLongInteger, &size );

					result = GetData( tag, part, size, &outSize, ptr );

					if ( result == noErr )
						verify_noerr( inEvent.SetParameter<Size>( kEventParamControlDataBufferSize, typeLongInteger, outSize ) );
					break;
				
				case kEventControlSetData:
					inEvent.GetParameter<ControlPartCode>( kEventParamControlPart, typeControlPartCode, &part );
					inEvent.GetParameter<OSType>( kEventParamControlDataTag, typeEnumeration, &tag );
					inEvent.GetParameter<Ptr>( kEventParamControlDataBuffer, typePtr, (Ptr*)&ptr );
					inEvent.GetParameter<Size>( kEventParamControlDataBufferSize, typeLongInteger, &size );

					result = SetData( tag, part, size, ptr );
					break;
				
				case kEventControlGetOptimalBounds:
					{
						HISize		size;
						float		floatBaseLine;
						
						result = GetOptimalSizeSelf( &size, &floatBaseLine );
						if ( result == noErr )
						{
							Rect		bounds;
							SInt16		baseLine;

							GetControlBounds( GetViewRef(), &bounds );

							bounds.bottom = bounds.top + (SInt16)size.height;
							bounds.right = bounds.left + (SInt16)size.width;
							baseLine = (SInt16)floatBaseLine;
							
							inEvent.SetParameter( kEventParamControlOptimalBounds, bounds );
							inEvent.SetParameter<SInt16>( kEventParamControlOptimalBaselineOffset, typeShortInteger, baseLine );
						}
					}
					break;
				
				case kEventControlBoundsChanged:
					{
						HIRect		prevRect, currRect;
						UInt32		attrs;
						
						inEvent.GetParameter( kEventParamAttributes, &attrs );
						inEvent.GetParameter( kEventParamOriginalBounds, &prevRect );
						inEvent.GetParameter( kEventParamCurrentBounds, &currRect );

						BoundsChanged( attrs, prevRect, currRect );
						
						return noErr;
					}
					break;

				case kEventControlHit:
					{
						UInt32		modifiers;
						
						inEvent.GetParameter<ControlPartCode>( kEventParamControlPart, typeControlPartCode, &part );
						inEvent.GetParameter( kEventParamKeyModifiers, &modifiers );
	
						result = ControlHit( part, modifiers );
					}
					break;
				
				case kEventControlHiliteChanged:
					{
						ControlPartCode	prevPart, currPart;
						
						prevPart = inEvent.GetParameter<ControlPartCode>( kEventParamControlPreviousPart, typeControlPartCode );
						currPart = inEvent.GetParameter<ControlPartCode>( kEventParamControlCurrentPart, typeControlPartCode );

						HiliteChanged( prevPart, currPart );
						
						if ( GetAutoInvalidateFlags() & kAutoInvalidateOnHilite )
							Invalidate();
						
						result = noErr;
					}
					break;
					
				case kEventControlActivate:
					ActiveStateChanged();

					if ( GetAutoInvalidateFlags() & kAutoInvalidateOnActivate )
						Invalidate();
					
					result = noErr;
					break;
					
				case kEventControlDeactivate:
					ActiveStateChanged();

					if ( GetAutoInvalidateFlags() & kAutoInvalidateOnActivate )
						Invalidate();
					
					result = noErr;
					break;
					
				case kEventControlValueFieldChanged:
					ValueChanged();

					if ( GetAutoInvalidateFlags() & kAutoInvalidateOnValueChange )
						Invalidate();
					
					result = noErr;
					break;
					
				case kEventControlTitleChanged:
					TitleChanged();

					if ( GetAutoInvalidateFlags() & kAutoInvalidateOnTitleChange )
						Invalidate();
					
					result = noErr;
					break;
					
				case kEventControlEnabledStateChanged:
					EnabledStateChanged();

					if ( GetAutoInvalidateFlags() & kAutoInvalidateOnEnable )
						Invalidate();
					
					result = noErr;
					break;
					
				case kEventControlOwningWindowChanged:
					{
						WindowRef       oldWindow, newWindow;
						
						inEvent.GetParameter<WindowRef>( kEventParamControlOriginalOwningWindow, typeWindowRef, &oldWindow );
						inEvent.GetParameter<WindowRef>( kEventParamControlCurrentOwningWindow, typeWindowRef, &newWindow );
						
						OwningWindowChanged( oldWindow, newWindow );
						
						result = noErr;
					}
					break;

				case kEventControlVisibilityChanged:
					{
						VisibilityChanged();
						
						result = noErr;
					}
					break;

				case kEventControlDragEnter:
				case kEventControlDragLeave:
				case kEventControlDragWithin:
					{
						DragRef		drag;
						bool		likesDrag;
						
						inEvent.GetParameter( kEventParamDragRef, &drag );

						switch ( inEvent.GetKind() )
						{
							case kEventControlDragEnter:
								likesDrag = DragEnter( drag );
								
								result = inEvent.SetParameter( kEventParamControlLikesDrag, likesDrag );
								break;
							
							case kEventControlDragLeave:
								likesDrag = DragLeave( drag );
								result = inEvent.SetParameter( kEventParamControlLikesDrag, likesDrag );
								break;
							
							case kEventControlDragWithin:
								likesDrag = DragWithin( drag );
								result = inEvent.SetParameter( kEventParamControlLikesDrag, likesDrag );
								break;
						}
					}
					break;
				
				case kEventControlDragReceive:
					{
						DragRef		drag;
						
						inEvent.GetParameter( kEventParamDragRef, &drag );

						result = DragReceive( drag );
					}
					break;
				
				case kEventControlTrack:
					{
						ControlPartCode		part;
						
						result = Track( inEvent, &part );
						if ( result == noErr )
							verify_noerr( inEvent.SetParameter<ControlPartCode>( kEventParamControlPart, typeControlPartCode, part ) );
					}
					break;

				case kEventControlGetSizeConstraints:
					{
						HISize		minSize, maxSize;
						
						result = GetSizeConstraints( &minSize, &maxSize );

						if ( result == noErr )
						{
							verify_noerr( inEvent.SetParameter( kEventParamMinimumSize, minSize ) );
							verify_noerr( inEvent.SetParameter( kEventParamMaximumSize, maxSize ) );
						}
					}
					break;

				case kEventControlSetFocusPart:
					{
						ControlPartCode		desiredFocus;
						Boolean				focusEverything;
						ControlPartCode		actualFocus;
						
						result = inEvent.GetParameter<ControlPartCode>( kEventParamControlPart, typeControlPartCode, &desiredFocus ); 
						require_noerr( result, MissingParameter );
						
						focusEverything = false; // a good default in case the parameter doesn't exist

						inEvent.GetParameter( kEventParamControlFocusEverything, &focusEverything );

						result = SetFocusPart( desiredFocus, focusEverything, &actualFocus );
						
						if ( result == noErr )
							verify_noerr( inEvent.SetParameter<ControlPartCode>( kEventParamControlPart, typeControlPartCode, actualFocus ) );
					}
					break;
				
				// some other kind of Control event
				default:
					// If you are reading this code to model your subclass, this would be a good
					// place to call your superclasses' method. TObject::HandleEvent doesn't care
					// about control events, so we don't pass the buck, but you should if you
					// are subclassing TView, calling TView::HandleEvent for your unhandled events
					break;
			}
			break;
			
		case kEventClassTextInput:
			result = TextInput( inEvent );
			break;
			
		case kEventClassScrollable:
			switch( inEvent.GetKind() )
			{
				case kEventScrollableGetInfo:
					{
						HISize	imageSize, viewSize, lineSize;
						HIPoint	origin;
						
						result = ScrollableGetInfo( &imageSize, &viewSize, &lineSize, &origin );
						if ( result == noErr )
						{
							result = inEvent.SetParameter( kEventParamImageSize, typeHISize, imageSize );
							require_noerr( result, MissingParameter );

							result = inEvent.SetParameter( kEventParamViewSize, typeHISize, viewSize );
							require_noerr( result, MissingParameter );

							result = inEvent.SetParameter( kEventParamLineSize, typeHISize, lineSize );
							require_noerr( result, MissingParameter );

							result = inEvent.SetParameter( kEventParamOrigin, typeHIPoint, origin );
							require_noerr( result, MissingParameter );
						}
					}
					break;

				case kEventScrollableScrollTo:
					{
						HIPoint	newOrigin;

						result = inEvent.GetParameter( kEventParamOrigin, &newOrigin );
						require_noerr( result, MissingParameter );

						result = ScrollableScrollTo( &newOrigin );
					}
					break;

				default:
					result = TObject::HandleEvent( inCallRef, inEvent );
					break;
			}
			break;

		default:
			// Some other event class, let the superclass handle it
			result = TObject::HandleEvent( inCallRef, inEvent );
			break;
	}

MissingParameter:
	return result;
}

//-----------------------------------------------------------------------------------
//	Frame
//-----------------------------------------------------------------------------------
//
HIRect
TView::Frame()
{
	HIRect		frame;

	HIViewGetFrame( GetViewRef(), &frame );
	
	return frame;
}

//-----------------------------------------------------------------------------------
//	SetFrame
//-----------------------------------------------------------------------------------
//
void
TView::SetFrame(
	const HIRect&			inFrame )
{
	verify_noerr( HIViewSetFrame( GetViewRef(), &inFrame ) );
}

//-----------------------------------------------------------------------------------
//	Bounds
//-----------------------------------------------------------------------------------
//
HIRect
TView::Bounds()
{
	HIRect		bounds;
	
	verify_noerr( HIViewGetBounds( GetViewRef(), &bounds ) );
	
	return bounds;
}

//-----------------------------------------------------------------------------------
//	Show
//-----------------------------------------------------------------------------------
//
void
TView::Show()
{
	verify_noerr( HIViewSetVisible( GetViewRef(), true ) );
}

//-----------------------------------------------------------------------------------
//	Hide
//-----------------------------------------------------------------------------------
//
void
TView::Hide()
{
	verify_noerr( HIViewSetVisible( GetViewRef(), false ) );
}

//-----------------------------------------------------------------------------------
//	AddSubView
//-----------------------------------------------------------------------------------
//
OSStatus
TView::AddSubView(
	TView*				inSubView )
{
	return HIViewAddSubview( GetViewRef(), inSubView->GetViewRef() );;
}

//-----------------------------------------------------------------------------------
//	RemoveFromSuperView
//-----------------------------------------------------------------------------------
//
OSStatus
TView::RemoveFromSuperView()
{
	return HIViewRemoveFromSuperview( GetViewRef() );
}

//-----------------------------------------------------------------------------------
//	GetHilite
//-----------------------------------------------------------------------------------
//
ControlPartCode
TView::GetHilite()
{
	return GetControlHilite( GetViewRef() );
}

//-----------------------------------------------------------------------------------
//	GetValue
//-----------------------------------------------------------------------------------
//
SInt32
TView::GetValue()
{
	return GetControl32BitValue( GetViewRef() );
}

//-----------------------------------------------------------------------------------
//	SetValue
//-----------------------------------------------------------------------------------
//
void
TView::SetValue(
	SInt32					inValue )
{
	SetControl32BitValue( GetViewRef(), inValue );
}

//-----------------------------------------------------------------------------------
//	GetMinimum
//-----------------------------------------------------------------------------------
//
SInt32
TView::GetMinimum()
{
	return GetControl32BitMinimum( GetViewRef() );
}

//-----------------------------------------------------------------------------------
//	SetMinimum
//-----------------------------------------------------------------------------------
//
void
TView::SetMinimum(
	SInt32					inMinimum )
{
	SetControl32BitMinimum( GetViewRef(), inMinimum );
}

//-----------------------------------------------------------------------------------
//	GetMaximum
//-----------------------------------------------------------------------------------
//
SInt32
TView::GetMaximum()
{
	return GetControl32BitMaximum( GetViewRef() );
}

//-----------------------------------------------------------------------------------
//	SetMaximum
//-----------------------------------------------------------------------------------
//
void
TView::SetMaximum(
	SInt32					inMaximum )
{
	SetControl32BitMaximum( GetViewRef(), inMaximum );
}

//-----------------------------------------------------------------------------------
//	GetOwner
//-----------------------------------------------------------------------------------
//
WindowRef
TView::GetOwner()
{
	return GetControlOwner( GetViewRef() );
}

//-----------------------------------------------------------------------------------
//	Hilite
//-----------------------------------------------------------------------------------
//
void
TView::Hilite(
	ControlPartCode			inPart )
{
	return HiliteControl( GetViewRef(), inPart );
}

//-----------------------------------------------------------------------------------
//	Invalidate
//-----------------------------------------------------------------------------------
//
OSStatus
TView::Invalidate()
{
	return HIViewSetNeedsDisplay( GetViewRef(), true );
}

//-----------------------------------------------------------------------------------
//	IsVisible
//-----------------------------------------------------------------------------------
//
Boolean
TView::IsVisible()
{
	return IsControlVisible( GetViewRef() );
}

//-----------------------------------------------------------------------------------
//	IsEnabled
//-----------------------------------------------------------------------------------
//
Boolean
TView::IsEnabled()
{
	return IsControlEnabled( GetViewRef() );
}

//-----------------------------------------------------------------------------------
//	IsActive
//-----------------------------------------------------------------------------------
//
Boolean
TView::IsActive()
{
	return IsControlActive( GetViewRef() );
}

//-----------------------------------------------------------------------------------
//	ActiveStateChanged
//-----------------------------------------------------------------------------------
//	Default activation method. Subclasses should override as necessary. We do nothing
//	here in the base class.
//
void
TView::ActiveStateChanged()
{
}

//-----------------------------------------------------------------------------------
//	ValueChanged
//-----------------------------------------------------------------------------------
//	Default value changed method. Subclasses should override as necessary. We do
//	nothing here in the base class.
//
void
TView::ValueChanged()
{
}

//-----------------------------------------------------------------------------------
//	TitleChanged
//-----------------------------------------------------------------------------------
//	Default title changed method. Subclasses should override as necessary. We
//	do nothing here in the base class.
//
void
TView::TitleChanged()
{
}

//-----------------------------------------------------------------------------------
//	EnabledStateChanged
//-----------------------------------------------------------------------------------
//	Default enable method. Subclasses should override as necessary. We
//	do nothing here in the base class.
//
void
TView::EnabledStateChanged()
{
}

//-----------------------------------------------------------------------------------
//	OwningWindowChanged
//-----------------------------------------------------------------------------------
//	Default owning window changed method. Subclasses should override as necessary.
//	We do nothing here in the base class.
//
void
TView::OwningWindowChanged(
    WindowRef               oldWindow,
    WindowRef               newWindow )
{
}

//-----------------------------------------------------------------------------------
//	VisibilityChanged
//-----------------------------------------------------------------------------------
//	Default title changed method. Subclasses should override as necessary. We
//	do nothing here in the base class.
//
void
TView::VisibilityChanged()
{
}

//-----------------------------------------------------------------------------------
//	TextInput
//-----------------------------------------------------------------------------------
//	Default text (Unicode) input method. Subclasses should override as necessary. We
//	do nothing here in the base class, so we return eventNotHandledErr.
//
OSStatus
TView::TextInput(
	TCarbonEvent&		inEvent )
{
	#pragma unused( inEvent )

	return eventNotHandledErr;
}

//-----------------------------------------------------------------------------------
//	ChangeAutoInvalidateFlags
//-----------------------------------------------------------------------------------
//	Change behavior for auto-invalidating views on certain actions.
//
void
TView::ChangeAutoInvalidateFlags(
	OptionBits			inSetThese,
	OptionBits			inClearThese )
{
    fAutoInvalidateFlags = ( ( fAutoInvalidateFlags | inSetThese ) & ( ~inClearThese ) );
}

//-----------------------------------------------------------------------------------
//	PrintDebugInfoSelf
//-----------------------------------------------------------------------------------
//	Prints debugging info specific to this object. Subclasses should call the
//	inherited method at the end of handling this so that superclass debugging info
//	also gets printed out.
//
void
TView::PrintDebugInfoSelf()
{
	fprintf( stdout, "TView\n" );
	
	TObject::PrintDebugInfoSelf();
}

// -----------------------------------------------------------------------------
//	ScrollableGetInfo
// -----------------------------------------------------------------------------
//
OSStatus
TView::ScrollableGetInfo(
	HISize*				ioImageSize,
	HISize*				ioViewSize,
	HISize*				ioLineSize,
	HIPoint*			ioOrigin )
{
	return eventNotHandledErr;
}

// -----------------------------------------------------------------------------
//	ScrollableScrollTo
// -----------------------------------------------------------------------------
//
OSStatus
TView::ScrollableScrollTo(
	const HIPoint*			inScrollTo )
{
	return eventNotHandledErr;
}

// -----------------------------------------------------------------------------
//	SendScrollableInfoChanged
// -----------------------------------------------------------------------------
//
OSStatus
TView::SendScrollableInfoChanged()
{
	OSStatus		err;
	EventRef		event;

	err = CreateEvent( NULL, kEventClassScrollable, kEventScrollableInfoChanged, 0, 0, &event );
	require_noerr( err, CantCreateEvent );

	err = SendEventToEventTargetWithOptions( event, GetControlEventTarget( HIViewGetSuperview( GetViewRef() ) ), kEventTargetDontPropagate );
	ReleaseEvent( event );

CantCreateEvent:
	return err;
}
