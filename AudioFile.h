/*
 *  AudioFile.h
 *  Chip700
 *
 *  Created by 藤田 匡彦 on 12/10/10.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "Chip700defines.h"
#include "FileAccess.h"

class AudioFile : public FileAccess {
public:
	typedef struct {
		int		basekey,lowkey,highkey,loop,lp,lp_end;
		double	srcSamplerate;
	} InstData;
	
	typedef struct {
		unsigned long manufacturer;
		unsigned long product;
		unsigned long sample_period;
		unsigned long note;
		unsigned long pitch_fraction;
		unsigned long smpte_format;
		unsigned long smpte_offset;
		unsigned long loops;
		unsigned long sampler_data;
		
		unsigned long cue_id;
		unsigned long type;
		unsigned long start;
		unsigned long end;
		unsigned long fraction;
		unsigned long play_count;
	} WAV_smpl;
	
	AudioFile( const char *path, bool isWriteable );
	virtual ~AudioFile();
	
	virtual bool	Load();

	short	*GetAudioData();
	int		GetLoadedSamples();
	bool	GetInstData( InstData *instData );
	bool	IsVarid();	//ロードが可能であるならtrue
	
private:
	static const unsigned int EXPAND_BUFFER = 4096;
	
	short		*m_pAudioData;
	int			mLoadedSamples;
	InstData	mInstData;
};
