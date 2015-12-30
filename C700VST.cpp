/*
 *  C700VST.cpp
 *  C700
 *
 *  Created by osoumen on 12/10/13.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "C700VST.h"
#include <math.h>


AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
	return new C700VST(audioMaster);
}

//-----------------------------------------------------------------------------------------
C700VST::C700VST(audioMasterCallback audioMaster)
: AudioEffectX(audioMaster, 128, kNumberOfParameters)
{
	mEfx = new C700Kernel();
	mEfx->SetPropertyNotifyFunc(PropertyNotifyFunc, this);
	mEfx->SetParameterSetFunc(ParameterSetFunc, this);
	
	if (audioMaster)
	{
		setNumInputs(0);				// no inputs
		setNumOutputs(NUM_OUTPUTS);	//
		canProcessReplacing();
		isSynth();
		programsAreChunks(true);
		setUniqueID(CCONST ('C', '7', '0', '0'));
	}
	
	suspend();
	
	mEditor = new C700Edit(this);
	editor = mEditor;
	
	efxAcc = new EfxAccess(this);
	mEditor->SetEfxAccess(efxAcc);
	
	//初期値の設定
	for ( int i=0; i<kNumberOfParameters; i++ ) {
		setParameter(i, shrinkParam(i, C700Kernel::GetParameterDefault(i)) );
	}
	
	//setProgram(0);
	mEfx->SelectPreset(1);
	
	mSaveChunk = NULL;
}

//-----------------------------------------------------------------------------------------
C700VST::~C700VST()
{
	if ( mSaveChunk ) {
		delete mSaveChunk;
	}
	delete efxAcc;
	delete mEfx;
}

//-----------------------------------------------------------------------------
float C700VST::expandParam( int index, float value )
{
	return ConvertFromVSTValue(value, C700Kernel::GetParameterMin(index), C700Kernel::GetParameterMax(index));
}

//-----------------------------------------------------------------------------
float C700VST::shrinkParam( int index, float value )
{
	return ConvertToVSTValue(value, C700Kernel::GetParameterMin(index), C700Kernel::GetParameterMax(index));
}

//-----------------------------------------------------------------------------
void C700VST::PropertyNotifyFunc(int propID, void* userData)
{
	C700VST	*This = reinterpret_cast<C700VST*> (userData);
	if ( propID == kAudioUnitCustomProperty_ProgramName ) {
		const char *pgname;
		pgname = This->mEfx->GetProgramName();
		if ( pgname ) {
			This->mEditor->SetProgramName(pgname);
		}
	}
	else if ( propID == kAudioUnitCustomProperty_BRRData ) {
		const	BRRData *brr;
		brr = This->mEfx->GetBRRData();
		if ( brr ) {
			This->mEditor->SetBRRData(brr);
		}
	}
	else {
		float	value = This->mEfx->GetPropertyValue(propID);
		This->mEditor->setParameter(propID, value);
		if ( propID == kAudioUnitCustomProperty_EditingProgram ) { 
			This->curProgram = value;
		}
	}
}

//-----------------------------------------------------------------------------
void C700VST::ParameterSetFunc(int paramID, float value, void* userData)
{
	C700VST	*This = reinterpret_cast<C700VST*> (userData);
	This->setParameter(paramID, This->shrinkParam(paramID, value));
}

//-----------------------------------------------------------------------------------------
void C700VST::open()
{
	AudioEffectX::open();
}

//-----------------------------------------------------------------------------------------
void C700VST::close()
{
	AudioEffectX::close();
}

//-----------------------------------------------------------------------------------------
void C700VST::suspend()
{
	AudioEffectX::suspend();
}

//-----------------------------------------------------------------------------------------
void C700VST::resume()
{
	mEfx->Reset();
	AudioEffectX::resume();
}

//------------------------------------------------------------------------

VstIntPtr C700VST::vendorSpecific(VstInt32 lArg1, VstIntPtr lArg2, void* ptrArg, float floatArg)
{
	//MouseWhell Enable 
	if (editor && lArg1 == CCONST('s','t','C','A') && lArg2 == CCONST('W','h','e','e'))
	{
		return editor->onWheel (floatArg) == true ? 1 : 0;
	}
	else {
		return AudioEffectX::vendorSpecific (lArg1, lArg2, ptrArg, floatArg);
	}
}

//-----------------------------------------------------------------------------------------
void C700VST::setProgram(VstInt32 program)
{
	AudioEffectX::setProgram(program);
#ifdef DEBUG
	DebugPrint("C700VST::setProgram %d",program);
#endif
	mEfx->SetPropertyValue(kAudioUnitCustomProperty_EditingProgram, program);
	mEditor->setParameter(kAudioUnitCustomProperty_EditingProgram, program);
	mEditor->SetProgramName(mEfx->GetProgramName());
}

//-----------------------------------------------------------------------------------------
void C700VST::setProgramName(char *name)
{
	mEfx->SetProgramName(name);
}

//-----------------------------------------------------------------------------------------
void C700VST::getProgramName(char *name)
{
	strncpy(name, mEfx->GetProgramName(), kVstMaxProgNameLen-1);
}

//-----------------------------------------------------------------------------------------
void C700VST::getParameterLabel(VstInt32 index, char *label)
{
#if DEBUG
	DebugPrint("exec C700VST::getParamaeterLabel");
#endif
	strncpy(label, mEfx->GetParameterName(index), kVstMaxParamStrLen-1);
}

//-----------------------------------------------------------------------------------------
void C700VST::getParameterDisplay(VstInt32 index, char *text)
{
#if DEBUG
	DebugPrint("exec C700VST::getParamaeterDisplay");
#endif
	snprintf(text, kVstMaxParamStrLen-1, "%f", mEfx->GetParameter(index) );
}

//-----------------------------------------------------------------------------------------
void C700VST::getParameterName(VstInt32 index, char *label)
{
#if DEBUG
	DebugPrint("exec C700VST::getParamaeterName TblLabel[%d]=%s",index,label);
#endif
	strncpy(label, mEfx->GetParameterName(index), kVstMaxParamStrLen-1);
}

//-----------------------------------------------------------------------------------------
void C700VST::setParameter(VstInt32 index, float value)
{
#if DEBUG
	DebugPrint("exec C700VST::setParamaeter index=%d value=%f",index,value);
#endif
	float	realValue = expandParam(index, value);
	mEfx->SetParameter(index, realValue);
	switch ( index ) {
		case kParam_echodelay:
			PropertyNotifyFunc(kAudioUnitCustomProperty_TotalRAM, this);
			break;
        case kParam_alwaysDelayNote:
            setInitialDelay(mEfx->GetProcessDelayTime() * sampleRate);
            break;
	}
	if (editor) {
		((AEffGUIEditor*)editor)->setParameter(index, realValue);
	}
}

//-----------------------------------------------------------------------------------------
float C700VST::getParameter(VstInt32 index)
{
#if DEBUG
	DebugPrint("exec C700VST::getParamaeter");
#endif
	return shrinkParam(index, mEfx->GetParameter(index));
}

//-----------------------------------------------------------------------------------------
bool C700VST::getOutputProperties(VstInt32 index, VstPinProperties* properties)
{
#if DEBUG
	DebugPrint("exec C700VST::getOutputProperties %d ",index);
#endif
	if (index < NUM_OUTPUTS)
	{
		sprintf (properties->label, "C700 %1d", index + 1);
		properties->flags = kVstPinIsActive;
		if (index < 2)
			properties->flags |= kVstPinIsStereo;	// test, make channel 1+2 stereo
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------------------
bool C700VST::copyProgram(long destination)
{
	return false;
}

//-----------------------------------------------------------------------------------------
bool C700VST::getEffectName(char* name)
{
	strcpy (name, "C700VST");
	return true;
}

//-----------------------------------------------------------------------------------------
bool C700VST::getVendorString(char* text)
{
	strcpy (text, "osoumen");
	return true;
}

//-----------------------------------------------------------------------------------------
bool C700VST::getProductString(char* text)
{
	strcpy (text, "C700");
	return true;
}

//-----------------------------------------------------------------------------------------

VstInt32 C700VST::canDo(char* text)
{
#if DEBUG
	DebugPrint("exec C700::canDo [%s] ",text);
#endif	
	if (!strcmp (text, "receiveVstEvents")) {
		return 1;
	}
	if (!strcmp (text, "receiveVstMidiEvent")) {
		return 1;
	}
	//if(!strcmp(text, "midiProgramNames"))
	//		return 1;
	return -1;	// explicitly can't do; 0 => don't know
	
}

//-----------------------------------------------------------------------------------------
VstInt32 C700VST::getChunk(void** data, bool isPreset)
{
	int	editProg = mEfx->GetPropertyValue(kAudioUnitCustomProperty_EditingProgram);
	int	editChan = mEfx->GetPropertyValue(kAudioUnitCustomProperty_EditingChannel);

	if ( mSaveChunk ) {
		delete mSaveChunk;
	}
	
	const InstParams	*vp = mEfx->GetVP();
	int		totalSize = 0;
	int		totalProgs = 0;
	
	if ( isPreset ) {
		totalSize = PGChunk::getPGChunkSize( &vp[editProg] ) + sizeof(PGChunk::MyChunkHead);
		totalProgs = 1;
	}
	else {
		for ( int i=0; i<128; i++ ) {
			int size = PGChunk::getPGChunkSize( &vp[i] );
			if ( size > 0 ) {
				totalSize += size + sizeof(PGChunk::MyChunkHead);
				totalProgs++;
			}
		}
		totalSize += (sizeof(PGChunk::MyChunkHead) + sizeof(int))*3;	//プログラム定義数
		//パラメータ設定値のサイズの追加
		totalSize += (sizeof(PGChunk::MyChunkHead) + sizeof(float)) * kNumberOfParameters;
	}
	
	
#if TESTING
	printf("getChunk totalSize=%d\n",totalSize);
	printf("getChunk totalProgs=%d\n",totalProgs);
#endif
	
	PGChunk		*saveChunk;
	saveChunk = new PGChunk( totalSize );
	if ( !isPreset ) {
		//パラメータの書き込み
		for ( int i=0; i<kNumberOfParameters; i++ ) {
			float	param;
			param = getParameter(i);
			saveChunk->writeChunk(i, &param, sizeof(float));
		}
		saveChunk->writeChunk(CKID_PROGRAM_TOTAL, &totalProgs, sizeof(int));
		saveChunk->writeChunk(kAudioUnitCustomProperty_EditingProgram, &editProg, sizeof(int));
		saveChunk->writeChunk(kAudioUnitCustomProperty_EditingChannel, &editChan, sizeof(int));
	}
	
	if ( isPreset ) {
		if ( vp[editProg].hasBrrData() ) {
			PGChunk		*pg = new PGChunk( PGChunk::getPGChunkSize( &vp[editProg] ) );
            //最終ブロックをループフラグにする
            /*
            if (vp[editProg].isLoop()) {
                vp[editProg].setLoop();
            }
            else {
                vp[editProg].unsetLoop();
            }*/
			pg->AppendDataFromVP(&vp[editProg]);
			saveChunk->writeChunk(CKID_PROGRAM_DATA+editProg, pg->GetDataPtr(), pg->GetDataSize());
			delete pg;
		}
	}
	else {
		for ( int i=0; i<128; i++ ) {
			if ( vp[i].hasBrrData() ) {
				PGChunk		*pg = new PGChunk( PGChunk::getPGChunkSize( &vp[i] ) );
				pg->AppendDataFromVP(&vp[i]);
				saveChunk->writeChunk(CKID_PROGRAM_DATA+i, pg->GetDataPtr(), pg->GetDataSize());
				delete pg;
			}
		}
	}
	
#if TESTING
	printf("getChunk saveChunk->GetDataSize()=%d\n",saveChunk->GetDataSize());
#endif
	mSaveChunk = saveChunk;
	*data = (void*)saveChunk->GetDataPtr();
	
	return saveChunk->GetDataSize();
}

//-----------------------------------------------------------------------------------------
VstInt32 C700VST::setChunk(void* data, VstInt32 byteSize, bool isPreset)
{
#if TESTING
	printf("setChunk byteSize=%d\n",byteSize);
#endif
	int	editProg = mEfx->GetPropertyValue(kAudioUnitCustomProperty_EditingProgram);
	int	editChan = mEfx->GetPropertyValue(kAudioUnitCustomProperty_EditingChannel);
	//const InstParams	*vp = mEfx->GetVP();
	
	PGChunk		*saveChunk;
	saveChunk = new PGChunk( data, byteSize );
	int			totalProgs;
	
	while ( byteSize - saveChunk->GetDataPos() > (int)sizeof( PGChunk::MyChunkHead ) ) {
		int		ckType;
		long	ckSize;
		saveChunk->readChunkHead(&ckType, &ckSize);
		
		//保存されているプログラム数
		
		if ( ckType < kNumberOfParameters && isPreset == false ) {
			//パラメータ読み込み
			float	param;
			saveChunk->readData( &param, sizeof(float), &ckSize );
			setParameter(ckType, param);
		}
		else if ( ckType == CKID_PROGRAM_TOTAL ) {
			saveChunk->readData( &totalProgs, sizeof(int), &ckSize );
		}
		else if ( ckType == kAudioUnitCustomProperty_EditingProgram && isPreset == false ) {
			saveChunk->readData( &editProg, sizeof(int), &ckSize );
		}
		else if ( ckType == kAudioUnitCustomProperty_EditingChannel && isPreset == false ) {
			saveChunk->readData( &editChan, sizeof(int), &ckSize );
		}
		else if ( ckType >= CKID_PROGRAM_DATA && ckType < (CKID_PROGRAM_DATA+128) ) {
			//CKID_PROGRAM_DATA+pgnumのチャンクに入れ子でプログラムデータが入っている
			int pgnum = ckType - CKID_PROGRAM_DATA;
			PGChunk	*pg = new PGChunk( saveChunk->GetDataPtr()+saveChunk->GetDataPos(), ckSize );
            InstParams inst;
			if ( isPreset ) {
				pg->ReadDataToVP(&inst);
                mEfx->SetVP(editProg, &inst);
			}
			else {
				pg->ReadDataToVP(&inst);
                mEfx->SetVP(pgnum, &inst);
			}
			delete pg;
			saveChunk->AdvDataPos(ckSize);
		}
		else {
			saveChunk->AdvDataPos(ckSize);
		}
	}
#if TESTING
	printf("setChunk saveChunk->GetDataPos()=%d\n",saveChunk->GetDataPos());
#endif	
	//UIに変更を反映
	mEfx->SetPropertyValue(kAudioUnitCustomProperty_EditingProgram, editProg );
	mEfx->SetPropertyValue(kAudioUnitCustomProperty_EditingChannel, editChan );
	mEfx->GetGenerator()->RefreshKeyMap();
	
	delete saveChunk;
	
	return 0;
}

//-----------------------------------------------------------------------------------------
void C700VST::setSampleRate(float sampleRate)
{
	AudioEffectX::setSampleRate(sampleRate);
	mEfx->SetSampleRate(sampleRate);
    setInitialDelay(mEfx->GetProcessDelayTime() * sampleRate);
}

//-----------------------------------------------------------------------------------------
void C700VST::setBlockSize(VstInt32 blockSize)
{
	AudioEffectX::setBlockSize(blockSize);
	// you may need to have to do something here...
}

//-----------------------------------------------------------------------------------------
void C700VST::processReplacing(float **inputs, float **outputs, int sampleFrames)
{
    setInitialDelay(mEfx->GetProcessDelayTime() * sampleRate);
	memset(outputs[0], 0, sampleFrames*sizeof(float));
	memset(outputs[1], 0, sampleFrames*sizeof(float));
	mEfx->Render(sampleFrames, outputs);
}

//-----------------------------------------------------------------------------------------
VstInt32 C700VST::processEvents(VstEvents* ev)
{
	for (long i = 0; i < ev->numEvents; i++)
	{
		if ((ev->events[i])->type != kVstMidiType) continue;
		
		VstMidiEvent* event		= (VstMidiEvent*)ev->events[i];
		char* midiData			= event->midiData;
		int	inFrames			= event->deltaFrames;
		unsigned char status	= midiData[0] & 0xf0;		// ignoring channel
		char MidiCh				= midiData[0] & 0x0f;
		char note				= midiData[1] & 0x7f;
		char velocity			= midiData[2] & 0x7f;
		/*
		 #if DEBUG
		 DebugPrint("VOPMproc::ProcessEvents Message[%02x %02x %02x]",
		 midiData[0],midiData[1],midiData[2]);
		 #endif
		 */
		switch (status) {
			case 0x80:
				mEfx->HandleNoteOff(MidiCh, note, note+MidiCh*256, inFrames);
				break;
			case 0x90:
				if (velocity==0) {
					mEfx->HandleNoteOff(MidiCh, note, note+MidiCh*256, inFrames);
				}
				else {
					mEfx->HandleNoteOn(MidiCh, note, velocity, note+MidiCh*256, inFrames);
				}
				break;
			case 0xb0:
				switch (note) {
					case 120://Force off
						mEfx->HandleAllSoundOff(MidiCh, inFrames);
						break;
                    case 121:
                        mEfx->HandleResetAllControllers(MidiCh, inFrames);
                        break;
					case 123://All Note Off
						mEfx->HandleAllNotesOff(MidiCh, inFrames);
						break;
					case 126://Mono
					case 127://Poly
					default:
                        mEfx->HandleControlChange(MidiCh, note, velocity, inFrames);
						break;
				}
				break;
			case 0xc0:
				mEfx->HandleProgramChange(MidiCh, note, inFrames);
				break;
			case 0xe0:
				mEfx->HandlePitchBend(MidiCh, note, velocity, inFrames);
				break;
		}
	}
	return 1;
}
