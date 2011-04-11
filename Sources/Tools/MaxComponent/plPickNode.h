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
#ifndef plPickNode_h_inc
#define plPickNode_h_inc

#include "hsConfig.h"
#include "max.h"
#include <vector>

class plComponentBase;

namespace plPick
{
	bool Node(IParamBlock2 *pb, int paramID, std::vector<Class_ID>* cids, bool single, bool canConvertToType);
	bool Activator(IParamBlock2 *pb, int paramID, bool single);
	bool GUIDialog(IParamBlock2 *pb, int paramID, bool single);
	bool ExcludeRegion(IParamBlock2 *pb, int paramID, bool single);
	bool WaterComponent(IParamBlock2 *pb, int paramID, bool single);
	bool Swim2DComponent(IParamBlock2 *pb, int paramID, bool single);
	bool ClusterComponent(IParamBlock2 *pb, int paramID, bool single);
	bool Animation(IParamBlock2 *pb, int paramID, bool single);
	bool Behavior(IParamBlock2 *pb, int paramID, bool single);
	bool GenericClass(IParamBlock2 *pb, int paramID, bool single, Class_ID classIDToPick );
	bool GrassComponent(IParamBlock2 *pb, int paramID, bool single);

	// Basically the same as activator, but includes other things with built in detectors (ladder)
	// that you can enable/disable but shouldn't be triggering off of
	bool DetectorEnable(IParamBlock2 *pb, int paramID, bool single);

	// Can only pick a target of comp
	bool CompTargets(IParamBlock2 *pb, int paramID, plComponentBase *comp);
	// Can only pick a node this material is applied to
	bool MtlNodes(IParamBlock2* pb, int paramID, Mtl* mtl);

	bool NodeRefKludge(IParamBlock2 *pb, int paramID, std::vector<Class_ID>* cids, bool single, bool canConvertToType);
}

#endif // plPickNode_h_inc