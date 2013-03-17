/*
 *  C700View.cpp
 *  C700
 *
 *  Created by on 06/11/30.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include <AudioToolbox/AudioToolbox.h>
#include "C700View.h"
#include "brrcodec.h"
#include "czt.h"
#include "TDragTextControl.h"
#include "TWaveView.h"
#include "TImageCache.h"
#include "AudioFile.h"

static Boolean MyFileSelectFilterProc(AEDesc *theItem, void *info, void *callBackUD, 
					 NavFilterModes filterMode);
static void LittleArrowsControlAction(ControlRef theControl, ControlPartCode partCode);
static OSErr MyTrackingHandler(DragTrackingMessage theMessage, WindowRef win,
							   void *handlerRefCon, DragRef theDrag);
static OSErr MyReceiveHandler(WindowRef win, void *handlerRefCon, 
							  DragRef theDrag);

bool DragItemsAreAcceptable(DragReference theDrag);

//short* loadPCMFile(FSRef *ref, long *numSamples, InstData *inst);

COMPONENT_ENTRY(C700View)

void C700View::InitWindow(CFBundleRef sBundle)
{
	CAUCarbonViewNib::InitWindow(sBundle);
    
    TViewNoCompositingCompatible::RegisterClassForBundleID<TDragTextControl>(GetBundleID());
	TViewNoCompositingCompatible::RegisterClassForBundleID<TWaveView>(GetBundleID());
}

typedef struct {
	unsigned long manufacturer;
	unsigned long product;
	unsigned long sample_period;
	unsigned long note;
	unsigned long pitch_fraction;
	unsigned long smpte_format;
	unsigned long smpte_offset;
	unsigned long loops;
	unsigned long sampler_data;
	
	unsigned long cue_id;
	unsigned long type;
	unsigned long start;
	unsigned long end;
	unsigned long fraction;
	unsigned long play_count;
} WAV_smpl;

// ウィンドウの初回生成後
void C700View::FinishWindow(CFBundleRef sBundle)
{
	int	cntlcmdId = kControlCommandsFirst;
	HIViewRef	viewIte = HIViewGetFirstSubview(mRootUserPane);
	do {
		printf("{\n");
		
		HIViewKind	outKind;
		HIViewGetKind(viewIte, &outKind);
		CFStringRef	ctitle = HIViewCopyText(viewIte);
		char		title[256];
		CFStringGetCString( ctitle, title, 256, kCFStringEncodingMacRoman );
		HIViewID	outId;
		HIViewGetID(viewIte, &outId);
		UInt32	cmdId;
		HIViewGetCommandID(viewIte, &cmdId);
		HIRect	outRect;
		HIViewGetFrame( viewIte, &outRect );
		char			fontname[256] = "";
		int				fontsize = 0;
		char			fontalign[256] = "kCenterText";
		int				style = 0;
		int				futureuse = 0;
		
		//チェックボックスのkind変更
		if ( outKind.kind == 'bttn' && outId.signature != 'tsel' && outId.signature != 'bank' && strncmp(title, "bt_velocitysens", 15) != 0 )
		{
			outKind.kind = 'cbtn';
		}
		//IDの変更
		if ( outId.signature == 'user' )
		{
			if ( outKind.kind == 'larr' ) {
				outId.id = cntlcmdId++;
			}
			else {
				outId.id += kAudioUnitCustomProperty_Begin;
			}
		}
		if ( outId.signature != 'user' && outId.signature != 'AUid' )
		{
			if ( outId.signature == 'trac' )
			{
				outId.id += kAudioUnitCustomProperty_Begin;
			}
			else if ( (outKind.kind == 'stxt' && (strcmp(title, "0 bytes")!=0) && (strcmp(title, "0123456abcde")!=0) ) ||
				outKind.kind == 'sepa' )
			{
				outId.id = -1;
			}
			else if ( strcmp(title, "0 bytes") == 0 ) {
				outId.id = kAudioUnitCustomProperty_TotalRAM;
				outKind.signature = 'airy';
				outKind.kind = 'dtxt';
			}
			else
			{
				outId.id = cntlcmdId++;
			}
		}
		//ベロシティカーブコントロールのkind変更
		if ( outKind.kind == 'bttn' && (strcmp(title, "bt_velocitysens")==0) )
		{
			outKind.kind = 'hzsw';
		}
		//Fontの設定
		if ( strcmp(title, "0 bytes") == 0 )
		{
			//strcpy(fontname, "Helvetica Bold");
			strcpy(title, "Monaco 10 180 248 255 0 2 1 bytes");
			//fontsize = 11;
			strcpy(fontalign, "kRightText");
		}
		if ( strcmp(title, "0123456abcde") == 0 )
		{
			strcpy(fontname, "Monaco");
			fontsize = 9;
			strcpy(fontalign, "kRightText");
		}
		if ( outKind.kind == 'eutx' && outId.signature == 'user' )
		{
			if ( outId.id == kAudioUnitCustomProperty_Begin )
			{
				strcpy(fontalign, "kLeftText");
			}
			else
			{
				strcpy(fontalign, "kRightText");
			}
		}
		//小数点表示の設定
		if ( strcmp(title, "7.0000") == 0 || strcmp(title, "1.000") == 0 )
		{
			futureuse = 1;
		}
		if ( outId.signature == 'user' && outId.id == kAudioUnitCustomProperty_Begin+2 )
		{
			futureuse = 1;
		}
		if ( outId.signature == 'user' && outId.id == kAudioUnitCustomProperty_Begin )
		{
			futureuse = 2;
		}
		
		//クラスIDの出力
		printf("'%c%c%c%c',\t//sig\n'%c%c%c%c',\t//kind\n",
			   (char)((outKind.signature >>24)	& 0xff),
			   (char)((outKind.signature >>16)	& 0xff),
			   (char)((outKind.signature >>8)	& 0xff),
			   (char)(outKind.signature		& 0xff),
			   (char)((outKind.kind >>24)	& 0xff),
			   (char)((outKind.kind >>16)	& 0xff),
			   (char)((outKind.kind >>8)	& 0xff),
			   (char)(outKind.kind		& 0xff)
			   );
		
		//titleの出力
		if ( strcmp(title, "bt_preemphasis") == 0 )
		{
			printf("\"Preemphasis\",\t//title\n");
		}
		else if ( strcmp(title, "bt_echo") == 0 )
		{
			printf("\"Echo\",\t//title\n");
		}
		else if ( strcmp(title, "bt_loop") == 0 )
		{
			printf("\"Loop\",\t//title\n");
		}
		else if ( strcmp(title, "bt_multi_bank_d") == 0 )
		{
			printf("\"Multi Bank D\",\t//title\n");
		}
		else if ( strcmp(title, "bt_multi_bank_c") == 0 )
		{
			printf("\"Multi Bank C\",\t//title\n");
		}
		else if ( strcmp(title, "bt_multi_bank_b") == 0 )
		{
			printf("\"Multi Bank B\",\t//title\n");
		}
		else if ( strcmp(title, "bt_multi_bank_a") == 0 )
		{
			printf("\"Multi Bank A\",\t//title\n");
		}
		else if ( strcmp(title, "bt_preemphasis") == 0 )
		{
			printf("\"Preemphasis\",\t//title\n");
		}
		else
		{
			printf("\"%s\",\t//title\n", title );
		}
		CFRelease(ctitle);
		
		//value,max,minの出力
		int value = HIViewGetValue(viewIte);
		int min = HIViewGetMinimum(viewIte);
		int max = HIViewGetMaximum(viewIte);
		printf("%d,\t//Value\n",value);
		printf("%d,\t//Minimum\n",min);
		printf("%d,\t//Maximum\n",max);
		
		//idの出力
		if ( outId.signature != 0 ) {
			printf("'%c%c%c%c',\t//sig\n", 
				   (char)((outId.signature >>24)	& 0xff),
				   (char)((outId.signature >>16)	& 0xff),
				   (char)((outId.signature >>8)	& 0xff),
				   (char)(outId.signature		& 0xff));
		}
		else {
			printf("0,\t//sig\n");
		}
		printf("%d,\t//id\n", outId.id );
		
		//コマンドIDの出力
		if ( cmdId != 0 ) {
			printf("'%c%c%c%c',\t//command\n",
				   (char)((cmdId >>24)	& 0xff),
				   (char)((cmdId >>16)	& 0xff),
				   (char)((cmdId >>8)	& 0xff),
				   (char)(cmdId		& 0xff));
		}
		else {
			printf("0,\t//command\n");
		}
		
		//座標領域の出力
		printf("%.0f, %.0f, %.0f, %.0f,\t//x,y,w,h\n"
			   ,outRect.origin.x
			   ,outRect.origin.y
			   ,outRect.size.width
			   ,outRect.size.height
			   );
		
		//フォント名の出力 -- 空白がデフォルト
		printf("\"%s\",\t//fontname\n", fontname);
		//フォントサイズの出力 -- 0でデフォルト
		printf("%d,\t//fontsize\n", fontsize);
		//揃え位置の出力 -- 中央がデフォルト
		printf("%s,\t//fontalign\n", fontalign);
		//その他スタイルの出力 -- 0がデフォルト
		printf("%d,\t//style\n",style);
		//予約 -- 0を入れておく
		printf("%d\t//future use\n",futureuse);
		printf("},\n");
	} while ( viewIte = HIViewGetNextView(viewIte) );
	
	
	HIViewRef	control;
	HIViewID	id = {'user', 0};
	OSStatus	result;
	
	OSStatus styleOK = 0;
    ControlFontStyleRec textStyle = { 0 };
	// テキストスタイル設定用構造体を作成
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
				// コントロールのリファレンスにこのクラスへのポインタを登録する
				SetControlReference(control, SInt32(this));
				
				if (styleOK) {
					if (i==0) {
						// kAudioUnitCustomProperty_ProgramName コントロールの文字色を黒に設定する
						textStyle.just = teJustLeft;
					}
					else {
						// それ以外のコントロールの文字色を水色に設定する
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
					// edittextのフォーカス変更イベント
					EventTypeSpec events[] = {
						{kEventClassControl, kEventControlSetFocusPart},
						{kEventClassControl, kEventControlClick}
					};
					WantEventTypes(GetControlEventTarget(control), GetEventTypeCount(events), events);
					// キーフィルターを登録する
					ControlKeyFilterUPP proc = i?NumericKeyFilterCallback:StdKeyFilterCallback;
					SetControlData(control, 0, kControlEditTextKeyFilterTag, sizeof(proc), &proc);
				}
				else {
					// 値変更イベントハンドラを登録する
					EventTypeSpec events[] = {
						{kEventClassControl, kEventControlValueFieldChanged},
						{kEventClassControl, kEventControlClick}
					};
					WantEventTypes(GetControlEventTarget(control), GetEventTypeCount(events), events);
				}
				if (thekind.kind == 'larr') {
					// LittleArrowコントロールの上下操作アクションを登録する
					SetControlAction(control, LittleArrowsControlAction);
				}
			}
		}
	}
	
	// 波形ビューにクリックイベントを登録
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
	
	for ( int i=0; i<1; i++ ) {
		Size actualSize;
		id.signature = 'text';
		id.id = i;
		result = HIViewFindByID(mRootUserPane, id, &control);
		GetControlData(control,0, kControlFontStyleTag, sizeof(textStyle), &textStyle, &actualSize);
		textStyle.font = FMGetFontFamilyFromName("\pMonaco");
		SetControlData(control,0, kControlFontStyleTag, sizeof(textStyle), &textStyle);
	}
	for ( int i=2; i<=30; i++ ) {
		Size actualSize;
		id.signature = 'text';
		id.id = i;
		result = HIViewFindByID(mRootUserPane, id, &control);
		GetControlData(control,0, kControlFontStyleTag, sizeof(textStyle), &textStyle, &actualSize);
		textStyle.font = FMGetFontFamilyFromName("\pHelvetica");
		textStyle.style = 1;
		textStyle.flags |= kControlUseFaceMask;
		SetControlData(control,0, kControlFontStyleTag, sizeof(textStyle), &textStyle);
	}
	
	// ドラッグ受付を登録
	InstallTrackingHandler((DragTrackingHandlerUPP)MyTrackingHandler, mCarbonWindow, this);
	InstallReceiveHandler((DragReceiveHandlerUPP)MyReceiveHandler, mCarbonWindow, this);
	
	// プロパティが変更されたらPropertyHasChangedが呼ばれるように設定
	for (int i=0; i<kNumberOfProperties; i++) {
		RegisterPropertyChanges(kAudioUnitCustomProperty_Begin+i);
		// デフォルト値を反映させる
		PropertyHasChanged(kAudioUnitCustomProperty_Begin+i,kAudioUnitScope_Global,0);
	}
}

C700View::~C700View()
{
	RemoveTrackingHandler((DragTrackingHandlerUPP)MyTrackingHandler, mCarbonWindow);
	RemoveReceiveHandler((DragReceiveHandlerUPP)MyReceiveHandler, mCarbonWindow);
}

// Commandを持つボタンコントロールが押されたときの動作
bool C700View::HandleCommand(EventRef inEvent, HICommandExtended &cmd)
{
	switch (cmd.commandID) {
		case 'emph':	//emphasise on/off
			preemphasis = GetControl32BitValue(cmd.source.control)?true:false;
			return true;
			
		case 'load':	//load button
			loadSelected();
			return true;
		
		case 'sav2':	//save XI button
			saveSelectedXI();
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
			
		case 'copy':
		{
			// FIRパラメータをクリップボードにコピー
			HIViewID	id = {'text',0};
			HIViewRef	control;
			OSStatus	result;
			result = HIViewFindByID(mRootUserPane, id, &control);
			CFStringRef param_str = HIViewCopyText( control );
			
			OSStatus err = noErr;
			PasteboardRef theClipboard;
			err = PasteboardCreate( kPasteboardClipboard, &theClipboard );
			err = PasteboardClear( theClipboard );
			
			char text[256];
			CFStringGetCString( param_str, text, 256, kCFStringEncodingUTF8 );
			CFDataRef   data = CFDataCreate( kCFAllocatorDefault, (UInt8*)text, (strlen(text)) * sizeof(char) );
			err = PasteboardPutItemFlavor( theClipboard, (PasteboardItemID)1, kUTTypeUTF8PlainText, data, 0 );
			
			CFRelease(theClipboard);
			CFRelease( data );
			CFRelease(param_str);
			
			return true;
		}
			
		case 'tra0':
			changeEditingChannel( 0 );
			return true;
		case 'tra1':
			changeEditingChannel( 1 );
			return true;
		case 'tra2':
			changeEditingChannel( 2 );
			return true;
		case 'tra3':
			changeEditingChannel( 3 );
			return true;
		case 'tra4':
			changeEditingChannel( 4 );
			return true;
		case 'tra5':
			changeEditingChannel( 5 );
			return true;
		case 'tra6':
			changeEditingChannel( 6 );
			return true;
		case 'tra7':
			changeEditingChannel( 7 );
			return true;
		case 'tra8':
			changeEditingChannel( 8 );
			return true;
		case 'tra9':
			changeEditingChannel( 9 );
			return true;
		case 'traA':
			changeEditingChannel( 10 );
			return true;
		case 'traB':
			changeEditingChannel( 11 );
			return true;
		case 'traC':
			changeEditingChannel( 12 );
			return true;
		case 'traD':
			changeEditingChannel( 13 );
			return true;
		case 'traE':
			changeEditingChannel( 14 );
			return true;
		case 'traF':
			changeEditingChannel( 15 );
			return true;
			
		case 'bnka':
			changeBank( 0 );
			return true;
		case 'bnkb':
			changeBank( 1 );
			return true;
		case 'bnkc':
			changeBank( 2 );
			return true;
		case 'bnkd':
			changeBank( 3 );
			return true;
	}
	return false;
}

void C700View::changeEditingChannel( int ch )
{
	int intValue = ch;
	AudioUnitSetProperty(mEditAudioUnit,kAudioUnitCustomProperty_EditingChannel,
						 kAudioUnitScope_Global,0,&intValue,sizeof(int));
}

void C700View::changeBank( int bank )
{
	int intValue = bank;
	AudioUnitSetProperty(mEditAudioUnit,kAudioUnitCustomProperty_Bank,
						 kAudioUnitScope_Global,0,&intValue,sizeof(int));
}

CFStringRef C700View::CreateXMSNESText()
{
	CFStringRef	param_str;
	
	Float32	echovol_L;
	Float32 echovol_R;
	Float32 fir0;
	Float32 fir1;
	Float32 fir2;
	Float32 fir3;
	Float32 fir4;
	Float32 fir5;
	Float32 fir6;
	Float32 fir7;
	Float32 echodelay;
	Float32 echoFB;
	
	AudioUnitGetParameter(mEditAudioUnit,kParam_echovol_L,kAudioUnitScope_Global,0,&echovol_L);
	AudioUnitGetParameter(mEditAudioUnit,kParam_echovol_R,kAudioUnitScope_Global,0,&echovol_R);
	AudioUnitGetParameter(mEditAudioUnit,kParam_fir0,kAudioUnitScope_Global,0,&fir0);
	AudioUnitGetParameter(mEditAudioUnit,kParam_fir1,kAudioUnitScope_Global,0,&fir1);
	AudioUnitGetParameter(mEditAudioUnit,kParam_fir2,kAudioUnitScope_Global,0,&fir2);
	AudioUnitGetParameter(mEditAudioUnit,kParam_fir3,kAudioUnitScope_Global,0,&fir3);
	AudioUnitGetParameter(mEditAudioUnit,kParam_fir4,kAudioUnitScope_Global,0,&fir4);
	AudioUnitGetParameter(mEditAudioUnit,kParam_fir5,kAudioUnitScope_Global,0,&fir5);
	AudioUnitGetParameter(mEditAudioUnit,kParam_fir6,kAudioUnitScope_Global,0,&fir6);
	AudioUnitGetParameter(mEditAudioUnit,kParam_fir7,kAudioUnitScope_Global,0,&fir7);
	AudioUnitGetParameter(mEditAudioUnit,kParam_echodelay,kAudioUnitScope_Global,0,&echodelay);
	AudioUnitGetParameter(mEditAudioUnit,kParam_echoFB,kAudioUnitScope_Global,0,&echoFB);
	
	int vol_l = echovol_L;
	int vol_r = echovol_R;
	if (vol_l >= 0) {
		vol_l = 32 + vol_l * 47 / 127;
	}
	else {
		vol_l = 80 - vol_l * 46 / 128;
	}
	if (vol_r >= 0) {
		vol_r = 32 + vol_r * 47 / 127;
	}
	else {
		vol_r = 80 - vol_r * 46 / 128;
	}
	
	param_str = CFStringCreateWithFormat(NULL,NULL,
										 CFSTR(">%c%c%02X%02X%02X%02X%02X%02X%02X%02X%1X%02X"),
										 vol_l,
										 vol_r,
										 (UInt8)fir0,
										 (UInt8)fir1,
										 (UInt8)fir2,
										 (UInt8)fir3,
										 (UInt8)fir4,
										 (UInt8)fir5,
										 (UInt8)fir6,
										 (UInt8)fir7,
										 (UInt8)echodelay,
										 (UInt8)echoFB
										 );
	return param_str;
}

// View上のコントロールをユーザーが操作されたときに呼ばれる。
// Parameter変更コントロール以外は手動でAU側に反映させる必要あり。
bool C700View::HandleEventForView(EventRef event, HIViewRef view)
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
		propertyID = (id.id%1000)+kAudioUnitCustomProperty_Begin;
//printf("propertyID=%d\n",propertyID);
		if (id.signature == 'user') {
			switch (ekind) {
				case kEventControlClick:
				{
					if (propertyID == kAudioUnitCustomProperty_BRRData) {
						//波形部分のクリックならドラッグ処理開始
						EventRecord	eventrec;
						if ( !ConvertEventRefToEventRecord(event,&eventrec) ) {
							return dragStart(view, &eventrec);
						}
						else {
							return false;
						}
					}
					else if ( propertyID >= kAudioUnitCustomProperty_MaxNoteTrack_1 && 
							 propertyID <= kAudioUnitCustomProperty_MaxNoteTrack_16 ) {
						intValue = 0;
						AudioUnitSetProperty(mEditAudioUnit,propertyID,
											 kAudioUnitScope_Global,0,&intValue,sizeof(int));
					}
					else {
						return false;
					}
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
							
						case kAudioUnitCustomProperty_Echo:
						{
							intValue = HIViewGetValue(view);
							bool param = intValue?true:false;
							AudioUnitSetProperty(mEditAudioUnit,kAudioUnitCustomProperty_Echo,
												 kAudioUnitScope_Global,0,&param,sizeof(bool));
							break;
						}
							
						case kAudioUnitCustomProperty_Bank:
						{
							/*
							intValue = HIViewGetValue(view);
							AudioUnitSetProperty(mEditAudioUnit,kAudioUnitCustomProperty_Bank,
												 kAudioUnitScope_Global,0,&intValue,sizeof(int));
							 */
							break;
						}
							
						case kAudioUnitCustomProperty_LoopPoint:
						{
							intValue = HIViewGetValue(view);
							intValue = intValue/16*9;
							AudioUnitSetProperty(mEditAudioUnit,propertyID,
												 kAudioUnitScope_Global,0,&intValue,sizeof(int));
							break;
						}
							
						case kAudioUnitCustomProperty_AR:
						case kAudioUnitCustomProperty_DR:
						case kAudioUnitCustomProperty_SL:
						case kAudioUnitCustomProperty_SR:
						case kAudioUnitCustomProperty_VolL:
						case kAudioUnitCustomProperty_VolR:
						case kAudioUnitCustomProperty_EditingProgram:
						case kAudioUnitCustomProperty_EditingChannel:
							intValue = HIViewGetValue(view);
							AudioUnitSetProperty(mEditAudioUnit,propertyID,
												 kAudioUnitScope_Global,0,&intValue,sizeof(int));
							break;
							
						case kAudioUnitCustomProperty_Band1:
						case kAudioUnitCustomProperty_Band2:
						case kAudioUnitCustomProperty_Band3:
						case kAudioUnitCustomProperty_Band4:
						case kAudioUnitCustomProperty_Band5:
						{
							SInt32		maximum,cval;
							Float32		floatValue;
							maximum = GetControl32BitMaximum(view);
							cval = GetControl32BitValue(view);
							floatValue = (float)cval / (float)maximum;
							AudioUnitSetProperty(mEditAudioUnit,kAudioUnitCustomProperty_Begin+id.id,
												 kAudioUnitScope_Global,0,&floatValue,sizeof(float));
							break;
						}
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

// テキストボックスの変更内容をAudioUnit側に反映させる
void C700View::applyEditTextProp(ControlRef control)
{
	HIViewID	id;
	CFStringRef	cstr;
	int			intValue;
	double		doubleValue;
	AudioUnitPropertyID propertyID;
	
	HIViewGetID(control,&id);
	propertyID = (id.id%1000)+kAudioUnitCustomProperty_Begin;
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
		case kAudioUnitCustomProperty_EditingChannel:
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

// プロパティが変更されたときに呼ばれ、表示内容を変更する処理を行う
void C700View::PropertyHasChanged(AudioUnitPropertyID inPropertyID, AudioUnitScope inScope,  
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
			id.id=inPropertyID-kAudioUnitCustomProperty_Begin;
			result = HIViewFindByID(mRootUserPane, id, &control);
			if (result == noErr) {
				if (pgstr)
					HIViewSetText(control, pgstr);
				else
					HIViewSetText(control, CFSTR(""));
			}
			CFRelease(pgstr);
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
			id.id=inPropertyID-kAudioUnitCustomProperty_Begin;
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
			id.id=inPropertyID-kAudioUnitCustomProperty_Begin+1000;
			result = HIViewFindByID(mRootUserPane, id, &control);
			if (result == noErr) {
				HIViewSetValue(control, intValue/9*16);
			}
			setLoopoint(intValue);
			id.id=inPropertyID-kAudioUnitCustomProperty_Begin;
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
			id.id=inPropertyID-kAudioUnitCustomProperty_Begin;
			result = HIViewFindByID(mRootUserPane, id, &control);
			if (result == noErr) {
				maximum = GetControl32BitMaximum(control);
				minimum = GetControl32BitMinimum(control);
				cval = looping?maximum:minimum;
				HIViewSetValue(control, cval);
			}
			break;
		}
			
		case kAudioUnitCustomProperty_Echo:
		{
			bool echo_on;
			size = sizeof(bool);
			AudioUnitGetProperty(mEditAudioUnit,kAudioUnitCustomProperty_Echo,
								 kAudioUnitScope_Global,0,&echo_on,&size);
			id.id=inPropertyID-kAudioUnitCustomProperty_Begin;
			result = HIViewFindByID(mRootUserPane, id, &control);
			if (result == noErr) {
				maximum = GetControl32BitMaximum(control);
				minimum = GetControl32BitMinimum(control);
				cval = echo_on?maximum:minimum;
				HIViewSetValue(control, cval);
			}
			break;
		}
		
		case kAudioUnitCustomProperty_Bank:
			size = sizeof(int);
			AudioUnitGetProperty(mEditAudioUnit,inPropertyID,kAudioUnitScope_Global,0,&intValue,&size);
			/*
			id.id=inPropertyID-kAudioUnitCustomProperty_Begin;
			result = HIViewFindByID(mRootUserPane, id, &control);
			if (result == noErr) {
				HIViewSetValue(control, intValue);
			}
			id.id=inPropertyID-kAudioUnitCustomProperty_Begin+1000;
			result = HIViewFindByID(mRootUserPane, id, &control);
			if (result == noErr) {
				HIViewSetValue(control, intValue);
			}*/
			ChangeBankSelectorValue( intValue );
			break;
			
		case kAudioUnitCustomProperty_BaseKey:
		case kAudioUnitCustomProperty_LowKey:
		case kAudioUnitCustomProperty_HighKey:
			size = sizeof(int);
			AudioUnitGetProperty(mEditAudioUnit,inPropertyID,kAudioUnitScope_Global,0,&intValue,&size);
			id.id=inPropertyID-kAudioUnitCustomProperty_Begin;
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
			id.id=inPropertyID-kAudioUnitCustomProperty_Begin;
			result = HIViewFindByID(mRootUserPane, id, &control);
			if (result == noErr) {
				HIViewSetValue(control, intValue);
			}
			id.id=inPropertyID-kAudioUnitCustomProperty_Begin+1000;
			result = HIViewFindByID(mRootUserPane, id, &control);
			if (result == noErr) {
				HIViewSetValue(control, intValue);
			}
			break;
			
		case kAudioUnitCustomProperty_EditingProgram:
			size = sizeof(int);
			AudioUnitGetProperty(mEditAudioUnit,inPropertyID,kAudioUnitScope_Global,0,&intValue,&size);
			id.id = inPropertyID-kAudioUnitCustomProperty_Begin+1000;
			result = HIViewFindByID(mRootUserPane, id, &control);
			if (result == noErr) {
				HIViewSetValue(control, intValue);
			}
			id.id=inPropertyID-kAudioUnitCustomProperty_Begin;
			result = HIViewFindByID(mRootUserPane, id, &control);
			if (result == noErr) {
				CFStringRef	cfstr=CFStringCreateWithFormat(NULL,NULL,CFSTR("%d"),intValue);
				HIViewSetText(control, cfstr);
			}
			break;
			
		case kAudioUnitCustomProperty_EditingChannel:
			size = sizeof(int);
			AudioUnitGetProperty(mEditAudioUnit,inPropertyID,kAudioUnitScope_Global,0,&intValue,&size);
			/*
			id.id = inPropertyID-kAudioUnitCustomProperty_Begin;
			result = HIViewFindByID(mRootUserPane, id, &control);
			if (result == noErr) {
				HIViewSetValue(control, intValue);
			}
			id.id=inPropertyID-kAudioUnitCustomProperty_Begin;
			result = HIViewFindByID(mRootUserPane, id, &control);
			if (result == noErr) {
				CFStringRef	cfstr=CFStringCreateWithFormat(NULL,NULL,CFSTR("%d"),intValue);
				HIViewSetText(control, cfstr);
			}
			*/
			ChangeTrackSelectorValue( intValue );
			break;
			
		case kAudioUnitCustomProperty_Band1:
		case kAudioUnitCustomProperty_Band2:
		case kAudioUnitCustomProperty_Band3:
		case kAudioUnitCustomProperty_Band4:
		case kAudioUnitCustomProperty_Band5:
		{
			float		floatValue;
			
			size = sizeof(float);
			AudioUnitGetProperty(mEditAudioUnit,inPropertyID,inScope,inElement,&floatValue,&size);
			id.id=inPropertyID-kAudioUnitCustomProperty_Begin;
			result = HIViewFindByID(mRootUserPane, id, &control);
			if (result == noErr) {
				maximum = GetControl32BitMaximum(control);
				cval = SInt32(floatValue*maximum + 0.5);
				SetControl32BitValue(control, cval);
			}
			
			//XMSNESテキストの更新
			CFStringRef param_str = CreateXMSNESText();
			HIViewID	id = {'text',0};
			HIViewRef	control;
			result = HIViewFindByID(mRootUserPane, id, &control);
			HIViewSetText( control, param_str );
			CFRelease(param_str);
			
			break;
		}
			
		case kAudioUnitCustomProperty_TotalRAM:
			// メモリ表示を更新する
			size = sizeof(int);
			AudioUnitGetProperty(mEditAudioUnit,inPropertyID,kAudioUnitScope_Global,0,&intValue,&size);
			id.signature = 'text';
			id.id=1;
			result = HIViewFindByID(mRootUserPane, id, &control);
			if (result == noErr) {
				CFStringRef	cfstr=CFStringCreateWithFormat(NULL,NULL,CFSTR("%d bytes"),intValue);
				HIViewSetText(control, cfstr);
			}
			break;
			
		case kAudioUnitCustomProperty_NoteOnTrack_1:
		case kAudioUnitCustomProperty_NoteOnTrack_2:
		case kAudioUnitCustomProperty_NoteOnTrack_3:
		case kAudioUnitCustomProperty_NoteOnTrack_4:
		case kAudioUnitCustomProperty_NoteOnTrack_5:
		case kAudioUnitCustomProperty_NoteOnTrack_6:
		case kAudioUnitCustomProperty_NoteOnTrack_7:
		case kAudioUnitCustomProperty_NoteOnTrack_8:
		case kAudioUnitCustomProperty_NoteOnTrack_9:
		case kAudioUnitCustomProperty_NoteOnTrack_10:
		case kAudioUnitCustomProperty_NoteOnTrack_11:
		case kAudioUnitCustomProperty_NoteOnTrack_12:
		case kAudioUnitCustomProperty_NoteOnTrack_13:
		case kAudioUnitCustomProperty_NoteOnTrack_14:
		case kAudioUnitCustomProperty_NoteOnTrack_15:
		case kAudioUnitCustomProperty_NoteOnTrack_16:
		{
			// トラックインジケーターを反映させる
			size = sizeof(int);
			AudioUnitGetProperty(mEditAudioUnit,inPropertyID,kAudioUnitScope_Global,0,&intValue,&size);
			
			//printf("cnote=%d\n",intValue);
			
			id.signature = 'trac';
			id.id=inPropertyID-kAudioUnitCustomProperty_Begin;
			result = HIViewFindByID(mRootUserPane, id, &control);
			if (result == noErr) {
				if ( intValue > 0 ) {
					SetControl32BitValue(control, 1);
				}
				else {
					SetControl32BitValue(control, 0);
				}
			}
			break;
		}
			
		case kAudioUnitCustomProperty_MaxNoteTrack_1:
		case kAudioUnitCustomProperty_MaxNoteTrack_2:
		case kAudioUnitCustomProperty_MaxNoteTrack_3:
		case kAudioUnitCustomProperty_MaxNoteTrack_4:
		case kAudioUnitCustomProperty_MaxNoteTrack_5:
		case kAudioUnitCustomProperty_MaxNoteTrack_6:
		case kAudioUnitCustomProperty_MaxNoteTrack_7:
		case kAudioUnitCustomProperty_MaxNoteTrack_8:
		case kAudioUnitCustomProperty_MaxNoteTrack_9:
		case kAudioUnitCustomProperty_MaxNoteTrack_10:
		case kAudioUnitCustomProperty_MaxNoteTrack_11:
		case kAudioUnitCustomProperty_MaxNoteTrack_12:
		case kAudioUnitCustomProperty_MaxNoteTrack_13:
		case kAudioUnitCustomProperty_MaxNoteTrack_14:
		case kAudioUnitCustomProperty_MaxNoteTrack_15:
		case kAudioUnitCustomProperty_MaxNoteTrack_16:
		{
			size = sizeof(int);
			AudioUnitGetProperty(mEditAudioUnit,inPropertyID,kAudioUnitScope_Global,0,&intValue,&size);
			
			//printf("mnote=%d\n",intValue);
			
			//id.signature = 'user';
			id.id=inPropertyID-kAudioUnitCustomProperty_Begin;
			result = HIViewFindByID(mRootUserPane, id, &control);
			if (result == noErr) {
				SetControl32BitValue(control, intValue);
			}
			break;
		}
	}
	mEventDisable = false;
}

void C700View::ChangeTrackSelectorValue( int track )
{
	static const int BEGIN_TRACKSELECTOR_ID = 3000;
	static const int NUM_TRACKSELECTOR = 16;
	
	OSStatus	result;
	HIViewID	id = {'tsel', BEGIN_TRACKSELECTOR_ID};
	HIViewRef	control;
	SInt32		maximum,minimum,cval;
	
	for ( int i=0; i<NUM_TRACKSELECTOR; i++ ) {
		id.id = BEGIN_TRACKSELECTOR_ID + i;
		
		result = HIViewFindByID(mRootUserPane, id, &control);
		if (result == noErr) {
			maximum = GetControl32BitMaximum(control);
			minimum = GetControl32BitMinimum(control);
			cval = (i==track)?maximum:minimum;
			HIViewSetValue(control, cval);
		}
	}
}

void C700View::ChangeBankSelectorValue( int bank )
{
	static const int BEGIN_BANKSELECTOR_ID = 0;
	
	OSStatus	result;
	HIViewID	id = {'bank', BEGIN_BANKSELECTOR_ID};
	HIViewRef	control;
	SInt32		maximum,minimum,cval;
	
	for ( int i=0; i<NUM_BANKS; i++ ) {
		id.id = BEGIN_BANKSELECTOR_ID + i;
		
		result = HIViewFindByID(mRootUserPane, id, &control);
		if (result == noErr) {
			maximum = GetControl32BitMaximum(control);
			minimum = GetControl32BitMinimum(control);
			cval = (i==bank)?maximum:minimum;
			HIViewSetValue(control, cval);
		}
	}
}

//
// internal function
//

pascal ControlKeyFilterResult C700View::StdKeyFilterCallback(ControlRef theControl, 
																		  SInt16 *keyCode, SInt16 *charCode, 
																		  EventModifiers *modifiers)
{
	SInt16 c = *charCode;
	C700View *This = (C700View*)GetControlReference(theControl);
	if (c >= ' ' || c == '\b' || c == 0x7F || (c >= 0x1c && c <= 0x1f) || c == '\t')
		return kControlKeyFilterPassKey;
	if (c == '\r' || c == 3) {	// return or Enter
		ControlEditTextSelectionRec sel = { 0, 32767 };
		SetControlData(theControl, 0, kControlEditTextSelectionTag, sizeof(sel), &sel);
		This->applyEditTextProp(theControl);
	}
	return kControlKeyFilterBlockKey;
}

pascal ControlKeyFilterResult C700View::NumericKeyFilterCallback(ControlRef theControl, 
																			  SInt16 *keyCode, SInt16 *charCode, 
																			  EventModifiers *modifiers)
{
	SInt16 c = *charCode;
	C700View *This = (C700View*)GetControlReference(theControl);
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

//波形表示をbrrデータで更新する
void C700View::setBRRData(UInt8 *data, UInt32 length)
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

// ループポイント表示を更新する
void C700View::setLoopoint(UInt32 lp)
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

// 波形のサンプリングレートを検出
void C700View::correctSampleRateSelected(void)
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

// 波形の基本ノートを検出
void C700View::correctBaseKeySelected(void)
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

void C700View::loadSelected(void)
{
	FSRef	ref;
	
	if (getLoadFile(&ref) != 0)
		return;
	enqueueFile(&ref);
}

void C700View::saveSelected(void)
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
	if (pgname == NULL || CFStringGetLength(pgname)==0)
		defaultname = CFStringCreateWithFormat(NULL,NULL,CFSTR("program_%03d.brr"),intValue);
	else
		defaultname = CFStringCreateWithFormat(NULL,NULL,CFSTR("%@.brr"),pgname);
	CFURLRef	savefile=getSaveFile(defaultname);
	CFRelease(defaultname);
	CFRelease(pgname);
	if (savefile == NULL)
		return;
	
	saveToFile(savefile);
	CFRelease(savefile);
}

void C700View::saveSelectedXI(void)
{
	UInt32		size;
	int			intValue;
	
	//編集中のプログラム番号を調べる
	size = sizeof(int);
	AudioUnitGetProperty(mEditAudioUnit,kAudioUnitCustomProperty_EditingProgram,kAudioUnitScope_Global,0,&intValue,&size);
	
	//サンプルデータの存在確認
	BRRData		brr;
	size = sizeof(BRRData);
	AudioUnitGetProperty(mEditAudioUnit,kAudioUnitCustomProperty_BRRData,kAudioUnitScope_Global,0,&brr,&size);
	//データが無ければ終了する
	if (brr.data == NULL)
		return;
	
	//ソースファイルURLが無ければ選択ダイアログを出す
	bool	existSrcFile = false;
	CFURLRef	srcURL;
	size = sizeof(CFURLRef);
	AudioUnitGetProperty(mEditAudioUnit,kAudioUnitCustomProperty_SourceFileRef,kAudioUnitScope_Global,0,&srcURL,&size);
	if ( srcURL ) {
		//オーディオファイルとしてオープンを試みる
		AudioFileID mAudioFileID;
		FSRef		ref;
		CFURLGetFSRef(srcURL, &ref);
		OSStatus err = AudioFileOpen(&ref, fsRdPerm, 0, &mAudioFileID);
		if (err == noErr) {
			existSrcFile = true;
			AudioFileClose(mAudioFileID);
		}
	}
	if ( existSrcFile == false ) {
		FSRef		ref;
		getLoadFile(&ref, CFSTR("Select Source File"));
		CFURLRef	url;
		url = CFURLCreateFromFSRef(NULL, &ref);
		if ( url ) {
			AudioUnitSetProperty(mEditAudioUnit, kAudioUnitCustomProperty_SourceFileRef, kAudioUnitScope_Global, 0, &url, sizeof(CFURLRef));
		}
	}
	
	//ファイルダイアログ
	CFStringRef	pgname;
	size = sizeof(CFStringRef);
	AudioUnitGetProperty(mEditAudioUnit,kAudioUnitCustomProperty_ProgramName,kAudioUnitScope_Global,0,&pgname,&size);
	CFStringRef	defaultname;
	if (pgname == NULL || CFStringGetLength(pgname)==0)
		defaultname = CFStringCreateWithFormat(NULL,NULL,CFSTR("program_%03d.xi"),intValue);
	else
		defaultname = CFStringCreateWithFormat(NULL,NULL,CFSTR("%@.xi"),pgname);
	CFURLRef	savefile=getSaveFile(defaultname);
	CFRelease(defaultname);
	CFRelease(pgname);
	if (savefile == NULL)
		return;
	
	saveToXIFile(savefile);
	CFRelease(savefile);
}

bool C700View::dragStart(ControlRef cont, EventRecord *event)
{
	//StandardDropLocation	loc;
	DragSendDataUPP	fptr;
	//DragAttributes	att;
	DragRef	dref;
	OSErr	err;
	PromiseHFSFlavor	phf;
	
	RgnHandle	rgn1;
	
	//サンプルデータ存在確認
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

void C700View::saveToFile(CFURLRef savefile)
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

void C700View::saveToXIFile(CFURLRef savefile)
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

int C700View::getLoadFile(FSRef *ref, CFStringRef window_title)
{
	OSStatus	status;
	NavDialogCreationOptions	myDialogOptions;
	NavDialogRef		myDialogRef;
	
	status=NavGetDefaultDialogCreationOptions(&myDialogOptions);
	myDialogOptions.optionFlags &= ~kNavAllowMultipleFiles;
	myDialogOptions.windowTitle = window_title;
	
	status = NavCreateChooseFileDialog(&myDialogOptions,NULL,NULL,NULL,MyFileSelectFilterProc,NULL,&myDialogRef);
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

static Boolean MyFileSelectFilterProc(AEDesc *theItem, void *info, void *callBackUD, 
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

CFURLRef C700View::getSaveFile(CFStringRef defaultName)
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

void C700View::loadFile(CFURLRef path)
{
	FSRef	ref;
	if (CFURLGetFSRef(path,&ref))
		loadFile(&ref);
}

void C700View::loadFile(FSRef *ref)
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
		AudioFile::InstData	inst;
		short	*wavedata;
		long	numSamples;
		BRRData	brr;
		int		looppoint;
		bool	loop;
		int		pad;
		char	cpath[PATH_LEN_MAX];
		
		CFStringRef pathStr = CFURLCopyFileSystemPath(path, kCFURLPOSIXPathStyle);
		CFStringGetCString(pathStr, cpath, PATH_LEN_MAX-1, kCFStringEncodingUTF8);
		CFRelease(pathStr);
		
		AudioFile	audioFile(cpath,false);
		audioFile.Load();
		
		if ( audioFile.IsLoaded() == false ) return;
		wavedata = audioFile.GetAudioData();
		numSamples = audioFile.GetLoadedSamples();
		audioFile.GetInstData(&inst);
		
		if (preemphasis) {
			emphasis(wavedata,numSamples);
		}
		
		brr.data = new unsigned char[numSamples/16*9+18];
		if (inst.loop) {
			numSamples=inst.lp_end;
		}
		looppoint = (inst.lp + 15)/16*9;
		loop = inst.loop?true:false;
		pad = 16-(numSamples % 16);
		brr.size = brrencode(wavedata, brr.data, numSamples, loop, (looppoint/9)*16, pad);
		looppoint += pad/16 * 9;
		
		AudioUnitSetProperty(mEditAudioUnit,kAudioUnitCustomProperty_BRRData,kAudioUnitScope_Global,0,&brr,sizeof(BRRData));
		AudioUnitSetProperty(mEditAudioUnit,kAudioUnitCustomProperty_Rate,kAudioUnitScope_Global,0,&inst.srcSamplerate,sizeof(double));
		AudioUnitSetProperty(mEditAudioUnit,kAudioUnitCustomProperty_BaseKey,kAudioUnitScope_Global,0,&inst.basekey,sizeof(int));
		AudioUnitSetProperty(mEditAudioUnit,kAudioUnitCustomProperty_LowKey,kAudioUnitScope_Global,0,&inst.lowkey,sizeof(int));
		AudioUnitSetProperty(mEditAudioUnit,kAudioUnitCustomProperty_HighKey,kAudioUnitScope_Global,0,&inst.highkey,sizeof(int));
		AudioUnitSetProperty(mEditAudioUnit,kAudioUnitCustomProperty_LoopPoint,kAudioUnitScope_Global,0,&looppoint,sizeof(int));
		AudioUnitSetProperty(mEditAudioUnit,kAudioUnitCustomProperty_Loop,kAudioUnitScope_Global,0,&loop,sizeof(bool));

		//元波形データの情報をセットする
		AudioUnitSetProperty(mEditAudioUnit,kAudioUnitCustomProperty_SourceFileRef,kAudioUnitScope_Global,0,&path,sizeof(CFURLRef));
		AudioUnitSetProperty(mEditAudioUnit,kAudioUnitCustomProperty_IsEmaphasized,kAudioUnitScope_Global,0,&preemphasis,sizeof(bool));
		
		//拡張子を除いたファイル名をプログラム名に設定する
		CFURLRef	noextpath=CFURLCreateCopyDeletingPathExtension(NULL,path);
		CFStringRef	dataname = CFURLCopyLastPathComponent(noextpath);
		AudioUnitSetProperty(mEditAudioUnit,kAudioUnitCustomProperty_ProgramName,kAudioUnitScope_Global,0,&dataname,sizeof(CFStringRef));
		CFRelease(dataname);
		CFRelease(noextpath);
		
		delete[] brr.data;
	}
	CFRelease(ext);
	CFRelease(path);
}

//--------------------------------------------------------------------------------------------------
#if 0
short* loadPCMFile(FSRef *ref, long *numSamples, InstData *inst)
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
				//end_point = EndianU32_LtoN( smpl->end ) + 1;	//SoundForge等では最終ポイントを含める解釈
				end_point = EndianU32_LtoN( smpl->end );	//PeakではAIFFと同じ。こちらに合わせておくこととする
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
		st_point *= adjustment;		//16サンプル境界にFIXする
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
#endif

int C700View::loadSPCFile(CFURLRef path)
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
		loop = checkbrrsize(&ramdata[startaddr], &brr.size) == 1?true:false;
		
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
	AudioUnitParameter	param={mEditAudioUnit,kParam_clipnoise,kAudioUnitScope_Global,0};
	AUParameterSet(mParameterListener, this, &param, 1, 0);
	
	delete[] filedata;
	CFRelease(filestream);
	return cEditNum;
}


static void LittleArrowsControlAction(ControlRef theControl, ControlPartCode partCode)
{
	// コントロールに設定されているincrement値を取得
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
	C700View		*This=(C700View*)handlerRefCon;
	
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
	C700View		*This=(C700View*)dragSendRefCon;
	
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
	size = sizeof(CFStringRef);
	AudioUnitGetProperty(This->mEditAudioUnit,kAudioUnitCustomProperty_ProgramName,kAudioUnitScope_Global,0,&pgstr,(UInt32*)&size);
	if (pgstr == NULL) {
		//編集中のプログラム番号を調べる
		int	intValue;
		size = sizeof(int);
		AudioUnitGetProperty(This->mEditAudioUnit,kAudioUnitCustomProperty_EditingProgram,kAudioUnitScope_Global,0,&intValue,(UInt32*)&size);
		pgstr = CFStringCreateWithFormat(NULL,NULL,CFSTR("program_%03d"),intValue);
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

void C700View::enqueueFile(FSRef *ref)
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


void C700View::Idle()
{
	if (shouldload) {
		loadFile(&queingfile);
		shouldload=false;
	}
}

