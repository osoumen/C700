/*
**	fft_czt.h -- header file for fft.c and czt.c
**
**	Public domain by MIYASAKA Masaru <alkaid@coral.ocn.ne.jp> (Sep 15, 2003)
**
**	[TAB4]
*/

#ifndef FFT_CZT_H
#define FFT_CZT_H

	/* データ実数型 REAL の定義 (float or double) */
	/* この型を long double にする場合は、czt.c / fft.c で使われている
	 * 三角関数 sin() / cos() を long double 版 (C99 対応コンパイラでは
	 * sinl / cosl) に変更する必要あり */
typedef double REAL;

	/* FFT計算用 数表保持構造体 */
typedef struct {
	int  samples;		/* 標本点の数( 4以上の 2の整数乗に限る) */
	int  *bitrev;		/* ビット反転表 - 要素数は(samples) */
	REAL *sintbl;		/* 三角関数表 - 要素数は(samples*3/4) */
} fft_struct;

	/* CZT計算用 数表保持構造体 */
typedef struct {
	fft_struct fft;		/* 下請け FFT計算用 */
	int  samples;		/* 標本点の数 */
	int  samples_out;	/* 出力する標本点の数 */
	int  samples_ex;	/* 2の整数乗で、(samples + samples_out) <= samples_ex
						 * である最小の数 */
	REAL *wr, *wi;		/* 重みデータ - 要素数は(samples) */
	REAL *vr, *vi;		/* インパルス応答データ - 要素数は(samples_ex) */
	REAL *tr, *ti;		/* 作業用領域 - 要素数は(samples_ex) */
} czt_struct;

	/* fft.c */
int fft_init(fft_struct *fftp, int n);
void fft_end(fft_struct *fftp);
void fft(fft_struct *fftp, int inv, REAL re[], REAL im[]);

	/* czt.c */
int czt_init(czt_struct *cztp, int n, int no);
void czt_end(czt_struct *cztp);
void czt(czt_struct *cztp, int inv, REAL re[], REAL im[]);

int estimatebasefreq(short *src, int length);

#endif	/* FFT_CZT_H */
