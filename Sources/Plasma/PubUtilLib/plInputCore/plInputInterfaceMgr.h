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
//  plInputInterfaceMgr.h - The manager of all input interface layers       //
//                                                                          //
//// History /////////////////////////////////////////////////////////////////
//                                                                          //
//  2.20.02 mcn - Created.                                                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _plInputInterfaceMgr_h
#define _plInputInterfaceMgr_h

#include "hsGeometry3.h"

#include <string_theory/string>
#include <utility>
#include <vector>

#include "pnModifier/plSingleModifier.h"
#include "pnInputCore/plKeyMap.h"

//// Class Definition ////////////////////////////////////////////////////////

class hsStream;
class hsResMgr;
class plInputInterface;
//class plKeyMap;
enum plKeyDef;
enum ControlEventCode;
class plKey;
class plCtrlCmd;
class plKeyCombo;
class plDefaultKeyCatcher;
class plKeyBinding;

class plInputInterfaceMgr : public plSingleModifier
{
    protected:

        static plInputInterfaceMgr  *fInstance;

        std::vector<plInputInterface *> fInterfaces;
        std::vector<plCtrlCmd *>        fMessageQueue;
        std::vector<plKey>              fReceivers;

        bool        fClickEnabled;
        int32_t       fCurrentCursor;
        float    fCursorOpacity;
        bool        fForceCursorHidden;
        int32_t       fForceCursorHiddenCount;
        plInputInterface        *fCurrentFocus;
        plDefaultKeyCatcher     *fDefaultCatcher;

        
        bool IEval(double secs, float del, uint32_t dirty) override;

        void    IAddInterface( plInputInterface *iface );
        void    IRemoveInterface( plInputInterface *iface );

        void    IUpdateCursor( int32_t newCursor );
        bool    ICheckCursor(plInputInterface *iFace); // returns true if the iface changed cursor settings
            
        void    IWriteConsoleCmdKeys( plKeyMap *keyMap, FILE *keyFile );
        void    IWriteNonConsoleCmdKeys( plKeyMap *keyMap, FILE *keyFile );

        plKeyMap    *IGetRoutedKeyMap( ControlEventCode code ); // Null for console commands
        void        IUnbind( const plKeyCombo &key );

        ST::string IKeyComboToString(const plKeyCombo &combo);
        
    public:

        plInputInterfaceMgr();
        virtual ~plInputInterfaceMgr();

        CLASSNAME_REGISTER( plInputInterfaceMgr );
        GETINTERFACE_ANY( plInputInterfaceMgr, plSingleModifier );

        bool    MsgReceive(plMessage *msg) override;
        void    Read(hsStream* s, hsResMgr* mgr) override;
        void    Write(hsStream* s, hsResMgr* mgr) override;

        void    Init();
        void    Shutdown();

        void        InitDefaultKeyMap();
        void        WriteKeyMap();
        void        RefreshInterfaceKeyMaps();

        void    SetCurrentFocus(plInputInterface *focus);
        void    ReleaseCurrentFocus(plInputInterface *focus);
        void    SetDefaultKeyCatcher( plDefaultKeyCatcher *c ) { fDefaultCatcher = c; }

        bool    IsClickEnabled() { return fClickEnabled; }

        void    ForceCursorHidden( bool requestedState );

        // Binding routers
        void    BindAction( const plKeyCombo &key, ControlEventCode code );
        void    BindAction( const plKeyCombo &key1, const plKeyCombo &key2, ControlEventCode code );
        void    BindConsoleCmd(const plKeyCombo &key, const ST::string& cmd, plKeyMap::BindPref pref = plKeyMap::kNoPreference);

        const plKeyBinding* FindBinding( ControlEventCode code );
        const plKeyBinding* FindBindingByConsoleCmd(const ST::string& cmd);

        void    ClearAllKeyMaps();
        void    ResetClickableState();
        static plInputInterfaceMgr  *GetInstance() { return fInstance; }
};

//// plCtrlCmd ///////////////////////////////////////////////////////////////
//  Networkable helper class that represents a single control statement

class plCtrlCmd
{
    private:
        ST::string          fCmd;
        plInputInterface    *fSource;

    public:
        plCtrlCmd(plInputInterface* source)
            : fCmd(), fPct(1.f), fSource(source),
              fControlCode(), fControlActivated(), fNetPropagateToPlayers()
        { }

        ST::string GetCmdString() { return fCmd; }
        void SetCmdString(ST::string cs) { fCmd = std::move(cs); }

        ControlEventCode    fControlCode;
        bool                fControlActivated;
        hsPoint3            fPt;
        float            fPct;

        bool                fNetPropagateToPlayers;

        void Read( hsStream* s, hsResMgr* mgr );
        void Write( hsStream* s, hsResMgr* mgr );

        plInputInterface    *GetSource() const { return fSource; }
};

//// Tiny Virtual Class For The Default Key Processor ////////////////////////
//
//  Basically, if you want to be the one to catch the leftover key events,
//  derive from this class and pass yourself to inputIFaceMgr.
//  (it'll auto-tell inputIFaceMgr when it goes away)
//
//  Note: if you want to do more than just get the darned key event (like
//  mouse events or key bindings or change the cursor or the like), don't do
//  this; create your own plInputInterface instead.

class plKeyEventMsg;
class plDefaultKeyCatcher
{
    public:
        virtual ~plDefaultKeyCatcher();
        virtual void    HandleKeyEvent( plKeyEventMsg *eventMsg ) = 0;
};


#endif // _plInputInterfaceMgr_h
