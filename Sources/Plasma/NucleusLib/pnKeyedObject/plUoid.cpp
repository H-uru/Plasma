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
#include "plUoid.h"
#include "hsStream.h"


//// plLocation //////////////////////////////////////////////////////////////

const plLocation plLocation::kGlobalFixedLoc(plLocation::kGlobalFixedLocIdx);
const plLocation plLocation::kLocalStartLoc(plLocation::kLocalLocStartIdx);
const plLocation plLocation::kLocalEndLoc(plLocation::kLocalLocEndIdx);
const plLocation plLocation::kNormalStartLoc(plLocation::kNormalLocStartIdx);
const plLocation plLocation::kGlobalServerLoc(plLocation::kGlobalServerLocIdx, plLocation::kReserved);
const plLocation plLocation::kInvalidLoc;

plLocation::plLocation(const plLocation& toCopyFrom)
{
    *this = toCopyFrom;
}

void plLocation::Read(hsStream* s)
{
    s->LogReadLE(&fSequenceNumber, "Location Sequence Number");
    s->LogReadLE(&fFlags, "Location Flags");
}

void plLocation::Write(hsStream* s) const
{
    s->WriteLE(fSequenceNumber);
    s->WriteLE(fFlags);
}

plLocation& plLocation::operator=(const plLocation& rhs)
{
    fSequenceNumber = rhs.fSequenceNumber;
    fFlags = rhs.fFlags;
    return *this;
}

void plLocation::Set(uint32_t seqNum)
{
    fSequenceNumber = seqNum;
}

void plLocation::Invalidate()
{
    fSequenceNumber = kInvalidLocIdx;
    fFlags = 0; // Set to kInvalid?
}

bool plLocation::IsValid() const
{
    return (fSequenceNumber == kInvalidLocIdx) ? false : true;
}

bool plLocation::IsReserved() const
{
    return hsCheckBits(fFlags, kReserved);
}

bool plLocation::IsItinerant() const
{
    return hsCheckBits(fFlags, kItinerant);
}

bool plLocation::IsVirtual() const
{
    // This returns whether the location is "virtual", i.e. isn't a true room per se. Like fixed keys
    if (fSequenceNumber == kGlobalFixedLocIdx)
        return true;

    return false;
}

// THIS SHOULD BE FOR DEBUGGING ONLY <hint hint>
plString plLocation::StringIze()  const // Format to displayable string
{
    return plFormat("S0x{x}F0x{x}", fSequenceNumber, fFlags);
}

PL_FORMAT_IMPL(const plLocation &)
{
    return PL_FORMAT_FORWARD(format, value.StringIze());
}

plLocation plLocation::MakeReserved(uint32_t number)
{
    return plLocation(kReservedLocAvailableStart + number, kReserved);
}

plLocation plLocation::MakeNormal(uint32_t number)
{
    return plLocation(kNormalLocStartIdx + number);
}

//// plUoid //////////////////////////////////////////////////////////////////

plUoid::plUoid(const plLocation& location, uint16_t classType, const plString& objectName, const plLoadMask& m)
{
    Invalidate();

    fLocation = location;
    fClassType = classType;
    fObjectName = objectName;
    fLoadMask = m;
    fClonePlayerID = 0;
}

plUoid::plUoid(const plUoid& src)
{
    Invalidate();
    *this = src;
}

plUoid::~plUoid()
{
    Invalidate();
}

void plUoid::Read(hsStream* s)
{
    hsAssert(fObjectName.IsNull(), "Reading over an old uoid? You're just asking for trouble, aren't you?");

    // first read contents flags
    uint8_t contents = s->ReadByte();

    fLocation.Read(s);

    // conditional loadmask read
    if (contents & kHasLoadMask)
        fLoadMask.Read(s);
    else
        fLoadMask.SetAlways();

    s->LogReadLE(&fClassType, "ClassType");
    s->LogReadLE(&fObjectID, "ObjectID");
    s->LogSubStreamPushDesc("ObjectName");
    fObjectName = s->LogReadSafeString();

    // conditional cloneIDs read
    if (contents & kHasCloneIDs)
    {       
        s->LogReadLE( &fCloneID ,"CloneID");
        uint16_t dummy;
        s->LogReadLE(&dummy, "dummy"); // To avoid breaking format
        s->LogReadLE( &fClonePlayerID ,"ClonePlayerID");
    }
    else
    {
        fCloneID = 0;
        fClonePlayerID = 0;
    }
}

void plUoid::Write(hsStream* s) const
{
    // first write contents byte
    uint8_t contents = IsClone() ? kHasCloneIDs : 0;
    if (fLoadMask.IsUsed())
        contents |= kHasLoadMask;
    s->WriteByte(contents);

    fLocation.Write(s);

    // conditional loadmask write
    if (contents & kHasLoadMask)
        fLoadMask.Write(s);

    s->WriteLE( fClassType );
    s->WriteLE( fObjectID );
    s->WriteSafeString( fObjectName );

    // conditional cloneIDs write
    if (contents & kHasCloneIDs)
    {
        s->WriteLE(fCloneID);
        uint16_t dummy = 0;
        s->WriteLE(dummy); // to avoid breaking format
        s->WriteLE(fClonePlayerID);
    }
}

void plUoid::Invalidate()
{
    fObjectID = 0;
    fCloneID = 0;
    fClonePlayerID = 0;
    fClassType = 0;
    fObjectName = plString::Null;
    fLocation.Invalidate();
    fLoadMask = plLoadMask::kAlways;

}

bool plUoid::IsValid() const
{
    if (!fLocation.IsValid() || fObjectName.IsNull())
        return false;

    return true;
}

bool plUoid::operator==(const plUoid& u) const
{
    return  fLocation == u.fLocation
            && fLoadMask == u.fLoadMask
            && fClassType == u.fClassType
            && fObjectName == u.fObjectName
            && fObjectID == u.fObjectID
            && fCloneID == u.fCloneID
            && fClonePlayerID == u.fClonePlayerID;
}

plUoid& plUoid::operator=(const plUoid& rhs)
{
    fObjectID = rhs.fObjectID;
    fCloneID = rhs.fCloneID;
    fClonePlayerID = rhs.fClonePlayerID;
    fClassType = rhs.fClassType;
    fObjectName = rhs.fObjectName;
    fLocation = rhs.fLocation;
    fLoadMask = rhs.fLoadMask;

    return *this;
}

// THIS SHOULD BE FOR DEBUGGING ONLY <hint hint>
plString plUoid::StringIze() const // Format to displayable string
{
    return plFormat("(0x{x}:0x{x}:{}:C:[{},{}])",
        fLocation.GetSequenceNumber(),
        fLocation.GetFlags(),
        fObjectName,
        GetClonePlayerID(),
        GetCloneID());
}

PL_FORMAT_IMPL(const plUoid &)
{
    return PL_FORMAT_FORWARD(format, value.StringIze());
}
