//
//  macOSUtils.mm
//  C700
//
//  Created by osoumen on 2017/03/05.
//
//

#include "macOSUtils.h"
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

void GetHomeDirectory(char *outPath, int inSize)
{
    const char *path = [NSHomeDirectory() UTF8String];
    strncpy(outPath, path, inSize);
}

void OpenURL(const char *curl)
{
    CFStringRef urlStr = CFStringCreateWithCString(kCFAllocatorDefault, curl, kCFStringEncodingUTF8);
    CFURLRef url = CFURLCreateWithString(kCFAllocatorDefault, urlStr, NULL);
    NSWorkspace* workspace = [NSWorkspace sharedWorkspace];
    [workspace openURL: (NSURL*)url];
    CFRelease(url);
    CFRelease(urlStr);
}
