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
//
//	plResManagerHelper - The wonderful helper class that can receive messages
//						 for the resManager.
//
//// History /////////////////////////////////////////////////////////////////
//
//	6.7.2002 mcn	- Created
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plResManagerHelper_h
#define _plResManagerHelper_h

#include "hsTypes.h"
#include "hsTemplates.h"
#include "plRegistryHelpers.h"
#include "../pnKeyedObject/hsKeyedObject.h"

// Defined as a project setting so we can do this right
//#define MCN_RESMGR_DEBUGGING


//// Class Definition ////////////////////////////////////////////////////////

#ifdef MCN_RESMGR_DEBUGGING
class plStatusLog;
class plDebugPrintIterator;
class plResMgrDebugInterface;
#endif

class plResManager;
class plRegistryPageNode;
class plResManagerHelper : public hsKeyedObject
{
	protected:

		plResManager				*fResManager;
		static plResManagerHelper	*fInstance;

		hsBool						fInShutdown;

#ifdef MCN_RESMGR_DEBUGGING
		friend class plDebugPrintIterator;
		friend class plResMgrDebugInterface;

		plStatusLog		*fDebugScreen;
		hsBool			fRefreshing, fCurrAgeExpanded;
		int				fCurrAge;
		int				fDebugDisplayType;

		enum DebugDisplayTypes
		{
			kSizes = 0,
			kPercents,
			kBars,
			kMaxDisplayType
		};
		plResMgrDebugInterface	*fDebugInput;
#endif

		void	IUpdateDebugScreen( hsBool force = false );

	public:

		plResManagerHelper( plResManager *resMgr );
		virtual ~plResManagerHelper();

		CLASSNAME_REGISTER( plResManagerHelper );
		GETINTERFACE_ANY( plResManagerHelper, hsKeyedObject );

		virtual hsBool	MsgReceive( plMessage *msg );
	
		virtual void	Read( hsStream *s, hsResMgr *mgr );
		virtual void	Write( hsStream *s, hsResMgr *mgr );

		void	Init( void );
		void	Shutdown( void );

		void	LoadAndHoldPageKeys( plRegistryPageNode *page );

		void	EnableDebugScreen( hsBool enable );

		// Please let the res manager handle telling this.
		void	SetInShutdown(hsBool b) { fInShutdown = b; }
		hsBool	GetInShutdown() const { return fInShutdown; }

		static plResManagerHelper	*GetInstance( void ) { return fInstance; }
};

//// Reffer Class ////////////////////////////////////////////////////////////

class plResPageKeyRefList : public plKeyCollector 
{
	protected:

		hsTArray<plKey>		fKeyList;

	public:

		plResPageKeyRefList() : plKeyCollector( fKeyList ) {}
		virtual ~plResPageKeyRefList() { fKeyList.Reset(); }
};

#endif // _plResManagerHelper_h
