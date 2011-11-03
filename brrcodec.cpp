/*
 *  brrcodec.c
 *  Chip700
 *
 *  Created by 開発用 on 06/11/06.
 *  Copyright 2006 Vermicelli Magic. All rights reserved.
 *
 */

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


//入力データ：ネイティブエンディアン１６bit、モノラル、３２kHz
//inputframeが 16*n でないとき、先頭に無音を付け加えます
int brrencode(short *input_data, unsigned char *output_data, long inputframes)
{
	short			*input = input_data;
	unsigned char	*output = output_data;
	int				r,f,frm;
	long			frames_remain,frame_offset,outbytes=0;
	unsigned int	filter, range, half;
	long			blockdata[13][4][16];
	long			out1[13][4], out2[13][4], err_sum[13][4];
	
	input[0]=0;
	frame_offset = (16-(inputframes % 16))%16;
	frames_remain = inputframes;
	
	//作業変数の初期化
	for (f=0; f<4; f++) {
		for (r=0; r<13; r++) {
			out1[r][f] = 0;
			out2[r][f] = 0;
			err_sum[r][f] = 0;
		}
	}
	
	while (frames_remain > 0) {
		for (frm=0; frm<16; frm++) {
			for (f=0; f<4; f++) {
				for (r=0; r<13; r++) {
					long	nbl,c1,c2,out;
					
					if (frame_offset > 0)
						nbl = 0;
					else
						nbl = *input;
					
					c1=out1[r][f];
					c2=out2[r][f];
					
					//f値ごとの予測誤差を求める
					if (f==1)
						nbl -= filter1(c1);
					else if (f==2)
						nbl -= filter2(c1,c2);
					else if (f==3)
						nbl -= filter3(c1,c2);
					
					//4bitに量子化する
					if (r != 0)
					{
						nbl += 1<<(r-1);
					}
					nbl >>= r;
					if ( nbl > 7 )
					{
						nbl = 7;
					}
					else if ( nbl < -8 )
					{
						nbl = -8;
					}
					blockdata[r][f][frm] = nbl;
					
					//誤差を求めるためデコードする
					out=nbl<<r;
					if(f==1)
						out+=filter1(c1);
					else if(f==2)
						out+=filter2(c1,c2);
					else if(f==3)
						out+=filter3(c1,c2);
					if( out < -32768 )
					{
						out = -32768;
					}
					else if( out > 32767 )
					{
						out = 32767;
					}
					out2[r][f]=c1;
					out1[r][f]=out<<1;
					
					//元信号との差の絶対値を求める
					nbl=out-*input;
					err_sum[r][f] += nbl<0 ? -nbl:nbl;
				}
			}
			input++;
			frames_remain--;
			if (frame_offset-- > 0) {
				input--;
				frames_remain++;
			}
		}
		
		//誤差の合計が最小なフィルタ＆レンジを選択
		filter = 0;
		range = 0;
		for (f=0; f<4; f++) {
			for (r=0; r<13; r++) {
				if (err_sum[12-r][f] < err_sum[range][filter]) {
					filter = f;
					range = 12-r;
				}
			}
		}
		//選んだフィルタの過去信号に更新、誤差値の初期化
		for (f=0; f<4; f++) {
			for (r=0; r<13; r++) {
				out1[r][f]=out1[range][filter];
				out2[r][f]=out2[range][filter];
				err_sum[r][f] = 0;
			}
		}
		
		//Headerバイトの設定
		*output = range<<4;
		*output |= filter<<2;
		if (frames_remain <= 0)
			*output |= 1;	//ENDbitの付加
		output++;
		outbytes++;
		
		//データバイトの書き込み
		half = 0;
		for (frm=0; frm<16; frm++) {
			if (half == 0) {
				*output = (blockdata[range][filter][frm] << 4) & 0xf0;
				half = 1;
			}
			else {
				*output |= blockdata[range][filter][frm] & 0xf;
				half = 0;
				output++;
				outbytes++;
			}
		}
	}
	return outbytes;
}

bool checkbrrsize(unsigned char *src, int *size)
{
	int		count=9;
	while((*src & 1)==0) {
		src += 9;
		count += 9;
	}
	*size = count;
	return (*src & 2)?true:false;
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

