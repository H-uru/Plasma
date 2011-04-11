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
#include "hsTypes.h"
#include "plMultipassMtl.h"
#include "plMultipassMtlPB.h"

ClassDesc2* GetMultiMtlDesc();

static ParamBlockDesc2 gMultipassMtlPB
(
	plMultipassMtl::kBlkPasses, _T("multipass"), 0, GetMultiMtlDesc(), 
	P_AUTO_CONSTRUCT, plMultipassMtl::kRefPasses, 

	kMultCount,			_T("numPasses"),	TYPE_INT,				0, 0,
		p_default,		0,
		end,

	kMultPasses,		_T("passes"),		TYPE_MTL_TAB, 0,		0, 0,
		end,
	kMultOn,			_T("passOn"),		TYPE_BOOL_TAB, 0,		0, 0,
		p_default,		TRUE,
		end,
	kMultLayerCounts,	_T("LayerCounts"),	TYPE_INT_TAB, 0,		0, 0,
		p_default,		0,
		end,
	end
);
