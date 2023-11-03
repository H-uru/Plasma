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
#ifndef PL_CAMERA_COMPONENTS_H
#define PL_CAMERA_COMPONENTS_H

#define FIXEDCAM_CID Class_ID(0x60f30a2a, 0x32d110e8)
#define AUTOCAM_CID Class_ID(0x65961f03, 0x775b77e6)
#define AVATAR_POA_CID Class_ID(0x1fe2720, 0x43857066)
#define OBJECT_POA_CID Class_ID(0x5c77163c, 0x78711171)
#define DEFAULTCAM_CID Class_ID(0x75bb1256, 0x49bf34c6)
#define TRANSCAM_CID Class_ID(0x5951682f, 0x1c310006)
#define LIMITPAN_CID Class_ID(0x18360ad4, 0x7dc74837)
#define FPCAM_CID Class_ID(0x1efe4e3e, 0x5b7369e4)
#define CAMERAZOOM_CID Class_ID(0x57b061be, 0x7c4f38db)
#define FOLLOWCAM_CID Class_ID(0x227c19a9, 0x55d809d6)
#define ANIMCAM_CMD_CID Class_ID(0x435913bf, 0x116c19fc)
#define CAM_REGION_CID Class_ID(0x60e27303, 0x1cf148a8)
#define CAM_IGNORE_SUB_CID Class_ID(0x33ae4c0d, 0x2b9d6513)

#include "plComponent.h"
#include <map>

struct PreTrans;
class plMaxNode;
class plErrorMsg;
class plCameraModifier1;
class plCameraBrain1;
class plRailCameraMod;

class plLimitPanComponent : public plComponent
{
public:
    plLimitPanComponent();

    // SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg) override;
    bool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;
};

class plCameraZoomComponent : public plComponent
{
public:
    plCameraZoomComponent();

    // SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg) override;
    bool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;
};

class plTransOverrideComponent : public plComponent
{
public:
    plTransOverrideComponent();

    // SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg) override;
    bool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;
    bool DeInit(plMaxNode *node, plErrorMsg *pErrMsg) override;

    typedef std::map<plMaxNode*, PreTrans*> TransitionKeys;
    TransitionKeys fTransKeys;
    
    const TransitionKeys& GetTransKeys();
};


class plPOAAvatarComponent : public plComponent
{
public:
    plPOAAvatarComponent();

    // SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg) override { return true; }
    bool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg) override { return true; }
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override { return true; }
};


class plPOAObjectComponent : public plComponent
{
public:
    plPOAObjectComponent();

    // SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg) override;
    bool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override { return true; }

    plKey GetObjectKey();
};

class plMakeDefaultCamComponent : public plComponent
{
public:
    plMakeDefaultCamComponent();

    // SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg) override { return true; }
    bool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg) override { return true; }
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override { return true; }
};

class plCameraBaseComponent : public plComponent
{
public:
    plCameraBaseComponent(){ }

    bool SetupProperties(plMaxNode* pNode, plErrorMsg* pErrMsg) override;
    bool PreConvert(plMaxNode* pNode, plErrorMsg* pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;
    
    plCameraModifier1* ICreateCameraModifier(plMaxNode* pNode, plErrorMsg* pErrMsg);
    plCameraModifier1* ICreateFocalPointObject(plMaxNode* pNode, plErrorMsg* pErrMsg);
    bool IsValidNodeType(plMaxNode *pNode);
    void ISetLimitPan(plMaxNode* pNode, plCameraBrain1* pBrain);    
    void ISetLimitZoom(plMaxNode* pNode, plCameraBrain1* pBrain);   
    void ISetIgnoreSubworld(plMaxNode* pNode, plCameraBrain1* pBrain);  
    bool ISetPOA(plMaxNode* pNode, plCameraBrain1* pBrain, plErrorMsg* pErrMsg);

    typedef std::map<plMaxNode*, plCameraModifier1*> ModKeys;
    ModKeys fModKeys;
    const ModKeys& GetModKeys();
};

class plCamera1Component : public plCameraBaseComponent
{
public:
    plCamera1Component();
    bool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg) override;
};

class plCameraIgnoreSub : public plComponent
{
public:
    plCameraIgnoreSub();
    // SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg) override { return true; }
    bool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg) override { return true; }
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override { return true; }

};

class plAutoCamComponent : public plCameraBaseComponent
{
public:
    plAutoCamComponent();
    bool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg) override;
};

class plFPCamComponent : public plCameraBaseComponent
{
public:
    plFPCamComponent();
    bool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg) override;
};


class plRailCameraComponent : public plCameraBaseComponent
{
private:
    bool            fValid;

    plRailCameraMod*    fLineMod;

    bool        IMakeLineMod(plMaxNode* pNode, plErrorMsg* pErrMsg);

public:
    plRailCameraComponent();

    bool SetupProperties(plMaxNode* pNode, plErrorMsg* pErrMsg) override;
    bool PreConvert(plMaxNode* pNode, plErrorMsg* pErrMsg) override;
};


class plCircleCameraComponent : public plCameraBaseComponent
{
private:
    bool            fValid;

public:
    plCircleCameraComponent();

    bool PreConvert(plMaxNode* pNode, plErrorMsg* pErrMsg) override;
};


class plFollowCamComponent : public plCameraBaseComponent
{
public:
    plFollowCamComponent();
    bool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg) override;
};

class plCameraAnimCmdComponent : public plComponent
{
    bool fIgnoreFOV;
public:
    plCameraAnimCmdComponent();
    bool SetupProperties(plMaxNode* pNode, plErrorMsg* pErrMsg) override { return true; }
    bool PreConvert(plMaxNode* pNode, plErrorMsg* pErrMsg) override;
    bool Convert(plMaxNode* pNode, plErrorMsg* pErrMsg) override;
    bool IgnoreFOV() { return fIgnoreFOV; }
};

#endif
