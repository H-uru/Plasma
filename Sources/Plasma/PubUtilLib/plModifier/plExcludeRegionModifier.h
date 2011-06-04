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
#ifndef plExcludeRegionModifier_inc
#define plExcludeRegionModifier_inc

#include "../pnModifier/plSingleModifier.h"
#include "hsMatrix44.h"
#include "hsTemplates.h"
#include "../plModifier/plSDLModifier.h"

//
// Moves all of the avatars out of the area it's SceneObject occupies and makes it
// physical on clear message.  Makes the SO non-physical again on release message.
//
class plExcludeRegionSDLModifier;
class plExcludeRegionModifier : public plSingleModifier
{
protected:
	enum
	{
		kBlockCameras,	
	};
	std::vector<plKey> fSafePoints;	// Safe positions to move avatars to
	hsTArray<plKey> fContainedAvatars;		// Avatars inside our volume
	plExcludeRegionSDLModifier	*fSDLModifier;
	hsBool fSeek; // use smart seek or teleport?
	hsScalar fSeekTime; // how long to seek for
	virtual hsBool IEval(double secs, hsScalar del, UInt32 dirty) {	return true; }

	void ISetPhysicalState(bool cleared);

	int IFindClosestSafePoint(plKey avatar);
	bool ICheckSubworlds(plKey avatar);
	void IMoveAvatars();

	friend class plExcludeRegionSDLModifier;

public:
	plExcludeRegionModifier();
	~plExcludeRegionModifier();

	CLASSNAME_REGISTER(plExcludeRegionModifier);
	GETINTERFACE_ANY(plExcludeRegionModifier, plSingleModifier);

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	virtual hsBool MsgReceive(plMessage* msg);

	virtual void AddTarget(plSceneObject* so);
	virtual void RemoveTarget( plSceneObject *so );

	void AddSafePoint(plKey& key);
	void UseSmartSeek() { fSeek = true; }
	void SetSeekTime(hsScalar s) { fSeekTime = s; }
	void SetBlockCameras(bool block) { fFlags.SetBit(kBlockCameras, block); }
};

class plExcludeRegionSDLModifier : public plSDLModifier
{
protected:
	plExcludeRegionModifier* fXRegion;

	void IPutCurrentStateIn(plStateDataRecord* dstState);
	void ISetCurrentStateFrom(const plStateDataRecord* srcState);

public:
	plExcludeRegionSDLModifier();
	plExcludeRegionSDLModifier(plExcludeRegionModifier* xregion);

	CLASSNAME_REGISTER(plExcludeRegionSDLModifier);
	GETINTERFACE_ANY(plExcludeRegionSDLModifier, plSDLModifier);

	const char* GetSDLName() const { return kSDLXRegion; }
};

#endif // plExcludeRegionModifier_inc
