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

#ifndef plShadowComponents_inc
#define plShadowComponents_inc

const Class_ID SHADOWCAST_COMP_CID(0x4f447666, 0x73a07cc6);
const Class_ID SHADOWRCV_COMP_CID(0x1d3009ca, 0x4d28537f);
const Class_ID SHADOWLIGHT_COMP_CID(0x2a996151, 0x4f4d1ae1);

class plMaxNode;
class plErrorMsg;
class plPointShadowMaster;
class plDirectShadowMaster;
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
	plShadowCaster*		fCaster;
public:
	plShadowCastComponent();

	hsBool SetupProperties(plMaxNode* pNode, plErrorMsg* pErrMsg);
	hsBool PreConvert(plMaxNode* pNode, plErrorMsg* pErrMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);

	static hsBool AddShadowCastModifier(plMaxNode* pNode, plShadowCaster* caster);
	static hsBool AddShadowCastModifier(plSceneObject* so, plShadowCaster* caster);
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

	hsBool SetupProperties(plMaxNode* pNode, plErrorMsg* pErrMsg);
	hsBool PreConvert(plMaxNode* pNode, plErrorMsg* pErrMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
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
	hsBool IAddDirectMaster(plMaxNode* node, plSceneObject* so);
	hsBool IAddPointMaster(plMaxNode* node, plSceneObject* so);

public:
	plShadowLightComponent();

	hsBool SetupProperties(plMaxNode* pNode, plErrorMsg* pErrMsg);
	hsBool PreConvert(plMaxNode* pNode, plErrorMsg* pErrMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
};


#endif // plShadowComponents_inc
