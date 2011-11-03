/*
**	czt.c -- Chirp z-Tramsform (CZT)
**	任意データ数での計算が可能な高速 DFT アルゴリズム
**
**	『トランジスタ技術』1993年2月号 P.363～368 の解説を参考にしています.
**
**	Public domain by MIYASAKA Masaru <alkaid@coral.ocn.ne.jp> (Sep 15, 2003)
**
**	[TAB4]
*/

#include <stdlib.h>
#include <math.h>

#include "czt.h"

#undef PI
#define PI	3.1415926535897932384626433832795L	/* 円周率(π) */


/*
**	重みデータ・インパルス応答データを作る
*/
static void make_cztdata(int n, int no, int nx, REAL wr[], REAL wi[],
                         REAL vr[], REAL vi[])
{
	/* ↓精度が重要でないなら double でもいいかも */
	long double r, d = PI / n;
	int i, j, n2 = n * 2;

	for (i = j = 0; i < n; i++, j = (j + (2 * i - 1)) % n2) {
		r = d * j;
		wr[i] =  cos(r);					/*  cos(i*i*PI/n) */
		wi[i] = -sin(r);					/* -sin(i*i*PI/n) */
	}

	vr[0] = nx;					/* nx: FFT順変換のスケーリングに対応する補正 */
	vi[0] = 0;
	for (i = 1; i < n; i++) {
		vr[i] = vr[nx - i] =  nx * wr[i];	/* nx * cos(i*i*PI/n) */
		vi[i] = vi[nx - i] = -nx * wi[i];	/* nx * sin(i*i*PI/n) */
	}
	for (i = no, j = nx - n; i <= j; i++, j--) {
		vr[j] = vr[i] = 0;					/* 残りの拡張部には 0 を */
		vi[j] = vi[i] = 0;
	}
}


/*
**	CZT計算用構造体に対し、標本数 n, 出力データ数 no 用の数表データを
**	作成する。
**
**	cztp	= CZT計算用構造体へのポインタ
**	n		= 標本点の数
**	no		= 出力するデータの数
**	return	= 0:正常終了 1:nが無効な数 2:メモリ不足
*/
int czt_init(czt_struct *cztp, int n, int no)
{
	int i, nx;

	if (n <= 1) return 1;
	if (no <= 1 || n < no) no = n;

	nx = n + no;		/* (n + no)を2の整数乗まで拡張する(nx) */
	for (i = 1; i < nx; i *= 2) ;
	nx = i;

	if (fft_init(&cztp->fft, nx) != 0) return 2;

	cztp->samples     = n;
	cztp->samples_out = no;
	cztp->samples_ex  = nx;
	cztp->wr = malloc(2 * n  * sizeof(REAL));
	cztp->vr = malloc(2 * nx * sizeof(REAL));
	cztp->tr = malloc(2 * nx * sizeof(REAL));
	if (cztp->wr == NULL || cztp->vr == NULL || cztp->tr == NULL) {
		czt_end(cztp);
		return 2;
	}
	cztp->wi = cztp->wr + n;
	cztp->vi = cztp->vr + nx;
	cztp->ti = cztp->tr + nx;

	make_cztdata(n, no, nx, cztp->wr, cztp->wi, cztp->vr, cztp->vi);
	fft(&cztp->fft, 0, cztp->vr, cztp->vi);

	return 0;
}


/*
**	CZT計算用構造体の数表データを消去してそのメモリ領域を開放する。
**
**	cztp	= CZT計算用構造体へのポインタ
*/
void czt_end(czt_struct *cztp)
{
	if (cztp->wr != NULL) { free(cztp->wr); cztp->wr = NULL; }
	if (cztp->vr != NULL) { free(cztp->vr); cztp->vr = NULL; }
	if (cztp->tr != NULL) { free(cztp->tr); cztp->tr = NULL; }

	cztp->samples     = 0;
	cztp->samples_out = 0;
	cztp->samples_ex  = 0;

	fft_end(&cztp->fft);
}


/*
**	CZT (Chirp z-Tramsform) アルゴリズムによる高速 DFT.
**	re[] が実部, im[] が虚部. 結果は re[],im[] に上書きされる.
**	inv!=0 (=TRUE) なら逆変換を行う. cztp には, 計算用データが
**	入っている構造体を指定する.
*/
void czt(czt_struct *cztp, int inv, REAL re[], REAL im[])
{
	REAL xr, xi, yr, yi, t;
	int i, n, no, nx;

	n  = cztp->samples;
	no = cztp->samples_out;
	nx = cztp->samples_ex;

	for (i = 0; i < n; i++) {		/* 入力の重みデータ乗算 */
		yr = cztp->wr[i];
		yi = cztp->wi[i];
		if (inv) yi = -yi;
		cztp->tr[i] = re[i] * yr - im[i] * yi;
		cztp->ti[i] = im[i] * yr + re[i] * yi;
	}
	for (; i < nx; i++) {			/* 残りの拡張部には 0 を */
		cztp->tr[i] = 0;
		cztp->ti[i] = 0;
	}

	fft(&cztp->fft, 0, cztp->tr, cztp->ti);

	for (i = 0; i < nx; i++) {		/* コンボリューション演算 */
		xr = cztp->tr[i];
		xi = cztp->ti[i];
		yr = cztp->vr[i];
		yi = cztp->vi[i];
		if (inv) yi = -yi;
		cztp->tr[i] = xr * yr - xi * yi;
		cztp->ti[i] = xi * yr + xr * yi;
	}

	fft(&cztp->fft, 1, cztp->tr, cztp->ti);

	for (i = 0; i < no; i++) {		/* 出力の重みデータ乗算 */
		yr = cztp->wr[i];
		yi = cztp->wi[i];
		if (inv) yi = -yi;
		re[i] = cztp->tr[i] * yr - cztp->ti[i] * yi;
		im[i] = cztp->ti[i] * yr + cztp->tr[i] * yi;
	}

	if (!inv) {						/* 逆変換でないならnで割る */
		t = 1.0 / n;			/* 逆数をかける(除算は遅いので) */
		for (i = 0; i < no; i++) {
			re[i] *= t;
			im[i] *= t;
		}
	}
}

//基本周波数を求める
int estimatebasefreq(short *src, int length)
{
	REAL	*real, *imag, *autoc;
	czt_struct cztd;
	int i, index=0, pitch, error, half=length/2;
	if (half > 530)
		half = 530;
	
	real = malloc(sizeof(REAL)*length);
	imag = malloc(sizeof(REAL)*length);
	autoc = malloc(sizeof(REAL)*length);
	
	for (i = 0; i < length; i++) {
		real[i] = src[i];
		imag[i] = 0;
	}
	error = czt_init(&cztd, length, length);
	if (error) {
		printf("error:%d\n",error);
	}
	czt(&cztd, 0, real, imag);
	
	//パワースペクトル化
	for (i = 0; i < length; i++) {
		real[i] = real[i]*real[i] + imag[i]*imag[i];
		real[i] = pow(real[i], 1.0/3.0);
		imag[i] = 0;
	}
	czt(&cztd, 1, real, imag);		/* 逆変換して自己相関を求める */
	czt_end(&cztd);
	
	//負値をクリップさせる
	for (i = 0; i < half; i++) {
		if (real[i] < 0.0)
			real[i] = 0.0;
		imag[i] = real[i];
	}
	for (i = 0; i < half; i++)
		if ((i % 2) == 0)
            real[i] -= imag[i/2];
		else
            real[i] -= ((imag[i/2] + imag[i/2 + 1]) / 2);
	//負値をクリップさせる
	/*
	for (i = 0; i < half; i++) {
		if (real[i] < 0.0)
			real[i] = 0.0;
	}
	*/
	//ピッチ推定
	for (i = 1; i < half; i++) {
		if (real[i] > real[index]) {
			index = i;
		}
	}
	
	pitch = length/index;
	
	free(real);
	free(imag);
	free(autoc);
	
	return pitch;
}

