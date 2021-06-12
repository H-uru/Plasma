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
#ifndef PLSEEKPOINTMOD_INC
#define PLSEEKPOINTMOD_INC

#include "pnModifier/plMultiModifier.h"

// PLSEEKPOINTMOD
// This modifier is something the avatar knows how to go to. (you know, seek)
// It's kind of like a magnet that, when activated, draws the avatar...
// Seen another way, it's a point with a name and a type....
class plSeekPointMod : public plMultiModifier
{
protected:
    bool IEval(double secs, float del, uint32_t dirty) override { return true; }
    ST::string fName;

public:

    plSeekPointMod();
    plSeekPointMod(ST::string name);

    const char * GetName() { return fName.c_str(); };
    void SetName(ST::string name) { fName = std::move(name); };

    CLASSNAME_REGISTER( plSeekPointMod );
    GETINTERFACE_ANY( plSeekPointMod, plMultiModifier );
    
    void AddTarget(plSceneObject* so) override;

    bool MsgReceive(plMessage* msg) override;

    void Read(hsStream *stream, hsResMgr *mgr) override;
    void Write(hsStream *stream, hsResMgr *mgr) override;
};

#endif
