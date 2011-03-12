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
#ifndef plAnimTimeConvertSDLModifier_inc
#define plAnimTimeConvertSDLModifier_inc

#include "../plModifier/plSDLModifier.h"

//
// This modifier (abstract baseclass) handles sending and recving 
// the state for an animTimeConvert
//
class plStateDataRecord;
class plAnimTimeConvert;

class plAnimTimeConvertSDLModifier : public plSDLModifier
{
protected:
	// var labels 
	struct AnimTimeConvertVarNames
	{
		static char	kStrFlags[];
		static char	kStrLastStateAnimTime[];		
		static char kStrLoopBegin[];
		static char kStrLoopEnd[];
		static char kStrSpeed[];
		static char	kStrCurrentEaseCurve[];
		static char	kStrCurrentEaseBeginWorldTime[];
		static char	kStrLastStateChange[];
	}; 
	
	void IPutATC(plStateDataRecord* state, plAnimTimeConvert* curAnimTimeConvert);
	void ISetCurrentATC(const plStateDataRecord* state, plAnimTimeConvert* curAnimTimeConvert);

public:
	CLASSNAME_REGISTER( plAnimTimeConvertSDLModifier);
	GETINTERFACE_ANY( plAnimTimeConvertSDLModifier, plSDLModifier);
};

#endif	// plAnimTimeConvertSDLModifier_inc

