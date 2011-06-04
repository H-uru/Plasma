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

#ifndef plDistOpacityMod_inc
#define plDistOpacityMod_inc

#include "hsGeometry3.h"
#include "../pnModifier/plSingleModifier.h"
#include "hsTemplates.h"

class plPipeline;
class plRenderMsg;
class plFadeOpacityLay;


class plDistOpacityMod : public plSingleModifier
{
public:

	enum {
		kTrackAvatar,
		kTrackCamera
	};
protected:

	enum {
		kRefFadeLay
	};

	// Volatile flag, whether we're setup yet or not.
	UInt8			fSetup;

	enum {
		kNearTrans,
		kNearOpaq,
		kFarOpaq,
		kFarTrans,

		kNumDists
	};
	hsScalar		fDists[kNumDists];

	hsPoint3		fRefPos;

	hsTArray<plFadeOpacityLay*> fFadeLays;

	// We only act in response to messages.
	virtual hsBool IEval(double secs, hsScalar del, UInt32 dirty) { return false; }

	hsScalar ICalcOpacity(const hsPoint3& targPos, const hsPoint3& refPos) const;
	void ISetOpacity();

	void ISetup();

	void ICheckDists()
	{
		hsAssert(fDists[kNearTrans] <= fDists[kNearOpaq], "Bad transition values");
		hsAssert(fDists[kNearOpaq] <= fDists[kFarOpaq], "Bad transition values");
		hsAssert(fDists[kFarOpaq] <= fDists[kFarTrans], "Bad transition values");
	}

public:
	plDistOpacityMod();
	virtual ~plDistOpacityMod();

	CLASSNAME_REGISTER( plDistOpacityMod );
	GETINTERFACE_ANY( plDistOpacityMod, plSingleModifier );

	virtual void			SetKey(plKey k);

	virtual hsBool			MsgReceive(plMessage* msg);

	virtual void			Read(hsStream* s, hsResMgr* mgr);
	virtual void			Write(hsStream* s, hsResMgr* mgr);

	virtual void			SetTarget(plSceneObject* so);

	// Rules are:
	// NearTrans <= NearOpaq <= FarOpaque <= FarTrans
	void SetFarDist(hsScalar opaque, hsScalar transparent);
	void SetNearDist(hsScalar transparent, hsScalar opaque);

	hsScalar GetFarTransparent() const { return fDists[kFarTrans]; }
	hsScalar GetNearTransparent() const { return fDists[kNearTrans]; }
	hsScalar GetFarOpaque() const { return fDists[kFarOpaq]; }
	hsScalar GetNearOpaque() const { return fDists[kNearOpaq]; }
};

#endif // plDistOpacityMod_inc
