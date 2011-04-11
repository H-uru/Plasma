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
#ifndef PL_FOOTSTEP_COMPONENT_H
#define PL_FOOTSTEP_COMPONENT_H

#include "plComponent.h"

class plFootstepSoundComponent : public plComponent
{
public:
	plFootstepSoundComponent();

	virtual void DeleteThis() { delete this; }
	virtual hsBool SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg) { return true; }
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg) { return true; }
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg); 

	enum // ParamBlock indices
	{
		kSurface,
		kSurfaceList,
		kNodePicker,
	};
};

#define FOOTSTEP_SOUND_COMPONENT_CLASS_ID Class_ID(0x15c93f12, 0x4c3f050f)

class plFootstepSoundComponentProc : public ParamMap2UserDlgProc
{
public:
	plFootstepSoundComponentProc() {}

	BOOL DlgProc(TimeValue t, IParamMap2 *pm, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void DeleteThis() {}
};

#endif