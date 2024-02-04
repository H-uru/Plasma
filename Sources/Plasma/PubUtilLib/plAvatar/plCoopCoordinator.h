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
#ifndef plCoopCoordinator_h
#define plCoopCoordinator_h

/////////////////////////////////////////////////////////////////////////////////////////
//
// INCLUDES
//
/////////////////////////////////////////////////////////////////////////////////////////

#include <string_theory/string>
// global
#include "pnKeyedObject/hsKeyedObject.h"

/////////////////////////////////////////////////////////////////////////////////////////
//
// FORWARDS
//
/////////////////////////////////////////////////////////////////////////////////////////

class plAvBrainCoop;
class plMessage;

/////////////////////////////////////////////////////////////////////////////////////////
//
// DEFINITIONS
//
/////////////////////////////////////////////////////////////////////////////////////////

/** \class plCoopCoordinator
    Manages cooperation betweeen several generic brains; primarily serves as a place
    to send synchronization messages. Originally this was going to be the role of the
    'host' brain, but that didn't provide a good interface point for Python.
    This object can either be a "helper" for Python, or its functionality can be
    recoded in Python.
    */
class plCoopCoordinator : public hsKeyedObject
{
public:
    plCoopCoordinator();
    plCoopCoordinator(plKey host, plKey guest,
                      plAvBrainCoop *hostBrain, plAvBrainCoop *guestBrain,
                      const ST::string &synchBone, uint32_t hostOfferStage, uint32_t guestAcceptStage,
                      plMessage *guestAcceptMsg,
                      bool autoStartGuest);

    bool MsgReceive(plMessage *msg) override;

    void Run();

    uint32_t GetInitiatorID();
    uint16_t GetInitiatorSerial();

    bool IsActiveForReal();

    // rtti
    CLASSNAME_REGISTER( plCoopCoordinator );
    GETINTERFACE_ANY( plCoopCoordinator, hsKeyedObject);

    // i/o
    void Read(hsStream *stream, hsResMgr *mgr) override;
    void Write(hsStream *stream, hsResMgr *mgr) override;

protected:
    void IStartHost();
    void IStartGuest();
    void IContinueGuest();
    void IAdvanceParticipant(bool host);

    void IStartTimeout();
    void ITimeout();

    plKey fHostKey;
    plKey fGuestKey;

    plAvBrainCoop *fHostBrain;
    plAvBrainCoop *fGuestBrain;

    uint32_t fInitiatorID;
    uint32_t fInitiatorSerial;
    
    uint32_t  fHostOfferStage;            // when we enter this stage, the offer is ready
    uint32_t  fGuestAcceptStage;          // when we enter this stage, the offer is accepted

    plMessage *fGuestAcceptMsg;         // send this when the guest accepts

    ST::string fSynchBone;
    bool fAutoStartGuest;
    bool fGuestAccepted;

    bool fGuestLinked; // guest linked, so ignore the timeout timer

    friend class plNetClientMsgScreener; // Needs to screen the brains and message
};

#endif // plCoopCoordinator_h
