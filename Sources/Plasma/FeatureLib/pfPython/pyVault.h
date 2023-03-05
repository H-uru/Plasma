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
#ifndef _pyVault_h_
#define _pyVault_h_

//////////////////////////////////////////////////////////////////////
//
// pyVault   - a wrapper class to provide interface to the plVault and the RelVaultNode
//
//////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "pyGlueHelpers.h"

class plKey;
class plDniCoordinateInfo;

class pyVaultNode;
class pyVaultAgeLinkNode;
class pyVaultFolderNode;
class pyVaultPlayerInfoListNode;
class pyVaultAgeInfoListNode;
class pyVaultAgeInfoNode;
class pyVaultChronicleNode;
class pyVaultTextNoteNode;

class pyAgeInfoStruct;
class pyAgeLinkStruct;

class pySDLStateDataRecord;

class pyVault
{
#ifndef BUILDING_PYPLASMA
protected:
    pyVault() {};

public:
    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptVault);
    PYTHON_CLASS_NEW_DEFINITION;
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyVault object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyVault); // converts a PyObject to a pyVault (throws error if not correct type)

    static void AddPlasmaClasses(PyObject *m);
#else
public:
#endif // BUILDING_PYPLASMA
    static void AddPlasmaConstantsClasses(PyObject *m);

    enum VaultCallbackType
    {
        kVaultConnected = 1,
        kVaultNodeSaved,
        kVaultNodeRefAdded,
        kVaultRemovingNodeRef,
        kVaultNodeRefRemoved,
        kVaultNodeInitialized,
        kVaultOperationFailed,
        kVaultNodeAdded,
        kVaultDisconnected,
    };

#ifndef BUILDING_PYPLASMA

    //////////////////////////////////////////////////
    PyObject* GetPlayerInfo(); // returns pyVaultNode
    PyObject* GetKIUsage();
    PyObject* GetAvatarOutfitFolder(); // returns pyVaultFolderNode
    PyObject* GetAvatarClosetFolder(); // returns pyVaultFolderNode
    PyObject* GetInbox(); // returns pyVaultFolderNode
    PyObject* GetChronicleFolder(); // returns pyVaultFolderNode
    PyObject* GetAgeJournalsFolder(); // returns pyVaultFolderNode
    PyObject* GetAllPlayersFolder();
    PyObject* GetIgnoreListFolder(); // returns pyVaultPlayerInfoListNode
    PyObject* GetBuddyListFolder(); // returns pyVaultPlayerInfoListNode
    PyObject* GetPeopleIKnowAboutFolder(); // returns pyVaultPlayerInfoListNode
    PyObject* GetAgesICanVisitFolder(); // returns pyVaultFolderNode
    PyObject* GetAgesIOwnFolder(); // returns pyVaultFolderNode
    PyObject* GetInviteFolder(); // returns pyVaultFolderNode

    ///////////////
    PyObject* GetLinkToMyNeighborhood() const; // returns pyVaultAgeLinkNode
    PyObject* GetLinkToCity() const; // returns pyVaultAgeLinkNode
    ///////////////
    // Owned ages
    PyObject* GetOwnedAgeLink( const pyAgeInfoStruct & info ); // returns pyVaultAgeLinkNode
    // Visit ages
    PyObject* GetVisitAgeLink( const pyAgeInfoStruct & info ); // returns pyVaultAgeLinkNode
    ///////////////
    // Chronicle
    PyObject* FindChronicleEntry(const ST::string& entryName); // returns pyVaultChronicleNode
    void AddChronicleEntry(const ST::string& name, uint32_t type, const ST::string& value);
    ///////////////
    // publishing
    void    SendToDevice(pyVaultNode& node, const ST::string& deviceName);
    ///////////////
    // yeesha pages, etc.
    PyObject* GetPsnlAgeSDL() const; // returns pySDLStateDataRecord
    void UpdatePsnlAgeSDL( pySDLStateDataRecord & rec );

    ///////////////
    // true if we are joined to our personal age.
    bool InMyPersonalAge() const;
    // true if we are joined to our neighborhood age.
    bool InMyNeighborhoodAge() const;
    // true if we own the age we are in
    bool AmOwnerOfCurrentAge() const;
    // true if we are czar of the age we are in
    bool AmCzarOfCurrentAge() const;
    // true if we own the given age
    bool AmAgeOwner( const pyAgeInfoStruct * ageInfo );
    // true if we are czar of the given age
    bool AmAgeCzar( const pyAgeInfoStruct * ageInfo );

    ///////////////
    // Registser the given age as owned by player.
    void RegisterOwnedAge( const pyAgeLinkStruct & link );
    void UnRegisterOwnedAge(const ST::string& ageFilename);
    // Register the given age as visitable by player
    void RegisterVisitAge( const pyAgeLinkStruct & link );
    void UnRegisterVisitAge(const ST::string& guid);
    // Register a nexus station
    void RegisterMTStation( const ST::string& stationName, const ST::string& mtSpawnPt );

    ///////////////
    // Invite player to visit an age.
    void InvitePlayerToAge( const pyAgeLinkStruct & link, uint32_t playerID );
    void UnInvitePlayerToAge(const ST::string& guid, uint32_t playerID);
    // Offer link to player
    void OfferLinkToPlayer( const pyAgeLinkStruct & link, uint32_t playerID );

    ///////////////
    // Creates neighborhood and joins the player to it as the mayor/czar.
    void CreateNeighborhood();
    // set an age's public status. will fail if you aren't czar of age.
    bool SetAgePublic( const pyAgeInfoStruct * ageInfo, bool makePublic );
    // set an age's public status, also works for non-owners
    bool SetAgePublic( const pyVaultAgeInfoNode * ageInfoNode, bool makePublic );

    PyObject* GetGlobalInbox(); // returns pyVaultFolderNode

    // find matching node
    PyObject* FindNode( pyVaultNode* templateNode ) const; // returns pyVaultNode

#endif // BUILDING_PYPLASMA
};

#endif // _pyVault_h_
