/*
 *  TWaveView.h
 */

#ifndef TWaveView_H_
#define TWaveView_H_

#include "TViewNoCompositingCompatible.h"

enum {
	//! A CGImageRef that you wish displayed in the control.
	kWaveDataTag = 'Wave',
	kLoopPointTag = 'Loop',
	kShowMsgTag = 'SMsg',
};

class TWaveView
: public TViewNoCompositingCompatible
{
	AUGUIDefineControl(TWaveView, TViewNoCompositingCompatible, "waveview");
protected:
	// Constructor/Destructor
	TWaveView(HIViewRef inControl);
    virtual ~TWaveView();

    virtual ControlKind		GetKind();

    virtual OSStatus		Initialize( TCarbonEvent& inEvent );
    virtual void		DrawStaticBackground(RgnHandle	inLimitRgn, CGContextRef inContext, bool  inCompositing );
    virtual void		CompatibleDraw(RgnHandle	inLimitRgn, CGContextRef inContext,bool  inCompositing );
    virtual void		TitleChanged();
    virtual UInt32			GetBehaviors();

    virtual OSStatus SetData(OSType inTag, ControlPartCode inPart, Size inSize, const void* inPtr);
    virtual OSStatus GetData(OSType inTag, ControlPartCode inPart, Size inSize, Size* outSize, void* inPtr);
	
private:
	CGMutablePathRef	path;
	bool				loaded;
	bool				converting;
	int					datanum;
	int					looppoint;
};

#endif // 