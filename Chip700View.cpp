/*
 *  Chip700View.cpp
 *  Chip700
 *
 *  Created by on 06/11/30.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include <AudioToolbox/AudioToolbox.h>
#include "Chip700View.h"
#include "brrcodec.h"
#include "czt.h"
#include "TDragTextControl.h"
#include "TWaveView.h"
#include "TImageCache.h"

static Boolean MyFilterProc(AEDesc *theItem, void *info, void *callBackUD, 
					 NavFilterModes filterMode);
static void LittleArrowsControlAction(ControlRef theControl, ControlPartCode partCode);
static OSErr MyTrackingHandler(DragTrackingMessage theMessage, WindowRef win,
							   void *handlerRefCon, DragRef theDrag);
static OSErr MyReceiveHandler(WindowRef win, void *handlerRefCon, 
							  DragRef theDrag);

bool DragItemsAreAcceptable(DragReference theDrag);


COMPONENT_ENTRY(Chip700View)

void Chip700View::InitWindow(CFBundleRef sBundle)
{
	CAUCarbonViewNib::InitWindow(sBundle);
    
    TViewNoCompositingCompatible::RegisterClassForBundleID<TDragTextControl>(GetBundleID());
	TViewNoCompositingCompatible::RegisterClassForBundleID<TWaveView>(GetBundleID());
}

void Chip700View::FinishWindow(CFBundleRef sBundle)
{
	HIViewRef	control;
	HIViewID	id = {'user', 0};
	OSStatus	result;
	
	OSStatus styleOK = 0;
    ControlFontStyleRec textStyle = { 0 };
    if (GetFontname()) {
        char fname[255];
        CFStringGetCString(GetFontname(), fname+1, sizeof(fname)-1, kCFStringEncodingASCII);
        *fname = strlen(fname+1);
        textStyle.flags = kControlUseFontMask | kControlUseSizeMask | kControlUseForeColorMask | kControlUseJustMask;
        textStyle.font = FMGetFontFamilyFromName((const unsigned char *)fname);
        textStyle.size = GetFontsize();
        textStyle.just = teJustRight;
        textStyle.foreColor.red = 180*256;
		textStyle.foreColor.green = 248*256;
		textStyle.foreColor.blue = 255*256;
        styleOK = 1;
    }
	
	//コントロール操作イベントの登録
	for (int j=0; j<=1000; j+=1000) {
		for (int i=0; i<kNumberOfProperties; i++) {
			id.id=i+j;
			result = HIViewFindByID(mRootUserPane, id, &control);
			if (result == noErr) {
				SetControlReference(control, SInt32(this));
				
				if (styleOK) {
					if (i==0) {
						textStyle.just = teJustLeft;
						textStyle.foreColor.red = 0;
						textStyle.foreColor.green = 0;
						textStyle.foreColor.blue = 0;
					}
					else {
						textStyle.just = teJustRight;
						textStyle.foreColor.red = 180*256;
						textStyle.foreColor.green = 248*256;
						textStyle.foreColor.blue = 255*256;
					}
					SetControlData(control,0, kControlFontStyleTag, sizeof(textStyle), &textStyle);
				}
				
				ControlKind	thekind;
				GetControlKind(control,&thekind);
				if (thekind.kind == 'eutx') {
					EventTypeSpec events[] = {
					{kEventClassControl, kEventControlSetFocusPart}
					};
					WantEventTypes(GetControlEventTarget(control), GetEventTypeCount(events), events);
					ControlKeyFilterUPP proc = i?NumericKeyFilterCallback:StdKeyFilterCallback;
					SetControlData(control, 0, kControlEditTextKeyFilterTag, sizeof(proc), &proc);
				}
				else {
					EventTypeSpec events[] = {
					{kEventClassControl, kEventControlValueFieldChanged}
					};
					WantEventTypes(GetControlEventTarget(control), GetEventTypeCount(events), events);
				}
				if (thekind.kind == 'larr') {
					SetControlAction(control, LittleArrowsControlAction);
				}
			}
		}
	}
	EventTypeSpec clickevents[] = {
	{kEventClassControl, kEventControlClick}
	};
	id.id=1;
	HIViewFindByID(mRootUserPane, id, &hiOverView);
	id.id=1001;
	HIViewFindByID(mRootUserPane, id, &hiTailView);
	id.id=2001;
	HIViewFindByID(mRootUserPane, id, &hiHeadView);
	WantEventTypes(GetControlEventTarget(hiOverView), GetEventTypeCount(clickevents), clickevents);
	WantEventTypes(GetControlEventTarget(hiTailView), GetEventTypeCount(clickevents), clickevents);
	WantEventTypes(GetControlEventTarget(hiHeadView), GetEventTypeCount(clickevents), clickevents);
	
	InstallTrackingHandler((DragTrackingHandlerUPP)MyTrackingHandler, mCarbonWindow, this);
	InstallReceiveHandler((DragReceiveHandlerUPP)MyReceiveHandler, mCarbonWindow, this);
	
	//プロパティ変更イベントの登録
	for (int i=0; i<kNumberOfProperties; i++) {
		RegisterPropertyChanges(kAudioUnitCustomProperty_First+i);
		PropertyHasChanged(kAudioUnitCustomProperty_First+i,kAudioUnitScope_Global,0);
	}

}

Chip700View::~Chip700View()
{
	RemoveTrackingHandler((DragTrackingHandlerUPP)MyTrackingHandler, mCarbonWindow);
	RemoveReceiveHandler((DragReceiveHandlerUPP)MyReceiveHandler, mCarbonWindow);
}

bool Chip700View::HandleCommand(EventRef inEvent, HICommandExtended &cmd)
{
	switch (cmd.commandID) {
		case 'emph':	//emphasise on/off
			preemphasis = GetControl32BitValue(cmd.source.control)?true:false;
			return true;
			
		case 'load':	//load button
			loadSelected();
			return true;
			
		case 'save':	//save button
			saveSelected();
			return true;
			
		case 'unlo':	//unload button
		{
			BRRData		brr;
			brr.data=NULL;
			AudioUnitSetProperty(mEditAudioUnit,kAudioUnitCustomProperty_BRRData,
								 kAudioUnitScope_Global,0,&brr,sizeof(BRRData));
			return true;
		}
			
		case 'drat':	//detect SampleRate
			correctSampleRateSelected();
			return true;
			
		case 'dkey':	//detect Key
			correctBaseKeySelected();
			return true;
	}
	return false;
}

bool Chip700View::HandleEventForView(EventRef event, HIViewRef view)
{
	TCarbonEvent theEvent = event;
    UInt32 eclass = theEvent.GetClass();
    UInt32 ekind = theEvent.GetKind();
	HIViewID	id;
	int			intValue;
	AudioUnitPropertyID propertyID;
	
	if (mEventDisable)
		return false;
	
	if (eclass == kEventClassControl) {
		HIViewGetID(view,&id);
		propertyID = (id.id%1000)+kAudioUnitCustomProperty_First;
		if (id.signature == 'user') {
			switch (ekind) {
				case kEventControlClick:
				{
					if (propertyID == kAudioUnitCustomProperty_BRRData) {
					//ドラッグ処理
					EventRecord	eventrec;
					if (!ConvertEventRefToEventRecord(event,&eventrec))
						return dragStart(view, &eventrec);
					else
						return false;
					}
					else return false;
				}
					
				case kEventControlValueFieldChanged:
					switch (propertyID) {
						case kAudioUnitCustomProperty_Loop:
						{
							intValue = HIViewGetValue(view);
							bool param = intValue?true:false;
							AudioUnitSetProperty(mEditAudioUnit,kAudioUnitCustomProperty_Loop,
												 kAudioUnitScope_Global,0,&param,sizeof(bool));
							break;
						}
							
						case kAudioUnitCustomProperty_LoopPoint:
							intValue = HIViewGetValue(view);
							intValue = intValue/16*9;
							AudioUnitSetProperty(mEditAudioUnit,propertyID,
												 kAudioUnitScope_Global,0,&intValue,sizeof(int));
							break;
							
						case kAudioUnitCustomProperty_AR:
						case kAudioUnitCustomProperty_DR:
						case kAudioUnitCustomProperty_SL:
						case kAudioUnitCustomProperty_SR:
						case kAudioUnitCustomProperty_VolL:
						case kAudioUnitCustomProperty_VolR:
						case kAudioUnitCustomProperty_EditingProgram:
							intValue = HIViewGetValue(view);
							AudioUnitSetProperty(mEditAudioUnit,propertyID,
												 kAudioUnitScope_Global,0,&intValue,sizeof(int));
							break;
					}
					return true;
					
				case kEventControlSetFocusPart:
					applyEditTextProp(view);
					return false;
			}
		}
	}
	return false;
}

void Chip700View::applyEditTextProp(ControlRef control)
{
	HIViewID	id;
	CFStringRef	cstr;
	int			intValue;
	double		doubleValue;
	AudioUnitPropertyID propertyID;
	
	HIViewGetID(control,&id);
	propertyID = (id.id%1000)+kAudioUnitCustomProperty_First;
	cstr=HIViewCopyText(control);
	switch (propertyID) {
		case kAudioUnitCustomProperty_ProgramName:
			AudioUnitSetProperty(mEditAudioUnit,propertyID,
								 kAudioUnitScope_Global,0,&cstr,sizeof(CFStringRef));
			break;
			
		case kAudioUnitCustomProperty_Rate:
			doubleValue = CFStringGetDoubleValue(cstr);
			AudioUnitSetProperty(mEditAudioUnit,propertyID,
								 kAudioUnitScope_Global,0,&doubleValue,sizeof(double));
			break;
			
		case kAudioUnitCustomProperty_BaseKey:
		case kAudioUnitCustomProperty_LowKey:
		case kAudioUnitCustomProperty_HighKey:
		case kAudioUnitCustomProperty_EditingProgram:
			intValue = CFStringGetIntValue(cstr);
			AudioUnitSetProperty(mEditAudioUnit,propertyID,
								 kAudioUnitScope_Global,0,&intValue,sizeof(int));
			break;
			
		case kAudioUnitCustomProperty_LoopPoint:
			intValue = CFStringGetIntValue(cstr)/16*9;
			AudioUnitSetProperty(mEditAudioUnit,propertyID,
								 kAudioUnitScope_Global,0,&intValue,sizeof(int));
			break;
	}
	CFRelease(cstr);
}

void Chip700View::PropertyHasChanged(AudioUnitPropertyID inPropertyID, AudioUnitScope inScope,  
						AudioUnitElement inElement)
{
	OSStatus	result;
	HIViewRef	control;
	HIViewID	id = {'user', 0};
	UInt32		size;
	int			intValue;
	SInt32		maximum,minimum,cval;
	
	mEventDisable = true;
	
	switch (inPropertyID) {
		case kAudioUnitCustomProperty_ProgramName:
		{
			CFStringRef	pgstr;
			size = sizeof(CFStringRef);
			AudioUnitGetProperty(mEditAudioUnit,kAudioUnitCustomProperty_ProgramName,
								 kAudioUnitScope_Global,0,&pgstr,&size);
			id.id=inPropertyID-kAudioUnitCustomProperty_First;
			result = HIViewFindByID(mRootUserPane, id, &control);
			if (result == noErr) {
				if (pgstr)
					HIViewSetText(control, pgstr);
				else
					HIViewSetText(control, CFSTR(""));
			}
			break;
		}
			
		case kAudioUnitCustomProperty_BRRData:
		{
			BRRData	brr;
			size = sizeof(BRRData);
			AudioUnitGetProperty(mEditAudioUnit,kAudioUnitCustomProperty_BRRData,
								 kAudioUnitScope_Global,0,&brr,&size);
			setBRRData(brr.data, brr.size);
			id.id=6;
			result = HIViewFindByID(mRootUserPane, id, &control);
			if (result == noErr) {
				HIViewSetMaximum(control,brr.size/9*16);
			}
			break;
		}
			
		case kAudioUnitCustomProperty_Rate:
		{
			double	doubleValue;
			size = sizeof(double);
			AudioUnitGetProperty(mEditAudioUnit,kAudioUnitCustomProperty_Rate,
								 kAudioUnitScope_Global,0,&doubleValue,&size);
			id.id=inPropertyID-kAudioUnitCustomProperty_First;
			result = HIViewFindByID(mRootUserPane, id, &control);
			if (result == noErr) {
				CFStringRef	cfstr=CFStringCreateWithFormat(NULL,NULL,CFSTR("%.03f"),doubleValue);
				HIViewSetText(control, cfstr);
			}
			break;
		}
			
		case kAudioUnitCustomProperty_LoopPoint:
			size = sizeof(int);
			AudioUnitGetProperty(mEditAudioUnit,kAudioUnitCustomProperty_LoopPoint,
								 kAudioUnitScope_Global,0,&intValue,&size);
			id.id=inPropertyID-kAudioUnitCustomProperty_First;
			result = HIViewFindByID(mRootUserPane, id, &control);
			if (result == noErr) {
				HIViewSetValue(control, intValue/9*16);
			}
			setLoopoint(intValue);
			id.id=inPropertyID-kAudioUnitCustomProperty_First+1000;
			result = HIViewFindByID(mRootUserPane, id, &control);
			if (result == noErr) {
				CFStringRef	cfstr=CFStringCreateWithFormat(NULL,NULL,CFSTR("%d"),intValue/9*16);
				HIViewSetText(control, cfstr);
			}
			break;
			
		case kAudioUnitCustomProperty_Loop:
		{
			bool looping;
			size = sizeof(bool);
			AudioUnitGetProperty(mEditAudioUnit,kAudioUnitCustomProperty_Loop,
								 kAudioUnitScope_Global,0,&looping,&size);
			id.id=inPropertyID-kAudioUnitCustomProperty_First;
			result = HIViewFindByID(mRootUserPane, id, &control);
			if (result == noErr) {
				maximum = GetControl32BitMaximum(control);
				minimum = GetControl32BitMinimum(control);
				cval = looping?maximum:minimum;
				HIViewSetValue(control, cval);
			}
			break;
		}
		
		case kAudioUnitCustomProperty_BaseKey:
		case kAudioUnitCustomProperty_LowKey:
		case kAudioUnitCustomProperty_HighKey:
			size = sizeof(int);
			AudioUnitGetProperty(mEditAudioUnit,inPropertyID,kAudioUnitScope_Global,0,&intValue,&size);
			id.id=inPropertyID-kAudioUnitCustomProperty_First;
			result = HIViewFindByID(mRootUserPane, id, &control);
			if (result == noErr) {
				HIViewSetValue(control, intValue);
				cval = GetControl32BitValue(control);
				CFStringRef	cfstr=CFStringCreateWithFormat(NULL,NULL,CFSTR("%d"),cval);
				HIViewSetText(control, cfstr);
			}
			break;
			
		case kAudioUnitCustomProperty_AR:
		case kAudioUnitCustomProperty_DR:
		case kAudioUnitCustomProperty_SL:
		case kAudioUnitCustomProperty_SR:
		case kAudioUnitCustomProperty_VolL:
		case kAudioUnitCustomProperty_VolR:
			size = sizeof(int);
			AudioUnitGetProperty(mEditAudioUnit,inPropertyID,kAudioUnitScope_Global,0,&intValue,&size);
			id.id=inPropertyID-kAudioUnitCustomProperty_First;
			result = HIViewFindByID(mRootUserPane, id, &control);
			if (result == noErr) {
				HIViewSetValue(control, intValue);
			}
			id.id=inPropertyID-kAudioUnitCustomProperty_First+1000;
			result = HIViewFindByID(mRootUserPane, id, &control);
			if (result == noErr) {
				HIViewSetValue(control, intValue);
			}
			break;
			
		case kAudioUnitCustomProperty_EditingProgram:
			size = sizeof(int);
			AudioUnitGetProperty(mEditAudioUnit,inPropertyID,kAudioUnitScope_Global,0,&intValue,&size);
			id.id=inPropertyID-kAudioUnitCustomProperty_First;
			result = HIViewFindByID(mRootUserPane, id, &control);
			if (result == noErr) {
				HIViewSetValue(control, intValue);
			}
			id.id=inPropertyID-kAudioUnitCustomProperty_First+1000;
			result = HIViewFindByID(mRootUserPane, id, &control);
			if (result == noErr) {
				CFStringRef	cfstr=CFStringCreateWithFormat(NULL,NULL,CFSTR("%d"),intValue);
				HIViewSetText(control, cfstr);
			}
			break;
	}
	mEventDisable = false;
}


//
// private function
//

pascal ControlKeyFilterResult Chip700View::StdKeyFilterCallback(ControlRef theControl, 
																		  SInt16 *keyCode, SInt16 *charCode, 
																		  EventModifiers *modifiers)
{
	SInt16 c = *charCode;
	Chip700View *This = (Chip700View*)GetControlReference(theControl);
	if (c >= ' ' || c == '\b' || c == 0x7F || (c >= 0x1c && c <= 0x1f) || c == '\t')
		return kControlKeyFilterPassKey;
	if (c == '\r' || c == 3) {	// return or Enter
		ControlEditTextSelectionRec sel = { 0, 32767 };
		SetControlData(theControl, 0, kControlEditTextSelectionTag, sizeof(sel), &sel);
		This->applyEditTextProp(theControl);
	}
	return kControlKeyFilterBlockKey;
}

pascal ControlKeyFilterResult Chip700View::NumericKeyFilterCallback(ControlRef theControl, 
																			  SInt16 *keyCode, SInt16 *charCode, 
																			  EventModifiers *modifiers)
{
	SInt16 c = *charCode;
	Chip700View *This = (Chip700View*)GetControlReference(theControl);
	if (isdigit(c) || c == '+' || c == '-' || c == '.' || c == '\b' || c == 0x7F || (c >= 0x1c && c <= 0x1f)
		|| c == '\t')
		return kControlKeyFilterPassKey;
	if (c == '\r' || c == 3) {	// return or Enter
		ControlEditTextSelectionRec sel = { 0, 32767 };
		SetControlData(theControl, 0, kControlEditTextSelectionTag, sizeof(sel), &sel);
		This->applyEditTextProp(theControl);
	}
	return kControlKeyFilterBlockKey;
}

//波形を表示する
void Chip700View::setBRRData(UInt8 *data, UInt32 length)
{
	HIRect	bounds;
	long	start,viewlength;
	
	short	*wavedata;
	long	numSamples;
	
	if (data) {
		numSamples = length/9 * 16;
		wavedata = new short[numSamples];
		brrdecode(data, wavedata,0,0);
		
		SetControlData(hiOverView,0,kWaveDataTag,numSamples,wavedata);
		HIViewGetBounds(hiTailView,&bounds);
		start = (numSamples < bounds.size.width)?0:(numSamples-bounds.size.width);
		viewlength = (numSamples < bounds.size.width)?numSamples:bounds.size.width;
		SetControlData(hiTailView,0,kWaveDataTag,viewlength,wavedata+start);
		
		delete[] wavedata;
	}
	else {
		SetControlData(hiOverView,0,kWaveDataTag,0,NULL);
		SetControlData(hiTailView,0,kWaveDataTag,0,NULL);
		SetControlData(hiHeadView,0,kWaveDataTag,0,NULL);
	}
}

//setBRRDataを呼び出した後に呼び出す
void Chip700View::setLoopoint(UInt32 lp)
{
	short	*wavedata;
	long	numSamples;
	
	BRRData	brr;
	UInt32	size = sizeof(BRRData);
	
	AudioUnitGetProperty(mEditAudioUnit,kAudioUnitCustomProperty_BRRData,kAudioUnitScope_Global,0,&brr,&size);
	
	if (brr.data == NULL) return;
	
	HIRect	bounds;
	UInt32	looppoint = lp/9*16;
	SetControlData(hiOverView,0,kLoopPointTag,sizeof(int),&looppoint);
	
	long	start,length;
	HIViewGetBounds(hiHeadView,&bounds);
	wavedata = new short[brr.size/9*16];
	brrdecode(brr.data, wavedata,0,0);
	numSamples = brr.size/9*16;
	start = looppoint;
	length = ((start+bounds.size.width)<numSamples?bounds.size.width:(numSamples-start));
	SetControlData(hiHeadView,0,kWaveDataTag,length,wavedata+start);
	delete[] wavedata;
}

void Chip700View::correctSampleRateSelected(void)
{
	int		looppoint, key;
	double	samplerate;
	BRRData	brr;
	short	*buffer;
	int		pitch, length;
	
	UInt32 size = sizeof(BRRData);
	AudioUnitGetProperty(mEditAudioUnit,kAudioUnitCustomProperty_BRRData,kAudioUnitScope_Global,0,&brr,&size);
	
	if (brr.data == NULL) return;
	
	size = sizeof(int);
	AudioUnitGetProperty(mEditAudioUnit,kAudioUnitCustomProperty_LoopPoint,kAudioUnitScope_Global,0,&looppoint,&size);
	if (looppoint == brr.size)
		looppoint = 0;
	
	size = sizeof(int);
	AudioUnitGetProperty(mEditAudioUnit,kAudioUnitCustomProperty_BaseKey,kAudioUnitScope_Global,0,&key,&size);
	
	buffer = new short[brr.size/9*16];
	brrdecode(brr.data, buffer, 0, 0);
	length = (brr.size-looppoint)/9*16;
	pitch = estimatebasefreq(buffer+looppoint/9*16, length);
	if (pitch > 0) {
		samplerate = length/(double)pitch * 440.0*pow(2.0,(key-69.0)/12);
		AudioUnitSetProperty(mEditAudioUnit,kAudioUnitCustomProperty_Rate,
							 kAudioUnitScope_Global,0,&samplerate,sizeof(double));
	}
	delete[] buffer;
}

void Chip700View::correctBaseKeySelected(void)
{
	int		looppoint, key;
	double	samplerate, freq;
	BRRData	brr;
	short	*buffer;
	int		pitch, length;
	
	UInt32 size = sizeof(BRRData);
	AudioUnitGetProperty(mEditAudioUnit,kAudioUnitCustomProperty_BRRData,kAudioUnitScope_Global,0,&brr,&size);
	
	if (brr.data == NULL) return;
	
	size = sizeof(int);
	AudioUnitGetProperty(mEditAudioUnit,kAudioUnitCustomProperty_LoopPoint,kAudioUnitScope_Global,0,&looppoint,&size);
	if (looppoint == brr.size)
		looppoint = 0;
	
	size = sizeof(double);
	AudioUnitGetProperty(mEditAudioUnit,kAudioUnitCustomProperty_Rate,kAudioUnitScope_Global,0,&samplerate,&size);
	
	buffer = new short[brr.size/9*16];
	brrdecode(brr.data, buffer, 0, 0);
	length = (brr.size-looppoint)/9*16;
	pitch = estimatebasefreq(buffer+looppoint/9*16, length);
	if (pitch > 0) {
		freq = samplerate / (length/(double)pitch);
		key = log(freq)*17.312-35.874;
		AudioUnitSetProperty(mEditAudioUnit,kAudioUnitCustomProperty_BaseKey,kAudioUnitScope_Global,0,&key,sizeof(int));
	}
	delete[] buffer;
}

void Chip700View::loadSelected(void)
{
	FSRef	ref;
	
	if (getLoadFile(&ref) != 0)
		return;
	enqueueFile(&ref);
}

void Chip700View::saveSelected(void)
{
	UInt32		size;
	int			intValue;
	
	//編集中のプログラム番号を調べる
	size = sizeof(int);
	AudioUnitGetProperty(mEditAudioUnit,kAudioUnitCustomProperty_EditingProgram,kAudioUnitScope_Global,0,&intValue,&size);
	
	//サンプルデータを取得する
	BRRData		brr;
	size = sizeof(BRRData);
	AudioUnitGetProperty(mEditAudioUnit,kAudioUnitCustomProperty_BRRData,kAudioUnitScope_Global,0,&brr,&size);
	//データが無ければ終了する
	if (brr.data == NULL)
		return;
	
	//ファイルダイアログ
	CFStringRef	pgname;
	size = sizeof(CFStringRef);
	AudioUnitGetProperty(mEditAudioUnit,kAudioUnitCustomProperty_ProgramName,kAudioUnitScope_Global,0,&pgname,&size);
	CFStringRef	defaultname;
	if (CFStringGetLength(pgname)==0)
		defaultname = CFStringCreateWithFormat(NULL,NULL,CFSTR("program_%03d.xi"),intValue);
	else
		defaultname = CFStringCreateWithFormat(NULL,NULL,CFSTR("%@.xi"),pgname);
	CFURLRef	savefile=getSaveFile(defaultname);
	CFRelease(defaultname);
	if (savefile == NULL)
		return;
	
	saveToXIFile(savefile);
	CFRelease(savefile);
}

bool Chip700View::dragStart(ControlRef cont, EventRecord *event)
{
	//StandardDropLocation	loc;
	DragSendDataUPP	fptr;
	//DragAttributes	att;
	DragRef	dref;
	OSErr	err;
	PromiseHFSFlavor	phf;
	
	RgnHandle	rgn1;
	
	//サンプルデータを取得する
	BRRData		brr;
	UInt32		size = sizeof(BRRData);
	AudioUnitGetProperty(mEditAudioUnit,kAudioUnitCustomProperty_BRRData,kAudioUnitScope_Global,0,&brr,&size);
	//データが無ければ終了する
	if (brr.data == NULL)
		return false;
	
	HIPoint		offset;
	CGImageRef	dragimg=TImageCache::GetImage(mBundleRef, CFSTR("dragimg"), CFSTR("png"), NULL);
	offset.x=-(float)CGImageGetWidth(dragimg)/2;
	offset.y=-(float)CGImageGetHeight(dragimg)/2;
	
	if (WaitMouseMoved(event->where)) {
		if (NewDrag(&dref))
			return false;
		
		SetDragAllowableActions(dref,kDragActionCopy,false);
		
		phf.fileType=0;
		phf.fileCreator=0;
		phf.fdFlags=kHasBeenInited;
		phf.promisedFlavor=kDragPromisedFlavor;
		AddDragItemFlavor(dref,(ItemReference)1,kDragFlavorTypePromiseHFS,(Ptr)&phf,sizeof(PromiseHFSFlavor),0);
		err=AddDragItemFlavor(dref,(ItemReference)1,kDragPromisedFlavor,NULL,0,0);
		if (err) {
			DisposeDrag(dref);
			return false;
		}
		
		rgn1=NewRgn();
		
		SetDragImageWithCGImage(dref,dragimg,&offset,kDragStandardTranslucency);
		//SetDragImageWithCGImage(dref,dragimg,&offset,kDragOpaqueTranslucency);
		
		fptr=NewDragSendDataUPP((DragSendDataProcPtr)MyDragSendDataFunction);
		err=SetDragSendProc(dref,fptr,this);
		if (err==noErr)
			err=TrackDrag(dref,event,rgn1);
		DisposeDragSendDataUPP(fptr);
		/*
		if (err==noErr) {
			GetDragAttributes(dref,&att);
			
			if (!GetStandardDropLocation(dref,&loc)) {
				if (loc==kDragStandardDropLocationTrash) {
				}
			}
		}
		 */
		if (rgn1)
			DisposeRgn(rgn1);
		DisposeDrag(dref);
	}
	CGImageRelease(dragimg);
	return true;
}

void Chip700View::saveToFile(CFURLRef savefile)
{
	//Dictionaryデータを取得する
	CFDictionaryRef	propertydata;
	UInt32 size = sizeof(CFDictionaryRef);
	AudioUnitGetProperty(mEditAudioUnit,kAudioUnitCustomProperty_PGDictionary,kAudioUnitScope_Global,0,&propertydata,&size);
	
	//バイナリ形式に変換
	CFWriteStreamRef	filestream=CFWriteStreamCreateWithFile(NULL,savefile);
	if (CFWriteStreamOpen(filestream)) {
		CFPropertyListWriteToStream(propertydata,filestream,kCFPropertyListBinaryFormat_v1_0,NULL);
		CFWriteStreamClose(filestream);
	}
	CFRelease(filestream);
	//GetしたPGDictionaryはreleaseが必要
	CFRelease(propertydata);
}

void Chip700View::saveToXIFile(CFURLRef savefile)
{
	//データを取得する
	CFDataRef	propertydata;
	UInt32 size = sizeof(CFDataRef);
	AudioUnitGetProperty(mEditAudioUnit,kAudioUnitCustomProperty_XIData,kAudioUnitScope_Global,0,&propertydata,&size);
	
	CFWriteStreamRef	filestream=CFWriteStreamCreateWithFile(NULL,savefile);
	if (CFWriteStreamOpen(filestream)) {
		CFWriteStreamWrite(filestream,CFDataGetBytePtr(propertydata),CFDataGetLength(propertydata));
		CFWriteStreamClose(filestream);
	}
	CFRelease(filestream);
	
	CFRelease(propertydata);
}

int Chip700View::getLoadFile(FSRef *ref)
{
	OSStatus	status;
	NavDialogCreationOptions	myDialogOptions;
	NavDialogRef		myDialogRef;
	
	status=NavGetDefaultDialogCreationOptions(&myDialogOptions);
	myDialogOptions.optionFlags &= ~kNavAllowMultipleFiles;
	
	status = NavCreateChooseFileDialog(&myDialogOptions,NULL,NULL,NULL,MyFilterProc,NULL,&myDialogRef);
	status = NavDialogRun(myDialogRef);
	
	NavUserAction userAction = NavDialogGetUserAction(myDialogRef);
	if (userAction != kNavUserActionChoose) {
		NavDialogDispose(myDialogRef);
		return -1;
	}
	NavReplyRecord reply;
	status = NavDialogGetReply(myDialogRef,&reply);
	long	ct;
	AECountItems(&reply.selection,&ct);
	if (ct >= 1) {
		AEGetNthPtr(&reply.selection,1,typeFSRef,NULL,NULL,ref,sizeof(FSRef),NULL);
	}
	status = NavDisposeReply(&reply);
	
	NavDialogDispose(myDialogRef);
	
	return 0;
}

static Boolean MyFilterProc(AEDesc *theItem, void *info, void *callBackUD, 
					  NavFilterModes filterMode)
{
	OSStatus status;
	Boolean				display = true;
	NavFileOrFolderInfo *theInfo = (NavFileOrFolderInfo*)info;
	FSRef				ref;
	
	if (theInfo->isFolder == true)
		return true;
	
	AECoerceDesc(theItem, typeFSRef, theItem);
	
	if ( AEGetDescData(theItem, &ref, sizeof(FSRef)) == noErr )
	{
		
		CFStringRef itemUTI = NULL;
		status = LSCopyItemAttribute(&ref, kLSRolesAll,
									  kLSItemContentType, (CFTypeRef*)&itemUTI);
		
		if (status == noErr)
		{
			display = UTTypeConformsTo(itemUTI, CFSTR("public.audio") );
			CFRelease(itemUTI);
		}
		if (display == false) {
			CFURLRef	filepath=CFURLCreateFromFSRef(NULL,&ref);
			CFStringRef	ext=CFURLCopyPathExtension(filepath);
			if (ext) {
				if (
					(kCFCompareEqualTo==CFStringCompare(ext,CFSTR("brr"),kCFCompareCaseInsensitive)) ||
					(kCFCompareEqualTo==CFStringCompare(ext,CFSTR("spc"),kCFCompareCaseInsensitive))
					)
					display = true;
				CFRelease(ext);
			}
			CFRelease(filepath);
		}
	}
	return display;
}

CFURLRef Chip700View::getSaveFile(CFStringRef defaultName)
{
	OSStatus	status;
	NavDialogCreationOptions	myDialogOptions;
	NavDialogRef		myDialogRef;
	
	status=NavGetDefaultDialogCreationOptions(&myDialogOptions);
	myDialogOptions.optionFlags &= ~kNavAllowMultipleFiles;
	myDialogOptions.optionFlags |= kNavPreserveSaveFileExtension;
	myDialogOptions.saveFileName = defaultName;
	
//	CFStringRef	format_strings[2];
//	format_strings[0] = CFStringCreateWithFormat(NULL,NULL,CFSTR(".brr"));
//	format_strings[1] = CFStringCreateWithFormat(NULL,NULL,CFSTR(".xi"));
//	CFArrayRef	formats = CFArrayCreate( kCFAllocatorDefault, (const void**)format_strings, 2, &kCFTypeArrayCallBacks );
//	myDialogOptions.popupExtension = formats;
//	myDialogOptions.optionFlags &= ~kNavNoTypePopup;
	
	status = NavCreatePutFileDialog(&myDialogOptions,0,0,NULL,NULL,&myDialogRef);
	
	status = NavDialogRun(myDialogRef);
	
	NavUserAction userAction = NavDialogGetUserAction(myDialogRef);
	if (userAction != kNavUserActionSaveAs) {
		NavDialogDispose(myDialogRef);
		return NULL;
	}

	NavReplyRecord reply;
	status = NavDialogGetReply(myDialogRef,&reply);
	long	ct;
	FSRef	ref;
	AECountItems(&reply.selection,&ct);
	if (ct >= 1) {
		AEGetNthPtr(&reply.selection,1,typeFSRef,NULL,NULL,&ref,sizeof(FSRef),NULL);
	}
	CFURLRef	pathURL=CFURLCreateFromFSRef(NULL,&ref);
	CFURLRef	savepath=CFURLCreateCopyAppendingPathComponent(NULL,pathURL,reply.saveFileName,false);
	CFRelease(pathURL);
	
	status = NavDisposeReply(&reply);
	NavDialogDispose(myDialogRef);
	
	return savepath;
}

void Chip700View::loadFile(CFURLRef path)
{
	FSRef	ref;
	if (CFURLGetFSRef(path,&ref))
		loadFile(&ref);
}

void Chip700View::loadFile(FSRef *ref)
{
	CFURLRef	path=CFURLCreateFromFSRef(NULL,ref);
	CFStringRef	ext=CFURLCopyPathExtension(path);
	if ( ext == NULL ) {
		ext = CFStringCreateCopy(NULL,CFSTR("(noext)") );
	}
	
	if ( kCFCompareEqualTo==CFStringCompare(ext,CFSTR("brr"),kCFCompareCaseInsensitive) ) {
		//保存されたパッチ(.brrファイル)の読み込み
		CFReadStreamRef	filestream=CFReadStreamCreateWithFile(NULL,path);
		if (CFReadStreamOpen(filestream)) {
			CFPropertyListFormat	format=kCFPropertyListBinaryFormat_v1_0;
			CFPropertyListRef	propertydata=CFPropertyListCreateFromStream(NULL,filestream,0,
																		kCFPropertyListImmutable,
																		&format,NULL);
			AudioUnitSetProperty(mEditAudioUnit,kAudioUnitCustomProperty_PGDictionary,
								 kAudioUnitScope_Global,0,&propertydata,sizeof(CFDictionaryRef));
			CFRelease(propertydata);
			CFReadStreamClose(filestream);
		}
		CFRelease(filestream);
	}
	else if ( kCFCompareEqualTo==CFStringCompare(ext,CFSTR("spc"),kCFCompareCaseInsensitive) ) {
		loadSPCFile(path);
	}
	else {
		//その他のオーディオファイルの読み込み
		InstData	inst;
		short	*wavedata;
		long	numSamples;
		BRRData	brr;
		int		looppoint;
		bool	loop;
		
		wavedata = loadPCMFile(ref,&numSamples,&inst);
		if (wavedata == NULL) return;
		
		if (preemphasis)
			emphasis(wavedata,numSamples);
		
		brr.data = new unsigned char[numSamples/16*9+9];
		if (inst.loop)
			numSamples=inst.lp_end;
		brr.size = brrencode(wavedata, brr.data, numSamples);
		
		looppoint = (inst.lp + 15)/16*9;
		loop = inst.loop?true:false;
		AudioUnitSetProperty(mEditAudioUnit,kAudioUnitCustomProperty_BRRData,kAudioUnitScope_Global,0,&brr,sizeof(BRRData));
		AudioUnitSetProperty(mEditAudioUnit,kAudioUnitCustomProperty_Rate,kAudioUnitScope_Global,0,&inst.srcSamplerate,sizeof(double));
		AudioUnitSetProperty(mEditAudioUnit,kAudioUnitCustomProperty_BaseKey,kAudioUnitScope_Global,0,&inst.basekey,sizeof(int));
		AudioUnitSetProperty(mEditAudioUnit,kAudioUnitCustomProperty_LowKey,kAudioUnitScope_Global,0,&inst.lowkey,sizeof(int));
		AudioUnitSetProperty(mEditAudioUnit,kAudioUnitCustomProperty_HighKey,kAudioUnitScope_Global,0,&inst.highkey,sizeof(int));
		AudioUnitSetProperty(mEditAudioUnit,kAudioUnitCustomProperty_LoopPoint,kAudioUnitScope_Global,0,&looppoint,sizeof(int));
		AudioUnitSetProperty(mEditAudioUnit,kAudioUnitCustomProperty_Loop,kAudioUnitScope_Global,0,&loop,sizeof(bool));
		
		CFURLRef	noextpath=CFURLCreateCopyDeletingPathExtension(NULL,path);
		CFStringRef	dataname = CFURLCopyLastPathComponent(noextpath);
		AudioUnitSetProperty(mEditAudioUnit,kAudioUnitCustomProperty_ProgramName,kAudioUnitScope_Global,0,&dataname,sizeof(CFStringRef));
		CFRelease(dataname);
		CFRelease(noextpath);
		
		free(wavedata);
		delete[] brr.data;
	}
	CFRelease(ext);
	CFRelease(path);
}

//--------------------------------------------------------------------------------------------------
short* Chip700View::loadPCMFile(FSRef *ref, long *numSamples, InstData *inst)
{
#define	EXPAND_BUFFER	4096
    AudioFileID mAudioFileID;
    AudioStreamBasicDescription mFileDescription, outputFormat;
    SInt64 dataSize64;
    UInt32 dataSize;
	short *wavedata;
	
	OSStatus err;
	UInt32 size;
	
    // ファイルを開く
	err = AudioFileOpen(ref, fsRdPerm, 0, &mAudioFileID);
    if (err) {
        //NSLog(@"AudioFileOpen failed");
        return NULL;
    }
	
    // 開いたファイルの基本情報を mFileDescription へ
    size = sizeof(AudioStreamBasicDescription);
	err = AudioFileGetProperty(mAudioFileID, kAudioFilePropertyDataFormat, 
							   &size, &mFileDescription);
    if (err) {
        //NSLog(@"AudioFileGetProperty failed");
        AudioFileClose(mAudioFileID);
        return NULL;
    }
	
    // 開いたファイルのデータ部のバイト数を dataSize へ
    size = sizeof(SInt64);
	err = AudioFileGetProperty(mAudioFileID, kAudioFilePropertyAudioDataByteCount, 
							   &size, &dataSize64);
    if (err) {
        //NSLog(@"AudioFileGetProperty failed");
        AudioFileClose(mAudioFileID);
        return NULL;
    }
    dataSize=(UInt32)dataSize64;

	AudioFileTypeID	fileTypeID;
	size = sizeof( AudioFileTypeID );
	err = AudioFileGetProperty(mAudioFileID, kAudioFilePropertyFileFormat, &size, &fileTypeID);
	if (err) {
        //NSLog(@"AudioFileGetProperty failed");
        AudioFileClose(mAudioFileID);
        return NULL;
    }
	
	inst->basekey = 60;
	inst->lowkey = 0;
	inst->highkey = 127;
	inst->loop = 0;
	
	//ループポイントの取得
	Float64		st_point=0.0,end_point=0.0;
	if ( fileTypeID == kAudioFileAIFFType || fileTypeID == kAudioFileAIFCType ) {
		//INSTチャンクの取得
		AudioFileGetUserDataSize(mAudioFileID, 'INST', 0, &size);
		if ( size > 4 ) {
			UInt8	*instChunk = (UInt8*)malloc(size);
			AudioFileGetUserData(mAudioFileID, 'INST', 0, &size, instChunk);
			
			//MIDI情報の取得
			inst->basekey = instChunk[0];
			inst->lowkey = instChunk[2];
			inst->highkey = instChunk[3];
			
			if ( instChunk[9] > 0 ) {	//ループフラグを確認
				//マーカーの取得
				UInt32	writable;
				err = AudioFileGetPropertyInfo(mAudioFileID, kAudioFilePropertyMarkerList,
											   &size, &writable);
				if (err) {
					//NSLog(@"AudioFileGetPropertyInfo failed");
					AudioFileClose(mAudioFileID);
					return NULL;
				}
				AudioFileMarkerList	*markers = (AudioFileMarkerList*)malloc(size);
				err = AudioFileGetProperty(mAudioFileID, kAudioFilePropertyMarkerList, 
										   &size, markers);
				if (err) {
					//NSLog(@"AudioFileGetProperty failed");
					AudioFileClose(mAudioFileID);
					return NULL;
				}
				
				//ループポイントの設定
				for (unsigned int i=0; i<markers->mNumberMarkers; i++) {
					if (markers->mMarkers[i].mMarkerID == instChunk[11] ) {
						st_point = markers->mMarkers[i].mFramePosition;
					}
					else if (markers->mMarkers[i].mMarkerID == instChunk[13] ) {
						end_point = markers->mMarkers[i].mFramePosition;
					}
					CFRelease(markers->mMarkers[i].mName);
				}
				if ( st_point < end_point ) {
					inst->loop = 1;
				}
				free( markers );
			}
			free( instChunk );
		}
				
	}
	else if ( fileTypeID == kAudioFileWAVEType ) {
		//smplチャンクの取得
		AudioFileGetUserDataSize( mAudioFileID, 'smpl', 0, &size );
		if ( size >= sizeof(WAV_smpl) ) {
			UInt8	*smplChunk = (UInt8*)malloc(size);
			AudioFileGetUserData( mAudioFileID, 'smpl', 0, &size, smplChunk );
			WAV_smpl	*smpl = (WAV_smpl *)smplChunk;
			
			smpl->loops = EndianU32_LtoN( smpl->loops );
			
			if ( smpl->loops > 0 ) {
				inst->loop = 1;
				inst->basekey = EndianU32_LtoN( smpl->note );
				st_point = EndianU32_LtoN( smpl->start );
				end_point = EndianU32_LtoN( smpl->end ) + 1;

			}
			else {
				inst->basekey = EndianU32_LtoN( smpl->note );
			}
			free( smplChunk );
		}
	}

    // 波形一時読み込み用メモリを確保
    char *mFileBuffer;
	if (inst->loop)
		mFileBuffer = (char *)calloc(dataSize+EXPAND_BUFFER*mFileDescription.mBytesPerFrame,sizeof(char));
	else
		mFileBuffer = (char *)calloc(dataSize,sizeof(char));
	
	// ファイルから読み込み
	err = AudioFileReadBytes(mAudioFileID, false, 0, &dataSize, mFileBuffer);
    if (err) {
        //NSLog(@"AudioFileReadBytes failed");
        AudioFileClose(mAudioFileID);
        free(mFileBuffer);
        return NULL;
    }
    AudioFileClose(mAudioFileID);
	
    //１６bitモノラルのデータに変換
    outputFormat=mFileDescription;
	if (inst->loop) {
		UInt32	plusalpha=0, framestocopy;
		while (plusalpha < EXPAND_BUFFER) {
			framestocopy = 
			(end_point-st_point)>(EXPAND_BUFFER-plusalpha)?(EXPAND_BUFFER-plusalpha):end_point-st_point;
			memcpy(mFileBuffer+((int)end_point+plusalpha)*mFileDescription.mBytesPerFrame,
				   mFileBuffer+(int)st_point*mFileDescription.mBytesPerFrame,
				   framestocopy*mFileDescription.mBytesPerFrame);
			plusalpha += framestocopy;
		}
		dataSize += plusalpha*mFileDescription.mBytesPerFrame;
		
		Float64	adjustment = ( (long long)((end_point-st_point)/16) ) / ((end_point-st_point)/16.0);
		outputFormat.mSampleRate *= adjustment;
		st_point *= adjustment;
		end_point *= adjustment;
		
	}
	outputFormat.mFormatID = kAudioFormatLinearPCM;
    outputFormat.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagsNativeEndian;
	outputFormat.mChannelsPerFrame = 1;
	outputFormat.mBytesPerFrame = sizeof(short);
	outputFormat.mBitsPerChannel = 16;
	outputFormat.mBytesPerPacket = outputFormat.mBytesPerFrame;
	
    // バイトオーダー変換用のコンバータを用意
    AudioConverterRef converter;
	err = AudioConverterNew(&mFileDescription, &outputFormat, &converter);
    if (err) {
        //NSLog(@"AudioConverterNew failed");
        free(mFileBuffer);
        return NULL;
    }
	
	//サンプリングレート変換の質を最高に設定
	if (mFileDescription.mSampleRate != outputFormat.mSampleRate) {
		size = sizeof(UInt32);
		UInt32	setProp = kAudioConverterQuality_Max;
		AudioConverterSetProperty(converter, kAudioConverterSampleRateConverterQuality, 
								  size, &setProp);
	}
	
    //出力に必要十分なバッファサイズを得る
    //if(wavedata) // 2度目以降
    //    free(wavedata);
	UInt32	outputSize = dataSize;
	size = sizeof(UInt32);
	err = AudioConverterGetProperty(converter, kAudioConverterPropertyCalculateOutputBufferSize, 
									&size, &outputSize);
	if (err) {
		//NSLog(@"AudioConverterGetProperty failed");
		free(mFileBuffer);
		AudioConverterDispose(converter);
	}
    wavedata=(short *)malloc(outputSize);
    
    // バイトオーダー変換
	AudioConverterConvertBuffer(converter, dataSize, mFileBuffer,
								&outputSize, wavedata);
    if(outputSize == 0) {
        //NSLog(@"AudioConverterConvertBuffer failed");
        free(mFileBuffer);
        AudioConverterDispose(converter);
        return NULL;
    }
    
    // 後始末
    free(mFileBuffer);
    AudioConverterDispose(converter);
	
	inst->lp = st_point;
	inst->lp_end = end_point;
    *numSamples=outputSize/outputFormat.mBytesPerFrame;
	inst->srcSamplerate=outputFormat.mSampleRate;
	
	return wavedata;
}

int Chip700View::loadSPCFile(CFURLRef path)
{
	CFReadStreamRef	filestream=CFReadStreamCreateWithFile(NULL,path);
	if (CFReadStreamOpen(filestream) == false)
		return 0;
	
	UInt8	*filedata=new UInt8[0x101c0];
	CFIndex	readbytes=CFReadStreamRead(filestream,filedata,0x101c0);
	if (readbytes < 0x101c0) {
		delete[] filedata;
		return 0;
	}
	CFReadStreamClose(filestream);
	
	UInt8	*ramdata = filedata + 0x100;
	int		smpladdr = (int)ramdata[0x1005d] << 8;
	int		startaddr,loopaddr;
	BRRData	brr;
	double	samplerate;
	int		looppoint, key;
	bool	loop;
	int		pitch, length;
	short	*buffer;
	
	int		cEditNum=0;
	for (int i=0; i<128; i++) {
		startaddr = ramdata[smpladdr + i*4];
		startaddr+= (int)ramdata[smpladdr + i*4 + 1] << 8;
		loopaddr  = ramdata[smpladdr + i*4 + 2];
		loopaddr += (int)ramdata[smpladdr + i*4 + 3] << 8;
		looppoint = loopaddr-startaddr;
		
		brr.data = &ramdata[startaddr];
		loop = checkbrrsize(&ramdata[startaddr], &brr.size);
		
		if ( startaddr == 0 || startaddr == 0xffff ||
			 looppoint < 0 || brr.size < looppoint || (looppoint%9) != 0 )
			continue;
		
		samplerate = 32000;
		if (loop) {
			buffer = new short[(brr.size*2)/9*16];
			brrdecode(brr.data, buffer, looppoint, 2);
			length = ((brr.size-looppoint)*2)/9*16;
			pitch = estimatebasefreq(buffer+looppoint/9*16, length);
			if (pitch > 0)
				samplerate = length/(double)pitch * 440.0*pow(2.0,-9.0/12);
			delete[] buffer;
		}
		
		AudioUnitSetProperty(mEditAudioUnit,kAudioUnitCustomProperty_EditingProgram,kAudioUnitScope_Global,0,&cEditNum,sizeof(int));
		AudioUnitSetProperty(mEditAudioUnit,kAudioUnitCustomProperty_BRRData,kAudioUnitScope_Global,0,&brr,sizeof(BRRData));
		AudioUnitSetProperty(mEditAudioUnit,kAudioUnitCustomProperty_Rate,kAudioUnitScope_Global,0,&samplerate,sizeof(double));
		key = 60;
		AudioUnitSetProperty(mEditAudioUnit,kAudioUnitCustomProperty_BaseKey,kAudioUnitScope_Global,0,&key,sizeof(int));
		key = 0;
		AudioUnitSetProperty(mEditAudioUnit,kAudioUnitCustomProperty_LowKey,kAudioUnitScope_Global,0,&key,sizeof(int));
		key = 127;
		AudioUnitSetProperty(mEditAudioUnit,kAudioUnitCustomProperty_HighKey,kAudioUnitScope_Global,0,&key,sizeof(int));
		AudioUnitSetProperty(mEditAudioUnit,kAudioUnitCustomProperty_LoopPoint,kAudioUnitScope_Global,0,&looppoint,sizeof(int));
		AudioUnitSetProperty(mEditAudioUnit,kAudioUnitCustomProperty_Loop,kAudioUnitScope_Global,0,&loop,sizeof(bool));
		
		CFURLRef	extlesspath=CFURLCreateCopyDeletingPathExtension(NULL,path);
		CFStringRef	filename = CFURLCopyLastPathComponent(extlesspath);
		CFStringRef	dataname = CFStringCreateWithFormat(NULL,NULL,CFSTR("%@#%02x"),filename,i);
		AudioUnitSetProperty(mEditAudioUnit,kAudioUnitCustomProperty_ProgramName,kAudioUnitScope_Global,0,&dataname,sizeof(CFStringRef));
		CFRelease(dataname);
		CFRelease(filename);
		CFRelease(extlesspath);
		
		cEditNum++;
	}
//別に０に戻す必要は無いな
//	cEditNum = 0;
//	AudioUnitSetProperty(mEditAudioUnit,kAudioUnitCustomProperty_EditingProgram,kAudioUnitScope_Global,0,&cEditNum,sizeof(int));

	AudioUnitParameter	param={mEditAudioUnit,kParam_clipnoise,kAudioUnitScope_Global,0};
	AUParameterSet(mParameterListener, this, &param, 1, 0);
	
	delete[] filedata;
	CFRelease(filestream);
	return cEditNum;
}


static void LittleArrowsControlAction(ControlRef theControl, ControlPartCode partCode)
{
	// increment値をコントロールから取得
	// 設定されていなければ１を使う
	SInt32 increment;
	OSStatus status = GetControlData(theControl, kControlEntireControl, kControlLittleArrowsIncrementValueTag, sizeof(increment), &increment, NULL);
	if (status != noErr)
		increment = 1;
	
	SInt32 value = GetControl32BitValue(theControl);
	
	switch (partCode)
	{
		case kControlUpButtonPart:
			SetControl32BitValue(theControl, value + increment);
			break;
			
		case kControlDownButtonPart:
			SetControl32BitValue(theControl, value - increment);
			break;
	}
	
}

bool dragAccepted=false;
static OSErr MyTrackingHandler(DragTrackingMessage theMessage, WindowRef win,
							   void *handlerRefCon, DragRef theDrag)
{
	OSErr		err = noErr;
	
	switch(theMessage)
	{
		case kDragTrackingEnterHandler:
			break;
			
		case kDragTrackingEnterWindow:
			dragAccepted = DragItemsAreAcceptable(theDrag);
			if (dragAccepted)
				SetThemeCursor(kThemeCopyArrowCursor);
			break;
			
		case kDragTrackingInWindow:
			break;
			
		case kDragTrackingLeaveWindow:
			if (dragAccepted)
				SetThemeCursor(kThemeArrowCursor);
			break;
			
		case kDragTrackingLeaveHandler:
			break;
			
		default:
			err = paramErr;
	}
	
	return err;
}

bool DragItemsAreAcceptable(DragReference theDrag)
{
	OSErr			err;
	unsigned short	totalItems;
	short			aIndex;
	ItemReference	itemRef;
	Boolean			acceptableFlag = false;
	HFSFlavor 		currHFSFlavor;
	Size			flavorDataSize;
	
	err = CountDragItems(theDrag, &totalItems);
	if (totalItems > 1)
		return false;
	
	for(aIndex=1; aIndex <= totalItems; aIndex++)
	{
		err = GetDragItemReferenceNumber(theDrag, aIndex, &itemRef);
		if(err == noErr)
		{
			flavorDataSize = sizeof(HFSFlavor);
			err = GetFlavorData(theDrag, itemRef, flavorTypeHFS, &currHFSFlavor, &flavorDataSize, 0);
			if(err == noErr)
			{
				if((currHFSFlavor.fileType != kDragPseudoFileTypeDirectory) && (currHFSFlavor.fileType != kDragPseudoFileTypeVolume) && 
				   (currHFSFlavor.fileType != 'APPL') && ((currHFSFlavor.fdFlags & kIsAlias) == 0)) {
					FSRef	theRef;
					if (FSpMakeFSRef(&currHFSFlavor.fileSpec,&theRef) == noErr) {
						CFStringRef itemUTI = NULL;
						err = LSCopyItemAttribute(&theRef, kLSRolesAll,
													 kLSItemContentType, (CFTypeRef*)&itemUTI);
						if (err == noErr) {
							acceptableFlag = UTTypeConformsTo(itemUTI, CFSTR("public.audio") );
							CFRelease(itemUTI);
						}
						
						if (acceptableFlag == false) {
							CFURLRef path=CFURLCreateFromFSRef(NULL,&theRef);
							CFStringRef	ext=CFURLCopyPathExtension(path);
							if (ext) {
								if (
									(kCFCompareEqualTo==CFStringCompare(ext,CFSTR("brr"),kCFCompareCaseInsensitive)) ||
									(kCFCompareEqualTo==CFStringCompare(ext,CFSTR("spc"),kCFCompareCaseInsensitive))
									)
									acceptableFlag = true;
								CFRelease(ext);
							}
							CFRelease(path);
						}
					}
				}
			}
		}
	}
	return acceptableFlag;
}

static OSErr MyReceiveHandler(WindowRef win, void *handlerRefCon, DragRef theDrag)
{
	DragItemRef		itemRef;
	short			aIndex;
	Size			dataSize;
	bool			acceptableFlag = false;
	unsigned short	totalItems;
	HFSFlavor		theHFSFlavor;
	OSErr			err=noErr;
	Chip700View		*This=(Chip700View*)handlerRefCon;
	
	if (!dragAccepted)
		return dragNotAcceptedErr;
	
	err = CountDragItems(theDrag, &totalItems);
	for(aIndex=1; aIndex <= totalItems; aIndex++)
	{
		err = GetDragItemReferenceNumber(theDrag, aIndex, &itemRef);
		if(err == noErr)
		{
			dataSize = sizeof(HFSFlavor);
			err = GetFlavorData(theDrag, itemRef, kDragFlavorTypeHFS, 
								&theHFSFlavor, &dataSize, 0);
			if (err == noErr)
			{
				FSRef	theRef;
				if (FSpMakeFSRef(&theHFSFlavor.fileSpec,&theRef) == noErr) {
					This->enqueueFile(&theRef);	//読み込み処理
					acceptableFlag = true;
				}
				else acceptableFlag = false;
			}
		}
	}
	
	if(!acceptableFlag)
		return dragNotAcceptedErr;
	
	return err;
}

static OSErr MyDragSendDataFunction(FlavorType theType, void *dragSendRefCon,
									DragItemRef theItemRef, DragRef theDrag)
{
	AEDesc	dropLocAlias={typeNull,NULL};
	AEDesc	dropLocFS={typeNull,NULL};
	FSRef	*fs;
	long	size;
	PromiseHFSFlavor	ff1;
	Chip700View		*This=(Chip700View*)dragSendRefCon;
	
	size=sizeof(PromiseHFSFlavor);
	if (GetFlavorData(theDrag,theItemRef,flavorTypePromiseHFS,&ff1,&size,0))
		return dragNotAcceptedErr;
	if (size!=sizeof(PromiseHFSFlavor) || theType!=ff1.promisedFlavor)
		return dragNotAcceptedErr;
	
	if (GetDropLocation(theDrag,&dropLocAlias)) {
		if (dropLocAlias.dataHandle)
			AEDisposeDesc(&dropLocAlias);
		return dragNotAcceptedErr;
	}
	
	if (dropLocAlias.descriptorType!=typeAlias)
		return dragNotAcceptedErr;

	if (AECoerceDesc(&dropLocAlias,typeFSRef,&dropLocFS))
		return dragNotAcceptedErr;
	
	fs=(FSRef*)*(dropLocFS.dataHandle);
	
	CFStringRef	pgstr;
	bool		pgstr_allocated=false;
	size = sizeof(CFStringRef);
	AudioUnitGetProperty(This->mEditAudioUnit,kAudioUnitCustomProperty_ProgramName,kAudioUnitScope_Global,0,&pgstr,(UInt32*)&size);
	if (pgstr == NULL) {
		//編集中のプログラム番号を調べる
		int	intValue;
		size = sizeof(int);
		AudioUnitGetProperty(This->mEditAudioUnit,kAudioUnitCustomProperty_EditingProgram,kAudioUnitScope_Global,0,&intValue,(UInt32*)&size);
		pgstr = CFStringCreateWithFormat(NULL,NULL,CFSTR("program_%03d"),intValue);
		pgstr_allocated = true;
	}
	
	CFURLRef	dropfileFoldURL=CFURLCreateFromFSRef(NULL,fs);
	CFURLRef	dropfilefullURL=CFURLCreateFromFSRef(NULL,fs);
	CFStringRef	dropfile,suffix=CFStringCreateCopy(NULL,CFSTR(""));
	int			suffixnum=0;
	CFBooleanRef	fileexistsprop;
	bool		fileexists;
	SInt32		err_code;
	do {
		//重複ファイル名にも対する処理
		dropfile = CFStringCreateWithFormat(NULL,NULL,CFSTR("%@%@.brr"),pgstr,suffix);
		CFRelease(dropfilefullURL);
		dropfilefullURL = CFURLCreateCopyAppendingPathComponent(NULL,dropfileFoldURL,dropfile,false);
		
		CFRelease(suffix);
		suffix=CFStringCreateWithFormat(NULL,NULL,CFSTR("-%d"),++suffixnum);

		CFRelease(dropfile);
		
		fileexistsprop=(CFBooleanRef)CFURLCreatePropertyFromResource(NULL,dropfilefullURL,kCFURLFileExists,&err_code);
		fileexists = CFBooleanGetValue(fileexistsprop);
		CFRelease(fileexistsprop);
	} while(fileexists);
	CFRelease(suffix);
	CFRelease(dropfileFoldURL);
	if (pgstr_allocated)
		CFRelease(pgstr);
	
	This->saveToFile(dropfilefullURL);
	
	FSRef	dropfileFS;
	FSSpec	dropfileFSS;
	CFURLGetFSRef(dropfilefullURL,&dropfileFS);
	FSGetCatalogInfo(&dropfileFS, kFSCatInfoNone, NULL, NULL, &dropfileFSS, NULL);
	SetDragItemFlavorData(theDrag,theItemRef,ff1.promisedFlavor,(Ptr)&dropfileFSS,sizeof(FSSpec),0);
	
	CFRelease(dropfilefullURL);
	
	AEDisposeDesc(&dropLocFS);
	if (dropLocAlias.dataHandle)
		AEDisposeDesc(&dropLocAlias);
				
	return 0;
}

void Chip700View::enqueueFile(FSRef *ref)
{
	if (!shouldload) {
		CFURLRef path=CFURLCreateFromFSRef(NULL,ref);
		CFStringRef	ext=CFURLCopyPathExtension(path);
		if ( ext ) {
			if ( (kCFCompareEqualTo!=CFStringCompare(ext,CFSTR("brr"),kCFCompareCaseInsensitive)) &&
				 (kCFCompareEqualTo!=CFStringCompare(ext,CFSTR("spc"),kCFCompareCaseInsensitive))
				) 
			{
				SetControlData(hiOverView,0,kShowMsgTag,0,NULL);
				SetControlData(hiTailView,0,kShowMsgTag,0,NULL);
				SetControlData(hiHeadView,0,kShowMsgTag,0,NULL);
			}
			CFRelease(ext);
		}
		CFRelease(path);
		
		queingfile=*ref;
		shouldload=true;
	}
}

void Chip700View::Idle()
{
	if (shouldload) {
		loadFile(&queingfile);
		shouldload=false;
	}
}

