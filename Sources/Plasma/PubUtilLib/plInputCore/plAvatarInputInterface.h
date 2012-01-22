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
#include "pnInputCore/plInputMap.h"

#include "hsTemplates.h"

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
        virtual const char *GetName() = 0;
        virtual hsBool IsBasic() { return false; }

        plMouseMap      *fMouseMap;
        uint32_t          fButtonState;
};

// Basic avatar mappings, for when the avatar is in "suspended input" mode.
class plSuspendedMovementMap : public plAvatarInputMap
{
public:
    plSuspendedMovementMap();
    virtual const char *GetName() { return "Suspended Movement"; }
};

// The above, plus movement
class plBasicControlMap : public plSuspendedMovementMap
{
public:
    plBasicControlMap();
    virtual const char *GetName() { return "Basic"; }
    virtual hsBool IsBasic() { return true; }

};
// The above, plus movement
class plBasicThirdPersonControlMap : public plBasicControlMap
{
public:
    plBasicThirdPersonControlMap();
    virtual const char *GetName() { return "Basic Third Person"; }
};

class plLadderControlMap : public plSuspendedMovementMap
{
public:
    plLadderControlMap();
    virtual const char *GetName() { return "LadderClimb"; }
};

class plLadderMountMap : public plSuspendedMovementMap
{
public:
    plLadderMountMap();
    virtual const char *GetName() { return "Ladder Mount"; }
};

class plLadderDismountMap : public plSuspendedMovementMap
{
public:
    plLadderDismountMap();
    virtual const char *GetName() { return "Ladder Dismount"; }
};


class plBasicFirstPersonControlMap : public plBasicControlMap
{
public:
    plBasicFirstPersonControlMap();
    virtual const char *GetName() { return "Basic First Person"; }
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
    virtual const char *GetName() { return "Walking Forward"; }
};

class pl3rdWalkBackwardMap : public pl3rdWalkMap
{
public:
    pl3rdWalkBackwardMap();
    virtual const char *GetName() { return "Walking Backward"; }
};

class pl3rdWalkBackwardLBMap : public pl3rdWalkMap
{
public:
    pl3rdWalkBackwardLBMap();
    virtual const char *GetName() { return "Walking Backward (LB)"; }
};

///////////////////////////////////////////////////////////////////////////////////

class plAvatarInputInterface : public plInputInterface
{
    protected:

        uint32_t      fCurrentCursor;
        float    fCursorOpacity, fCursorTimeout, fCursorFadeDelay;

        plAvatarInputMap        *fInputMap;

        static plAvatarInputInterface       *fInstance;

        virtual hsBool  IHandleCtrlCmd( plCtrlCmd *cmd );

        // Gets called once per IUpdate(), just like normal IEval()s
        virtual hsBool IEval( double secs, float del, uint32_t dirty );

        void    IDeactivateCommand(plMouseInfo *info);
        void    IChangeInputMaps(plAvatarInputMap *newMap);
        void    ISetSuspendMovementMode();
        void    ISetBasicMode();
        void    ISetMouseWalkMode(ControlEventCode code);
        void    ISetLadderMap();
        void    ISetPreLadderMap();
        void    ISetPostLadderMap();

        hsBool  IHasControlFlag(int f) const    { return fControlFlags.IsBitSet(f); }
        void    IClearControlFlag(int which)    { fControlFlags.ClearBit( which ); }

        hsBool  CursorInBox(plMouseEventMsg* pMsg, hsPoint4 box);
        void    ClearMouseCursor();
        void    DisableMouseInput() { fMouseDisabled = true; }
        void    EnableMouseInput() { fMouseDisabled = false; }
        
        void    Reset();

        void    RequestCursorToWorldPos(float xPos, float yPos, int ID);

        hsBitVector     fControlFlags;
        hsBool          fMouseDisabled;

        plPipeline*     fPipe;
        int             fCursorState;
        int             fCursorPriority;
        hsBool  f3rdPerson;

    public:

        plAvatarInputInterface();
        virtual ~plAvatarInputInterface();
    
        void CameraInThirdPerson(hsBool state);
    
        // Always return true, since the cursor should be representing how we control the avatar
        virtual hsBool      HasInterestingCursorID( void ) const { return true; }
        virtual uint32_t      GetPriorityLevel( void ) const { return kAvatarInputPriority; }
        virtual uint32_t      GetCurrentCursorID( void ) const { return fCurrentCursor; }
        virtual float    GetCurrentCursorOpacity( void ) const { return fCursorOpacity; }
        const char*         GetInputMapName() { return fInputMap ? fInputMap->GetName() : ""; }

        virtual hsBool      InterpretInputEvent( plInputEventMsg *pMsg );
        virtual void        MissedInputEvent( plInputEventMsg *pMsg );

        virtual hsBool      MsgReceive( plMessage *msg );

        virtual void        Init( plInputInterfaceMgr *manager );
        virtual void        Shutdown( void );

        virtual void        RestoreDefaultKeyMappings( void );
        virtual void        ClearKeyMap(); 
        
        // [dis/en]able mouse commands for avatar movement
        void SuspendMouseMovement();
        void EnableMouseMovement();
        void EnableJump(hsBool val);
        void EnableForwardMovement(hsBool val);
        void EnableControl(hsBool val, ControlEventCode code);
        void ClearLadderMode();
        void SetLadderMode();
        void ForceAlwaysRun(hsBool val);
        
        void    SetControlFlag(int f, hsBool val = true)            { fControlFlags.SetBit(f, val); }

        void    SetCursorFadeDelay( float delay ) { fCursorFadeDelay = delay; }

        hsBool  IsEnterChatModeBound();

        static plAvatarInputInterface   *GetInstance( void ) { return fInstance; }
};



#endif //plAvatarInputInterface_inc
