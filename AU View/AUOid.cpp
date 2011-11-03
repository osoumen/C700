/*
 *  gui.cpp
 *  Muso
 *
 *  Created by Airy André on 11/11/05.
 *  Copyright 2005 plasq. All rights reserved.
 *
 */

#include "AUOid.h"

namespace AUGUI {
	
oid_t newOid(oid_t parent, int auid)
{
		return (parent<<16)|auid;
}

oid_t newOid()
{
	static uint16_t currentPaneOid = 0;
	return ++currentPaneOid;
}

oid_t newOid(oid_t parent)
{
	static uint16_t currentPaneOid = 4000;
	return (parent<<16)|(--currentPaneOid);
}

oid_t newPaneOid(oid_t parent)
{
	static uint16_t currentPaneOid = 0xFFFF;
	return (parent<<16)|(--currentPaneOid);
}

};
