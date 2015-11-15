//
//  DspRegFIFO.h
//  gimicUsbSpcPlay
//
//  Created by osoumen on 2014/10/26.
//  Copyright (c) 2014年 osoumen. All rights reserved.
//

#ifndef __gimicUsbSpcPlay__DspRegFIFO__
#define __gimicUsbSpcPlay__DspRegFIFO__

#include <iostream>
#include <list>

class DspRegFIFO {
public:
    typedef struct {
        long    time;
        bool    isRam;
        unsigned short addr;
        unsigned char data;
    } DspWrite;
    
    DspRegFIFO();
    ~DspRegFIFO();
    
    void AddDspWrite(long time, unsigned char addr, unsigned char data);
    void AddRamWrite(long time, unsigned short addr, unsigned char data);
    size_t GetNumWrites() { return mDspWrite.size(); }
    DspWrite PopFront();
    void Clear();
    
    std::list<DspWrite> mDspWrite;
    
    //static DspRegFIFO* GetInstance();
};

#endif /* defined(__gimicUsbSpcPlay__DspRegFIFO__) */
