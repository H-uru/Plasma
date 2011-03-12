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

#ifndef plFlexibilityComponent_inc
#define plFlexibilityComponent_inc

const Class_ID FLEXIBILITY_COMP_CID(0x6fec783f, 0x705536d3);

class plFlexibilityComponent : public plComponent
{
public:
	enum {
		kFlexibility,
		kInterRand,
		kIntraRand
	};

public:
	plFlexibilityComponent();
	void DeleteThis() { delete this; }

	Point3 GetFlexibility() const;

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg) { return true; }
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)		{ return true; }
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg) { return true; }
};

#endif // plFlexibilityComponent_inc
