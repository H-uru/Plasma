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
#ifndef hsGMatState_inc
#define hsGMatState_inc

#include "hsColorRGBA.h"

class hsStream;

class hsGMatState {
public:
    #define GMAT_STATE_ENUM_START(name)       enum name {
    #define GMAT_STATE_ENUM_VALUE(name, val)    name = val,
    #define GMAT_STATE_ENUM_END(name)         };

    #include "hsGMatStateEnums.h"

    #undef GMAT_STATE_ENUM_START
    #undef GMAT_STATE_ENUM_VALUE
    #undef GMAT_STATE_ENUM_END

    enum StateIdx {
        kBlend,
        kClamp,
        kShade,
        kZ,
        kMisc
    };

    uint32_t          fBlendFlags;
    uint32_t          fClampFlags;
    uint32_t          fShadeFlags;
    uint32_t          fZFlags;
    uint32_t          fMiscFlags;

    static bool Differs(uint32_t mine, uint32_t hers, uint32_t mask)
    {
        return (mine & mask) ^ (hers & mask);
    }

    static bool Differs(uint32_t mine, uint32_t hers)
    {
        return mine ^ hers;
    }

    bool operator!=(const hsGMatState& other)
    {
        return ((fBlendFlags ^ other.fBlendFlags)
            | (fClampFlags ^ other.fClampFlags)
            | (fShadeFlags ^ other.fShadeFlags)
            | (fZFlags ^ other.fZFlags)
            | (fMiscFlags ^ other.fMiscFlags));
    }
    uint32_t Value(int i) const
    {
        switch(i)
        {
        case kBlend:
            return fBlendFlags;
        case kClamp:
            return fClampFlags;
        case kShade:
            return fShadeFlags;
        case kZ:
            return fZFlags;
        case kMisc:
            return fMiscFlags;
        }
        hsAssert(false, "Bad param");
        return fBlendFlags;
    }
    uint32_t& operator[](const int i)
    {
        switch(i)
        {
        case kBlend:
            return fBlendFlags;
        case kClamp:
            return fClampFlags;
        case kShade:
            return fShadeFlags;
        case kZ:
            return fZFlags;
        case kMisc:
            return fMiscFlags;
        }
        hsAssert(false, "Bad param");
        return fBlendFlags;
    }
    hsGMatState& operator|=(const hsGMatState& other)
    {
        fBlendFlags |= other.fBlendFlags;
        fClampFlags |= other.fClampFlags;
        fShadeFlags |= other.fShadeFlags;
        fZFlags |= other.fZFlags;
        fMiscFlags |= other.fMiscFlags;
        return *this;
    }
    hsGMatState& operator+=(const hsGMatState& other)
    {
        return operator|=(other);
    }
    hsGMatState& operator-=(const hsGMatState& other)
    {
        fBlendFlags &= ~other.fBlendFlags;
        fClampFlags &= ~other.fClampFlags;
        fShadeFlags &= ~other.fShadeFlags;
        fZFlags &= ~other.fZFlags;
        fMiscFlags &= ~other.fMiscFlags;
        return *this;
    }

    inline void Read(hsStream* s);
    inline void Write(hsStream* s);

    hsGMatState(uint32_t blend=0, uint32_t clamp=0, uint32_t shade=0, uint32_t z=0, uint32_t misc=0) 
        :   fBlendFlags(blend), 
            fClampFlags(clamp),
            fShadeFlags(shade),
            fZFlags(z),
            fMiscFlags(misc) {}
    void Reset() { fBlendFlags = fClampFlags = fShadeFlags = fZFlags = fMiscFlags = 0; }
    inline void Clear(const hsGMatState& state);
    inline void Composite(const hsGMatState& want, const hsGMatState& on, const hsGMatState& off);
};

#endif // hsGMatState_inc
