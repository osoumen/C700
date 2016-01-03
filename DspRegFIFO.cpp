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
    MutexInit(mListMtx);
}

DspRegFIFO::~DspRegFIFO()
{
    MutexDestroy(mListMtx);
}
/*
DspRegFIFO* DspRegFIFO::GetInstance()
{
    return sInstance;
}
*/
void DspRegFIFO::AddDspWrite(long time, unsigned char addr, unsigned char data)
{
    MutexLock(mListMtx);
    DspWrite dsp;
    dsp.time = time;
    dsp.isRam = false;
    dsp.addr = addr;
    dsp.data = data;
    mDspWrite.push_back(dsp);
    MutexUnlock(mListMtx);
}

void DspRegFIFO::AddRamWrite(long time, unsigned short addr, unsigned char data)
{
    MutexLock(mListMtx);
    DspWrite ram;
    ram.time = time;
    ram.isRam = true;
    ram.addr = addr;
    ram.data = data;
    mDspWrite.push_back(ram);
    MutexUnlock(mListMtx);
}

DspRegFIFO::DspWrite DspRegFIFO::PopFront()
{
    MutexLock(mListMtx);
    DspWrite front = mDspWrite.front();
    mDspWrite.pop_front();
    MutexUnlock(mListMtx);
    return front;
}

long DspRegFIFO::GetFrontTime()
{
    long time = 0;
    MutexLock(mListMtx);
    if (mDspWrite.size() > 0) {
        time = mDspWrite.front().time;
    }
    MutexUnlock(mListMtx);
    return time;
}

void DspRegFIFO::AddTime(long time)
{
    MutexLock(mListMtx);
    std::list<DspWrite>::iterator it = mDspWrite.begin();
    while (it != mDspWrite.end()) {
        it->time += time;
        it++;
    }
    MutexUnlock(mListMtx);
}

void DspRegFIFO::Clear()
{
    MutexLock(mListMtx);
    mDspWrite.clear();
    MutexUnlock(mListMtx);
}
