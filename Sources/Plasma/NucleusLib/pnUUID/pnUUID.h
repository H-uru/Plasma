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
#ifndef pnUUID_h_inc
#define pnUUID_h_inc

#include "HeadSpin.h"
#include <string_theory/formatter>

class hsStream;

extern const class plUUID kNilUuid;

class plUUID
{
    // must be first field in class
public:
    uint8_t   fData[16];
    struct Match
    {
        const plUUID* fGuid;
        Match(const plUUID* guid) : fGuid(guid) {}
        bool operator()(const plUUID* guid) const { return guid->IsEqualTo(fGuid); }
    };

    plUUID();
    plUUID(const char* s);
    plUUID(const ST::string& s);
    plUUID(const plUUID&) = default;

    plUUID& operator=(const plUUID&) = default;

    void     Clear();
    bool     IsNull() const;
    bool     IsSet() const { return !IsNull(); }
    void     CopyFrom(const plUUID* v);
    void     CopyFrom(const plUUID& v);
    int      CompareTo(const plUUID* v) const;
    bool     IsEqualTo(const plUUID* v) const;
    bool     FromString(const char* str);
    bool     ToString(ST::string& out) const;
    ST::string AsString() const;
    void     Read(hsStream* s);
    void     Write(hsStream* s);

    operator bool () const { return !IsNull(); }
    inline bool operator ! () const { return IsNull(); }

    bool operator==(const plUUID& other) const {
        return IsEqualTo(&other);
    }
    bool operator!=(const plUUID& other) const {
        return !IsEqualTo(&other);
    }
    bool operator<(const plUUID& other) const {
        return CompareTo(&other) == -1;
    }
    operator ST::string () const { return AsString(); }

    static plUUID Generate();
};

inline ST_FORMAT_TYPE(const plUUID &)
{
    ST_FORMAT_FORWARD(value.AsString());
}

#endif // pnUUID_h_inc
