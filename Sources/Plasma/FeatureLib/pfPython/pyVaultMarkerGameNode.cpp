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
//////////////////////////////////////////////////////////////////////
//
// pyVaultMarkerGameNode   - a wrapper class to provide interface to the RelVaultNode
//
//////////////////////////////////////////////////////////////////////

#include <Python.h>
#include <string_theory/string>

#include "pyGeometry3.h"

#include "pyVaultMarkerGameNode.h"
#include "plVault/plVault.h"

//create from the Python side
pyVaultMarkerGameNode::pyVaultMarkerGameNode()
    : pyVaultNode()
{
    fNode->SetNodeType(plVault::kNodeType_MarkerGame);
}

ST::string pyVaultMarkerGameNode::GetGameName () const
{
    if (fNode) {
        VaultMarkerGameNode access(fNode);
        return access.GetGameName();
    }
    return ST::string();
}

void pyVaultMarkerGameNode::SetGameName (const ST::string& name)
{
    if (fNode) {
        VaultMarkerGameNode access(fNode);
        access.SetGameName(name);
    }
}

ST::string pyVaultMarkerGameNode::GetReward() const
{
    if (fNode) {
        VaultMarkerGameNode access(fNode);
        return access.GetReward();
    }
    return ST::string();
}

void pyVaultMarkerGameNode::SetReward(const ST::string& value)
{
    if (fNode) {
        VaultMarkerGameNode access(fNode);
        access.SetReward(value);
    }
}

PyObject* pyVaultMarkerGameNode::GetMarkers() const
{
    if (!fNode)
        PYTHON_RETURN_NONE;

    VaultMarkerGameNode marker(fNode);
    std::vector<VaultMarker> collector;
    marker.GetMarkerData(collector);

    PyObject* list = PyList_New(collector.size());
    for (size_t i = 0; i < collector.size(); ++i) {
        PyObject* marker_tup = PyTuple_New(4);
        PyTuple_SET_ITEM(marker_tup, 0, PyLong_FromLong(collector[i].id));
        PyTuple_SET_ITEM(marker_tup, 1, PyUnicode_FromSTString(collector[i].age));
        PyTuple_SET_ITEM(marker_tup, 2, pyPoint3::New(collector[i].pos));
        PyTuple_SET_ITEM(marker_tup, 3, PyUnicode_FromSTString(collector[i].description));
        PyList_SET_ITEM(list, i, marker_tup);
    }
    return list;
}

void pyVaultMarkerGameNode::SetMarkers(const std::vector<VaultMarker>& markers)
{
    if (fNode) {
        VaultMarkerGameNode marker(fNode);
        marker.SetMarkerData(markers);
    }
}
