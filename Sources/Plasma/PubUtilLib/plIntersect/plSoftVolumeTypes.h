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

#ifndef plSoftVolumeTypes_inc
#define plSoftVolumeTypes_inc

#include <vector>

#include "plSoftVolume.h"

class plVolumeIsect;

class plSoftVolumeSimple : public plSoftVolume
{
protected:
    plVolumeIsect*              fVolume;
    float                    fSoftDist;

private:
    float            IGetStrength(const hsPoint3& pos) const override;

public:
    plSoftVolumeSimple();
    virtual ~plSoftVolumeSimple();

    CLASSNAME_REGISTER( plSoftVolumeSimple );
    GETINTERFACE_ANY( plSoftVolumeSimple, plSoftVolume );

    void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l) override;

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

    // Now Simple specifics
    plVolumeIsect* GetVolume() const { return fVolume; }
    void SetVolume(plVolumeIsect* v); // Takes ownership, don't delete after giving to SoftVolume

    float GetDistance() const { return fSoftDist; }
    void SetDistance(float d) { fSoftDist = d; }

};

class plSoftVolumeComplex : public plSoftVolume
{
protected:
    std::vector<plSoftVolume*> fSubVolumes;

public:
    plSoftVolumeComplex();
    virtual ~plSoftVolumeComplex();

    CLASSNAME_REGISTER( plSoftVolumeComplex );
    GETINTERFACE_ANY( plSoftVolumeComplex, plSoftVolume );

    // Don't propagate the settransform to our children, they move independently
    void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l) override { }

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

    void        UpdateListenerPosition(const hsPoint3& p) override;

    // Now Complex specifics
    bool MsgReceive(plMessage* msg) override;

    size_t GetNumSubs() const { return fSubVolumes.size(); }
    const plSoftVolume* GetSub(size_t i) const { return fSubVolumes[i]; }
};

class plSoftVolumeUnion : public plSoftVolumeComplex
{
protected:
    float            IUpdateListenerStrength() const override;

private:
    float            IGetStrength(const hsPoint3& pos) const override;

public:
    plSoftVolumeUnion();
    virtual ~plSoftVolumeUnion();

    CLASSNAME_REGISTER( plSoftVolumeUnion );
    GETINTERFACE_ANY( plSoftVolumeUnion, plSoftVolumeComplex );

};

class plSoftVolumeIntersect : public plSoftVolumeComplex
{
protected:
    float            IUpdateListenerStrength() const override;

private:
    float            IGetStrength(const hsPoint3& pos) const override;

public:
    plSoftVolumeIntersect();
    virtual ~plSoftVolumeIntersect();

    CLASSNAME_REGISTER( plSoftVolumeIntersect );
    GETINTERFACE_ANY( plSoftVolumeIntersect, plSoftVolumeComplex );

};

class plSoftVolumeInvert : public plSoftVolumeComplex
{
protected:
    float            IUpdateListenerStrength() const override;

private:
    float            IGetStrength(const hsPoint3& pos) const override;

public:
    plSoftVolumeInvert();
    virtual ~plSoftVolumeInvert();

    CLASSNAME_REGISTER( plSoftVolumeInvert );
    GETINTERFACE_ANY( plSoftVolumeInvert, plSoftVolumeComplex );


};

#endif // plSoftVolumeTypes_inc
