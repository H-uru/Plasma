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
#include "plPassMtl.h"
#include "plPassBaseParamIDs.h"

using namespace plPassBaseParamIDs;

static ParamBlockDesc2 gPassAdvPB
(
	plPassMtl::kBlkAdv, _T("advanced"), IDS_PASS_ADV, GetPassMtlDesc(),
	P_AUTO_CONSTRUCT + P_AUTO_UI, plPassMtl::kRefAdv,

	// UI
	IDD_PASS_ADV, IDS_PASS_ADV, 0, APPENDROLL_CLOSED, NULL,

	// Misc Properties
	kPBAdvWire,			_T("basicWire"),	TYPE_BOOL,		0, 0,
		p_ui,				TYPE_SINGLECHEKBOX, IDC_MISC_WIRE,
		end,
	kPBAdvMeshOutlines,	_T("meshOutlines"),	TYPE_BOOL,		0, 0,
		p_ui,				TYPE_SINGLECHEKBOX, IDC_MISC_MESHOUTLINES,
		end,
	kPBAdvTwoSided,		_T("twoSided"),		TYPE_BOOL,		0, 0,
		p_ui,				TYPE_SINGLECHEKBOX, IDC_MISC_TWOSIDED,
		end,

	// Shade properties
	kPBAdvSoftShadow,		_T("softShadow"),	TYPE_BOOL,		0, 0,
		p_ui,				TYPE_SINGLECHEKBOX, IDC_SHADE_SOFTSHADOW,
		end,
	kPBAdvNoProj,			_T("noProj"),		TYPE_BOOL,		0, 0,
		p_ui,				TYPE_SINGLECHEKBOX, IDC_SHADE_NO_PROJ,
		end,
	kPBAdvVertexShade,	_T("vertexShade"),	TYPE_BOOL,		0, 0,
		p_ui,				TYPE_SINGLECHEKBOX, IDC_SHADE_VERTEXSHADE,
		end,
	kPBAdvNoShade,		_T("noShade"),		TYPE_BOOL,		0, 0,
		p_ui,				TYPE_SINGLECHEKBOX, IDC_SHADE_NOSHADE,
		end,
	kPBAdvNoFog,			_T("noFog"),		TYPE_BOOL,		0, 0,
		p_ui,				TYPE_SINGLECHEKBOX, IDC_SHADE_NO_FOG,
		end,
	kPBAdvWhite,			_T("white"),		TYPE_BOOL,		0, 0,
		p_ui,				TYPE_SINGLECHEKBOX, IDC_SHADE_WHITE,
		end,

	// Z Properties
	kPBAdvZOnly,		_T("zOnly"),		TYPE_BOOL,		0, 0,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_Z_ZONLY,
		end,
	kPBAdvZClear,		_T("zClear"),		TYPE_BOOL,		0, 0,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_Z_ZCLEAR,
		end,
	kPBAdvZNoRead,		_T("zNoRead"),		TYPE_BOOL,		0, 0,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_Z_ZNOREAD,
		end,
	kPBAdvZNoWrite,		_T("zNoWrite"),		TYPE_BOOL,		0, 0,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_Z_ZNOWRITE,
		end,
	kPBAdvZInc,			_T("zInc"),			TYPE_BOOL,		0, 0,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_Z_INC,
		end,

	kPBAdvAlphaTestHigh,			_T("aTestHigh"),			TYPE_BOOL,		0, 0,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_ALPHA_TEST_HIGH,
		p_default,		FALSE,
		end,

	end
);
ParamBlockDesc2 *GetPassAdvPB() { return &gPassAdvPB; }