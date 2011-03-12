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
#ifndef plListener_h
#define plListener_h

#include "../pnModifier/plSingleModifier.h"


class plSceneObject;
class plVirtualCam1;

class plListener : public plSingleModifier
{
public:

	plListener() :	fVCam(nil), fInitMe(true){;}
	~plListener(){;}

	CLASSNAME_REGISTER( plListener );
	GETINTERFACE_ANY( plListener, plSingleModifier );

	virtual hsBool MsgReceive(plMessage* msg);

	static void	ShowDebugInfo( hsBool s ) { fPrintDbgInfo = s; }

	// Get info for which object these things are attached to - camera or refObject
	UInt8 GetAttachedPosType() { return (UInt8)fPosRatio; }
	UInt8 GetAttachedFacingType() { return (UInt8)fFacingRatio; }
	UInt8 GetAttachedVelType() { return (UInt8)fVelRatio; }
	
	enum 
	{
		kCamera = 0,
		kAvatar = 1
	};

protected:
	
	enum Refs
	{
		kRefObject,
		kRefVCam
	};

	plVirtualCam1*		fVCam;

	hsScalar			fPosRatio, fFacingRatio, fVelRatio;	 // 0 is vCam, 1 is refObject
	hsBool				fInitMe;

	static hsBool		fPrintDbgInfo;

	virtual hsBool IEval(double secs, hsScalar del, UInt32 dirty);
	void			ISetRef( const plKey &ref, hsBool binding, int type );
	void			ICheckAudio( void ) const;

	void			IEnsureVCamValid( void );
};

#endif //plWin32Sound_h
