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
#ifndef plNetTransportMember_h
#define plNetTransportMember_h

#include "hsConfig.h"
#include "hsStlUtils.h"
#include "../plNetCommon/plNetMember.h"
#include "../pnKeyedObject/plKey.h" 

//
// This represents a participant in the game, ie. another 
// remote user.
// It is a basic net member with a list of channels that it subscribes to.
//
class plKey;

class plNetTransportMember : public plNetMember
{
public:
	enum TransportFlags
	{
		kSendingVoice	= 1<<0,
		kSendingActions	= 1<<1,
	};
protected:
	plKey 			fAvatarKey;
	std::string		fPlayerName;
	UInt32			fPlayerID;
	std::vector<int> fSubscriptions;	// list of channelGrp subscriptions
	UInt32			fTransportFlags;
	float			fDistSq;			// from local player, temp
	UInt8			fCCRLevel;
public:
	CLASSNAME_REGISTER( plNetTransportMember);
	GETINTERFACE_ANY( plNetTransportMember, plNetMember);

	plNetTransportMember() : fAvatarKey(nil),
		fTransportFlags(0),fPlayerID(0),fDistSq(-1),fCCRLevel(0) {}
	plNetTransportMember(plNetApp* na) : plNetMember(na),
		fAvatarKey(nil),fTransportFlags(0),fPlayerID(0),fDistSq(-1),fCCRLevel(0) {}
	~plNetTransportMember() {}

	void SetDistSq(float s) { fDistSq=s; }
	float GetDistSq() const { return fDistSq; }

	plKey GetAvatarKey() { return fAvatarKey; }
	void SetAvatarKey(plKey k)
		{
			fAvatarKey=k;
		}
	void SetPlayerName(const char * value) { fPlayerName=value;}
	const char * GetPlayerName() const { return fPlayerName.c_str();}
	void SetPlayerID(UInt32 value) { fPlayerID=value;}
	UInt32 GetPlayerID() const { return fPlayerID;}
	void SetIsServer(bool value) { (value)?SetFlags(GetFlags()|kIsServer):SetFlags(GetFlags()&!kIsServer);}
	bool IsServer() const { return (GetFlags()&kIsServer)?true:false;}

	hsBool AddSubscription(int chan);
	hsBool RemoveSubscription(int chan);	// return true on success
	int FindSubscription(int chan);			// return index into subscription array or -1

	int GetNumSubscriptions() { return fSubscriptions.size(); }
	int GetSubscription(int i) { return fSubscriptions[i]; }

	void CopySubscriptions(std::vector<int>* channels) { *channels = fSubscriptions; }

	void SetTransportFlags(UInt32 f) { fTransportFlags=f; }
	UInt32 GetTransportFlags() const { return fTransportFlags; }

	bool IsPeerToPeer() const { return hsCheckBits(fFlags, kRequestP2P); }
	std::string AsStdString() const;
	bool IsEqualTo(const plNetMember * other) const
	{
		const plNetTransportMember * o = plNetTransportMember::ConvertNoRef(other);
		if (!o) return false;
		return o->fPlayerID==fPlayerID;
	}

	bool IsCCR() const { return (fCCRLevel>0);	}
	UInt8 GetCCRLevel() const { return fCCRLevel;	}
	void SetCCRLevel(UInt8 cl) { fCCRLevel=cl;	}
};

#endif	// plNetTransportMember_h
