/*
 *  AudioFile.cpp
 *  C700
 *
 *  Created by osoumen on 12/10/10.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "AudioFile.h"
#include "vstgui.h"

#if MAC
#include <AudioToolbox/AudioFile.h>
#include <AudioToolbox/AudioConverter.h>
#else
#pragma comment ( lib, "winmm.lib" )
#include <mmsystem.h>
#include <math.h>
#endif

//-----------------------------------------------------------------------------
AudioFile::AudioFile( const char *path, bool isWriteable )
: FileAccess(path, isWriteable)
, m_pAudioData( NULL )
, mLoadedSamples( 0 )
{
	mPi = acosf(-1.0f);
}

//-----------------------------------------------------------------------------
AudioFile::~AudioFile()
{
	if ( m_pAudioData ) {
		delete [] m_pAudioData;
	}
}

//-----------------------------------------------------------------------------
bool AudioFile::IsVarid()
{
#if MAC
	AudioFileID mAudioFileID;
	FSRef	ref;
	Boolean	isDirectory=false;
	FSPathMakeRef((const UInt8*)GetFilePath(), &ref, &isDirectory);
	OSStatus err = AudioFileOpen(&ref, fsRdPerm, 0, &mAudioFileID);
	if (err == noErr) {
		AudioFileClose(mAudioFileID);
		return true;
	}
#endif
	return false;
}

//-----------------------------------------------------------------------------
bool AudioFile::Load()
{
#if MAC
	AudioFileID mAudioFileID;
    AudioStreamBasicDescription fileDescription, outputFormat;
    SInt64 dataSize64;
    UInt32 dataSize;
	
	OSStatus err;
	UInt32 size;
	
    // �t�@�C�����J��
	FSRef	ref;
	Boolean	isDirectory=false;
	FSPathMakeRef((const UInt8*)GetFilePath(), &ref, &isDirectory);
	
	err = AudioFileOpen(&ref, fsRdPerm, 0, &mAudioFileID);
    if (err) {
        //NSLog(@"AudioFileOpen failed");
        return false;
    }
	
    // �J�����t�@�C���̊�{���� fileDescription ��
    size = sizeof(AudioStreamBasicDescription);
	err = AudioFileGetProperty(mAudioFileID, kAudioFilePropertyDataFormat, 
							   &size, &fileDescription);
    if (err) {
        //NSLog(@"AudioFileGetProperty failed");
        AudioFileClose(mAudioFileID);
        return false;
    }
	
    // �J�����t�@�C���̃f�[�^���̃o�C�g���� dataSize ��
    size = sizeof(SInt64);
	err = AudioFileGetProperty(mAudioFileID, kAudioFilePropertyAudioDataByteCount, 
							   &size, &dataSize64);
    if (err) {
        //NSLog(@"AudioFileGetProperty failed");
        AudioFileClose(mAudioFileID);
        return false;
    }
	dataSize = static_cast<UInt32>(dataSize64);
	
	AudioFileTypeID	fileTypeID;
	size = sizeof( AudioFileTypeID );
	err = AudioFileGetProperty(mAudioFileID, kAudioFilePropertyFileFormat, &size, &fileTypeID);
	if (err) {
        //NSLog(@"AudioFileGetProperty failed");
        AudioFileClose(mAudioFileID);
        return false;
    }
	
	// Instrument����������
	mInstData.basekey	= 60;
	mInstData.lowkey	= 0;
	mInstData.highkey	= 127;
	mInstData.loop		= 0;
	
	//���[�v�|�C���g�̎擾
	Float64		st_point=0.0,end_point=0.0;
	if ( fileTypeID == kAudioFileAIFFType || fileTypeID == kAudioFileAIFCType ) {
		//INST�`�����N�̎擾
		AudioFileGetUserDataSize(mAudioFileID, 'INST', 0, &size);
		if ( size > 4 ) {
			UInt8	*instChunk = new UInt8[size];
			AudioFileGetUserData(mAudioFileID, 'INST', 0, &size, instChunk);
			
			//MIDI���̎擾
			mInstData.basekey = instChunk[0];
			mInstData.lowkey = instChunk[2];
			mInstData.highkey = instChunk[3];
			
			if ( instChunk[9] > 0 ) {	//���[�v�t���O���m�F
				//�}�[�J�[�̎擾
				UInt32	writable;
				err = AudioFileGetPropertyInfo(mAudioFileID, kAudioFilePropertyMarkerList,
											   &size, &writable);
				if (err) {
					//NSLog(@"AudioFileGetPropertyInfo failed");
					AudioFileClose(mAudioFileID);
					return NULL;
				}
				UInt8	*markersBuffer = new UInt8[size];
				AudioFileMarkerList	*markers = reinterpret_cast<AudioFileMarkerList*>(markersBuffer);
				
				err = AudioFileGetProperty(mAudioFileID, kAudioFilePropertyMarkerList, 
										   &size, markers);
				if (err) {
					//NSLog(@"AudioFileGetProperty failed");
					AudioFileClose(mAudioFileID);
					return NULL;
				}
				
				//���[�v�|�C���g�̐ݒ�
				for (unsigned int i=0; i<markers->mNumberMarkers; i++) {
					if (markers->mMarkers[i].mMarkerID == instChunk[11] ) {
						st_point = markers->mMarkers[i].mFramePosition;
					}
					else if (markers->mMarkers[i].mMarkerID == instChunk[13] ) {
						end_point = markers->mMarkers[i].mFramePosition;
					}
					CFRelease(markers->mMarkers[i].mName);
				}
				if ( st_point < end_point ) {
					mInstData.loop = 1;
				}
				delete [] markersBuffer;
			}
			delete [] instChunk;
		}
		
	}
	else if ( fileTypeID == kAudioFileWAVEType ) {
		//smpl�`�����N�̎擾
		AudioFileGetUserDataSize( mAudioFileID, 'smpl', 0, &size );
		if ( size >= sizeof(WAV_smpl) ) {
			UInt8	*smplChunk = new UInt8[size];
			AudioFileGetUserData( mAudioFileID, 'smpl', 0, &size, smplChunk );
			WAV_smpl	*smpl = (WAV_smpl *)smplChunk;
			
			smpl->loops = EndianU32_LtoN( smpl->loops );
			
			if ( smpl->loops > 0 ) {
				mInstData.loop = true;
				mInstData.basekey = EndianU32_LtoN( smpl->note );
				st_point = EndianU32_LtoN( smpl->start );
				end_point = EndianU32_LtoN( smpl->end ) + 1;	//SoundForge���ł͍ŏI�|�C���g���܂߂����
				//end_point = EndianU32_LtoN( smpl->end );	//Peak�ł͂Ȃ���AIFF�Ɠ���
			}
			else {
				mInstData.basekey = EndianU32_LtoN( smpl->note );
			}
			delete [] smplChunk;
		}
	}
	
	//�e�ʂ̐���
	SInt64	dataSamples = dataSize / fileDescription.mBytesPerFrame;
	if ( dataSamples > MAXIMUM_SAMPLES ) {
		dataSize = MAXIMUM_SAMPLES * fileDescription.mBytesPerFrame;
	}
	if ( st_point > MAXIMUM_SAMPLES ) {
		st_point = MAXIMUM_SAMPLES;
	}
	if ( end_point > MAXIMUM_SAMPLES ) {
		end_point = MAXIMUM_SAMPLES;
	}
	
    // �g�`�ꎞ�ǂݍ��ݗp���������m��
    char *fileBuffer;
	unsigned int	fileBufferSize;
	if (mInstData.loop) {
		fileBufferSize = dataSize+EXPAND_BUFFER*fileDescription.mBytesPerFrame;
	}
	else {
		fileBufferSize = dataSize;
	}
	fileBuffer = new char[fileBufferSize];
	memset(fileBuffer, 0, fileBufferSize);
	
	// �t�@�C������g�`�f�[�^�̓ǂݍ���
	err = AudioFileReadBytes(mAudioFileID, false, 0, &dataSize, fileBuffer);
    if (err) {
        //NSLog(@"AudioFileReadBytes failed");
        AudioFileClose(mAudioFileID);
        delete [] fileBuffer;
        return false;
    }
    AudioFileClose(mAudioFileID);
	
    //���[�v��W�J����
    Float64	adjustment = 1.0;
    outputFormat=fileDescription;
	if (mInstData.loop) {
		UInt32	plusalpha=0, framestocopy;
		while (plusalpha < EXPAND_BUFFER) {
			framestocopy = 
			(end_point-st_point)>(EXPAND_BUFFER-plusalpha)?(EXPAND_BUFFER-plusalpha):end_point-st_point;
			memcpy(fileBuffer+((int)end_point+plusalpha)*fileDescription.mBytesPerFrame,
				   fileBuffer+(int)st_point*fileDescription.mBytesPerFrame,
				   framestocopy*fileDescription.mBytesPerFrame);
			plusalpha += framestocopy;
		}
		dataSize += plusalpha*fileDescription.mBytesPerFrame;
		
		//16�T���v�����E��FIX����
		adjustment = ( (long long)((end_point-st_point)/16) ) / ((end_point-st_point)/16.0);
		st_point *= adjustment;
		end_point *= adjustment;
	}
	outputFormat.mFormatID = kAudioFormatLinearPCM;
    outputFormat.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagsNativeEndian;
	outputFormat.mChannelsPerFrame = 1;
	outputFormat.mBytesPerFrame = sizeof(float);
	outputFormat.mBitsPerChannel = 32;
	outputFormat.mBytesPerPacket = outputFormat.mBytesPerFrame;
	
    // �o�C�g�I�[�_�[�ϊ��p�̃R���o�[�^��p��
    AudioConverterRef converter;
	err = AudioConverterNew(&fileDescription, &outputFormat, &converter);
    if (err) {
        //NSLog(@"AudioConverterNew failed");
        delete [] fileBuffer;
        return false;
    }
	
	//�T���v�����O���[�g�ϊ��̎����ō��ɐݒ�
//	if (fileDescription.mSampleRate != outputFormat.mSampleRate) {
//		size = sizeof(UInt32);
//		UInt32	setProp = kAudioConverterQuality_Max;
//		AudioConverterSetProperty(converter, kAudioConverterSampleRateConverterQuality,
//								  size, &setProp);
//        
//        size = sizeof(UInt32);
//		setProp = kAudioConverterSampleRateConverterComplexity_Mastering;
//		AudioConverterSetProperty(converter, kAudioConverterSampleRateConverterComplexity,
//								  size, &setProp);
//        
//	}
	
    //�o�͂ɕK�v�\���ȃo�b�t�@�T�C�Y�𓾂�
	UInt32	outputSize = dataSize;
	size = sizeof(UInt32);
	err = AudioConverterGetProperty(converter, kAudioConverterPropertyCalculateOutputBufferSize, 
									&size, &outputSize);
	if (err) {
		//NSLog(@"AudioConverterGetProperty failed");
		delete [] fileBuffer;
		AudioConverterDispose(converter);
        return false;
	}
    UInt32 monoSamples = outputSize/sizeof(float);
    
    // �o�C�g�I�[�_�[�ϊ�
    float *monoData = new float[monoSamples];
	AudioConverterConvertBuffer(converter, dataSize, fileBuffer,
								&outputSize, monoData);
    if(outputSize == 0) {
        //NSLog(@"AudioConverterConvertBuffer failed");
        delete [] fileBuffer;
        AudioConverterDispose(converter);
        return false;
    }
    
    //���[�v����16�̔{���łȂ��ꍇ�̓T���v�����O���[�g�ϊ�
    Float64 inputSampleRate = fileDescription.mSampleRate;
    Float64 outputSampleRate = fileDescription.mSampleRate * adjustment;
    int	outSamples = monoSamples;
    if ( outputSampleRate == inputSampleRate ) {
        m_pAudioData = new short[monoSamples];
        for (int i=0; i<monoSamples; i++) {
            m_pAudioData[i] = static_cast<short>(monoData[i] * 32768);
        }
    }
    else {
        outSamples = static_cast<int>(monoSamples / (inputSampleRate / outputSampleRate));
        m_pAudioData = new short[outSamples];
        resampling(monoData, monoSamples, inputSampleRate,
                   m_pAudioData, &outSamples, outputSampleRate);
    }
    
    // ��n��
    delete [] monoData;
    delete [] fileBuffer;
    AudioConverterDispose(converter);
	
	//Inst�f�[�^�̐ݒ�
	if ( st_point > MAXIMUM_SAMPLES ) {
		mInstData.lp = MAXIMUM_SAMPLES;
	}
	else {
		mInstData.lp			= st_point;
	}
	if ( end_point > MAXIMUM_SAMPLES ) {
		mInstData.lp_end = MAXIMUM_SAMPLES;
	}
	else {
		mInstData.lp_end		= end_point;
	}
	mInstData.srcSamplerate	= outputSampleRate;
    mLoadedSamples			= outSamples;
	
	mIsLoaded = true;
	
	return true;
#else
	//Windows�̃I�[�f�B�I�t�@�C���ǂݍ��ݏ���

	// �t�@�C�����J��
	HMMIO	hmio = NULL;
	MMRESULT	err;
	DWORD		size;

	hmio = mmioOpen( mPath, NULL, MMIO_READ );
	if ( !hmio ) {
		return false;
	}
	
	// RIFF�`�����N��T��
	MMCKINFO	riffChunkInfo;
	riffChunkInfo.fccType = mmioFOURCC('W', 'A', 'V', 'E');
	err = mmioDescend( hmio, &riffChunkInfo, NULL, MMIO_FINDRIFF );
	if ( err != MMSYSERR_NOERROR ) {
		mmioClose( hmio, 0 );
		return false;
	}
	if ( (riffChunkInfo.ckid != FOURCC_RIFF) || (riffChunkInfo.fccType != mmioFOURCC('W', 'A', 'V', 'E') ) ) {
		mmioClose( hmio, 0 );
		return false;
	}

	// �t�H�[�}�b�g�`�����N��T��
	MMCKINFO	formatChunkInfo;
	formatChunkInfo.ckid = mmioFOURCC('f', 'm', 't', ' ');
	err = mmioDescend( hmio, &formatChunkInfo, &riffChunkInfo, MMIO_FINDCHUNK );
	if ( err != MMSYSERR_NOERROR ) {
		mmioClose( hmio, 0 );
		return false;
	}
	if ( formatChunkInfo.cksize < sizeof(PCMWAVEFORMAT) ) {
		mmioClose( hmio, 0 );
		return false;
	}

	//�t�H�[�}�b�g�����擾
	WAVEFORMATEX	pcmWaveFormat;
	DWORD			fmsize = (formatChunkInfo.cksize > sizeof(WAVEFORMATEX)) ? sizeof(WAVEFORMATEX):formatChunkInfo.cksize;
	size = mmioRead( hmio, (HPSTR)&pcmWaveFormat, fmsize );
	if ( size != fmsize ) {
		mmioClose( hmio, 0 );
		return false;
	}
	if ( pcmWaveFormat.wFormatTag != WAVE_FORMAT_PCM ) {
		mmioClose( hmio, 0 );
		return false;
	}
	mmioAscend(hmio, &formatChunkInfo, 0);

	// Instrument����������
	mInstData.basekey	= 60;
	mInstData.lowkey	= 0;
	mInstData.highkey	= 127;
	mInstData.loop		= 0;

	//smpl�`�����N��T��
	MMCKINFO	smplChunkInfo;
	smplChunkInfo.ckid = mmioFOURCC('s', 'm', 'p', 'l');
	err = mmioDescend( hmio, &smplChunkInfo, &riffChunkInfo, MMIO_FINDCHUNK );
	if ( err != MMSYSERR_NOERROR ) {
		smplChunkInfo.cksize = 0;
	}
	double	st_point=0.0;
	double	end_point=0.0;
	if ( smplChunkInfo.cksize >= sizeof(WAV_smpl) ) {
		//���[�v�|�C���g�̎擾
		unsigned char	*smplChunk = new unsigned char[smplChunkInfo.cksize];
		size = mmioRead(hmio,(HPSTR)smplChunk, smplChunkInfo.cksize);
		WAV_smpl	*smpl = (WAV_smpl *)smplChunk;

		if ( smpl->loops > 0 ) {
			mInstData.loop = 1;
			mInstData.basekey = smpl->note;
			st_point = smpl->start;
			end_point = smpl->end + 1;	//SoundForge���ł͍ŏI�|�C���g���܂߂����
		}
		else {
			mInstData.basekey = smpl->note;
		}
		delete [] smplChunk;
	}
	mmioAscend(hmio, &formatChunkInfo, 0);

	//data�`�����N��T��
	MMCKINFO dataChunkInfo;
	dataChunkInfo.ckid = mmioFOURCC('d', 'a', 't', 'a');
	err = mmioDescend( hmio, &dataChunkInfo, &riffChunkInfo, MMIO_FINDCHUNK );
	if( err != MMSYSERR_NOERROR ) {
		mmioClose( hmio, 0 );
		return false;
	}

	// �g�`�ꎞ�ǂݍ��ݗp���������m��
	unsigned int	dataSize = dataChunkInfo.cksize;
	int				bytesPerSample = pcmWaveFormat.nBlockAlign;
	char			*fileBuffer;
	unsigned int	fileBufferSize;

	//�e�ʐ���
	int	dataSamples = dataSize / pcmWaveFormat.nBlockAlign;
	if ( dataSamples > MAXIMUM_SAMPLES ) {
		dataSize = MAXIMUM_SAMPLES * pcmWaveFormat.nBlockAlign;
	}
	if ( st_point > MAXIMUM_SAMPLES ) {
		st_point = MAXIMUM_SAMPLES;
	}
	if ( end_point > MAXIMUM_SAMPLES ) {
		end_point = MAXIMUM_SAMPLES;
	}
	
	
	if (mInstData.loop) {
		fileBufferSize = dataSize+EXPAND_BUFFER*bytesPerSample;
	}
	else {
		fileBufferSize = dataSize;
	}
	fileBuffer = new char[fileBufferSize];
	memset(fileBuffer, 0, fileBufferSize);
	
	// �t�@�C������g�`�f�[�^�̓ǂݍ���
	size = mmioRead(hmio, (HPSTR)fileBuffer, dataSize);
	if ( size != dataSize ) {
		mmioClose( hmio, 0 );
		return false;
	}
	mmioClose(hmio,0);

	//���[�v��W�J����
	double	inputSampleRate = pcmWaveFormat.nSamplesPerSec;
	double	outputSampleRate = inputSampleRate;
	if (mInstData.loop) {
		unsigned int	plusalpha=0;
		double			framestocopy;
		while (plusalpha < EXPAND_BUFFER) {
			framestocopy = 
			(end_point-st_point)>(EXPAND_BUFFER-plusalpha)?(EXPAND_BUFFER-plusalpha):end_point-st_point;
			memcpy(fileBuffer+((int)end_point+plusalpha)*bytesPerSample,
				   fileBuffer+(int)st_point*bytesPerSample,
				   static_cast<size_t>(framestocopy*bytesPerSample));
			plusalpha += static_cast<unsigned int>(framestocopy);
		}
		dataSize += plusalpha*bytesPerSample;
		
		//16�T���v�����E��FIX����
		double	adjustment = ( (long long)((end_point-st_point)/16) ) / ((end_point-st_point)/16.0);
		outputSampleRate *= adjustment;
		st_point *= adjustment;
		end_point *= adjustment;
	}

	//��Ufloat���m�����f�[�^�ɕϊ�
	int	bytesPerChannel = bytesPerSample / pcmWaveFormat.nChannels;
	unsigned int	inputPtr = 0;
	unsigned int	outputPtr = 0;
	int				monoSamples = dataSize / bytesPerSample;
	float	range = static_cast<float>((1<<(bytesPerChannel*8-1)) * pcmWaveFormat.nChannels);
	float	*monoData = new float[monoSamples];
	while (inputPtr < dataSize) {
		int	frameSum = 0;
		for (int ch=0; ch<pcmWaveFormat.nChannels; ch++) {
			for (int i=0; i<bytesPerChannel; i++) {
				if (i<bytesPerChannel-1) {
					frameSum += (unsigned char)fileBuffer[inputPtr] << (8*i);
				}
				else {
					frameSum += fileBuffer[inputPtr] << (8*i);
				}
				inputPtr++;
			}
		}
		monoData[outputPtr] = frameSum / range;
		outputPtr++;
	}

	//���[�v����16�̔{���łȂ��ꍇ�̓T���v�����O���[�g�ϊ�
	int	outSamples = monoSamples;
	if ( outputSampleRate == inputSampleRate ) {
		m_pAudioData = new short[monoSamples];
		for (int i=0; i<monoSamples; i++) {
			m_pAudioData[i] = static_cast<short>(monoData[i] * 32768);
		}
	}
	else {
		outSamples = static_cast<int>(monoSamples / (inputSampleRate / outputSampleRate));
		m_pAudioData = new short[outSamples];
		resampling(monoData, monoSamples, inputSampleRate,
				   m_pAudioData, &outSamples, outputSampleRate);
	}

	// ��n��
	delete [] fileBuffer;
	delete [] monoData;

	//Inst�f�[�^�̐ݒ�
	mInstData.lp			= static_cast<int>(st_point);
	mInstData.lp_end		= static_cast<int>(end_point);
	mInstData.srcSamplerate	= outputSampleRate;
    mLoadedSamples			= outSamples;

	mIsLoaded = true;

	return true;
#endif
}

//-----------------------------------------------------------------------------
int AudioFile::resampling(const float *src, int srcSamples, double srcRate,
						  short *dst, int *dstSamples, double dstRate)
{
	static const int	window_len = 256;
	static const float	stopband = 0.9999f;
	int					half_window_len = window_len / 2;
	float				srcStride = srcRate / dstRate;
	float				cutoffRate = srcStride > stopband?stopband:srcStride;
	int					dstSize = *dstSamples;
	int					actualDstSize = static_cast<int>(srcSamples / srcStride);
	
	if ( actualDstSize < dstSize ) {
		dstSize = actualDstSize;
	}
	for (int i=0; i<dstSize; i++) {
		float	src_pos = i*srcStride;
		float	dstSum = .0f;
		for (int j=-half_window_len; j<half_window_len; j++) {
			int	src_index = static_cast<int>(floorf(src_pos+0.5f)) + j;
			if (src_index >= 0 && src_index < srcSamples) {
				float	x = src_index - src_pos;
				float	window_x = x/half_window_len + 1.0f;
				if (window_x < 0.0f)	window_x = 0.0f;
				if (window_x > 2.0f)	window_x = 2.0f;
				float	window = 0.5f - 0.5f*cosf(mPi*(window_x));
				float	value = src[src_index] * sinc(x*cutoffRate) * window;
				dstSum += value * stopband;
			}
		}
		if (dstSum > 1.0f) {
			dstSum = 1.0f;
		}
		if (dstSum < -1.0f) {
			dstSum = -1.0f;
		}
		dst[i] = static_cast<short>(dstSum * 32767.0f);
	}
	*dstSamples = dstSize;
	return dstSize;
}

//-----------------------------------------------------------------------------
float AudioFile::sinc(float p_x1)
{
	if ( p_x1==.0f ) return 1.0f;
	else {
		float x1 = p_x1*mPi;
		return (sinf(x1)/x1);
	}
}

//-----------------------------------------------------------------------------
short *AudioFile::GetAudioData()
{
	return m_pAudioData;
}

//-----------------------------------------------------------------------------
int AudioFile::GetLoadedSamples()
{
	return mLoadedSamples;
}

//-----------------------------------------------------------------------------
bool AudioFile::GetInstData( InstData *instData )
{
	if ( IsLoaded() ) {
		*instData = mInstData;
		return true;
	}
	return false;
}
