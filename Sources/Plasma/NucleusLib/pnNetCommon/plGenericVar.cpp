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
#include "plGenericVar.h"

//////////////////////////////////////////////////////
// plGenericType
//////////////////////////////////////////////////////

// reset runtime state, not inherent state
void plGenericType::Reset() 
{ 
    fI=0;
}

void plGenericType::CopyFrom(const plGenericType& c)
{
    fType = c.fType;
    if (fType==kString || fType==kAny)
    {
        fS = c.fS;
    }
    else
    {
        memmove(&fD, &c.fD, sizeof(double));
    }
}

//// Conversion Functions ////////////////////////////////////////////////////

int32_t plGenericType::IToInt() const
{
    hsAssert( fType == kInt || fType == kAny, "Trying to use a non-int parameter as an int!" );

    if( fType == kAny )
    {
        return fS.to_int();
    }
    
    return fI;
}

uint32_t plGenericType::IToUInt() const
{
    hsAssert( fType == kUInt || fType == kAny, "Trying to use a non-int parameter as an int!" );

    if( fType == kAny )
    {
        return fS.to_uint();
    }
    
    return fU;
}

double plGenericType::IToDouble() const
{
    hsAssert( fType == kDouble || fType == kAny, "Trying to use a non-float parameter as a Double!" );

    if( fType == kAny )
    {
        return fS.to_double();
    }
    
    return fD;
}

float plGenericType::IToFloat() const
{
    hsAssert( fType == kFloat || fType == kAny, "Trying to use a non-float parameter as a float!" );

    if( fType == kAny )
    {
        return fS.to_float();
    }
    
    return fF;
}

bool plGenericType::IToBool() const
{
    hsAssert( fType == kBool || fType == kAny, "Trying to use a non-bool parameter as a bool!" );

    if( fType == kAny )
    {
        return fS.to_bool();
    }
    
    return fB;
}

ST::string plGenericType::IToString() const
{
    hsAssert( fType == kString || fType == kAny, "Trying to use a non-string parameter as a string!" );

    return fS;
}

char plGenericType::IToChar() const
{
    hsAssert( fType == kChar || fType == kAny, "Trying to use a non-char parameter as a char!" );

    if( fType == kAny )
    {
        return fS.front();
    }
    
    return fC;
}

void    plGenericType::Read(hsStream* s)
{
    s->ReadLE(&fType);

    switch ( fType )
    {
    case kString:
    case kAny:
        fS=s->ReadSafeString();
        break;
    case kBool:
        fB = s->ReadBool();
        break;
    case kChar:
        s->ReadLE( &fC );
        break;
    case kInt   :
        s->ReadLE( &fI );
        break;
    case kUInt:
        s->ReadLE( &fU );
        break;
    case kFloat:
        s->ReadLE( &fF );
        break;
    case kDouble:
        s->ReadLE( &fD );
        break;
    case kNone :
        break;
    }
}

void    plGenericType::Write(hsStream* s)
{
    s->WriteLE(fType);

    switch ( fType )
    {
    case kString:
    case kAny:
        s->WriteSafeString(fS);
        break;
    case kBool:
        s->WriteBool(fB);
        break;
    case kChar:
        s->WriteLE( fC );
        break;
    case kInt   :
        s->WriteLE( fI );
        break;
    case kUInt:
        s->WriteLE( fU );
        break;
    case kFloat:
        s->WriteLE( fF );
        break;
    case kDouble:
        s->WriteLE( fD );
        break;
    case kNone :
        break;
    }
}

///////////////////////////////////////////////////
///////////////////////////////////////////////////
void    plGenericVar::Read(hsStream* s)
{
    fName = s->ReadSafeString();
    fValue.Read(s);
}

void    plGenericVar::Write(hsStream* s)
{
    s->WriteSafeString(fName);
    fValue.Write(s);
}


//////////////////////////////////

ST::string plGenericType::GetAsString() const
{
    switch (fType)
    {
    case kInt :
        return ST::string::from_int(fI);
    case kBool :
        return ST::string::from_int(fB ? 1 : 0);
    case kUInt:
        return ST::string::from_uint(fU);
    case kFloat :
    case kDouble :
        return (fType==kDouble) ? ST::string::from_double(fD, 'f')
                                : ST::string::from_float(fF, 'f');
    case kChar :
        return ST::string::fill(1, fC);
    case kAny :
    case kString :
        return fS;
    case kNone :
        break;
    default:
        hsAssert(false,"plGenericType::GetAsString unknown type");
    }

    return ST::string();
}
