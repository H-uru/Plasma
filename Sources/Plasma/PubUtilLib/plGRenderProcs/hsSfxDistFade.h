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

#ifndef hsSfxDistFade_inc
#define hsSfxDistFade_inc

#include "hsBiExpander.h"
#include "hsGRenderProcs.h"

class hsSfxDistFade : public hsGRenderProcs {
public:
    enum {
        kCullsBefore        = 0x10000,
        kCullsBeyond        = 0x20000,

        kDistFromView       = 0x40000,
        kDistFromTarget     = 0x80000,
        kDistAlongX         = 0x100000,
        kDistAlongY         = 0x200000,
        kDistAlongZ         = 0x400000,
        kWorldDist          = 0x800000,

        kPostInterp         = 0x1000000,

        kIdleBefore         = 0x2000000,
        kIdleBeyond         = 0x4000000
    };

    struct hsSfxDfTableEntry {
        hsScalar            fDistDel;
        hsScalar            fDistNorm;
        hsScalar            fOpacity;
    };
protected:

    hsScalar                            fMinDist;
    hsScalar                            fMaxDist;

    hsScalar                            fMinIdle;
    hsScalar                            fMaxIdle;

    hsExpander<hsSfxDfTableEntry>       fTable;

    hsScalar IOpacFromDist(hsScalar dist);
public:
    hsSfxDistFade();
    virtual ~hsSfxDistFade();

    virtual hsBool32 BeginObject(plPipeline* pipe, plDrawable* obj);

    virtual void ProcessPreInterpShadeVerts(hsExpander<hsGShadeVertex*>& vList);
    virtual void ProcessPostInterpShadeVerts(hsExpander<hsGShadeVertex*>& vList);

    void MakeTable(float* distList, float* opacList, int num); // lists sorted from lowest cosine to highest

    virtual void Read(hsStream* s);
    virtual void Write(hsStream* s);

    virtual const char* GetLabel() const { return "hsSfxDistFade"; }

    virtual ProcType GetType() const { return kTypeDistFade; }

    CLASSNAME_REGISTER( hsSfxDistFade );
    GETINTERFACE_ANY( hsSfxDistFade, hsGRenderProcs );

};

#endif // hsSfxDistFade_inc
