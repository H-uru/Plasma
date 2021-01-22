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
//  pfGUIClickMapCtrl Definition                                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "pfGUIClickMapCtrl.h"

#include "plInputCore/plInputInterface.h"

//// Constructor/Destructor //////////////////////////////////////////////////

pfGUIClickMapCtrl::pfGUIClickMapCtrl()
{
    fTracking = false;
    fCustomCursor = -1;
}

pfGUIClickMapCtrl::~pfGUIClickMapCtrl()
{
}

//// IEval ///////////////////////////////////////////////////////////////////

bool    pfGUIClickMapCtrl::IEval( double secs, float del, uint32_t dirty )
{
    return pfGUIControlMod::IEval( secs, del, dirty );
}

//// MsgReceive //////////////////////////////////////////////////////////////

bool    pfGUIClickMapCtrl::MsgReceive( plMessage *msg )
{
    return pfGUIControlMod::MsgReceive( msg );
}

//// Read/Write //////////////////////////////////////////////////////////////

void    pfGUIClickMapCtrl::Read( hsStream *s, hsResMgr *mgr )
{
    pfGUIControlMod::Read(s, mgr);
}

void    pfGUIClickMapCtrl::Write( hsStream *s, hsResMgr *mgr )
{
    pfGUIControlMod::Write( s, mgr );
}

void    pfGUIClickMapCtrl::HandleMouseDown( hsPoint3 &mousePt, uint8_t modifiers )
{
    IScreenToLocalPt( mousePt );
    fLastMousePt = fLastMouseDragPt = mousePt;
    fTracking = true;
}

void    pfGUIClickMapCtrl::HandleMouseUp( hsPoint3 &mousePt, uint8_t modifiers )
{
    if( fTracking )
    {
        IScreenToLocalPt( mousePt );
        fLastMousePt = fLastMouseUpPt = fLastMouseDragPt = mousePt;
        DoSomething();
        fTracking = false;
    }
}

void    pfGUIClickMapCtrl::HandleMouseDrag( hsPoint3 &mousePt, uint8_t modifiers )
{
    if( fTracking )
    {
        IScreenToLocalPt( mousePt );
        fLastMousePt = fLastMouseDragPt = mousePt;
        if( HasFlag( kReportDragging ) )
            HandleExtendedEvent( kMouseDragged );
    }
}

void    pfGUIClickMapCtrl::HandleMouseHover( hsPoint3 &mousePt, uint8_t modifiers )
{
    IScreenToLocalPt( mousePt );
    fLastMousePt = mousePt;

    if( HasFlag( kReportHovering ) )
        HandleExtendedEvent( kMouseHovered );
}

//// IGetDesiredCursor ///////////////////////////////////////////////////////

uint32_t      pfGUIClickMapCtrl::IGetDesiredCursor() const
{
    if( fCustomCursor != -1 )
        return (uint32_t)fCustomCursor;

    return plInputInterface::kCursorPoised;
}

