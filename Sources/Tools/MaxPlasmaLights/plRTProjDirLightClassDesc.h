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
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  plRTProjDirLight.h - Header for the derived MAX RT projected directional //
//                       light                                               //
//  Cyan, Inc.                                                               //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  8.2.2001 mcn - Created.                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plRTProjDirLightClassDesc_h
#define _plRTProjDirLightClassDesc_h

#include "plRTProjDirLight.h"

class plRTProjDirLightDesc : public plMaxClassDesc<ClassDesc2>
{
    public:
        int             IsPublic() override             { return TRUE; }
        void*           Create(BOOL loading) override   { return new plRTProjDirLight; }
        const TCHAR*    ClassName() override            { return GetString(IDS_DB_PROJDIR); }
        SClass_ID       SuperClassID() override         { return LIGHT_CLASS_ID; }
        Class_ID        ClassID() override              { return RTPDIR_LIGHT_CLASSID; }
        const TCHAR*    Category() override             { return _T("Plasma RunTime");}
        const TCHAR*    InternalName() override         { return _T("RTProjDir"); } // returns fixed parsable name (scripter-visible name)
        HINSTANCE       HInstance() override            { return hInstance; }

        static plRTProjDirLightDesc fStaticDesc;

        static ClassDesc2   *GetDesc()        { return &fStaticDesc; }
};

#endif  // _plRTProjDirLightClassDesc_h