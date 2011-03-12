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
//////////////////////////////////////////////////////////////////////////////
//
//	plResMgrSettings - Class that holds all the various settings for 
//					   plResManager
//
//// History /////////////////////////////////////////////////////////////////
//
//	6.22.2002 mcn	- Created
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plResMgrSettings_h
#define _plResMgrSettings_h

#include "hsTypes.h"

class plResMgrSettings
{
protected:
	friend class plResManager;

	bool fFilterOlderPageVersions;
	bool fFilterNewerPageVersions;

	UInt8 fLoggingLevel;

	bool fPassiveKeyRead;
	bool fLoadPagesOnInit;

	plResMgrSettings()
	{
		fFilterOlderPageVersions = true;
		fFilterNewerPageVersions = true;
		fPassiveKeyRead = false;
		fLoadPagesOnInit = true;
		fLoggingLevel = 0;
	}

public:
	enum LogLevels
	{
		kNoLogging = 0,
		kBasicLogging = 1,
		kDetailedLogging = 2,
		kObjectLogging = 3,
		kObjectDetailLogging = 4
	};

	bool GetFilterOlderPageVersions() const { return fFilterOlderPageVersions; }
	void SetFilterOlderPageVersions(bool f) { fFilterOlderPageVersions = f; }

	bool GetFilterNewerPageVersions() const { return fFilterNewerPageVersions; }
	void SetFilterNewerPageVersions(bool f) { fFilterNewerPageVersions = f; }

	UInt8	GetLoggingLevel() const { return fLoggingLevel; }
	void	SetLoggingLevel(UInt8 level) { fLoggingLevel = level; }

	bool GetPassiveKeyRead() const { return fPassiveKeyRead; }
	void SetPassiveKeyRead(bool p) { fPassiveKeyRead = p; }

	bool GetLoadPagesOnInit() const { return fLoadPagesOnInit; }
	void SetLoadPagesOnInit(bool load) { fLoadPagesOnInit = load; }

	static plResMgrSettings& Get();
};

#endif // _plResMgrSettings_h
