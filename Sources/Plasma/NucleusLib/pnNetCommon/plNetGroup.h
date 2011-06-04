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
#ifndef plNetGroup_h
#define plNetGroup_h

#include "../pnKeyedObject/plUoid.h"
#include "hsStream.h"
#include "hsStlUtils.h"

class plNetGroupId
{
private:
   enum NetGroupConstants
   {
      kNetGroupConstant	= 0x01,
      kNetGroupLocal	= 0x02,
   };
   
   plLocation fId;
   UInt8 fFlags;
   std::string	fDesc;		// description of room
public:

   plNetGroupId() : fFlags(0) {}
   plNetGroupId(const plLocation& id, const UInt8 flags) : fId(id), fFlags(flags) {  }
   plNetGroupId(const plLocation& id) : fId(id), fFlags(0) {  }
   
   hsBool IsConstant() { return (fFlags & kNetGroupConstant) != 0; }
   void SetConstant(hsBool constantGroup) { fFlags &= constantGroup ? kNetGroupConstant : 0; }
   
   plLocation& Room() { return fId; }
   const char* GetDesc() const { return fDesc.c_str();   }
   void SetDesc(const char* c) { fDesc = c; }
   
   hsBool operator==(const plNetGroupId& netGroup) const { return fId == netGroup.fId; }
   hsBool operator!=(const plNetGroupId& netGroup) const { return fId != netGroup.fId; }
   bool operator<(const plNetGroupId& netGroup) const { return fId < netGroup.fId; }
   
   // read and write to hsStream
   void Write(hsStream *s) const { fId.Write(s); s->WriteSwap(fFlags); }
   void Read(hsStream *s) { fId.Read(s); s->LogReadSwap(&fFlags,"GroupId Flags"); }
};

namespace plNetGroup
{
	extern plNetGroupId kNetGroupLocalPlayer;
	extern plNetGroupId kNetGroupRemotePlayer;
	extern plNetGroupId kNetGroupUnknown;
	extern plNetGroupId kNetGroupLocalPhysicals;
	extern plNetGroupId kNetGroupRemotePhysicals;
}

#endif	// plNetGroup_h

