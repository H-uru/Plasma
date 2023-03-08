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
//  plAvatarInputInterface                                                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef plAvatarInputInterface_inc
#define plAvatarInputInterface_inc

#include "plInputInterface.h"

#include <string_theory/string>

#include "pnInputCore/plInputMap.h"

#include "hsGeometry3.h"



//// Class Definition ////////////////////////////////////////////////////////
        
class plInputEventMsg;
class plMouseEventMsg;
class plKeyMap;
class plMouseMap;
class plKey;
class hsStream;
class hsResMgr;
class plAvatarInputMap;
class plPipeline;
class plAvatarInputInterface;

//// Little Input Map Helpers ////////////////////////////////////////////////

class plAvatarInputMap 
{
    protected:

        plAvatarInputInterface  *fInterface;

    public:

        plAvatarInputMap();
        virtual ~plAvatarInputMap();
        virtual ST::string GetName() = 0;
        virtual bool IsBasic() { return false; }

        plMouseMap      *fMouseMap;
        uint32_t          fButtonState;
};

// Basic avatar mappings, for when the avatar is in "suspended input" mode.
class plSuspendedMovementMap : public plAvatarInputMap
{
public:
    plSuspendedMovementMap();
    ST::string GetName() override { return ST_LITERAL("Suspended Movement"); }
};

// The above, plus movement
class plBasicControlMap : public plSuspendedMovementMap
{
public:
    plBasicControlMap();
    ST::string GetName() override { return ST_LITERAL("Basic"); }
    bool IsBasic() override { return true; }

};
// The above, plus movement
class plBasicThirdPersonControlMap : public plBasicControlMap
{
public:
    plBasicThirdPersonControlMap();
    ST::string GetName() override { return ST_LITERAL("Basic Third Person"); }
};

class plLadderControlMap : public plSuspendedMovementMap
{
public:
    plLadderControlMap();
    ST::string GetName() override { return ST_LITERAL("LadderClimb"); }
};

class plLadderMountMap : public plSuspendedMovementMap
{
public:
    plLadderMountMap();
    ST::string GetName() override { return ST_LITERAL("Ladder Mount"); }
};

class plLadderDismountMap : public plSuspendedMovementMap
{
public:
    plLadderDismountMap();
    ST::string GetName() override { return ST_LITERAL("Ladder Dismount"); }
};


class plBasicFirstPersonControlMap : public plBasicControlMap
{
public:
    plBasicFirstPersonControlMap();
    ST::string GetName() override { return ST_LITERAL("Basic First Person"); }
};

// Mouse walk mode
class pl3rdWalkMap : public plAvatarInputMap
{
public:
    pl3rdWalkMap();
    virtual ~pl3rdWalkMap();
};

class pl3rdWalkForwardMap : public pl3rdWalkMap
{
public:
    pl3rdWalkForwardMap();
    ST::string GetName() override { return ST_LITERAL("Walking Forward"); }
};

class pl3rdWalkBackwardMap : public pl3rdWalkMap
{
public:
    pl3rdWalkBackwardMap();
    ST::string GetName() override { return ST_LITERAL("Walking Backward"); }
};

class pl3rdWalkBackwardLBMap : public pl3rdWalkMap
{
public:
    pl3rdWalkBackwardLBMap();
    ST::string GetName() override { return ST_LITERAL("Walking Backward (LB)"); }
};

///////////////////////////////////////////////////////////////////////////////////

class plAvatarInputInterface : public plInputInterface
{
    protected:

        uint32_t      fCurrentCursor;
        float    fCursorOpacity, fCursorTimeout, fCursorFadeDelay;

        plAvatarInputMap        *fInputMap;

        static plAvatarInputInterface       *fInstance;

        bool    IHandleCtrlCmd(plCtrlCmd *cmd) override;

        // Gets called once per IUpdate(), just like normal IEval()s
        bool IEval(double secs, float del, uint32_t dirty) override;

        void    IDeactivateCommand(plMouseInfo *info);
        void    IChangeInputMaps(plAvatarInputMap *newMap);
        void    ISetSuspendMovementMode();
        void    ISetBasicMode();
        void    ISetMouseWalkMode(ControlEventCode code);
        void    ISetLadderMap();
        void    ISetPreLadderMap();
        void    ISetPostLadderMap();

        bool    IHasControlFlag(int f) const    { return fControlFlags.IsBitSet(f); }
        void    IClearControlFlag(int which)    { fControlFlags.ClearBit( which ); }

        bool    CursorInBox(plMouseEventMsg* pMsg, hsPoint4 box);
        void    ClearMouseCursor();
        void    DisableMouseInput() { fMouseDisabled = true; }
        void    EnableMouseInput() { fMouseDisabled = false; }
        
        void    Reset();

        void    RequestCursorToWorldPos(float xPos, float yPos, int ID);

        hsBitVector     fControlFlags;
        bool            fMouseDisabled;

        plPipeline*     fPipe;
        int             fCursorState;
        int             fCursorPriority;
        bool    f3rdPerson;

    public:

        plAvatarInputInterface();
        virtual ~plAvatarInputInterface();
    
        void CameraInThirdPerson(bool state);
    
        // Always return true, since the cursor should be representing how we control the avatar
        bool            HasInterestingCursorID() const override { return true; }
        uint32_t        GetPriorityLevel() const override { return kAvatarInputPriority; }
        uint32_t        GetCurrentCursorID() const override { return fCurrentCursor; }
        float           GetCurrentCursorOpacity() const override { return fCursorOpacity; }
        ST::string      GetInputMapName() { return fInputMap ? fInputMap->GetName() : ST::string(); }

        bool        InterpretInputEvent(plInputEventMsg *pMsg) override;
        void        MissedInputEvent(plInputEventMsg *pMsg) override;

        bool        MsgReceive(plMessage *msg) override;

        void        Init(plInputInterfaceMgr *manager) override;
        void        Shutdown() override;

        void        RestoreDefaultKeyMappings() override;
        void        ClearKeyMap() override;
        
        // [dis/en]able mouse commands for avatar movement
        void SuspendMouseMovement();
        void EnableMouseMovement();
        void EnableJump(bool val);
        void EnableForwardMovement(bool val);
        void EnableControl(bool val, ControlEventCode code);
        void ClearLadderMode();
        void SetLadderMode();
        void ForceAlwaysRun(bool val);
        
        void    SetControlFlag(int f, bool val = true)            { fControlFlags.SetBit(f, val); }

        void    SetCursorFadeDelay( float delay ) { fCursorFadeDelay = delay; }

        bool    IsEnterChatModeBound();

        static plAvatarInputInterface   *GetInstance() { return fInstance; }
};



#endif //plAvatarInputInterface_inc
