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
#ifndef plNetCommonMessage_inc
#define plNetCommonMessage_inc

#include "hsTypes.h"
#include "hsSafeRefCnt.h"

//
// refcntable data
//
class plNetCommonMessageData : public hsSafeRefCnt
{
private:
	char *fData;			// sent
public:
	plNetCommonMessageData(char* d) : fData(d) {}
	~plNetCommonMessageData() 
	{ 
		hsAssert(RefCnt()==1, "illegal refcnt");
		delete [] fData; 
	}

	char* GetData() const { return fData; }
};

//
// basic msg payload w/ refcntable data
//
class plNetCommonMessage
{
	plNetCommonMessage(const plNetCommonMessage & other);
private:
	plNetCommonMessageData* fMsgData;
protected:
	UInt32 fLen;			// sent
public:
	plNetCommonMessage() : fLen(0),fMsgData(nil) {}
	virtual ~plNetCommonMessage() { hsRefCnt_SafeUnRef(fMsgData); }

	// setters
	void SetData(char *d)		
	{	
		plNetCommonMessageData* n = d ? TRACKED_NEW plNetCommonMessageData(d) : nil;
		hsRefCnt_SafeAssign(fMsgData, n);		
		hsRefCnt_SafeUnRef(n);
	}
	void SetMsgData(plNetCommonMessageData* d)		
	{	
		hsRefCnt_SafeAssign(fMsgData, d);		
	}
	void SetLen(UInt32 l)		{ fLen=l; }

	// getters
	char* GetData()	const { return fMsgData ? fMsgData->GetData() : nil; } 
	virtual UInt32 GetDataLen() { return fLen; }
	UInt32 GetLen()			const { return fLen;  }
	plNetCommonMessageData* GetMsgData() const { return fMsgData; }
};
#endif // plNetCommonMessage_inc
