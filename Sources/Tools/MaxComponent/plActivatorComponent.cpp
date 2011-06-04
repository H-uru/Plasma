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
#include "HeadSpin.h"
#include "plComponent.h"
#include "plComponentReg.h"

void DummyCodeIncludeFuncActive() {}

#define ACTIVATOR_CID Class_ID(0x205a7c50, 0x7a095602)

class plActiveComponent : public plComponent
{
public:
	plActiveComponent();
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg) { return true; }
};

OBSOLETE_CLASS_DESC(plActiveComponent, gActiveDesc, "Activator", "Activator", COMP_TYPE_LOGIC, ACTIVATOR_CID)

plActiveComponent::plActiveComponent()
{
	fClassDesc = &gActiveDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

ParamBlockDesc2 gActiveBlock
(
	plComponent::kBlkComp, _T("activeComp"), 0, &gActiveDesc, P_AUTO_CONSTRUCT, plComponent::kRefComp,
		
	end
);
