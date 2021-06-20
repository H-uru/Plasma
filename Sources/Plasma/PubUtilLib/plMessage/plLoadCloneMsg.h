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

#ifndef plLoadCloneMsgMsg_INC
#define plLoadCloneMsgMsg_INC

#include "pnMessage/plMessage.h"

class plKey;
class plUoid;

/** \class plLoadCloneMsg
    Tell the net client manager to allocate a new object based on copying an
    existing, statically keyed object.
    On the initiating machine, this will synthesize a new key and load the object.
    On the remote machines, it will use the (enclosed) new key and load the object.
    On both machines, the requestor will be notified after the object is loaded.
    */
class plLoadCloneMsg : public plMessage
{
public:
    /** Default constructor. only useful for sending over the network, prior
        to reading and writing. The clone message needs to synthesize its key
        to do anything, and this constructor doesn't do that, because it doesn't
        have enough information. */
    plLoadCloneMsg();

    /** Canonical constructor. If you're trying to initiate a clone, this is
        the one you want to use.
        These messages are *always* sent to the net client manager. You can't
        address them.
        After they are received on the remote machines, they are forwarded
        the remote versions of the requestor.

        \param uoidToClone - Specifies the template that will be cloned.
        \param requestorKey - The key of the object that is requesting the clone. It's
            strongly recommended that this be a local object, so that we don't get the same
            requestor creating multiple clones by starting the process on several machines.
        \param userData - Whatever you want. Will be propagated to the requestor after cloning.
                        
    */
    plLoadCloneMsg(const plUoid &uoidToClone, plKey requestorKey, uint32_t userData);

    /** This constructor form is for when you want to send a clone message based on
        an existing cloned object. The two primary uses of this are:
        - unload an existing clone
        - replicate a locally existing clone to other machines. You'll need
        to set up the network propagation parameters yourself.
        \param existing - The key to the clone you want to work on
        \param requestor - Will be notified when the (un)clone is processed)
        \param userData - Whatever you want. Will be propagated to the requestor.
        \param isLoading - Are we loading or unloading?
        */
    plLoadCloneMsg(plKey existing, plKey requestor, uint32_t userData, bool isLoading);

    virtual ~plLoadCloneMsg();

    CLASSNAME_REGISTER(plLoadCloneMsg);
    GETINTERFACE_ANY(plLoadCloneMsg, plMessage);

    // IO 
    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

    void ReadVersion(hsStream* stream, hsResMgr* mgr) override;
    void WriteVersion(hsStream* stream, hsResMgr* mgr) override;
    
    plKey   GetCloneKey();
    plKey   GetRequestorKey();
    bool    IsValidMessage();
    uint32_t  GetUserData();
    uint32_t  GetOriginatingPlayerID();
    void    SetOriginatingPlayerID(uint32_t playerId);
    bool    GetIsLoading();
    void    SetTriggerMsg(plMessage *msg);
    plMessage *GetTriggerMsg();


protected:
    plKey   fCloneKey;              // the key that will be loaded
    plKey   fRequestorKey;          // forward the message to this guy after the clone is created
    bool    fValidMsg;              // only gets set if the message built successfully
    uint32_t  fUserData;              // let requestors send some data to their remote versions
    uint32_t  fOriginatingPlayerID;   // network / player id of the client initiating the request
    bool    fIsLoading;             // true if we're loading; false if we're unloading
    plMessage *fTriggerMsg;         // Handy place to store a message that caused you to request a clone,
                                    // so you can see it and continue processing once your clone is loaded.
};

#endif
