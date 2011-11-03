/*
 *  brrcodec.h
 *  Chip700
 *
 *  Created by ŠJ”­—p on 06/11/06.
 *  Copyright 2006 Vermicelli Magic. All rights reserved.
 *
 */

int brrencode(short *input_data, unsigned char *output_data, long inputframes);
int brrdecode(unsigned char *src, short *output, int looppoint, int loops);
bool checkbrrsize(unsigned char *src, int *size);
int emphasis(short *data, unsigned int length);
