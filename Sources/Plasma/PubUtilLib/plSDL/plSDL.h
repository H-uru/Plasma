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

#include "plSDLDescriptor.h"
#include "hsUtils.h"
#include "hsStlUtils.h"

#include "../pnFactory/plCreatable.h"
#include "../pnKeyedObject/plKey.h"
#include "../pnKeyedObject/plUoid.h"

#include "../plUnifiedTime/plUnifiedTime.h"

namespace plSDL
{
	typedef std::list<plStateDescriptor*> DescriptorList;	
	enum	GeneralPurpose
	{
		kLatestVersion  = -1,		// used when requesting the latest version of a state descriptor
		kMaxListSize	= 9999		// maximum size of var lists
	};

	enum ContentsFlags // or saveFlags, 16 bits
	{
		kHasUoid		= 0x1,		
		kHasNotificationInfo = 0x2,
		kHasTimeStamp	= 0x4,
		kSameAsDefault	= 0x8,
		kHasDirtyFlag	= 0x10,
		kWantTimeStamp	= 0x20,

		kAddedVarLengthIO = 0x8000,		// using to establish a new version in the header, can delete in 8/03
		
		kHasMaximumValue= 0xffff,	
	};

	enum RWOptions
	{
		kDirtyOnly				= 1<< 0,			// write option
		kSkipNotificationInfo	= 1<< 1,			// read/write option
		kBroadcast				= 1<< 2,			// send option
		kWriteTimeStamps		= 1<< 3,			// write out time stamps	
		kTimeStampOnRead		= 1<< 4,			// read: timestamp each var when it gets read. write: request that the reader timestamp the dirty vars.
		kTimeStampOnWrite		= 1<< 5,			// read: n/a. write: timestamp each var when it gets written.
		kKeepDirty				= 1<< 6,			// don't clear dirty flag on read
		kDontWriteDirtyFlag		= 1<< 7,			// write option. don't write var dirty flag.
		kMakeDirty				= 1<< 8,			// read/write: set dirty flag on var read/write. 
		kDirtyNonDefaults		= 1<< 9,			// dirty the var if non default value.
		kForceConvert			= 1<<10,			// always try to convert rec to latest on read
	};

	enum BehaviorFlags
	{
		kDisallowTimeStamping = 0x1,
	};

	extern const char* kAgeSDLObjectName;
	void VariableLengthRead(hsStream* s, int size, int* val);
	void VariableLengthWrite(hsStream* s, int size, int val);
};

class plStateVarNotificationInfo
{
private:
	std::string fHintString;
public:
	void SetHintString(const char* c) { fHintString=c;	}
	const char* GetHintString() const { return fHintString.c_str();	}

	void Read(hsStream* s, UInt32 readOptions);
	void Write(hsStream* s, UInt32 writeOptions) const;
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
		kDirty	= 0x1,	// true when someone sets the value using Set(...), can be cleared after writing
		kUsed	= 0x2	// true when it contains some value (either by Set(...) or Read() )	
	};
protected:
	UInt32 fFlags;
	plStateVarNotificationInfo fNotificationInfo;
public:
	plStateVariable() : fFlags(0) {}
	virtual ~plStateVariable() {}

	const char* GetName() const { return GetVarDescriptor()->GetName(); }
	bool IsNamed(const char* n) const { return (n && !stricmp(GetName(), n)); }
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
	virtual void TimeStamp( const plUnifiedTime & ut=plUnifiedTime::GetCurrentTime() ) = 0;
	virtual const plUnifiedTime& GetTimeStamp() const = 0;

	plStateVarNotificationInfo& GetNotificationInfo() {	return fNotificationInfo; }
	const plStateVarNotificationInfo& GetNotificationInfo() const {	return fNotificationInfo; }
	
	virtual void DumpToObjectDebugger(bool dirtyOnly, int level) const {}
	virtual void DumpToStream(hsStream* stream, bool dirtyOnly, int level) const {}

	// IO
	virtual bool ReadData(hsStream* s, float timeConvert, UInt32 readOptions);
	virtual bool WriteData(hsStream* s, float timeConvert, UInt32 writeOptions) const;
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
	KeyList fKeys;		// the objects to notify on change>delta
	static UInt32 fCurrentPlayerID;

	void IAddKey(plKey k);
	int IRemoveKey(plKey k);
public:
	plStateChangeNotifier();
	plStateChangeNotifier(float i, plKey k);

	void AddNotificationKey(plKey k) { IAddKey(k);	}
	void AddNotificationKeys(KeyList keys);
	int RemoveNotificationKey(plKey k);			// returns number of keys left after removal
	int RemoveNotificationKeys(KeyList keys);	// returns number of keys left after removal

	void SendNotificationMsg(const plSimpleStateVariable* srcVar, const plSimpleStateVariable* dstVar, const char* sdlName);
	
	bool GetValue(float* i) const;
	bool SetValue(float i);

	static UInt32 GetCurrentPlayerID() { return fCurrentPlayerID;	}
	static void SetCurrentPlayerID(UInt32 p) { fCurrentPlayerID=p;	}

	bool operator==(const plStateChangeNotifier &) const;
};

//
// A (non-nested) variable descriptor and its data contents.
//
class plUoid;
class plKey;
class plClientUnifiedTime;
typedef unsigned char byte;
class plSimpleStateVariable : public plStateVariable
{
protected:
	union
	{
		int*	fI;		// array of int
		short*	fS;		// array of short
		byte*	fBy;	// array of byte
		float*	fF;		// array of float
		double*	fD;		// array of double
		bool*	fB;		// array of bool
		plUoid*	fU;		// array of uoid
		plCreatable** fC;	// array of plCreatable ptrs
		plVarDescriptor::String32* fS32;	// array of strings
		plClientUnifiedTime* fT;	// array of Times
	};
	mutable plUnifiedTime	fTimeStamp;		// the last time the var was changed
	plSimpleVarDescriptor fVar;

	typedef std::vector<plStateChangeNotifier> StateChangeNotifiers;
	StateChangeNotifiers fChangeNotifiers;

	void IDeAlloc();
	void IInit();	// initize vars
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

	bool IReadData(hsStream* s, float timeConvert, int idx, UInt32 readOptions);	
	bool IWriteData(hsStream* s, float timeConvert, int idx, UInt32 writeOptions) const;

public:

	plSimpleStateVariable() { IInit(); }		
	plSimpleStateVariable(plVarDescriptor* vd) { IInit(); CopyFrom(vd); }	
	~plSimpleStateVariable() { IDeAlloc(); }
	
	// conversion ops
	plSimpleStateVariable* GetAsSimpleStateVar() { return this; }
	plSDStateVariable* GetAsSDStateVar() { return nil; }
	bool operator==(const plSimpleStateVariable &other) const;	// assumes matching var descriptors

	void TimeStamp( const plUnifiedTime & ut=plUnifiedTime::GetCurrentTime() );
	void CopyFrom(plVarDescriptor* v);
	void CopyData(const plSimpleStateVariable* other, UInt32 writeOptions=0);
	bool SetFromString(const char* value, int idx, bool timeStampNow);		// set value from string, type.  return false on err
	char* GetAsString(int idx) const;
	bool ConvertTo(plSimpleVarDescriptor* toVar, bool force=false);			// return false on err
	void Alloc(int cnt=-1 /* -1 means don't change count */);									// alloc memory after setting type
	void Reset();

	// setters
	bool Set(float v, int idx=0);
	bool Set(float* v, int idx=0);							// floatVector
	bool Set(double v, int idx=0);
	bool Set(double* v, int idx=0);							// doubleVector
	bool Set(int v, int idx=0);
	bool Set(int* v, int idx=0) { return Set(*v, idx);	}	// helper since there is no int vec type
	bool Set(byte v, int idx=0);
	bool Set(byte* v, int idx=0);							// byteVector
	bool Set(short v, int idx=0);
	bool Set(short* v, int idx=0) { return Set(*v, idx); }	// helper since there is no short vec type
	bool Set(bool v, int idx=0);
	bool Set(bool* v, int idx=0) { return Set(*v, idx);	}	// helper since there is no bool vec type
	bool Set(const char* v, int idx=0);						// string
	bool Set(const plKey& v, int idx=0);
	bool Set(plCreatable*, int idx=0);						// only SDL generated by the server is allowed to use this type.
	void SetFromDefaults(bool timeStampNow);
	
	// getters
	bool Get(int* value, int idx=0) const;
	bool Get(short* value, int idx=0) const;
	bool Get(byte* value, int idx=0) const;				// returns byte or byteVector
	bool Get(float* value, int idx=0) const;			// returns float or floatVector
	bool Get(double* value, int idx=0) const;			// returns double or doubleVector
	bool Get(bool* value, int idx=0) const;
	bool Get(char value[], int idx=0) const;
	bool Get(plKey* value, int idx=0) const;
	bool Get(plCreatable** value, int idx=0) const;
	const plUnifiedTime& GetTimeStamp() const { return fTimeStamp;	}

	// Special backdoor so the KI Manager can get the key name without having a ResMgr
	const char* GetKeyName(int idx=0) const;

	int GetCount() const { return fVar.GetCount(); }	// helper
	plVarDescriptor* GetVarDescriptor() { return &fVar; }
	plSimpleVarDescriptor* GetSimpleVarDescriptor() { return fVar.GetAsSimpleVarDescriptor(); }
	const plVarDescriptor* GetVarDescriptor() const { return &fVar; }
	const plSimpleVarDescriptor* GetSimpleVarDescriptor() const { return fVar.GetAsSimpleVarDescriptor(); }

	// State Change Notification
	void AddStateChangeNotification(plStateChangeNotifier& n);
	void RemoveStateChangeNotification(plKey notificationObj);		// remove all with this key 
	void RemoveStateChangeNotification(plStateChangeNotifier n);	// remove ones which match
	void NotifyStateChange(const plSimpleStateVariable* other, const char* sdlName);		// send notification msg if necessary, internal use

	void DumpToObjectDebugger(bool dirtyOnly, int level) const;
	void DumpToStream(hsStream* stream, bool dirtyOnly, int level) const;

	// IO
	bool ReadData(hsStream* s, float timeConvert, UInt32 readOptions);	
	bool WriteData(hsStream* s, float timeConvert, UInt32 writeOptions) const;
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
	~plSDStateVariable();	// delete all records

	// conversion ops
	plSimpleStateVariable* GetAsSimpleStateVar() { return nil; }
	plSDStateVariable* GetAsSDStateVar() { return this; }
	bool operator==(const plSDStateVariable &other) const;	// assumes matching var descriptors

	void ConvertTo(plSDStateVariable* otherSDVar, bool force=false);
	void CopyFrom(plSDStateVariable* other, UInt32 writeOptions=0);
	void UpdateFrom(plSDStateVariable* other, UInt32 writeOptions=0);
	void AddStateDataRecord(plStateDataRecord *sdr) { fDataRecList.push_back(sdr); SetDirty(true); SetUsed(true); }
	void InsertStateDataRecord(plStateDataRecord *sdr, int i) { fDataRecList[i] = sdr; SetDirty(true); SetUsed(true);}
	void SetFromDefaults(bool timeStampNow);
	void TimeStamp( const plUnifiedTime & ut=plUnifiedTime::GetCurrentTime() );
	const plUnifiedTime& GetTimeStamp() const { static plUnifiedTime foo; return foo; }
	
	void Alloc(int cnt=-1 /* -1 means don't change count */);	// wipe and re-create
	void Alloc(plSDVarDescriptor* sdvd, int cnt=-1);			// wipe and re-create
	void Resize(int cnt);
	
	bool IsDirty() const;
	bool IsUsed() const;

	// getters
	plStateDataRecord* GetStateDataRecord(int i) { return fDataRecList[i]; }
	const plStateDataRecord* GetStateDataRecord(int i) const { return fDataRecList[i]; }
	int GetCount() const { return fDataRecList.size(); }
	int GetUsedCount() const;
	int GetDirtyCount() const;
	void GetUsedDataRecords(ConstDataRecList*) const;
	void GetDirtyDataRecords(ConstDataRecList*) const;
	plVarDescriptor* GetVarDescriptor() { return fVarDescriptor; }
	plSDVarDescriptor* GetSDVarDescriptor() { return fVarDescriptor->GetAsSDVarDescriptor(); }
	const plVarDescriptor* GetVarDescriptor() const { return fVarDescriptor; }
	const plSDVarDescriptor* GetSDVarDescriptor() const { return fVarDescriptor->GetAsSDVarDescriptor(); }
	void FlagNewerState(const plSDStateVariable&, bool respectAlwaysNew);
	void FlagAlwaysNewState();
	void DumpToObjectDebugger(bool dirtyOnly, int level) const;
	void DumpToStream(hsStream* stream, bool dirtyOnly, int level) const;
	
	// IO
	bool ReadData(hsStream* s, float timeConvert, UInt32 readOptions);	
	bool WriteData(hsStream* s, float timeConvert, UInt32 writeOptions) const;
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
	plUoid		fAssocObject;		// optional
	VarsList	fVarsList;			// list of variables
	VarsList	fSDVarsList;		// list of nested data records
	UInt32		fFlags;
	static const UInt8 kIOVersion;	// I/O Version
	
	void IDeleteVarsList(VarsList& vars);
	void IInitDescriptor(const char* name, int version);	// or plSDL::kLatestVersion
	void IInitDescriptor(const plStateDescriptor* sd);
	
	void IReadHeader(hsStream* s);
	void IWriteHeader(hsStream* s) const;
	bool IConvertVar(plSimpleStateVariable* fromVar, plSimpleStateVariable* toVar, bool force);

	plStateVariable* IFindVar(const VarsList& vars, const char* name) const;
	int IGetNumUsedVars(const VarsList& vars) const;
	int IGetUsedVars(const VarsList& varsOut, VarsList *varsIn) const;	// build a list of vars that have data
	bool IHasUsedVars(const VarsList& vars) const;

	int IGetNumDirtyVars(const VarsList& vars) const;
	int IGetDirtyVars(const VarsList& varsOut, VarsList *varsIn) const;	// build a list of vars that are dirty
	bool IHasDirtyVars(const VarsList& vars) const;
public:
	CLASSNAME_REGISTER( plStateDataRecord );
	GETINTERFACE_ANY( plStateDataRecord, plCreatable);

	plStateDataRecord(const char* sdName, int version=plSDL::kLatestVersion);
	plStateDataRecord(plStateDescriptor* sd);
	plStateDataRecord(const plStateDataRecord &other, UInt32 writeOptions=0 ):fFlags(0) { CopyFrom(other, writeOptions); }
	plStateDataRecord():fFlags(0) {}
	~plStateDataRecord();
	
	bool ConvertTo(plStateDescriptor* other, bool force=false );
	bool operator==(const plStateDataRecord &other) const;	// assumes matching state descriptors

	UInt32 GetFlags() const { return fFlags;	}
	void SetFlags(UInt32 f) { fFlags =f;	}
	
	plSimpleStateVariable* FindVar(const char* name) const { return (plSimpleStateVariable*)IFindVar(fVarsList, name); }
	plSDStateVariable* FindSDVar(const char* name) const { return (plSDStateVariable*)IFindVar(fSDVarsList, name); }
	
	plStateDataRecord& operator=(const plStateDataRecord& other) { CopyFrom(other); }
	void CopyFrom(const plStateDataRecord& other, UInt32 writeOptions=0);
	void UpdateFrom(const plStateDataRecord& other, UInt32 writeOptions=0);
	void SetFromDefaults(bool timeStampNow);
	void TimeStampDirtyVars();
	
	int GetNumVars() const { return fVarsList.size();	}
	plSimpleStateVariable* GetVar(int i) const { return (plSimpleStateVariable*)fVarsList[i];	}
	int GetNumSDVars() const { return fSDVarsList.size();	}
	plSDStateVariable* GetSDVar(int i) const { return (plSDStateVariable*)fSDVarsList[i];	}

	// Used vars
	bool IsUsed() const { return (HasUsedVars() || HasUsedSDVars()); }
	
	int GetNumUsedVars() const { return IGetNumUsedVars(fVarsList); }
	int GetUsedVars(SimpleVarsList *vars) const { return IGetUsedVars(fVarsList, (VarsList*)vars); }	// build a list of vars that have data
	bool HasUsedVars() const { return IHasUsedVars(fVarsList); }

	int GetNumUsedSDVars() const { return IGetNumUsedVars(fSDVarsList); }
	int GetUsedSDVars(SDVarsList *vars) const { return IGetUsedVars(fSDVarsList, (VarsList*)vars); }	// build a list of SD vars that have data
	bool HasUsedSDVars() const { return IHasUsedVars(fSDVarsList); }

	// Dirty Vars
	bool IsDirty() const { return (HasDirtyVars() || HasDirtySDVars()); }	
	
	int GetNumDirtyVars() const { return IGetNumDirtyVars(fVarsList); }
	int GetDirtyVars(SimpleVarsList *vars) const { return IGetDirtyVars(fVarsList, (VarsList*)vars); }	// build a list of vars that are dirty
	bool HasDirtyVars() const { return IHasDirtyVars(fVarsList); }

	int GetNumDirtySDVars() const { return IGetNumDirtyVars(fSDVarsList); }
	int GetDirtySDVars(SDVarsList *vars) const { return IGetDirtyVars(fSDVarsList, (VarsList*)vars); };	// build a list of Sdvars that are dirty
	bool HasDirtySDVars() const { return IHasDirtyVars(fSDVarsList); }

	const plStateDescriptor* GetDescriptor() const { return fDescriptor; }
	void SetDescriptor(const char* sdName, int version);
	
	plNetMsgSDLState* PrepNetMsg(float timeConvert, UInt32 writeOptions) const;	// create/prep a net msg with this data
	
	void SetAssocObject(const plUoid& u) { fAssocObject=u; }		// optional 
	plUoid* GetAssocObject() { return &fAssocObject; }		// optional
	const plUoid* GetAssocObject() const { return &fAssocObject; }		// optional

	// utils
	void FlagDifferentState(const plStateDataRecord& other);	// mark items which differ from 'other' as dirty
	void FlagNewerState(const plStateDataRecord& other, bool respectAlwaysNew=false);	// mark items which are newer than 'other' as dirty
	void FlagAlwaysNewState();	// mark 'alwaysNew' items as dirty
	void DumpToObjectDebugger(const char* msg, bool dirtyOnly=false, int level=0) const;
	void DumpToStream(hsStream* stream, const char* msg, bool dirtyOnly=false, int level=0) const;

	// IO
	bool Read(hsStream* s, float timeConvert, UInt32 readOptions=0);
	void Write(hsStream* s, float timeConvert, UInt32 writeOptions=0) const;

	static bool ReadStreamHeader(hsStream* s, char** name, int* version, plUoid* objUoid=nil);
	void WriteStreamHeader(hsStream* s, plUoid* objUoid=nil) const;
};

//
// Simple SDL parser
//
class plSDLMgr;
class plSDLParser
{
private:
	bool IReadDescriptors() const;
	bool ILoadSDLFile(const char* fileName) const;
	bool IParseVarDesc(const char* fileName, hsStream* stream, char token[], plStateDescriptor*& curDesc, 
		plVarDescriptor*& curVar) const;
	bool IParseStateDesc(const char* fileName, hsStream* stream, char token[], plStateDescriptor*& curDesc) const;

	void DebugMsg(char* fmt, ...) const;
	void DebugMsgV(char* fmt, va_list args) const;

public:

	bool Parse() const;	// reads sdl folder, creates descriptor list
};

//
// Holds, loads and unloads all state descriptors from sdl files.
// Singleton.
//
class plNetApp;
class plSDLMgr
{
	friend class plSDLParser;
private:
	std::string fSDLDir;
	plSDL::DescriptorList fDescriptors;
	plNetApp*	fNetApp;
	UInt32		fBehaviorFlags;

	void IDeleteDescriptors(plSDL::DescriptorList* dl);
public:
	plSDLMgr();
	~plSDLMgr();

	static plSDLMgr* GetInstance();
	plStateDescriptor* FindDescriptor(const char* name, int version, const plSDL::DescriptorList * dl=nil) const;	// version or kLatestVersion
	
	const plSDL::DescriptorList * GetDescriptors( void ) const { return &fDescriptors;}

	void SetSDLDir(const char* s) { fSDLDir=s; }
	const char* GetSDLDir() const { return fSDLDir.c_str(); }

	void SetNetApp(plNetApp* a) { fNetApp=a; }
	plNetApp* GetNetApp() const { return fNetApp; }
	
	bool Init( UInt32 behaviorFlags=0 );	// parse sdl folder
	void DeInit();
	UInt32 GetBehaviorFlags() const { return fBehaviorFlags; }
	void SetBehaviorFlags(UInt32 v) { fBehaviorFlags=v; }
	bool AllowTimeStamping() const { return ! ( fBehaviorFlags&plSDL::kDisallowTimeStamping ); }

	// I/O - return # of bytes read/written
	int Write(hsStream* s, const plSDL::DescriptorList* dl=nil);	// write descriptors to a stream
	int Read(hsStream* s, plSDL::DescriptorList* dl=nil);		// read descriptors into provided list (use legacyList if nil)
};

#endif	// PL_SDL_inc
