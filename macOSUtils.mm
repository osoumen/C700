//
//  macOSUtils.mm
//  C700
//
//  Created by osoumen on 2017/03/05.
//
//

#include "macOSUtils.h"
#import <Foundation/Foundation.h>

void GetHomeDirectory(char *outPath, int inSize)
{
    const char *path = [NSHomeDirectory() UTF8String];
    strncpy(outPath, path, inSize);
}
