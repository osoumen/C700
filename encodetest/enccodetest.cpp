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
}spc_header;

static const int MAIN_CODE_SIZE = 0x2c;
const unsigned char spc_main[MAIN_CODE_SIZE] = {
	0xE8, 0x5D, 0xC4, 0xF2, 0xE8, 0x02, 0xC4, 0xF3, 0xE8, 0x00, 0xC5, 0x00, 0x02, 0xC5, 0x02, 0x02, 
	0xE8, 0x05, 0xC5, 0x01, 0x02, 0xC5, 0x03, 0x02, 0xCD, 0x00, 0xF5, 0x2C, 0x04, 0x30, 0x0B, 0xC4, 
	0xF2, 0x3D, 0xF5, 0x2C, 0x04, 0xC4, 0xF3, 0x3D, 0x2F, 0xF0, 0x2F, 0xFE
};

unsigned char soundtable[] = {
	0x00,
	0x7f,           //Left volume (V1)
	0x01,
	0x7f,           //Right volume (V1)
	0x02,
	0x00,
	0x03,
	0x10,          //Pitch is 1:2 of original

	0x04,
	0x00,           //Sample #0
	0x05,
	0x00,
	0x07,
	0x7f,           //GAIN Settings
	0x0C,
	0x7F ,          //Main volume left

	0x1C,
	0x7F ,          //Main volume right
	0x2C,
	0x00 ,          //Echo volume left
	0x3C,
	0x00 ,          //Echo volume right
	0x4D,
	0x00 ,          //Echo off

	0x3D,
	0x00,           //Noise off
	0x2D,
	0x00 ,          //Modulation off
	0x07,
	0x7F ,          //Gain off
	0x6C,
	0x20 ,          //Mute off, echo off

	0x4C,
	0x01 ,          //Key On for voice 1
		
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
	outsize=brrencode((short*)readbuff,writebuff,inframes);
	
	//SPCヘッダ初期化
	for ( int i=0; i<SPC_FILE_SIZE; i++ ) {
		spc[i] = 0;
	}
	spc_header	*header = (spc_header*)spc;
	strncpy(header->file_header, "SNES-SPC700 Sound File Data v0.30", 33);
	header->info_21h[0] = 0x26;
	header->info_21h[1] = 0x26;
	header->headerhastag = 0x27;
	header->version_minor = 0x1e;
	header->reg_pc[0] = 0x00;
	header->reg_pc[1] = 0x04;
	//SPCコード
	memcpy(spc+0x500, spc_main, MAIN_CODE_SIZE);
	//サンプリングレート調整
	int samplerate = mAiffGetSampleRate(argv[1]);
	int	pitch = 4096 * samplerate / 32000;
	soundtable[5] = pitch & 0x00ff;
	soundtable[7] = pitch >> 8;
	//サウンドテーブル
	memcpy(spc+0x500+MAIN_CODE_SIZE, soundtable, sizeof(soundtable));
	
	//波形データ
	if ( outsize > (65536 - 0x500) ) {
		outsize = 65536 - 0x500;
	}
	memcpy(spc+0x600, writebuff, outsize);
	
	if (argc >= 3) {
		fp = fopen(argv[2],"wb");
	}
	else {
		fp = fopen("output.spc","wb");
	}
	fwrite(spc,sizeof(char),SPC_FILE_SIZE,fp);
	fclose(fp);
	
    return 0;
}

