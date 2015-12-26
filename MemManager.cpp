//
//  MemManager.cpp
//  C700
//
//  Created by osoumen on 2015/11/07.
//
//

#include "MemManager.h"

BrrRegion::BrrRegion(const unsigned char *data, int size, int loopPoint)
: mAddr(-1)
, mData(NULL)
{
    if (data != NULL) {
        mData = new unsigned char[size];
        memcpy(mData, data, size);
        mSize = size;
        mLoopPoint = loopPoint;
    }
}
/*
BrrRegion::BrrRegion(const BrrRegion &obj)
: mAddr(-1)
, mData(NULL)
{
    mData = new unsigned char[obj.mSize];
    memcpy(mData, obj.mData, obj.mSize);
    mSize = obj.mSize;
    mLoopPoint = obj.mLoopPoint;
}
*/
BrrRegion &BrrRegion::operator=(const BrrRegion &obj)
{
    if (mData != NULL) {
        delete [] mData;
    }
    mData = new unsigned char[obj.mSize];
    memcpy(mData, obj.mData, obj.mSize);
    mSize = obj.mSize;
    mAddr = obj.mAddr;
    mLoopPoint = obj.mLoopPoint;
    return(*this);
}

BrrRegion::~BrrRegion()
{
    if (mData != NULL) {
        delete [] mData;
    }
}

bool BrrRegion::SetAddr(int addr)
{
    if (mAddr != addr) {
        mAddr = addr;
        return true;
    }
    return false;
}

int BrrRegion::GetAddr()
{
    return mAddr;
}

const unsigned char *BrrRegion::GetData()
{
    return mData;
}

void BrrRegion::SetLoopFlag(bool isLoop)
{
    if (mData != NULL) {
        int flagPtr = (mSize / 9) * 9 - 9;
        if (isLoop) {
            mData[flagPtr] |= 2;
        }
        else {
            mData[flagPtr] &= ~2;
        }
    }
}
//--------------------------------------

MemManager::MemManager(int size)
: mTotalSize(0)
{
    mMemSize = size;
    mDirAddr = 0x200;
    mBrrStartAddr = 0x600;
    mBrrEndAddr = 0x10000;
    
    pthread_mutex_init(&mMapMtx, 0);
}

MemManager::~MemManager()
{
    pthread_mutex_destroy(&mMapMtx);
}

bool MemManager::WriteData(int srcn, const unsigned char *data, int size, int loopPoint)
{
    BrrRegion newRegion(data, size, loopPoint);
    if (data == NULL) {
        return false;
    }
    pthread_mutex_lock(&mMapMtx);
    // 容量不足ではfalseを返す
    if ((mTotalSize+size) > CalcBrrSize()) {
        return false;
    }
    mTotalSize += size;
    mRegions[srcn & 0xff] = newRegion;
    pthread_mutex_unlock(&mMapMtx);
    return true;
}

void MemManager::DeleteData(int srcn)
{
    pthread_mutex_lock(&mMapMtx);
    auto it = mRegions.find(srcn);
    if (it != mRegions.end()) {
        mTotalSize -= it->second.GetSize();
        mRegions.erase(it);
    }
    pthread_mutex_unlock(&mMapMtx);
}

bool MemManager::HasData(int srcn)
{
    auto it = mRegions.find(srcn);
    if (it != mRegions.end()) {
        return true;
    }
    return false;
}

void MemManager::UpdateMem(C700DSP *dsp)
{
    pthread_mutex_lock(&mMapMtx);
    int nextAddr = mBrrEndAddr;
    auto it = mRegions.begin();
    while (it != mRegions.end()) {
        nextAddr -= it->second.GetSize();
        if (nextAddr < mBrrStartAddr) {
            break;
        }
        if (it->second.SetAddr(nextAddr)) {
            dsp->WriteRam(nextAddr, it->second.GetData(), it->second.GetSize());
            // DIRに書き込む
            unsigned char addrLoop[4];
            addrLoop[0] = nextAddr & 0xff;
            addrLoop[1] = (nextAddr >> 8) & 0xff;
            addrLoop[2] = (nextAddr + it->second.GetLoopPoint()) & 0xff;
            addrLoop[3] = ((nextAddr + it->second.GetLoopPoint()) >> 8) & 0xff;
            dsp->WriteRam(mDirAddr + it->first * 4, addrLoop[0]);
            dsp->WriteRam(mDirAddr + it->first * 4+1, addrLoop[1]);
            dsp->WriteRam(mDirAddr + it->first * 4+2, addrLoop[2]);
            dsp->WriteRam(mDirAddr + it->first * 4+3, addrLoop[3]);
        }
        it++;
    }
    pthread_mutex_unlock(&mMapMtx);
}

void MemManager::ChangeLoopPoint(int srcn, int lp, C700DSP *dsp)
{
    pthread_mutex_lock(&mMapMtx);
    auto it = mRegions.find(srcn);
    if (it != mRegions.end()) {
        it->second.SetLoopPoint(lp);
        unsigned char loopPoint[2];
        loopPoint[0] = (it->second.GetAddr() + lp) & 0xff;
        loopPoint[1] = ((it->second.GetAddr() + lp) >> 8) & 0xff;
        dsp->WriteRam(mDirAddr + it->first * 4 + 2, loopPoint[0]);
        dsp->WriteRam(mDirAddr + it->first * 4 + 3, loopPoint[1]);
    }
    pthread_mutex_unlock(&mMapMtx);
}

void MemManager::ChangeLoopFlag(int srcn, bool isLoop, C700DSP *dsp)
{
    pthread_mutex_lock(&mMapMtx);
    auto it = mRegions.find(srcn);
    if (it != mRegions.end()) {
        it->second.SetLoopFlag(isLoop);
        int flagPtr = (it->second.GetSize() / 9) * 9 - 9;
        unsigned char loopFlag = it->second.GetData()[flagPtr];
        dsp->WriteRam(it->second.GetAddr() + flagPtr, loopFlag);
    }
    pthread_mutex_unlock(&mMapMtx);
}

void MemManager::SetBrrEndAddr(int addr)
{
    if (addr > mMemSize) {
        addr = mMemSize;
    }
    mMemSize = addr;
}
