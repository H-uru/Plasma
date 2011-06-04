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
#include "plSDL.h"
#include "../pnNetCommon/plNetApp.h"

const UInt8 plStateDescriptor::kVersion=1;		// for Read/Write format

/////////////////////////////////////////////////////////////////////////////////
// STATE DESC
/////////////////////////////////////////////////////////////////////////////////

plStateDescriptor::~plStateDescriptor() 
{ 
	IDeInit();
}

void plStateDescriptor::IDeInit()
{
	delete [] fName;  
	int i;
	for(i=0;i<fVarsList.size();i++)
		delete fVarsList[i];
	fVarsList.clear();
}

plVarDescriptor* plStateDescriptor::FindVar(const char* name, int* idx) const
{
	VarsList::const_iterator it;
	for(it=fVarsList.begin(); it != fVarsList.end(); it++)
	{
		if (!stricmp((*it)->GetName(), name))
		{
			if (idx)
				*idx = it-fVarsList.begin();
			return *it;
		}
	}

	return nil;
}


//
// Usage: The GameServer reads and write state descriptors along with each saved game
// 
bool plStateDescriptor::Read(hsStream* s)
{
	UInt8 rwVersion;
	s->ReadSwap(&rwVersion);
	if (rwVersion != kVersion)
	{
		plNetApp::StaticWarningMsg("StateDescriptor Read/Write version mismatch, mine %d, read %d", kVersion, rwVersion);
		return false;
	}

	IDeInit();
	
	delete [] fName;
	fName = s->ReadSafeString();

	UInt16 version=s->ReadSwap16();
	fVersion=version;

	UInt16 numVars=s->ReadSwap16();
	fVarsList.reserve(numVars);

	int i;
	for(i=0;i<numVars; i++)
	{
		UInt8 SDVar=s->ReadByte();		
		plVarDescriptor* var = nil;
		if (SDVar)
			var = TRACKED_NEW plSDVarDescriptor;
		else
			var = TRACKED_NEW plSimpleVarDescriptor;
		if (var->Read(s))
			fVarsList.push_back(var);
		else
			return false;
	}
	return true;
}

//
// Usage: The GameServer reads and write state descriptors alon with each saved game
// 
void plStateDescriptor::Write(hsStream* s) const
{
	s->WriteSwap(kVersion);
	
	s->WriteSafeString(fName);

	UInt16 version=fVersion;
	s->WriteSwap(version);

	UInt16 numVars=fVarsList.size();
	s->WriteSwap(numVars);

	VarsList::const_iterator it;
	for(it=fVarsList.begin(); it!=fVarsList.end(); it++)
	{
		UInt8 SDVar = ((*it)->GetAsSDVarDescriptor() != nil);
		s->WriteByte(SDVar);
		(*it)->Write(s);
	}
}

