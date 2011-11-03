/*
 *  Chip700View.h
 *  Chip700
 *
 *  Created by 開発用 on 06/11/30.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __Chip700View__H_
#define __Chip700View__H_

#include "Chip700Version.h"
#include "AUCarbonViewNib.h"
#include "Chip700defines.h"

typedef struct {
	int		basekey,lowkey,highkey,loop,lp,lp_end;
	double	srcSamplerate;
}InstData;

static OSErr MyDragSendDataFunction(FlavorType theType, void *dragSendRefCon,
									DragItemRef theItemRef, DragRef theDrag);

class Chip700View: public CAUCarbonViewNib
{
public:
    // Construction ------------------------------
    Chip700View(AudioUnitCarbonView inInstance): CAUCarbonViewNib(inInstance)
	{
		mEventDisable=false;
		preemphasis=true;
		shouldload=false;
	};
	~Chip700View();
    // Baseclass overrides -----------------------
    CFStringRef GetBundleID() { return CFSTR("com.VeMa.audiounit.Chip700"); };  // my Bundle ID 
    void InitWindow(CFBundleRef sBundle);
	virtual void FinishWindow(CFBundleRef sBundle);
	virtual bool HandleCommand(EventRef	inEvent, HICommandExtended &cmd);
	virtual bool HandleEventForView(EventRef event, HIViewRef view);
	virtual int GetFontsize() { return 10; }
	virtual int GetTextJustification() { return teJustCenter; }
	virtual CFStringRef GetFontname() { return CFSTR("Monaco"); }
	virtual void PropertyHasChanged(AudioUnitPropertyID inPropertyID, AudioUnitScope inScope,  
                                    AudioUnitElement inElement);
	virtual void Idle();
	
	void applyEditTextProp(ControlRef control);
	void enqueueFile(FSRef *ref);
	void loadFile(FSRef *ref);
	void loadFile(CFURLRef path);
	void saveToFile(CFURLRef savefile);
	void saveToXIFile(CFURLRef savefile);
private:
	static pascal ControlKeyFilterResult StdKeyFilterCallback(ControlRef theControl, 
																		SInt16 *keyCode, SInt16 *charCode, 
																		EventModifiers *modifiers);
	static pascal ControlKeyFilterResult NumericKeyFilterCallback(ControlRef theControl, 
																		SInt16 *keyCode, SInt16 *charCode, 
																		EventModifiers *modifiers);
	
	void setBRRData(UInt8 *data, UInt32 length);
	void setLoopoint(UInt32 lp);
	
	void correctSampleRateSelected(void);
	void correctBaseKeySelected(void);
	void loadSelected(void);
	void saveSelected(void);
	bool dragStart(ControlRef cont, EventRecord *event);
	int getLoadFile(FSRef *ref);
	CFURLRef getSaveFile(CFStringRef defaultName);
	short* loadPCMFile(FSRef *ref, long *numSamples, InstData *inst);
	int loadSPCFile(CFURLRef path);
	
	
	HIViewRef	hiOverView,hiTailView,hiHeadView;
	bool	preemphasis;
	bool	mEventDisable;
	
	FSRef	queingfile;
	bool	shouldload;
	
	friend OSErr MyDragSendDataFunction(FlavorType theType, void *dragSendRefCon,
								  DragItemRef theItemRef, DragRef theDrag);
};

#endif
