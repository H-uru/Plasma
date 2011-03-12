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
//////////////////////////////////////////////////////////////////////////////
//																			//
//	pfGUIValueCtrl Definition												//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "pfGUIValueCtrl.h"
#include "pfGameGUIMgr.h"

#include "plgDispatch.h"
#include "hsResMgr.h"


//// Constructor/Destructor //////////////////////////////////////////////////

pfGUIValueCtrl::pfGUIValueCtrl()
{
	fValue = fMin = fMax = fStep = 0.f;
}

pfGUIValueCtrl::~pfGUIValueCtrl()
{
}

//// SetCurrValue ////////////////////////////////////////////////////////////

void	pfGUIValueCtrl::SetCurrValue( hsScalar v )
{
	fValue = v;
	if( fValue < fMin )
		fValue = fMin;
	else if( fValue > fMax )
		fValue = fMax;
}

//// SetRange ////////////////////////////////////////////////////////////////

void	pfGUIValueCtrl::SetRange( hsScalar min, hsScalar max )
{
	fMin = min; 
	fMax = max; 
	if( fValue < fMin )
		SetCurrValue( fMin );
	else if( fValue > fMax )
		SetCurrValue( fMax );
}

//// Read/Write //////////////////////////////////////////////////////////////

void	pfGUIValueCtrl::Read( hsStream *s, hsResMgr *mgr )
{
	pfGUIControlMod::Read(s, mgr);

	s->ReadSwap( &fMin );
	s->ReadSwap( &fMax );
	s->ReadSwap( &fStep );

	fValue = fMin;
}

void	pfGUIValueCtrl::Write( hsStream *s, hsResMgr *mgr )
{
	pfGUIControlMod::Write( s, mgr );

	s->WriteSwap( fMin );
	s->WriteSwap( fMax );
	s->WriteSwap( fStep );
}

