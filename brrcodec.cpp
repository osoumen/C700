/*
 *  brrcodec.c
 *  C700
 *
 *  Created by 開発用 on 06/11/06.
 *  Copyright 2006 osoumen. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <algorithm>
#include "brrcodec.h"

int IntAbs( int value ) { return value < 0 ? -value : value; }

#define XMSNES_LIKE_ENC

#define looploss_tolerance 30000
/*
#define filter1(a1)	(( a1 >> 1 ) + ( ( -a1 ) >> 5 ))
#define filter2(a1,a2)	(a1 \
						 + ( ( -( a1 + ( a1 >> 1 ) ) ) >> 5 ) \
						 - ( a2 >> 1 ) + ( a2 >> 5 ))
#define filter3(a1,a2)	(a1 \
						 + ( ( -( a1 + ( a1 << 2 ) \
								  + ( a1 << 3 ) ) ) >> 7 ) \
						 - ( a2 >> 1 ) \
						 + ( ( a2 + ( a2 >> 1 ) ) >> 4 ))

*/
#ifdef XMSNES_LIKE_ENC
int ComputeFilter( int x_2, int x_1, int filter )
{
	int cp = 0;
	switch( filter )
	{
		case 0:											// 0, 0
			cp = 0;										// add 0
			break;
		case 1:											// 15/16
			cp  = x_1;									// add 16/16
			cp += -x_1 >> 4;							// add -1/16
			break;
		case 2:											// 61/32, -15/16
			cp  = x_1<<1;								// add 64/32
			cp += -(x_1 + (x_1 << 1)) >> 5;				// add -3/32
			cp += -x_2;									// add -16/16
			cp += x_2 >> 4;								// add 1/16
			break;
		case 3:											// 115/64, -13/16
			cp  = x_1 << 1;								// add 128/64
			cp += -(x_1 + (x_1 << 2) + (x_1 << 3)) >> 6;// add -13/64
			cp += -x_2;									// add -16/16
			cp += (x_2 + (x_2 << 1)) >> 4;				// add 3/16
	}
	return cp;
}
#else
int ComputeFilter( int x_2, int x_1, int filter )
{
	int cp;
	switch( filter )
	{
		case 0:											// 0, 0
			cp = 0;										// add 0
			break;
		case 1:											// 15/16
			cp  = x_1 >> 1;									// add 16/16
			cp += -x_1 >> 5;							// add -1/16
			break;
		case 2:											// 61/32, -15/16
			cp  = x_1;								// add 64/32
			cp += -(x_1 + (x_1 >> 1)) >> 5;				// add -3/32
			cp += -(x_2 >> 1);									// add -16/16
			cp += x_2 >> 5;								// add 1/16
			break;
		case 3:											// 115/64, -13/16
			cp  = x_1;								// add 128/64
			cp += -(x_1 + (x_1 << 2) + (x_1 << 3)) >> 7;// add -13/64
			cp += -(x_2 >> 1);									// add -16/16
			cp += (x_2 + (x_2 >> 1)) >> 4;				// add 3/16
	}
	return cp;
}
#endif

//入力データ：ネイティブエンディアン１６bit、モノラル、３２kHz
//inputframeが 16*n でないとき、先頭に無音を付け加えます
int encodeBlock( const short *input, int frame_offset, int *out_1, int *out_2, int filter_fix, int range_fix, bool clip_fix, int *filter, int *range, int blockout[16], int blockdec[16], int *err );


int brrencode(short *input_data, unsigned char *output_data, long inputframes, bool isLoop, long loop_point, int pad_frames)
{
	int				frm;
	short			*input;
	unsigned char	*output;
	
	int				out_1, out_2;
	int				frames_remain,frame_offset,outbytes=0;
	int				*filter, *range, half;
	int				numblocks;
	int				current_block;
	int				loopstart_block = (pad_frames+loop_point)/16;
	int				loopstart_sample=0;
	int				blockout[16];
	int				blockdec[16];
	bool			use_filter0;
	bool			redo_loopf;
	int				adv_frame=0;
	
	//frame_offset = (16-(inputframes % 16))%16;
	frame_offset = pad_frames;
	numblocks = (inputframes+frame_offset) / 16;
	filter = new int[numblocks];
	range = new int[numblocks];
	
	use_filter0 = false;
	redo_loopf = true;
	frames_remain = inputframes;
	out_1 = out_2 = 0;
	input = input_data;
	output = output_data;
	current_block = 0;
	
	while ( redo_loopf ) {
		redo_loopf=false;
		//int	err_sum = 0;
		while (frames_remain > 0) {
//			printf("current_block=%d\n",current_block);
			
			int	err;
			int filter_fix = -1;
			int range_fix = -1;
			if ( current_block == 0 || use_filter0 ) {
				filter_fix = 0;
			}
			adv_frame = encodeBlock( input, frame_offset, &out_1, &out_2, filter_fix, range_fix, true, &filter[current_block], &range[current_block], blockout, blockdec, &err );
			use_filter0 = false;
			if ( current_block == loopstart_block ) {
				loopstart_sample = blockdec[0];
			}
			//		err_sum += err;
			input += adv_frame;
			frames_remain -= adv_frame;
			frame_offset -= 16-adv_frame;
			
			//Headerバイトの設定
//			printf("filter=%d\n",filter[current_block]);
			*output = range[current_block]<<4;
			*output |= filter[current_block]<<2;
			if (frames_remain <= 0) {
				*output |= 1;	//ENDbitの付加
			}
			if (isLoop) {
				*output |= 2;	//ループフラグの付加
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
		
		if ( isLoop ) {
			int lc_range;
			int lc_filter;
			int lc_value;
			int	loop_w = loopstart_block*9;
			lc_filter = (output_data[loop_w] & 0x0c) >> 2;
			if ( lc_filter != 0 ) {
				lc_range = (output_data[loop_w] & 0xf0) >> 4;
				lc_value = output_data[loop_w+1] >> 4;
				lc_value <<= lc_range;
//				printf("lc_value=%d\n",lc_value);
				input -= adv_frame;
				lc_value += ComputeFilter( input[13], input[12], lc_filter );
//				printf("out_2=%d\n",out_2);
//				printf("out_1=%d\n",out_1);
//				printf("lc_value=%d\n",lc_value);
//				printf("loopstart_sample=%d\n",loopstart_sample);
				if( IntAbs( lc_value - loopstart_sample ) > looploss_tolerance )
				{
					// ループ地点から再エンコード
					frame_offset = pad_frames;
					if ( 16*loopstart_block < frame_offset ) {
						frame_offset -= 16*loopstart_block;
						frames_remain = inputframes;
						input = input_data;
					}
					else {
						frames_remain = inputframes+frame_offset - 16*loopstart_block;
						input = input_data + (16*loopstart_block - frame_offset);
						frame_offset = 0;
					}
					output = output_data + loopstart_block*9;
					current_block = loopstart_block;
					outbytes = loopstart_block*9;
					
					redo_loopf = true;
					use_filter0 = true;
				}
			}
		}
	}
//	printf( "err_sum=%d\n",err_sum);
	
	delete [] filter;
	delete [] range;
	
	return outbytes;
}

int encodeBlock( const short *input, int frame_offset, int *out_1, int *out_2, int filter_fix, int range_fix, bool clip_fix, int *filter, int *range, int blockout[16], int blockdec[16], int *err )
{
	int				r,f,frm;
	int				out1[13][4], out2[13][4], err_sum[13][4];
	int				blockdata[13][4][16];
	int				blocksamp[13][4][16];
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
				int		df[2] = {0x7fffffff, 0x7fffffff};	//max
				
				if (frame_offset > 0) {
					in = 0;
				}
				else {
					in = (*input);
#ifdef XMSNES_LIKE_ENC
					in >>= 1;
#endif
				}
				
				c1=out1[r][f];
				c2=out2[r][f];
				
				//各フィルタの予測誤差を求める
				cp = ComputeFilter(c2,c1,f);
				
				for (int i=0; i<(clip_fix?2:1); i++ ) {
					
					nbl[i]	= in;
					nbl[i] -= cp;
					
#ifdef XMSNES_LIKE_ENC
					nbl[i] <<= 1;
#endif
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
#ifdef XMSNES_LIKE_ENC
					out[i] >>= 1;
#endif
					out[i] += cp;
				
#ifdef XMSNES_LIKE_ENC
					if ( f >= 2 ) {
						if( out[i] < -32768 ) {
							out[i] = -32768;
						}
						else if( out[i] > 32767 ) {
							out[i] = 32767;
						}
					}
#else
					if( out[i] < -65536 ) {
						out[i] = -65536;
					}
					else if( out[i] > 65535 ) {
						out[i] = 65535;
					}
#endif

					out[i] <<= 1;
					if ( clip_fix ) {
#ifdef XMSNES_LIKE_ENC
						out[i] = ((signed short)out[i])/*>>1*/;
#else
						out[i] = out[i] & 0x1ffff;
						if ( out[i] > 0xffff ) {
							out[i] = out[i] | 0xffff0000;
						}
#endif
					}

					//元信号との差の絶対値を求める
#ifdef XMSNES_LIKE_ENC
					df[i] = ( (out[i]) - ((*input)/*>>1*/) );
					df[i] = df[i]<0 ? -df[i]:df[i];
					
					if ( in < 0 ) {
						in = in & 0x7fff;
					}
					else {
						in = in | (~0x7fff);
					}
#else
					df[i] = out[i]-((*input)<<1);
					df[i] = df[i]<0 ? -df[i]:df[i];
					
					if ( in < 0 ) {
						in = in & 0xffff;
					}
					else {
						in = in | (~0xffff);
					}
#endif
				}
				
				if ( df[1] < df[0] ) {
					blockdata[r][f][frm] = nbl[1];
					blocksamp[r][f][frm] = out[1]>>1;
					out2[r][f]=c1;
					out1[r][f]=out[1]>>1;
					err_sum[r][f] += df[1];
				}
				else {
					blockdata[r][f][frm] = nbl[0];
					blocksamp[r][f][frm] = out[0]>>1;
					out2[r][f]=c1;
					out1[r][f]=out[0]>>1;
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
	*range = r_end;
	for (f=f_begin; f<=f_end; f++) {
		for (r=r_end; r>=r_begin; r--) {
			if (err_sum[r][f] < err_sum[*range][*filter]) {
				*filter = f;
				*range = r;
			}
		}
	}
	//選んだフィルタの過去信号採用
	*out_1 = out1[*range][*filter];
	*out_2 = out2[*range][*filter];
	
	//データの出力
	for (frm=0; frm<16; frm++) {
		blockout[frm] = blockdata[*range][*filter][frm];
		blockdec[frm] = blocksamp[*range][*filter][frm];
	}
	if ( err ) {
		*err = err_sum[*range][*filter];
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
	long	out=0,out1=0,out2=0, temp2=0;
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
			out1=temp2>>1;
            if ( range <= 0xC ) {
                out=(input<<range)>>1;
            }
            else {
                out = input & ~0x7FF;
            }
			
			out += ComputeFilter( out2, out1, filter );
			
			temp2 = out;
			if( temp2 < -32768 )
			{
				temp2 = -32768;
			}
			else if( temp2 > 32767 )
			{
				temp2 = 32767;
			}
			temp2 = (signed short)(temp2 << 1);
			output[now] = temp2;
			
			now++;
			
			input=temp&0xF;
			if(input>7)
				input |= ~0xF;
			
			out2=out1;
			out1=temp2>>1;
			if ( range <= 0xC ) {
                out=(input<<range)>>1;
            }
            else {
                out = input & ~0x7FF;
            }
			
			out += ComputeFilter( out2, out1, filter );
			
			temp2 = out;
			if( temp2 < -32768 )
			{
				temp2 = -32768;
			}
			else if( temp2 > 32767 )
			{
				temp2 = 32767;
			}
			temp2 = (signed short)(temp2 << 1);
			output[now] = temp2;
			
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
		if (max < std::abs(buf[i]))
			max=std::abs(buf[i]);
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

