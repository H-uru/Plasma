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
#if 0//ndef PL_ANTI_GRAV_ACTION_H
#define PL_ANTI_GRAV_ACTION_H

#include "plAvCallbackAction.h"

class plSwimRegionInterface;

class plAntiGravAction : public plAnimatedCallbackAction
{
public:
	plAntiGravAction(plHKPhysical *physical, plAGApplicator *rootApp);

	/** Return the type of the action as defined in the enum plSimDefs::ActionType.
		Used to retrieve actions by entity/type indexing, and to
		reuse actions that can be shared between entities. */
	virtual plSimDefs::ActionType GetType();

	/** Called by Havok at substep frequency. */
	void apply(Havok::Subspace &s, Havok::hkTime time);

	void SetSurface(plSwimRegionInterface *region, hsScalar surfaceHeight);
	hsScalar GetBuoyancy() { return fBuoyancy; }
	hsBool IsOnGround() { return fOnGround; }
	hsBool HadContacts() { return fHadContacts; }
		
protected:
	void IAdjustBuoyancy();

	hsBool fOnGround;
	hsBool fHadContacts;
	hsScalar fBuoyancy;
	hsScalar fSurfaceHeight;
	plSwimRegionInterface *fCurrentRegion;
};

#endif


