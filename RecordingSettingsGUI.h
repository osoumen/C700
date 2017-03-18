//
//  RecordingSettingsGUI.h
//  C700
//
//  Created by osoumen on 2017/02/03.
//
//

#ifndef __C700__RecordingSettingsGUI__
#define __C700__RecordingSettingsGUI__

#include "vstgui.h"
#include "C700defines.h"
#include "GUIUtils.h"
#include "EfxAccess.h"
#include "TextKickButton.h"
#include "UrlLinkBox.h"

#include <map>

class RecordingSettingsGUI : public CViewContainer, public CControlListener
{
public:
    RecordingSettingsGUI(const CRect &size, CFrame *pParent, CBitmap *pBackground = 0);
	~RecordingSettingsGUI();
	
	CControl*	FindControlByTag( long tag );
	void		SetEfxAccess(EfxAccess* efxacc);
	
	// CViewより
	virtual void	valueChanged(CControl* control);
	bool			attached(CView* view);
	bool			removed(CView* parent);
	CMessageResult	notify(CBaseObject* sender, const char* message);
    
    bool            loadCode(const char *path);
    
    CLASS_METHODS(RecordingSettingsGUI, CViewContainer)
    
private:
    EfxAccess                   *efxAcc;
    
    std::map<long, CControl*>   mCntl;
    
    CUrlLinkBox     *cUrlLinkBox;
    
    bool getLoadFile( char *path, int maxLen, const char *title );
    bool getFolder( char *path, int maxLen, const char *title );
};

#endif /* defined(__C700__RecordingSettingsGUI__) */
