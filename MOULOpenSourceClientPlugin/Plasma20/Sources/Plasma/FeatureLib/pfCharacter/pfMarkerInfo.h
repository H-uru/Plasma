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
#ifndef pfMarkerInfo_h_inc
#define pfMarkerInfo_h_inc

#include "../pnKeyedObject/plKey.h"
#include "../pnKeyedObject/plUoid.h"
#include "hsGeometry3.h"

class plMessage;
class plGameMarkerModifier;

class pfMarkerInfo
{
public:
	enum MarkerType { kMarkerOpen, kMarkerGreen, kMarkerRed, kMarkerLocal, kMarkerLocalSelected };

protected:
	// MarkerMgr will set this up
	static plUoid fMarkerUoid;

	plKey fKey;
	plGameMarkerModifier* fMod;
	hsPoint3 fPosition;
	MarkerType fType;
	double fLastChange;	// Last time this marker changed hands
	bool fVisible;
	bool fIsNew;
	bool fSpawned;

	void IPlayBounce(bool play);
	void IPlayColor(bool play);
	void IPlaySound(bool place);

public:
	pfMarkerInfo(const hsPoint3& pos, bool isNew);
	~pfMarkerInfo() {}

	static void Init();

	plKey GetKey() { return fKey; }

	void Spawn(MarkerType type);
	void InitSpawned(plKey markerKey);
	void Remove();

	void Update(double curTime);

	void Show(bool show);
	bool IsVisible() { return fVisible; }

	void SetType(pfMarkerInfo::MarkerType type);
	pfMarkerInfo::MarkerType GetType() { return fType; }

	void SetFrozen(double freezeStartTime);
	bool IsFrozen() { return fLastChange != 0; }

	void PlayHitSound() { IPlaySound(false); }
};

#endif // pfMarkerInfo_h_inc
