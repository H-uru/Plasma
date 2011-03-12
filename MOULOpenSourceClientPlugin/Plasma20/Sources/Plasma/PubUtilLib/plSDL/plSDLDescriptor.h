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
#ifndef plSDL_DESC_inc
#define plSDL_DESC_inc

//
// Code for State Description Language (SDL) Descriptors.
// These define a schema representing an object's saveState buffer.
//

#include "hsTypes.h"
#include "hsUtils.h"
#include "hsStlUtils.h"

//
// Describes a variable in a state descriptor.
// Every variable is actually a list, either fixed or variable length.
// Abstract base class.
//
class hsStream;
class plSimpleVarDescriptor;
class plSDVarDescriptor;
class plVarDescriptor
{
public:
	enum Type
	{
		kNone	= -1,

		// atomic types
		kInt,
		kFloat,
		kBool,
		kString32,
		kKey,		// plKey - basically a uoid
		kStateDescriptor,	// this var refers to another state descriptor
		kCreatable,	// plCreatable - basically a classIdx and a read/write buffer
		kDouble,
		kTime,		// double which is automatically converted to server clock and back, use for animation times
		kByte,
		kShort,
		kAgeTimeOfDay,	// float which is automatically set to the current age time of day (0-1)
		
		// the following are a vector of floats
		kVector3=50,// atomicCount=3
		kPoint3,	// atomicCount=3
		kRGB,		// atomicCount=3
		kRGBA,		// atomicCount=4
		kQuaternion,	// atomicCount=4
		kRGB8,		// atomicCount=3
		kRGBA8,		// atomicCount=4
	};
	typedef char String32[32];
	
	enum Flags
	{
		kInternal	= 0x1,		// Don't allow access to this var in Vault Mgr
		kAlwaysNew	= 0x2,		// Treat this var as always having the latest timestamp when FlaggingNewerState
		kVariableLength = 0x4	// Var is defined as int foo[], so it's length is variable, starting at 0
	};
protected:
	static const UInt8 kVersion;	// for Read/Write format
	char*	fDefault;				// set by .sdl
	char*	fName;					// set by .sdl
	int		fCount;					// set by .sdl
	Type	fType;					// set by .sdl
	char*	fTypeString;			// string version of fType
	UInt32  fFlags;
	std::string	fDisplayOptions;	// set by .sdl
public:
	plVarDescriptor();
	virtual ~plVarDescriptor();
	
	virtual void CopyFrom(const plVarDescriptor* v);
	
	// conversion ops
	virtual plSimpleVarDescriptor*	GetAsSimpleVarDescriptor() = 0;
	virtual plSDVarDescriptor* GetAsSDVarDescriptor() = 0;
	virtual const plSimpleVarDescriptor*	GetAsSimpleVarDescriptor() const = 0;
	virtual const plSDVarDescriptor* GetAsSDVarDescriptor() const = 0;

	// getters
	const char*	GetDefault()	const	{ return fDefault; }
	const char*	GetName()	const		{ return fName; }
	Type	GetType() const				{ return fType; }
	const char*	GetTypeString() const	{ return fTypeString; }
	int		GetCount() const			{ return fCount; }
	bool	IsInternal() const			{ return (fFlags & kInternal) != 0; }
	bool	IsAlwaysNew() const			{ return (fFlags & kAlwaysNew) != 0; }
	bool	IsVariableLength() const	{ return (fFlags & kVariableLength) != 0; }
	const char* GetDisplayOptions() const { return fDisplayOptions.c_str();	}
	
	// setters
	void	SetDefault(const char* n)	{ delete [] fDefault; fDefault= hsStrcpy(n); }
	void	SetName(const char* n)		{ delete [] fName; fName = hsStrcpy(n); }
	void	SetCount(int c)				{ fCount=c; }
	virtual bool	SetType(const char* type);
	void	SetType(Type t) { fType=t; }
	void	SetInternal(bool d)			{ if (d) fFlags |= kInternal; else fFlags &= ~kInternal; }
	void	SetAlwaysNew(bool d)		{ if (d) fFlags |= kAlwaysNew; else fFlags &= ~kAlwaysNew; }
	void	SetVariableLength(bool d)	{ if (d) fFlags |= kVariableLength; else fFlags &= ~kVariableLength; }
	void	SetDisplayOptions(const char* s) { fDisplayOptions=s;	}

	// IO
	virtual bool	Read(hsStream* s);	
	virtual void	Write(hsStream* s) const;
};

//
// Simple, non-nested var descriptors.  These are comprised of single types, as opposed to 
// referring to another state descriptor.
//
class plSimpleVarDescriptor : public plVarDescriptor
{
protected:
	Type	fAtomicType;			// base type (it. quaternion == kFloat)
	int		fAtomicCount;			// computed from type in .sdl (ie. quaternion == 4)
public:
	plSimpleVarDescriptor();
	virtual ~plSimpleVarDescriptor() {  }
		
	plSimpleVarDescriptor*	GetAsSimpleVarDescriptor() { return this; }
	plSDVarDescriptor* GetAsSDVarDescriptor() { return nil; }
	const plSimpleVarDescriptor*	GetAsSimpleVarDescriptor() const { return this; }
	const plSDVarDescriptor* GetAsSDVarDescriptor() const { return nil; }

	void CopyFrom(const plSimpleVarDescriptor* v);
	void CopyFrom(const plVarDescriptor* v) { plVarDescriptor::CopyFrom(v); }	// lame compiler

	// getters
	int		GetSize() const;
	int		GetAtomicSize() const;		// size of one item in bytes (regardless of count)
	Type	GetAtomicType() const		{ return fAtomicType; }
	int		GetAtomicCount() const		{ return fAtomicCount; }	
	
	// setters
	bool	SetType(const char* type);
	void	SetType(Type t) { plVarDescriptor::SetType(t); }	// for lame compiler
	void	SetAtomicType(Type t) { fAtomicType=t; }	

	// IO
	virtual bool	Read(hsStream* s);	
	virtual void	Write(hsStream* s) const;
};

//
// A var descriptor which references another state descriptor.
//
class plStateDescriptor;
class plSDVarDescriptor : public plVarDescriptor
{
protected:
	plStateDescriptor* fStateDesc;		
public:
	plSDVarDescriptor(plStateDescriptor* sd=nil) : fStateDesc(sd) { }

	plSimpleVarDescriptor*	GetAsSimpleVarDescriptor() { return nil; }
	plSDVarDescriptor* GetAsSDVarDescriptor() { return this; }
	const plSimpleVarDescriptor*	GetAsSimpleVarDescriptor() const { return nil; }
	const plSDVarDescriptor* GetAsSDVarDescriptor() const { return this; }

	// getters
	plStateDescriptor* GetStateDescriptor() const { return fStateDesc; }

	// setters
	void SetStateDesc(plStateDescriptor* sd) { fStateDesc=sd; }

	void CopyFrom(const plSDVarDescriptor* v);
	void CopyFrom(const plVarDescriptor* v) { plVarDescriptor::CopyFrom(v); }	// lame compiler

	// IO
	bool	Read(hsStream* s);	
	void	Write(hsStream* s) const;
};

//
// A state descriptor - describes the contents of a type of state buffer.
// There is one of these for each persistent object type.
// These descriptors are defined in a user-created .sdl file.
//
class plKey;
class plStateDescriptor
{
private:
	static const UInt8 kVersion;		// for Read/Write format
	typedef std::vector<plVarDescriptor*> VarsList;	
	VarsList fVarsList;
	int fVersion;
	char* fName;
	std::string fFilename;	// the filename this descriptor was read from

	void IDeInit();
public:
	plStateDescriptor() : fVersion(-1),fName(nil) {}
	~plStateDescriptor(); 

	// getters
	const char* GetName() const { return fName; }
	int GetNumVars() const { return fVarsList.size(); }
	plVarDescriptor* GetVar(int i) const { return fVarsList[i];	}
	int GetVersion() const { return fVersion; }
	const char * GetFilename() const { return fFilename.c_str();}
	
	// setters
	void SetVersion(int v) { fVersion=v; }
	void SetName(const char* n) { delete [] fName; fName=hsStrcpy(n); }
	void AddVar(plVarDescriptor* v) { fVarsList.push_back(v); }
	void SetFilename( const char * n ) { fFilename=n;}
	
	plVarDescriptor* FindVar(const char* name, int* idx=nil) const;

	// IO
	bool Read(hsStream* s);	
	void Write(hsStream* s) const;
};

#endif	// plSDL_DESC_inc
