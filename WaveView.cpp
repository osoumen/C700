/*
 *  WaveView.cpp
 *  Chip700
 *
 *  Created by “¡“c ‹§•F on 12/10/04.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "WaveView.h"

//-----------------------------------------------------------------------------
CWaveView::CWaveView(CRect &size, CFrame *pFrame)
: CControl(size, 0, 0, 0)
{
	buffer = new COffscreenContext(pFrame, size.right-size.left, size.bottom-size.top);
	
	lineColor = kWhiteCColor;
	backColor = kBlackCColor;
	isWaveLoaded = false;
}

//-----------------------------------------------------------------------------
CWaveView::~CWaveView()
{
	delete buffer;
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
		buffer->copyFrom(pContext, size);
	}
}

//------------------------------------------------------------------------
void CWaveView::setWave(float *wavedata, long frames)
{
	buffer->setFillColor(backColor);
	CRect	r(-1, -1, buffer->getWidth(), buffer->getHeight());
	buffer->drawRect(r, kDrawFilled);
	
	float	*temp=wavedata;
	float	pixelPerFrame=(float)buffer->getWidth()/(float)frames;
	
	CPoint p;
	p(0, 0);
	temp++;
	buffer->setFrameColor(lineColor);
	buffer->setDrawMode(kAntialias);
	float	xLoc=0.0f;
	float	max=0.0f,min=1.0f;
	while (temp < wavedata+frames) {
		if (fabs(*temp) > max) max = fabs(*temp);
		if (fabs(*temp) < min) min = fabs(*temp);
		if (xLoc > 1.0f) {
			double	xadd;
			xLoc = modf(xLoc, &xadd);
			p.y = (1.0f - min)*(buffer->getHeight() - 1) + 0.5f;
			if (p.x == 0)
				buffer->moveTo(p);
			else
				buffer->lineTo(p);
			if (max != min) {
				p.y = (1.0f - max)*(buffer->getHeight() - 1) + 0.5f;
				buffer->lineTo(p);
			}
			max=0.0f;
			min=1.0f;
			
			p.x += xadd;
		}
		temp++;
		xLoc += pixelPerFrame;
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
