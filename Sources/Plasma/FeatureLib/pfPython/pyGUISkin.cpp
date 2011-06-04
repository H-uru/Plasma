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
//////////////////////////////////////////////
//
//
///////////////////////////////////////////////

#include "pyKey.h"
#include "pyColor.h"
#include "cyPythonInterface.h"

#include "../pfGameGUIMgr/pfGameGUIMgr.h"
#include "../pfGameGUIMgr/pfGUIPopUpMenu.h"

#include "pyGUISkin.h"

pyGUISkin::pyGUISkin(pyKey& gckey)
{
	fGCkey = gckey.getKey();
}

pyGUISkin::pyGUISkin(plKey objkey)
{
	fGCkey = objkey;
}

pyGUISkin::pyGUISkin()
{
	fGCkey = nil;
}

hsBool pyGUISkin::IsGUISkin(pyKey& gckey)
{
	if ( gckey.getKey() && pfGUISkin::ConvertNoRef(gckey.getKey()->GetObjectPtr()) )
		return true;
	return false;
}

// override the equals to operator
hsBool pyGUISkin::operator==(const pyGUISkin &gcobj) const
{
	plKey theirs = ((pyGUISkin&)gcobj).getObjKey();
	if ( fGCkey == nil && theirs == nil )
		return true;
	else if ( fGCkey != nil && theirs != nil )
		return (fGCkey->GetUoid()==theirs->GetUoid());
	else
		return false;
}


// getter and setters
plKey pyGUISkin::getObjKey()
{
	return fGCkey;
}


PyObject* pyGUISkin::getObjPyKey()
{
	// create a pyKey object that Python will manage
	return pyKey::New(fGCkey);
}


