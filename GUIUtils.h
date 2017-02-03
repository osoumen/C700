//
//  GUIUtils.h
//  C700
//
//  Created by osoumen on 2017/02/03.
//
//

#ifndef __C700__GUIUtils__
#define __C700__GUIUtils__

#include "vstgui.h"

typedef struct {
	unsigned int	kind_sig;
	unsigned int	kind;
	char			title[256];
	int				value;
	int				minimum;
	int				maximum;
	unsigned int	sig;
	int				id;
	unsigned int	command;
	int				x;
	int				y;
	int				w;
	int				h;
	char			fontname[256];
	int				fontsize;
	CHoriTxtAlign	fontalign;
	int				style;
	int				futureuse;
} ControlInstances;

int splitMenuItem( char *str, const char *delim, char *outlist[] );
void getFileNameParentPath( const char *path, char *out, int maxLen );
void getFileNameDeletingPathExt( const char *path, char *out, int maxLen );
void getFileNameExt( const char *path, char *out, int maxLen );
CControl *makeControlFrom( const ControlInstances *desc, CFrame *frame, CControlListener* listener, CBitmap *sliderHandleBitmap, CBitmap *onOffButton, CBitmap *bgKnob, CBitmap *rocker );

#endif /* defined(__C700__GUIUtils__) */
