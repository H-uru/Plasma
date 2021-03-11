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
//  pfGUIControl Handler Definitions                                        //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "pfGUIControlHandlers.h"

#include "HeadSpin.h"

#include "pfGUIControlMod.h"
#include "pfGUIDialogMod.h"

#include "plMessage/plConsoleMsg.h"

//// Writeable Stuff /////////////////////////////////////////////////////////

void    pfGUICtrlProcWriteableObject::Write( pfGUICtrlProcWriteableObject *obj, hsStream *s )
{
    if (obj != nullptr)
    {
        s->WriteLE32( obj->fType );
        obj->IWrite( s );
    }
    else
        s->WriteLE32((uint32_t)kNull);
}

pfGUICtrlProcWriteableObject *pfGUICtrlProcWriteableObject::Read( hsStream *s )
{
    pfGUICtrlProcWriteableObject    *obj;

    uint32_t type = s->ReadLE32();

    switch( type )
    {
        case kConsoleCmd:
            obj = new pfGUIConsoleCmdProc;
            break;

        case kPythonScript:
            obj = new pfGUIPythonScriptProc;
            break;

        case kCloseDlg:
            obj = new pfGUICloseDlgProc;
            break;

        case kNull:
            return nullptr;

        default:
            hsAssert( false, "Invalid proc type in Read()" );
            return nullptr;
    }

    obj->IRead( s );
    return obj;
}

//////////////////////////////////////////////////////////////////////////////
//// Predefined Exportables //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// pfGUIConsoleCmdProc /////////////////////////////////////////////////////

pfGUIConsoleCmdProc::pfGUIConsoleCmdProc() : pfGUICtrlProcWriteableObject( kConsoleCmd ) 
{ 
    fCommand = nullptr;
}

pfGUIConsoleCmdProc::pfGUIConsoleCmdProc( const char *cmd )
                : pfGUICtrlProcWriteableObject( kConsoleCmd ) 
{
    fCommand = nullptr;
    SetCommand( cmd );
}

pfGUIConsoleCmdProc::~pfGUIConsoleCmdProc()
{
    delete [] fCommand;
}

void    pfGUIConsoleCmdProc::IRead( hsStream *s )
{
    int i = s->ReadLE32();
    if( i > 0 )
    {
        fCommand = new char[ i + 1 ];
        memset( fCommand, 0, i + 1 );
        s->Read( i, fCommand );
    }
    else
        fCommand = nullptr;
}

void    pfGUIConsoleCmdProc::IWrite( hsStream *s )
{
    if (fCommand != nullptr)
    {
        s->WriteLE32( strlen( fCommand ) );
        s->Write( strlen( fCommand ), fCommand );
    }
    else
        s->WriteLE32( 0 );
}

void    pfGUIConsoleCmdProc::DoSomething( pfGUIControlMod *ctrl )
{
    if (fCommand != nullptr)
    {
        plConsoleMsg *cMsg = new plConsoleMsg( plConsoleMsg::kExecuteLine, fCommand );
        cMsg->Send();
    }
}

void    pfGUIConsoleCmdProc::SetCommand( const char *cmd )
{
    delete [] fCommand;

    if (cmd == nullptr)
        fCommand = nullptr;
    else
    {
        fCommand = new char[ strlen( cmd ) + 1 ];
        memset( fCommand, 0, strlen( cmd ) + 1 );
        strcpy( fCommand, cmd );
    }
}

//// pfGUIPythonScriptProc ///////////////////////////////////////////////////

pfGUIPythonScriptProc::pfGUIPythonScriptProc() : pfGUICtrlProcWriteableObject( kPythonScript ) 
{ 
}

pfGUIPythonScriptProc::~pfGUIPythonScriptProc()
{
}

void    pfGUIPythonScriptProc::IRead( hsStream *s )
{
}

void    pfGUIPythonScriptProc::IWrite( hsStream *s )
{
}

void    pfGUIPythonScriptProc::DoSomething( pfGUIControlMod *ctrl )
{
}

//////////////////////////////////////////////////////////////////////////////
//// Simple Runtime Ones /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void    pfGUICloseDlgProc::DoSomething( pfGUIControlMod *ctrl )
{
    ctrl->GetOwnerDlg()->Hide();
}
