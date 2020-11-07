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
#ifndef plGenericVar_inc
#define plGenericVar_inc

#include "HeadSpin.h"

#include "pnFactory/plCreatable.h"

class hsStream;

//
// a generic (unioned) type
//
class plGenericType
{
protected:
    union
    {
        int32_t         fI;
        uint32_t        fU;
        float           fF;
        double          fD;
        bool            fB;
        char            fC;
    };
    ST::string fS;

public:
    
    enum Types
    {
        kInt    = 0,
        kFloat,
        kBool,
        kString,
        kChar,
        kAny,
        kUInt,
        kDouble,
        kNone = 0xff
    };

protected:
    uint8_t   fType;
    
    int32_t   IToInt() const;
    uint32_t  IToUInt() const;
    float     IToFloat() const;
    double    IToDouble() const;
    bool      IToBool() const;
    ST::string  IToString() const;
    char      IToChar() const;

public:

    plGenericType() : fType(kNone) { Reset(); }
    plGenericType(const plGenericType& c) { CopyFrom(c);    }
    virtual ~plGenericType() { }

    plGenericType& operator=(const plGenericType& c) { CopyFrom(c); return *this;   }

    void CopyFrom(const plGenericType& c);
    virtual void Reset();
    operator int32_t() const { return IToInt(); }
    operator uint32_t() const { return IToUInt(); }
    operator double() const { return IToDouble(); }
    operator float() const { return IToFloat(); }
    operator bool() const { return IToBool(); }
    operator ST::string() const { return IToString(); }
    operator char() const { return IToChar(); }

    void    SetType(Types t)        { fType=t; }
    uint8_t GetType() const   { return fType; }

    ST::string GetAsString() const;

    // implicit set
    void    Set( int32_t i )    { fI = i; fType = kInt; }
    void    Set( uint32_t i )   { fU = i; fType = kUInt; }
    void    Set( float f )      { fF = f; fType = kFloat; }
    void    Set( double d )     { fD = d; fType = kDouble; }
    void    Set( bool b )       { fB = b; fType = kBool; }
    void    Set( const ST::string& s )  { fS = s; fType = kString; }
    void    Set( char c )       { fC = c; fType = kChar; }

    // explicit set
    void    SetInt( int32_t i )     { fI = i; fType = kInt; }
    void    SetUInt( uint32_t i )   { fU = i; fType = kUInt; }
    void    SetFloat( float f )     { fF = f; fType = kFloat; }
    void    SetDouble( double d )   { fD = d; fType = kDouble; }
    void    SetBool( bool b )       { fB = b; fType = kBool; }
    void    SetString( const ST::string& s )  { fS = s; fType = kString; }
    void    SetChar( char c )       { fC = c; fType = kChar; }
    void    SetAny( const ST::string& s )     { fS = s; fType = kAny; }
    void    SetNone()         { fType = kNone; }

    virtual void    Read(hsStream* s);
    virtual void    Write(hsStream* s);
};

//
// a generic variable (similar to pfConsoleCmdParam)
//
class plGenericVar 
{
protected:
    plGenericType fValue;
    ST::string    fName;
public:
    plGenericVar(const plGenericVar &c) { CopyFrom(c); }
    plGenericVar(const ST::string& name={}) : fName(name) { }
    virtual ~plGenericVar() { }

    virtual void Reset() { Value().Reset(); }   // reset runtime state, not inherent state
    plGenericVar& operator=(const plGenericVar &c) { CopyFrom(c); return *this; }
    void CopyFrom(const plGenericVar &c) { fName=c.GetName(); fValue=c.Value();  }
    ST::string GetName()   const         { return fName; }
    void   SetName(const ST::string& n)  { fName = n; }
    plGenericType& Value() { return fValue; }
    const plGenericType& Value() const { return fValue; }

    virtual void    Read(hsStream* s);
    virtual void    Write(hsStream* s);
};


// A creatable wrapper for plGenericType

class plCreatableGenericValue : public plCreatable
{
public:
    plGenericType fValue;

    CLASSNAME_REGISTER(plCreatableGenericValue);
    GETINTERFACE_ANY(plCreatableGenericValue, plCreatable);

    void Read(hsStream* s, hsResMgr*) override {
        fValue.Read(s);
    }
    void Write(hsStream* s, hsResMgr*) override {
        fValue.Write(s);
    }

    plGenericType& Value() { return fValue; }
    const plGenericType& Value() const { return fValue; }

    operator int32_t() const { return (int32_t)fValue; }
    operator uint32_t() const { return (uint32_t)fValue; }
    operator float() const { return (float)fValue; }
    operator double() const { return (double)fValue; }
    operator bool() const { return (bool)fValue; }
    operator ST::string() const { return (ST::string)fValue; }
    operator char() const { return (char)fValue; }
};


#endif
