/*
 *  AUProperty.h
 *  Muso
 *
 *  Created by Airy André on 13/11/05.
 *  Copyright 2005 plasq. All rights reserved.
 *
 */

#pragma once

#include <string>
#include <sys/types.h>

namespace AUGUI {
	typedef enum {
		kInteger,
		kFloat,
		kString,
		kPicture,
		kColor,
		kPoint,
		kRect,
		kFont,
		kBool
	} property_type_t;
	
	class font_t {
public:
		CFStringRef name;
		long size;
	};
	
	class color_t {
public:
		float red, green, blue, alpha; /* 0..1 */
	};
	
	class point_t {
public:
		float x, y;
	};
	
	class rect_t {
public:
		float x, y, w, h;
	};
	
	class property_t {
public:
		property_t() : mTag(0) {};
		property_t(uint32_t tag, CFStringRef name, CFStringRef label, property_type_t type) :
		mTag(tag), mName(name), mLabel(label), mType(type) {};
		uint32_t tag() const { return mTag; };
		property_type_t type() const { return mType; };
		CFStringRef label() const { return mLabel; };
		CFStringRef name() const { return mName; };
		
		static property_t null;
protected:
		uint32_t mTag;
		CFStringRef mName;
		CFStringRef mLabel;
		property_type_t mType;
	};
};