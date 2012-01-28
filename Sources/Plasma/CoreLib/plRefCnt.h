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
#ifndef plRefCnt_Defined
#define plRefCnt_Defined

#include "HeadSpin.h"

// plRef count addes refcount abilities to any plCreatable

class plRefCnt
{   uint32_t fRefCnt;
public:
    plRefCnt() : fRefCnt(1){}
    ~plRefCnt(){}
    hsBool TimeToDelete()   { return (fRefCnt == 1); }
    void    Incr()          { fRefCnt++; }
    void    Decr()          { fRefCnt--; }
};


#define DEFINE_REF_COUNT  plRefCnt fMyRef;\
    virtual void    UnRef() {   /*hsDebugCode(hsThrowIfFalse(fRefCnt >= 1);)*/if (fMyRef.TimeToDelete()) delete this; else fMyRef.Decr(); }\
    virtual void    Ref() { fMyRef.Incr(); }
/*
class hsRefCnt {
private:
    int32_t       fRefCnt;
public:
                hsRefCnt() : fRefCnt(1) {}
    virtual     ~hsRefCnt();

    int32_t       RefCnt() const { return fRefCnt; }
    virtual void    UnRef();
    virtual void    Ref();
};

#define hsRefCnt_SafeRef(obj)       do { if (obj) (obj)->Ref(); } while (0)
#define hsRefCnt_SafeUnRef(obj) do { if (obj) (obj)->UnRef(); } while (0)

#define hsRefCnt_SafeAssign(dst, src)       \
        do {                            \
            hsRefCnt_SafeRef(src);      \
            hsRefCnt_SafeUnRef(dst);        \
            dst = src;                  \
        } while (0)

*/

#endif

