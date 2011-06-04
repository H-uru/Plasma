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
//	pfGUITextBoxMod Definition												//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "hsStlUtils.h"
#include "pfGUITextBoxMod.h"
#include "pfGameGUIMgr.h"

#include "../pnMessage/plRefMsg.h"
#include "../pfMessage/pfGameGUIMsg.h"
#include "../plMessage/plAnimCmdMsg.h"
#include "../plAvatar/plAGModifier.h"
#include "../plGImage/plDynamicTextMap.h"
#include "plgDispatch.h"
#include "hsResMgr.h"
#include "../plResMgr/plLocalization.h"

#include "../pfLocalizationMgr/pfLocalizationMgr.h"



//// Constructor/Destructor //////////////////////////////////////////////////

pfGUITextBoxMod::pfGUITextBoxMod()
{
//	SetFlag( kWantsInterest );
	SetFlag( kIntangible );
	fText = nil;
	fUseLocalizationPath = false;
}

pfGUITextBoxMod::~pfGUITextBoxMod()
{
	delete [] fText;
}

//// IEval ///////////////////////////////////////////////////////////////////

hsBool	pfGUITextBoxMod::IEval( double secs, hsScalar del, UInt32 dirty )
{
	return pfGUIControlMod::IEval( secs, del, dirty );
}

//// MsgReceive //////////////////////////////////////////////////////////////

hsBool	pfGUITextBoxMod::MsgReceive( plMessage *msg )
{
	return pfGUIControlMod::MsgReceive( msg );
}

//// IPostSetUpDynTextMap ////////////////////////////////////////////////////

void	pfGUITextBoxMod::IPostSetUpDynTextMap( void )
{
	pfGUIColorScheme *scheme = GetColorScheme();

	fDynTextMap->SetFont( scheme->fFontFace, scheme->fFontSize, scheme->fFontFlags, 
							HasFlag( kXparentBgnd ) ? false : true );
	fDynTextMap->SetTextColor( scheme->fForeColor, 
							( HasFlag( kXparentBgnd ) && scheme->fBackColor.a == 0.f ) ? true : false );
}

//// IUpdate /////////////////////////////////////////////////////////////////

void	pfGUITextBoxMod::IUpdate( void )
{
	if( fDynTextMap == nil || !fDynTextMap->IsValid() )
		return;

	if( HasFlag( kCenterJustify ) )
		fDynTextMap->SetJustify( plDynamicTextMap::kCenter );
	else if( HasFlag( kRightJustify ) )
		fDynTextMap->SetJustify( plDynamicTextMap::kRightJustify );
	else
		fDynTextMap->SetJustify( plDynamicTextMap::kLeftJustify );

	fDynTextMap->ClearToColor( GetColorScheme()->fBackColor );

	std::wstring drawStr;
	if (fUseLocalizationPath && !fLocalizationPath.empty() && pfLocalizationMgr::InstanceValid())
		drawStr = pfLocalizationMgr::Instance().GetString(fLocalizationPath.c_str());
	else
	{
		if( fText != nil )
		{
			int lang = plLocalization::GetLanguage();
			std::vector<std::wstring> translations = plLocalization::StringToLocal(fText);
			if (translations[lang] == L"") // if the translations doesn't exist, draw English
				drawStr = translations[0].c_str();
			else
				drawStr = translations[lang].c_str();
		}
	}

	if (!drawStr.empty())
		fDynTextMap->DrawWrappedString( 4, 4, drawStr.c_str(), fDynTextMap->GetVisibleWidth() - 8, fDynTextMap->GetVisibleHeight() - 8 );

	fDynTextMap->FlushToHost();
}

void pfGUITextBoxMod::PurgeDynaTextMapImage()
{
	if ( fDynTextMap != nil )
		fDynTextMap->PurgeImage();
}

//// Read/Write //////////////////////////////////////////////////////////////

void	pfGUITextBoxMod::Read( hsStream *s, hsResMgr *mgr )
{
	pfGUIControlMod::Read(s, mgr);

	UInt32 len = s->ReadSwap32();
	if( len > 0 )
	{
		char *text = TRACKED_NEW char[ len + 1 ];
		s->Read( len, text );
		text[ len ] = 0;

		fText = hsStringToWString(text);
		delete [] text;
	}
	else
		fText = nil;

	fUseLocalizationPath = (s->ReadBool() != 0);
	if (fUseLocalizationPath)
	{
		wchar_t* temp = s->ReadSafeWString();
		fLocalizationPath = temp;
		delete [] temp;
	}
}

void	pfGUITextBoxMod::Write( hsStream *s, hsResMgr *mgr )
{
	pfGUIControlMod::Write( s, mgr );

	if( fText == nil )
		s->WriteSwap32( 0 );
	else
	{
		char *text = hsWStringToString(fText);
		s->WriteSwap32( strlen( text ) );
		s->Write( strlen( text ), text );
		delete [] text;
	}

	// Make sure we only write out to use localization path if the box is checked
	// and the path isn't empty
	bool useLoc = fUseLocalizationPath && !fLocalizationPath.empty();

	s->WriteBool(useLoc);
	if (useLoc)
		s->WriteSafeWString(fLocalizationPath.c_str());
}

//// HandleMouseDown/Up //////////////////////////////////////////////////////

void	pfGUITextBoxMod::HandleMouseDown( hsPoint3 &mousePt, UInt8 modifiers )
{
}

void	pfGUITextBoxMod::HandleMouseUp( hsPoint3 &mousePt, UInt8 modifiers )
{
}

void	pfGUITextBoxMod::HandleMouseDrag( hsPoint3 &mousePt, UInt8 modifiers )
{
}

//// SetText /////////////////////////////////////////////////////////////////

void	pfGUITextBoxMod::SetText( const char *text )
{
	delete [] fText;
	if (text)
	{
		fText = hsStringToWString(text);
	}
	else
		fText = nil;
	IUpdate();
}

void	pfGUITextBoxMod::SetText( const wchar_t *text )
{
	delete [] fText;
	if (text)
	{
		fText = TRACKED_NEW wchar_t[wcslen(text)+1];
		wcscpy(fText,text);
	}
	else
		fText = nil;
	IUpdate();
}

void pfGUITextBoxMod::SetLocalizationPath(const wchar_t* path)
{
	if (path)
		fLocalizationPath = path;
}

void pfGUITextBoxMod::SetLocalizationPath(const char* path)
{
	if (path)
	{
		wchar_t* wPath = hsStringToWString(path);
		fLocalizationPath = wPath;
		delete [] wPath;
	}
}

void pfGUITextBoxMod::SetUseLocalizationPath(bool use)
{
	fUseLocalizationPath = use;
}