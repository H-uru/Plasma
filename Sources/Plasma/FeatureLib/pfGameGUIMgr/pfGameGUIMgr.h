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
//  pfGameGUIMgr Header                                                     //
//  A.K.A. "Ooh, we get a GUI!"                                             //
//                                                                          //
//// Description /////////////////////////////////////////////////////////////
//                                                                          //
//  The in-game GUI manager. Handles reading, creation, and input for       //
//  dialog boxes at runtime.                                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfGameGUIMgr_h
#define _pfGameGUIMgr_h

#include "HeadSpin.h"

#include "pnInputCore/plKeyDef.h"
#include "pnKeyedObject/plKey.h"
#include "pnKeyedObject/hsKeyedObject.h"

#include <string_theory/string>
#include <vector>

class pfGameUIInputInterface;
class pfGUIDialogMod;
class pfGUIControlMod;
class plMessage;
class plPipeline;
class plPostEffectMod;

//// Tag Definitions /////////////////////////////////////////////////////////
//  Each dialog/control gets an optional tag ID number. This is the link
//  between MAX and C++. You attach a Tag component to a control or dialog
//  in MAX and assign it an ID (supplied by a list of konstants that are
//  hard-coded). Then, in code, you ask the gameGUIMgr for the dialog (or
//  control) with that ID, and pop, you get it back. Then you run with it.
//
//  Easy, huh?

class pfGUITag
{
    public:
        uint32_t   fID;
        ST::string fName;
};


//
// This class just holds a name and the key to set the receiver to
// after the dialog gets loaded.
class pfDialogNameSetKey
{
private:
    ST::string fName;
    plKey      fKey;

public:
    pfDialogNameSetKey(ST::string name, plKey key)
        : fName(std::move(name)), fKey(std::move(key)) { }

    ST::string GetName() const { return fName; }
    plKey GetKey() const { return fKey; }
};

//// Manager Class Definition ////////////////////////////////////////////////

class pfGUIPopUpMenu;
class pfGameGUIMgr : public hsKeyedObject
{
    friend class pfGameUIInputInterface;

    public:

        enum EventType
        {
            kMouseDown,
            kMouseUp,
            kMouseMove,
            kMouseDrag,
            kKeyDown,
            kKeyUp,
            kKeyRepeat,
            kMouseDblClick
        };

        enum
        {
            kNoModifiers = 0,
            kShiftDown  = 0x01,
            kCtrlDown   = 0x02,
            kCapsDown   = 0x04
        };

    private:

        static pfGameGUIMgr *fInstance;

    protected:

        std::vector<pfGUIDialogMod *>  fDialogs;
        pfGUIDialogMod              *fActiveDialogs;

        bool    fActivated;
        uint32_t  fActiveDlgCount;

        pfGameUIInputInterface      *fInputConfig;
        uint32_t                      fInputCtlIndex;

        uint32_t                      fDefaultCursor;
        float                    fCursorOpacity;
        float                fAspectRatio;

        // This is an array of the dialogs (by name) that need their
        // receiver key set once they are loaded.
        // This array shouldn't get more than one entry... but
        // it could be more....
        // LoadDialog adds an entry and MsgReceive removes it
        std::vector<pfDialogNameSetKey *>  fDialogToSetKeyOf;

        void    IShowDialog( const ST::string& name );
        void    IHideDialog( const ST::string& name );

        void    IAddDlgToList( hsKeyedObject *obj );
        void    IRemoveDlgFromList( hsKeyedObject *obj );

        void    IActivateGUI( bool activate );

        bool    IHandleMouse( EventType event, float mouseX, float mouseY, uint8_t modifiers, uint32_t *desiredCursor );
        bool    IHandleKeyEvt( EventType event, plKeyDef key, uint8_t modifiers );
        bool    IHandleKeyPress( wchar_t key, uint8_t modifiers );

        bool    IModalBlocking();

        pfGUIDialogMod  *IGetTopModal() const;

    public:

        enum
        {
            kDlgModRef = 0
        };


        pfGameGUIMgr();
        ~pfGameGUIMgr();

        CLASSNAME_REGISTER( pfGameGUIMgr );
        GETINTERFACE_ANY( pfGameGUIMgr, hsKeyedObject );

        void        Draw( plPipeline *p );

        bool        Init();

        bool    MsgReceive(plMessage* pMsg) override;

        void    LoadDialog(const ST::string& name, plKey recvrKey = {}, const char *ageName = nullptr);  // AgeName = nil defaults to "GUI"
        void    ShowDialog( const ST::string& name ) { IShowDialog(name); }
        void    HideDialog( const ST::string&  name ) { IHideDialog(name); }
        void    UnloadDialog( const ST::string& name );
        void    UnloadDialog( pfGUIDialogMod *dlg );

        void    ShowDialog( pfGUIDialogMod *dlg, bool resetClickables=true );
        void    HideDialog( pfGUIDialogMod *dlg );

        bool    IsDialogLoaded( const ST::string& name );
        pfGUIDialogMod *GetDialogFromString( const ST::string& name );

        void    SetDialogToNotify(const ST::string& name, plKey recvrKey);
        void    SetDialogToNotify(pfGUIDialogMod *dlg, plKey recvrKey);

        void    SetDefaultCursor(uint32_t defaultCursor) { fDefaultCursor = defaultCursor; }
        uint32_t  GetDefaultCursor() { return fDefaultCursor; }
        void    SetCursorOpacity(float opacity) { fCursorOpacity = opacity; }
        float    GetCursorOpacity() { return fCursorOpacity; }

        pfGUIPopUpMenu  *FindPopUpMenu( const ST::string& name );

        std::vector<plPostEffectMod*> GetDlgRenderMods() const;
        bool    IsModalBlocking() {return IModalBlocking();}

        // Tag ID stuff
        pfGUIDialogMod  *GetDialogFromTag( uint32_t tagID );
        pfGUIControlMod *GetControlFromTag( pfGUIDialogMod *dlg, uint32_t tagID );

        static uint32_t       GetNumTags();
        static pfGUITag     *GetTag( uint32_t tagIndex );
        static uint32_t       GetHighestTag();
        void SetAspectRatio(float aspectratio);
        float GetAspectRatio() { return fAspectRatio; }
 
        static pfGameGUIMgr *GetInstance() { return fInstance; }
};

#endif //_pfGameGUIMgr_h

