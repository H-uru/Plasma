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
#ifndef PL_LAYER_ANIMATION_INC
#define PL_LAYER_ANIMATION_INC

#include "plLayerInterface.h"
#include "hsMatrix44.h"

// Instead of overwriting owned channels, this layer type will multiply
// its source channel data to the underlayer
class plLayerMultiply : public plLayerInterface
{
public:
	plLayerMultiply();
	virtual ~plLayerMultiply();

	CLASSNAME_REGISTER( plLayerMultiply );
	GETINTERFACE_ANY( plLayerMultiply, plLayerInterface );

	virtual plLayerInterface*	Attach(plLayerInterface* prev);
	virtual UInt32				Eval(double secs, UInt32 frame, UInt32 ignore);
	virtual hsBool				MsgReceive(plMessage* msg);
	virtual void				Read(hsStream* s, hsResMgr* mgr);
	virtual void				Write(hsStream* s, hsResMgr* mgr);

	void SetPreshadeColor(const hsColorRGBA& col);
	void SetRuntimeColor(const hsColorRGBA& col);
	void SetAmbientColor(const hsColorRGBA& col);
	void SetOpacity(hsScalar a);
	void SetTransform(const hsMatrix44& xfm);

protected:
	UInt32			fDirtyChannels;
	hsColorRGBA		fSrcPreshadeColor;
	hsColorRGBA		fSrcRuntimeColor;
	hsColorRGBA		fSrcAmbientColor;
	hsScalar		fSrcOpacity;
	hsMatrix44		fSrcTransform;
};

#endif // PL_LAYER_ANIMATION_INC