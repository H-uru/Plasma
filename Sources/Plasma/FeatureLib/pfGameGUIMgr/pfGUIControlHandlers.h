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
//  pfGUIControlHandlers Header                                             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfGUIControlHandlers_h
#define _pfGUIControlHandlers_h

#include "HeadSpin.h"

class pfGUIControlMod;
class hsStream;

//// pfGUICtrlProcObject Definition //////////////////////////////////////////
//  Any control which "does something" (buttons, edit boxes on Enter/Return, 
//  etc) uses this in some form. Basically, each control will have an (optional?)
//  pointer to an object derived from this class type. The class has a single
//  standard, virtual function that gets called on the "do something" event.
//  Derive from this class, override the virtual and set the control's handler
//  to your object and you're all set. Kinda like windowProcs wrapped in a
//  C++ object.
//  Note: there are some predefined objects for simple, common events. See
//  below.
//  Note the second: DoSomething() takes a parameter--namely, a pointer to
//  the control that called it. Thus, you can make one object handle
//  several controls by just switch()ing on that parameter and save yourself
//  some object creation. 
//
//  UserCallback() is an additional function for use in communicating between
//  procs. Basically, if you want another proc to do something (another dialog),
//  and want you to get called once it's done, set the callback on the other
//  proc/whatever to you and override UserCallback().
//
//  HandleExtendedEvent() is similar to DoSomething(), but is called for extended
//  event types, such as value changing (for an edit control while typing) or
//  list scrolled. The event parameter is control-type-specific.
//
//  Dialogs will use a similar functionality, but with more functions available.

class pfGUICtrlProcObject
{
    protected:

        uint32_t      fRefCnt;

    public:

        pfGUICtrlProcObject() { fRefCnt = 0; }
        virtual ~pfGUICtrlProcObject() { }

        virtual void    DoSomething( pfGUIControlMod *ctrl ) = 0;

        virtual void    HandleExtendedEvent( pfGUIControlMod *ctrl, uint32_t event ) { }

        virtual void    UserCallback( uint32_t userValue ) { }

        // ONLY THE GUI SYSTEM SHOULD CALL THESE
        void    IncRef() { fRefCnt++; }
        bool    DecRef() { fRefCnt--; return ( fRefCnt > 0 ) ? false : true; }
};

//// pfGUICtrlProcWriteableObject ////////////////////////////////////////////
//  This one is a read/writeable version of the above. Basically, you can just
//  call Read/Write() and it'll do all the mini-functionality of the factory
//  stuff so you don't have to worry about the derived type at runtime. Do
//  NOT derive from this class for your own handlers; this is just for the
//  handfull that will get added on export.

class pfGUICtrlProcWriteableObject : public pfGUICtrlProcObject
{
    protected:

        uint32_t          fType;

        virtual void    IRead( hsStream *s ) = 0;
        virtual void    IWrite( hsStream *s ) = 0;

    public:

        enum Types
        {
            kNull,
            kConsoleCmd,
            kPythonScript,
            kCloseDlg
        };

        pfGUICtrlProcWriteableObject() { fType = kNull; }
        pfGUICtrlProcWriteableObject( uint32_t type ) : fType( type ) { }
        virtual ~pfGUICtrlProcWriteableObject() { }

        void    DoSomething(pfGUIControlMod *ctrl) override = 0;

        static void Write( pfGUICtrlProcWriteableObject *obj, hsStream *s );

        static pfGUICtrlProcWriteableObject *Read( hsStream *s );
};

//// pfGUIConsoleCmdProc /////////////////////////////////////////////////////
//  Simply runs the console command specified. Exportable.

class pfGUIConsoleCmdProc : public pfGUICtrlProcWriteableObject
{
    protected:

        ST::string fCommand;

        void    IRead(hsStream *s) override;
        void    IWrite(hsStream *s) override;
    
    public:

        pfGUIConsoleCmdProc(ST::string cmd = {});

        void    DoSomething(pfGUIControlMod *ctrl) override;

        void    SetCommand(ST::string cmd) { fCommand = std::move(cmd); }
};

//// pfGUIPythonScriptProc ///////////////////////////////////////////////////

class pfGUIPythonScriptProc : public pfGUICtrlProcWriteableObject
{
    protected:

        void    IRead(hsStream *s) override;
        void    IWrite(hsStream *s) override;
    
    public:

        pfGUIPythonScriptProc();
        virtual ~pfGUIPythonScriptProc();

        void    DoSomething(pfGUIControlMod *ctrl) override;
};

//// Simple Runtime Ones /////////////////////////////////////////////////////

class pfGUICloseDlgProc : public pfGUICtrlProcWriteableObject
{
    protected:

        void    IRead(hsStream *s) override { }
        void    IWrite(hsStream *s) override { }

    public:

        pfGUICloseDlgProc() : pfGUICtrlProcWriteableObject( kCloseDlg ) {}
        virtual ~pfGUICloseDlgProc() {}

        void    DoSomething(pfGUIControlMod *ctrl) override;
};

#endif // _pfGUIControlHandlers_h
