//
//  RecordingSettingsGUI.cpp
//  C700
//
//  Created by osoumen on 2017/02/03.
//
//

#include "RecordingSettingsGUI.h"
#include "RecordingViewCntls.h"
#include "MyTextEdit.h"
#include "cfileselector.h"

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
    
    CBitmap					*firstOpen;
	firstOpen = new CBitmap("firstopen.png");
    CRect csize(0,0,inSize.width(), inSize.height()-32);
    cUrlLinkBox = new CUrlLinkBox(csize, frame, this, -1, firstOpen, "http://picopicose.com/software.html");
    addView(cUrlLinkBox);
    //cUrlLinkBox->setAttribute(kCViewTooltipAttribute,strlen("CUrlLinkBox")+1,"CUrlLinkBox");
    firstOpen->forget();
    cUrlLinkBox->setVisible(false);

#if 0
    extern CFontRef kLabelFont;
    
    CTextKickButton  *cTextKickButton;
    //--CTextKickButton--------------------------------------
	CRect csize(0, 0, 80, 16);
	csize.offset(50, 20);
	cTextKickButton = new CTextKickButton(csize, this, kControlButtonRecordSettingExit, NULL, "O.K.", kLabelFont);
	addView(cTextKickButton);
	cTextKickButton->setAttribute(kCViewTooltipAttribute,strlen("CTextKickButton")+1,"CTextKickButton");
    
    CBitmap					*firstOpen;
	firstOpen = new CBitmap("firstopen.png");
    
    //--CUrlLinkBox--------------------------------------
    csize = inSize;
    csize.offset(-inSize.x, -inSize.y);
    cUrlLinkBox = new CUrlLinkBox(csize, frame, this, -1, firstOpen, "http://picopicose.com");
    addView(cUrlLinkBox);
    cUrlLinkBox->setAttribute(kCViewTooltipAttribute,strlen("CUrlLinkBox")+1,"CUrlLinkBox");
    firstOpen->forget();
    cUrlLinkBox->setVisible(false);
#endif
    
}

RecordingSettingsGUI::~RecordingSettingsGUI()
{
    
}

void RecordingSettingsGUI::SetEfxAccess(EfxAccess* efxacc)
{
    efxAcc = efxacc;
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
    const char	*text = NULL;
    
	if ( control->isTypeOf("CMyTextEdit") ) {
		CMyTextEdit		*textedit = reinterpret_cast<CMyTextEdit*> (control);
		text = textedit->getText();
	}
    
    if ( tag < kControlCommandsFirst ) {
        // プロパティ系の操作
        int	propertyId = ((tag-kAudioUnitCustomProperty_Begin)%1000)+kAudioUnitCustomProperty_Begin;
        switch (propertyId) {
            case kAudioUnitCustomProperty_GameTitle:
            case kAudioUnitCustomProperty_SongTitle:
            case kAudioUnitCustomProperty_NameOfDumper:
            case kAudioUnitCustomProperty_ArtistOfSong:
            case kAudioUnitCustomProperty_SongComments:
                if ( text ) {
                    efxAcc->SetCStringProperty(propertyId, text);
                }
                break;
            case kAudioUnitCustomProperty_RepeatNumForSpc:
                efxAcc->SetPropertyValue( propertyId, value / 10.0 );
                break;
            case kAudioUnitCustomProperty_FadeMsTimeForSpc:
                if ( text ) {
                    value = std::atoi(text);
                    if (value > 99999) value = 99999;
                    if (value < 0) value = 0;
                    CMyTextEdit *textedit = reinterpret_cast<CMyTextEdit*> (mCntl[kAudioUnitCustomProperty_FadeMsTimeForSpc]);
                    textedit->setValue(value);
                }
            default:
                efxAcc->SetPropertyValue( propertyId, value );
                break;
        }
    }
    else {
        switch (tag) {
            case kControlButtonRecordSettingExit:
                if ( value > 0 ) {
                    // 閉じる
                    if (getFrame()) {
                        if (getFrame()->getModalView() == this) {
                            invalid();
                            getFrame()->setModalView(NULL);
                        }
                    }
                }
                break;
                
            case kControlButtonChooseRecordPath:
                if ( value > 0 ) {
					char	path[PATH_LEN_MAX];
					bool	isSelected;
					isSelected = getFolder(path, PATH_LEN_MAX, "Choose Folder...");
					if ( isSelected ) {
						efxAcc->SetFilePathProperty(kAudioUnitCustomProperty_SongRecordPath, path);
                        CTextLabel *textlabel = reinterpret_cast<CTextLabel*> (mCntl[kAudioUnitCustomProperty_SongRecordPath]);
                        textlabel->setText(path);
					}
				}
                break;
                
            case kControlButtonSetRecordStart:
            {
                if ( value > 0 ) {
                    double pos = efxAcc->GetPropertyValue(kAudioUnitCustomProperty_HostBeatPos);
                    efxAcc->SetPropertyValue(kAudioUnitCustomProperty_RecordStartBeatPos, pos);
                    mCntl[kAudioUnitCustomProperty_RecordStartBeatPos]->setValue(pos);
                }
                break;
            }
            case kControlButtonSetRecordLoopStart:
            {
                if ( value > 0 ) {
                    double pos = efxAcc->GetPropertyValue(kAudioUnitCustomProperty_HostBeatPos);
                    efxAcc->SetPropertyValue(kAudioUnitCustomProperty_RecordLoopStartBeatPos, pos);
                    mCntl[kAudioUnitCustomProperty_RecordLoopStartBeatPos]->setValue(pos);
                }
                break;
            }
            case kControlButtonSetRecordEnd:
            {
                if ( value > 0 ) {
                    double pos = efxAcc->GetPropertyValue(kAudioUnitCustomProperty_HostBeatPos);
                    efxAcc->SetPropertyValue(kAudioUnitCustomProperty_RecordEndBeatPos, pos);
                    mCntl[kAudioUnitCustomProperty_RecordEndBeatPos]->setValue(pos);
                }
                break;
            }
            case kControlButtonLoadPlayerCode:
            {
                if ( value > 0 ) {
                    char	path[PATH_LEN_MAX];
					bool	isSelected;
					isSelected = getLoadFile(path, PATH_LEN_MAX, "");
					if ( isSelected ) {
                        loadCode(path);
					}
                }
                break;
            }
        }
    }
}

bool RecordingSettingsGUI::attached(CView* view)
{
    if (efxAcc == NULL) {
        CViewContainer::attached(view);
    }
    
    // 設定値を読み込んで反映
    mCntl[kAudioUnitCustomProperty_RecSaveAsSpc]->setValue(efxAcc->GetPropertyValue(kAudioUnitCustomProperty_RecSaveAsSpc));
    mCntl[kAudioUnitCustomProperty_RecSaveAsSmc]->setValue(efxAcc->GetPropertyValue(kAudioUnitCustomProperty_RecSaveAsSmc));
    mCntl[kAudioUnitCustomProperty_TimeBaseForSmc]->setValue(efxAcc->GetPropertyValue(kAudioUnitCustomProperty_TimeBaseForSmc));
    
    char str[PATH_LEN_MAX];
    CTextLabel *textlabel = reinterpret_cast<CTextLabel*> (mCntl[kAudioUnitCustomProperty_SongRecordPath]);
    efxAcc->GetFilePathProperty(kAudioUnitCustomProperty_SongRecordPath, str, PATH_LEN_MAX);
    textlabel->setText(str);
    
    CMyTextEdit		*textedit;
    textedit = reinterpret_cast<CMyTextEdit*> (mCntl[kAudioUnitCustomProperty_GameTitle]);
    efxAcc->GetCStringProperty(kAudioUnitCustomProperty_GameTitle, str, 33);
    textedit->setText(str);
    textedit = reinterpret_cast<CMyTextEdit*> (mCntl[kAudioUnitCustomProperty_SongTitle]);
    efxAcc->GetCStringProperty(kAudioUnitCustomProperty_SongTitle, str, 33);
    textedit->setText(str);
    textedit = reinterpret_cast<CMyTextEdit*> (mCntl[kAudioUnitCustomProperty_NameOfDumper]);
    efxAcc->GetCStringProperty(kAudioUnitCustomProperty_NameOfDumper, str, 17);
    textedit->setText(str);
    textedit = reinterpret_cast<CMyTextEdit*> (mCntl[kAudioUnitCustomProperty_ArtistOfSong]);
    efxAcc->GetCStringProperty(kAudioUnitCustomProperty_ArtistOfSong, str, 33);
    textedit->setText(str);
    textedit = reinterpret_cast<CMyTextEdit*> (mCntl[kAudioUnitCustomProperty_SongComments]);
    efxAcc->GetCStringProperty(kAudioUnitCustomProperty_SongComments, str, 33);
    textedit->setText(str);
    
    textedit = reinterpret_cast<CMyTextEdit*> (mCntl[kAudioUnitCustomProperty_RecordStartBeatPos]);
    textedit->setValue(efxAcc->GetPropertyValue(kAudioUnitCustomProperty_RecordStartBeatPos));
    textedit = reinterpret_cast<CMyTextEdit*> (mCntl[kAudioUnitCustomProperty_RecordLoopStartBeatPos]);
    textedit->setValue(efxAcc->GetPropertyValue(kAudioUnitCustomProperty_RecordLoopStartBeatPos));
    textedit = reinterpret_cast<CMyTextEdit*> (mCntl[kAudioUnitCustomProperty_RecordEndBeatPos]);
    textedit->setValue(efxAcc->GetPropertyValue(kAudioUnitCustomProperty_RecordEndBeatPos));
    
    textedit = reinterpret_cast<CMyTextEdit*> (mCntl[kAudioUnitCustomProperty_RepeatNumForSpc]);
    textedit->setValue(10 * efxAcc->GetPropertyValue(kAudioUnitCustomProperty_RepeatNumForSpc));
    textedit = reinterpret_cast<CMyTextEdit*> (mCntl[kAudioUnitCustomProperty_FadeMsTimeForSpc]);
    textedit->setValue(efxAcc->GetPropertyValue(kAudioUnitCustomProperty_FadeMsTimeForSpc));

    textedit = reinterpret_cast<CMyTextEdit*> (mCntl[kAudioUnitCustomProperty_SongPlayerCodeVer]);
    int codeVer = efxAcc->GetPropertyValue(kAudioUnitCustomProperty_SongPlayerCodeVer);
    if (codeVer > 0) {
        sprintf(str, "Valid [%08x]", codeVer);
        textedit->setText(str);
        cUrlLinkBox->setVisible(false);
    }
    else {
        textedit->setText("not Loaded");
        cUrlLinkBox->setVisible(true);
    }
    
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

bool RecordingSettingsGUI::getLoadFile( char *path, int maxLen, const char *title )
{
#if VSTGUI_NEW_CFILESELECTOR
	CNewFileSelector* selector = CNewFileSelector::create(getFrame(), CNewFileSelector::kSelectFile);
	if (selector)
	{
		if ( title ) selector->setTitle(title);
		selector->runModal();
		if ( selector->getNumSelectedFiles() > 0 ) {
			const char *url = selector->getSelectedFile(0);
			strncpy(path, url, maxLen-1);
			selector->forget();
			return true;
		}
		selector->forget();
		return false;
	}
#else
	CFileSelector OpenFile(0);
	VstFileSelect Filedata;
	memset(&Filedata, 0, sizeof(VstFileSelect));
	Filedata.command=kVstFileLoad;
	Filedata.type= kVstFileType;
	strncpy(Filedata.title, title, maxLen-1 );
	Filedata.nbFileTypes=0;
	Filedata.fileTypes=NULL;
	Filedata.returnPath= path;
	Filedata.initialPath = 0;
	Filedata.future[0] = 0;
	if (OpenFile.run(&Filedata) > 0) {
		return true;
	}
#endif
	return false;
}

bool RecordingSettingsGUI::getFolder( char *path, int maxLen, const char *title )
{
#if VSTGUI_NEW_CFILESELECTOR
	CNewFileSelector* selector = CNewFileSelector::create(getFrame(), CNewFileSelector::kSelectDirectory);
	if (selector)
	{
        selector->addFileExtension (CFileExtension ("folder", ""));
		if ( title ) selector->setTitle(title);
		selector->runModal();
		if ( selector->getNumSelectedFiles() > 0 ) {
			const char *url = selector->getSelectedFile(0);
			strncpy(path, url, maxLen-1);
			selector->forget();
			return true;
		}
		selector->forget();
		return false;
	}
#else
	CFileSelector OpenFile(0);
	VstFileSelect Filedata;
	memset(&Filedata, 0, sizeof(VstFileSelect));
	Filedata.command=kVstDirectorySelect;
	Filedata.type= kVstFileType;
	strncpy(Filedata.title, title, maxLen-1 );
	Filedata.nbFileTypes=0;
	Filedata.fileTypes=NULL;
	Filedata.returnPath= path;
	Filedata.initialPath = 0;
	Filedata.future[0] = 0;
	if (OpenFile.run(&Filedata) > 0) {
		return true;
	}
#endif
	return false;
}

bool RecordingSettingsGUI::loadCode(const char *path)
{
    bool isLoaded = efxAcc->LoadSongPlayerCode(path);
    CMyTextEdit *textedit = reinterpret_cast<CMyTextEdit*> (mCntl[kAudioUnitCustomProperty_SongPlayerCodeVer]);
    int codeVer = efxAcc->GetPropertyValue(kAudioUnitCustomProperty_SongPlayerCodeVer);
    if (codeVer > 0) {
        char str[20];
        sprintf(str, "Valid [%08x]", codeVer);
        textedit->setText(str);
        cUrlLinkBox->setVisible(false);
        setDirty();
    }
    else {
        textedit->setText("not Loaded");
        isLoaded = false;
    }
    return isLoaded;
}
