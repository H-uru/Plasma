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
//  pfGUICheckBoxCtrl Header                                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfGUICheckBoxCtrl_h
#define _pfGUICheckBoxCtrl_h

#include "pfGUIControlMod.h"

class plMessage;
class plPostEffectMod;
class plAGMasterMod;

class pfGUICheckBoxCtrl : public pfGUIControlMod
{
    protected:

        hsTArray<plKey> fAnimationKeys;
        char            *fAnimName;
        hsBool          fClicking;

        hsBool          fChecked;
        hsBool          fPlaySound;

        virtual hsBool IEval( double secs, hsScalar del, UInt32 dirty ); // called only by owner object's Eval()

        virtual UInt32      IGetDesiredCursor( void ) const;    // As specified in plInputInterface.h

    public:

        pfGUICheckBoxCtrl();
        virtual ~pfGUICheckBoxCtrl();

        CLASSNAME_REGISTER( pfGUICheckBoxCtrl );
        GETINTERFACE_ANY( pfGUICheckBoxCtrl, pfGUIControlMod );


        virtual hsBool  MsgReceive( plMessage* pMsg );
        
        virtual void Read( hsStream* s, hsResMgr* mgr );
        virtual void Write( hsStream* s, hsResMgr* mgr );

        virtual void    HandleMouseDown( hsPoint3 &mousePt, UInt8 modifiers );
        virtual void    HandleMouseUp( hsPoint3 &mousePt, UInt8 modifiers );

        virtual void    UpdateBounds( hsMatrix44 *invXformMatrix = nil, hsBool force = false );

        void        SetChecked( hsBool checked, hsBool immediate = false );
        hsBool      IsChecked( void ) { return fChecked; }

        void DontPlaySounds() { fPlaySound = false; } // should the checkbox play sounds?
        
        const hsTArray<plKey>   &GetAnimationKeys( void ) const { return fAnimationKeys; }
        const char              *GetAnimationName( void ) const { return fAnimName; }

        enum SoundEvents
        {
            kMouseDown,
            kMouseUp,
            kMouseOver,
            kMouseOff
        };

        // Export only
        void    SetAnimationKeys( hsTArray<plKey> &keys, const char *name );
};

#endif // _pfGUICheckBoxCtrl_h
