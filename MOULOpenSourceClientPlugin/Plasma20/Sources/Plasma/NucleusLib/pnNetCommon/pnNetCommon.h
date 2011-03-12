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
#ifndef pnNetCommon_h_inc
#define pnNetCommon_h_inc

#include "hsConfig.h"
#include "hsTypes.h"
#include "hsStlUtils.h"
#include "hsRefCnt.h"
#include "hsStream.h"
#include "../pnFactory/plCreatable.h"

//
// main logging switch
//
#ifndef PLASMA_EXTERNAL_RELEASE
# define NET_LOGGING
#endif

#ifndef hsLogEntry
# ifdef NET_LOGGING
#  define hsLogEntry(x) x
# else
#  define hsLogEntry(x)
# endif
#endif

#define hsDbgLogEntry(x) 
#ifdef NET_LOGGING
# ifdef HS_DEBUGGING
#  undef hsDbgLogEntry
#  define hsDbgLogEntry(x) x
# endif
#endif

#define Bool2Int(value)	((value)?1:0)
#define Int2Bool(value)	((value)?true:false)


///////////////////////////////////////////////////////////////////

namespace pnNetCommon
{
#ifndef SERVER

	UInt32 GetBinAddr(const char * textAddr);
	const char * GetTextAddr(UInt32 binAddr);
	
#endif // SERVER
}

///////////////////////////////////////////////////////////////////

class plCreatableStream : public plCreatable
{
	hsRAMStream fStream;
public:
	CLASSNAME_REGISTER( plCreatableStream );
	GETINTERFACE_ANY( plCreatableStream, plCreatable );
	void Read( hsStream* stream, hsResMgr* mgr=nil );
	void Write( hsStream* stream, hsResMgr* mgr=nil );
	hsStream * GetStream( void ) { return &fStream;}
};


///////////////////////////////////////////////////////////////////
// hsTempRef is incomplete. This type fills in some of the gaps
// (like symmetrical ref/unref and correct self-assign)

#ifndef SERVER

template <class T>
class plSafePtr
{
	T * fPtr;
public:
	plSafePtr(T * ptr = nil): fPtr(ptr) {hsRefCnt_SafeRef(fPtr);}
	~plSafePtr() { hsRefCnt_SafeUnRef(fPtr); }
	operator T*() const { return fPtr; }
	operator T*&() { return fPtr; }
	operator const T&() const { return *fPtr; }
	operator bool() const { return fPtr!=nil;}
	T * operator->() const { return fPtr; }
	T * operator *() const { return fPtr; }
	T * operator=(T * ptr)
	{
		hsRefCnt_SafeRef(ptr);
		hsRefCnt_SafeUnRef(fPtr);
		fPtr = ptr;
		return fPtr;
	}
	void Attach(T * ptr)
	{
		if (fPtr==ptr)
			return;
		hsRefCnt_SafeUnRef(fPtr);
		fPtr = ptr;
	}
	void Detach() { fPtr=nil;}
};

#endif // SERVER


#endif // pnNetCommon_h_inc

///////////////////////////////////////////////////////////////////
// End.
