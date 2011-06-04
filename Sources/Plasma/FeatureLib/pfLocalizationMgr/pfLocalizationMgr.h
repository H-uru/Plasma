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

#ifndef _pfLocalizationMgr_h
#define _pfLocalizationMgr_h

#include "hsTypes.h"
#include "hsStlUtils.h"

class pfLocalizationMgr
{
private:
	static pfLocalizationMgr	*fInstance;
protected:
	pfLocalizationMgr();
public:
	virtual ~pfLocalizationMgr();

	static void Initialize(const std::string & dataPath);
	static void Shutdown();
	static pfLocalizationMgr &Instance(void) {return *fInstance;}
	static bool InstanceValid(void) {return fInstance != nil;}

	// Returns the final localized string, designated by path, and with the arguments properly substituted
	// if you want to use the default argument order, just use %s like you would with printf, BUT, if you
	// want the arguments in a different order (like you had to switch things around for a specific language)
	// then you use %1s, %2s, %3s and so on to specify arguments, these two cannot be mixed and you won't get
	// the results you expect if you do mix them. Path is specified by Age.Set.Name
	std::wstring GetString(const std::wstring & path, const std::vector<std::wstring> & args);
	std::wstring GetString(const std::wstring & path);
};

#endif
