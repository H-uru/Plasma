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
#ifndef plGameMarkerModifier_h_inc
#define plGameMarkerModifier_h_inc

#include "../pnModifier/plSingleModifier.h"

class plGameMarkerModifier : public plSingleModifier
{
protected:
	plKey fGreenAnimKey;
	plKey fRedAnimKey;
	plKey fOpenAnimKey;
	plKey fBounceAnimKey;
	UInt16 fPlaceSndIdx;
	UInt16 fHitSndIdx;

	virtual hsBool IEval(double secs, hsScalar del, UInt32 dirty) { return true; }

	plKey IFindCloneKey(plKey baseKey);
	
	friend class pfMarkerInfo;
	friend class pfMarkerInfoOwned;

public:
	plGameMarkerModifier() {}

	CLASSNAME_REGISTER(plGameMarkerModifier);
	GETINTERFACE_ANY(plGameMarkerModifier, plSingleModifier);

	void ExportInit(plKey greenKey, plKey redKey, plKey openKey, plKey bounceAnimKey,
					UInt16 placeSndIdx, UInt16 hitSndIdx)
	{
		fGreenAnimKey = greenKey;
		fRedAnimKey = redKey;
		fOpenAnimKey = openKey;
		fBounceAnimKey = bounceAnimKey;
		fPlaceSndIdx = placeSndIdx;
		fHitSndIdx = hitSndIdx;
	}

	void FixupAnimKeys();

	virtual hsBool MsgReceive(plMessage* msg);

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);
};

#endif // plGameMarkerModifier_h_inc
