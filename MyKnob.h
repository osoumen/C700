/*
 *  CMyKnob.h
 *  Chip700
 *
 *  Created by osoumen on 12/10/02.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "vstgui.h"
#include "Chip700defines.h"

class CMyKnob : public CKnob
{
public:
	CMyKnob(const CRect& size, CControlListener* listener, long tag, CBitmap* background, CBitmap* handle, const CPoint& offset = CPoint (0, 0));
	virtual ~CMyKnob();
	
	//0-1.0にスケーリングしてmin,maxの変更に対応する
	virtual void  setValue (float val) { value = ConvertToVSTValue(val, mRealVmin, mRealVmax); }
	virtual float getValue () const { return ConvertFromVSTValue(value, mRealVmin, mRealVmax); }
	virtual void  setMin (float val) { mRealVmin = val; }
	virtual float getMin () const { return mRealVmin; }
	virtual void  setMax (float val) { mRealVmax = val; }
	virtual float getMax () const { return mRealVmax; }
	virtual void  setDefaultValue (float val) { defaultValue = (val - mRealVmin) / (mRealVmax - mRealVmin); }
	//virtual	float getDefaultValue (void) const { return (defaultValue * (mRealVmax - mRealVmin) + mRealVmin); }

	CLASS_METHODS(CMyKnob, CKnob)
protected:
	virtual void drawHandle(CDrawContext* pContext);
	
	float	mRealValue;
	float	mRealVmin;
	float	mRealVmax;
};
