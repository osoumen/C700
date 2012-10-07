/*
 *  C700Edit.cpp
 *  Chip700
 *
 *  Created by osoumen on 12/10/01.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "C700Edit.h"

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
void C700Edit::valueChanged(CControl *pControl)
{
}
//-----------------------------------------------------------------------------
long C700Edit::getTag()
{
	return (-1);
}
