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
//	pfGUIUpDownPairMod Definition											//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "pfGUIUpDownPairMod.h"
#include "pfGameGUIMgr.h"
#include "pfGUIButtonMod.h"
#include "pfGUIControlHandlers.h"

#include "../pnMessage/plRefMsg.h"
#include "../pfMessage/pfGameGUIMsg.h"
#include "../plMessage/plAnimCmdMsg.h"
#include "../plAvatar/plAGModifier.h"
#include "plgDispatch.h"
#include "hsResMgr.h"

//// Wee Little Control Proc for our buttons /////////////////////////////////

class pfUpDownBtnProc : public pfGUICtrlProcObject
{
	protected:

		pfGUIButtonMod		*fUp, *fDown;
		pfGUIUpDownPairMod	*fParent;

	public:

		pfUpDownBtnProc( pfGUIButtonMod *up, pfGUIButtonMod *down, pfGUIUpDownPairMod *parent )
		{
			fUp = up;
			fDown = down;
			fParent = parent;
		}

		void	SetUp( pfGUIButtonMod *up ) { fUp = up; }
		void	SetDown( pfGUIButtonMod *down ) { fDown = down; }

		virtual void	DoSomething( pfGUIControlMod *ctrl )
		{
			if( (pfGUIButtonMod *)ctrl == fUp )
			{
				fParent->fValue += fParent->fStep;
				if( fParent->fValue > fParent->fMax )
					fParent->fValue = fParent->fMax;
			}
			else
			{
				fParent->fValue -= fParent->fStep;
				if( fParent->fValue < fParent->fMin )
					fParent->fValue = fParent->fMin;
			}
			fParent->Update();
			fParent->DoSomething();
		}
};

//// Constructor/Destructor //////////////////////////////////////////////////

pfGUIUpDownPairMod::pfGUIUpDownPairMod()
{
	fUpControl = nil;
	fDownControl = nil;
	fValue = fMin = fMax = fStep = 0.f;

	fButtonProc = TRACKED_NEW pfUpDownBtnProc( nil, nil, this );
	fButtonProc->IncRef();
	SetFlag( kIntangible );
}

pfGUIUpDownPairMod::~pfGUIUpDownPairMod()
{
	if( fButtonProc->DecRef() )
		delete fButtonProc;
}

//// IEval ///////////////////////////////////////////////////////////////////

hsBool	pfGUIUpDownPairMod::IEval( double secs, hsScalar del, UInt32 dirty )
{
	return pfGUIValueCtrl::IEval( secs, del, dirty );
}

void	pfGUIUpDownPairMod::IUpdate( void )
{
	if (fEnabled)
	{
		if (fUpControl)
		{
			if ( fValue >= fMax)
				fUpControl->SetVisible(false);
			else
				fUpControl->SetVisible(true);
		}
		if (fDownControl)
		{
			if ( fValue <= fMin )
				fDownControl->SetVisible(false);
			else
				fDownControl->SetVisible(true);
		}
	}
	else
	{
		fUpControl->SetVisible(false);
		fDownControl->SetVisible(false);
	}
}

void	pfGUIUpDownPairMod::Update( void )
{
	IUpdate();
}

//// MsgReceive //////////////////////////////////////////////////////////////

hsBool	pfGUIUpDownPairMod::MsgReceive( plMessage *msg )
{
	plGenRefMsg	*refMsg = plGenRefMsg::ConvertNoRef( msg );
	if( refMsg != nil )
	{
		if( refMsg->fType == kRefUpControl )
		{
			if( refMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
			{
				fUpControl = pfGUIButtonMod::ConvertNoRef( refMsg->GetRef() );
				fUpControl->SetHandler( fButtonProc );
				fButtonProc->SetUp( fUpControl );
			}
			else
			{
				fUpControl = nil;
				fButtonProc->SetUp( nil );
			}
			return true;
		}
		else if( refMsg->fType == kRefDownControl )
		{
			if( refMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
			{
				fDownControl = pfGUIButtonMod::ConvertNoRef( refMsg->GetRef() );
				fDownControl->SetHandler( fButtonProc );
				fButtonProc->SetDown( fDownControl );
			}
			else
			{
				fDownControl = nil;
				fButtonProc->SetDown( nil );
			}
			return true;
		}
	}

	return pfGUIValueCtrl::MsgReceive( msg );
}

//// Read/Write //////////////////////////////////////////////////////////////

void	pfGUIUpDownPairMod::Read( hsStream *s, hsResMgr *mgr )
{
	pfGUIValueCtrl::Read(s, mgr);

	fUpControl = nil;
	fDownControl = nil;
	mgr->ReadKeyNotifyMe( s, TRACKED_NEW plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, kRefUpControl ), plRefFlags::kActiveRef );
	mgr->ReadKeyNotifyMe( s, TRACKED_NEW plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, kRefDownControl ), plRefFlags::kActiveRef );

	s->ReadSwap( &fMin );
	s->ReadSwap( &fMax );
	s->ReadSwap( &fStep );

	fValue = fMin;
}

void	pfGUIUpDownPairMod::Write( hsStream *s, hsResMgr *mgr )
{
	pfGUIValueCtrl::Write( s, mgr );

	mgr->WriteKey( s, fUpControl->GetKey() );
	mgr->WriteKey( s, fDownControl->GetKey() );

	s->WriteSwap( fMin );
	s->WriteSwap( fMax );
	s->WriteSwap( fStep );
}


void	pfGUIUpDownPairMod::SetRange( hsScalar min, hsScalar max )
{
	pfGUIValueCtrl::SetRange( min, max );
	IUpdate();
}

void	pfGUIUpDownPairMod::SetCurrValue( hsScalar v )
{
	pfGUIValueCtrl::SetCurrValue( v );
	IUpdate();
}
