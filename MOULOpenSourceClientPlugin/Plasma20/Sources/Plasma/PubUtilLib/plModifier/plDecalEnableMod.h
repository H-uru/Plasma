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

#ifndef plDecalEnableMod_inc
#define plDecalEnableMod_inc

#include "hsTemplates.h"
#include "../pnModifier/plSingleModifier.h"
#include "../pnKeyedObject/plKey.h"

class plDecalEnableMod : public plSingleModifier
{
protected:
	
	hsTArray<plKey>		fDecalMgrs;

	hsScalar			fWetLength;

	virtual hsBool IEval(double secs, hsScalar del, UInt32 dirty) { return false; }

public:
	plDecalEnableMod();
	virtual ~plDecalEnableMod();

	CLASSNAME_REGISTER( plDecalEnableMod );
	GETINTERFACE_ANY( plDecalEnableMod, plSingleModifier );

	virtual hsBool MsgReceive(plMessage* msg);
	
	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	void SetWetLength(hsScalar t) { fWetLength = t; }
	hsScalar GetWetLength() const { return fWetLength; }

	void AddDecalKey(const plKey& k) { fDecalMgrs.Append(k); }
	UInt32 GetNumDecalKeys() const { return fDecalMgrs.GetCount(); }
	const plKey& GetDecalKey(int i) const { return fDecalMgrs[i]; }
};

#endif // plDecalEnableMod_inc
