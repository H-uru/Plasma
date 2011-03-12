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

#ifndef plSoftVolumeTypes_inc
#define plSoftVolumeTypes_inc

#include "plSoftVolume.h"
#include "hsTemplates.h"

class plVolumeIsect;

class plSoftVolumeSimple : public plSoftVolume
{
protected:
	plVolumeIsect*				fVolume;
	hsScalar					fSoftDist;

private:
	virtual hsScalar			IGetStrength(const hsPoint3& pos) const;

public:
	plSoftVolumeSimple();
	virtual ~plSoftVolumeSimple();

	CLASSNAME_REGISTER( plSoftVolumeSimple );
	GETINTERFACE_ANY( plSoftVolumeSimple, plSoftVolume );

	virtual void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l);

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	// Now Simple specifics
	plVolumeIsect* GetVolume() const { return fVolume; }
	void SetVolume(plVolumeIsect* v); // Takes ownership, don't delete after giving to SoftVolume

	hsScalar GetDistance() const { return fSoftDist; }
	void SetDistance(hsScalar d) { fSoftDist = d; }

};

class plSoftVolumeComplex : public plSoftVolume
{
protected:
	hsTArray<plSoftVolume*>			fSubVolumes;

public:
	plSoftVolumeComplex();
	virtual ~plSoftVolumeComplex();

	CLASSNAME_REGISTER( plSoftVolumeComplex );
	GETINTERFACE_ANY( plSoftVolumeComplex, plSoftVolume );

	// Don't propagate the settransform to our children, they move independently
	virtual void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l) {}

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	virtual void		UpdateListenerPosition(const hsPoint3& p);

	// Now Complex specifics
	virtual hsBool MsgReceive(plMessage* msg);

	UInt16			GetNumSubs() const { return fSubVolumes.GetCount(); }
	const plSoftVolume* GetSub(int i) const { return fSubVolumes[i]; }
};

class plSoftVolumeUnion : public plSoftVolumeComplex
{
protected:
	virtual hsScalar			IUpdateListenerStrength() const;

private:
	virtual hsScalar			IGetStrength(const hsPoint3& pos) const;

public:
	plSoftVolumeUnion();
	virtual ~plSoftVolumeUnion();

	CLASSNAME_REGISTER( plSoftVolumeUnion );
	GETINTERFACE_ANY( plSoftVolumeUnion, plSoftVolumeComplex );

};

class plSoftVolumeIntersect : public plSoftVolumeComplex
{
protected:
	virtual hsScalar			IUpdateListenerStrength() const;

private:
	virtual hsScalar			IGetStrength(const hsPoint3& pos) const;

public:
	plSoftVolumeIntersect();
	virtual ~plSoftVolumeIntersect();

	CLASSNAME_REGISTER( plSoftVolumeIntersect );
	GETINTERFACE_ANY( plSoftVolumeIntersect, plSoftVolumeComplex );

};

class plSoftVolumeInvert : public plSoftVolumeComplex
{
protected:
	virtual hsScalar			IUpdateListenerStrength() const;

private:
	virtual hsScalar			IGetStrength(const hsPoint3& pos) const;

public:
	plSoftVolumeInvert();
	virtual ~plSoftVolumeInvert();

	CLASSNAME_REGISTER( plSoftVolumeInvert );
	GETINTERFACE_ANY( plSoftVolumeInvert, plSoftVolumeComplex );


};

#endif // plSoftVolumeTypes_inc
