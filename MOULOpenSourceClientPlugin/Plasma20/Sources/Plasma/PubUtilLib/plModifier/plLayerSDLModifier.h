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
#ifndef plLayerSDLModifier_inc
#define plLayerSDLModifier_inc

#include "../plModifier/plAnimTimeConvertSDLModifier.h"

//
// This modifier is responsible for sending and recving 
// an layer's animation state (basically an animTimeConvert)
//
class plLayerAnimation;
class plStateDataRecord;

class plLayerSDLModifier : public plAnimTimeConvertSDLModifier
{
protected:

	static char	kStrAtc[];		// animTimeConvert var name
	static char	kStrPassThruChannels[];
	static char	kStrTransform[];
	static char kStrChannelData[];
	//static char	kStrPreShadeColor[];
	//static char	kStrRuntimeColor[];
	//static char	kStrAmbientColor[];
	//static char	kStrOpacity[];
	plLayerAnimation* fLayerAnimation;

	void IPutCurrentStateIn(plStateDataRecord* dstState);
	void ISetCurrentStateFrom(const plStateDataRecord* srcState);
public:
	CLASSNAME_REGISTER( plLayerSDLModifier);
	GETINTERFACE_ANY( plLayerSDLModifier, plAnimTimeConvertSDLModifier);
		
	plLayerSDLModifier() : fLayerAnimation(nil) {}
	
	const char* GetSDLName() const { return kSDLLayer; }
	
	plLayerAnimation* GetLayerAnimation() const { return fLayerAnimation; }
	void SetLayerAnimation(plLayerAnimation* l) { fLayerAnimation=l; AddTarget(nil); }
	plKey GetStateOwnerKey() const;
};

#endif	// plLayerSDLModifier_inc

