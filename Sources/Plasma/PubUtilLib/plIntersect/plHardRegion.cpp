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

#include "hsTypes.h"
#include "hsStream.h"

#include "plHardRegion.h"

#include "plgDispatch.h"
#include "../plMessage/plRenderMsg.h"
#include "plPipeline.h"

plHardRegion::plHardRegion() 
:	fState(kDirty)
{
}

plHardRegion::~plHardRegion()
{
}

void plHardRegion::SetKey(plKey k)
{
	plRegionBase::SetKey(k);

#if 0 // The caching of this probably isn't worth it.
	// We'll try evaluation every time first, and try
	// this later as an optimization.
	if( k )
	{
		plgDispatch::Dispatch()->RegisterForExactType(plRenderMsg::Index(), GetKey());
	}
#endif // Caching
}

void plHardRegion::Read(hsStream* s, hsResMgr* mgr)
{
	plRegionBase::Read(s, mgr);

	fState = kDirty;
}

void plHardRegion::Write(hsStream* s, hsResMgr* mgr)
{
	plRegionBase::Write(s, mgr);
}

hsBool plHardRegion::CameraInside() const
{
	if( fState & kDirty )
	{
		if( ICameraInside() )
			fState |= kCamInside;
		else
			fState &= ~kCamInside;
		fState &= ~kDirty;
	}
	return 0 != (fState & kCamInside);
}

hsBool plHardRegion::MsgReceive(plMessage* msg)
{
	plRenderMsg* rend = plRenderMsg::ConvertNoRef(msg);
	if( rend )
	{
		fState |= kDirty;
		fCamPos = rend->Pipeline()->GetViewPositionWorld();
		return true;
	}

	return plRegionBase::MsgReceive(msg);
}

