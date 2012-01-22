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
#ifndef _pyVaultAgeInfoNode_h_
#define _pyVaultAgeInfoNode_h_

//////////////////////////////////////////////////////////////////////
//
// pyVaultAgeInfoNode   - a wrapper class to provide interface to the RelVaultNode
//
//////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "hsStlUtils.h"

#include <Python.h>
#include "pyGlueHelpers.h"

#include "pyVaultNode.h"

struct RelVaultNode;
class pyVaultSDLNode;
class pyAgeInfoStruct;


class pyVaultAgeInfoNode : public pyVaultNode
{
private:
    mutable char fAgeInstGuid[64];
    mutable std::string fAgeFilename;
    mutable std::string fAgeInstName;
    mutable std::string fAgeUserName;
    mutable std::string fAgeDispName;
    mutable std::string fAgeDescription;

protected:
    // should only be created from C++ side
    pyVaultAgeInfoNode(RelVaultNode* vaultNode);

    //create from the Python side
    pyVaultAgeInfoNode(int n=0);

public:
    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptVaultAgeInfoNode);
    static PyObject *New(RelVaultNode* vaultNode);
    static PyObject *New(int n=0);
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyVaultAgeInfoNode object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyVaultAgeInfoNode); // converts a PyObject to a pyVaultAgeInfoNode (throws error if not correct type)

    static void AddPlasmaClasses(PyObject *m);

//==================================================================
// class RelVaultNode : public plVaultNode
//
    PyObject *  GetCanVisitFolder() const; // returns pyVaultPlayerInfoListNode
    PyObject * GetAgeOwnersFolder() const; // returns pyVaultPlayerInfoListNode
    PyObject* GetChildAgesFolder( void ); // returns pyVaultFolderNode
    PyObject *  GetAgeSDL() const; // returns pyVaultSDLNode
    PyObject * GetCzar() const; // returns pyVaultPlayerInfoNode

    PyObject * GetParentAgeLink () const;   // returns pyVaultAgeLinkNode, or None if not a child age.

    const char * GetAgeFilename() const;
    void    SetAgeFilename( const char * v );

    const char * GetAgeInstanceName() const;
    void    SetAgeInstanceName( const char * v );

    const char * GetAgeUserDefinedName() const;
    void    SetAgeUserDefinedName( const char * v );

    const char * GetAgeInstanceGuid() const;
    void    SetAgeInstanceGuid( const char * guid );

    const char * GetAgeDescription() const;
    void    SetAgeDescription( const char * v );

    int32_t   GetSequenceNumber() const;
    void    SetSequenceNumber( int32_t v );
    
    int32_t   GetAgeLanguage() const;
    void    SetAgeLanguage( int32_t v );

    uint32_t  GetAgeID() const;
    void    SetAgeID( uint32_t v );

    uint32_t  GetCzarID() const;

    bool    IsPublic() const;

    const char * GetDisplayName() const;

    PyObject * AsAgeInfoStruct() const; // returns pyAgeInfoStruct
};

#endif // _pyVaultAgeInfoNode_h_
