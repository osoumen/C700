/*
 *  WaveView.cpp
 *  Chip700
 *
 *  Created by osoumen on 12/10/04.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "WaveView.h"

//-----------------------------------------------------------------------------
CWaveView::CWaveView(CRect &size, CFrame *pFrame, CControlListener* listener, long tag)
: CControl(size, listener, tag, 0)
{
	m_pDrawBuffer = new COffscreenContext(pFrame, size.right-size.left, size.bottom-size.top);
	
	lineColor = MakeCColor(180, 248, 255, 255);
	backColor = MakeCColor(67, 75, 88, 255);
	isWaveLoaded = false;
}

//-----------------------------------------------------------------------------
CWaveView::~CWaveView()
{
	delete m_pDrawBuffer;
}

//-----------------------------------------------------------------------------
bool CWaveView::onDrop(void **ptrItems, long nbItems, long type, CPoint &where)
{
//	if (nbItems > 0 && type == kDropFiles) {
//		X_fierGUI	*editor = (X_fierGUI *)getEditor();
//		editor->fileDropped((FSSpec *)ptrItems[0]);
//	}
	
	return true;
}

//------------------------------------------------------------------------
void CWaveView::draw(CDrawContext *pContext)
{
	if (isWaveLoaded)
	{
		m_pDrawBuffer->copyFrom(pContext, size);
	}
	else
	{
		pContext->setFrameColor(kBlackCColor);
		pContext->setFillColor(backColor);
		pContext->drawRect(getVisibleSize(), kDrawFilledAndStroked);
	}
}

//------------------------------------------------------------------------
void CWaveView::setWave(float *wavedata, long frames)
{
	CRect	r(0, 0, m_pDrawBuffer->getWidth(), m_pDrawBuffer->getHeight());
	m_pDrawBuffer->setFrameColor(kBlackCColor);
	m_pDrawBuffer->setFillColor(backColor);
	m_pDrawBuffer->drawRect(r, kDrawFilledAndStroked);
	
	float	*dataptr=wavedata;
	float	*dataend=wavedata+frames;
	float	pixelPerFrame=(float)m_pDrawBuffer->getWidth()/(float)frames;
	
	dataptr++;
	m_pDrawBuffer->setFrameColor(lineColor);
	m_pDrawBuffer->setDrawMode(kAntialias);
	
	CPoint p;
	float	center = m_pDrawBuffer->getHeight()/2 - 1;	//外枠１ドット分余白のため
	float	x=1,y=0;
	float	xLoc=1.0f;
	float	max=.0f,min=.0f;
	while (dataptr < dataend) {
		float	temp = *dataptr * center;
		if (max==.0f) max=temp;
		if (min==.0f) min=temp;
		if (temp > max) max = temp;
		if (temp < min) min = temp;
		if (xLoc > 1.0f) {
			double	xadd;
			xLoc = modf(xLoc, &xadd);
			y = center - min + 2;	//外枠１ドット分余白のため
			p.x = x;
			p.y = y;
			if ( x == 1 )
			{
				m_pDrawBuffer->moveTo(p);
			}
			else
			{
				m_pDrawBuffer->lineTo(p);
			}
				
			if (max != min) {
				y = center-max;
				m_pDrawBuffer->lineTo(p);
			}
			max=0.0f;
			min=0.0f;
			
			x += xadd;
		}
		dataptr++;
		xLoc += pixelPerFrame;
	}
	if (max != 0.0f) {
		y = center-min;
		m_pDrawBuffer->lineTo(p);
		if (max != min) {
			y = center-max;
			m_pDrawBuffer->lineTo(p);
		}
	}
	isWaveLoaded = true;
	
	setDirty();
}

//------------------------------------------------------------------------
void CWaveView::setBackColor(CColor color)
{
	backColor = color;
}

//------------------------------------------------------------------------
void CWaveView::setLineColor(CColor color)
{
	lineColor = color;
}
