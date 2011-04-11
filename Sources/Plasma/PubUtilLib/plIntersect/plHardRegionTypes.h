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

#ifndef plHardRegionTypes_inc
#define plHardRegionTypes_inc

#include "plHardRegion.h"
#include "hsTemplates.h"

class plHardRegionComplex : public plHardRegion
{
protected:
	hsTArray<plHardRegion*>			fSubRegions;

public:
	plHardRegionComplex();
	virtual ~plHardRegionComplex();

	CLASSNAME_REGISTER( plHardRegionComplex );
	GETINTERFACE_ANY( plHardRegionComplex, plHardRegion );

	// Don't propagate the settransform to our children, they move independently
	virtual void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l) {}

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	// Now Complex specifics
	virtual hsBool MsgReceive(plMessage* msg);

	UInt16			GetNumSubs() const { return fSubRegions.GetCount(); }
	const plHardRegion* GetSub(int i) const { return fSubRegions[i]; }
};

class plHardRegionUnion : public plHardRegionComplex
{
protected:
public:
	plHardRegionUnion();
	virtual ~plHardRegionUnion();

	CLASSNAME_REGISTER( plHardRegionUnion );
	GETINTERFACE_ANY( plHardRegionUnion, plHardRegionComplex );

	virtual hsBool	IIsInside(const hsPoint3& pos) const;
	virtual hsBool	ICameraInside() const;

};

class plHardRegionIntersect : public plHardRegionComplex
{
protected:
public:
	plHardRegionIntersect();
	virtual ~plHardRegionIntersect();

	CLASSNAME_REGISTER( plHardRegionIntersect );
	GETINTERFACE_ANY( plHardRegionIntersect, plHardRegionComplex );

	virtual hsBool	IIsInside(const hsPoint3& pos) const;
	virtual hsBool	ICameraInside() const;

};

class plHardRegionInvert : public plHardRegionComplex
{
protected:
public:
	plHardRegionInvert();
	virtual ~plHardRegionInvert();

	CLASSNAME_REGISTER( plHardRegionInvert );
	GETINTERFACE_ANY( plHardRegionInvert, plHardRegionComplex );

	virtual hsBool	IIsInside(const hsPoint3& pos) const;
	virtual hsBool	ICameraInside() const;

};


#endif // plHardRegionTypes_inc
