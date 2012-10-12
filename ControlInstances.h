/*
 *  ControlInstances.h
 *  Chip700
 *
 *  Created by osoumen on 12/10/04.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

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
