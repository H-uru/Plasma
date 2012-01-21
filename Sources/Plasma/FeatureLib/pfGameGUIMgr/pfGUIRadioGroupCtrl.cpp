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
//  pfGUIRadioGroupCtrl Definition                                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "pfGUIRadioGroupCtrl.h"
#include "pfGameGUIMgr.h"
#include "pfGUICheckBoxCtrl.h"
#include "pfGUIControlHandlers.h"

#include "pnMessage/plRefMsg.h"
#include "pfMessage/pfGameGUIMsg.h"
#include "plgDispatch.h"
#include "hsResMgr.h"

//// Wee Little Control Proc for our buttons /////////////////////////////////

class pfGroupProc : public pfGUICtrlProcObject
{
    protected:

        pfGUIRadioGroupCtrl *fParent;

    public:

        pfGroupProc( pfGUIRadioGroupCtrl *parent )
        {
            fParent = parent;
        }

        virtual void    DoSomething( pfGUIControlMod *ctrl )
        {
            int32_t   newIdx;


            // So one of our controls got clicked. That means that we change our value
            // to the proper index
            
            pfGUICheckBoxCtrl *check = pfGUICheckBoxCtrl::ConvertNoRef( ctrl );

            // Are we unselecting? And do we allow this?
            if( !check->IsChecked() && !fParent->HasFlag( pfGUIRadioGroupCtrl::kAllowNoSelection ) )
            {
                // Boo on you. Re-check
                check->SetChecked( true );
                return;
            }

            for( newIdx = 0; newIdx < fParent->fControls.GetCount(); newIdx++ )
            {
                if( fParent->fControls[ newIdx ] == check )
                    break;
            }

            if( newIdx == fParent->fControls.GetCount() )
                newIdx = -1;

            if( newIdx != fParent->fValue )
            {
                if( fParent->fValue != -1 )
                    fParent->fControls[ fParent->fValue ]->SetChecked( false );

                fParent->fValue = newIdx;
                if( newIdx != -1 )
                    fParent->fControls[ newIdx ]->SetChecked( true );
            }
            else
            {
                if( !check->IsChecked() && fParent->HasFlag( pfGUIRadioGroupCtrl::kAllowNoSelection ) )
                {
                    // nobody is checked!
                    fParent->fValue = -1;
                }
            }

            fParent->DoSomething();
        }
};

//// Constructor/Destructor //////////////////////////////////////////////////

pfGUIRadioGroupCtrl::pfGUIRadioGroupCtrl()
{
    fButtonProc = new pfGroupProc( this );
    fButtonProc->IncRef();
    SetFlag( kIntangible );
}

pfGUIRadioGroupCtrl::~pfGUIRadioGroupCtrl()
{
    if( fButtonProc->DecRef() )
        delete fButtonProc;
}

//// IEval ///////////////////////////////////////////////////////////////////

hsBool  pfGUIRadioGroupCtrl::IEval( double secs, float del, uint32_t dirty )
{
    return pfGUIControlMod::IEval( secs, del, dirty );
}

//// MsgReceive //////////////////////////////////////////////////////////////

hsBool  pfGUIRadioGroupCtrl::MsgReceive( plMessage *msg )
{
    plGenRefMsg *refMsg = plGenRefMsg::ConvertNoRef( msg );
    if( refMsg != nil )
    {
        if( refMsg->fType == kRefControl )
        {
            if( refMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
            {
                fControls[ refMsg->fWhich ] = pfGUICheckBoxCtrl::ConvertNoRef( refMsg->GetRef() );
                fControls[ refMsg->fWhich ]->SetHandler( fButtonProc );
                if( fValue == refMsg->fWhich )
                    fControls[ refMsg->fWhich ]->SetChecked( true );
            }
            else
            {
                fControls[ refMsg->fWhich ] = nil;
            }
            return true;
        }
    }

    return pfGUIControlMod::MsgReceive( msg );
}

//// Read/Write //////////////////////////////////////////////////////////////

void    pfGUIRadioGroupCtrl::Read( hsStream *s, hsResMgr *mgr )
{
    pfGUIControlMod::Read(s, mgr);

    uint32_t  i, count = s->ReadLE32();
    fControls.SetCountAndZero( count );

    for( i = 0; i < count; i++ )
    {
        mgr->ReadKeyNotifyMe( s, new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, i, kRefControl ), plRefFlags::kActiveRef );
    }

    fValue = fDefaultValue = s->ReadLE16();
    if( fValue != -1 && fControls[ fValue ] != nil )
        fControls[ fValue ]->SetChecked( true );
}

void    pfGUIRadioGroupCtrl::Write( hsStream *s, hsResMgr *mgr )
{
    uint32_t  i;


    pfGUIControlMod::Write( s, mgr );

    s->WriteLE32( fControls.GetCount() );
    for( i = 0; i < fControls.GetCount(); i++ )
        mgr->WriteKey( s, fControls[ i ]->GetKey() );

    s->WriteLE16( (uint16_t)fDefaultValue );
}

//// SetValue ////////////////////////////////////////////////////////////////

void    pfGUIRadioGroupCtrl::SetValue( int32_t value )
{
    if( value != fValue && ( value != -1 || HasFlag( kAllowNoSelection ) ) )
    {
        if( fValue != -1 )
            fControls[ fValue ]->SetChecked( false );

        fValue = value;
        if( value != -1 )
            fControls[ value ]->SetChecked( true );

        DoSomething();
    }
}

///// Setting to be trickled down to the underlings

void    pfGUIRadioGroupCtrl::SetEnabled( hsBool e )
{
    int i;
    for( i = 0; i < fControls.GetCount(); i++ )
        fControls[ i ]->SetEnabled(e);
}

void    pfGUIRadioGroupCtrl::SetInteresting( hsBool e )
{
    int i;
    for( i = 0; i < fControls.GetCount(); i++ )
        fControls[ i ]->SetInteresting(e);
}

void    pfGUIRadioGroupCtrl::SetVisible( hsBool vis )
{
    int i;
    for( i = 0; i < fControls.GetCount(); i++ )
        fControls[ i ]->SetVisible(vis);
}

void    pfGUIRadioGroupCtrl::SetControlsFlag( int flag )
{
    int i;
    for( i = 0; i < fControls.GetCount(); i++ )
        fControls[ i ]->SetFlag(flag);
}


void    pfGUIRadioGroupCtrl::ClearControlsFlag( int flag )
{
    int i;
    for( i = 0; i < fControls.GetCount(); i++ )
        fControls[ i ]->ClearFlag(flag);
}


//// Export Functions ////////////////////////////////////////////////////////

void    pfGUIRadioGroupCtrl::ClearControlList( void )
{
    fControls.Reset();
    fValue = -1;
}

void    pfGUIRadioGroupCtrl::AddControl( pfGUICheckBoxCtrl *ctrl )
{
    fControls.Append( ctrl );
}

