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
#include "plORConditionalObject.h"
#include "../plPhysical/plDetectorModifier.h"
#include "hsResMgr.h"
#include "../../NucleusLib/pnModifier/plLogicModBase.h"

#include "../plMessage/plCondRefMsg.h"


plORConditionalObject::plORConditionalObject() 
{
	
}

plORConditionalObject::~plORConditionalObject()
{
	fChildren.SetCountAndZero(0);
}

hsBool plORConditionalObject::MsgReceive(plMessage* msg)
{
	plCondRefMsg* pCondMsg = plCondRefMsg::ConvertNoRef(msg);
	if (pCondMsg)
	{
		fChildren[pCondMsg->fWhich] = plConditionalObject::ConvertNoRef( pCondMsg->GetRef() );
		fChildren[pCondMsg->fWhich]->SetLogicMod(fLogicMod);
		return true;
	}
	
	return plConditionalObject::MsgReceive(msg);
}

void plORConditionalObject::SetLogicMod(plLogicModBase* pMod)
{
	fLogicMod = pMod;
	for (int i = 0; i < fChildren.Count(); i++)
	{
		if( fChildren[i] )
			fChildren[i]->SetLogicMod(pMod);
	}
}

hsBool plORConditionalObject::Satisfied() 
{
	for (int i = 0; i < fChildren.Count(); i++)
	{
		if (fChildren[i]->Satisfied())
			return true;
	}
	return false;
}

void plORConditionalObject::AddChild(plConditionalObject* pObj)
{
	fChildren.Append(pObj);
	pObj->SetLogicMod(fLogicMod);
}

void plORConditionalObject::Reset()
{
	for (int i = 0; i < fChildren.Count(); i++)
		fChildren[i]->Reset();
}

void plORConditionalObject::Read(hsStream* stream, hsResMgr* mgr)
{
	plConditionalObject::Read(stream, mgr);
	
	plCondRefMsg* refMsg;
	int n = stream->ReadSwap32();
	fChildren.SetCountAndZero(n);
	for(int i = 0; i < n; i++ )
	{	
		refMsg = TRACKED_NEW plCondRefMsg(GetKey(), i);
		mgr->ReadKeyNotifyMe(stream,refMsg, plRefFlags::kActiveRef);
	}	
}

void plORConditionalObject::Write(hsStream* stream, hsResMgr* mgr)
{
	plConditionalObject::Write(stream, mgr);
	
	stream->WriteSwap32(fChildren.GetCount());
	for( int i = 0; i < fChildren.GetCount(); i++ )
		mgr->WriteKey(stream, fChildren[i]);
}

