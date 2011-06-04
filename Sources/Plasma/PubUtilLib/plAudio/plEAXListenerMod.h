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
//	plEAXListenerMod Header													//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plEAXListenerMod_h
#define _plEAXListenerMod_h


#include "../pnModifier/plSingleModifier.h"

class plMessage;
class plSoftVolume;
#ifdef EAX_SDK_AVAILABLE
typedef struct _EAXREVERBPROPERTIES EAXREVERBPROPERTIES;
#endif

class plEAXListenerMod : public plSingleModifier
{
public:

	plEAXListenerMod();
	virtual ~plEAXListenerMod();

	CLASSNAME_REGISTER( plEAXListenerMod );
	GETINTERFACE_ANY( plEAXListenerMod, plSingleModifier );

	enum Refs
	{
		kRefSoftRegion = 0,
	};

	virtual hsBool	MsgReceive( plMessage* pMsg );
	virtual void	Read( hsStream* s, hsResMgr* mgr );
	virtual void	Write( hsStream* s, hsResMgr* mgr );
	float			GetStrength( void );

	EAXREVERBPROPERTIES	*	GetListenerProps( void ) { return fListenerProps; }
	void					SetFromPreset( UInt32 preset );

protected:
	plSoftVolume	*fSoftRegion;
	EAXREVERBPROPERTIES	*fListenerProps;
	hsBool		fRegistered, fGetsMessages;

	void			IRegister( void );
	void			IUnRegister( void );
	virtual hsBool	IEval( double secs, hsScalar del, UInt32 dirty ); // called only by owner object's Eval()
};

#endif // _plEAXListenerMod_h
