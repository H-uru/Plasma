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

#ifndef plLightModifier_inc
#define plLightModifier_inc

#include "../../PubUtilLib/plModifier/plSimpleModifier.h"
#include "hsGeometry3.h"

class plController;
class plController;
class plLightInfo;
class plOmniLightInfo;
class plSpotLightInfo;
class plLimitedDirLightInfo;

class plLightModifier : public plSimpleModifier
{
protected:

	plController*		fColorCtl;
	plController*		fAmbientCtl;
	plController*		fSpecularCtl;

	plLightInfo*		fLight;

	virtual void IApplyDynamic();

	virtual void IClearCtls();
public:
	plLightModifier();
	virtual ~plLightModifier();

	CLASSNAME_REGISTER( plLightModifier );
	GETINTERFACE_ANY( plLightModifier, plSimpleModifier );

	virtual void AddTarget(plSceneObject* so);
	virtual void RemoveTarget(plSceneObject* so);

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	virtual hsBool HasAnima() const { return fColorCtl || fAmbientCtl || fSpecularCtl; }

	// Export only
	void SetColorCtl(plController* ctl) { fColorCtl = ctl; }
	void SetAmbientCtl(plController* ctl) { fAmbientCtl = ctl; }
	void SetSpecularCtl(plController* ctl) { fSpecularCtl = ctl; }

	virtual void DefaultAnimation();
	virtual hsScalar MaxAnimLength(hsScalar len) const;
};

class plOmniModifier : public plLightModifier
{
protected:

	plOmniLightInfo*	fOmni;

	plController*		fAttenCtl;
	hsPoint3			fInitAtten;

	virtual void IApplyDynamic();

	virtual void IClearCtls();
public:
	plOmniModifier();
	virtual ~plOmniModifier();

	CLASSNAME_REGISTER( plOmniModifier );
	GETINTERFACE_ANY( plOmniModifier, plLightModifier );

	virtual void AddTarget(plSceneObject* so);
	virtual void RemoveTarget(plSceneObject* so);

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	virtual hsBool HasAnima() const { return plLightModifier::HasAnima() || fAttenCtl; }

	// Export Only
	void SetAttenCtl(plController* ctl) { fAttenCtl = ctl; }
	void SetInitAtten(const hsPoint3& p) { fInitAtten = p; }

	virtual hsScalar MaxAnimLength(hsScalar len) const;
};

class plSpotModifier : public plOmniModifier
{
protected:

	plSpotLightInfo*			fSpot;

	plController*			fInnerCtl;
	plController*			fOuterCtl;

	virtual void IApplyDynamic();

	virtual void IClearCtls();
public:
	plSpotModifier();
	virtual ~plSpotModifier();

	CLASSNAME_REGISTER( plSpotModifier );
	GETINTERFACE_ANY( plSpotModifier, plLightModifier );

	virtual void AddTarget(plSceneObject* so);
	virtual void RemoveTarget(plSceneObject* so);

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	virtual hsBool HasAnima() const { return plOmniModifier::HasAnima() || fInnerCtl || fOuterCtl; }

	// Export Only
	void SetInnerCtl(plController* ctl) { fInnerCtl = ctl; }
	void SetOuterCtl(plController* ctl) { fOuterCtl = ctl; }

	virtual hsScalar MaxAnimLength(hsScalar len) const;
};

class plLtdDirModifier : public plLightModifier
{
protected:

	plLimitedDirLightInfo*		fLtdDir;

	plController*			fWidthCtl;
	plController*			fHeightCtl;
	plController*			fDepthCtl;

	virtual void IApplyDynamic();

	virtual void IClearCtls();
public:
	plLtdDirModifier();
	virtual ~plLtdDirModifier();

	CLASSNAME_REGISTER( plLtdDirModifier );
	GETINTERFACE_ANY( plLtdDirModifier, plLightModifier );

	virtual void AddTarget(plSceneObject* so);
	virtual void RemoveTarget(plSceneObject* so);

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	virtual hsBool HasAnima() const { return plLightModifier::HasAnima() || fWidthCtl || fHeightCtl || fDepthCtl; }

	// Export Only
	void SetWidthCtl(plController* ctl) { fWidthCtl = ctl; }
	void SetHeightCtl(plController* ctl) { fHeightCtl = ctl; }
	void SetDepthCtl(plController* ctl) { fDepthCtl = ctl; }

	virtual hsScalar MaxAnimLength(hsScalar len) const;
};

#endif // plLightModifier_inc
