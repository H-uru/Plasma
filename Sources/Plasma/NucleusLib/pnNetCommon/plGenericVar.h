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
#ifndef plGenericVar_inc
#define plGenericVar_inc

#include "hsTypes.h"
#include "hsUtils.h"
#include "hsStlUtils.h"
#include "../pnFactory/plCreatable.h"

class hsStream;

//
// a generic (unioned) type
//
class plGenericType
{
public:
	typedef char*	CharPtr;

protected:
	union
	{
		Int32			fI;
		UInt32			fU;
		float			fF;
		double			fD;
		bool			fB;
		CharPtr			fS;
		char			fC;
	};
	
public:
	
	enum Types
	{
		kInt	= 0,
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
	UInt8	fType;
	
	const Int32		&IToInt( void ) const;
	const UInt32	&IToUInt( void ) const;
	const float		&IToFloat( void ) const;
	const double		&IToDouble( void ) const;
	const bool		&IToBool( void ) const;
	const CharPtr	&IToString( void ) const;
	const char		&IToChar( void ) const;

	void IDeallocString() { if (fType==kString || fType==kAny) {delete [] fS; fS=nil;} }
public:

	plGenericType() : fType(kNone) { Reset(); }
	plGenericType(const plGenericType& c) { CopyFrom(c);	}
	virtual ~plGenericType() { IDeallocString(); }

	plGenericType& operator=(const plGenericType& c) { CopyFrom(c); return *this;	}

	void CopyFrom(const plGenericType& c);
	virtual void Reset();
	operator Int32() const { return IToInt(); }
	operator UInt32() const { return IToUInt(); }
	operator double() const { return IToDouble(); }
	operator float() const { return IToFloat(); }
	operator bool() const { return IToBool(); }
	operator const CharPtr() const { return IToString(); }
	operator char() const { return IToChar(); }
	operator unsigned int() const { return IToUInt(); }
	operator int() const { return IToInt(); }

	void	SetType(Types t)		{ fType=t; }
	UInt8	GetType( void ) const	{ return fType; }
	
	std::string GetAsStdString() const;

	// implicit set
	void	Set( Int32 i )		{ fI = i; fType = kInt; }
	void	Set( UInt32 i )		{ fU = i; fType = kUInt; }
	void	Set( float f )		{ fF = f; fType = kFloat; }
	void	Set( double d )		{ fD = d; fType = kDouble; }
	void	Set( bool b ) 		{ fB = b; fType = kBool; }
	void	Set( CharPtr s )	{ IDeallocString(); fS = hsStrcpy(s); fType = kString; }
	void	Set( char c )		{ fC = c; fType = kChar; }

	// explicit set
	void	SetInt( Int32 i )			{ fI = i; fType = kInt; }
	void	SetUInt( UInt32 i )			{ fU = i; fType = kUInt; }
	void	SetFloat( float f )		{ fF = f; fType = kFloat; }
	void	SetDouble( double d )	{ fD = d; fType = kDouble; }
	void	SetBool( bool b ) 		{ fB = b; fType = kBool; }
	void	SetString( CharPtr s )	{ IDeallocString(); fS = hsStrcpy(s); fType = kString; }
	void	SetChar( char c )		{ fC = c; fType = kChar; }
	void	SetAny( CharPtr s )		{ IDeallocString(); fS = hsStrcpy(s); fType = kAny; }
	void	SetNone( void )			{ fType = kNone; }
	void	SetVar(Types t, unsigned int size, void* val);

	virtual void	Read(hsStream* s);
	virtual void	Write(hsStream* s);
};

//
// a generic variable (similar to pfConsoleCmdParam)
//
class plGenericVar 
{
protected:
	plGenericType fValue;
	char*	fName;
public:
	plGenericVar(const plGenericVar &c) : fName(nil) { CopyFrom(c);	}
	plGenericVar(const char* name=nil) : fName(nil) { SetName(name); }
	virtual ~plGenericVar() { delete [] fName;  }

	virtual void Reset() { Value().Reset();	}	// reset runtime state, not inherent state
	plGenericVar& operator=(const plGenericVar &c) { CopyFrom(c); return *this; }
	void CopyFrom(const plGenericVar &c) { delete [] fName; fName=hsStrcpy(c.GetName()); fValue=c.Value();	}
	const char*	GetName()	const		{ return fName; }
	void	SetName(const char* n)		{ delete [] fName; fName = hsStrcpy(n); }
	plGenericType& Value() { return fValue; }
	const plGenericType& Value() const { return fValue; }

	virtual void	Read(hsStream* s);
	virtual void	Write(hsStream* s);
};


// A creatable wrapper for plGenericType

class plCreatableGenericValue : public plCreatable
{
public:
	plGenericType	fValue;
	CLASSNAME_REGISTER( plCreatableGenericValue );
	GETINTERFACE_ANY( plCreatableGenericValue, plCreatable );
	void	Read(hsStream* s, hsResMgr* mgr) { fValue.Read(s);}
	void	Write(hsStream* s, hsResMgr* mgr) { fValue.Write(s);}
	plGenericType& Value() { return fValue; }
	const plGenericType& Value() const { return fValue; }
	operator Int32() const { return (Int32)fValue; }
	operator UInt32() const { return (UInt32)fValue; }
	operator float() const { return (float)fValue; }
	operator double() const { return (double)fValue; }
	operator bool() const { return (bool)fValue; }
	operator const char *() const { return (const char *)fValue; }
	operator char() const { return (char)fValue; }
};


#endif
