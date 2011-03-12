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

#ifndef plLayerShadowBase_inc
#define plLayerShadowBase_inc

#include "plLayerInterface.h"
#include "hsGMatState.h"

class plLayerLightBase : public plLayerInterface
{
protected:
	hsBool				fDirty;
public:

	plLayerLightBase();
	virtual ~plLayerLightBase();

	CLASSNAME_REGISTER( plLayerLightBase );
	GETINTERFACE_ANY( plLayerLightBase, plLayerInterface );


	virtual plLayerInterface*	Attach(plLayerInterface* prev);

	virtual UInt32			Eval(double secs, UInt32 frame, UInt32 ignore);


};

class plLayerShadowBase : public plLayerInterface
{
protected:
	hsBool				fDirty;
public:

	plLayerShadowBase();
	virtual ~plLayerShadowBase();

	CLASSNAME_REGISTER( plLayerShadowBase );
	GETINTERFACE_ANY( plLayerShadowBase, plLayerInterface );


	virtual plLayerInterface*	Attach(plLayerInterface* prev);

	virtual UInt32			Eval(double secs, UInt32 frame, UInt32 ignore);


};


#endif // plLayerShadowBase_inc
