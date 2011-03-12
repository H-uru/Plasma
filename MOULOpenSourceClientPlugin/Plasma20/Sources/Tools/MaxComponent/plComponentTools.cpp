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
#include "max.h"
#include "../pnModifier/plModifier.h"

#include "plComponentTools.h"

plKey plComponentTools::AddModifier(plMaxNodeBase *node, plModifier *mod)
{
	return fAddModFunc(node, mod);
}

plKey plComponentTools::GetNewKey(const char *name, plModifier *mod, plLocation loc)
{
	return fNewKey(name, mod, loc);
}

void plComponentTools::SetActivatorKey(plMaxNodeBase *activatorNode, plMaxNodeBase *responderNode, plMaxNodeBase *convertNode, plResponderModifier *responderLogic)
{
	fActivator(activatorNode, responderNode, convertNode, responderLogic);
}

plKey plComponentTools::GetAnimCompModKey(plComponentBase *comp, plMaxNodeBase *node)
{
	return fAnimKey(comp, node);
}
/*
plKey plComponentTools::GetAnimCompLightModKey(plComponentBase *comp, plMaxNodeBase *node)
{
	return fAnimLightKey(comp, node);
}
*/
const char *plComponentTools::GetAnimCompAnimName(plComponentBase *comp)
{
	return fAnimName(comp);
}

int plComponentTools::GetMaterialAnimModKey(Mtl* mtl, plMaxNodeBase* node, const char *segName, hsTArray<plKey>& keys)
{
	return fMatMod(mtl, node, segName, keys);
}

int plComponentTools::GetSoundNameAndIndex(plComponentBase* comp, plMaxNodeBase* node, const char*& name)
{
	return fSndNameAndIdx(comp, node, name);
}
