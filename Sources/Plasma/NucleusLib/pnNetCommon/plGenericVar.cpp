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
#include "hsMemory.h"
#include "hsStlUtils.h"

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
    IDeallocString();
    fType = c.fType;
    if (fType==kString || fType==kAny)
    {
        fS=hsStrcpy(c.fS);
    }
    else
    {
        HSMemory::BlockMove((void*)&c.fI, (void*)&fI, 4);
    }
}

//// Conversion Functions ////////////////////////////////////////////////////

const int32_t &   plGenericType::IToInt( void ) const
{
    hsAssert( fType == kInt || fType == kAny, "Trying to use a non-int parameter as an int!" );

    static int32_t i;
    if( fType == kAny )
    {
        hsAssert( fS != nil, "Weird parameter during conversion" );
        i = atoi( fS );
        return i;
    }
    
    return fI;
}

const uint32_t &  plGenericType::IToUInt( void ) const
{
    hsAssert( fType == kUInt || fType == kAny, "Trying to use a non-int parameter as an int!" );

    static uint32_t i;
    if( fType == kAny )
    {
        hsAssert( fS != nil, "Weird parameter during conversion" );
        i = atoi( fS );
        return i;
    }
    
    return fU;
}

const double &  plGenericType::IToDouble( void ) const
{
    hsAssert( fType == kDouble || fType == kAny, "Trying to use a non-float parameter as a Double!" );

    static double d;
    if( fType == kAny )
    {
        hsAssert( fS != nil, "Weird parameter during conversion" );
        d = atof( fS );
        return d;
    }
    
    return fD;
}

const float &   plGenericType::IToFloat( void ) const
{
    hsAssert( fType == kFloat || fType == kAny, "Trying to use a non-float parameter as a float!" );

    static float f;
    if( fType == kAny )
    {
        hsAssert( fS != nil, "Weird parameter during conversion" );
        f = (float)atof( fS );
        return f;
    }
    
    return fF;
}

const bool &    plGenericType::IToBool( void ) const
{
    hsAssert( fType == kBool || fType == kAny, "Trying to use a non-bool parameter as a bool!" );

    static bool b;
    if( fType == kAny )
    {
        hsAssert( fS != nil, "Weird parameter during conversion" );
        if( atoi( fS ) > 0 || stricmp( fS, "true" ) == 0 )
            b = true;
        else
            b = false;

        return b;
    }
    
    return fB;
}

const plGenericType::CharPtr &  plGenericType::IToString( void ) const
{
    hsAssert( fType == kString || fType == kAny, "Trying to use a non-string parameter as a string!" );

    return fS;
}

const char &    plGenericType::IToChar( void ) const
{
    hsAssert( fType == kChar || fType == kAny, "Trying to use a non-char parameter as a char!" );

    static char     c;
    if( fType == kAny )
    {
        hsAssert( fS != nil, "Weird parameter during conversion" );
        c = fS[ 0 ];
        return c;
    }
    
    return fC;
}

void    plGenericType::Read(hsStream* s)
{
    IDeallocString();
    s->ReadLE(&fType);

    switch ( fType )
    {
    case kString:
    case kAny:
        fS=s->ReadSafeString();
        break;
    case kBool:
        {int8_t b;
        s->ReadLE( &b );
        fB = b?true:false;}
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
        {int8_t b = fB?1:0;
        s->WriteLE( b );}
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
    delete [] fName;
    fName = s->ReadSafeString();
    fValue.Read(s);
}

void    plGenericVar::Write(hsStream* s)
{
    s->WriteSafeString(fName);
    fValue.Write(s);
}


//////////////////////////////////

void plGenericType::SetVar(Types t, unsigned int size, void* val)
{
    fType = t;

    switch (t)
    {
    case kInt :
        {
            hsAssert(size <= sizeof(fI), "plGenericType::SetVar size too large for int");
            memcpy(&fI, val, size);
            break;
        }
    case kUInt :
        {
            hsAssert(size <= sizeof(fU), "plGenericType::SetVar size too large for unsigned int");
            memcpy(&fU, val, size);
            break;
        }
    case kFloat :
        {
            hsAssert(size <= sizeof(fF), "plGenericType::SetVar size too large for float");
            memcpy(&fF, val, size);
            break;
        }
    case kDouble :
        {
            hsAssert(size <= sizeof(fD), "plGenericType::SetVar size too large for double");
            memcpy(&fD, val, size);
            break;
        }
    case kBool :
        {
            hsAssert(size <= sizeof(fB), "plGenericType::SetVar size too large for bool");
            memcpy(&fB, val, size);
            break;
        }
    case kChar :
        {
            hsAssert(size <= sizeof(fC), "plGenericType::SetVar size too large for char");
            memcpy(&fC, val, size);
            break;
        }
    case kString :
        {
            delete [] fS;
            fS = new char[size+1];
            memcpy(fS,val,size);
            fS[size] = 0;
            break;
        }
    case kNone :
        break;
    default:
        hsAssert(false,"plGenericType::SetVar unknown type");
    }
}


std::string plGenericType::GetAsStdString() const
{
    std::string s;

    switch (fType)
    {
    case kInt :
        {
            xtl::format(s,"%d",fI);
            break;
        }
    case kBool :
    case kUInt :
        {
            xtl::format(s,"%u",fType==kBool?fB:fU);
            break;
        }
    case kFloat :
    case kDouble :
        {
            xtl::format(s,"%f",fType==kDouble?fD:fF);
            break;
        }
    case kChar :
        {
            xtl::format(s,"%c",fC);
            break;
        }
    case kAny :
    case kString :
        {
            s = fS;
            break;
        }
    case kNone :
        break;
    default:
        hsAssert(false,"plGenericType::GetAsStdString unknown type");
    }

    return s;
}