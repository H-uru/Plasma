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
#include "hsStlUtils.h"
#include "hsRefCnt.h"

#include "../pnKeyedObject/plKey.h"

class hsStream;
class hsResMgr;

//
// HACK
// Basically, I just made this class because I couldn't stand duplicating
// all the code and strings for plOneShotMsg and plAvOneShotMsg. - Colin
//
class plOneShotCallbacks : public hsRefCnt
{
public:
	class plOneShotCallback
	{
	public:
		plOneShotCallback(char *marker, plKey &receiver, Int16 user) :
		  fMarker(marker), fReceiver(receiver) , fUser(user) {}

		char *fMarker;
		plKey fReceiver;
		Int16 fUser;
	};

protected:
	std::vector<plOneShotCallback> fCallbacks;

public:
	plOneShotCallbacks();
	~plOneShotCallbacks();

	void AddCallback(const char *marker, plKey &receiver, Int16 user=0);
	int GetNumCallbacks();
	plOneShotCallback& GetCallback(int i);

	// IO 
	void Read(hsStream* stream, hsResMgr* mgr);
	void Write(hsStream* stream, hsResMgr* mgr);
};
