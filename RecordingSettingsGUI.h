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

#include <map>

class RecordingSettingsGUI : public CViewContainer, public CControlListener
{
public:
    RecordingSettingsGUI(const CRect &size, CFrame *pParent, CBitmap *pBackground = 0);
	~RecordingSettingsGUI();
	
	CControl*	FindControlByTag( long tag );
	void		SetEfxAccess(EfxAccess* efxacc) { efxAcc = efxacc; }
	
	// CViewより
	virtual void	valueChanged(CControl* control);
	bool			attached(CView* view);
	bool			removed(CView* parent);
	CMessageResult	notify(CBaseObject* sender, const char* message);
    
    CLASS_METHODS(RecordingSettingsGUI, CViewContainer)
    
private:
    EfxAccess                   *efxAcc;
    
    std::map<long, CControl*>   mCntl;
};

#endif /* defined(__C700__RecordingSettingsGUI__) */
