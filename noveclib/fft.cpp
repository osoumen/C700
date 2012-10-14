/*
**	fft.c -- Fast Fourier Tramsform (FFT)
**	時間軸間引きFFT (Cooley--Tukeyのアルゴリズム)
**
**	『Ｃ言語による最新アルゴリズム事典』（奥村晴彦・著）に掲載の
**	プログラムを改変して利用させて頂きました.
**
**	Revised by MIYASAKA Masaru <alkaid@coral.ocn.ne.jp> (Sep 15, 2003)
**
**	[TAB4]
*/

#include <stdlib.h>
#include <math.h>

#include "fft_czt.h"

#undef PI
#define PI	3.1415926535897932384626433832795L	/* 円周率(π) */


/*
**	三角関数表を作る.
*/
static void make_sintbl(int n, REAL sintbl[])
{
	/* ↓精度が重要でないなら double でもいいかも */
	long double r, d = (2 * PI) / n;
	int i, n2 = n / 2, n4 = n / 4, n8 = n / 8;

	for (i = 0; i <= n8; i++) {
		r = d * i;
		sintbl[i]      = sin(r);			/* sin(2*PI*i/n) */
		sintbl[n4 - i] = cos(r);
	}
	for (i = 0; i < n4; i++) {
		sintbl[n2 - i] =  sintbl[i];
		sintbl[i + n2] = -sintbl[i];
	}
}


/*
**	ビット反転表を作る.
*/
static void make_bitrev(int n, int bitrev[])
{
	int i = 0, j = 0, k;
	int n2 = n / 2;

	for (;;) {
		bitrev[i] = j;
		if (++i >= n) break;
		k = n2;
		while (k <= j) { j -= k; k /= 2; }
		j += k;
	}
}


/*
**	FFT計算用構造体に対し、標本数 n 用の数表データを作成する。
**
**	fftp	= FFT計算用構造体へのポインタ
**	n		= 標本点の数 ( 4以上の 2の整数乗に限る)
**	return	= 0:正常終了 1:nが無効な数 2:メモリ不足
*/
int fft_init(fft_struct *fftp, int n)
{
	int i;

	for (i = 4; i < n; i *= 2) ;
	if (i != n) return 1;

	fftp->samples = n;
	fftp->sintbl  = (REAL*)malloc((n / 4 * 3) * sizeof(REAL));
	fftp->bitrev  = (int*)malloc(n * sizeof(int));
	if (fftp->sintbl == NULL || fftp->bitrev == NULL) {
		fft_end(fftp);
		return 2;
	}
	make_sintbl(n, fftp->sintbl);
	make_bitrev(n, fftp->bitrev);

	return 0;
}


/*
**	FFT計算用構造体の数表データを消去してそのメモリ領域を開放する。
**
**	fftp	= FFT計算用構造体へのポインタ
*/
void fft_end(fft_struct *fftp)
{
	if (fftp->sintbl != NULL) { free(fftp->sintbl); fftp->sintbl = NULL; }
	if (fftp->bitrev != NULL) { free(fftp->bitrev); fftp->bitrev = NULL; }

	fftp->samples = 0;
}


/*
**	高速Fourier変換 (FFT) (Cooley--Tukeyのアルゴリズム).
**	re[] が実部, im[] が虚部. 結果は re[],im[] に上書きされる.
**	inv!=0 (=TRUE) なら逆変換を行う. fftp には, 計算用データが
**	入っている構造体を指定する.
*/
void fft(fft_struct *fftp, int inv, REAL re[], REAL im[])
{
	REAL si, co, dr, di, t;
	int i, j, k, ik, h, d, k2, n, n4;

	n  = fftp->samples;
	n4 = n / 4;

	for (i = 0; i < n; i++) {		/* ビット反転 */
		j = fftp->bitrev[i];
		if (i < j) {
			t = re[i];  re[i] = re[j];  re[j] = t;
			t = im[i];  im[i] = im[j];  im[j] = t;
		}
	}
	for (k = 1; k < n; k = k2) {	/* 変換 */
		h = 0;  k2 = k + k;  d = n / k2;
		for (j = 0; j < k; j++) {
			co = fftp->sintbl[h + n4];
			si = fftp->sintbl[h];
			if (inv) si = -si;
			for (i = j; i < n; i += k2) {
				ik = i + k;
				dr = si * im[ik] + co * re[ik];
				di = co * im[ik] - si * re[ik];
				re[ik] = re[i] - dr;  re[i] += dr;
				im[ik] = im[i] - di;  im[i] += di;
			}
			h += d;
		}
	}
	if (!inv) {						/* 逆変換でないならnで割る */
		t = 1.0 / n;			/* 逆数をかける(除算は遅いので) */
		for (i = 0; i < n; i++) {
			re[i] *= t;
			im[i] *= t;
		}
	}
}

