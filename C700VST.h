/*
 *  C700VST.h
 *  Chip700
 *
 *  Created by osoumen on 12/10/13.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "audioeffectx.h"
#include "C700Kernel.h"
#include "C700Edit.h"
#include "EfxAccess.h"

class C700VST : public AudioEffectX
{
	friend class EfxAccess;
public:
	C700VST(audioMasterCallback audioMaster);
	virtual ~C700VST();
	
	virtual void open();
	virtual void close();
	virtual void suspend();
	virtual void resume();
	
	virtual void processReplacing(float **inputs, float **outputs, int sampleframes);
	virtual VstInt32 processEvents(VstEvents* events);
	
	virtual void setProgram(VstInt32 program);
	virtual void setProgramName(char *name);
	virtual void getProgramName(char *name);
	virtual void setParameter(VstInt32 index, float value);
	virtual float getParameter(VstInt32 index);
	virtual void getParameterLabel(VstInt32 index, char *label);
	virtual void getParameterDisplay(VstInt32 index, char *text);
	virtual void getParameterName(VstInt32 index, char *text);
	virtual void setSampleRate(float sampleRate);
	virtual void setBlockSize(long blockSize);
	
	virtual bool getOutputProperties (VstInt32 index, VstPinProperties* properties);
	//	virtual bool getProgramNameIndexed (long category, VstInt32 index, char* text);
	virtual bool copyProgram (long destination);
	virtual bool getEffectName (char* name);
	virtual bool getVendorString (char* text);
	virtual bool getProductString (char* text);
	//	long getVendorVersion () {return 1;}
	virtual VstIntPtr vendorSpecific (VstInt32 lArg, VstIntPtr lArg2, void* ptrArg, float floatArg);
	
	
	virtual VstInt32 canDo(char* text);
	virtual VstInt32 getChunk(void** data, bool isPreset = false);	// returns byteSize
	virtual VstInt32 setChunk(void* data, VstInt32 byteSize, bool isPreset = false);
	
	static void		PropertyNotifyFunc(int propID, void* userData);
	static void		ParameterSetFunc(int paramID, float value, void* userData);
private:
	float		expandParam( int index, float value );
	float		shrinkParam( int index, float value );
	
	C700Edit			*mEditor;
	EfxAccess			*efxAcc;
	double				mTempo;
	
	C700Kernel			*mEfx;
	int					mCurrentPreset;
};