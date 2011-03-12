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
//	pfPlayerBookMod Definition												//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "pfPlayerBookMod.h"
#include "../pfGameGUIMgr/pfGameGUIMgr.h"
#include "../pfGameGUIMgr/pfGUIButtonMod.h"
#include "../pfGameGUIMgr/pfGUICheckBoxCtrl.h"

#include "../pnMessage/plRefMsg.h"
#include "plgDispatch.h"
#include "hsResMgr.h"


//// Constructor/Destructor //////////////////////////////////////////////////

pfPlayerBookMod::pfPlayerBookMod()
{
	int		i;


	for( i = 0; i < 6; i++ )
	{
		fCheckBoxes[ i ] = nil;
		fDynLayerKeys[ i ] = nil;
	}

	fLoadButton = nil;
	fSaveButton = nil;

	fPBProc = nil;
}

pfPlayerBookMod::~pfPlayerBookMod()
{
}

//// IEval ///////////////////////////////////////////////////////////////////

hsBool	pfPlayerBookMod::IEval( double secs, hsScalar del, UInt32 dirty )
{
	return false;
}

//// MsgReceive //////////////////////////////////////////////////////////////

hsBool	pfPlayerBookMod::MsgReceive( plMessage *msg )
{
	plGenRefMsg	*refMsg = plGenRefMsg::ConvertNoRef( msg );
	if( refMsg != nil )
	{
		if( refMsg->fType == kRefCheckBox )
		{
			if( refMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
				fCheckBoxes[ refMsg->fWhich ] = pfGUICheckBoxCtrl::ConvertNoRef( refMsg->GetRef() );
			else
				fCheckBoxes[ refMsg->fWhich ] = nil;
		}
		else if( refMsg->fType == kRefLoadButton )
		{
			if( refMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
				fLoadButton = pfGUIButtonMod::ConvertNoRef( refMsg->GetRef() );
			else
				fLoadButton = nil;
		}
		else if( refMsg->fType == kRefSaveButton )
		{
			if( refMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
				fSaveButton = pfGUIButtonMod::ConvertNoRef( refMsg->GetRef() );
			else
				fSaveButton = nil;
		}
		return true;
	}

	return plSingleModifier::MsgReceive( msg );
}

//// Read/Write //////////////////////////////////////////////////////////////

void	pfPlayerBookMod::Read( hsStream *s, hsResMgr *mgr )
{
	int		i;


	plSingleModifier::Read(s, mgr);

	for( i = 0; i < 6; i++ )
		mgr->ReadKeyNotifyMe( s, new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, i, kRefCheckBox ), plRefFlags::kActiveRef );

	for( i = 0; i < 6; i++ )
		fDynLayerKeys[ i ] = mgr->ReadKey( s );

	mgr->ReadKeyNotifyMe( s, new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, kRefLoadButton ), plRefFlags::kActiveRef );
	mgr->ReadKeyNotifyMe( s, new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, kRefSaveButton ), plRefFlags::kActiveRef );
}

void	pfPlayerBookMod::Write( hsStream *s, hsResMgr *mgr )
{
	int		i;


	plSingleModifier::Write( s, mgr );

	for( i = 0; i < 6; i++ )
		mgr->WriteKey( s, fCheckBoxes[ i ]->GetKey() );

	for( i = 0; i < 6; i++ )
		mgr->WriteKey( s, fDynLayerKeys[ i ] );

	mgr->WriteKey( s, fLoadButton->GetKey() );
	mgr->WriteKey( s, fSaveButton->GetKey() );
}

