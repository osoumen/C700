/*
 *  brrcodec.h
 *  Chip700
 *
 *  Created by osoumen on 06/11/06.
 *  Copyright 2006 Vermicelli Magic. All rights reserved.
 *
 */

int brrencode(short *input_data, unsigned char *output_data, long inputframes, bool isLoop, long loop_point, int pad_frames);
int brrdecode(unsigned char *src, short *output, int looppoint, int loops);
int checkbrrsize(unsigned char *src, int *size);
int emphasis(short *data, unsigned int length);
