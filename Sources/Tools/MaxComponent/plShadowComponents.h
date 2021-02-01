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

#ifndef plShadowComponents_inc
#define plShadowComponents_inc

const Class_ID SHADOWCAST_COMP_CID(0x4f447666, 0x73a07cc6);
const Class_ID SHADOWRCV_COMP_CID(0x1d3009ca, 0x4d28537f);
const Class_ID SHADOWLIGHT_COMP_CID(0x2a996151, 0x4f4d1ae1);

class plDirectShadowMaster;
class plErrorMsg;
class plMaxNode;
class plPointShadowMaster;
class plShadowCaster;

class plShadowCastComponent : public plComponent
{
public:
enum    
{
    kSelfShadow,
    kBlur,
    kBlurScale,
    kAtten,
    kAttenScale,
    kBoost,
    kQuality,
    kLimitRes
};
protected:
    plShadowCaster*     fCaster;
public:
    plShadowCastComponent();

    bool SetupProperties(plMaxNode* pNode, plErrorMsg* pErrMsg) override;
    bool PreConvert(plMaxNode* pNode, plErrorMsg* pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;

    static bool AddShadowCastModifier(plMaxNode* pNode, plShadowCaster* caster);
    static bool AddShadowCastModifier(plSceneObject* so, plShadowCaster* caster);
};

class plShadowRcvComponent : public plComponent
{
public:
enum    
{
    kForceRadio
};
enum
{
    kForceOn,
    kForceOff
};
public:
    plShadowRcvComponent();

    bool SetupProperties(plMaxNode* pNode, plErrorMsg* pErrMsg) override;
    bool PreConvert(plMaxNode* pNode, plErrorMsg* pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;
};

class plShadowLightComponent : public plComponent
{
public:
    enum {
        kFalloff,
        kMaxDist,
        kPower,
        kShadowOnly,
        kObeyGroups,
        kSelfShadow,
        kQuality
    };
protected:
    bool IAddDirectMaster(plMaxNode* node, plSceneObject* so);
    bool IAddPointMaster(plMaxNode* node, plSceneObject* so);

public:
    plShadowLightComponent();

    bool SetupProperties(plMaxNode* pNode, plErrorMsg* pErrMsg) override;
    bool PreConvert(plMaxNode* pNode, plErrorMsg* pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;
};


#endif // plShadowComponents_inc
