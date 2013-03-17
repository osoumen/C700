/*
 *  WaveView.cpp
 *  C700
 *
 *  Created by osoumen on 12/10/04.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "C700GUI.h"
#include "WaveView.h"
#include <math.h>

//-----------------------------------------------------------------------------
CWaveView::CWaveView(CRect &size, CFrame *pFrame, CControlListener* listener, long tag)
: CControl(size, listener, tag, 0)
{
	m_pDrawBuffer = new COffscreenContext(pFrame, size.right-size.left, size.bottom-size.top);
	
	lineColor = MakeCColor(180, 248, 255, 255);
	backColor = MakeCColor(67, 75, 88, 255);
	mMessageFont = new CFontDesc("Courier", 18);
	isWaveLoaded = false;
	looppoint = 0;
	datanum = 0;
	converting = false;
}

//-----------------------------------------------------------------------------
CWaveView::~CWaveView()
{
	delete m_pDrawBuffer;
	mMessageFont->forget();
}

//-----------------------------------------------------------------------------
bool CWaveView::onDrop(CDragContainer* drag, const CPoint& where)
{
	C700GUI	*guiview = reinterpret_cast<C700GUI*> (getParentView());
	if ( !guiview->isTypeOf("C700GUI") ) {
		return false;
	}
	
	long size, type;
	void* ptr = drag->first(size, type);
	if ( ptr ) {
		if ( type == CDragContainer::kFile ) {
			return guiview->loadToCurrentProgram((char *)ptr);
		}
	}
	return false;
}

//------------------------------------------------------------------------
void CWaveView::onDragEnter (CDragContainer* drag, const CPoint& where)
{
	getFrame()->setCursor(kCursorCopy);
	setDirty();
}

//------------------------------------------------------------------------
void CWaveView::onDragLeave (CDragContainer* drag, const CPoint& where)
{
	getFrame()->setCursor(kCursorNotAllowed);
	setDirty();
}

//------------------------------------------------------------------------
void CWaveView::onDragMove (CDragContainer* drag, const CPoint& where)
{
}

//------------------------------------------------------------------------
void CWaveView::draw(CDrawContext *pContext)
{
	if (converting)
	{
		pContext->setFrameColor(kBlackCColor);
		pContext->setFillColor(backColor);
		pContext->drawRect(getVisibleSize(), kDrawFilledAndStroked);
		
		//CRect oldClip;
		//pContext->getClipRect(oldClip);
		CRect newClip(size);
		newClip.offset(10, 16);
		//newClip.bound(oldClip);
		//pContext->setClipRect(newClip);
		pContext->setFont(mMessageFont);
		pContext->setFontColor(lineColor);
		
#if VSTGUI_USES_UTF8
		pContext->drawStringUTF8("Converting to ADPCM...", newClip, kLeftText, true);
#else
		pContext->drawString("Converting to ADPCM...", newClip, true, kLeftText);
#endif
		//pContext->setClipRect(oldClip);
	}
	else
	{
		if (isWaveLoaded)
		{
			m_pDrawBuffer->copyFrom(pContext, size);
			//ループポイントの位置を描画
			if (looppoint > 0) {
				pContext->setFrameColor(kYellowCColor);
				CPoint	point(size.left + getWidth()*looppoint/datanum, size.top + 2);
				pContext->moveTo(point);
				point.offset(0, getHeight() - 3);
				pContext->lineTo(point);
			}
		}
		else
		{
			pContext->setFrameColor(kBlackCColor);
			pContext->setFillColor(backColor);
			pContext->drawRect(getVisibleSize(), kDrawFilledAndStroked);
			
			//CRect oldClip;
			//pContext->getClipRect(oldClip);
			CRect newClip(size);
			newClip.offset(10, 16);
			//newClip.bound(oldClip);
			//pContext->setClipRect(newClip);
			pContext->setFont(mMessageFont);
			pContext->setFontColor(lineColor);
			
#if VSTGUI_USES_UTF8
#if MAC
			pContext->drawStringUTF8("Drop .aif .wav .brr .spc", newClip, kLeftText, true);
#else
			pContext->drawStringUTF8("Drop .wav .brr .spc", newClip, kLeftText, true);
#endif
#else
#if MAC
			pContext->drawString("Drop .aif .wav .brr .spc", newClip, true, kLeftText);
#else
			pContext->drawString("Drop .wav .brr .spc", newClip, true, kLeftText);
#endif
#endif
			//pContext->setClipRect(oldClip);
		}
	}
}

//------------------------------------------------------------------------
void CWaveView::setWave(short *wavedata, long frames)
{
	CRect	r(0, 0, m_pDrawBuffer->getWidth(), m_pDrawBuffer->getHeight());
	m_pDrawBuffer->setFrameColor(kBlackCColor);
	m_pDrawBuffer->setFillColor(backColor);
	m_pDrawBuffer->drawRect(r, kDrawFilledAndStroked);
	
	short	*dataptr=wavedata;
	short	*dataend=wavedata+frames;
	float	pixelPerFrame=(float)m_pDrawBuffer->getWidth()/(float)frames;
	
	//dataptr++;
	m_pDrawBuffer->setFrameColor(lineColor);
	m_pDrawBuffer->setDrawMode(kAntialias);
	
	CPoint p;
	float	center = m_pDrawBuffer->getHeight()/2 - 1;	//外枠１ドット分余白のため
	float	x=1,y=0;
	float	xLoc=1.0f;
	float	max=.0f,min=.0f;
	while (dataptr < dataend) {
		float	temp = (*dataptr / 32768.0f) * center;
		if (max==.0f) max = temp;
		if (min==.0f) min = temp;
		if (temp > max) max = temp;
		if (temp < min) min = temp;
		if (xLoc >= 1.0f) {
			float	xadd;
			xLoc = modff(xLoc, &xadd);
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
				p.y = center-max + 2;
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
	converting = false;
	datanum = frames;
	if ( frames > 0 ) {
		isWaveLoaded = true;
	}
	else {
		isWaveLoaded = false;
	}
	
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

//------------------------------------------------------------------------
void CWaveView::beginConverting()
{
	converting = true;
	setDirty();
}

//------------------------------------------------------------------------
void CWaveView::endConverting()
{
	converting = false;
	setDirty();
}

//------------------------------------------------------------------------
void CWaveView::setLooppoint(int loopStartSamp)
{
	looppoint = loopStartSamp;
	setDirty();
}
