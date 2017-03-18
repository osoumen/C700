//
//  GUIUtils.cpp
//  C700
//
//  Created by osoumen on 2017/02/03.
//
//

#include "GUIUtils.h"

#include "vstgui.h"
#include "DummyCntl.h"
#include "MyKnob.h"
#include "MySlider.h"
#include "LabelOnOffButton.h"
#include "WaveView.h"
#include "SeparatorLine.h"
#include "MyParamDisplay.h"
#include "MyTextEdit.h"
#include "TextKickButton.h"
#include <stdio.h>

static CFontDesc g_LabelFont("Arial", 9, kBoldFace);
CFontRef kLabelFont = &g_LabelFont;

//-----------------------------------------------------------------------------
#define MAXMENUITEM 20
int splitMenuItem( char *str, const char *delim, char *outlist[] )
{
    char    *tk;
    int     cnt = 0;
    
    tk = strtok( str, delim );
    while( tk != NULL && cnt < MAXMENUITEM ) {
        outlist[cnt++] = tk;
        tk = strtok( NULL, delim );
    }
    return cnt;
}

//-----------------------------------------------------------------------------
void getFileNameParentPath( const char *path, char *out, int maxLen )
{
#if MAC
	CFURLRef	url = CFURLCreateFromFileSystemRepresentation(NULL, (UInt8*)path, strlen(path), false);
	CFURLRef	extlesspath=CFURLCreateCopyDeletingLastPathComponent(NULL, url);
	CFStringRef	filename = CFURLCopyFileSystemPath(extlesspath, kCFURLPOSIXPathStyle);
	CFStringGetCString(filename, out, maxLen-1, kCFStringEncodingUTF8);
	CFRelease(filename);
	CFRelease(extlesspath);
	CFRelease(url);
#else
	// Windowsでの親フォルダパス取得処理
	int	len = static_cast<int>(strlen(path));
	int bcPos = 0;
	for ( int i=0; i<len; i++ ) {
		if (path[i] == '\\') {
			bcPos = i;
		}
	}
	strncpy(out, path, bcPos);
	out[bcPos] = 0;
#endif
}

//-----------------------------------------------------------------------------
void getFileNameDeletingPathExt( const char *path, char *out, int maxLen )
{
#if MAC
	CFURLRef	url = CFURLCreateFromFileSystemRepresentation(NULL, (UInt8*)path, strlen(path), false);
	CFURLRef	extlesspath=CFURLCreateCopyDeletingPathExtension(NULL, url);
	CFStringRef	filename = CFURLCopyLastPathComponent(extlesspath);
	CFStringGetCString(filename, out, maxLen-1, kCFStringEncodingUTF8);
	CFRelease(filename);
	CFRelease(extlesspath);
	CFRelease(url);
#else
	//Windowsでの拡張子、パス除去処理
	int	len = static_cast<int>(strlen(path));
	int extPos = len;
	int bcPos = 0;
	for ( int i=len-1; i>=0; i-- ) {
		if ( path[i] == '.' ) {
			extPos = i;
			break;
		}
	}
	for ( int i=0; i<len; i++ ) {
		if (path[i] == '\\') {
			bcPos = i+1;
		}
	}
	strncpy(out, path+bcPos, extPos-bcPos);
	out[extPos-bcPos] = 0;
#endif
}

//-----------------------------------------------------------------------------
void getFileNameExt( const char *path, char *out, int maxLen )
{
#if MAC
	CFURLRef	url = CFURLCreateFromFileSystemRepresentation(NULL, (UInt8*)path, strlen(path), false);
	CFStringRef	ext = CFURLCopyPathExtension(url);
	CFStringGetCString(ext, out, maxLen-1, kCFStringEncodingUTF8);
	CFRelease(ext);
	CFRelease(url);
#else
	//Windowsでの拡張子抽出処理
	int	len = static_cast<int>(strlen(path));
	int extPos = len;
	for ( int i=len-1; i>=0; i-- ) {
		if ( path[i] == '.' ) {
			extPos = i+1;
			break;
		}
	}
	strncpy(out, path+extPos, maxLen);
	out[maxLen-1] = 0;
#endif
}

//-----------------------------------------------------------------------------
CControl *makeControlFrom( const ControlInstances *desc, CFrame *frame, CControlListener* listener, CBitmap *sliderHandleBitmap, CBitmap *onOffButton, CBitmap *bgKnob, CBitmap *rocker )
{
	CControl	*cntl = NULL;
	CRect cntlSize(0, 0, desc->w , desc->h);
	cntlSize.offset(desc->x, desc->y);
	int	value = desc->value;
	int	minimum = desc->minimum;
	int	maximum = desc->maximum;
	
	switch (desc->kind_sig) {
		case 'VeMa':
			switch (desc->kind ) {
				case 'wave':
				{
					CWaveView	*waveview;
					waveview = new CWaveView(cntlSize, frame, listener, desc->id);
					cntl = waveview;
					break;
				}
                case 'spls':
                {
                    char		rsrcName[100];
					snprintf(rsrcName, 99, "%s.png", desc->title);
					CBitmap		*helpPicture = new CBitmap(rsrcName);
                    CRect toDisplay(10, 10, helpPicture->getWidth()+10, helpPicture->getHeight()+10);
                    CSplashScreen *splash;
                    splash = new CSplashScreen(cntlSize, listener, desc->id, helpPicture, toDisplay);
                    helpPicture->forget();
                    cntl = splash;
                    break;
                }
                case 'menu':
                {
                    long style = kCheckStyle;
                    COptionMenu *optionMenu = new COptionMenu(cntlSize, listener, desc->id, 0, 0, style);
                    if (optionMenu)
                    {
                        CFontRef fontDesc = new CFontDesc(kLabelFont->getName(), desc->fontsize);
                        optionMenu->setFont(fontDesc);
                        optionMenu->setFontColor(MakeCColor(180, 248, 255, 255));
                        optionMenu->setBackColor(kBlackCColor);
                        optionMenu->setFrameColor(kBlackCColor);
                        optionMenu->setHoriAlign(desc->fontalign);
                        char    *menuItemList[MAXMENUITEM];
                        char    itemText[512];
                        strcpy(itemText, desc->title);
                        int itemNum = splitMenuItem(itemText, ";", menuItemList);
                        for (int i=0; i<itemNum; i++) {
                            optionMenu->addEntry(menuItemList[i]);
                        }
                    }
                    cntl = optionMenu;
                    break;
                }
                case 'push':
				{
					CFontRef	fontDesc;
					int			fontsize = 9;
					if ( desc->fontsize > 0 )
					{
						fontsize = desc->fontsize;
					}
					if ( desc->fontname[0] != 0 || desc->fontsize != 0)
					{
						if ( desc->fontname[0] == 0 )
						{
							fontDesc = new CFontDesc(kLabelFont->getName(), fontsize);
						}
						else
						{
							fontDesc = new CFontDesc(desc->fontname, fontsize);
						}
					}
					else
					{
                        fontDesc = new CFontDesc(*kLabelFont);
					}
                    
                    CTextKickButton	*button;
					button = new CTextKickButton(cntlSize, listener, desc->id, NULL, desc->title, fontDesc );
                    fontDesc->forget();
                    
					cntl = button;
					break;
				}
				default:
					goto makeDummy;
					break;
			}
			break;
			
		case 'airy':
			switch (desc->kind) {
				case 'slid':
				{
					CMySlider	*slider;
					if ( desc->w < desc->h )
					{
						slider = new CMySlider(cntlSize, listener, desc->id, cntlSize.top, cntlSize.bottom - sliderHandleBitmap->getHeight(), sliderHandleBitmap, 0, CPoint(0, 0), kBottom|kVertical );
					}
					else
					{
						slider = new CMySlider(cntlSize, listener, desc->id, cntlSize.left, cntlSize.right - sliderHandleBitmap->getWidth(), sliderHandleBitmap, 0, CPoint(0, 0), kLeft|kHorizontal );
					}
					cntl = slider;
					break;
				}
				case 'knob':
				{
					CMyKnob		*knob;
					knob = new CMyKnob(cntlSize, listener, desc->id, bgKnob, 0);
					knob->setColorHandle( MakeCColor(67, 75, 88, 255) );
					knob->setColorShadowHandle( kTransparentCColor );
					knob->setInsetValue(1);
					cntl = knob;
					break;
				}
				case 'cbtn':
				{
					CLabelOnOffButton	*button;
					button = new CLabelOnOffButton(cntlSize, listener, desc->id, onOffButton, desc->title);
					cntl = button;
					break;
				}
				case 'dtxt':
				{
					char fontName[100], unitStr[100];
					float fontSize, fontRColour, fontGColour, fontBColour, horizBorder, vertBorder, valueMultipler;
					sscanf(desc->title, "%s %f %f %f %f %f %f %f %s", fontName, &fontSize, &fontRColour, &fontGColour, &fontBColour, &horizBorder, &vertBorder, &valueMultipler, unitStr);
					
					CFontRef	fontDesc = new CFontDesc(fontName, fontSize);
					CMyParamDisplay	*paramdisp;
					paramdisp = new CMyParamDisplay(cntlSize, desc->id, valueMultipler, unitStr, 0, 0);
					paramdisp->setListener(listener);
					paramdisp->setFont(fontDesc);
					paramdisp->setFontColor(MakeCColor(fontRColour, fontGColour, fontBColour, 255));
					paramdisp->setAntialias(true);
                    
					fontDesc->forget();
					cntl = paramdisp;
					break;
				}
				case 'eutx':
				{
					CMyTextEdit	*textEdit;
                    cntlSize.offset(0, -2);
					textEdit = new CMyTextEdit(cntlSize, listener, desc->id, desc->title, desc->futureuse==2?true:false, desc->futureuse==1?true:false);
					textEdit->setFontColor(MakeCColor(180, 248, 255, 255));
					textEdit->setAntialias(true);
					
					CFontRef	fontDesc;
					int			fontsize = 10;
					if ( desc->fontsize > 0 )
					{
						fontsize = desc->fontsize;
					}
					if ( desc->fontname[0] != 0 )
					{
						fontDesc = new CFontDesc(desc->fontname, fontsize);
					}
					else
					{
						fontDesc = new CFontDesc("Monaco", fontsize);
					}
					textEdit->setFont(fontDesc);
					textEdit->setHoriAlign(desc->fontalign);
					fontDesc->forget();
					cntl = textEdit;
					
					if ( desc->futureuse==1 )
					{
						//小数型のとき最大値の制限を外す
						maximum = 0x7fffffff;
					}
					break;
				}
				case 'valp':
				{
					char		rsrcName[100];
					snprintf(rsrcName, 99, "%s.png", desc->title);
					CBitmap		*btnImage = new CBitmap(rsrcName);
					CMovieBitmap	*button;
					button = new CMovieBitmap(cntlSize, listener, desc->id, btnImage );
					btnImage->forget();
					cntl = button;
					break;
				}
				case 'bttn':
				{
					char		rsrcName[100];
					snprintf(rsrcName, 99, "%s.png", desc->title);
					CBitmap		*btnImage = new CBitmap(rsrcName);
					COnOffButton	*button;
					button = new COnOffButton(cntlSize, listener, desc->id, btnImage );
					btnImage->forget();
					cntl = button;
					break;
				}
				case 'push':
				{
					char		rsrcName[100];
					snprintf(rsrcName, 99, "%s.png", desc->title);
					CBitmap		*btnImage = new CBitmap(rsrcName);
					CKickButton	*button;
					button = new CKickButton(cntlSize, listener, desc->id, btnImage );
					btnImage->forget();
					cntl = button;
					break;
				}
				case 'hzsw':
				{
					char		rsrcName[100];
					snprintf(rsrcName, 99, "%s.png", desc->title);
					CBitmap				*btnImage = new CBitmap(rsrcName);
					CHorizontalSwitch	*button;
					int					subPixmaps = desc->maximum-desc->minimum+1;
					CCoord				heightOfOneImage = btnImage->getHeight()/subPixmaps;
					button = new CHorizontalSwitch(cntlSize, listener, desc->id, subPixmaps, heightOfOneImage, subPixmaps, btnImage );
					btnImage->forget();
					cntl = button;
					break;
				}
				default:
					goto makeDummy;
					break;
			}
			break;
			
		case 'appl':
			switch (desc->kind) {
				case 'stxt':
				{
					CTextLabel	*textLabel;
					cntlSize.offset(0, -2);	//位置補正
					textLabel = new CTextLabel(cntlSize, desc->title, 0, desc->style);
					textLabel->setFontColor(kBlackCColor);
					textLabel->setHoriAlign(desc->fontalign);
					textLabel->setTransparency(true);
					textLabel->setAntialias(true);
					textLabel->setTag(desc->id);
					
					CFontRef	fontDesc;
					int			fontsize = 9;
					if ( desc->fontsize > 0 )
					{
						fontsize = desc->fontsize;
					}
					if ( desc->fontname[0] != 0 || desc->fontsize != 0)
					{
						if ( desc->fontname[0] == 0 )
						{
							fontDesc = new CFontDesc(kLabelFont->getName(), fontsize);
						}
						else
						{
							fontDesc = new CFontDesc(desc->fontname, fontsize);
						}
						textLabel->setFont(fontDesc);
						fontDesc->forget();
					}
					else
					{
						fontDesc = new CFontDesc(g_LabelFont);
						textLabel->setFont(fontDesc);
					}
					cntl = textLabel;
					break;
				}
				case 'larr':
				{
					CRockerSwitch *rockerSwitch;
					rockerSwitch = new CRockerSwitch(cntlSize, listener, desc->id, rocker->getHeight() / 3, rocker, CPoint(0, 0), kVertical);
					cntl = rockerSwitch;
					minimum = -1;
					maximum = 1;
					break;
				}
				case 'sepa':
				{
					CSeparatorLine	*sepa;
					sepa = new CSeparatorLine(cntlSize, listener, desc->id);
					cntl = sepa;
					break;
				}
				default:
					goto makeDummy;
			}
			break;
			
		default:
			goto makeDummy;
	}
	goto setupCntl;
	
makeDummy:
	cntl = new CDummyCntl(cntlSize);
	
setupCntl:
	if ( cntl )
	{
		cntl->setMin(minimum);
		cntl->setMax(maximum);
		cntl->setValue(value);
		//cntl->setAttribute(kCViewTooltipAttribute,strlen(desc->title)+1,desc->title);
	}
	return cntl;
}
