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

#ifndef plInputEventMsg_inc
#define plInputEventMsg_inc

#include "../pnMessage/plMessage.h"
#include "../pnInputCore/plControlDefinition.h"
#include "hsGeometry3.h"
#include "hsStream.h"
#include "hsUtils.h"

class plKeyEventMsg;
class plMouseEventMsg;

class plInputEventMsg : public plMessage
{
public:
	enum
	{
		kConfigure = 0,
	};
	plInputEventMsg();
	plInputEventMsg(const plKey &s,
					  const plKey &r,
					  const double* t);

	~plInputEventMsg();

	CLASSNAME_REGISTER( plInputEventMsg );
	GETINTERFACE_ANY( plInputEventMsg, plMessage );

	int fEvent;
	
	// IO 
	void Read(hsStream* stream, hsResMgr* mgr);
	void Write(hsStream* stream, hsResMgr* mgr);

	void ReadVersion(hsStream* s, hsResMgr* mgr);
	void WriteVersion(hsStream* s, hsResMgr* mgr);
};


class plControlEventMsg : public plInputEventMsg
{
private:
	char*				fCmd;
protected:
	
	ControlEventCode	fControlCode;
	hsBool				fControlActivated;
	hsPoint3			fTurnToPt;
	hsScalar			fControlPct;
public:

	plControlEventMsg();
	plControlEventMsg(const plKey &s, 
					const plKey &r, 
					const double* t);
	~plControlEventMsg();

	CLASSNAME_REGISTER( plControlEventMsg );
	GETINTERFACE_ANY( plControlEventMsg, plInputEventMsg );

	void SetCmdString(const char* cs)		{ delete [] fCmd; fCmd=hsStrcpy(cs); }
	void SetControlCode(ControlEventCode c)	{ fControlCode = c; }
	void SetControlActivated(hsBool b) 		{ fControlActivated = b; }
	void SetTurnToPt(hsPoint3 pt)			{ fTurnToPt = pt; }
	void SetControlPct(hsScalar p)			{ fControlPct = p; }

	ControlEventCode	GetControlCode()	const { return fControlCode; }
	hsBool				ControlActivated()	{ return fControlActivated; }
	hsPoint3			GetTurnToPt()		{ return fTurnToPt; }
	hsScalar			GetPct()			{ return fControlPct; }
	char*				GetCmdString()		{ return fCmd; }

	// IO
	void Read(hsStream* stream, hsResMgr* mgr);
	void Write(hsStream* stream, hsResMgr* mgr);

	void ReadVersion(hsStream* s, hsResMgr* mgr);
	void WriteVersion(hsStream* s, hsResMgr* mgr);
};


class plKeyEventMsg : public plInputEventMsg
{
protected:
	plKeyDef		fKeyCode;
	hsBool			fKeyDown;
	hsBool			fCapsLockKeyDown;
	hsBool			fShiftKeyDown;
	hsBool			fCtrlKeyDown;
	hsBool			fRepeat;

public:

	
	plKeyEventMsg();
	plKeyEventMsg(const plKey &s, 
					const plKey &r, 
					const double* t);
	~plKeyEventMsg();

	CLASSNAME_REGISTER( plKeyEventMsg );
	GETINTERFACE_ANY( plKeyEventMsg, plInputEventMsg );

	void SetKeyCode(plKeyDef w)	{ fKeyCode = w; }
	void SetKeyDown(hsBool b) 	{ fKeyDown = b; }
	void SetShiftKeyDown(hsBool b) 	{ fShiftKeyDown = b; }
	void SetCtrlKeyDown(hsBool b) 	{ fCtrlKeyDown = b; }
	void SetCapsLockKeyDown(hsBool b) 	{ fCapsLockKeyDown = b; }
	void SetRepeat(hsBool b) 	{ fRepeat = b; }
	
	plKeyDef	GetKeyCode()		{ return fKeyCode; }
	hsBool		GetKeyDown()		{ return fKeyDown; }
	hsBool		GetShiftKeyDown()	{ return fShiftKeyDown; }
	hsBool		GetCtrlKeyDown()	{ return fCtrlKeyDown; }
	hsBool		GetCapsLockKeyDown()		{ return fCapsLockKeyDown; }
	hsBool		GetRepeat()			{ return fRepeat; }

	// IO
	void Read(hsStream* stream, hsResMgr* mgr)
	{
		plInputEventMsg::Read(stream, mgr);
		stream->ReadSwap((Int32*)&fKeyCode);
		stream->ReadSwap(&fKeyDown);
		stream->ReadSwap(&fCapsLockKeyDown);
		stream->ReadSwap(&fShiftKeyDown);
		stream->ReadSwap(&fCtrlKeyDown);
		stream->ReadSwap(&fRepeat);
	}

	void Write(hsStream* stream, hsResMgr* mgr)
	{
		plInputEventMsg::Write(stream, mgr);
		stream->WriteSwap((Int32)fKeyCode);
		stream->WriteSwap(fKeyDown);
		stream->WriteSwap(fCapsLockKeyDown);
		stream->WriteSwap(fShiftKeyDown);
		stream->WriteSwap(fCtrlKeyDown);
		stream->WriteSwap(fRepeat);
	}
};


class plDebugKeyEventMsg : public plInputEventMsg
{
protected:
	ControlEventCode	fKeyCode;
	hsBool				fKeyDown;
	hsBool			fCapsLockKeyDown;
	hsBool			fShiftKeyDown;
	hsBool			fCtrlKeyDown;

public:

	
	plDebugKeyEventMsg();
	plDebugKeyEventMsg(const plKey &s, 
					const plKey &r, 
					const double* t);
	~plDebugKeyEventMsg();

	CLASSNAME_REGISTER( plDebugKeyEventMsg );
	GETINTERFACE_ANY( plDebugKeyEventMsg, plInputEventMsg );

	void SetKeyCode(ControlEventCode w)	{ fKeyCode = w; }
	void SetKeyDown(hsBool b) 			{ fKeyDown = b; }
	void SetShiftKeyDown(hsBool b) 	{ fShiftKeyDown = b; }
	void SetCtrlKeyDown(hsBool b) 	{ fCtrlKeyDown = b; }
	void SetCapsLockKeyDown(hsBool b) 	{ fCapsLockKeyDown = b; }

	ControlEventCode	GetKeyCode()	{ return fKeyCode; }
	hsBool				GetKeyDown()	{ return fKeyDown; }
	hsBool		GetShiftKeyDown()	{ return fShiftKeyDown; }
	hsBool		GetCtrlKeyDown()	{ return fCtrlKeyDown; }
	hsBool		GetCapsLockKeyDown()		{ return fCapsLockKeyDown; }


	// IO
	void Read(hsStream* stream, hsResMgr* mgr)
	{
		plInputEventMsg::Read(stream, mgr);
		stream->ReadSwap((Int32*)&fKeyCode);
		stream->ReadSwap(&fKeyDown);
		stream->ReadSwap(&fCapsLockKeyDown);
		stream->ReadSwap(&fShiftKeyDown);
		stream->ReadSwap(&fCtrlKeyDown);
	}

	void Write(hsStream* stream, hsResMgr* mgr)
	{
		plInputEventMsg::Write(stream, mgr);
		stream->WriteSwap((Int32)fKeyCode);
		stream->WriteSwap(fKeyDown);
		stream->WriteSwap(fCapsLockKeyDown);
		stream->WriteSwap(fShiftKeyDown);
		stream->WriteSwap(fCtrlKeyDown);
	}
};

class plIMouseXEventMsg : public plInputEventMsg
{
public:
	float	fX;
	int		fWx;
	
	plIMouseXEventMsg() : 
	fX(0),fWx(0) {}
	plIMouseXEventMsg(const plKey &s, 
					const plKey &r, 
					const double* t) : 
	fX(0),fWx(0) {}
	~plIMouseXEventMsg(){}

	CLASSNAME_REGISTER( plIMouseXEventMsg );
	GETINTERFACE_ANY( plIMouseXEventMsg, plInputEventMsg );

	void Read(hsStream* stream, hsResMgr* mgr)
	{
		plInputEventMsg::Read(stream, mgr);
		stream->ReadSwap(&fX);
		stream->ReadSwap(&fWx);
	}

	void Write(hsStream* stream, hsResMgr* mgr)
	{
		plInputEventMsg::Write(stream, mgr);
		stream->WriteSwap(fX);
		stream->WriteSwap(fWx);
	}

};

class plIMouseYEventMsg : public plInputEventMsg
{
public:
	float	fY;
	int		fWy;

	plIMouseYEventMsg() : 
	fY(0),fWy(0) {}
	plIMouseYEventMsg(const plKey &s, 
					const plKey &r, 
					const double* t) : 
	fY(0),fWy(0) {}
	~plIMouseYEventMsg(){}

	CLASSNAME_REGISTER( plIMouseYEventMsg );
	GETINTERFACE_ANY( plIMouseYEventMsg, plInputEventMsg );

	void Read(hsStream* stream, hsResMgr* mgr)
	{
		plInputEventMsg::Read(stream, mgr);
		stream->ReadSwap(&fY);
		stream->ReadSwap(&fWy);
	}

	void Write(hsStream* stream, hsResMgr* mgr)
	{
		plInputEventMsg::Write(stream, mgr);
		stream->WriteSwap(fY);
		stream->WriteSwap(fWy);
	}

};
class plIMouseBEventMsg : public plInputEventMsg
{
public:
	short	fButton;

	plIMouseBEventMsg() : 
	fButton(0) {}
	plIMouseBEventMsg(const plKey &s, 
					const plKey &r, 
					const double* t) : 
	fButton(0) {}
	~plIMouseBEventMsg(){}

	CLASSNAME_REGISTER( plIMouseBEventMsg );
	GETINTERFACE_ANY( plIMouseBEventMsg, plInputEventMsg );

	void Read(hsStream* stream, hsResMgr* mgr)
	{
		plInputEventMsg::Read(stream, mgr);
		stream->ReadSwap(&fButton);
	}

	void Write(hsStream* stream, hsResMgr* mgr)
	{
		plInputEventMsg::Write(stream, mgr);
		stream->WriteSwap(fButton);
	}

};

class plMouseEventMsg : public plInputEventMsg
{
	
protected:

	float fXPos;
	float fYPos;
	float fDX;
	float fDY;
	float fWheelDelta;

	short	fButton;


public:
	plMouseEventMsg();
	plMouseEventMsg(const plKey &s, 
					const plKey &r, 
					const double* t);
	~plMouseEventMsg();

	CLASSNAME_REGISTER( plMouseEventMsg );
	GETINTERFACE_ANY( plMouseEventMsg, plInputEventMsg );
	
	void SetXPos(float Xpos) { fXPos = Xpos; };
	void SetYPos(float Ypos) { fYPos = Ypos; };
	void SetDX(float dX)	   { fDX = dX; }
	void SetDY(float dY)	   { fDY = dY; }
	void SetButton(short _button) { fButton = _button; }
	void SetWheelDelta(float d) { fWheelDelta = d; }
	
	float GetXPos() { return fXPos; }
	float GetYPos() { return fYPos; }
	float GetDX()   { return fDX; }
	float GetDY()   { return fDY; }
	float GetWheelDelta() { return fWheelDelta;	}
	short GetButton() { return fButton; }

	// IO
	void Read(hsStream* stream, hsResMgr* mgr)
	{
		plInputEventMsg::Read(stream, mgr);
		stream->ReadSwap(&fXPos);
		stream->ReadSwap(&fYPos);
		stream->ReadSwap(&fDX);
		stream->ReadSwap(&fDY);
		stream->ReadSwap(&fButton);
		stream->ReadSwap(&fWheelDelta);
	}

	void Write(hsStream* stream, hsResMgr* mgr)
	{
		plInputEventMsg::Write(stream, mgr);
		stream->WriteSwap(fXPos);
		stream->WriteSwap(fYPos);
		stream->WriteSwap(fDX);
		stream->WriteSwap(fDY);
		stream->WriteSwap(fButton);
		stream->WriteSwap(fWheelDelta);
	}
};

class plAvatarInputStateMsg : public plMessage
{
public:
	UInt16 fState;

	plAvatarInputStateMsg() : plMessage(), fState(0) {}
	~plAvatarInputStateMsg() {}

	CLASSNAME_REGISTER( plAvatarInputStateMsg );
	GETINTERFACE_ANY( plAvatarInputStateMsg, plMessage );

	void Read(hsStream *s, hsResMgr *mgr);
	void Write(hsStream *s, hsResMgr *mgr);

	void ReadVersion(hsStream* s, hsResMgr* mgr);
	void WriteVersion(hsStream* s, hsResMgr* mgr);

	// Mapping of bits to the control events we care about
	static const ControlEventCode fCodeMap[];
	static const UInt8 fMapSize;

	static hsBool IsCodeInMap(ControlEventCode code);
};

#endif // plInputEventMsg_inc
