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
#include "hsGeometry3.h"
#include "plBoundInterface.h"
#include "plConvexVolume.h"
#include "hsResMgr.h"

plBoundInterface::plBoundInterface() : fBounds(nil)
{
}

plBoundInterface::~plBoundInterface()
{
	ReleaseData();
}

void plBoundInterface::ReleaseData()
{
	delete fBounds;
	fBounds = nil;
}

void plBoundInterface::Init(plConvexVolume *bounds)
{
	ReleaseData();
	fBounds = bounds;
}

// Right now, this is ignoring the enabled property of ObjInterface, since I'm not aware that
// anything ever makes use of it (and if nothing does, this saves us on some needless matrix
// copying). Should we make use of the disabled prop, this function should just store the l2w
// matrix, but not send an update to fBounds.
void plBoundInterface::SetTransform(const hsMatrix44 &l2w, const hsMatrix44&w2l)
{
	if (fBounds != nil)
		fBounds->Update(l2w);
}

void plBoundInterface::Read(hsStream* s, hsResMgr* mgr)
{
	plObjInterface::Read(s, mgr);
	fBounds = plConvexVolume::ConvertNoRef(mgr->ReadCreatable(s));
	//mgr->ReadKeyNotifyMe(s, new plIntRefMsg(GetKey(), plRefMsg::kOnCreate, 0, plIntRefMsg::kOwner), plRefFlags::kPassiveRef);
}

void plBoundInterface::Write(hsStream* s, hsResMgr* mgr)
{
	plObjInterface::Write(s, mgr);
	mgr->WriteCreatable(s, fBounds);
	//mgr->WriteKey(s, fBounds);
}

// No need to save/load. The coordinate interface on our sceneObject will update us.