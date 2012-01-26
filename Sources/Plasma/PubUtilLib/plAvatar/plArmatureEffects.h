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
/** \file plArmatureEffects.h
    Manages environmental effects for the avatar.
*/
#ifndef plArmatureEffects_inc
#define plArmatureEffects_inc

#include "pnKeyedObject/hsKeyedObject.h"
#include "hsTemplates.h"
#include "hsBitVector.h"

class plArmatureMod;
class plArmatureEffect;
class plRandomSoundMod;

/** \class plArmatureEffects
    Passes key avatar events to external effects generators.
    Currently used for footstep sounds only, but should eventually
    generalize to water splashes, etc. 
    More to come...*/
class plArmatureEffectsMgr : public hsKeyedObject
{
protected:
    hsTArray<plArmatureEffect *> fEffects;
    hsBool fEnabled;

public:

    plArmatureEffectsMgr() : fArmature(nil), fEnabled(true) {}
    virtual ~plArmatureEffectsMgr() {}

    CLASSNAME_REGISTER( plArmatureEffectsMgr );
    GETINTERFACE_ANY( plArmatureEffectsMgr, hsKeyedObject );

    virtual void Read(hsStream* s, hsResMgr* mgr);
    virtual void Write(hsStream* s, hsResMgr* mgr); 

    virtual hsBool MsgReceive(plMessage* msg);

    uint32_t GetNumEffects();
    plArmatureEffect *GetEffect(uint32_t num);
    void ResetEffects();

    plArmatureMod *fArmature;

    enum
    {
        kFootDirt,
        kFootPuddle,
        kFootWater,
        kFootTile,
        kFootMetal,
        kFootWoodBridge,
        kFootRopeLadder,
        kFootGrass,
        kFootBrush,
        kFootHardWood,
        kFootRug,
        kFootStone,
        kFootMud,
        kFootMetalLadder,
        kFootWoodLadder,
        kFootDeepWater,
        kFootMaintainerGlass,
        kFootMaintainerStone,
        kFootSwimming,
        kMaxSurface,
        kFootNoSurface = kMaxSurface,
    };  
    static const char *SurfaceStrings[];
};

class plArmatureEffect : public hsKeyedObject
{
public:
    plArmatureEffect() {}
    ~plArmatureEffect() {}

    CLASSNAME_REGISTER( plArmatureEffect );
    GETINTERFACE_ANY( plArmatureEffect, hsKeyedObject );

    virtual hsBool HandleTrigger(plMessage* msg) = 0;
    virtual void Reset() {}
};

class plArmatureEffectFootSurface
{
public:
    uint8_t fID;
    plKey fTrigger;
};

class plArmatureEffectFootSound : public plArmatureEffect
{
protected:
    hsTArray<plArmatureEffectFootSurface *> fSurfaces;
    hsBitVector fActiveSurfaces;
    plRandomSoundMod *fMods[plArmatureEffectsMgr::kMaxSurface];

    uint32_t IFindSurfaceByTrigger(plKey trigger);

public:
    plArmatureEffectFootSound();
    ~plArmatureEffectFootSound();

    CLASSNAME_REGISTER( plArmatureEffectFootSound );
    GETINTERFACE_ANY( plArmatureEffectFootSound, plArmatureEffect );

    virtual void Read(hsStream* s, hsResMgr* mgr);
    virtual void Write(hsStream* s, hsResMgr* mgr); 

    virtual hsBool MsgReceive(plMessage* msg);
    virtual hsBool HandleTrigger(plMessage* msg);
    virtual void Reset();
    void SetFootType(uint8_t);

    enum
    {
        kFootTypeShoe,
        kFootTypeBare,
    };
};

#endif
