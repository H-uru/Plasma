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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  pfGUIUpDownPairMod Definition                                           //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "pfGUIUpDownPairMod.h"

#include "HeadSpin.h"
#include "hsResMgr.h"
#include "hsStream.h"

#include "pfGameGUIMgr.h"
#include "pfGUIButtonMod.h"
#include "pfGUIControlHandlers.h"

#include "pnMessage/plRefMsg.h"

//// Wee Little Control Proc for our buttons /////////////////////////////////

class pfUpDownBtnProc : public pfGUICtrlProcObject
{
    protected:

        pfGUIButtonMod      *fUp, *fDown;
        pfGUIUpDownPairMod  *fParent;

    public:

        pfUpDownBtnProc( pfGUIButtonMod *up, pfGUIButtonMod *down, pfGUIUpDownPairMod *parent )
        {
            fUp = up;
            fDown = down;
            fParent = parent;
        }

        void    SetUp( pfGUIButtonMod *up ) { fUp = up; }
        void    SetDown( pfGUIButtonMod *down ) { fDown = down; }

        void    DoSomething(pfGUIControlMod *ctrl) override
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
    : fUpControl(), fDownControl()
{
    fValue = fMin = fMax = fStep = 0.f;

    fButtonProc = new pfUpDownBtnProc(nullptr, nullptr, this);
    fButtonProc->IncRef();
    SetFlag( kIntangible );
}

pfGUIUpDownPairMod::~pfGUIUpDownPairMod()
{
    if( fButtonProc->DecRef() )
        delete fButtonProc;
}

//// IEval ///////////////////////////////////////////////////////////////////

bool    pfGUIUpDownPairMod::IEval( double secs, float del, uint32_t dirty )
{
    return pfGUIValueCtrl::IEval( secs, del, dirty );
}

void    pfGUIUpDownPairMod::IUpdate()
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

void    pfGUIUpDownPairMod::Update()
{
    IUpdate();
}

//// MsgReceive //////////////////////////////////////////////////////////////

bool    pfGUIUpDownPairMod::MsgReceive( plMessage *msg )
{
    plGenRefMsg *refMsg = plGenRefMsg::ConvertNoRef( msg );
    if (refMsg != nullptr)
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
                fUpControl = nullptr;
                fButtonProc->SetUp(nullptr);
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
                fDownControl = nullptr;
                fButtonProc->SetDown(nullptr);
            }
            return true;
        }
    }

    return pfGUIValueCtrl::MsgReceive( msg );
}

//// Read/Write //////////////////////////////////////////////////////////////

void    pfGUIUpDownPairMod::Read( hsStream *s, hsResMgr *mgr )
{
    pfGUIValueCtrl::Read(s, mgr);

    fUpControl = nullptr;
    fDownControl = nullptr;
    mgr->ReadKeyNotifyMe( s, new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, kRefUpControl ), plRefFlags::kActiveRef );
    mgr->ReadKeyNotifyMe( s, new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, kRefDownControl ), plRefFlags::kActiveRef );

    s->ReadLEFloat(&fMin);
    s->ReadLEFloat(&fMax);
    s->ReadLEFloat(&fStep);

    fValue = fMin;
}

void    pfGUIUpDownPairMod::Write( hsStream *s, hsResMgr *mgr )
{
    pfGUIValueCtrl::Write( s, mgr );

    mgr->WriteKey( s, fUpControl->GetKey() );
    mgr->WriteKey( s, fDownControl->GetKey() );

    s->WriteLEFloat(fMin);
    s->WriteLEFloat(fMax);
    s->WriteLEFloat(fStep);
}


void    pfGUIUpDownPairMod::SetRange( float min, float max )
{
    pfGUIValueCtrl::SetRange( min, max );
    IUpdate();
}

void    pfGUIUpDownPairMod::SetCurrValue( float v )
{
    pfGUIValueCtrl::SetCurrValue( v );
    IUpdate();
}
