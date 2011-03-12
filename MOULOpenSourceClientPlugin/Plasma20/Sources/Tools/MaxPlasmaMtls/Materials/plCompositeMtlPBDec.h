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
#include "plCompositeMtl.h"
//#include "plCompositeMtlPB.h"

ClassDesc2* GetCompMtlDesc();

static ParamBlockDesc2 gCompositeMtlPB
(
	plCompositeMtl::kBlkPasses, _T("composite"), 0, GetCompMtlDesc(), 
	P_AUTO_CONSTRUCT, plCompositeMtl::kRefPasses, 

	plCompositeMtl::kCompPasses,		_T("passes"),			TYPE_MTL_TAB, 3,		0, 0,
		end,
	plCompositeMtl::kCompOn,			_T("passOn"),			TYPE_BOOL_TAB, 2,		0, 0,
		p_default,		TRUE,
		end,
	plCompositeMtl::kCompBlend,			_T("BlendMethod"),		TYPE_INT_TAB, 2,		0, 0,
		p_default,		0,
		end,
	plCompositeMtl::kCompUVChannels,	_T("UVChannels"),		TYPE_INT_TAB, 2,		0, 0,
		p_default,		0,
		end,
	plCompositeMtl::kCompLayerCounts,	_T("LayerCounts"),		TYPE_INT_TAB, 3,		0, 0,
		p_default,		0,
		end,
	end
);
