/*
 *  C700Edit.h
 *  C700
 *
 *  Created by osoumen on 12/10/01.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#if AU
#include "plugguieditor.h"
#else
#include "aeffguieditor.h"
#endif

#include "ctooltipsupport.h"

#include "vstgui.h"
#include "C700GUI.h"
#include "C700defines.h"
#include "EfxAccess.h"
#include "C700Properties.h"

class C700Edit : public AEffGUIEditor, CControlListener
{
public:
					C700Edit( void *pEffect );
	virtual			~C700Edit();
	
	void			SetEfxAccess(EfxAccess* efxacc);
	void			SetParameterInfo(long index, float minValue, float maxValue, float defaultValue );
	void			SetProgramName( const char *pgname );
	void			SetBRRData( const BRRData *brr );
	
	virtual bool	getRect(ERect **);
	virtual bool	open(void *ptr);
	virtual void	close();
	
    void            setParameter(int32_t index, const void *inPtr);
	virtual void	setParameter(int32_t index, float value);
	virtual void	valueChanged(CControl *pControl);	
	long			getTag();

	virtual void	idle();

    std::map<int, PropertyDescription>  mPropertyParams;

private:
	void			SetLoopPoint( int lp );
	void			UpdateXMSNESText();
	void			SetTrackSelectorValue( int track );
	void			SetBankSelectorValue( int bank );
	
	CBitmap				*m_pBackground;
	C700GUI				*m_pUIView;
	CTooltipSupport		*m_pTooltipSupport;
	
	EfxAccess			*efxAcc;
};
