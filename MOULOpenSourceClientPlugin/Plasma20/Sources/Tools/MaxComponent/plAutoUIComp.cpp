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
#include "plAutoUIComp.h"

#include <algorithm>

#include "resource.h"
#include "notify.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

plAutoUIComp::plAutoUIComp(plAutoUIClassDesc *cd)
{
	fDesc = TRACKED_NEW ParamBlockDesc2(plComponentBase::kBlkComp, "Auto", IDS_COMP_AUTO, cd, P_AUTO_CONSTRUCT, plComponentBase::kRefComp, end);
	cd->autoComp = this;
}

////////////////////////////////////////////////////////////////////////////////
// Get value from control
//

hsBool plAutoUIComp::GetCheckBox(Int16 id, plComponentBase *comp)
{
	IParamBlock2 *pblock = comp->GetParamBlockByID(plComponentBase::kBlkComp);
	return pblock->GetInt(id);
}

hsScalar plAutoUIComp::GetFloatSpinner(Int16 id, plComponentBase *comp)
{
	IParamBlock2 *pblock = comp->GetParamBlockByID(plComponentBase::kBlkComp);
	return pblock->GetFloat(id);
}

int plAutoUIComp::GetIntSpinner(Int16 id, plComponentBase *comp)
{
	IParamBlock2 *pblock = comp->GetParamBlockByID(plComponentBase::kBlkComp);
	return pblock->GetInt(id);
}

TSTR plAutoUIComp::GetEditBox(Int16 id, plComponentBase *comp)
{
	IParamBlock2 *pblock = comp->GetParamBlockByID(plComponentBase::kBlkComp);
	return pblock->GetStr(id);	
}

INode *plAutoUIComp::GetPickNode(Int16 id, plComponentBase *comp, int idx)
{
	IParamBlock2 *pblock = comp->GetParamBlockByID(plComponentBase::kBlkComp);
	return pblock->GetINode(id, 0, idx);
}

int plAutoUIComp::Count(Int16 id, plComponentBase *comp)
{
	IParamBlock2 *pblock = comp->GetParamBlockByID(plComponentBase::kBlkComp);
	return pblock->Count(id);
}

//

void plAutoUIComp::BeginEditParams(IObjParam *ip, ReferenceMaker *obj, ULONG flags, Animatable *prev)
{
	CreateAutoRollup(((plComponentBase*)obj)->GetParamBlockByID(plComponentBase::kBlkComp));
}

void plAutoUIComp::EndEditParams(IObjParam *ip, ReferenceMaker *obj, ULONG flags, Animatable *prev)
{
	DestroyAutoRollup();
}
