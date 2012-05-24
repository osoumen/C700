/*
 *  brrcodec.c
 *  Chip700
 *
 *  Created by 開発用 on 06/11/06.
 *  Copyright 2006 Vermicelli Magic. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "brrcodec.h"

#define filter1(a1)	(( a1 >> 1 ) + ( ( -a1 ) >> 5 ))
#define filter2(a1,a2)	(a1 \
						 + ( ( -( a1 + ( a1 >> 1 ) ) ) >> 5 ) \
						 - ( a2 >> 1 ) + ( a2 >> 5 ))
#define filter3(a1,a2)	(a1 \
						 + ( ( -( a1 + ( a1 << 2 ) \
								  + ( a1 << 3 ) ) ) >> 7 ) \
						 - ( a2 >> 1 ) \
						 + ( ( a2 + ( a2 >> 1 ) ) >> 4 ))
/*
#define filter1(a1)	( a1 + ( ( -a1 ) >> 4 ))
#define filter2(a1,a2)	(( a1 << 1) \
						 + ( ( -( a1 + ( a1 << 1 ) ) ) >> 5 ) \
						 - a2 + ( a2 >> 4 ))
#define filter3(a1,a2)	(( a1 << 1 ) \
						 + ( -( a1 + ( a1 << 2 ) + ( a1 << 3 )) >> 6 ) \
						 - a2 \
						 + ( a2 + ( a2 << 1 )) >> 4 )
*/

//入力データ：ネイティブエンディアン１６bit、モノラル、３２kHz
//inputframeが 16*n でないとき、先頭に無音を付け加えます
int encodeBlock( const short *input, int frame_offset, int *out_1, int *out_2, int filter_fix, int range_fix, bool clip_fix, int *filter, int *range, int blockout[16] );

int brrencode(short *input_data, unsigned char *output_data, long inputframes)
{
	int				frm;
	short			*input;
	unsigned char	*output;
	
	int				out_1, out_2;
	long			frames_remain,frame_offset,outbytes=0;
	int				*filter, *range, half;
	int				numblocks;
	int				current_block;
	int				blockout[16];
	
	
	input_data[0]=0;
	out_1 = out_2 = 0;
	
	frame_offset = (16-(inputframes % 16))%16;
	frames_remain = inputframes;
	
	numblocks = (inputframes+frame_offset) / 16;
	filter = new int[numblocks];
	range = new int[numblocks];
	
	//1パス目--filter rangeを計算--
	/*
	input = input_data;
	current_block = 0;
	while (frames_remain > 0) {
		int adv_frame;
		adv_frame = encodeBlock( input, frame_offset, &out_1, &out_2, -1, -1, true, &filter[current_block], &range[current_block], blockout );
		input += adv_frame;
		frames_remain -= adv_frame;
		frame_offset = 0;
				
		current_block++;
	}
	 */
	
	//2パス目--filter rangeを固定してクリップ回避--
	input = input_data;
	output = output_data;
	out_1 = out_2 = 0;
	frame_offset = (16-(inputframes % 16))%16;
	frames_remain = inputframes;
	current_block = 0;
	while (frames_remain > 0) {
		int adv_frame;
		//adv_frame = encodeBlock( input, frame_offset, &out_1, &out_2, filter[current_block], range[current_block], true, &filter[current_block], &range[current_block], blockout );
		adv_frame = encodeBlock( input, frame_offset, &out_1, &out_2, -1, -1, false, &filter[current_block], &range[current_block], blockout );
		input += adv_frame;
		frames_remain -= adv_frame;
		frame_offset = 0;
		
		//Headerバイトの設定
		*output = range[current_block]<<4;
		*output |= filter[current_block]<<2;
		if (frames_remain <= 0) {
			*output |= 1;	//ENDbitの付加
		}
		output++;
		outbytes++;
		
		//データバイトの書き込み
		half = 0;
		for (frm=0; frm<16; frm++) {
			if (half == 0) {
				*output = (blockout[frm] << 4) & 0xf0;
				half = 1;
			}
			else {
				*output |= blockout[frm] & 0xf;
				half = 0;
				output++;
				outbytes++;
			}
		}
		
		current_block++;
	}
	
	delete [] filter;
	delete [] range;
	
	return outbytes;
}

int encodeBlock( const short *input, int frame_offset, int *out_1, int *out_2, int filter_fix, int range_fix, bool clip_fix, int *filter, int *range, int blockout[16] )
{
	int				r,f,frm;
	int				out1[13][4], out2[13][4], err_sum[13][4];
	int				blockdata[13][4][16];
	int				adv_frame = 0;
	
	int				f_begin = 0;
	int				f_end = 3;
	int				r_begin = 0;
	int				r_end = 12;
	
	//作業変数の初期化
	for (f=f_begin; f<=f_end; f++) {
		for (r=r_begin; r<=r_end; r++) {
			out1[r][f] = *out_1;
			out2[r][f] = *out_2;
			err_sum[r][f] = 0;
		}
	}
	
	if ( filter_fix >= 0 && filter_fix <= 3 ) {
		f_begin = f_end = filter_fix;
	}
	if ( range_fix >= 0 && range_fix <= 12 ) {
		r_begin = r_end = range_fix;
	}
	
	for (frm=0; frm<16; frm++) {
		for (f=f_begin; f<=f_end; f++) {
			for (r=r_begin; r<=r_end; r++) {
				int		in,nbl[2],c1,c2,out[2],cp=0;
				int		df[2] = {0x7fffffff, 0x7fffffff};
				
				if (frame_offset > 0)
					in = 0;
				else
					in = (*input);
				
				c1=out1[r][f];
				c2=out2[r][f];
				
				//各フィルタの予測誤差を求める
				if (f==1) {
					cp = filter1(c1);
				}
				else if (f==2) {
					cp = filter2(c1,c2);
				}
				else if (f==3) {
					cp = filter3(c1,c2);
				}
				else {
					cp = 0;
				}
				
				for (int i=0; i<(clip_fix?2:1); i++ ) {
					
					nbl[i]	= in;
					nbl[i] -= cp;
					
					//4bitに量子化する
					nbl[i] += (1 << r) >> 1;
					nbl[i] >>= r;
					if ( nbl[i] > 7 ) {
						nbl[i] = 7;
					}
					else if ( nbl[i] < -8 ) {
						nbl[i] = -8;
					}
					
					//最終的な誤差を求めるためデコードする
					out[i] = (nbl[i]<<r);
					out[i] += cp;
				
					if( out[i] < -65536 ) {
						out[i] = -65536;
					}
					else if( out[i] > 65535 ) {
						out[i] = 65535;
					}

					out[i] <<= 1;
					if ( clip_fix ) {
						//out[i] = (signed short)out[i];
						out[i] = out[i] & 0x1ffff;
						if ( out[i] > 0xffff ) {
							out[i] = out[i] | 0xffff0000;
						}
					}
					
					//元信号との差の絶対値を求める
					df[i] = out[i]-((*input)<<1);
					df[i] = df[i]<0 ? -df[i]:df[i];
					
					if ( in < 0 ) {
						in = in & 0xffff;
					}
					else {
						in = in | -0x10000;
					}
					
				}
				
				if ( df[0] > df[1] ) {
					blockdata[r][f][frm] = nbl[1];
					out2[r][f]=c1;
					out1[r][f]=out[1];
					err_sum[r][f] += df[1];
				}
				else {
					blockdata[r][f][frm] = nbl[0];
					out2[r][f]=c1;
					out1[r][f]=out[0];
					err_sum[r][f] += df[0];
				}
			}
		}
		input++;
		adv_frame++;
		if (frame_offset-- > 0) {
			input--;
			adv_frame--;
		}
	}
	
	//誤差の合計が最小なフィルタ＆レンジを選択
	*filter = f_begin;
	*range = r_begin;
	for (f=f_begin; f<=f_end; f++) {
		for (r=r_begin; r<=r_end; r++) {
			if (err_sum[r][f] < err_sum[*range][*filter]) {
				*filter = f;
				*range = r;
			}
		}
	}
	//選んだフィルタの過去信号採用
	*out_1 = out1[*range][*filter];
	*out_2 = out2[*range][*filter];
	
	for (frm=0; frm<16; frm++) {
		blockout[frm] = blockdata[*range][*filter][frm];
	}
	
	return adv_frame;
}

int checkbrrsize(unsigned char *src, int *size)
{
	int		count=9;
	while((*src & 1)==0) {
		src += 9;
		count += 9;
	}
	*size = count;
	return (*src & 2)?1:0;
}

int brrdecode(unsigned char *src, short *output, int looppoint, int loops)
{
	int range, end=0, loop, filter, counter, temp;
	short input;
	long	out=0,out1=0,out2=0, temp2;
	long	outb1=0,outb2=0;
	int		now=0;
	int		remainloop=loops-1;
	unsigned char *loopaddr = src+looppoint;
	
	while(end==0)
	{
        range=*(src++);
        end=range&1;
        loop=range&2;
        filter=(range>>2)&3;
        range>>=4;
		
        for(counter=0;counter<8;counter++)
        {
			temp=*(src++);
			input=temp>>4;
			input&=0xF;
			if(input>7)
				input |= ~0xF;
			
			out2=out1;
			out1=out<<1;
			out=input<<range;
			
			if(filter==1)
				out+=filter1(out1);
			else if(filter==2)
				out+=filter2(out1,out2);
			else if(filter==3)
				out+=filter3(out1,out2);
			
			temp2 = out;
			if( temp2 < -32768 )
			{
				temp2 = -32768;
			}
			else if( temp2 > 32767 )
			{
				temp2 = 32767;
			}
			output[now] = temp2;
			outb2=outb1;
			outb1=temp2;
			
			now++;
			
			input=temp&0xF;
			if(input>7)
				input |= ~0xF;
			
			out2=out1;
			out1=out<<1;
			out=input<<range;
			
			if(filter==1)
				out+=filter1(out1);
			else if(filter==2)
				out+=filter2(out1,out2);
			else if(filter==3)
				out+=filter3(out1,out2);
			
			temp2 = out;
			if( temp2 < -32768 )
			{
				temp2 = -32768;
			}
			else if( temp2 > 32767 )
			{
				temp2 = 32767;
			}
			output[now] = temp2;
			outb2=outb1;
			outb1=temp2;
			
			now++;
        }
		if (end) {
			if (remainloop > 0) {
				remainloop--;
				end = 0;
				src = loopaddr;
			}
		}
	}
	return now;
}

int emphasis(short *data, unsigned int length)
{
	unsigned int	i;
	double	now=.0,a1=.0,a2=.0,*buf,max=.0;
	
	buf=(double*)malloc(length*sizeof(double));
	
	for (i=0; i<length; i++) {
		now=data[i];
		buf[i] = now*1.72713771313805 - a1*0.63418337904287769 - a2*0.10054429474861125;	//平滑フィルタの逆スペクトルのLPC係数より
		a2=a1;
		a1=buf[i];
		if (max < abs(buf[i]))
			max=abs(buf[i]);
	}
	if (max > 32767) {
		for (i=0; i<length; i++)
			data[i]=buf[i]*32767/max;
	} else {
		for (i=0; i<length; i++)
			data[i]=buf[i];
	}
	
	free(buf);
	return 0;
}

