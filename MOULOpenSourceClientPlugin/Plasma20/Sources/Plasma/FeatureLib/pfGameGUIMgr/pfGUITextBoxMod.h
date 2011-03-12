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
//	pfGUITextBoxMod Header													//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfGUITextBoxMod_h
#define _pfGUITextBoxMod_h

#include "pfGUIControlMod.h"

class plMessage;
class hsGMaterial;
class plTextGenerator;

class pfGUITextBoxMod : public pfGUIControlMod
{
	protected:

		wchar_t			*fText;
		std::wstring	fLocalizationPath;
		bool			fUseLocalizationPath;


		virtual hsBool IEval( double secs, hsScalar del, UInt32 dirty ); // called only by owner object's Eval()

		virtual void	IUpdate( void );
		virtual void	IPostSetUpDynTextMap( void );

	public:

		pfGUITextBoxMod();
		virtual ~pfGUITextBoxMod();

		CLASSNAME_REGISTER( pfGUITextBoxMod );
		GETINTERFACE_ANY( pfGUITextBoxMod, pfGUIControlMod );

		enum OurFlags
		{
			kCenterJustify = kDerivedFlagsStart,
			kRightJustify
		};

		virtual hsBool	MsgReceive( plMessage* pMsg );
		
		virtual void Read( hsStream* s, hsResMgr* mgr );
		virtual void Write( hsStream* s, hsResMgr* mgr );

		virtual void	HandleMouseDown( hsPoint3 &mousePt, UInt8 modifiers );
		virtual void	HandleMouseUp( hsPoint3 &mousePt, UInt8 modifiers );
		virtual void	HandleMouseDrag( hsPoint3 &mousePt, UInt8 modifiers );

		virtual void	PurgeDynaTextMapImage();

		virtual const wchar_t*	GetText() { return fText; }

		// Export only
		void	SetText( const char *text );
		void	SetText( const wchar_t *text );

		void	SetLocalizationPath(const wchar_t* path);
		void	SetLocalizationPath(const char* path);
		void	SetUseLocalizationPath(bool use);

		virtual void	UpdateColorScheme() { IPostSetUpDynTextMap(); IUpdate(); }
};

#endif // _pfGUITextBoxMod_h
