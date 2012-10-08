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
	virtual ~CWaveView();
	
	virtual bool onDrop (CDragContainer* drag, const CPoint& where);
	virtual void onDragEnter (CDragContainer* drag, const CPoint& where);
	virtual void onDragLeave (CDragContainer* drag, const CPoint& where);
	virtual void onDragMove (CDragContainer* drag, const CPoint& where);
	
	void draw(CDrawContext *pContext);
	void setWave(short *wavedata, long frames);
	void setBackColor(CColor color);
	void setLineColor(CColor color);
	bool isActive(void) {return isWaveLoaded;}
	void beginConverting();
	void endConverting();
	void setLooppoint(int loopStartSamp);
	
	CLASS_METHODS(CWaveView, CControl)
protected:
	COffscreenContext	*m_pDrawBuffer;
	CColor	lineColor;
	CColor	backColor;
	
	bool	isWaveLoaded;
	bool	converting;
	int		datanum;
	int		looppoint;
};
