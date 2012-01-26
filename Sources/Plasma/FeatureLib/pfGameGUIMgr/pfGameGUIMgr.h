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
#include "hsTemplates.h"
#include "pnInputCore/plKeyDef.h"
#include "pnKeyedObject/hsKeyedObject.h"
#include <vector>

class plPipeline;
class plMessage;
class pfGUIDialogMod;
class pfGUIControlMod;
class pfGameUIInputInterface;
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
        uint32_t  fID;
        char    fName[ 128 ];
};


//
// This class just holds a name and the key to set the receiver to
// after the dialog gets loaded.
class pfDialogNameSetKey
{
private:
    char    *fName;
    plKey   fKey;
public:
    pfDialogNameSetKey(const char *name, plKey key) { fName = hsStrcpy(name); fKey=key; }
    ~pfDialogNameSetKey() { delete [] fName; }
    const char *GetName() { return fName; }
    plKey GetKey() { return fKey; }
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

        hsTArray<pfGUIDialogMod *>  fDialogs;
        pfGUIDialogMod              *fActiveDialogs;

        // These two lists help us manage when dialogs get told to load or unload versus when they actually *do*
        hsTArray<pfDialogNameSetKey *>  fDlgsPendingLoad;
        hsTArray<pfDialogNameSetKey *>  fDlgsPendingUnload;

        hsBool  fActivated;
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
        hsTArray<pfDialogNameSetKey *>  fDialogToSetKeyOf;

        void    ILoadDialog( const char *name );
        void    IShowDialog( const char *name );
        void    IHideDialog( const char *name );

        void    IAddDlgToList( hsKeyedObject *obj );
        void    IRemoveDlgFromList( hsKeyedObject *obj );

        void    IActivateGUI( hsBool activate );

        hsBool  IHandleMouse( EventType event, float mouseX, float mouseY, uint8_t modifiers, uint32_t *desiredCursor );
        hsBool  IHandleKeyEvt( EventType event, plKeyDef key, uint8_t modifiers );
        hsBool  IHandleKeyPress( wchar_t key, uint8_t modifiers );

        hsBool  IModalBlocking( void );

        pfGUIDialogMod  *IGetTopModal( void ) const;

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

        hsBool      Init( void );

        virtual hsBool  MsgReceive( plMessage* pMsg );

        void    LoadDialog( const char *name, plKey recvrKey=nil, const char *ageName = nil );  // AgeName = nil defaults to "GUI"
        void    ShowDialog( const char *name ) { IShowDialog(name); }
        void    HideDialog( const char *name ) { IHideDialog(name); }
        void    UnloadDialog( const char *name );
        void    UnloadDialog( pfGUIDialogMod *dlg );

        void    ShowDialog( pfGUIDialogMod *dlg, bool resetClickables=true );
        void    HideDialog( pfGUIDialogMod *dlg );

        hsBool  IsDialogLoaded( const char *name );
        pfGUIDialogMod *GetDialogFromString( const char *name );

        void    SetDialogToNotify(const char *name, plKey recvrKey);
        void    SetDialogToNotify(pfGUIDialogMod *dlg, plKey recvrKey);

        void    SetDefaultCursor(uint32_t defaultCursor) { fDefaultCursor = defaultCursor; }
        uint32_t  GetDefaultCursor() { return fDefaultCursor; }
        void    SetCursorOpacity(float opacity) { fCursorOpacity = opacity; }
        float    GetCursorOpacity() { return fCursorOpacity; }

        pfGUIPopUpMenu  *FindPopUpMenu( const char *name );

        std::vector<plPostEffectMod*> GetDlgRenderMods( void ) const;
        hsBool  IsModalBlocking( void ) {return IModalBlocking();}

        // Tag ID stuff
        pfGUIDialogMod  *GetDialogFromTag( uint32_t tagID );
        pfGUIControlMod *GetControlFromTag( pfGUIDialogMod *dlg, uint32_t tagID );

        static uint32_t       GetNumTags( void );
        static pfGUITag     *GetTag( uint32_t tagIndex );
        static uint32_t       GetHighestTag( void );
        void SetAspectRatio(float aspectratio);
        float GetAspectRatio() { return fAspectRatio; }
 
        static pfGameGUIMgr *GetInstance( void ) { return fInstance; }
};

#endif //_pfGameGUIMgr_h

