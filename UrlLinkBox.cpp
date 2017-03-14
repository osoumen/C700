//
//  UrlLinkBox.cpp
//  C700
//
//  Created by osoumen on 2017/03/14.
//
//

#include "UrlLinkBox.h"
#if MAC
#include "macOSUtils.h"
#endif

CUrlLinkBox::CUrlLinkBox(CRect &size, CFrame *pFrame, CControlListener* listener, long tag, CBitmap *pBackground, const char *jumpUrl)
: CControl(size, listener, tag, pBackground)
{
    strncpy(mJumpUrl, jumpUrl, URL_LEN_MAX);
}

//-----------------------------------------------------------------------------
CUrlLinkBox::~CUrlLinkBox()
{
}

//-----------------------------------------------------------------------------
void CUrlLinkBox::draw(CDrawContext *pContext)
{
    
}

//-----------------------------------------------------------------------------
bool CUrlLinkBox::onDrop(CDragContainer* drag, const CPoint& where)
{
	long size, type;
	void* ptr = drag->first(size, type);
	if ( ptr ) {
		if ( type == CDragContainer::kFile ) {
			// TODO: LoadSongPlayerCode を実行
		}
	}
	return false;
}

//------------------------------------------------------------------------
void CUrlLinkBox::onDragEnter (CDragContainer* drag, const CPoint& where)
{
	getFrame()->setCursor(kCursorCopy);
	setDirty();
}

//------------------------------------------------------------------------
void CUrlLinkBox::onDragLeave (CDragContainer* drag, const CPoint& where)
{
	getFrame()->setCursor(kCursorNotAllowed);
	setDirty();
}

//------------------------------------------------------------------------
void CUrlLinkBox::onDragMove (CDragContainer* drag, const CPoint& where)
{
}

//------------------------------------------------------------------------
CMouseEventResult CUrlLinkBox::onMouseDown (CPoint& where, const long& buttons)
{
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;
    // URLジャンプ
    OpenURL(mJumpUrl);
	return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
}
