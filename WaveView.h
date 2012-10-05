/*
 *  WaveView.h
 *  Chip700
 *
 *  Created by osoumen on 12/10/04.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "vstgui.h"

class CWaveView : public CControl
{
public:
	CWaveView(CRect &size, CFrame *pFrame, CControlListener* listener, long tag);
	~CWaveView();
	
	virtual bool onDrop(void **ptrItems, long nbItems, long type, CPoint &where);
	void draw(CDrawContext *pContext);
	void setWave(float *wavedata, long frames);
	void setBackColor(CColor color);
	void setLineColor(CColor color);
	bool isActive(void) {return isWaveLoaded;}
	
	CLASS_METHODS(CWaveView, CControl)
protected:
	COffscreenContext	*m_pDrawBuffer;
	CColor	lineColor;
	CColor	backColor;
	bool	isWaveLoaded;
};
