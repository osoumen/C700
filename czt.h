/*
**	fft_czt.h -- header file for fft.c and czt.c
**
**	Public domain by MIYASAKA Masaru <alkaid@coral.ocn.ne.jp> (Sep 15, 2003)
**
**	[TAB4]
*/

#ifndef FFT_CZT_H
#define FFT_CZT_H

#include <Accelerate/Accelerate.h>

	/* CZT計算用 数表保持構造体 */
typedef struct {
	FFTSetup		fftsetup;		/* 下請け FFT計算用 */
	bool	no_czt;
	int		m;
	int		samples;		/* 標本点の数 */
	int		samples_out;	/* 出力する標本点の数 */
	int		samples_ex;	/* 2の整数乗で、(samples + samples_out) <= samples_ex
						 * である最小の数 */
	DSPSplitComplex w;		/* 重みデータ - 要素数は(samples) */
	DSPSplitComplex v;		/* インパルス応答データ - 要素数は(samples_ex) */
	DSPSplitComplex t;		/* 作業用領域 - 要素数は(samples_ex) */
} czt_struct;

	/* czt.c */
int czt_init(czt_struct *cztp, int n, int no);
void czt_end(czt_struct *cztp);
void czt(czt_struct *cztp, int inv, DSPSplitComplex *input);

int estimatebasefreq(short *src, int length);

#endif	/* FFT_CZT_H */
