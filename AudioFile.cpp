/*
 *  AudioFile.cpp
 *  Chip700
 *
 *  Created by 藤田 匡彦 on 12/10/10.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "AudioFile.h"
#include "vstgui.h"

#if MAC
#include <AudioToolbox/AudioFile.h>
#include <AudioToolbox/AudioConverter.h>
#endif

//-----------------------------------------------------------------------------
AudioFile::AudioFile( const char *path, bool isWriteable )
: FileAccess(path, isWriteable)
, m_pAudioData( NULL )
, mLoadedSamples( 0 )
{
}

//-----------------------------------------------------------------------------
AudioFile::~AudioFile()
{
	if ( m_pAudioData ) {
		free(m_pAudioData);
	}
}

//-----------------------------------------------------------------------------
bool AudioFile::IsVarid()
{
	AudioFileID mAudioFileID;
	FSRef	ref;
	Boolean	isDirectory=false;
	FSPathMakeRef((const UInt8*)GetFilePath(), &ref, &isDirectory);
	OSStatus err = AudioFileOpen(&ref, fsRdPerm, 0, &mAudioFileID);
	if (err == noErr) {
		AudioFileClose(mAudioFileID);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool AudioFile::Load()
{
#if MAC
	AudioFileID mAudioFileID;
    AudioStreamBasicDescription mFileDescription, outputFormat;
    SInt64 dataSize64;
    UInt32 dataSize;
	
	OSStatus err;
	UInt32 size;
	
    // ファイルを開く
	FSRef	ref;
	Boolean	isDirectory=false;
	FSPathMakeRef((const UInt8*)GetFilePath(), &ref, &isDirectory);
	
	err = AudioFileOpen(&ref, fsRdPerm, 0, &mAudioFileID);
    if (err) {
        //NSLog(@"AudioFileOpen failed");
        return false;
    }
	
    // 開いたファイルの基本情報を mFileDescription へ
    size = sizeof(AudioStreamBasicDescription);
	err = AudioFileGetProperty(mAudioFileID, kAudioFilePropertyDataFormat, 
							   &size, &mFileDescription);
    if (err) {
        //NSLog(@"AudioFileGetProperty failed");
        AudioFileClose(mAudioFileID);
        return false;
    }
	
    // 開いたファイルのデータ部のバイト数を dataSize へ
    size = sizeof(SInt64);
	err = AudioFileGetProperty(mAudioFileID, kAudioFilePropertyAudioDataByteCount, 
							   &size, &dataSize64);
    if (err) {
        //NSLog(@"AudioFileGetProperty failed");
        AudioFileClose(mAudioFileID);
        return false;
    }
    dataSize=(UInt32)dataSize64;
	
	AudioFileTypeID	fileTypeID;
	size = sizeof( AudioFileTypeID );
	err = AudioFileGetProperty(mAudioFileID, kAudioFilePropertyFileFormat, &size, &fileTypeID);
	if (err) {
        //NSLog(@"AudioFileGetProperty failed");
        AudioFileClose(mAudioFileID);
        return false;
    }
	
	mInstData.basekey	= 60;
	mInstData.lowkey	= 0;
	mInstData.highkey	= 127;
	mInstData.loop		= 0;
	
	//ループポイントの取得
	Float64		st_point=0.0,end_point=0.0;
	if ( fileTypeID == kAudioFileAIFFType || fileTypeID == kAudioFileAIFCType ) {
		//INSTチャンクの取得
		AudioFileGetUserDataSize(mAudioFileID, 'INST', 0, &size);
		if ( size > 4 ) {
			UInt8	*instChunk = (UInt8*)malloc(size);
			AudioFileGetUserData(mAudioFileID, 'INST', 0, &size, instChunk);
			
			//MIDI情報の取得
			mInstData.basekey = instChunk[0];
			mInstData.lowkey = instChunk[2];
			mInstData.highkey = instChunk[3];
			
			if ( instChunk[9] > 0 ) {	//ループフラグを確認
				//マーカーの取得
				UInt32	writable;
				err = AudioFileGetPropertyInfo(mAudioFileID, kAudioFilePropertyMarkerList,
											   &size, &writable);
				if (err) {
					//NSLog(@"AudioFileGetPropertyInfo failed");
					AudioFileClose(mAudioFileID);
					return NULL;
				}
				AudioFileMarkerList	*markers = (AudioFileMarkerList*)malloc(size);
				err = AudioFileGetProperty(mAudioFileID, kAudioFilePropertyMarkerList, 
										   &size, markers);
				if (err) {
					//NSLog(@"AudioFileGetProperty failed");
					AudioFileClose(mAudioFileID);
					return NULL;
				}
				
				//ループポイントの設定
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
				free( markers );
			}
			free( instChunk );
		}
		
	}
	else if ( fileTypeID == kAudioFileWAVEType ) {
		//smplチャンクの取得
		AudioFileGetUserDataSize( mAudioFileID, 'smpl', 0, &size );
		if ( size >= sizeof(WAV_smpl) ) {
			UInt8	*smplChunk = (UInt8*)malloc(size);
			AudioFileGetUserData( mAudioFileID, 'smpl', 0, &size, smplChunk );
			WAV_smpl	*smpl = (WAV_smpl *)smplChunk;
			
			smpl->loops = EndianU32_LtoN( smpl->loops );
			
			if ( smpl->loops > 0 ) {
				mInstData.loop = 1;
				mInstData.basekey = EndianU32_LtoN( smpl->note );
				st_point = EndianU32_LtoN( smpl->start );
				end_point = EndianU32_LtoN( smpl->end ) + 1;	//SoundForge等では最終ポイントを含める解釈
				//end_point = EndianU32_LtoN( smpl->end );	//PeakではなぜかAIFFと同じ
			}
			else {
				mInstData.basekey = EndianU32_LtoN( smpl->note );
			}
			free( smplChunk );
		}
	}
	
    // 波形一時読み込み用メモリを確保
    char *mFileBuffer;
	if (mInstData.loop) {
		mFileBuffer = (char *)calloc(dataSize+EXPAND_BUFFER*mFileDescription.mBytesPerFrame,sizeof(char));
	}
	else {
		mFileBuffer = (char *)calloc(dataSize,sizeof(char));
	}
	
	// ファイルから読み込み
	err = AudioFileReadBytes(mAudioFileID, false, 0, &dataSize, mFileBuffer);
    if (err) {
        //NSLog(@"AudioFileReadBytes failed");
        AudioFileClose(mAudioFileID);
        free(mFileBuffer);
        return false;
    }
    AudioFileClose(mAudioFileID);
	
    //１６bitモノラルのデータに変換
    outputFormat=mFileDescription;
	if (mInstData.loop) {
		UInt32	plusalpha=0, framestocopy;
		while (plusalpha < EXPAND_BUFFER) {
			framestocopy = 
			(end_point-st_point)>(EXPAND_BUFFER-plusalpha)?(EXPAND_BUFFER-plusalpha):end_point-st_point;
			memcpy(mFileBuffer+((int)end_point+plusalpha)*mFileDescription.mBytesPerFrame,
				   mFileBuffer+(int)st_point*mFileDescription.mBytesPerFrame,
				   framestocopy*mFileDescription.mBytesPerFrame);
			plusalpha += framestocopy;
		}
		dataSize += plusalpha*mFileDescription.mBytesPerFrame;
		
		Float64	adjustment = ( (long long)((end_point-st_point)/16) ) / ((end_point-st_point)/16.0);
		outputFormat.mSampleRate *= adjustment;
		st_point *= adjustment;		//16サンプル境界にFIXする
		end_point *= adjustment;
		
	}
	outputFormat.mFormatID = kAudioFormatLinearPCM;
    outputFormat.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagsNativeEndian;
	outputFormat.mChannelsPerFrame = 1;
	outputFormat.mBytesPerFrame = sizeof(short);
	outputFormat.mBitsPerChannel = 16;
	outputFormat.mBytesPerPacket = outputFormat.mBytesPerFrame;
	
    // バイトオーダー変換用のコンバータを用意
    AudioConverterRef converter;
	err = AudioConverterNew(&mFileDescription, &outputFormat, &converter);
    if (err) {
        //NSLog(@"AudioConverterNew failed");
        free(mFileBuffer);
        return false;
    }
	
	//サンプリングレート変換の質を最高に設定
	if (mFileDescription.mSampleRate != outputFormat.mSampleRate) {
		size = sizeof(UInt32);
		UInt32	setProp = kAudioConverterQuality_Max;
		AudioConverterSetProperty(converter, kAudioConverterSampleRateConverterQuality, 
								  size, &setProp);
	}
	
    //出力に必要十分なバッファサイズを得る
    //if(m_pAudioData) // 2度目以降
    //    free(m_pAudioData);
	UInt32	outputSize = dataSize;
	size = sizeof(UInt32);
	err = AudioConverterGetProperty(converter, kAudioConverterPropertyCalculateOutputBufferSize, 
									&size, &outputSize);
	if (err) {
		//NSLog(@"AudioConverterGetProperty failed");
		free(mFileBuffer);
		AudioConverterDispose(converter);
	}
    m_pAudioData=(short *)malloc(outputSize);
    
    // バイトオーダー変換
	AudioConverterConvertBuffer(converter, dataSize, mFileBuffer,
								&outputSize, m_pAudioData);
    if(outputSize == 0) {
        //NSLog(@"AudioConverterConvertBuffer failed");
        free(mFileBuffer);
        AudioConverterDispose(converter);
        return false;
    }
    
    // 後始末
    free(mFileBuffer);
    AudioConverterDispose(converter);
	
	mInstData.lp			= st_point;
	mInstData.lp_end		= end_point;
	mInstData.srcSamplerate	= outputFormat.mSampleRate;
    mLoadedSamples			= outputSize/outputFormat.mBytesPerFrame;
	
	mIsLoaded = true;
	
	return true;
#else
	//TODO : Windowsのオーディオファイル読み込み処理
	return false;
#endif
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
