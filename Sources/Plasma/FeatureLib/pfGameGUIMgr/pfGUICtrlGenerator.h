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
//  pfGUICtrlGenerator Header                                               //
//  Generates really primitive GUI controls (and dialogs) at runtime.       //
//  Useful for, well, generating really primitive GUI controls and dialogs  //
//  at runtime...
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfGUICtrlGenerator_h
#define _pfGUICtrlGenerator_h

#include "HeadSpin.h"

#include <string_theory/string>
#include <vector>

//// pfGUICtrlGenerator Definition ///////////////////////////////////////////

struct hsColorRGBA;
class plDrawable;
class hsGMaterial;
class pfGUIButtonMod;
class pfGUIDialogMod;
class pfGUIDragBarCtrl;
class hsKeyedObject;
class plKey;
struct hsMatrix44;
class plSceneNode;
class plSceneObject;
class plTextGenerator;

class pfGUICtrlGenerator
{
    protected:

        char    fFontFace[ 256 ];
        uint32_t  fFontSize;

        std::vector<plTextGenerator *> fTextGens;

        std::vector<plSceneNode *>     fDynDlgNodes;
        std::vector<pfGUIDialogMod *>  fDynDialogs;
        std::vector<plSceneObject *>   fDynDragBars;


        plKey       IAddKey( hsKeyedObject *ko, const char *prefix );
        ST::string  IGetNextKeyName( const char *prefix );

        hsGMaterial *ICreateSolidMaterial( hsColorRGBA &color );

        hsGMaterial *ICreateTextMaterial( const char *text, hsColorRGBA &bgColor, 
                                                 hsColorRGBA &textColor, float objWidth, float objHeight );

        pfGUIDialogMod  *IGetDialog();
        pfGUIDialogMod  *IGenerateDialog( const char *name, float scrnWidth, bool show = true );

        plSceneObject   *IGenSceneObject(pfGUIDialogMod *dlg, plDrawable *myDraw,
                                         plSceneObject *parent = nullptr,
                                         hsMatrix44 *l2w = nullptr,
                                         hsMatrix44 *w2l = nullptr);

    public:
        
        pfGUICtrlGenerator();
        ~pfGUICtrlGenerator();

        void    Shutdown();

        void            SetFont( const char *face, uint16_t size );


        pfGUIButtonMod  *GenerateRectButton( const char *title, float x, float y, float width, float height,
                                                const char *consoleCmd, hsColorRGBA &color, hsColorRGBA &textColor );

        pfGUIButtonMod  *GenerateSphereButton( float x, float y, float radius,
                                                const char *consoleCmd, hsColorRGBA &color );

        pfGUIDragBarCtrl *GenerateDragBar( float x, float y, float width, float height, hsColorRGBA &color );

        void            GenerateDialog( const char *name );


        pfGUIButtonMod  *CreateRectButton( pfGUIDialogMod *parent, float x, float y,
                                                float width, float height, hsGMaterial *material, bool asMenuItem = false );

        static pfGUICtrlGenerator   &Instance();
};

#endif // _pfGUICtrlGenerator_h
