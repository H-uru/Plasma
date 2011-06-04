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

#ifndef plMovieMsg_inc
#define plMovieMsg_inc

#include "../pnMessage/plMessage.h"
#include "../pnKeyedObject/plFixedKey.h"
#include "hsPoint2.h"
#include "hsTemplates.h"

class plMovieMsg : public plMessage
{
public:
	enum
	{
		kIgnore				= 0x0,
		kStart				= 0x1,
		kPause				= 0x2,
		kResume				= 0x4,
		kStop				= 0x8,
		kMove				= 0x10, // Call SetCenter() or default is 0,0
		kScale				= 0x20, // Call SetScale() or default is 1,1
		kColor				= 0x40, // Call SetColor() or default is 1,1,1
		kVolume				= 0x80, // Call SetVolume() or default is 1
		kOpacity			= 0x100, // Call SetOpacity() or default is 1
		kColorAndOpacity	= 0x200, // Call SetColor() or default is 1,1,1,1
		kMake				= 0x400,		//Installs, but doesn't play until kStart
		kAddCallbacks		= 0x800, // Call AddCallback() for each callback message
		kFadeIn				= 0x1000, // Call SetFadeInSecs() and SetFadeInColor() or defs are 0 and 0,0,0,0
		kFadeOut			= 0x2000 // Call SetFadeOutSecs() and SetFadeOutColor() or defs are 0 and 0,0,0,0
	};
protected:

	hsPoint2	fCenter;

	hsPoint2	fScale;

	hsColorRGBA	fColor;

	hsColorRGBA fFadeInColor;
	hsScalar	fFadeInSecs;

	hsColorRGBA fFadeOutColor;
	hsScalar	fFadeOutSecs;

	hsScalar	fVolume;

	char*		fFileName;

	UInt16		fCmd;

	hsTArray<plMessage*>	fCallbacks;

public:
	plMovieMsg(const char* n, UInt16 cmd) 
		: plMessage(nil, nil, nil) 
	{ 
		fFileName = hsStrcpy(n);
		SetCmd(cmd).MakeDefault();
	}
	plMovieMsg() : fFileName(nil), fCmd(kIgnore)
	{ 
		MakeDefault();
	}
	virtual ~plMovieMsg() 
	{ 
		delete [] fFileName; 
		int i;
		for( i = 0; i < fCallbacks.GetCount(); i++ )
		{
			hsRefCnt_SafeUnRef(fCallbacks[i]);
		}
	}

	CLASSNAME_REGISTER( plMovieMsg );
	GETINTERFACE_ANY( plMovieMsg, plMessage );

	plMovieMsg& MakeDefault() 
	{ 
		SetCenter(0,0);
		SetScale(1.f,1.f);
		SetColor(1.f, 1.f, 1.f, 1.f);
		SetFadeInSecs(0);
		SetFadeInColor(0, 0, 0, 0);
		SetFadeOutSecs(0);
		SetFadeOutColor(0, 0, 0, 0);
		SetVolume(1.f);
		SetBCastFlag(kBCastByType); 
		return *this; 
	}

	// Make sure you set at least one command, and set appropriate params for the command
	UInt16 GetCmd() const { return fCmd; }
	plMovieMsg& SetCmd(UInt16 c) { fCmd = c; return *this; }

	// Center 0,0 is center of screen, 1,1 is Upper-Right, -1,-1 is Lower-Left, etc.
	const hsPoint2& GetCenter() const { return fCenter; }
	hsScalar GetCenterX() const { return fCenter.fX; }
	hsScalar GetCenterY() const { return fCenter.fY; }

	plMovieMsg& SetCenter(const hsPoint2& p) { fCenter = p; return *this; }
	plMovieMsg& SetCenter(hsScalar x, hsScalar y) { fCenter.Set(x, y); return *this; }
	plMovieMsg& SetCenterX(hsScalar x) { fCenter.fX = x; return *this; }
	plMovieMsg& SetCenterY(hsScalar y) { fCenter.fY = y; return *this; }

	// Scale of 1.0 matches movie pixel to screen pixel (whatever the resolution).
	// Scale of 2.0 doubles each movie pixel across 2 screen pixels.
	// Etc.
	const hsPoint2& GetScale() const { return fScale; }
	hsScalar GetScaleX() const { return fScale.fX; }
	hsScalar GetScaleY() const { return fScale.fY; }

	plMovieMsg& SetScale(const hsPoint2& p) { fScale = p; return *this; }
	plMovieMsg& SetScale(hsScalar x, hsScalar y) { fScale.Set(x, y); return *this; }
	plMovieMsg& SetScaleX(hsScalar x) { fScale.fX = x; return *this; }
	plMovieMsg& SetScaleY(hsScalar y) { fScale.fY = y; return *this; }

	// Include the movie folder, e.g. "avi/porno.bik"
	// String is copied, not pointer copy.
	const char* GetFileName() const { return fFileName; }
	plMovieMsg& SetFileName(const char* n) { delete [] fFileName; fFileName = hsStrcpy(n); return *this; }

	// Color is mostly useful for alpha fade up and down.
	const hsColorRGBA& GetColor() const { return fColor; }
	plMovieMsg& SetColor(const hsColorRGBA& c) { fColor = c; return *this; }
	plMovieMsg& SetColor(hsScalar r, hsScalar g, hsScalar b, hsScalar a) { fColor.Set(r,g,b,a); return *this; }
	plMovieMsg& SetOpacity(hsScalar a) { return SetColor(1.f, 1.f, 1.f, a); }

	// Or the auto matic fades
	const hsColorRGBA& GetFadeInColor() const { return fFadeInColor; }
	plMovieMsg& SetFadeInColor(const hsColorRGBA& c) { fFadeInColor = c; return *this; }
	plMovieMsg& SetFadeInColor(hsScalar r, hsScalar g, hsScalar b, hsScalar a) { fFadeInColor.Set(r,g,b,a); return *this; }

	hsScalar GetFadeInSecs() const { return fFadeInSecs; }
	plMovieMsg& SetFadeInSecs(hsScalar s) { fFadeInSecs = s; return *this; }

	const hsColorRGBA& GetFadeOutColor() const { return fFadeOutColor; }
	plMovieMsg& SetFadeOutColor(const hsColorRGBA& c) { fFadeOutColor = c; return *this; }
	plMovieMsg& SetFadeOutColor(hsScalar r, hsScalar g, hsScalar b, hsScalar a) { fFadeOutColor.Set(r,g,b,a); return *this; }

	hsScalar GetFadeOutSecs() const { return fFadeOutSecs; }
	plMovieMsg& SetFadeOutSecs(hsScalar s) { fFadeOutSecs = s; return *this; }

	// Volume is on scale of 0=muted to 1=full
	hsScalar GetVolume() const { return fVolume; }
	plMovieMsg& SetVolume(hsScalar v) { fVolume = v; return *this; }

	plMovieMsg& AddCallback(plMessage* msg) { hsRefCnt_SafeRef(msg); fCallbacks.Append(msg); return *this; }
	UInt32 GetNumCallbacks() const { return fCallbacks.GetCount(); }
	plMessage* GetCallback(int i) const { return fCallbacks[i]; }

	virtual void Read(hsStream* s, hsResMgr* mgr) { hsAssert(false, "Not for I/O"); plMessage::IMsgRead(s, mgr); }
	virtual void Write(hsStream* s, hsResMgr* mgr) { hsAssert(false, "Not for I/O"); plMessage::IMsgWrite(s, mgr); }
};

#endif // plMovieMsg_inc
