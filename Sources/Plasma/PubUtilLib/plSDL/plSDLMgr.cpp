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
#include "hsStlUtils.h"
#include "plSDL.h"
#include "../pnNetCommon/plNetApp.h"
#include "../pnNetCommon/pnNetCommon.h"
#include <algorithm>

/////////////////////////////////////////////////////////////////////////////////
// SDL MGR
/////////////////////////////////////////////////////////////////////////////////

//
//
//
plSDLMgr::plSDLMgr() : fSDLDir("SDL"), fNetApp(nil), fBehaviorFlags(0)
{

}

//
//
//
plSDLMgr::~plSDLMgr()
{
	DeInit();
}

bool plSDLMgr::Init( UInt32 behaviorFlags )
{
	fBehaviorFlags = behaviorFlags;
	plSDLParser parser;
	return parser.Parse();
}

void plSDLMgr::DeInit()
{
	IDeleteDescriptors(&fDescriptors);
}

//
// delete all descriptors
//
void plSDLMgr::IDeleteDescriptors(plSDL::DescriptorList* dl)
{
	std::for_each( dl->begin(), dl->end(), xtl::delete_ptr() );
	dl->clear();
}


//
// STATIC
//
plSDLMgr* plSDLMgr::GetInstance()
{
	static plSDLMgr gSDLMgr;
	return &gSDLMgr;
}

//
// search latest and legacy descriptors for one that matches.
// if version is -1, search for latest descriptor with matching name
//
plStateDescriptor* plSDLMgr::FindDescriptor(const char* name, int version, const plSDL::DescriptorList * dl) const
{
	if (!name)
		return nil;

	if ( !dl )
		dl = &fDescriptors;

	plStateDescriptor* sd = nil;

	plSDL::DescriptorList::const_iterator it;

	int highestFound = -1;
	for(it=(*dl).begin(); it!= (*dl).end(); it++)
	{
		if (!_stricmp((*it)->GetName(), name) )
		{
			if ( (*it)->GetVersion()==version )
			{
				sd = *it;
				break;
			}
			else if ( version==plSDL::kLatestVersion && (*it)->GetVersion()>highestFound )
			{
				sd = *it;
				highestFound = (*it)->GetVersion();
			}
		}
	}

	return sd;
}

//
// write latest descriptors to a stream.
// return number of bytes
//
int plSDLMgr::Write(hsStream* s, const plSDL::DescriptorList* dl)
{
	int pos=s->GetPosition();

	if (dl==nil)
		dl=&fDescriptors;

	UInt16 num=dl->size();
	s->WriteSwap(num);

	plSDL::DescriptorList::const_iterator it;
	for(it=dl->begin(); it!= dl->end(); it++)
		(*it)->Write(s);	

	int bytes=s->GetPosition()-pos;
	if (fNetApp)
	{
		hsLogEntry(fNetApp->DebugMsg("Writing %d SDL descriptors, %d bytes", num, bytes));
	}
	return bytes;
}

//
// read descriptors into provided list 
// return number of bytes
//
int plSDLMgr::Read(hsStream* s, plSDL::DescriptorList* dl)
{
	int pos=s->GetPosition();

	if (dl==nil)
		dl=&fDescriptors;

	// clear dl
	IDeleteDescriptors(dl);

	UInt16 num;
	try
	{		
		// read dtor list
		s->ReadSwap(&num);

		int i;
		for(i=0;i<num;i++)
		{
			plStateDescriptor* sd=TRACKED_NEW plStateDescriptor;
			if (sd->Read(s))
				dl->push_back(sd);
		}		
	}
	catch(...)
	{
		if (fNetApp)
		{
			hsLogEntry(fNetApp->DebugMsg("Something bad happened while reading SDLMgr data"));
		}
		return 0;
	}

	int bytes=s->GetPosition()-pos;
	if (fNetApp)
	{
		hsLogEntry(fNetApp->DebugMsg("Reading %d SDL descriptors, %d bytes", num, bytes));
	}
	return bytes;
}

