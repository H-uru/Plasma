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

#ifndef plDynaDecalEnableMsg_inc
#define plDynaDecalEnableMsg_inc

#include "../pnMessage/plMessage.h"
#include "../pnKeyedObject/plKey.h"

class hsStream;
class hsResMgr;

class plDynaDecalEnableMsg : public plMessage
{
protected:
	enum {
		kAtEnd		= 0x1,
		kArmature	= 0x2
	};
	plKey					fKey;
	double					fConTime;
	hsScalar				fWetLength;
	UInt32					fFlags;
	UInt32					fID;
public:
	plDynaDecalEnableMsg();
	plDynaDecalEnableMsg(const plKey& r, const plKey& armOrShapeKey, double conTime, hsScalar wetLength, hsBool end, UInt32 id=UInt32(-1), hsBool isArm=true);
	~plDynaDecalEnableMsg();

	CLASSNAME_REGISTER( plDynaDecalEnableMsg );
	GETINTERFACE_ANY( plDynaDecalEnableMsg, plMessage );

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	// ArmKey undefined unless kArmature flag is set. You have to check.
	const plKey&			GetArmKey() const { return fKey; }
	void					SetArmKey(const plKey& k) { fKey = k; SetArmature(true); }

	hsBool					IsArmature() const { return 0 != (fFlags & kArmature); }
	void					SetArmature(hsBool b) { if(b)fFlags |= kArmature; else fFlags &= ~kArmature; }

	const plKey&			GetShapeKey() const { return fKey; }
	void					SetShapeKey(const plKey& k) { fKey = k; SetArmature(false); }

	double					GetContactTime() const { return fConTime; }
	void					SetContactTime(double t) { fConTime = t; }

	hsScalar				GetWetLength() const { return fWetLength; }
	void					SetWetLength(hsScalar w) { fWetLength = w; }

	hsBool					AtEnd() const { return 0 != (fFlags & kAtEnd); }
	void					SetAtEnd(hsBool b) { if(b)fFlags |= kAtEnd; else fFlags &= ~kAtEnd; }

	UInt32					GetID() const { return fID; }
	void					SetID(UInt32 n) { fID = n; }
};

#endif // plDynaDecalEnableMsg_inc
