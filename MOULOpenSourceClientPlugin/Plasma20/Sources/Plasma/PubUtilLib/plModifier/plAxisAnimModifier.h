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

#ifndef plAxisAnimMod_inc
#define plAxisAnimMod_inc

#include "hsStlUtils.h"
#include "../pnModifier/plSingleModifier.h"


class plKey;
class plNotifyMsg;
class plAxisInputInterface;

class plAxisAnimModifier : public plSingleModifier
{
public:
	enum
	{
		kTypeX,
		kTypeY,
		kTypeLogic,
	};
protected:
	
	plKey fXAnim;
	plKey fYAnim;
	plKey fNotificationKey;
	
	hsScalar	fXPos;
	hsScalar	fYPos;

	hsBool			fActive;
	hsBool			fAllOrNothing;
	int				fIface;
	plNotifyMsg*	fNotify;
	
	std::string		fAnimLabel;

	plAxisInputInterface	*fInputIface;

	virtual hsBool IEval(double secs, hsScalar del, UInt32 dirty);

public:
	plAxisAnimModifier(); 
	virtual ~plAxisAnimModifier();

	CLASSNAME_REGISTER( plAxisAnimModifier );
	GETINTERFACE_ANY( plAxisAnimModifier, plSingleModifier );

	virtual hsBool	MsgReceive(plMessage* msg);
	virtual void	SetTarget(plSceneObject* so);

	void SetAllOrNothing(hsBool b) { fAllOrNothing = b; }

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	void SetXAnim(plKey c) { fXAnim = c; }
	void SetYAnim(plKey c) { fYAnim = c; }
	void SetNotificationKey(plKey k) { fNotificationKey = k; }
	plNotifyMsg* GetNotify() { return fNotify; }

	const char* GetAnimLabel() const {return fAnimLabel.c_str();}
	void SetAnimLabel(const char* a) {fAnimLabel = a;			}

};

#endif // plAxisAnimMod_inc
