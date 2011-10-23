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
//  pfGUIDynDisplayCtrl Header                                              //
//                                                                          //
//  Fun little helper control that just stores a pointer to a single        //
//  plDynamicTextMap, chosen in MAX. Note that we could also just search    //
//  for the right key name, but that requires a StupidSearch(tm), while     //
//  this way just requires an extra dummy control that automatically reads  //
//  in the right ref (and searching for controls by TagID is a lot faster   //
//  than searching for keys).                                               //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfGUIDynDisplayCtrl_h
#define _pfGUIDynDisplayCtrl_h

#include "pfGUIControlMod.h"
#include "hsTemplates.h"

class plMessage;
class plDynamicTextMap;
class plLayerInterface;
class hsGMaterial;

class pfGUIDynDisplayCtrl : public pfGUIControlMod
{
    protected:

        enum
        {
            kRefTextMap = kRefDerivedStart,
            kRefLayer,
            kRefMaterial
        };

        hsTArray<plDynamicTextMap *>    fTextMaps;
        hsTArray<plLayerInterface *>    fLayers;

        hsTArray<hsGMaterial *>         fMaterials;

        virtual hsBool IEval( double secs, hsScalar del, UInt32 dirty ); // called only by owner object's Eval()

    public:

        pfGUIDynDisplayCtrl();
        virtual ~pfGUIDynDisplayCtrl();

        CLASSNAME_REGISTER( pfGUIDynDisplayCtrl );
        GETINTERFACE_ANY( pfGUIDynDisplayCtrl, pfGUIControlMod );


        virtual hsBool  MsgReceive( plMessage* pMsg );
        
        virtual void Read( hsStream* s, hsResMgr* mgr );
        virtual void Write( hsStream* s, hsResMgr* mgr );

        UInt32              GetNumMaps( void ) const { return fTextMaps.GetCount(); }
        plDynamicTextMap    *GetMap( UInt32 i ) const { return fTextMaps[ i ]; }

        UInt32              GetNumLayers( void ) const { return fLayers.GetCount(); }
        plLayerInterface    *GetLayer( UInt32 i ) const { return fLayers[ i ]; }

        UInt32              GetNumMaterials( void ) const { return fMaterials.GetCount(); }
        hsGMaterial         *GetMaterial( UInt32 i ) const { return fMaterials[ i ]; }

        // Export only
        void    AddMap( plDynamicTextMap *map );
        void    AddLayer( plLayerInterface *layer );
        void    AddMaterial( hsGMaterial *material );
};

#endif // _pfGUIDynDisplayCtrl_h
