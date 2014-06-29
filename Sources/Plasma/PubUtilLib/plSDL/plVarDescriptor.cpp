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
#include "hsStream.h"
#include "plSDL.h"

#include "pnKeyedObject/plUoid.h"
#include "pnNetCommon/plNetApp.h"
#include "pnMessage/plMessage.h"

#include "plUnifiedTime/plUnifiedTime.h"

const uint8_t plVarDescriptor::kVersion=3;        // for Read/Write format

/////////////////////////////////////////////////////////////////////////////////
// State Var
/////////////////////////////////////////////////////////////////////////////////

//
// Set type from a string.  Return false on err.
//
bool plVarDescriptor::SetType(const plString& type)
{
    if (type.IsNull())
        return false;

    if (!type.CompareI("vector3"))
        fType=kVector3;
    else
    if (!type.CompareI("point3"))
        fType=kPoint3;
    else
    if (!type.CompareI("rgb"))
        fType=kRGB;
    else
    if (!type.CompareI("rgba"))
        fType=kRGBA;
    else
    if (!type.CompareI("rgb8"))
        fType=kRGB8;
    else
    if (!type.CompareI("rgba8"))
        fType=kRGBA8;
    else
    if (!type.CompareNI("quat", 4))
        fType=kQuaternion;
    else
    if (!type.CompareI("rgba"))
        fType=kRGBA;
    else
    if (!type.CompareI("int"))
        fType=kInt;
    else
    if (!type.CompareI("byte"))
        fType=kByte;
    else
    if (!type.CompareI("short"))
        fType=kShort;
    else
    if (!type.CompareI("float"))
        fType=kFloat;
    else
    if (!type.CompareI("double"))
        fType=kDouble;
    else
    if (!type.CompareI("time"))
        fType=kTime;
    else
    if (!type.CompareI("ageTimeOfDay"))
        fType=kAgeTimeOfDay;
    else
    if (!type.CompareI("bool"))
        fType=kBool;
    else
    if (!type.CompareI("string32"))
        fType=kString32;
    else
    if (!type.CompareI("plKey"))
        fType=kKey;
    else
    if (!type.CompareI("message") || !type.CompareI("creatable") )
        fType=kCreatable;
    else
    if (type.CharAt(0)=='$')
        fType=kStateDescriptor;
    else
        return false;   // err
    
    fTypeString = type;

    return true;    // ok
}

void plVarDescriptor::CopyFrom(const plVarDescriptor* other)
{
    SetName(other->GetName());
    SetDefault(other->GetDefault());
    SetCount(other->GetCount());
    SetDisplayOptions(other->GetDisplayOptions());

    fTypeString=other->GetTypeString();
    
    fType = other->GetType();
    fFlags = other->fFlags;
}

//
// Var descriptors are read and written by state descriptors
//
bool plVarDescriptor::Read(hsStream* s) 
{
    uint8_t version;
    s->ReadLE(&version);
    if (version != kVersion)
    {
        if (plSDLMgr::GetInstance()->GetNetApp())
            plSDLMgr::GetInstance()->GetNetApp()->WarningMsg("SDL VarDescriptor version mismatch, read %d, should be %d - ignoring",
            version, kVersion);
        return false;
    }

    fName=s->ReadSafeString();

    plMsgStdStringHelper::Peek(fDisplayOptions, s);

    fCount=s->ReadLE32();

    fType=(Type)s->ReadByte();

    fDefault = s->ReadSafeString();

    fFlags = s->ReadLE32();
    return true;
}

//
// Var descriptors are read and written by state descriptors
//
void plVarDescriptor::Write(hsStream* s) const
{
    s->WriteLE(kVersion);
    s->WriteSafeString(fName);
    plMsgStdStringHelper::Poke(fDisplayOptions, s);
    s->WriteLE32(fCount);
    s->WriteByte((uint8_t)fType);
    s->WriteSafeString(fDefault);
    s->WriteLE32(fFlags);
}

/////////////////////////////////////////////////////////////////////////////////
// plSimpleVarDescriptor
/////////////////////////////////////////////////////////////////////////////////

plSimpleVarDescriptor::plSimpleVarDescriptor() :
    fAtomicType(kNone),
    fAtomicCount(1)
{   

}

// size in bytes
int plSimpleVarDescriptor::GetAtomicSize() const
{
    switch(fAtomicType)
    {
    case kInt:
        return sizeof(int)*GetAtomicCount();
    case kByte:
        return sizeof(uint8_t)*GetAtomicCount();
    case kShort:
        return sizeof(short)*GetAtomicCount();
    case kAgeTimeOfDay:
    case kFloat:
        return sizeof(float)*GetAtomicCount();
    case kTime:
        return sizeof(plUnifiedTime)*GetAtomicCount();
    case kDouble:
        return sizeof(double)*GetAtomicCount();
    case kBool:
        return sizeof(bool)*GetAtomicCount();
    case kString32:
        return sizeof(String32)*GetAtomicCount();
    case kKey:
        return sizeof(plUoid)*GetAtomicCount();
    default:
        return -1;  // err
    }
}

// size of var in bytes
int plSimpleVarDescriptor::GetSize() const
{
    int size=GetAtomicSize();
    return size>=0 ? size*GetCount() : size;
}

//
// Set type from a string.  Return false on err.
// Sets atomicCount and atomicType
//
bool plSimpleVarDescriptor::SetType(const plString& type)
{
    if (!plVarDescriptor::SetType(type))
        return false;

    if (!type.CompareI("vector3"))
    {
        fAtomicCount = 3;
        fAtomicType=kFloat;
    }
    else
    if (!type.CompareI("point3"))
    {
        fAtomicCount = 3;
        fAtomicType=kFloat;
    }
    else
    if (!type.CompareI("rgb"))
    {
        fAtomicCount = 3;
        fAtomicType=kFloat;
    }
    else
    if (!type.CompareI("rgba"))
    {
        fAtomicCount = 4;
        fAtomicType=kFloat;
    }
    else
    if (!type.CompareI("rgb8"))
    {
        fAtomicCount = 3;
        fAtomicType=kByte;
    }
    else
    if (!type.CompareI("rgba8"))
    {
        fAtomicCount = 4;
        fAtomicType=kByte;
    }
    else
    if (!type.CompareNI("quat", 4))
    {
        fAtomicCount = 4;
        fAtomicType=kFloat;
    }
    else
    if (!type.CompareI("int"))
        fAtomicType=kInt;
    else
    if (!type.CompareI("byte"))
        fAtomicType=kByte;
    else
    if (!type.CompareI("short"))
        fAtomicType=kShort;
    else
    if (!type.CompareI("float"))
        fAtomicType=kFloat;
    else
    if (!type.CompareI("double"))
        fAtomicType=kDouble;
    else
    if (!type.CompareI("time"))
        fAtomicType=kTime;
    else
    if (!type.CompareI("ageTimeOfDay"))
        fAtomicType=kAgeTimeOfDay;
    else
    if (!type.CompareI("bool"))
        fAtomicType=kBool;
    else
    if (!type.CompareI("string32"))
        fAtomicType=kString32;
    else
    if (!type.CompareI("plKey"))
        fAtomicType=kKey;
    else
    if (!type.CompareI("message") || !type.CompareI("creatable"))
        fAtomicType=kCreatable;
    else
        return false;   // err

    return true;    // ok
}

void plSimpleVarDescriptor::CopyFrom(const plSimpleVarDescriptor* other)
{
    plVarDescriptor::CopyFrom(other);

    fAtomicCount=other->GetAtomicCount();
    fAtomicType=other->GetAtomicType();
}

//
// Var descriptors are read and written by state descriptors
//
bool plSimpleVarDescriptor::Read(hsStream* s)   
{
    if (!plVarDescriptor::Read(s))
        return false;

    fAtomicCount=s->ReadLE16();
    fAtomicType=(Type)s->ReadByte();
    return true;
}

//
// Var descriptors are read and written by state descriptors
//
void plSimpleVarDescriptor::Write(hsStream* s) const
{
    plVarDescriptor::Write(s);

    s->WriteLE16((int16_t)fAtomicCount);    
    s->WriteByte((uint8_t)fAtomicType);
}

/////////////////////////////////////////////////////////////////////////////////
// plSDVarDescriptor
// A var which references another state descriptor
/////////////////////////////////////////////////////////////////////////////////
void plSDVarDescriptor::CopyFrom(const plSDVarDescriptor* other)
{
    plVarDescriptor::CopyFrom(other);

    SetStateDesc(other->GetStateDescriptor());
}

//
// Var descriptors are read and written by state descriptors
//
bool plSDVarDescriptor::Read(hsStream* s)   
{
    if (!plVarDescriptor::Read(s))
        return false;

    plString sdName=s->ReadSafeString();
    uint16_t version = s->ReadLE16();
    plStateDescriptor* sd=plSDLMgr::GetInstance()->FindDescriptor(sdName, version);
    hsAssert(sd, plFormat("Failed to find sdl descriptor: {},{}. Missing legacy descriptor?", sdName, version).c_str());
    SetStateDesc(sd);
    return true;
}

//
// Var descriptors are read and written by state descriptors
//
void plSDVarDescriptor::Write(hsStream* s) const
{
    plVarDescriptor::Write(s);

    s->WriteSafeString(GetStateDescriptor()->GetName());
    uint16_t version=GetStateDescriptor()->GetVersion();
    s->WriteLE(version);
}
