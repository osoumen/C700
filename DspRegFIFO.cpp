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
    pthread_mutex_init(&mListMtx, 0);
}

DspRegFIFO::~DspRegFIFO()
{
    pthread_mutex_destroy(&mListMtx);
}
/*
DspRegFIFO* DspRegFIFO::GetInstance()
{
    return sInstance;
}
*/
void DspRegFIFO::AddDspWrite(long time, unsigned char addr, unsigned char data)
{
    pthread_mutex_lock(&mListMtx);
    DspWrite dsp;
    dsp.time = time;
    dsp.isRam = false;
    dsp.addr = addr;
    dsp.data = data;
    mDspWrite.push_back(dsp);
    pthread_mutex_unlock(&mListMtx);
}

void DspRegFIFO::AddRamWrite(long time, unsigned short addr, unsigned char data)
{
    pthread_mutex_lock(&mListMtx);
    DspWrite ram;
    ram.time = time;
    ram.isRam = true;
    ram.addr = addr;
    ram.data = data;
    mDspWrite.push_back(ram);
    pthread_mutex_unlock(&mListMtx);
}

DspRegFIFO::DspWrite DspRegFIFO::PopFront()
{
    pthread_mutex_lock(&mListMtx);
    DspWrite front = mDspWrite.front();
    mDspWrite.pop_front();
    pthread_mutex_unlock(&mListMtx);
    return front;
}

long DspRegFIFO::GetFrontTime()
{
    long time = 0;
    pthread_mutex_lock(&mListMtx);
    if (mDspWrite.size() > 0) {
        time = mDspWrite.front().time;
    }
    pthread_mutex_unlock(&mListMtx);
    return time;
}

void DspRegFIFO::AddTime(long time)
{
    pthread_mutex_lock(&mListMtx);
    std::list<DspWrite>::iterator it = mDspWrite.begin();
    while (it != mDspWrite.end()) {
        it->time += time;
        it++;
    }
    pthread_mutex_unlock(&mListMtx);
}

void DspRegFIFO::Clear()
{
    pthread_mutex_lock(&mListMtx);
    mDspWrite.clear();
    pthread_mutex_unlock(&mListMtx);
}
