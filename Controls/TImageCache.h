/*
 *  TImageCache.h
 *  NeuSynth
 *
 *  Created by Airy André on Sun Nov 24 2002.
 *  Copyright (c) 2002 Airy André. All rights reserved.
 *
 */

#include <Carbon/Carbon.h>
#include <TRect.h>

/*!
 * \brief Manages a single cache of images for all control classes.
 *
 * \todo Add a method to draw an image in a given context. This will allo
 *		 us to change the way images are drawn without requiring changes
 *		 in the control classes themselves.
 */
class TImageCache
{
public:
    static CGImageRef GetImage(CFURLRef url);
    static CGImageRef GetImage(CFBundleRef bundle, CFStringRef resourceName, CFStringRef resourceType, CFStringRef subDirName=NULL);
    static void Reset();

	static CGImageRef PreRenderImage(CGImageRef theImage);
private:
    static CFMutableDictionaryRef mImageCache;
	
	static void ReleaseImageBuffer(void * info, const void* data, size_t dataSize);
};
