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
#ifndef plClothingMsg_inc
#define plClothingMsg_inc

#include "../pnMessage/plRefMsg.h"
#include "hsStream.h"
#include "../CoreLib/hsColorRGBA.h"

class hsResMgr;

class plClothingMsg : public plMessage
{
protected:
	UInt32 fCommands;

public:
	plKey fItemKey;
	hsColorRGBA fColor;
	UInt8 fLayer;
	UInt8 fDelta;
	hsScalar fWeight;

	plClothingMsg() : fCommands(0), fItemKey(nil), fLayer(0), fDelta(0), fWeight(0) { fColor.Set(1.f, 1.f, 1.f, 1.f); }
	~plClothingMsg() {}

	CLASSNAME_REGISTER( plClothingMsg );
	GETINTERFACE_ANY( plClothingMsg, plMessage );

	enum commands
	{
		kAddItem =				0x0001,
		kRemoveItem =			0x0002,
		kUpdateTexture =		0x0004,
		kTintItem =				0x0008,
		kRetry =				0x0010,
		kTintSkin =				0x0020,
		kBlendSkin =			0x0040,
		kMorphItem =			0x0080,
		kSaveCustomizations	=	0x0100,
	};

	hsBool GetCommand(UInt32 command) { return fCommands & command; }
	void AddCommand(UInt32 command) { fCommands |= command; }
	hsBool ResendUpdate() { return fCommands != kUpdateTexture; }

	// IO 
	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);
	
	// WriteVersion writes the current version of this creatable and ReadVersion will read in
	// any previous version.
	virtual void ReadVersion(hsStream* s, hsResMgr* mgr);
	virtual void WriteVersion(hsStream* s, hsResMgr* mgr);
};

class plElementRefMsg : public plGenRefMsg
{
public:
	char		*fElementName;
	UInt32		fLayer;

	plElementRefMsg() : plGenRefMsg(), fElementName(nil), fLayer(1) {}
	plElementRefMsg(const plKey &r, UInt8 c, int which, int type, char *name, UInt8 layer) : plGenRefMsg(r, c, which, type)
	{
		fLayer = layer;
		fElementName = hsStrcpy(name);
	}
	~plElementRefMsg() { delete [] fElementName; }

	CLASSNAME_REGISTER( plElementRefMsg );
	GETINTERFACE_ANY( plElementRefMsg, plGenRefMsg );
};

class plClothingUpdateBCMsg : public plMessage
{
public:
	plClothingUpdateBCMsg();
	~plClothingUpdateBCMsg() {}

	CLASSNAME_REGISTER( plClothingUpdateBCMsg );
	GETINTERFACE_ANY( plClothingUpdateBCMsg, plMessage );	

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);
};

#endif // plClothingMsg_inc
