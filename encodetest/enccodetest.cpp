//レンジとフィルタの組み合わせを全て（４＊１３通り）試行し
//元信号との差が最小になるものを選ぶ


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "brrcodec.h"
#include "MiniAiff.h"

typedef struct spc_header {
	char	file_header[33];
	char	info_21h[2];
	char	headerhastag;
	char	version_minor;
	char	reg_pc[2];
	char	reg_a;
	char	reg_x;
	char	reg_y;
	char	psw;
	char	sp;
	char	reserved_2ch[2];
} spc_header;

static const char spc_main[] = {
    0xe8, 0x5d, 0xc4, 0xf2, 0xe8, 0x02, 0xc4, 0xf3, 0xe8, 0x00, 0xc5, 0x00, 0x02, 0x8d, 0x00, 0xcc,
    0x02, 0x02, 0xe8, 0x05, 0xc5, 0x01, 0x02, 0xc5, 0x03, 0x02, 0xcd, 0x00, 0xf5, 0x60, 0x04, 0x30,
    0x0b, 0xc4, 0xf2, 0x3d, 0xf5, 0x60, 0x04, 0xc4, 0xf3, 0x3d, 0x2f, 0xf0, 0x2f, 0xfe
};

static const int	SOUND_TABLE_POS = 0x560;
unsigned char soundtable[] = {
	0x00,	//VOL(L)
	0x00,           //Left volume (V1)
	0x01,	//VOL(R)
	0x00,           //Right volume (V1)
	0x02,	//P(L)
	0x00,
	0x03,	//P(H)
	0x10,          //Pitch is 1:2 of original
	0x04,	//SRCN
	0x00,           //Sample #0
	0x05,	//ADSR(1)
	0x9a,
	0x06,	//ADSR(2)
	0x00,
	

	0x10,	//VOL(L)
	0x7f,           //Left volume (V1)
	0x11,	//VOL(R)
	0x7f,           //Right volume (V1)
	0x12,	//P(L)
	0x00,
	0x13,	//P(H)
	0x10,          //Pitch is 1:2 of original
	0x14,	//SRCN
	0x00,           //Sample #0
	0x15,	//ADSR(1)
	0x00,
	0x17,	//GAIN
	0x7f,           //GAIN Settings
	
	
	0x0C,	//MVOL(L)
	0x3F ,          //Main volume left
	0x1C,	//MVOL(R)
	0x3F ,          //Main volume right
	0x2C,	//EVOL(L)
	0x00 ,          //Echo volume left
	0x3C,	//EVOL(R)
	0x00 ,          //Echo volume right
	0x4D,	//EON
	0x00 ,          //Echo off

	0x3D,	//NON
	0x00,           //Noise off
	0x2D,	//PMON
	0x02 ,          //Modulation off
	0x6C,	//FLG
	0x20 ,          //Mute off, echo off

	0x4C,	//KOL
	0x03 ,          //Key On for voice 1
		
	0xFF           //Terminator Byte (do not remove!)
};

int main (int argc, char * argv[]) {
		FILE	*fp;
	int		inframes;
	int		outsize;
	
	float			floatbuff[65536*2];
	short			readbuff[65536*2];
	unsigned char	writebuff[65536*2];
	
	static const int	SPC_FILE_SIZE = 66048;
	unsigned char	spc[SPC_FILE_SIZE];
	
	
	if (argc < 2) return 0;
	
	inframes = mAiffReadDataFromChannel(argv[1], floatbuff, 0, 65536*2, 0);
	
	//16bitに変換
	for (int i=0; i<inframes; i++ ) {
		float	temp = floatbuff[i] * 32768;
		if ( temp > 32767 ) {
			readbuff[i] = 32767;
		}
		else {
			readbuff[i] = temp;
		}
	}
	
	//高域強調処理
	emphasis((short*)readbuff, inframes);
	
	//エンコード処理
	int pad = 16-(inframes % 16);
	outsize=brrencode((short*)readbuff,writebuff,inframes, true, 0, 0);
	
	//SPCヘッダ初期化
	memset(spc, 0, SPC_FILE_SIZE);
	spc_header	*header = (spc_header*)spc;
	strncpy(header->file_header, "SNES-SPC700 Sound File Data v0.30", 33);
	header->info_21h[0] = 0x26;
	header->info_21h[1] = 0x26;
	header->headerhastag = 0x27;
	header->version_minor = 0x1e;
	header->reg_pc[0] = 0x00;
	header->reg_pc[1] = 0x04;
	//プログラムコード
	int main_code_sice = sizeof( spc_main );
	memcpy(spc+0x500, spc_main, main_code_sice);
	//再生周波数設定
	int samplerate = mAiffGetSampleRate(argv[1]);
	int	pitch = 4096 * samplerate / 32000;
	soundtable[5] = pitch & 0x00ff;
	soundtable[7] = pitch >> 8;
	soundtable[5+14] = pitch & 0x00ff;
	soundtable[7+14] = pitch >> 8;
	//サウンドテーブル
	memcpy(spc+SOUND_TABLE_POS, soundtable, sizeof(soundtable));
	
	//波形データ
	if ( outsize > (65536 - 0x500) ) {
		outsize = 65536 - 0x500;
	}
	memcpy(spc+0x600, writebuff, outsize);
	
	char	outfname[256];
	sprintf(outfname, "%s.spc", argv[1]);
	fp = fopen(outfname,"wb");

	fwrite(spc,sizeof(char),SPC_FILE_SIZE,fp);
	fclose(fp);
	
    return 0;
}

