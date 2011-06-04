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

#ifndef plStereizer_inc
#define plStereizer_inc

#include "../pnModifier/plSingleModifier.h"
#include "hsGeometry3.h"
#include "hsMatrix44.h"

class plListenerMsg;
class plMessage;

class plCoordinateInterface;

class hsStream;
class hsResMgr;

class plStereizer : public plSingleModifier
{
protected:

	// Flags - in a plSingleModifier::hsBitVector.
	enum
	{
		kLeftChannel,
		kHasMaster
	};

	// Static properties
	hsScalar		fAmbientDist;
	hsScalar		fTransition;

	hsScalar		fMaxSepDist;
	hsScalar		fMinSepDist;

	hsScalar		fTanAng;

	hsPoint3		fInitPos;

	// Environmental properties, namely of the current listener
	hsPoint3		fListPos;
	hsVector3		fListDirection;
	hsVector3		fListUp;

	virtual hsBool IEval(double secs, hsScalar del, UInt32 dirty);

	hsPoint3	IGetLocalizedPos(const hsVector3& posToList, hsScalar distToList) const;
	hsPoint3	IGetAmbientPos() const;
	void		ISetNewPos(const hsPoint3& newPos);

	hsPoint3	IGetUnStereoPos() const;

	plCoordinateInterface*	IGetParent() const;

	void		ISetHasMaster(hsBool on) { if(on)SetFlag(kHasMaster); else ClearFlag(kHasMaster); }

public:
	plStereizer();
	virtual ~plStereizer();

	CLASSNAME_REGISTER( plStereizer );
	GETINTERFACE_ANY( plStereizer, plSingleModifier );
	
	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	virtual hsBool MsgReceive(plMessage* msg);

	hsBool	Stereize();
	void	SetFromListenerMsg(const plListenerMsg* listMsg);

	void SetAmbientDist(hsScalar d) { fAmbientDist = d; }
	hsScalar GetAmbientDist() const { return fAmbientDist; }

	void SetTransition(hsScalar d) { fTransition = d; }
	hsScalar GetTransition() const { return fTransition; }

	void SetMaxSepDist(hsScalar d) { fMaxSepDist = d; }
	hsScalar GetMaxSepDist() const { return fMaxSepDist; }

	void SetMinSepDist(hsScalar d) { fMinSepDist = d; }
	hsScalar GetMinSepDist() const { return fMinSepDist; }

	void SetSepAngle(hsScalar rads);
	hsScalar GetSepAngle() const;

	void SetAsLeftChannel(hsBool on) { if(on)SetFlag(kLeftChannel); else ClearFlag(kLeftChannel); }
	hsBool IsLeftChannel() const { return HasFlag(kLeftChannel); }

	void SetParentInitPos(const hsPoint3& pos) { fInitPos = pos; }
	const hsPoint3& GetParentInitPos() const { return fInitPos; }

	void SetWorldInitPos(const hsPoint3& pos);
	hsPoint3 GetWorldInitPos() const;

	hsBool CheckForMaster();
	hsBool HasMaster() const { return HasFlag(kHasMaster); }
};

#endif // plStereizer_inc