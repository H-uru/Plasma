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
#ifndef plCCRMgr_h
#define plCCRMgr_h

//
// Implementation for CCR commands
//

#include "hsTypes.h"

// Error constants and conversion are outside of the CCR_RELEASE define,
// So that non-CCR code can report CCR errors, and the plCCRMgr can
// share this code.
namespace plCCRError
{
	enum Errors
	{
		kError			= hsFail,
		kNotAuthorized	= -2,
		kNilLocalAvatar	= -3,
		kCCRAlreadyAllocated = -4,
		kNetworkingIsDisabled = -5,
		kCantFindPlayer	= -6,
		kInvalidLevel	= -7,
		kPlayerNotInAge = -8,
		kVaultTimedOut	= -9,
		kVaultFetchFailed = -10,
		kAuthTimedOut	= -11		
	};
}

#endif	// plCCRMgr_h
