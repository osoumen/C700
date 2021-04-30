/*
 *  C700Edit.cpp
 *  C700
 *
 *  Created by osoumen on 12/10/01.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "C700Edit.h"
#include "brrcodec.h"
#include "C700Parameters.h"
#ifndef AU
#include "C700VST.h"
#endif

//-----------------------------------------------------------------------------
C700Edit::C700Edit( void *pEffect )
#if AU
: AEffGUIEditor(pEffect)
#else
: AEffGUIEditor((AudioEffect*)pEffect)
#endif
, m_pBackground( NULL )
, m_pUIView( NULL )
, m_pTooltipSupport( NULL )
, efxAcc( NULL )
{
	m_pBackground = new CBitmap("AUBackground.png");
	
	rect.left	= 0;
	rect.top	= 0;
	rect.right	= (short)m_pBackground->getWidth();
	rect.bottom = (short)m_pBackground->getHeight();
    
    createPropertyParamMap(mPropertyParams);
}

//-----------------------------------------------------------------------------
C700Edit::~C700Edit()
{
	if ( m_pBackground )
	{
		m_pBackground->forget();
		m_pBackground = 0;
	}
}

//-----------------------------------------------------------------------------
void C700Edit::SetEfxAccess(EfxAccess* efxacc)
{
	efxAcc = efxacc;
	if ( m_pUIView )
	{
		m_pUIView->SetEfxAccess(efxacc);
	}
}

//-----------------------------------------------------------------------------
void C700Edit::SetParameterInfo(long index, float minValue, float maxValue, float defaultValue)
{
	if (!frame) return;
	if (m_pUIView == NULL) return;
	
	CControl	*cntl;
	int			tag = index;
	cntl = m_pUIView->FindControlByTag(tag);
	while (cntl)
	{
		cntl->setMin(minValue);
		cntl->setMax(maxValue);
		cntl->setDefaultValue(defaultValue);
		
		tag += 1000;
		cntl = m_pUIView->FindControlByTag(tag);
	}
}

//-----------------------------------------------------------------------------
bool C700Edit::getRect(ERect **erect)
{
	static struct ERect r={0,0,m_pBackground->getHeight(),m_pBackground->getWidth()};
	*erect = &r;
	return true;
}

//-----------------------------------------------------------------------------
bool C700Edit::open(void *ptr)
{
	AEffGUIEditor::open(ptr);
	systemWindow = ptr;
	
	CRect frameSize(0, 0, m_pBackground->getWidth(), m_pBackground->getHeight());
	frame = new CFrame(frameSize, ptr, this);
	frame->setBackground(m_pBackground);

	m_pUIView = new C700GUI(frameSize, frame);
	m_pUIView->setTransparency(true);
	frame->addView(m_pUIView);
	if ( efxAcc ) {
		m_pUIView->SetEfxAccess(efxAcc);
	}
#ifndef AU
	//現在パラメータの反映
	C700VST	*efx = (C700VST*)effect;
	for (int i=0; i<kNumberOfParameters; i++) {
		SetParameterInfo(i, C700Parameters::GetParameterMin(i),
						 C700Parameters::GetParameterMax(i), C700Parameters::GetParameterDefault(i));
        if ((i != kParam_program) && ((i < kParam_program_2) || (i > kParam_program_16))) {
            efx->ParameterSetFunc( i, efxAcc->GetParameter(i), effect );
        }
	}
	//現在プロパティの反映
	for (int i=kAudioUnitCustomProperty_Begin; i<kAudioUnitCustomProperty_End; i++) {
		efx->PropertyNotifyFunc( i, effect );
	}
#endif
	
	m_pTooltipSupport = new CTooltipSupport(frame);
	frame->setMouseObserver(m_pTooltipSupport);
	
	// last but not least set the class variable frame to our newly created frame
	//this->frame = frame;
	return true;
}

//-----------------------------------------------------------------------------
void C700Edit::close()
{
	if ( m_pTooltipSupport )
	{
		m_pTooltipSupport->forget();
		m_pTooltipSupport = NULL;
	}
	if (frame)
	{
		//frame->removeAll();
		frame->forget();
	}
	frame = NULL;
	m_pUIView = NULL;
}

//-----------------------------------------------------------------------------
void C700Edit::setParameter(long index, float value)
{
	//エフェクタのパラメータが変化したときに呼ばれる
	//GUI側に変化したパラメータを反映させる処理を行う
	
	if (!frame) return;
	if (m_pUIView == NULL) return;
	
	//チャンネル表示更新
	if ( index == kAudioUnitCustomProperty_EditingChannel ) {
		SetTrackSelectorValue(value);
		return;
	}
	
	//バンク表示更新
	if ( index == kAudioUnitCustomProperty_Bank ) {
		SetBankSelectorValue(value);
		return;
	}
	
	//brr換算のループポイントを実サンプル単位に変換する
	if ( index == kAudioUnitCustomProperty_LoopPoint ) {
		value = (int)value/9*16;
	}
	
	//brr換算のループポイントを実サンプル単位に変換する
	if ( index == kParam_velocity ) {
		value = value/2.0f;
	}
	
	CControl	*cntl;
	int			tag = static_cast<int>(index);
	cntl = m_pUIView->FindControlByTag(tag);
	while (cntl)
	{
		cntl->invalid();
//        cntl->setDirty();
		cntl->setValue(value);
		//printf("tag=%d, value=%f\n",tag,value);
        
        // 使用RAMが実機の容量を超えたら赤字に変える
        if (tag == kAudioUnitCustomProperty_TotalRAM) {
            int ramMax = BRR_ENDADDR - BRR_STARTADDR;
            CMyParamDisplay *paramdisp = reinterpret_cast<CMyParamDisplay*>(cntl);
            if (value > ramMax) {
                // 数値を赤くする
                paramdisp->setFontColor(MakeCColor(255, 0, 0, 255));
            }
            else {
                // 数値を通常の色に戻す
                paramdisp->setFontColor(MakeCColor(180, 248, 255, 255));
            }
        }
		
		tag += 1000;
		cntl = m_pUIView->FindControlByTag(tag);
	}
	
	//ループポイント更新処理
	if ( index == kAudioUnitCustomProperty_LoopPoint ) {
		SetLoopPoint( value );
	}
	
	switch ( index ) {
		case kParam_echodelay:
		case kParam_echovol_L:
		case kParam_echovol_R:
		case kParam_echoFB:
		case kParam_fir0:
		case kParam_fir1:
		case kParam_fir2:
		case kParam_fir3:
		case kParam_fir4:
		case kParam_fir5:
		case kParam_fir6:
		case kParam_fir7:
		case kAudioUnitCustomProperty_Band1:
		case kAudioUnitCustomProperty_Band2:
		case kAudioUnitCustomProperty_Band3:
		case kAudioUnitCustomProperty_Band4:
		case kAudioUnitCustomProperty_Band5:
			UpdateXMSNESText();
			break;
	}
}

//-----------------------------------------------------------------------------
void C700Edit::setParameter(long index, const void *inPtr)
{
#if AU
    switch ( index ) {
		case kAudioUnitCustomProperty_BRRData:
        {
            CFDataRef   data = reinterpret_cast<CFDataRef>(inPtr);
            BRRData		brrdata;
            brrdata.data = (unsigned char *)CFDataGetBytePtr(data);
            brrdata.size = CFDataGetLength(data);
            SetBRRData( &brrdata );
            CFRelease(inPtr);
            break;
        }
		case kAudioUnitCustomProperty_ProgramName:
        {
            CFStringRef		cfpgname = reinterpret_cast<CFStringRef>(inPtr);
            if ( cfpgname == NULL )
            {
                SetProgramName( "" );
            }
            else {
                char	pgname[PROGRAMNAME_MAX_LEN];
                CFStringGetCString(cfpgname, pgname, PROGRAMNAME_MAX_LEN, kCFStringEncodingUTF8);
                SetProgramName( pgname );
                CFRelease(cfpgname);
            }
            break;
        }
        case kAudioUnitCustomProperty_PGDictionary:
        case kAudioUnitCustomProperty_XIData:
            if (inPtr != NULL) {
                CFRelease(inPtr);
            }
            break;
        default:
        {
            auto it = mPropertyParams.find(index);
            if (it != mPropertyParams.end()) {
                if (it->second.dataType == propertyDataTypeString ||
                    it->second.dataType == propertyDataTypeFilePath ||
                    it->second.dataType == propertyDataTypeVariableData) {
                    if (inPtr != NULL) {
                        CFRelease(inPtr);
                    }
                }
            }
            break;
        }
    }
#endif
}

//-----------------------------------------------------------------------------
void C700Edit::SetLoopPoint( int lp )
{
	if (!frame) return;
	if (m_pUIView == NULL) return;
	
	CWaveView	*overView=NULL;
	CWaveView	*headView=NULL;
	CControl	*cntl;
	cntl = m_pUIView->FindControlByTag(kAudioUnitCustomProperty_BRRData);
	if ( cntl ) {
		overView = reinterpret_cast<CWaveView*> (cntl);
	}
	else return;
	cntl = m_pUIView->FindControlByTag(kAudioUnitCustomProperty_BRRData+2000);
	if ( cntl ) {
		headView = reinterpret_cast<CWaveView*> (cntl);
	}
	else return;
	
	short	*wavedata;
	long	numSamples;

	BRRData	brr;
	if ( efxAcc->GetBRRData( &brr ) == false ) return;
	
	if ( brr.data == NULL ) return;
	
	CRect	bounds;
	int		looppoint = lp;
	overView->setLooppoint(looppoint);
	
	int		start;
	int		length;
	wavedata = new short[brr.samples()];
	brrdecode(brr.data, wavedata,0,0);
	numSamples = brr.samples();
	start = looppoint;
	length = static_cast<int>(((start+headView->getWidth())<numSamples?headView->getWidth():(numSamples-start)));
	headView->setWave(wavedata+start, length);
	delete[] wavedata;
}

//-----------------------------------------------------------------------------
void C700Edit::SetProgramName( const char *pgname )
{
	if (!frame) return;
	if (m_pUIView == NULL) return;
	
	CControl	*cntl;
	cntl = m_pUIView->FindControlByTag(kAudioUnitCustomProperty_ProgramName);
	if ( cntl ) {
		if ( cntl->isTypeOf("CMyTextEdit") ) {
			CMyTextEdit	*textbox = reinterpret_cast<CMyTextEdit*> (cntl);
			//textbox->invalid();		//Windowsではこれが無いとなぜか更新されない
            textbox->setDirty();
			textbox->setText(pgname);
			textbox->invalid();
		}
	}
}

//-----------------------------------------------------------------------------
void C700Edit::SetBRRData( const BRRData *brr )
{
	if (!frame) return;
	if (m_pUIView == NULL) return;
	
	long	start;
	long	viewlength;
	
	short	*wavedata;
	long	numSamples;
	
	CWaveView	*overView=NULL;
	CWaveView	*tailView=NULL;
	CWaveView	*headView=NULL;
	CControl	*cntl;
	cntl = m_pUIView->FindControlByTag(kAudioUnitCustomProperty_BRRData);
	if ( cntl ) {
		overView = reinterpret_cast<CWaveView*> (cntl);
	}
	else return;
	cntl = m_pUIView->FindControlByTag(kAudioUnitCustomProperty_BRRData+1000);
	if ( cntl ) {
		tailView = reinterpret_cast<CWaveView*> (cntl);
	}
	else return;
	cntl = m_pUIView->FindControlByTag(kAudioUnitCustomProperty_BRRData+2000);
	if ( cntl ) {
		headView = reinterpret_cast<CWaveView*> (cntl);
	}
	else return;
	
	if (brr->data && brr->size > 0) {
		numSamples = brr->size/9 * 16;
		wavedata = new short[numSamples];
		brrdecode(brr->data, wavedata,0,0);
		
		if ( overView ) {
			overView->setWave(wavedata, numSamples);
		}
		
		start = (numSamples < tailView->getWidth())?0:(numSamples-tailView->getWidth());
		viewlength = (numSamples < tailView->getWidth())?numSamples:tailView->getWidth();
		if ( tailView ) {
			tailView->setWave(wavedata+start, viewlength);
		}
		
		delete[] wavedata;
		
		//ループポイントの最大値を設定
		CControl	*cntl = m_pUIView->FindControlByTag(kAudioUnitCustomProperty_LoopPoint);
		if ( cntl ) {
			cntl->setMax(numSamples);
		}		
	}
	else {
		if ( overView ) {
			overView->setWave(NULL, 0);
		}
		if ( tailView ) {
			tailView->setWave(NULL, 0);
		}
		if ( headView ) {
			headView->setWave(NULL, 0);
		}
	}
}

//-----------------------------------------------------------------------------
void C700Edit::UpdateXMSNESText()
{
#if 0
	if (!frame) return;
	if (m_pUIView == NULL) return;
	
	float echovol_L;
	float echovol_R;
	float fir0;
	float fir1;
	float fir2;
	float fir3;
	float fir4;
	float fir5;
	float fir6;
	float fir7;
	float echodelay;
	float echoFB;
	
	echovol_L	= efxAcc->GetParameter(kParam_echovol_L);
	echovol_R	= efxAcc->GetParameter(kParam_echovol_R);
	fir0		= efxAcc->GetParameter(kParam_fir0);
	fir1		= efxAcc->GetParameter(kParam_fir1);
	fir2		= efxAcc->GetParameter(kParam_fir2);
	fir3		= efxAcc->GetParameter(kParam_fir3);
	fir4		= efxAcc->GetParameter(kParam_fir4);
	fir5		= efxAcc->GetParameter(kParam_fir5);
	fir6		= efxAcc->GetParameter(kParam_fir6);
	fir7		= efxAcc->GetParameter(kParam_fir7);
	echodelay	= efxAcc->GetParameter(kParam_echodelay);
	echoFB		= efxAcc->GetParameter(kParam_echoFB);
	
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
	
	char	param_str[256];
	sprintf(param_str,
			 ">%c%c%02X%02X%02X%02X%02X%02X%02X%02X%1X%02X",
			 vol_l,
			 vol_r,
			 (unsigned char)fir0,
			 (unsigned char)fir1,
			 (unsigned char)fir2,
			 (unsigned char)fir3,
			 (unsigned char)fir4,
			 (unsigned char)fir5,
			 (unsigned char)fir6,
			 (unsigned char)fir7,
			 (unsigned char)echodelay,
			 (unsigned char)echoFB
			 );
	
	CTextLabel	*textbox = reinterpret_cast<CTextLabel*> (m_pUIView->FindControlByTag(kControlXMSNESText));
	textbox->setText(param_str);
#endif
}

//-----------------------------------------------------------------------------
void C700Edit::SetTrackSelectorValue( int track )
{
	if (!frame) return;
	if (m_pUIView == NULL) return;
	
	track = 15-track;
	for ( int i=0; i<16; i++ ) {
		CControl *cntl = m_pUIView->FindControlByTag(kControlSelectTrack16 + i);
		if ( cntl ) {
			if ( track == i ) {
				cntl->setValue(1.0f);
			}
			else {
				cntl->setValue(.0f);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void C700Edit::SetBankSelectorValue( int bank )
{
	if (!frame) return;
	if (m_pUIView == NULL) return;
	
	bank = 3-bank;
	for ( int i=0; i<4; i++ ) {
		CControl *cntl = m_pUIView->FindControlByTag(kControlBankDBtn + i);
		if ( cntl ) {
			if ( bank == i ) {
				cntl->setValue(1.0f);
			}
			else {
				cntl->setValue(.0f);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void C700Edit::valueChanged(CControl *pControl)
{
}
//-----------------------------------------------------------------------------
long C700Edit::getTag()
{
	return (-1);
}
//-----------------------------------------------------------------------------
void C700Edit::idle()
{
#ifdef _WIN32
	// Windows環境で音色を切り替え時にしばしばビューの一部が再描画されない対策
	frame->setDirty();
#endif
	AEffGUIEditor::idle();
}
