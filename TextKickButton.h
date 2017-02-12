//
//  TextKickButton.h
//  C700
//
//  Created by osoumen on 2017/02/11.
//
//

#ifndef __C700__TextKickButton__
#define __C700__TextKickButton__

#include "vstgui.h"

class CTextKickButton : public CControl
{
public:
	CTextKickButton(const CRect& size, CControlListener* listener, long tag, CBitmap* background, const char *txt, CFontRef font);
	virtual ~CTextKickButton();
	
	virtual void setText(const char* txt);
	virtual const char* getText() const;
	
	virtual	void draw(CDrawContext* pContext);
    
    virtual CMouseEventResult onMouseDown(CPoint& where, const long& buttons);
    virtual CMouseEventResult onMouseUp(CPoint& where, const long& buttons);
    virtual CMouseEventResult onMouseMoved(CPoint& where, const long& buttons);
	
	CLASS_METHODS(CTextKickButton, CControl)
protected:
	void freeText();
	char* text;
	CFontRef	mLabelFont;
    
private:
    float   fEntryState;
};


#endif /* defined(__C700__TextKickButton__) */
