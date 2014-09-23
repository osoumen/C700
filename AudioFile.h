/*
 *  AudioFile.h
 *  C700
 *
 *  Created by osoumen on 12/10/10.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "C700defines.h"
#include "FileAccess.h"

class AudioFile : public FileAccess {
public:
	typedef struct {
		int		basekey,lowkey,highkey,loop,lp,lp_end;
		double	srcSamplerate;
	} InstData;
	
	typedef struct {
		unsigned int manufacturer;
		unsigned int product;
		unsigned int sample_period;
		unsigned int note;
		unsigned int pitch_fraction;
		unsigned int smpte_format;
		unsigned int smpte_offset;
		unsigned int loops;
		unsigned int sampler_data;
		
		unsigned int cue_id;
		unsigned int type;
		unsigned int start;
		unsigned int end;
		unsigned int fraction;
		unsigned int play_count;
	} WAV_smpl;
	
	AudioFile( const char *path, bool isWriteable );
	virtual ~AudioFile();
	
	virtual bool	Load();

	short	*GetAudioData();
	int		GetLoadedSamples();
	bool	GetInstData( InstData *instData );
	bool	IsVarid();	//ÉçÅ[ÉhÇ™â¬î\Ç≈Ç†ÇÈÇ»ÇÁtrue
	
private:
	static const unsigned int MAXIMUM_SAMPLES = 116480;	//65529-9*2
	static const unsigned int EXPAND_BUFFER = 4096;
	
	short		*m_pAudioData;
	int			mLoadedSamples;
	InstData	mInstData;
	
	float		mPi;
	
	int resampling(const float *src, int srcSamples, double srcRate, short *dst, int *dstSamples, double dstRate);
	float sinc(float p_x1);
};
