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


#include "hsKeyedObject.h"


struct hsMatrix44;
class hsBounds3Ext;
class hsGMaterial;
class hsTriangle3;
struct hsGSplat3;

class plDrawPrim : public hsRefCnt
{
public:
	enum plDPPrimType
	{
		kTypeNone				= 0x0,
		kTypeTriList			= 0x1,
		kTypeSplatList			= 0x2
	};
protected:
	UInt32			fPrimType;

	UInt32			fDrawProps;

	hsGMaterial*	fMaterial;

public:
	plDrawPrim() : fMaterial(nil), fDrawProps(0), fPrimType(kTypeNone) {}
	virtual ~plDrawPrim();

	virtual const hsBounds3Ext&	GetLocalBounds() const = 0;

	hsGMaterial*	GetMaterial() { return fMaterial; }
	UInt32			GetPrimType() { return fPrimType; }
	UInt32			GetDrawProps() { return fDrawProps; }
};

class plTriListPrim : public plDrawPrim
{
public:
	plTriListPrim() { fPrimType |= kTypeTriList; }
	virtual ~plTriListPrim();

	virtual const hsBounds3Ext&	GetLocalBounds() const = 0;

	virtual hsTriangle3*		GetTriList(int& num) = 0;

};

class plSplatListPrim : public plDrawPrim
{
public:
	plSplatListPrim() { fPrimType |= kTypeSplatList; }

	virtual const hsBounds3Ext&	GetLocalBounds() const = 0;

	virtual hsGSplat3*			GetSplatList(int& num) = 0;
};




