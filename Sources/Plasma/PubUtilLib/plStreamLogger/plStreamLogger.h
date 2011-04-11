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
#ifndef PL_STREAM_LOGGER
#define PL_STREAM_LOGGER

#include "hsStream.h"
#include "hsStlUtils.h"
#include "../../NucleusLib/pnNetCommon/plGenericVar.h"

class plStreamLogger
{
public:
	class Event
	{
	public:
		enum Type
		{
			kSubStart,
			kSubEnd,
			kValue,
			kString,
		};
	private:
		Type fType;
		plGenericVar fVar;
		unsigned int fSize;
	public:
		Event(Type type, unsigned int size, plGenericVar& var) : fType(type), fSize(size), fVar(var) { }
		Type GetType() { return fType; }
		const plGenericVar& GetVar() { return fVar; }
		unsigned int  GetSize() { return fSize; }
	};

	typedef std::list<Event> EventList;
	typedef std::list<std::string> DescStack;

#ifdef STREAM_LOGGER
protected:

	EventList* fList;
	bool fEntryWaiting;  // don't log an "Unknown" b/c an entry is waiting
	DescStack fDescStack;

	void ILogEntryWaiting();

public:
	plStreamLogger() : fEntryWaiting(false), fList(nil) { }
	const EventList* GetList() { return fList; }
	void LogSetList(EventList* el) { fList = el; }

	void LogEntry(plGenericType::Types type, unsigned int size, void* value, const char* desc);
	bool IsLogEntryWaiting();

#endif
};


#ifndef STREAM_LOGGER
#define LogSetList(l) LogVoidFunc()
#else

#define LOG_READ_SWAP(type, enumtype) \
	void LogReadSwap(type* value, const char* desc) \
			{ ILogEntryWaiting(); ReadSwap(value); LogEntry(plGenericType::enumtype,sizeof(type),value, desc);}

#define LOG_READ_SWAP_ARRAY(type, enumtype) \
	void LogReadSwapV(int count, type values[], const char* desc) \
			{ ILogEntryWaiting(); ReadSwap(count,values); int i; for (i=0; i < count; i++) LogEntry(plGenericType::enumtype,sizeof(type),&(values[i]), desc);}

class hsReadOnlyLoggingStream : public hsReadOnlyStream, public plStreamLogger
{
private:

public:
	void	Rewind();
	void	FastFwd();
    void    SetPosition(UInt32 position);

	UInt32 Read(UInt32 byteCount, void * buffer);
	void Skip(UInt32 deltaByteCount);

	UInt32	LogRead(UInt32 byteCount, void * buffer, const char* desc);
	char*   LogReadSafeString();
	char*   LogReadSafeStringLong();
	void	LogSkip(UInt32 deltaByteCount, const char* desc);
	void	LogStringString(const char* s);
	void	LogSubStreamStart(const char* desc);
	void	LogSubStreamEnd();
	void	LogSubStreamPushDesc(const char* desc);

	LOG_READ_SWAP(bool, kBool)
	LOG_READ_SWAP(UInt8, kUInt)
	LOG_READ_SWAP(UInt16, kUInt)
	LOG_READ_SWAP(UInt32, kUInt)
	LOG_READ_SWAP_ARRAY(UInt8, kUInt)
	LOG_READ_SWAP_ARRAY(UInt16, kUInt)
	LOG_READ_SWAP_ARRAY(UInt32, kUInt)

	LOG_READ_SWAP(Int8, kInt)
	LOG_READ_SWAP(char, kChar)
	LOG_READ_SWAP(Int16, kInt)
	LOG_READ_SWAP(Int32, kInt)
	LOG_READ_SWAP(int, kInt)
	LOG_READ_SWAP_ARRAY(Int8, kInt)
	LOG_READ_SWAP_ARRAY(char, kChar)
	LOG_READ_SWAP_ARRAY(Int16, kInt)
	LOG_READ_SWAP_ARRAY(Int32, kInt)
	LOG_READ_SWAP_ARRAY(int, kInt)

	LOG_READ_SWAP(float, kFloat)
	LOG_READ_SWAP(double, kDouble)
	LOG_READ_SWAP_ARRAY(float, kFloat)
	LOG_READ_SWAP_ARRAY(double, kDouble)

};

#endif //STREAM_LOGGER

#endif //PL_STREAM_LOGGER

