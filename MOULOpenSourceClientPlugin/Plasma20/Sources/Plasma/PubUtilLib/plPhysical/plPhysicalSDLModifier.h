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
#ifndef plPhysicalSDLModifier_inc
#define plPhysicalSDLModifier_inc

#include "../plModifier/plSDLModifier.h"

class plStateDataRecord;
class plStatusLog;
class plPhysical;

//
// This modifier is responsible for sending and recving 
// an object's physical state.
//
class plPhysicalSDLModifier : public plSDLModifier
{
public:
	CLASSNAME_REGISTER( plPhysicalSDLModifier );
	GETINTERFACE_ANY( plPhysicalSDLModifier, plSDLModifier);

	const char* GetSDLName() const { return kSDLPhysical; }

	// For the console
	static void SetLogLevel(int level) { fLogLevel = level; }

protected:
	static int fLogLevel;
	static plStatusLog* IGetLog();
	void ILogState(const plStateDataRecord* state, bool useDirty, const char* prefix, UInt32 color);

	plPhysical* IGetPhysical();
	virtual void IPutCurrentStateIn(plStateDataRecord* dstState);
	virtual void ISetCurrentStateFrom(const plStateDataRecord* srcState);
	virtual void ISentState(const plStateDataRecord* sentState);
};

#endif	// plPhysicalSDLModifier_inc
