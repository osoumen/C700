/*
 *  TMultiPane.h
 *  NeuSynth
 *
 *  Created by Airy on Wed Mar 12 2003.
 *  Copyright (c) 2003 Airy ANDRE. All rights reserved.
 *
 */
#ifndef _TMultiPane_h_
#define _TMultiPane_h_

#include "TViewNoCompositingCompatible.h"

/*!
* \class TMultiPane
 * \brief A Pane that switch its content according to its value.

 */
class TMultiPane : public TViewNoCompositingCompatible
{
	AUGUIDefineControl(TMultiPane, TViewNoCompositingCompatible, "multipane");
public:
	static const OSType kControlKind = 'Mpan';
	static const OSType kSubPanelSignature = 'SubP';
	static const OSType kSubPanelID = 'SbID';
protected:
	// Constructor/Destructor
	TMultiPane(HIViewRef inControl);
	virtual ~TMultiPane();

	virtual ControlKind GetKind();

	virtual OSStatus Initialize(TCarbonEvent& inEvent);
	virtual void		Draw(RgnHandle	inLimitRgn, CGContextRef inContext );
	virtual void CompatibleDraw(RgnHandle inLimitRgn, CGContextRef inContext, bool inCompositing);
	virtual void ValueChanged();

        virtual OSStatus SetData(OSType inTag, ControlPartCode inPart, Size inSize, const void* inPtr);
	virtual OSStatus GetData(OSType inTag, ControlPartCode inPart, Size inSize, Size* outSize, void* inPtr);

	virtual UInt32 GetBehaviors() { return TViewNoCompositingCompatible::GetBehaviors() | kControlSupportsEmbedding; }

private:
        int mID;
};


#endif // _TMultiPane_h_
