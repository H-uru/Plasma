/*==LICENSE==*

CyanWorlds.com Engine - MMOG client, server and tools
Copyright (C) 2011  Cyan Worlds, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#ifndef PL_COMPONENT_EXT_H
#define PL_COMPONENT_EXT_H

#include "plComponentBase.h"
#include "plComponentTools.h"
#include "../MaxExport/plErrorMsg.h"

class plMaxNodeBase;

class plComponentExt : public plComponentBase
{
public:
	int CanConvertToType(Class_ID obtype)
		{ return (obtype == EXT_COMPONENT_CLASSID) ? 1 : plComponentBase::CanConvertToType(obtype); }

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNodeBase *node, plComponentTools *tools, plErrorMsg *pErrMsg) { return true; }
	virtual hsBool PreConvert(plMaxNodeBase *node, plComponentTools *tools, plErrorMsg *pErrMsg) { return true; }
	virtual hsBool Convert(plMaxNodeBase *node, plComponentTools *tools, plErrorMsg *pErrMsg) = 0;

	// DeInit pass--free up any temp memory you might have allocated here
	virtual hsBool DeInit(plMaxNodeBase *node, plComponentTools *tools, plErrorMsg *pErrMsg) { return true; }
};

#endif