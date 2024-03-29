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
//
//  plUoid - A Unique Object IDentifier -- basically, each unique Uoid refers
//           to one and exactly one object.
//           To define such, it contains three elements:
//               - A plLocation, which specifies an (age,chapter,page) combo
//                 (as a sequence number)
//               - A creatable class type (from plFactory)
//               - An object name
//
//////////////////////////////////////////////////////////////////////////////

#ifndef plUoid_h_inc
#define plUoid_h_inc

#include "HeadSpin.h"
#include "plFixedKey.h"
#include "plLoadMask.h"
#include <string_theory/formatter>

class hsStream;

//// plLocation //////////////////////////////////////////////////////////////

class plLocation
{
public:
    enum LocFlags
    {
        kLocalOnly  = 0x1,  // Set if nothing in the room saves state.
        kVolatile   = 0x2,  // Set is nothing in the room persists when the server exits.
        kReserved   = 0x4,
        kBuiltIn    = 0x8,
        kItinerant  = 0x10,
    };

protected:
    uint32_t fSequenceNumber;
    uint16_t fFlags;

    enum : uint32_t
    {
        kGlobalFixedLocIdx = 0,     // Fixed keys go here, think of as "global,fixed,keys"
        kSceneViewerLocIdx = 1,

        kLocalLocStartIdx = 3,      // These are a range of #s that go to local, testing-only pages.
        kLocalLocEndIdx = 32,       // You can't go over the network with any keys with these locs.

        kNormalLocStartIdx = kLocalLocEndIdx + 1,

        kReservedLocStart = 0xff000000, // Reserved locations are ones that aren't real game locations,
        kGlobalServerLocIdx = kReservedLocStart,    // Global pool room for the server. Only the server gets this one

        kReservedLocAvailableStart = kGlobalServerLocIdx + 1,   // This is the start of the *really* available ones
        kReservedLocEnd = 0xfffffffe,   // But instead act as a holding place for data

        kInvalidLocIdx = 0xffffffff
    };

    plLocation(uint32_t seqNum, uint16_t flags=0) : fFlags(flags) { Set(seqNum); }

public:
    plLocation() { Invalidate(); }
    plLocation(const plLocation& toCopyFrom);
    ~plLocation() {}

    void      Invalidate();
    bool      IsValid() const;
    bool      IsReserved() const;
    bool      IsItinerant() const;
    void      Set(uint32_t seqNum);
    uint32_t  GetSequenceNumber() const { return fSequenceNumber; }
    bool      IsVirtual() const;

    void      SetFlags(uint16_t flags) { fFlags |= flags; }
    uint16_t  GetFlags() const { return fFlags; }

    void    Read(hsStream* s);
    void    Write(hsStream* s) const;

    bool operator==(const plLocation& loc) const
    {
        // Ignore the itinerant flag when comparing, because
        return (fSequenceNumber == loc.fSequenceNumber) &&
               ((fFlags & ~kItinerant) == (loc.fFlags & ~kItinerant));
    }
    bool operator!=(const plLocation& loc) const { return !(loc == *this); }
    plLocation& operator=(const plLocation& loc);
    bool operator<(const plLocation& loc ) const { return fSequenceNumber < loc.fSequenceNumber; }

    // THIS SHOULD BE FOR DEBUGGING ONLY <hint hint>
    ST::string StringIze() const;  // Format to displayable string.

    static plLocation MakeReserved(uint32_t number);
    static plLocation MakeNormal(uint32_t number);

    static const plLocation kGlobalFixedLoc;
    static const plLocation kSceneViewerLoc;
    static const plLocation kLocalStartLoc;
    static const plLocation kLocalEndLoc;
    static const plLocation kNormalStartLoc;
    static const plLocation kGlobalServerLoc;
    static const plLocation kInvalidLoc;
};

inline ST_FORMAT_TYPE(const plLocation &)
{
    ST_FORMAT_FORWARD(value.StringIze());
}

//// plUoid //////////////////////////////////////////////////////////////////

class plUoid
{
public:
    plUoid() { Invalidate(); }
    plUoid(const plLocation& location, uint16_t classType, const ST::string& objectName, const plLoadMask& m=plLoadMask::kAlways);
    plUoid(plFixedKeyId fixedKey);
    plUoid(const plUoid& copy) = default;
    plUoid(plUoid&& move) = default;
    ~plUoid();

    const plLocation&   GetLocation() const { return fLocation; }
    uint16_t            GetClassType() const { return fClassType; }
    ST::string          GetObjectName() const { return fObjectName; }
    const plLoadMask&   GetLoadMask() const { return fLoadMask; }

    void Read(hsStream* s);
    void Write(hsStream* s) const;

    void Invalidate();
    bool IsValid() const;

    plUoid& operator=(const plUoid& copy) = default;
    plUoid& operator=(plUoid&& move) = default;
    bool  operator==(const plUoid& u) const;
    bool  operator!=(const plUoid& u) const { return !operator==(u); }

    bool  IsClone() const             { return fCloneID != 0; }
    uint32_t  GetClonePlayerID() const    { return fClonePlayerID; }
    uint32_t  GetCloneID() const          { return fCloneID; }
    void    SetClone(uint32_t playerID, uint32_t cloneID) { hsAssert(cloneID < 0xffff, "Clone id too high"); fCloneID = uint16_t(cloneID); fClonePlayerID = playerID; }

    uint32_t GetObjectID() const { return fObjectID; }
    // Export time only.  Only plRegistryKeyList should call this.
    void SetObjectID(uint32_t id) { fObjectID = id; }

    // THIS SHOULD BE FOR DEBUGGING ONLY <hint hint>
    ST::string StringIze() const;  // Format to displayable string

protected:
    enum ContentsFlags  // for read/write functions
    {
        kHasCloneIDs    = 0x1,
        kHasLoadMask    = 0x2,
    };

    uint32_t    fObjectID;
    uint32_t    fClonePlayerID; // The ID of the player who made this clone
    uint16_t    fCloneID;       // The ID of this clone (unique per client)
    uint16_t    fClassType;
    ST::string  fObjectName;
    plLocation  fLocation;
    plLoadMask  fLoadMask;
};

inline ST_FORMAT_TYPE(const plUoid &)
{
    ST_FORMAT_FORWARD(value.StringIze());
}

#endif // plUoid_h_inc
