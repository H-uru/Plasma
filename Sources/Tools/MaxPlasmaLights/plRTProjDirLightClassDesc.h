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
///////////////////////////////////////////////////////////////////////////////
//																			 //
//	plRTProjDirLight.h - Header for the derived MAX RT projected directional //
//						 light												 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	8.2.2001 mcn - Created.													 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plRTProjDirLightClassDesc_h
#define _plRTProjDirLightClassDesc_h

#include "plRTProjDirLight.h"
#include "iparamm2.h"
#include "resource.h"

class plRTProjDirLightDesc : public ClassDesc2 
{
	public:
		int 			IsPublic()						{ return TRUE; }
		void*			Create(BOOL loading)			{ return TRACKED_NEW plRTProjDirLight; }
		const TCHAR*	ClassName()						{ return GetString(IDS_DB_PROJDIR); }
		SClass_ID		SuperClassID()					{ return LIGHT_CLASS_ID; }
		Class_ID		ClassID()						{ return RTPDIR_LIGHT_CLASSID; }
		const TCHAR* 	Category()						{ return _T("Plasma RunTime");}
		const TCHAR*	InternalName()					{ return _T("RTProjDir"); }	// returns fixed parsable name (scripter-visible name)
		HINSTANCE		HInstance()						{ return hInstance; }

		static plRTProjDirLightDesc	fStaticDesc;

		static ClassDesc2	*GetDesc( void )		{ return &fStaticDesc; }
};

#endif	// _plRTProjDirLightClassDesc_h