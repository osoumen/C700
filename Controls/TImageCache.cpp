/*
 *  TImageCache.cpp
 *  NeuSynth
 *
 *  Created by Airy André on Sun Nov 24 2002.
 *  Copyright (c) 2002 Airy André. All rights reserved.
 *
 */

#include "TImageCache.h"
#include "AUGUIUtilities.h"

using namespace AUGUI;

CFMutableDictionaryRef TImageCache::mImageCache = 0;

CGImageRef TImageCache::GetImage(CFURLRef url)
{
    CGImageRef image = 0;

    if (mImageCache == 0) {
		mImageCache = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    }
    if (CFDictionaryGetValueIfPresent(mImageCache, url, (const void **)&image)) {
		if (image) CFRetain(image);
    } else  {
		CGDataProviderRef provider = CGDataProviderCreateWithURL( url );
		if (provider) {
			image = CGImageCreateWithPNGDataProvider( provider, NULL, false,  kCGRenderingIntentDefault );
			if (image == NULL)
				image = CGImageCreateWithJPEGDataProvider( provider, NULL, false,  kCGRenderingIntentDefault );
			CGDataProviderRelease( provider );
		}
		if (image)
		{
			CGImageRef prerenderedImage = PreRenderImage(image);
			if (prerenderedImage)
			{
				CFRelease(image);
				image = prerenderedImage;
			}
			CFDictionaryAddValue(mImageCache, url, image);
		}
    }
    return image;
}

CGImageRef TImageCache::GetImage(CFBundleRef bundle, CFStringRef resourceName, CFStringRef resourceType, CFStringRef subDirName)
{
    CGImageRef image = 0;
	if (resourceName && CFStringGetLength(resourceName)) {
	CFURLRef url = ::CFBundleCopyResourceURL( bundle, resourceName, resourceType, subDirName );
	if (url) {
		image = TImageCache::GetImage(url);
		CFRelease( url );
	}
	}
    return image;
}

//! This routine creates an offscreen bitmap graphics context pointing at
//! a data buffer allocated with \c malloc() that is large enough to hold
//! a rendered version of the image passed in to it. After the image is
//! drawn into this context, a data provider is created for the data buffer.
//! A \c CGImageRef is created with this data provider and returned. The
//! buffer used to hold the image data is allocated aligned on a 16-byte
//! boundary, to give the OS the option to use Altivec.
//! \param theImage An image to render into an offscreen buffer.
//! \result New CGImageRef owned by the called.
//! \todo Instead of using the image's bit depth, use the bit depth of
//!		  the main device. This should help speed up drawing by not
//!		  requiring up or downsampling to blit to the device buffer.
//! \todo Flip the context before rendering the image. Then control classes
//!		  could use CGContextDrawImage instead of HIViewDrawCGImage and
//!		  hopefully that would speed things up a little.
CGImageRef TImageCache::PreRenderImage(CGImageRef theImage)
{
	// get image info
	size_t	width = CGImageGetWidth(theImage);
	size_t height = CGImageGetHeight(theImage);

	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
	size_t bitsPerComponent = 8;
	size_t bitsPerPixel = 32;
	size_t bytesPerRow = ((width * bitsPerPixel) + 7) / 8;
	CGImageAlphaInfo alphaInfo = kCGImageAlphaPremultipliedLast;

	// create buffer and context
	size_t dataSize = bytesPerRow * height;
	void *buffer;
	void *data = malloc_aligned(dataSize, &buffer);
	if (!data) {
		CFRelease(colorSpace);
		return NULL;
	}
	CGContextRef context = CGBitmapContextCreate(data, width, height, bitsPerComponent, bytesPerRow, colorSpace, alphaInfo);
	if (!context) {
		CFRelease(colorSpace);
		return NULL;
	}

	// draw image into context
	CGRect drawRect = CGRectMake(0, 0, width, height);
	CGContextDrawImage(context, drawRect, theImage);
	CGContextRelease(context);
	
	// create data provider for this image buffer
	CGDataProviderRef provider = CGDataProviderCreateWithData(buffer, data, dataSize, ReleaseImageBuffer);
	CGImageRef resultImage = CGImageCreate(width, height, bitsPerComponent, bitsPerPixel, bytesPerRow, colorSpace, alphaInfo, provider, NULL, true, kCGRenderingIntentDefault);
	CGDataProviderRelease(provider);
	CFRelease(colorSpace);
	
	return resultImage;
}

//! Called when the data provider's release count reaches zero and it is
//! time to dispose of the image data buffer. We simply call \c free()
//! on \c info, which is the pointer to the real, unaligned buffer.
void TImageCache::ReleaseImageBuffer(void * info, const void* data, size_t dataSize)
{
	if (data)
		free(info); //const_cast<void*>(data));
}

void TImageCache::Reset()
{
    if (mImageCache) {
		CFRelease(mImageCache);
		mImageCache = 0;
    }
}
