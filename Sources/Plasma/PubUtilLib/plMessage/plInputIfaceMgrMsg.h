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
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  plInputIfaceMgrMsg Header                                               //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _plInputIfaceMgrMsg_h
#define _plInputIfaceMgrMsg_h

#include "HeadSpin.h"
#include "pnMessage/plMessage.h"
#include "pnUUID/pnUUID.h"

class plInputInterface;
class plInputIfaceMgrMsg : public plMessage
{
    protected:

        uint8_t   fCommand;
        plInputInterface    *fInterface;
        uint32_t  fPageID;
        ST::string  ageName;
        ST::string  ageFileName;
        ST::string  spawnPoint;
        plUUID ageInstanceGuid;
        plKey fAvKey;
    public:

        enum 
        {
            kAddInterface,
            kRemoveInterface,
            kEnableClickables,          /// YEEEEEECH!!!!
            kDisableClickables,
            kSetOfferBookMode,
            kClearOfferBookMode,
            kNotifyOfferAccepted,
            kNotifyOfferRejected,
            kNotifyOfferCompleted,
            kDisableAvatarClickable,
            kEnableAvatarClickable,
            kGUIDisableAvatarClickable,
            kGUIEnableAvatarClickable,
            kSetShareSpawnPoint,
            kSetShareAgeInstanceGuid,
        };

        plInputIfaceMgrMsg() : plMessage(nullptr, nullptr, nullptr) { SetBCastFlag(kBCastByExactType); fInterface = nullptr; fAvKey = nullptr; }
        plInputIfaceMgrMsg(plKey &receiver, uint8_t command) : plMessage(nullptr, nullptr, nullptr) { AddReceiver(receiver); fCommand = command; fInterface = nullptr; fAvKey = nullptr; }
        plInputIfaceMgrMsg(uint8_t command) : plMessage(nullptr, nullptr, nullptr) { SetBCastFlag(kBCastByExactType); fCommand = command; fInterface = nullptr; fAvKey = nullptr; }
        plInputIfaceMgrMsg(uint8_t command, uint32_t pageID) : plMessage(nullptr, nullptr, nullptr) { SetBCastFlag(kBCastByExactType); fCommand = command; fPageID = pageID; fInterface = nullptr; fAvKey = nullptr; }
        ~plInputIfaceMgrMsg();

        CLASSNAME_REGISTER( plInputIfaceMgrMsg );
        GETINTERFACE_ANY( plInputIfaceMgrMsg, plMessage );

        void Read(hsStream* s, hsResMgr* mgr) override;
        void Write(hsStream* s, hsResMgr* mgr) override;

        void              SetAgeName(const ST::string& s) { ageName = s; }
        ST::string        GetAgeName() const { return ageName; }
        void              SetAgeFileName(const ST::string& s) { ageFileName = s; }
        ST::string        GetAgeFileName() const { return ageFileName; }
        void              SetSpawnPoint(const ST::string& s) { spawnPoint = s; }
        ST::string        GetSpawnPoint() const { return spawnPoint; }
        void              SetAgeInstanceGuid(const plUUID& guid) { ageInstanceGuid = guid; }
        const plUUID&     GetAgeInstanceGuid() const { return ageInstanceGuid; }
        uint8_t           GetCommand() const { return fCommand; }
        uint32_t          GetPageID() const { return fPageID; }
        void              SetIFace(plInputInterface *iface);
        plInputInterface* GetIFace() const { return fInterface; }
        plKey             GetAvKey() const { return fAvKey; }
        void              SetAvKey(const plKey& k ) { fAvKey = k; }
};

#endif // _plInputIfaceMgrMsg_h
