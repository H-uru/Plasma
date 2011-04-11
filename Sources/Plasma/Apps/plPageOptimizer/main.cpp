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
#include "../plResMgr/plResManager.h"
#include "../pfPython/plPythonFileMod.h"
#include "../plGImage/plFontCache.h"
#include "../plPhysX/plSimulationMgr.h"
#include "../plAvatar/plAvatarMgr.h"

#include "plPageOptimizer.h"
#include "../plFile/plFileUtils.h"

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		printf("plPageOptimizer: wrong number of arguments");
		return 1;
	}

	printf("Optimizing %s...", plFileUtils::GetFileName(argv[1]));

	plFontCache* fontCache;
#ifndef _DEBUG
	try
	{
#endif
		plResManager* resMgr = TRACKED_NEW plResManager;
		hsgResMgr::Init(resMgr);

		// Setup all the crap that needs to be around to load
		plSimulationMgr::Init();
		fontCache = TRACKED_NEW plFontCache;
		plPythonFileMod::SetAtConvertTime();
#ifndef _DEBUG
	} catch (...)
	{
		printf(" ***crashed on init");
		return 2;
	}
#endif

#ifndef _DEBUG
	try
#endif
	{
		plPageOptimizer optimizer(argv[1]);
		optimizer.Optimize();
	}
#ifndef _DEBUG
	catch (...)
	{
		printf(" ***crashed on optimizing");
		return 2;
	}
#endif

#ifndef _DEBUG
	try
	{
#endif
		// Deinit the crap
		fontCache->UnRegisterAs(kFontCache_KEY);
		fontCache = nil;
		plSimulationMgr::Shutdown();

		// Reading in objects may have generated dirty state which we're obviously
		// not sending out. Clear it so that we don't have leaked keys before the
		// ResMgr goes away.
		std::vector<plSynchedObject::StateDefn> carryOvers;
		plSynchedObject::ClearDirtyState(carryOvers);

		hsgResMgr::Shutdown();
#ifndef _DEBUG
	} catch (...)
	{
		printf(" ***crashed on shutdown");
		return 2;
	}
#endif

	return 0;
}
