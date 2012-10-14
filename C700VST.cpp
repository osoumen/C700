/*
 *  C700VST.cpp
 *  Chip700
 *
 *  Created by ì°ìc ãßïF on 12/10/13.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "C700VST.h"

AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
	return new C700VST(audioMaster);
}

//-----------------------------------------------------------------------------------------
C700VST::C700VST(audioMasterCallback audioMaster)
: AudioEffectX(audioMaster, NUM_PRESETS, kNumberOfParameters)
{
	mEfx = new C700Kernel();
	mEfx->SetPropertyNotifyFunc(PropertyNotifyFunc, this);
	mEfx->SetParameterSetFunc(ParameterSetFunc, this);
	
	//setProgram(0);
	mCurrentPreset = 0;
	
	if (audioMaster)
	{
		setNumInputs(0);				// no inputs
		setNumOutputs(NUM_OUTPUTS);	//
		canProcessReplacing();
		isSynth();
		//programsAreChunks(true);
		setUniqueID(CCONST ('C', '7', '0', '0'));
	}
	
	//èâä˙ílÇÃê›íË
	for ( int i=0; i<kNumberOfParameters; i++ ) {
		setParameter(i, shrinkParam(i, C700Kernel::GetParameterDefault(i)) );
	}
		
	suspend();
	
//	pChunk= new unsigned char[32*1024];
	mEditor = new C700Edit(this);
	editor = mEditor;
	
	efxAcc = new EfxAccess(this);
	mEditor->SetEfxAccess(efxAcc);
	
//	if(!editor){
//		oome = true;
//	}
}

//-----------------------------------------------------------------------------------------
C700VST::~C700VST()
{
	delete efxAcc;
	delete mEfx;
}

//-----------------------------------------------------------------------------
float C700VST::expandParam( int index, float value )
{
	return (value * (C700Kernel::GetParameterMax(index) - C700Kernel::GetParameterMin(index)) + C700Kernel::GetParameterMin(index));
}

//-----------------------------------------------------------------------------
float C700VST::shrinkParam( int index, float value )
{
	return (value - C700Kernel::GetParameterMin(index)) / (C700Kernel::GetParameterMax(index) - C700Kernel::GetParameterMin(index));
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
#ifdef DEBUG
	DebugPrint("C700VST::setProgram %d",program);
#endif
	mEfx->SelectPreset(program);
	mCurrentPreset = program;
}

//-----------------------------------------------------------------------------------------
void C700VST::setProgramName(char *name)
{
}

//-----------------------------------------------------------------------------------------
void C700VST::getProgramName(char *name)
{
	strncpy(name, mEfx->GetPresetName(mCurrentPreset), kVstMaxProgNameLen-1);
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
	DebugPrint("exec VOPM::canDo [%s] ",text);
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
}

//-----------------------------------------------------------------------------------------
VstInt32 C700VST::setChunk(void* data, VstInt32 byteSize, bool isPreset)
{
}

//-----------------------------------------------------------------------------------------
void C700VST::setSampleRate(float sampleRate)
{
	AudioEffectX::setSampleRate(sampleRate);
	mEfx->SetSampleRate(sampleRate);
}

//-----------------------------------------------------------------------------------------
void C700VST::setBlockSize(long blockSize)
{
	AudioEffectX::setBlockSize(blockSize);
	// you may need to have to do something here...
}

//-----------------------------------------------------------------------------------------
void C700VST::processReplacing(float **inputs, float **outputs, int sampleFrames)
{
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
					case 0x01://Modulation Wheel
						mEfx->HandleControlChange(MidiCh, note, velocity, inFrames);
						break;
					case 0x78://Force off
						mEfx->HandleAllSoundOff(MidiCh, inFrames);
						break;
					case 0x7B://All Note Off
						mEfx->HandleAllNotesOff(MidiCh, inFrames);
						break;
					case 0x7E://Mono
					case 0x7F://Poly
					default:
						;
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
