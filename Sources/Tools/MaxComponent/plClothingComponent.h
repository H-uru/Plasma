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
#ifndef PL_CLOTHING_COMPONENT_H
#define PL_CLOTHING_COMPONENT_H

#include "plComponent.h"

class plClothingComponent : public plComponent
{
public:
	plClothingComponent();

	virtual void DeleteThis() { delete this; }
	virtual hsBool SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg);
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg); 

	enum // ParamBlock indices
	{
		kMaterials,
		kGroup,
		kType,
		kLODState,
		kMeshNodeTab,
		kMeshNodeAddBtn,
	};
};

#define CLOTHING_COMPONENT_CLASS_ID Class_ID(0x2df85c56, 0x27bc2a7a)

class plClothingComponentProc : public ParamMap2UserDlgProc
{
public:
	plClothingComponentProc() {}

	BOOL DlgProc(TimeValue t, IParamMap2 *pm, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void DeleteThis() {}
};

#endif