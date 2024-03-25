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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

#ifndef plLightModifier_inc
#define plLightModifier_inc

#include "hsGeometry3.h"
#include "plModifier/plSimpleModifier.h"

class plController;
class plController;
class plLightInfo;
class plLimitedDirLightInfo;
class plOmniLightInfo;
class plSpotLightInfo;

class plLightModifier : public plSimpleModifier
{
protected:

    plController*       fColorCtl;
    plController*       fAmbientCtl;
    plController*       fSpecularCtl;

    plLightInfo*        fLight;

    void IApplyDynamic() override;

    virtual void IClearCtls();
public:
    plLightModifier();
    virtual ~plLightModifier();

    CLASSNAME_REGISTER( plLightModifier );
    GETINTERFACE_ANY( plLightModifier, plSimpleModifier );

    void AddTarget(plSceneObject* so) override;
    void RemoveTarget(plSceneObject* so) override;

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

    virtual bool HasAnima() const { return fColorCtl || fAmbientCtl || fSpecularCtl; }

    // Export only
    void SetColorCtl(plController* ctl) { fColorCtl = ctl; }
    void SetAmbientCtl(plController* ctl) { fAmbientCtl = ctl; }
    void SetSpecularCtl(plController* ctl) { fSpecularCtl = ctl; }

    virtual void DefaultAnimation();
    virtual float MaxAnimLength(float len) const;
};

class plOmniModifier : public plLightModifier
{
protected:

    plOmniLightInfo*    fOmni;

    plController*       fAttenCtl;
    hsPoint3            fInitAtten;

    void IApplyDynamic() override;

    void IClearCtls() override;
public:
    plOmniModifier();
    virtual ~plOmniModifier();

    CLASSNAME_REGISTER( plOmniModifier );
    GETINTERFACE_ANY( plOmniModifier, plLightModifier );

    void AddTarget(plSceneObject* so) override;
    void RemoveTarget(plSceneObject* so) override;

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

    bool HasAnima() const override { return plLightModifier::HasAnima() || fAttenCtl; }

    // Export Only
    void SetAttenCtl(plController* ctl) { fAttenCtl = ctl; }
    void SetInitAtten(const hsPoint3& p) { fInitAtten = p; }

    float MaxAnimLength(float len) const override;
};

class plSpotModifier : public plOmniModifier
{
protected:

    plSpotLightInfo*         fSpot;

    plController*           fInnerCtl;
    plController*           fOuterCtl;

    void IApplyDynamic() override;

    void IClearCtls() override;
public:
    plSpotModifier();
    virtual ~plSpotModifier();

    CLASSNAME_REGISTER( plSpotModifier );
    GETINTERFACE_ANY( plSpotModifier, plLightModifier );

    void AddTarget(plSceneObject* so) override;
    void RemoveTarget(plSceneObject* so) override;

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

    bool HasAnima() const override { return plOmniModifier::HasAnima() || fInnerCtl || fOuterCtl; }

    // Export Only
    void SetInnerCtl(plController* ctl) { fInnerCtl = ctl; }
    void SetOuterCtl(plController* ctl) { fOuterCtl = ctl; }

    float MaxAnimLength(float len) const override;
};

class plLtdDirModifier : public plLightModifier
{
protected:

    plLimitedDirLightInfo*      fLtdDir;

    plController*           fWidthCtl;
    plController*           fHeightCtl;
    plController*           fDepthCtl;

    void IApplyDynamic() override;

    void IClearCtls() override;
public:
    plLtdDirModifier();
    virtual ~plLtdDirModifier();

    CLASSNAME_REGISTER( plLtdDirModifier );
    GETINTERFACE_ANY( plLtdDirModifier, plLightModifier );

    void AddTarget(plSceneObject* so) override;
    void RemoveTarget(plSceneObject* so) override;

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

    bool HasAnima() const override { return plLightModifier::HasAnima() || fWidthCtl || fHeightCtl || fDepthCtl; }

    // Export Only
    void SetWidthCtl(plController* ctl) { fWidthCtl = ctl; }
    void SetHeightCtl(plController* ctl) { fHeightCtl = ctl; }
    void SetDepthCtl(plController* ctl) { fDepthCtl = ctl; }

    float MaxAnimLength(float len) const override;
};

#endif // plLightModifier_inc
