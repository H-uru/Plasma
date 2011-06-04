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
//	pfGUICtrlGenerator Header 												//
//	Generates really primitive GUI controls (and dialogs) at runtime.		//
//	Useful for, well, generating really primitive GUI controls and dialogs	//
//	at runtime...
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfGUICtrlGenerator_h
#define _pfGUICtrlGenerator_h

#include "hsStream.h"
#include "hsTemplates.h"


//// pfGUICtrlGenerator Definition ///////////////////////////////////////////

class pfGUIDialogMod;
class pfGUIButtonMod;
class pfGUIDragBarCtrl;
class hsGMaterial;
struct hsColorRGBA;
class plSceneNode;
class hsKeyedObject;
class plKey;
class plTextGenerator;
class plSceneObject;
class plDrawable;
struct hsMatrix44;

class pfGUICtrlGenerator
{
	protected:

		char	fFontFace[ 256 ];
		UInt32	fFontSize;

		hsTArray<plTextGenerator *>	fTextGens;

		hsTArray<plSceneNode *>		fDynDlgNodes;
		hsTArray<pfGUIDialogMod *>	fDynDialogs;
		hsTArray<plSceneObject *>	fDynDragBars;


		plKey		IAddKey( hsKeyedObject *ko, const char *prefix );
		void		IGetNextKeyName( char *name, const char *prefix );

		hsGMaterial	*ICreateSolidMaterial( hsColorRGBA &color );

		hsGMaterial	*ICreateTextMaterial( const char *text, hsColorRGBA &bgColor, 
												 hsColorRGBA &textColor, float objWidth, float objHeight );

		pfGUIDialogMod	*IGetDialog( void );
		pfGUIDialogMod	*IGenerateDialog( const char *name, float scrnWidth, hsBool show = true );

		plSceneObject	*IGenSceneObject( pfGUIDialogMod *dlg, plDrawable *myDraw, plSceneObject *parent = nil, hsMatrix44 *l2w = nil, hsMatrix44 *w2l = nil );

	public:
		
		pfGUICtrlGenerator();
		~pfGUICtrlGenerator();

		void	Shutdown( void );

		void			SetFont( const char *face, UInt16 size );


		pfGUIButtonMod	*GenerateRectButton( const char *title, float x, float y, float width, float height,
												const char *consoleCmd, hsColorRGBA &color, hsColorRGBA &textColor );

		pfGUIButtonMod	*GenerateSphereButton( float x, float y, float radius,
												const char *consoleCmd, hsColorRGBA &color );

		pfGUIDragBarCtrl *GenerateDragBar( float x, float y, float width, float height, hsColorRGBA &color );

		void			GenerateDialog( const char *name );


		pfGUIButtonMod	*CreateRectButton( pfGUIDialogMod *parent, const char *title, float x, float y, 
												float width, float height, hsGMaterial *material, hsBool asMenuItem = false );
		pfGUIButtonMod	*CreateRectButton( pfGUIDialogMod *parent, const wchar_t *title, float x, float y,
												float width, float height, hsGMaterial *material, hsBool asMenuItem = false );

		static pfGUICtrlGenerator	&Instance( void );
};

#endif // _pfGUICtrlGenerator_h
