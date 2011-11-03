/*
 *  gui.h
 *  Muso
 *
 *  Created by Airy André on 11/11/05.
 *  Copyright 2005 plasq. All rights reserved.
 *
 */
#pragma once

#include <sys/types.h>

namespace AUGUI {
	typedef uint32_t oid_t;
	
	static const oid_t kRootPane=0x0000FFFF;
	
	oid_t newOid(oid_t parent, int auid);
	oid_t newOid();
	oid_t newOid(oid_t parent);	
	oid_t newPaneOid(oid_t parent);
}