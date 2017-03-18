//
//  UrlLinkBox.cpp
//  C700
//
//  Created by osoumen on 2017/03/14.
//
//

#include "UrlLinkBox.h"
#include "RecordingSettingsGUI.h"
#if MAC
#include "macOSUtils.h"
#else
void OpenURL(const char *curl);

void OpenURL(const char *curl)
{
	ShellExecute(0, 0, curl, 0, 0, SW_SHOW);
}
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
    CRect r(0, 0, getWidth(), getHeight());
    pBackground->draw(pContext, r);
}

//-----------------------------------------------------------------------------
bool CUrlLinkBox::onDrop(CDragContainer* drag, const CPoint& where)
{
    RecordingSettingsGUI	*guiview = reinterpret_cast<RecordingSettingsGUI*> (getParentView());
	if ( !guiview->isTypeOf("RecordingSettingsGUI") ) {
		return false;
	}
	
    long size, type;
	void* ptr = drag->first(size, type);
	if ( ptr ) {
		if ( type == CDragContainer::kFile ) {
            return guiview->loadCode((char *)ptr);
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
