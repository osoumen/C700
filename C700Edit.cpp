/*
 *  C700Edit.cpp
 *  Chip700
 *
 *  Created by osoumen on 12/10/01.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "C700Edit.h"
#include "brrcodec.h"

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
	if (!frame)
	{
		return;
	}
	
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
	static struct ERect r={0,0,200,200};
	*erect = &r;
	return true;
}

//-----------------------------------------------------------------------------
bool C700Edit::open(void *ptr)
{
	AEffGUIEditor::open (ptr);
	systemWindow = ptr;
	
	CRect frameSize(0, 0, m_pBackground->getWidth(), m_pBackground->getHeight());
	frame = new CFrame(frameSize, ptr, this);
	frame->setBackground(m_pBackground);

	m_pUIView = new C700GUI(frameSize, frame);
	m_pUIView->setTransparency(true);
	frame->addView(m_pUIView);
	
	m_pTooltipSupport = new CTooltipSupport(frame);
	frame->setMouseObserver(m_pTooltipSupport);
	
	// last but not least set the class variable frame to our newly created frame
	this->frame = frame;
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
		frame->removeAll();
		frame->forget();
	}
	frame = NULL;
}

//-----------------------------------------------------------------------------
void C700Edit::setParameter(long index, float value)
{
	//エフェクタのパラメータが変化したときに呼ばれる
	//GUI側に変化したパラメータを反映させる処理を行う
	
	if (!frame)
	{
		return;
	}
	
	//brr換算のループポイントを実サンプル単位に変換する
	if ( index == kAudioUnitCustomProperty_LoopPoint ) {
		value = (int)value/9*16;
	}
	
	CControl	*cntl;
	int			tag = index;
	cntl = m_pUIView->FindControlByTag(tag);
	while (cntl)
	{
		cntl->setValue(value);
		//printf("tag=%d, value=%f\n",tag,value);
		
		tag += 1000;
		cntl = m_pUIView->FindControlByTag(tag);
	}
}

//-----------------------------------------------------------------------------
void C700Edit::SetLoopPoint( int lp )
{
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
	int		size = sizeof(BRRData);
	efxAcc->GetBRRData( &brr, &size );
	
	if ( size != sizeof(BRRData) ) return;
	if ( brr.data == NULL ) return;
	
	CRect	bounds;
	int		looppoint = lp/9*16;
	overView->setLooppoint(looppoint);
	
	int		start;
	int		length;
	wavedata = new short[brr.size/9*16];
	brrdecode(brr.data, wavedata,0,0);
	numSamples = brr.size/9*16;
	start = looppoint;
	length = ((start+headView->getWidth())<numSamples?headView->getWidth():(numSamples-start));
	headView->setWave(wavedata+start, length);
	delete[] wavedata;
}

//-----------------------------------------------------------------------------
void C700Edit::SetProgramName( const char *pgname )
{
	CControl	*cntl;
	cntl = m_pUIView->FindControlByTag(kAudioUnitCustomProperty_ProgramName);
	if ( cntl ) {
		if ( cntl->isTypeOf("CMyTextEdit") ) {
			CMyTextEdit	*textbox = reinterpret_cast<CMyTextEdit*> (cntl);
			textbox->setText(pgname);
		}
	}
}

//-----------------------------------------------------------------------------
void C700Edit::SetBRRData( const BRRData *brr )
{
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
	
	if (brr->data) {
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
}

//-----------------------------------------------------------------------------
void C700Edit::SetTrackSelectorValue( int track )
{
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
#if 0
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
#endif
}

//-----------------------------------------------------------------------------
void C700Edit::SetBankSelectorValue( int bank )
{
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
#if 0
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
#endif
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
