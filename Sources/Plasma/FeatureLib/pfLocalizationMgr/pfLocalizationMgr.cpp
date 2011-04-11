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
//////////////////////////////////////////////////////////////////////
//
// pfLocalizationMgr - singleton class for managing localization of
//                     strings (so python doesn't have to do it)
//
//////////////////////////////////////////////////////////////////////

#include "hsTypes.h"

#include "pfLocalizedString.h"
#include "pfLocalizationDataMgr.h"
#include "pfLocalizationMgr.h"

//////////////////////////////////////////////////////////////////////
//// pfLocalizationMgr Functions /////////////////////////////////////
//////////////////////////////////////////////////////////////////////

pfLocalizationMgr	*pfLocalizationMgr::fInstance = nil;

//// Constructor/Destructor //////////////////////////////////////////

pfLocalizationMgr::pfLocalizationMgr()
{
	hsAssert(!fInstance, "Tried to create the localization manager more than once!");
	fInstance = this;
}

pfLocalizationMgr::~pfLocalizationMgr()
{
	fInstance = nil;
}

//// Initialize //////////////////////////////////////////////////////

void pfLocalizationMgr::Initialize(const std::string & dataPath)
{
	if (fInstance)
		return;

	fInstance = TRACKED_NEW pfLocalizationMgr();
	pfLocalizationDataMgr::Initialize(dataPath); // set up the data manager
}

//// Shutdown ////////////////////////////////////////////////////////

void pfLocalizationMgr::Shutdown()
{
	if (fInstance)
	{
		pfLocalizationDataMgr::Shutdown(); // make sure the subtitle data manager is shut down
		delete fInstance;
	}
}

//// GetString ///////////////////////////////////////////////////////

std::wstring pfLocalizationMgr::GetString(const std::wstring & path, const std::vector<std::wstring> & args)
{
	return pfLocalizationDataMgr::Instance().GetElement(path) % args;
}

std::wstring pfLocalizationMgr::GetString(const std::wstring & path)
{
	std::vector<std::wstring> args; // blank args so that % signs are still handled correctly
	return pfLocalizationDataMgr::Instance().GetElement(path) % args;
}
