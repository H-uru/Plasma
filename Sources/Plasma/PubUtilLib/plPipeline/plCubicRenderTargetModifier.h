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
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  plCubicRenderTargetModifier Class Header                                 //
//  Modifier applied to an object to specify the source object to render a   //
//  cubicRenderTarget from. Owns a key to the cubicRenderTarget in question, //
//  which it updates with positions and submits renderRequests for.          //
//                                                                           //
//  Cyan, Inc.                                                               //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  7.20.2001 mcn - Created.                                                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plCubicRenderTargetModifier_h
#define _plCubicRenderTargetModifier_h

#include "pnModifier/plModifier.h"
#include "pnNetCommon/plSynchedValue.h"


class plCubicRenderTarget;
class plRenderRequest;

//// Class Definition /////////////////////////////////////////////////////////

class plCubicRenderTargetModifier : public plModifier
{

public:
    plCubicRenderTargetModifier();
    virtual ~plCubicRenderTargetModifier();

    CLASSNAME_REGISTER( plCubicRenderTargetModifier );
    GETINTERFACE_ANY( plCubicRenderTargetModifier, plModifier);

    // Functions related to/required by plModifier
    virtual int             GetNumTargets( void ) const { return fTarget ? 1 : 0; }
    virtual plSceneObject   *GetTarget( int w ) const { hsAssert(w < GetNumTargets(), "Bad target"); return fTarget; }
    virtual void            AddTarget( plSceneObject* so );
    virtual void            RemoveTarget( plSceneObject* so );

    virtual void    Read( hsStream* s, hsResMgr* mgr ); 
    virtual void    Write( hsStream* s, hsResMgr* mgr );
    virtual hsBool  MsgReceive( plMessage* msg );

protected:

    plSceneObject           *fTarget;
    plCubicRenderTarget     *fCubic;

    plRenderRequest         *fRequests[ 6 ];

    virtual hsBool  IEval( double secs, float del, uint32_t dirty ); // required by plModifier

    void            ICreateRenderRequest( int face );

};


#endif  //_plCubicRenderTargetModifier_h
