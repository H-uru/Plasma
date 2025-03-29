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
#ifndef plClientMsg_inc
#define plClientMsg_inc

#include "plRefMsg.h"

#include "pnKeyedObject/plUoid.h"

//
// Handles various types of client (app) msgs, relating 
// to loading rooms, players, camera, and progress bars
//
class plClientMsg : public plMessage
{
    int fMsgFlag;
    ST::string fAgeName;
    std::vector<plLocation> fRoomLocs;

    void IReset();

    class GraphicsSettings
    {
    public:
        GraphicsSettings() : fWidth (800), fHeight(600), fColorDepth(32), fWindowed(false), fNumAASamples(0),
                             fMaxAnisoSamples(0), fVSync(false) {}
        int fWidth;
        int fHeight;
        int fColorDepth;
        bool fWindowed;
        int fNumAASamples;
        int fMaxAnisoSamples;
        bool fVSync;
    };


public:
    enum
    {
        kLoadRoom,
        kLoadRoomHold,
        kUnloadRoom,
        kLoadNextRoom,  // For internal client use only

        kLoadAgeKeys,
        kReleaseAgeKeys,

        kQuit,              // exit the app
        kInitComplete,
        kDisableRenderScene,
        kEnableRenderScene,
        kResetGraphicsDevice,
        kSetGraphicsDefaults,

        kFlashWindow,
    };

    // graphics settings fields
    GraphicsSettings fGraphicsSettings;


    plClientMsg() { IReset();}
    plClientMsg(int i) { IReset(); fMsgFlag = i; }  

    CLASSNAME_REGISTER(plClientMsg);
    GETINTERFACE_ANY(plClientMsg, plMessage);

    int GetClientMsgFlag() const { return fMsgFlag; }

    void AddRoomLoc(const plLocation& loc);

    // Used for kLoadAgeKeys, kLetGoOfAgeKeys only
    ST::string  GetAgeName() const { return fAgeName; }
    void        SetAgeName(ST::string age) { fAgeName = std::move(age); }

    int GetNumRoomLocs() { return fRoomLocs.size(); }
    const plLocation& GetRoomLoc(int i) const { return fRoomLocs[i]; }
    const std::vector<plLocation>& GetRoomLocs() { return fRoomLocs; }

    // IO
    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;
};

class plClientRefMsg : public plRefMsg
{
public:
    enum
    {
        kLoadRoom   = 0,
        kLoadRoomHold,
        kManualRoom,
    };

    plClientRefMsg(): fType(-1), fWhich(-1) {};

    plClientRefMsg(const plKey &r, uint8_t refMsgFlags, int8_t which , int8_t type)
        : plRefMsg(r, refMsgFlags), fType(type), fWhich(which) {}


    CLASSNAME_REGISTER(plClientRefMsg);
    GETINTERFACE_ANY(plClientRefMsg, plRefMsg);

    int8_t                    fType;
    int8_t                    fWhich;

    // IO - not really applicable to ref msgs, but anyway
    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;
};

#endif // plClientMsg
