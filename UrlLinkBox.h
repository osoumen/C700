//
//  UrlLinkBox.h
//  C700
//
//  Created by osoumen on 2017/03/14.
//
//

#ifndef __C700__UrlLinkBox__
#define __C700__UrlLinkBox__

#include "vstgui.h"

class CUrlLinkBox : public CControl
{
public:
    CUrlLinkBox(CRect &size, CFrame *pFrame, CControlListener* listener, long tag, CBitmap *pBackground, const char *jumpUrl);
    virtual ~CUrlLinkBox();

    void draw(CDrawContext *pContext);
    
    virtual bool onDrop (CDragContainer* drag, const CPoint& where);
	virtual void onDragEnter (CDragContainer* drag, const CPoint& where);
	virtual void onDragLeave (CDragContainer* drag, const CPoint& where);
	virtual void onDragMove (CDragContainer* drag, const CPoint& where);

	virtual CMouseEventResult onMouseDown (CPoint& where, const long& buttons);

    CLASS_METHODS(CUrlLinkBox, CControl)
private:
    static const int URL_LEN_MAX = 256;
    char    mJumpUrl[URL_LEN_MAX];
};

#endif /* defined(__C700__UrlLinkBox__) */
