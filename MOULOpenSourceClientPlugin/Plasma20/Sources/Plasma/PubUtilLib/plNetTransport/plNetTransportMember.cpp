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
#include <algorithm>
#include "plNetTransportMember.h"

//
// add a channel subscription if it's not already there
// returns true if added
//
hsBool plNetTransportMember::AddSubscription(int chan)
{
	if (FindSubscription(chan)==-1)
	{
		fSubscriptions.push_back(chan);
		return true;
	}
	return false;
}

hsBool plNetTransportMember::RemoveSubscription(int chan)
{
	int idx=FindSubscription(chan);
	if (idx>=0)
	{
		fSubscriptions.erase(fSubscriptions.begin()+idx);
		return true;
	}
	return false;
}

int plNetTransportMember::FindSubscription(int chan)
{
	std::vector<int>::iterator it=std::find(fSubscriptions.begin(), fSubscriptions.end(), chan);
	return (it==fSubscriptions.end()) ? -1 : (it-fSubscriptions.begin());
}

std::string plNetTransportMember::AsStdString() const
{
	if (IsServer())
	{
		return "(server)";
	}
	else
	{
		return GetPlayerName();
	}
}
