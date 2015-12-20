//
//  DspRegFIFO.cpp
//  gimicUsbSpcPlay
//
//  Created by osoumen on 2014/10/26.
//  Copyright (c) 2014年 osoumen. All rights reserved.
//

#include "DspRegFIFO.h"

//static DspRegFIFO  *sInstance = NULL;

DspRegFIFO::DspRegFIFO()
{
    //sInstance = this;
}

DspRegFIFO::~DspRegFIFO()
{
    
}
/*
DspRegFIFO* DspRegFIFO::GetInstance()
{
    return sInstance;
}
*/
void DspRegFIFO::AddDspWrite(long time, unsigned char addr, unsigned char data)
{
    DspWrite dsp;
    dsp.time = time;
    dsp.isRam = false;
    dsp.addr = addr;
    dsp.data = data;
    mDspWrite.push_back(dsp);
}

void DspRegFIFO::AddRamWrite(long time, unsigned short addr, unsigned char data)
{
    DspWrite ram;
    ram.time = time;
    ram.isRam = true;
    ram.addr = addr;
    ram.data = data;
    mDspWrite.push_back(ram);
}

DspRegFIFO::DspWrite DspRegFIFO::PopFront()
{
    DspWrite front = mDspWrite.front();
    mDspWrite.pop_front();
    return front;
}

long DspRegFIFO::GetFrontTime()
{
    long time = 0;
    if (mDspWrite.size() > 0) {
        time = mDspWrite.front().time;
    }
    return time;
}

void DspRegFIFO::Clear()
{
    mDspWrite.clear();
}
