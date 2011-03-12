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
	virtual hsBool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg);
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
};

class plCameraZoomComponent : public plComponent
{
public:
	plCameraZoomComponent();

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg);
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
};

class plTransOverrideComponent : public plComponent
{
public:
	plTransOverrideComponent();

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg);
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool DeInit(plMaxNode *node, plErrorMsg *pErrMsg);

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
	virtual hsBool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg) { return true; }
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg) { return true; }
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg) { return true; }
};


class plPOAObjectComponent : public plComponent
{
public:
	plPOAObjectComponent();

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg);
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg) { return true; }

	plKey GetObjectKey();
};

class plMakeDefaultCamComponent : public plComponent
{
public:
	plMakeDefaultCamComponent();

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg) { return true; }
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg) { return true; }
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg) { return true; }
};

class plCameraBaseComponent : public plComponent
{
public:
	plCameraBaseComponent(){;}

	virtual hsBool SetupProperties(plMaxNode* pNode, plErrorMsg* pErrMsg);
	virtual hsBool PreConvert(plMaxNode* pNode, plErrorMsg* pErrMsg);
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
	
	plCameraModifier1* ICreateCameraModifier(plMaxNode* pNode, plErrorMsg* pErrMsg);
	plCameraModifier1* ICreateFocalPointObject(plMaxNode* pNode, plErrorMsg* pErrMsg);
	hsBool IsValidNodeType(plMaxNode *pNode);
	void ISetLimitPan(plMaxNode* pNode, plCameraBrain1* pBrain);	
	void ISetLimitZoom(plMaxNode* pNode, plCameraBrain1* pBrain);	
	void ISetIgnoreSubworld(plMaxNode* pNode, plCameraBrain1* pBrain);	
	hsBool ISetPOA(plMaxNode* pNode, plCameraBrain1* pBrain, plErrorMsg* pErrMsg);

	typedef std::map<plMaxNode*, plCameraModifier1*> ModKeys;
	ModKeys fModKeys;
	const ModKeys& GetModKeys();
};

class plCamera1Component : public plCameraBaseComponent
{
public:
	plCamera1Component();
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg);
};

class plCameraIgnoreSub : public plComponent
{
public:
	plCameraIgnoreSub();
	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg) { return true; }
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg) { return true; }
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg) { return true; }

};

class plAutoCamComponent : public plCameraBaseComponent
{
public:
	plAutoCamComponent();
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg);
};

class plFPCamComponent : public plCameraBaseComponent
{
public:
	plFPCamComponent();
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg);
};


class plRailCameraComponent : public plCameraBaseComponent
{
private:
	hsBool			fValid;

	plRailCameraMod*	fLineMod;

	hsBool		IMakeLineMod(plMaxNode* pNode, plErrorMsg* pErrMsg);

public:
	plRailCameraComponent();

	virtual hsBool SetupProperties(plMaxNode* pNode, plErrorMsg* pErrMsg);
	virtual hsBool PreConvert(plMaxNode* pNode, plErrorMsg* pErrMsg);
};


class plCircleCameraComponent : public plCameraBaseComponent
{
private:
	hsBool			fValid;

public:
	plCircleCameraComponent();

	virtual hsBool PreConvert(plMaxNode* pNode, plErrorMsg* pErrMsg);
};


class plFollowCamComponent : public plCameraBaseComponent
{
public:
	plFollowCamComponent();
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg);
};

class plCameraAnimCmdComponent : public plComponent
{
	hsBool fIgnoreFOV;
public:
	plCameraAnimCmdComponent();
	virtual hsBool SetupProperties(plMaxNode* pNode, plErrorMsg* pErrMsg){ return true; }
	virtual hsBool PreConvert(plMaxNode* pNode, plErrorMsg* pErrMsg);
	virtual hsBool Convert(plMaxNode* pNode, plErrorMsg* pErrMsg);
	hsBool IgnoreFOV() { return fIgnoreFOV;	}
};

#endif