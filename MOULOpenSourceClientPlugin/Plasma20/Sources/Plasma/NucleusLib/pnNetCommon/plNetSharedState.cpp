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
#include "hsStream.h"
#include "plNetSharedState.h"
#include "plGenericVar.h"
#include "../pnMessage/plMessage.h"

plNetSharedState::plNetSharedState(char* name) : fServerMayDelete(false)
{ 
	SetName(name);
}

plNetSharedState::~plNetSharedState() 
{ 
	Reset();
}

void plNetSharedState::Reset()
{
	int i;
	for(i=0;i<fVars.size();i++)
		delete fVars[i];
	fVars.clear();
}

void plNetSharedState::Copy(plNetSharedState *ss)
{
	Reset();

	// copy name
	SetName(ss->GetName());

	SetServerMayDelete(ss->GetServerMayDelete());

	// copy vars
	int i;
	for(i=0;i<ss->GetNumVars();i++)
	{
		plGenericVar* sv = TRACKED_NEW plGenericVar;
		*sv = *(ss->GetVar(i));
		AddVar(sv);
	}
}

void plNetSharedState::Read(hsStream* stream)
{	
	Reset();

	plMsgStdStringHelper::Peek(fName, stream);
	Int32 num=stream->ReadSwap32();
	fServerMayDelete = stream->Readbool();
	
	fVars.reserve(num);
	int i;
	for(i=0;i<num;i++)
	{
		plGenericVar* v = TRACKED_NEW plGenericVar;
		v->Read(stream);
		AddVar(v);
	}
}

void plNetSharedState::Write(hsStream* stream)
{	
	plMsgStdStringHelper::Poke(fName, stream);
	Int32 num=GetNumVars();
	stream->WriteSwap32(num);
	
	stream->Writebool(fServerMayDelete);
	int i;
	for(i=0;i<num;i++)
	{
		fVars[i]->Write(stream);
	}
}

