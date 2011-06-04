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
#ifndef _plSoftVolumeComponent_h
#define _plSoftVolumeComponent_h

#include "../pnKeyedObject/plKey.h"
#include "hsTemplates.h"
#include "../pnKeyedObject/plUoid.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
const Class_ID SOFTVOLUME_BASE_CID(0x3d9e214e, 0xadb41c9);
const Class_ID SOFTVOLUME_CID(0x39612efc, 0x2a23293b);
const Class_ID SOFTVOLUME_UNION_CID(0x54666594, 0xf543eea);
const Class_ID SOFTVOLUME_ISECT_CID(0x28755c38, 0x5f63259);
const Class_ID SOFTVOLUME_NEGATE_CID(0xe681c08, 0x7102cec);

const Class_ID LIGHTREGION_CID(0x5c17303, 0x315f2d51);
const Class_ID VISREGION_CID(0x5b144089, 0x5dc870a2);
const Class_ID RELREGION_CID(0x318e0a6d, 0x364a6a35);
const Class_ID EFFVISSET_CID(0x30141876, 0x3ffb3fda);

//// plSoftVolBaseComponent Declaration /////////////////////////////////////////////////////////

class plSoftVolume;
class plVisRegion;

class plSoftVolBaseComponent : public plComponent
{
protected:
	hsBool				fValid;
	plKey				fSoftKey;

	void				IAddSubVolume(plKey masterKey, plKey subKey);
	plKey				ISetVolumeKey(plSoftVolume* vol);
	plKey				IInvertVolume(plKey subKey);
	virtual plKey		ICreateSoftVolume() = 0;
public:

	hsBool SetupProperties(plMaxNode* pNode, plErrorMsg* errMsg);
	hsBool PreConvert(plMaxNode* pNode, plErrorMsg* errMsg) { return true; }
	hsBool Convert(plMaxNode *node, plErrorMsg *errMsg) { return true; }

	plKey GetSoftVolume(); 

	virtual hsBool DeInit(plMaxNode *node, plErrorMsg *pErrMsg);

	static plSoftVolBaseComponent* GetSoftComponent(INode* node);
	static plSoftVolBaseComponent* GetSoftComponent(plComponentBase *comp);
};

//// plSingleCompSelProc Declaration ////////////////////////////////////////////////////////////
//	Utilitiy class for any dialog that consists of a single button that is used to select
//	a soft volume. 
//	Just declare like this:
//		static plSingleCompSelProc gSoftVolNegateSingleSel( paramID, buttonCtrlID, promptString );
//	and use it as the dialog proc for your dialog's ParamMap.

class plSingleCompSelProc : public ParamMap2UserDlgProc
{
protected:
	ParamID			fNodeID;
	int				fDlgItem;
	TCHAR			fTitle[ 128 ];

public:
	plSingleCompSelProc(ParamID nodeID, int dlgItem, TCHAR *title);
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() {}
};

class plVisRegionComponent : public plComponent
{
public:
	enum {
		kSoftVolume,
		kAffectDraw,
		kAffectLight,
		kAffectOcc,
		kExcludes,
		kDisableNormal
	};
protected:
	plVisRegion*	fVisReg;

	void			ICheckVisRegion(const plLocation& loc);
public:
	plVisRegionComponent();
	void DeleteThis() { delete this; }

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	hsBool SetupProperties(plMaxNode *pNode, plErrorMsg *errMsg);

	hsBool PreConvert(plMaxNode *pNode, plErrorMsg *errMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *errMsg);

	static void CollectRegions(plMaxNode* node, hsTArray<plVisRegion*>& regions);
};

class plEffVisSetComponent : public plComponent
{
public:
	enum {
		kHideNormal
	};
protected:
	plVisRegion*		fVisReg;
public:
	plEffVisSetComponent();
	void DeleteThis() { delete this; }
	
	hsBool SetupProperties(plMaxNode* pNode, plErrorMsg* errMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *errMsg);
	
	plVisRegion* GetVisRegion(plMaxNode* node);
	
	static plEffVisSetComponent* ConvertToEffVisSetComponent(plMaxNode* node);
	static void CollectRegions(plMaxNode* node, hsTArray<plVisRegion*>& regions);
};

#endif //_plSoftVolumeComponent_h
