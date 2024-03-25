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
#ifndef plSDL_DESC_inc
#define plSDL_DESC_inc

//
// Code for State Description Language (SDL) Descriptors.
// These define a schema representing an object's saveState buffer.
//

#include "HeadSpin.h"
#include "plFileSystem.h"

#include <string_theory/string>

class plKey;
class plSDVarDescriptor;
class plSimpleVarDescriptor;
class plStateDescriptor;
class hsStream;

//
// Describes a variable in a state descriptor.
// Every variable is actually a list, either fixed or variable length.
// Abstract base class.
//
class plVarDescriptor
{
public:
    enum Type
    {
        kNone   = -1,

        // atomic types
        kInt,
        kFloat,
        kBool,
        kString32,
        kKey,       // plKey - basically a uoid
        kStateDescriptor,   // this var refers to another state descriptor
        kCreatable, // plCreatable - basically a classIdx and a read/write buffer
        kDouble,
        kTime,      // double which is automatically converted to server clock and back, use for animation times
        kByte,
        kShort,
        kAgeTimeOfDay,  // float which is automatically set to the current age time of day (0-1)
        
        // the following are a vector of floats
        kVector3=50,// atomicCount=3
        kPoint3,    // atomicCount=3
        kRGB,       // atomicCount=3
        kRGBA,      // atomicCount=4
        kQuaternion,    // atomicCount=4
        kRGB8,      // atomicCount=3
        kRGBA8,     // atomicCount=4
    };
    typedef char String32[32];
    
    enum Flags
    {
        kInternal   = 0x1,      // Don't allow access to this var in Vault Mgr
        kAlwaysNew  = 0x2,      // Treat this var as always having the latest timestamp when FlaggingNewerState
        kVariableLength = 0x4   // Var is defined as int foo[], so it's length is variable, starting at 0
    };
protected:
    static const uint8_t kVersion;      // for Read/Write format
    ST::string  fDefault;               // set by .sdl
    ST::string  fName;                  // set by .sdl
    int         fCount;                 // set by .sdl
    Type        fType;                  // set by .sdl
    ST::string  fTypeString;            // string version of fType
    uint32_t    fFlags;
    ST::string  fDisplayOptions;        // set by .sdl
public:
    plVarDescriptor() : fCount(1), fType(kNone), fFlags(0) { }
    virtual ~plVarDescriptor() { }
    
    virtual void CopyFrom(const plVarDescriptor* v);
    
    // conversion ops
    virtual plSimpleVarDescriptor*  GetAsSimpleVarDescriptor() = 0;
    virtual plSDVarDescriptor* GetAsSDVarDescriptor() = 0;
    virtual const plSimpleVarDescriptor*    GetAsSimpleVarDescriptor() const = 0;
    virtual const plSDVarDescriptor* GetAsSDVarDescriptor() const = 0;

    // getters
    ST::string GetDefault()    const { return fDefault; }
    ST::string GetName()   const     { return fName; }
    Type    GetType() const          { return fType; }
    ST::string GetTypeString() const { return fTypeString; }
    int     GetCount() const         { return fCount; }
    bool    IsInternal() const       { return (fFlags & kInternal) != 0; }
    bool    IsAlwaysNew() const      { return (fFlags & kAlwaysNew) != 0; }
    bool    IsVariableLength() const { return (fFlags & kVariableLength) != 0; }
    ST::string GetDisplayOptions() const { return fDisplayOptions; }
    
    // setters
    void    SetDefault(const ST::string& n)  { fDefault = n; }
    void    SetName(const ST::string& n)  { fName = n; }
    void    SetCount(int c)             { fCount=c; }
    virtual bool SetType(const ST::string& type);
    void    SetType(Type t)             { fType=t; }
    void    SetInternal(bool d)         { if (d) fFlags |= kInternal; else fFlags &= ~kInternal; }
    void    SetAlwaysNew(bool d)        { if (d) fFlags |= kAlwaysNew; else fFlags &= ~kAlwaysNew; }
    void    SetVariableLength(bool d)   { if (d) fFlags |= kVariableLength; else fFlags &= ~kVariableLength; }
    void    SetDisplayOptions(const ST::string& s) { fDisplayOptions=s;   }

    // IO
    virtual bool    Read(hsStream* s);  
    virtual void    Write(hsStream* s) const;
};

//
// Simple, non-nested var descriptors.  These are comprised of single types, as opposed to 
// referring to another state descriptor.
//
class plSimpleVarDescriptor : public plVarDescriptor
{
protected:
    Type    fAtomicType;            // base type (it. quaternion == kFloat)
    int     fAtomicCount;           // computed from type in .sdl (ie. quaternion == 4)
public:
    plSimpleVarDescriptor();
    virtual ~plSimpleVarDescriptor() {  }
        
    plSimpleVarDescriptor* GetAsSimpleVarDescriptor() override { return this; }
    plSDVarDescriptor* GetAsSDVarDescriptor() override { return nullptr; }
    const plSimpleVarDescriptor* GetAsSimpleVarDescriptor() const override { return this; }
    const plSDVarDescriptor* GetAsSDVarDescriptor() const override { return nullptr; }

    void CopyFrom(const plSimpleVarDescriptor* v);
    void CopyFrom(const plVarDescriptor* v) override { plVarDescriptor::CopyFrom(v); }   // lame compiler

    // getters
    int     GetSize() const;
    int     GetAtomicSize() const;      // size of one item in bytes (regardless of count)
    Type    GetAtomicType() const       { return fAtomicType; }
    int     GetAtomicCount() const      { return fAtomicCount; }    
    
    // setters
    bool    SetType(const ST::string& type) override;
    void    SetType(Type t) { plVarDescriptor::SetType(t); }    // for lame compiler
    void    SetAtomicType(Type t) { fAtomicType=t; }    

    // IO
    bool    Read(hsStream* s) override;
    void    Write(hsStream* s) const override;
};

//
// A var descriptor which references another state descriptor.
//
class plSDVarDescriptor : public plVarDescriptor
{
protected:
    plStateDescriptor* fStateDesc;      
public:
    plSDVarDescriptor(plStateDescriptor* sd=nullptr) : fStateDesc(sd) { }

    plSimpleVarDescriptor* GetAsSimpleVarDescriptor() override { return nullptr; }
    plSDVarDescriptor* GetAsSDVarDescriptor() override { return this; }
    const plSimpleVarDescriptor* GetAsSimpleVarDescriptor() const override { return nullptr; }
    const plSDVarDescriptor* GetAsSDVarDescriptor() const override { return this; }

    // getters
    plStateDescriptor* GetStateDescriptor() const { return fStateDesc; }

    // setters
    void SetStateDesc(plStateDescriptor* sd) { fStateDesc=sd; }

    void CopyFrom(const plSDVarDescriptor* v);
    void CopyFrom(const plVarDescriptor* v) override { plVarDescriptor::CopyFrom(v); }   // lame compiler

    // IO
    bool    Read(hsStream* s) override;
    void    Write(hsStream* s) const override;
};

//
// A state descriptor - describes the contents of a type of state buffer.
// There is one of these for each persistent object type.
// These descriptors are defined in a user-created .sdl file.
//
class plStateDescriptor
{
private:
    static const uint8_t kVersion;        // for Read/Write format
    typedef std::vector<plVarDescriptor*> VarsList; 
    VarsList fVarsList;
    int fVersion;
    ST::string fName;
    plFileName fFilename;  // the filename this descriptor was read from

    void IDeInit();
public:
    plStateDescriptor() : fVersion(-1) {}
    ~plStateDescriptor(); 

    // getters
    ST::string GetName() const { return fName; }
    int GetNumVars() const { return fVarsList.size(); }
    plVarDescriptor* GetVar(int i) const { return fVarsList[i]; }
    int GetVersion() const { return fVersion; }
    plFileName GetFilename() const { return fFilename; }

    // setters
    void SetVersion(int v) { fVersion=v; }
    void SetName(const ST::string& n) { fName=n; }
    void AddVar(plVarDescriptor* v) { fVarsList.push_back(v); }
    void SetFilename(const plFileName& n) { fFilename=n;}

    plVarDescriptor* FindVar(const ST::string& name, int* idx=nullptr) const;

    // IO
    bool Read(hsStream* s); 
    void Write(hsStream* s) const;
};

#endif  // plSDL_DESC_inc
