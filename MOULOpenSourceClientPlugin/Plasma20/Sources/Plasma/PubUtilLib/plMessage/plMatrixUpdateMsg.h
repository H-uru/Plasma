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

#ifndef plMatrixUpdateMsg_inc
#define plMatrixUpdateMsg_inc

#include "../pnMessage/plMessage.h"
#include "hsMatrix44.h"

class plMatrixUpdateMsg : public plMessage
{

public:
	plMatrixUpdateMsg(){SetBCastFlag(plMessage::kPropagateToModifiers);}
	plMatrixUpdateMsg(const plKey &s, 
					const plKey &r, 
					const double* t){SetBCastFlag(plMessage::kPropagateToModifiers);}
	~plMatrixUpdateMsg(){;}

	CLASSNAME_REGISTER( plMatrixUpdateMsg );
	GETINTERFACE_ANY( plMatrixUpdateMsg, plMessage );

	void Read( hsStream* s, hsResMgr* mgr );
	void Write( hsStream* s, hsResMgr* mgr );


	hsMatrix44	fMatrix;

};

#endif plMatrixUpdateMsg_inc
