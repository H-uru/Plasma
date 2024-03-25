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
#ifndef PL_SDL_inc
#define PL_SDL_inc

//
// Code for the State Description Language (SDL)
//

#include <list>
#include <string_theory/format>

#include "plSDLDescriptor.h"

#include "pnFactory/plCreatable.h"
#include "pnKeyedObject/plKey.h"
#include "pnKeyedObject/plUoid.h"

#include "plUnifiedTime/plUnifiedTime.h"

class plClientUnifiedTime;
class plNetApp;
class plSDLMgr;

namespace plSDL
{
    typedef std::list<plStateDescriptor*> DescriptorList;
    enum    GeneralPurpose
    {
        kLatestVersion  = -1,       // used when requesting the latest version of a state descriptor
        kMaxListSize    = 9999      // maximum size of var lists
    };

    enum ContentsFlags // or saveFlags, 16 bits
    {
        kHasUoid        = 0x1,      
        kHasNotificationInfo = 0x2,
        kHasTimeStamp   = 0x4,
        kSameAsDefault  = 0x8,
        kHasDirtyFlag   = 0x10,
        kWantTimeStamp  = 0x20,

        kAddedVarLengthIO = 0x8000,     // using to establish a new version in the header, can delete in 8/03
        
        kHasMaximumValue= 0xffff,   
    };

    enum RWOptions
    {
        // General note: when writing nested SDL variables (plSDStateVariable),
        // only the kDirtyOnly option applies recursively.
        // All other write options are ignored when writing the nested records!

        // When writing: only write variables marked dirty.
        // Default is to write all used variables, dirty or not.
        kDirtyOnly = 1 << 0,

        // When reading: ignore any notification infos in blob and leave them empty in the variables in memory.
        // When writing: don't write notification infos to the blob and don't set kHasNotificationInfo flags.
        kSkipNotificationInfo = 1 << 1,

        // Only applies to plStateDataRecord::PrepNetMsg.
        // Create a message of type plNetMsgSDLStateBCast instead of plNetMsgSDLState.
        kBroadcast = 1 << 2,

        // When writing: write timestamps of variables that have one.
        // Default is to not write any variable timestamps unless specifically requested.
        // If plSDLMgr has kDisallowTimeStamping set, this flag causes a debug assert.
        kWriteTimeStamps = 1 << 3,

        // When reading: for variables that will be set as dirty (depending on other read options) and that don't have a timestamp,
        // set their timestamp to the current time upon reading, even if the variables don't have kWantTimeStamp set.
        // Ignored if plSDLMgr has kDisallowTimeStamping set.
        // When writing: set kWantTimeStamp flag for all variables.
        kTimeStampOnRead = 1 << 4,

        // When writing: enable writing variable timestamps and set all variables' timestamps to the current time before writing them.
        kTimeStampOnWrite = 1 << 5,

        // When reading: update variables' dirty status based on kHasDirtyFlag.
        // May be overridden by kMakeDirty and kDirtyNonDefaults.
        kKeepDirty = 1 << 6,

        // When writing: don't set kHasDirtyFlag based on variables' dirty status.
        // kMakeDirty and kDirtyNonDefaults may still cause kHasDirtyFlag to be set.
        kDontWriteDirtyFlag = 1 << 7,

        // When reading: set all variables as dirty, regardless of whether they have kHasDirtyFlag set.
        // Takes priority over kKeepDirty and kDirtyNonDefaults.
        // When writing: set kHasDirtyFlag for all variables, regardless of their dirty status.
        // Takes priority over kDontWriteDirtyFlag.
        kMakeDirty = 1 << 8,

        // When reading: for variables that don't have kSameAsDefault set, always set them as dirty, even if they don't have kHasDirtyFlag.
        // Takes priority over kKeepDirty, but may be overridden by kMakeDirty.
        // When writing: for variables whose values are different from their default, always set kHasDirtyFlag, even if they aren't dirty.
        // Takes priority over kDontWriteDirtyFlag.
        kDirtyNonDefaults = 1 << 9,

        // When reading: if the blob uses the latest known version, perform an "update" anyway, as if it had an older version.
        kForceConvert = 1 << 10,
    };

    enum BehaviorFlags
    {
        kDisallowTimeStamping = 0x1,
    };

    extern const ST::string kAgeSDLObjectName;
    void VariableLengthRead(hsStream* s, int size, int* val);
    void VariableLengthWrite(hsStream* s, int size, int val);
};

class plStateVarNotificationInfo
{
private:
    ST::string fHintString;
public:
    void SetHintString(ST::string c) { fHintString = std::move(c); }
    ST::string GetHintString() const { return fHintString; }

    void Read(hsStream* s);
    void Write(hsStream* s) const;
};

//
// Base class for a state variable.
// A state var is a var descriptor and it's value (contents)
//
class plSimpleStateVariable;
class plSDStateVariable;
class plStateVariable
{
public:
    enum Flags
    {
        kDirty  = 0x1,  // true when someone sets the value using Set(...), can be cleared after writing
        kUsed   = 0x2   // true when it contains some value (either by Set(...) or Read() ) 
    };
protected:
    uint32_t fFlags;
    plStateVarNotificationInfo fNotificationInfo;
public:
    plStateVariable() : fFlags(0) {}
    virtual ~plStateVariable() {}

    ST::string GetName() const { return GetVarDescriptor()->GetName(); }
    bool IsNamed(const char* n) const { return (n && !GetName().compare_i(n)); }
    virtual int GetCount() const = 0;

    // conversion ops
    virtual plSimpleStateVariable* GetAsSimpleStateVar() = 0;
    virtual plSDStateVariable* GetAsSDStateVar() = 0;
    virtual plVarDescriptor* GetVarDescriptor() = 0;
    virtual const plVarDescriptor* GetVarDescriptor() const = 0;
    virtual void Alloc(int cnt=-1 /* -1 means don't change count */) = 0;
    
    virtual bool IsDirty() const { return (fFlags & kDirty) != 0; }
    virtual bool IsUsed() const  { return (fFlags & kUsed) != 0; }

    void SetDirty(bool d) { if (d) fFlags |= kDirty; else fFlags &= ~kDirty; }
    void SetUsed(bool d) { if (d) fFlags |= kUsed; else fFlags &= ~kUsed; }
    virtual void SetFromDefaults(bool timeStampNow) = 0;
    virtual void TimeStamp( const plUnifiedTime & ut=plUnifiedTime::GetCurrent() ) = 0;
    virtual const plUnifiedTime& GetTimeStamp() const = 0;

    plStateVarNotificationInfo& GetNotificationInfo() { return fNotificationInfo; }
    const plStateVarNotificationInfo& GetNotificationInfo() const { return fNotificationInfo; }
    
    virtual void DumpToObjectDebugger(bool dirtyOnly, int level) const {}
    virtual void DumpToStream(hsStream* stream, bool dirtyOnly, int level) const {}

    // IO
    virtual bool ReadData(hsStream* s, float timeConvert, uint32_t readOptions);
    virtual bool WriteData(hsStream* s, float timeConvert, uint32_t writeOptions) const;
};

//
// Change Notifier. 
// When a plSimpleStateVariable changes it's value by more than the given delta value,
// a notification msg will be sent to the objects that registered interest.
//
class plStateChangeNotifier
{
    friend class plSimpleStateVariable;
private:
    float fDelta;
    typedef std::vector<plKey> KeyList;
    KeyList fKeys;      // the objects to notify on change>delta
    static uint32_t fCurrentPlayerID;

    void IAddKey(plKey k);
    int IRemoveKey(const plKey& k);
public:
    plStateChangeNotifier() : fDelta(0.f) { }
    plStateChangeNotifier(float i, plKey k);
    plStateChangeNotifier(const plStateChangeNotifier& copy) = default;
    plStateChangeNotifier(plStateChangeNotifier&& move) = default;

    virtual ~plStateChangeNotifier() = default;

    void AddNotificationKey(plKey k);
    void AddNotificationKeys(KeyList keys);
    int RemoveNotificationKey(const plKey& k);         // returns number of keys left after removal
    int RemoveNotificationKeys(KeyList keys);   // returns number of keys left after removal

    void SendNotificationMsg(const plSimpleStateVariable* srcVar, const plSimpleStateVariable* dstVar, const ST::string& sdlName);
    
    bool GetValue(float* i) const;
    bool SetValue(float i);

    static uint32_t GetCurrentPlayerID() { return fCurrentPlayerID;   }
    static void SetCurrentPlayerID(uint32_t p) { fCurrentPlayerID=p;  }

    plStateChangeNotifier& operator=(const plStateChangeNotifier& copy) = default;
    plStateChangeNotifier& operator=(plStateChangeNotifier&& move) = default;
    bool operator==(const plStateChangeNotifier &) const;
};

//
// A (non-nested) variable descriptor and its data contents.
//

class plSimpleStateVariable : public plStateVariable
{
protected:
    union
    {
        int*    fI;     // array of int
        short*  fS;     // array of short
        uint8_t* fBy;    // array of byte
        float*  fF;     // array of float
        double* fD;     // array of double
        bool*   fB;     // array of bool
        plUoid* fU;     // array of uoid
        plCreatable** fC;   // array of plCreatable ptrs
        plVarDescriptor::String32* fS32;    // array of strings
        plClientUnifiedTime* fT;    // array of Times
    };
    mutable plUnifiedTime   fTimeStamp;     // the last time the var was changed
    plSimpleVarDescriptor fVar;

    typedef std::vector<plStateChangeNotifier> StateChangeNotifiers;
    StateChangeNotifiers fChangeNotifiers;

    void IDeAlloc();
    void IInit();   // initize vars
    void IVarSet(bool timeStampNow=false);
    
    // converter fxns
    bool IConvertFromBool(plVarDescriptor::Type newType);
    bool IConvertFromInt(plVarDescriptor::Type newType);
    bool IConvertFromByte(plVarDescriptor::Type newType);
    bool IConvertFromShort(plVarDescriptor::Type newType);
    bool IConvertFromFloat(plVarDescriptor::Type newType);
    bool IConvertFromDouble(plVarDescriptor::Type newType);
    bool IConvertFromString(plVarDescriptor::Type newType);
    bool IConvertFromRGB(plVarDescriptor::Type newType);
    bool IConvertFromRGBA(plVarDescriptor::Type newType);
    bool IConvertFromRGB8(plVarDescriptor::Type newType);
    bool IConvertFromRGBA8(plVarDescriptor::Type newType);

    bool IReadData(hsStream* s, float timeConvert, int idx);
    bool IWriteData(hsStream* s, float timeConvert, int idx) const;

public:

    plSimpleStateVariable() { IInit(); }        
    plSimpleStateVariable(plVarDescriptor* vd) { IInit(); CopyFrom(vd); }   
    ~plSimpleStateVariable() { IDeAlloc(); }
    
    // conversion ops
    plSimpleStateVariable* GetAsSimpleStateVar() override { return this; }
    plSDStateVariable* GetAsSDStateVar() override { return nullptr; }
    bool operator==(const plSimpleStateVariable &other) const;  // assumes matching var descriptors

    void TimeStamp(const plUnifiedTime & ut=plUnifiedTime::GetCurrent()) override;
    void CopyFrom(plVarDescriptor* v);
    void CopyData(const plSimpleStateVariable* other, uint32_t writeOptions=0);
    bool SetFromString(const ST::string& value, int idx, bool timeStampNow);  // set value from string, type.  return false on err
    ST::string GetAsString(int idx) const;
    bool ConvertTo(plSimpleVarDescriptor* toVar, bool force=false);         // return false on err
    void Alloc(int cnt=-1 /* -1 means don't change count */) override;      // alloc memory after setting type
    void Reset();

    // setters
    bool Set(float v, int idx=0);
    bool Set(float* v, int idx=0);                          // floatVector
    bool Set(double v, int idx=0);
    bool Set(double* v, int idx=0);                         // doubleVector
    bool Set(int v, int idx=0);
    bool Set(int* v, int idx=0) { return Set(*v, idx);  }   // helper since there is no int vec type
    bool Set(uint8_t v, int idx=0);
    bool Set(uint8_t* v, int idx=0);                           // uint8_tVector
    bool Set(short v, int idx=0);
    bool Set(short* v, int idx=0) { return Set(*v, idx); }  // helper since there is no short vec type
    bool Set(bool v, int idx=0);
    bool Set(bool* v, int idx=0) { return Set(*v, idx); }   // helper since there is no bool vec type
    bool Set(const char* v, int idx=0);                     // string
    bool Set(const plKey& v, int idx=0);
    bool Set(plCreatable*, int idx=0);                      // only SDL generated by the server is allowed to use this type.
    void SetFromDefaults(bool timeStampNow) override;
    
    // getters
    bool Get(int* value, int idx=0) const;
    bool Get(short* value, int idx=0) const;
    bool Get(uint8_t* value, int idx=0) const;          // returns uint8_t or uint8_tVector
    bool Get(float* value, int idx=0) const;            // returns float or floatVector
    bool Get(double* value, int idx=0) const;           // returns double or doubleVector
    bool Get(bool* value, int idx=0) const;
    bool Get(char value[], int idx=0) const;
    bool Get(plKey* value, int idx=0) const;
    bool Get(plCreatable** value, int idx=0) const;
    const plUnifiedTime& GetTimeStamp() const override { return fTimeStamp; }

    // Special backdoor so the KI Manager can get the key name without having a ResMgr
    ST::string GetKeyName(int idx=0) const;

    int GetCount() const override { return fVar.GetCount(); }    // helper
    plVarDescriptor* GetVarDescriptor() override { return &fVar; }
    plSimpleVarDescriptor* GetSimpleVarDescriptor() { return fVar.GetAsSimpleVarDescriptor(); }
    const plVarDescriptor* GetVarDescriptor() const override { return &fVar; }
    const plSimpleVarDescriptor* GetSimpleVarDescriptor() const { return fVar.GetAsSimpleVarDescriptor(); }

    // State Change Notification
    void AddStateChangeNotification(plStateChangeNotifier n);
    void RemoveStateChangeNotification(const plKey& notificationObj);      // remove all with this key 
    void RemoveStateChangeNotification(const plStateChangeNotifier& n);    // remove ones which match
    void NotifyStateChange(const plSimpleStateVariable* other, const ST::string& sdlName);      // send notification msg if necessary, internal use

    void DumpToObjectDebugger(bool dirtyOnly, int level) const override;
    void DumpToStream(hsStream* stream, bool dirtyOnly, int level) const override;

    // IO
    bool ReadData(hsStream* s, float timeConvert, uint32_t readOptions) override;
    bool WriteData(hsStream* s, float timeConvert, uint32_t writeOptions) const override;
};

//
// A list of state data records, all of which are the same kind. 
// Corresponds to a SD var descriptor.
//
class plStateDataRecord;
class plSDStateVariable : public plStateVariable
{
public:
    typedef std::vector<const plStateDataRecord*> ConstDataRecList; 
protected:
    typedef std::vector<plStateDataRecord*> DataRecList;
    DataRecList fDataRecList;   
    plSDVarDescriptor* fVarDescriptor;

    void IDeInit();
public:
    plSDStateVariable(plSDVarDescriptor* sdvd);
    ~plSDStateVariable();   // delete all records

    // conversion ops
    plSimpleStateVariable* GetAsSimpleStateVar() override { return nullptr; }
    plSDStateVariable* GetAsSDStateVar() override { return this; }
    bool operator==(const plSDStateVariable &other) const;  // assumes matching var descriptors

    void ConvertTo(plSDStateVariable* otherSDVar, bool force=false);
    void CopyFrom(plSDStateVariable* other, uint32_t writeOptions=0);
    void UpdateFrom(plSDStateVariable* other, uint32_t writeOptions=0);
    void AddStateDataRecord(plStateDataRecord *sdr) { fDataRecList.push_back(sdr); SetDirty(true); SetUsed(true); }
    void InsertStateDataRecord(plStateDataRecord *sdr, int i) { fDataRecList[i] = sdr; SetDirty(true); SetUsed(true);}
    void SetFromDefaults(bool timeStampNow) override;
    void TimeStamp(const plUnifiedTime & ut=plUnifiedTime::GetCurrent()) override;
    const plUnifiedTime& GetTimeStamp() const override { static plUnifiedTime foo; return foo; }
    
    void Alloc(int cnt=-1 /* -1 means don't change count */) override;   // wipe and re-create
    void Alloc(plSDVarDescriptor* sdvd, int cnt=-1);            // wipe and re-create
    void Resize(int cnt);
    
    bool IsDirty() const override;
    bool IsUsed() const override;

    // getters
    plStateDataRecord* GetStateDataRecord(int i) { return fDataRecList[i]; }
    const plStateDataRecord* GetStateDataRecord(int i) const { return fDataRecList[i]; }
    int GetCount() const override { return fDataRecList.size(); }
    int GetUsedCount() const;
    int GetDirtyCount() const;
    void GetUsedDataRecords(ConstDataRecList*) const;
    void GetDirtyDataRecords(ConstDataRecList*) const;
    plVarDescriptor* GetVarDescriptor() override { return fVarDescriptor; }
    plSDVarDescriptor* GetSDVarDescriptor() { return fVarDescriptor->GetAsSDVarDescriptor(); }
    const plVarDescriptor* GetVarDescriptor() const override { return fVarDescriptor; }
    const plSDVarDescriptor* GetSDVarDescriptor() const { return fVarDescriptor->GetAsSDVarDescriptor(); }
    void FlagNewerState(const plSDStateVariable&, bool respectAlwaysNew);
    void FlagAlwaysNewState();
    void DumpToObjectDebugger(bool dirtyOnly, int level) const override;
    void DumpToStream(hsStream* stream, bool dirtyOnly, int level) const override;
    
    // IO
    bool ReadData(hsStream* s, float timeConvert, uint32_t readOptions) override;
    bool WriteData(hsStream* s, float timeConvert, uint32_t writeOptions) const override;
};

//
// Contains the actual data contents and points to its associated descriptor
//
class plNetMsgSDLState;
class plStateDataRecord : public plCreatable
{
public:
    typedef std::vector<plSimpleStateVariable*> SimpleVarsList;
    typedef std::vector<plSDStateVariable*> SDVarsList;
    enum Flags
    {
        kVolatile = 0x1
    };
protected:
    typedef std::vector<plStateVariable*> VarsList;

    const plStateDescriptor* fDescriptor;
    plUoid      fAssocObject;       // optional
    VarsList    fVarsList;          // list of variables
    VarsList    fSDVarsList;        // list of nested data records
    uint32_t    fFlags;
    static const uint8_t kIOVersion;  // I/O Version
    
    void IDeleteVarsList(VarsList& vars);
    void IInitDescriptor(const ST::string& name, int version);    // or plSDL::kLatestVersion
    void IInitDescriptor(const plStateDescriptor* sd);
    
    void IReadHeader(hsStream* s);
    void IWriteHeader(hsStream* s) const;
    bool IConvertVar(plSimpleStateVariable* fromVar, plSimpleStateVariable* toVar, bool force);

    plStateVariable* IFindVar(const VarsList& vars, const ST::string& name) const;
    int IGetNumUsedVars(const VarsList& vars) const;
    int IGetUsedVars(const VarsList& varsOut, VarsList *varsIn) const;  // build a list of vars that have data
    bool IHasUsedVars(const VarsList& vars) const;

    int IGetNumDirtyVars(const VarsList& vars) const;
    int IGetDirtyVars(const VarsList& varsOut, VarsList *varsIn) const; // build a list of vars that are dirty
    bool IHasDirtyVars(const VarsList& vars) const;
public:
    CLASSNAME_REGISTER( plStateDataRecord );
    GETINTERFACE_ANY( plStateDataRecord, plCreatable);

    plStateDataRecord(const ST::string& sdName, int version=plSDL::kLatestVersion);
    plStateDataRecord(plStateDescriptor* sd);
    plStateDataRecord(const plStateDataRecord &other, uint32_t writeOptions=0 ):fFlags(0) { CopyFrom(other, writeOptions); }
    plStateDataRecord() : fDescriptor(), fFlags() { }
    ~plStateDataRecord();

    bool ConvertTo(plStateDescriptor* other, bool force=false );
    bool operator==(const plStateDataRecord &other) const;  // assumes matching state descriptors

    uint32_t GetFlags() const { return fFlags;    }
    void SetFlags(uint32_t f) { fFlags =f;    }
    
    plSimpleStateVariable* FindVar(const ST::string& name) const { return (plSimpleStateVariable*)IFindVar(fVarsList, name); }
    plSDStateVariable* FindSDVar(const ST::string& name) const { return (plSDStateVariable*)IFindVar(fSDVarsList, name); }
    
    plStateDataRecord& operator=(const plStateDataRecord& other) { CopyFrom(other); return *this; }
    void CopyFrom(const plStateDataRecord& other, uint32_t writeOptions=0);
    void UpdateFrom(const plStateDataRecord& other, uint32_t writeOptions=0);
    void SetFromDefaults(bool timeStampNow);
    void TimeStampDirtyVars();
    
    int GetNumVars() const { return fVarsList.size();   }
    plSimpleStateVariable* GetVar(int i) const { return (plSimpleStateVariable*)fVarsList[i];   }
    int GetNumSDVars() const { return fSDVarsList.size();   }
    plSDStateVariable* GetSDVar(int i) const { return (plSDStateVariable*)fSDVarsList[i];   }

    // Used vars
    bool IsUsed() const { return (HasUsedVars() || HasUsedSDVars()); }
    
    int GetNumUsedVars() const { return IGetNumUsedVars(fVarsList); }
    int GetUsedVars(SimpleVarsList *vars) const { return IGetUsedVars(fVarsList, (VarsList*)vars); }    // build a list of vars that have data
    bool HasUsedVars() const { return IHasUsedVars(fVarsList); }

    int GetNumUsedSDVars() const { return IGetNumUsedVars(fSDVarsList); }
    int GetUsedSDVars(SDVarsList *vars) const { return IGetUsedVars(fSDVarsList, (VarsList*)vars); }    // build a list of SD vars that have data
    bool HasUsedSDVars() const { return IHasUsedVars(fSDVarsList); }

    // Dirty Vars
    bool IsDirty() const { return (HasDirtyVars() || HasDirtySDVars()); }   
    
    int GetNumDirtyVars() const { return IGetNumDirtyVars(fVarsList); }
    int GetDirtyVars(SimpleVarsList *vars) const { return IGetDirtyVars(fVarsList, (VarsList*)vars); }  // build a list of vars that are dirty
    bool HasDirtyVars() const { return IHasDirtyVars(fVarsList); }

    int GetNumDirtySDVars() const { return IGetNumDirtyVars(fSDVarsList); }
    int GetDirtySDVars(SDVarsList *vars) const { return IGetDirtyVars(fSDVarsList, (VarsList*)vars); }; // build a list of Sdvars that are dirty
    bool HasDirtySDVars() const { return IHasDirtyVars(fSDVarsList); }

    const plStateDescriptor* GetDescriptor() const { return fDescriptor; }
    void SetDescriptor(const ST::string& sdName, int version);
    
    plNetMsgSDLState* PrepNetMsg(float timeConvert, uint32_t writeOptions) const; // create/prep a net msg with this data
    
    void SetAssocObject(const plUoid& u) { fAssocObject=u; }        // optional 
    plUoid* GetAssocObject() { return &fAssocObject; }      // optional
    const plUoid* GetAssocObject() const { return &fAssocObject; }      // optional

    // utils
    void FlagDifferentState(const plStateDataRecord& other);    // mark items which differ from 'other' as dirty
    void FlagNewerState(const plStateDataRecord& other, bool respectAlwaysNew=false);   // mark items which are newer than 'other' as dirty
    void FlagAlwaysNewState();  // mark 'alwaysNew' items as dirty
    void DumpToObjectDebugger(const char* msg, bool dirtyOnly=false, int level=0) const;
    void DumpToStream(hsStream* stream, const char* msg, bool dirtyOnly=false, int level=0) const;

    // IO
    bool Read(hsStream* s, float timeConvert, uint32_t readOptions=0);
    void Write(hsStream* s, float timeConvert, uint32_t writeOptions=0) const;

    static bool ReadStreamHeader(hsStream* s, ST::string* name, int* version, plUoid* objUoid=nullptr);
    void WriteStreamHeader(hsStream* s, plUoid* objUoid=nullptr) const;
};

//
// Simple SDL parser
//
class plSDLParser
{
private:
    bool IReadDescriptors() const;
    bool ILoadSDLFile(const plFileName& fileName) const;
    bool IParseVarDesc(const plFileName& fileName, hsStream* stream, char token[],
                       plStateDescriptor*& curDesc, plVarDescriptor*& curVar) const;
    bool IParseStateDesc(const plFileName& fileName, hsStream* stream, char token[],
                         plStateDescriptor*& curDesc) const;

    void DebugMsg(const ST::string& msg) const;

    void DebugMsg(const char* msg) const
    {
        DebugMsg(ST::string(msg));
    }

    template <typename... _Args>
    void DebugMsg(const char* fmt, _Args... args) const
    {
        DebugMsg(ST::format(fmt, args...));
    }

public:

    bool Parse() const; // reads sdl folder, creates descriptor list
};

//
// Holds, loads and unloads all state descriptors from sdl files.
// Singleton.
//
class plSDLMgr
{
    friend class plSDLParser;
private:
    plFileName  fSDLDir;
    plSDL::DescriptorList fDescriptors;
    plNetApp*   fNetApp;
    uint32_t    fBehaviorFlags;

    void IDeleteDescriptors(plSDL::DescriptorList* dl);
public:
    plSDLMgr();
    ~plSDLMgr();

    static plSDLMgr* GetInstance();
    plStateDescriptor* FindDescriptor(const ST::string& name, int version, const plSDL::DescriptorList * dl=nullptr) const;   // version or kLatestVersion
    
    const plSDL::DescriptorList * GetDescriptors() const { return &fDescriptors;}

    void SetSDLDir(const plFileName& s) { fSDLDir=s; }
    plFileName GetSDLDir() const { return fSDLDir; }

    void SetNetApp(plNetApp* a) { fNetApp=a; }
    plNetApp* GetNetApp() const { return fNetApp; }
    
    bool Init( uint32_t behaviorFlags=0 );    // parse sdl folder
    void DeInit();
    uint32_t GetBehaviorFlags() const { return fBehaviorFlags; }
    void SetBehaviorFlags(uint32_t v) { fBehaviorFlags=v; }
    bool AllowTimeStamping() const { return ! ( fBehaviorFlags&plSDL::kDisallowTimeStamping ); }

    // I/O - return # of bytes read/written
    int Write(hsStream* s, const plSDL::DescriptorList* dl=nullptr);    // write descriptors to a stream
    int Read(hsStream* s, plSDL::DescriptorList* dl=nullptr);       // read descriptors into provided list (use legacyList if nil)
};

#endif  // PL_SDL_inc
