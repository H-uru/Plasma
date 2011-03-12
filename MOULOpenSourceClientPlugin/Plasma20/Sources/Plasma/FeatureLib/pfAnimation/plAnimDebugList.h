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
#ifndef plAnimDebugList_inc
#define plAnimDebugList_inc

// Simple debugging tool, everything is public
// This class collects a list of keyed objects that deal with
// animation, to report info on them when requested.
class plAnimDebugList
{
public:
	hsBool fEnabled;
	hsTArray<plKey> fSOKeys;
	hsTArray<plKey> fMaterialKeys;

	plAnimDebugList() : fEnabled(false) {}
	~plAnimDebugList() {}

	void AddObjects(char *subString);
	void RemoveObjects(char *subString);
	void ShowReport();
};

#endif