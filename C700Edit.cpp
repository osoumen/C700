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
{
	m_pBackground = new CBitmap("AUBackground.png");
	
	rect.left	 = 0;
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
void C700Edit::SetEventListener(AUEventListenerRef	eventListener)
{
	mEventListener = eventListener;
	if ( m_pUIView )
	{
		m_pUIView->SetEventListener(eventListener);
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
//	CRect	bounds;
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
	cntl = m_pUIView->FindControlByTag(kAudioUnitCustomProperty_BRRData+1000);
	if ( cntl ) {
		tailView = reinterpret_cast<CWaveView*> (cntl);
	}
	cntl = m_pUIView->FindControlByTag(kAudioUnitCustomProperty_BRRData+2000);
	if ( cntl ) {
		headView = reinterpret_cast<CWaveView*> (cntl);
	}
	
	if (brr->data) {
		numSamples = brr->size/9 * 16;
		wavedata = new short[numSamples];
		brrdecode(brr->data, wavedata,0,0);
		
		//SetControlData(hiOverView,0,kWaveDataTag,numSamples,wavedata);
		
		//HIViewGetBounds(hiTailView,&bounds);
		start = (numSamples < tailView->getWidth())?0:(numSamples-tailView->getWidth());
		viewlength = (numSamples < tailView->getWidth())?numSamples:tailView->getWidth();
		//SetControlData(hiTailView,0,kWaveDataTag,viewlength,wavedata+start);
		
		delete[] wavedata;
		
		//ループポイントの最大値を設定
		CControl	*cntl = m_pUIView->FindControlByTag(kAudioUnitCustomProperty_LoopPoint);
		if ( cntl ) {
			cntl->setMin(numSamples);
		}		
	}
	else {
		//SetControlData(hiOverView,0,kWaveDataTag,0,NULL);
		//SetControlData(hiTailView,0,kWaveDataTag,0,NULL);
		//SetControlData(hiHeadView,0,kWaveDataTag,0,NULL);
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
