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
                    efxAcc->SetSongInfoString(propertyId, text);
                }
                break;
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
					isSelected = getFolder(path, PATH_LEN_MAX, "");
					if ( isSelected ) {
						efxAcc->SetSongRecordPath(path);
                        CTextLabel *textlabel = reinterpret_cast<CTextLabel*> (mCntl[kAudioUnitCustomProperty_SongRecordPath]);
                        textlabel->setText(path);
					}
				}
                break;
                
            case kControlButtonSetRecordStart:
            {
                if ( value > 0 ) {
                    double pos = efxAcc->GetHostBeatPos();
                    efxAcc->SetPropertyValue(kAudioUnitCustomProperty_RecordStartBeatPos, pos);
                    mCntl[kAudioUnitCustomProperty_RecordStartBeatPos]->setValue(pos);
                }
                break;
            }
            case kControlButtonSetRecordLoopStart:
            {
                if ( value > 0 ) {
                    double pos = efxAcc->GetHostBeatPos();
                    efxAcc->SetPropertyValue(kAudioUnitCustomProperty_RecordLoopStartBeatPos, pos);
                    mCntl[kAudioUnitCustomProperty_RecordLoopStartBeatPos]->setValue(pos);
                }
                break;
            }
            case kControlButtonSetRecordEnd:
            {
                if ( value > 0 ) {
                    double pos = efxAcc->GetHostBeatPos();
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
                        efxAcc->LoadSongPlayerCode(path);
					}
                    CMyTextEdit *textedit = reinterpret_cast<CMyTextEdit*> (mCntl[kAudioUnitCustomProperty_SongPlayerCodeVer]);
                    int codeVer = efxAcc->GetPropertyValue(kAudioUnitCustomProperty_SongPlayerCodeVer);
                    if (codeVer > 0) {
                        char str[20];
                        sprintf(str, "%08x...OK!", codeVer);
                        textedit->setText(str);
                    }
                    else {
                        textedit->setText("not Loaded");
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
    efxAcc->GetSongRecordPath(str, PATH_LEN_MAX);
    textlabel->setText(str);
    
    CMyTextEdit		*textedit;
    textedit = reinterpret_cast<CMyTextEdit*> (mCntl[kAudioUnitCustomProperty_GameTitle]);
    efxAcc->GetSongInfoString(kAudioUnitCustomProperty_GameTitle, str, 33);
    textedit->setText(str);
    textedit = reinterpret_cast<CMyTextEdit*> (mCntl[kAudioUnitCustomProperty_SongTitle]);
    efxAcc->GetSongInfoString(kAudioUnitCustomProperty_SongTitle, str, 33);
    textedit->setText(str);
    textedit = reinterpret_cast<CMyTextEdit*> (mCntl[kAudioUnitCustomProperty_NameOfDumper]);
    efxAcc->GetSongInfoString(kAudioUnitCustomProperty_NameOfDumper, str, 17);
    textedit->setText(str);
    textedit = reinterpret_cast<CMyTextEdit*> (mCntl[kAudioUnitCustomProperty_ArtistOfSong]);
    efxAcc->GetSongInfoString(kAudioUnitCustomProperty_ArtistOfSong, str, 33);
    textedit->setText(str);
    textedit = reinterpret_cast<CMyTextEdit*> (mCntl[kAudioUnitCustomProperty_SongComments]);
    efxAcc->GetSongInfoString(kAudioUnitCustomProperty_SongComments, str, 33);
    textedit->setText(str);
    
    textedit = reinterpret_cast<CMyTextEdit*> (mCntl[kAudioUnitCustomProperty_RecordStartBeatPos]);
    textedit->setValue(efxAcc->GetPropertyValue(kAudioUnitCustomProperty_RecordStartBeatPos));
    textedit = reinterpret_cast<CMyTextEdit*> (mCntl[kAudioUnitCustomProperty_RecordLoopStartBeatPos]);
    textedit->setValue(efxAcc->GetPropertyValue(kAudioUnitCustomProperty_RecordLoopStartBeatPos));
    textedit = reinterpret_cast<CMyTextEdit*> (mCntl[kAudioUnitCustomProperty_RecordEndBeatPos]);
    textedit->setValue(efxAcc->GetPropertyValue(kAudioUnitCustomProperty_RecordEndBeatPos));

    textedit = reinterpret_cast<CMyTextEdit*> (mCntl[kAudioUnitCustomProperty_SongPlayerCodeVer]);
    int codeVer = efxAcc->GetPropertyValue(kAudioUnitCustomProperty_SongPlayerCodeVer);
    if (codeVer > 0) {
        sprintf(str, "%08x...OK!", codeVer);
        textedit->setText(str);
    }
    else {
        textedit->setText("not Loaded");
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
	VstFileType brrType("AddmusicM(Raw) BRR Sample", "", "brr", "brr");
	VstFileType waveType("Wave File", "WAVE", "wav", "wav",  "audio/wav", "audio/x-wav");
	VstFileType spcType("SPC File", "", "spc", "spc");
	VstFileType types[] = {brrType, waveType, spcType};
    
    //	CFileSelector OpenFile( ((AEffGUIEditor *)getEditor())->getEffect() );
	CFileSelector OpenFile(0);
	VstFileSelect Filedata;
	memset(&Filedata, 0, sizeof(VstFileSelect));
	Filedata.command=kVstFileLoad;
	Filedata.type= kVstFileType;
	strncpy(Filedata.title, title, maxLen-1 );
	Filedata.nbFileTypes=3;
	Filedata.fileTypes=types;
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
	VstFileType brrType("AddmusicM(Raw) BRR Sample", "", "brr", "brr");
	VstFileType waveType("Wave File", "WAVE", "wav", "wav",  "audio/wav", "audio/x-wav");
	VstFileType spcType("SPC File", "", "spc", "spc");
	VstFileType types[] = {brrType, waveType, spcType};
    
    //	CFileSelector OpenFile( ((AEffGUIEditor *)getEditor())->getEffect() );
	CFileSelector OpenFile(0);
	VstFileSelect Filedata;
	memset(&Filedata, 0, sizeof(VstFileSelect));
	Filedata.command=kVstDirectorySelect;
	Filedata.type= kVstFileType;
	strncpy(Filedata.title, title, maxLen-1 );
	Filedata.nbFileTypes=3;
	Filedata.fileTypes=types;
	Filedata.returnPath= path;
	Filedata.initialPath = 0;
	Filedata.future[0] = 0;
	if (OpenFile.run(&Filedata) > 0) {
		return true;
	}
#endif
	return false;
}