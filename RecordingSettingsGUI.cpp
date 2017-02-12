//
//  RecordingSettingsGUI.cpp
//  C700
//
//  Created by osoumen on 2017/02/03.
//
//

#include "RecordingSettingsGUI.h"
#include "RecordingViewCntls.h"

RecordingSettingsGUI::RecordingSettingsGUI(const CRect &inSize, CFrame *frame, CBitmap *pBackground)
: CViewContainer (inSize, frame, pBackground)
, efxAcc(NULL)
{
	CBitmap					*onOffButton;
	onOffButton = new CBitmap("bt_check.png");
    
    
	int numCntls = sizeof(sRecordingViewCntls) / sizeof(ControlInstances);
    
    for (int i=0; i<numCntls; i++) {
        CControl	*cntl;
		cntl = makeControlFrom( &sRecordingViewCntls[i], frame, this, NULL, onOffButton, NULL, NULL );
		if ( cntl )
		{
			addView(cntl);
            // -1はタグ未設定を表す
            if (cntl->getTag() != -1) {
                mCntl[cntl->getTag()] = cntl;
            }
		}
    }
    
    onOffButton->forget();
    
#if 0
    extern CFontRef kLabelFont;
    
    CTextKickButton  *cTextKickButton;
    //--CTextKickButton--------------------------------------
	CRect csize(0, 0, 80, 16);
	csize.offset(50, 20);
	cTextKickButton = new CTextKickButton(csize, this, 77778, NULL, "O.K.", kLabelFont);
	addView(cTextKickButton);
	cTextKickButton->setAttribute(kCViewTooltipAttribute,strlen("CTextKickButton")+1,"CTextKickButton");
    
    CFrameTextView *cFrameTextView;
    //--CFrameTextView--------------------------------------
    csize(0, 0, 120, 16);
	csize.offset(50, 50);
    cFrameTextView = new CFrameTextView(csize, "CFrameTextView");
    addView(cFrameTextView);
    cFrameTextView->setAttribute(kCViewTooltipAttribute,strlen("CFrameTextView")+1,"CFrameTextView");
#endif
}

RecordingSettingsGUI::~RecordingSettingsGUI()
{
    
}

CControl* RecordingSettingsGUI::FindControlByTag( long tag )
{
    auto itr = mCntl.find(tag);
    if (itr != mCntl.end()) {
        if (itr->second->getTag() == tag) {
            return mCntl[tag];
        }
    }
    return NULL;
}

void RecordingSettingsGUI::valueChanged(CControl* control)
{
    int		tag = control->getTag();
	float	value = control->getValue();
    
    if (tag == kControlButtonRecordSettingExit) {
        if ( value > 0 ) {
            // 閉じる
            if (getFrame()) {
                if (getFrame()->getModalView() == this) {
                    invalid();
                    getFrame()->setModalView(NULL);
                }
            }
        }
    }
}

bool RecordingSettingsGUI::attached(CView* view)
{
    return CViewContainer::attached(view);
}

bool RecordingSettingsGUI::removed(CView* parent)
{
    return CViewContainer::removed(parent);
}

CMessageResult RecordingSettingsGUI::notify(CBaseObject* sender, const char* message)
{
    return CViewContainer::notify(sender, message);
}
