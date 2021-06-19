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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

#include <Python.h>
#include "pyKey.h"

#include "pfGameGUIMgr/pfGUIDynDisplayCtrl.h"
#include "plGImage/plDynamicTextMap.h"
#include "pyDynamicText.h"

#include "pyGUIControlDynamicText.h"

pyGUIControlDynamicText::pyGUIControlDynamicText(pyKey& gckey) : pyGUIControl(gckey)
{
}

pyGUIControlDynamicText::pyGUIControlDynamicText(plKey objkey) : pyGUIControl(std::move(objkey))
{
}


bool pyGUIControlDynamicText::IsGUIControlDynamicText(pyKey& gckey)
{
    if ( gckey.getKey() && pfGUIDynDisplayCtrl::ConvertNoRef(gckey.getKey()->ObjectIsLoaded()) )
        return true;
    return false;
}


    //specific interface functions
uint32_t pyGUIControlDynamicText::GetNumMaps()
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIDynDisplayCtrl* pdtmod = pfGUIDynDisplayCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( pdtmod )
            return pdtmod->GetNumMaps();
    }
    return 0;
}


PyObject* pyGUIControlDynamicText::GetMap(uint32_t i)
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIDynDisplayCtrl* pdtmod = pfGUIDynDisplayCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( pdtmod )
        {
            // get the owner dialog modifier pointer
            plDynamicTextMap* pdyntm = pdtmod->GetMap(i);
            if ( pdyntm )
            {
                // create a pythonized Dialog class object (Python will manage it)
                PyObject* dynTextObj = pyDynamicText::New(pdyntm->GetKey());
                pyDynamicText* pDynText = pyDynamicText::ConvertFrom(dynTextObj);
                // since these are for GUI dialog items, don't net propagate these!
                pDynText->SetNetPropagate(false);
                pDynText->SetNetForce(false);
                return dynTextObj;
            }
        }
    }
    char errmsg[256];
    sprintf(errmsg,"DynamicDisplay map number %d could be found on this control...?",i);
    PyErr_SetString(PyExc_KeyError, errmsg);
    PYTHON_RETURN_ERROR;
}
