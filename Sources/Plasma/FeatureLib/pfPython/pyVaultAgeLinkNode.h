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
#ifndef _pyVaultAgeLinkNode_h_
#define _pyVaultAgeLinkNode_h_

//////////////////////////////////////////////////////////////////////
//
// pyVaultAgeLinkNode   - a wrapper class to provide interface to the RelVaultNode
//
//////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "hsStlUtils.h"

#include <Python.h>
#include "pyGlueHelpers.h"

#include "pyVaultNode.h"
#include "plNetCommon/plNetServerSessionInfo.h" // for plAgeLinkStruct

class pyVaultAgeInfoNode;
struct RelVaultNode;
class pyAgeLinkStruct;
class pySpawnPointInfo;
class pySpawnPointInfoRef;


class pyVaultAgeLinkNode : public pyVaultNode
{
private:
    mutable std::string fAgeGuidStr;    // for getting Age GUID

    mutable plAgeLinkStruct     fAgeLinkStruct; // for use with AsAgeLinkStruct()

protected:
    // should only be created from C++ side
    pyVaultAgeLinkNode(RelVaultNode* nfsNode);

    //create from the Python side
    pyVaultAgeLinkNode(int n=0);

public:
    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptVaultAgeLinkNode);
    static PyObject *New(RelVaultNode* nfsNode);
    static PyObject *New(int n=0);
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyVaultAgeLinkNode object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyVaultAgeLinkNode); // converts a PyObject to a pyVaultAgeLinkNode (throws error if not correct type)

    static void AddPlasmaClasses(PyObject *m);

//==================================================================
// class RelVaultNode : public plVaultNode
//
    PyObject*   GetAgeInfo() const; // returns pyVaultAgeInfoNode
    // locked on psnl age bookshelf
    void    SetLocked( bool v );
    bool    GetLocked() const;
    // volatile on psnl age bookshelf
    void    SetVolatile( bool v );
    bool    GetVolatile() const;
    // spawn points
    void    AddSpawnPoint( pySpawnPointInfo & point );  // will only add if not there already.
    void    AddSpawnPointRef( pySpawnPointInfoRef & point );    // will only add if not there already.
    void    RemoveSpawnPoint( pySpawnPointInfo & point );
    void    RemoveSpawnPointRef( pySpawnPointInfoRef & point );
    void    RemoveSpawnPointByName( const char * spawnPtName );
    bool    HasSpawnPoint( const char * spawnPtName ) const;
    PyObject * GetSpawnPoints() const;  // returns list of pySpawnPointInfo

    PyObject * AsAgeLinkStruct() const; // returns pyAgeLinkStruct

};

#endif // _pyVaultAgeLinkNode_h_
