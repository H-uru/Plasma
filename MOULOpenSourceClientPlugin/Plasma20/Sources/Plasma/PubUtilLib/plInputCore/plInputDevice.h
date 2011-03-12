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
// plInputDevice.h

#ifndef PL_INPUT_DEVICE_H
#define PL_INPUT_DEVICE_H

#include "HeadSpin.h"
#include "hsWindows.h"
//#include "../pnInputCore/plControlDefinition.h"
#include "../pnInputCore/plOSMsg.h"
#include "hsBitVector.h"
#include "hsTemplates.h"
#include "../../apps/plClient/res/resource.h"
class plMessage;
enum plKeyDef;
struct plMouseInfo;
class plPipeline;

class plInputDevice 
{
public:
	enum Flags
	{
		kDisabled	 = 0x1
	};
protected:
	UInt32 fFlags;
public:
	
	plInputDevice() {;}
	virtual ~plInputDevice() {;}

	virtual const char* GetInputName() = 0;

	UInt32 GetFlags() { return fFlags; }
	void SetFlags(UInt32 f) { fFlags = f; }
	virtual void HandleKeyEvent(plOSMsg message, plKeyDef key, bool bKeyDown, hsBool bKeyRepeat) {;}
	virtual void HandleMouseEvent(plOSMsg message, plMouseState state)	{;}
	virtual void HandleWindowActivate(bool bActive, HWND hWnd) {;}
	virtual hsBool MsgReceive(plMessage* msg) {return false;}
	virtual void Shutdown() {;}


};

class plKeyEventMsg;

class plKeyboardDevice : public plInputDevice
{
	hsBool	fAltKeyDown;
	hsBool	fShiftKeyDown;
	hsBool	fCtrlKeyDown;
	hsBool	fCapsLockKeyDown;
	int		fControlMode;
	hsBool	fCapsLockLock;

	static bool		fKeyboardState[256]; // virtual key code is the index, bool is whether it is down or not
	static hsBool	fIgnoreCapsLock; // set if we want it to ignore this key when translating characters (i.e. for chatting)
	static hsBool	fKeyIsDeadKey; // the key we just got was a dead key, store the value if you're a text input object

	static plKeyboardDevice* fInstance;
	void InitKeyboardMaps();
	void InitKeyboardState();

	void ReleaseAllKeys();

public:
	enum
	{
		CONSOLE_MODE = 0,
		CONSOLE_FULL,
		STANDARD_MODE,
	};
	plKeyboardDevice();
	~plKeyboardDevice();

	void SetControlMode(int i) { fControlMode = i; }

	const char* GetInputName() { return "keyboard"; }
	void HandleKeyEvent(plOSMsg message, plKeyDef key, bool bKeyDown, hsBool bKeyRepeat);
	virtual void HandleWindowActivate(bool bActive, HWND hWnd);
	virtual hsBool IsCapsLockKeyOn();
	virtual void Shutdown();

#if HS_BUILD_FOR_WIN32	
	void ForceNumLock(hsBool on);
#endif

	static hsBool	IgnoreCapsLock() { return fIgnoreCapsLock; }
	static void		IgnoreCapsLock(hsBool ignore) { fIgnoreCapsLock = ignore; }

	static hsBool	KeyIsDeadKey() { return fKeyIsDeadKey; }
	
	static plKeyboardDevice* GetInstance() { return fInstance; }

	static char	KeyEventToChar( plKeyEventMsg *msg );

protected:
	hsBool fStartedUpWithNumLockOn; // maintaining a separate flag since apparently the other one can get confused
	hsBool fPrevNumLockOn;
};

class plPlate;

#define CURSOR_UP					IDB_CURSOR_UP									
#define CURSOR_DOWN					IDB_CURSOR_DOWN		
#define CURSOR_RIGHT				IDB_CURSOR_RIGHT	
#define CURSOR_LEFT					IDB_CURSOR_LEFT		
#define CURSOR_OPEN					IDB_CURSOR_OPEN		
#define CURSOR_GRAB					IDB_CURSOR_GRAB		
#define CURSOR_CLICKED				IDB_CURSOR_CLICKED	
#define CURSOR_POISED				IDB_CURSOR_POISED	
#define CURSOR_ARROW				IDB_CURSOR_ARROW	
#define CURSOR_4WAY_OPEN            IDB_CURSOR_4WAYOPEN         
#define CURSOR_4WAY_CLOSED          IDB_CURSOR_4WAYCLOSED       
#define CURSOR_UPDOWN_CLOSED        IDB_CURSOR_UPDOWNCLOSED     
#define CURSOR_UPDOWN_OPEN          IDB_CURSOR_UPDOWNOPEN       
#define CURSOR_LEFTRIGHT_CLOSED     IDB_CURSOR_LEFTRIGHTCLOSED  
#define CURSOR_LEFTRIGHT_OPEN       IDB_CURSOR_LEFTRIGHTOPEN    
#define CURSOR_OFFER_BOOK			IDB_CURSOR_BOOK		 
#define CURSOR_OFFER_BOOK_HI		IDB_CURSOR_BOOK_HIGHLIGHT
#define CURSOR_OFFER_BOOK_CLICKED	IDB_CURSOR_BOOK_CLICKED	 
#define CURSOR_CLICK_DISABLED		IDB_CURSOR_DISABLED
#define CURSOR_HAND					IDB_CURSOR_HAND
#define CURSOR_UPWARD				IDB_CURSOR_UPWARD

class plInputEventMsg;

class plMouseDevice : public plInputDevice
{
public:
	plMouseDevice();
	~plMouseDevice();

	const char* GetInputName() { return "mouse"; }
	void HandleWindowActivate(bool bActive, HWND hWnd);

	hsBool	HasControlFlag(int f) const { return fControlFlags.IsBitSet(f); }
	void	SetControlFlag(int f) 
	{ 
		fControlFlags.SetBit(f); 
	}
	void	ClearControlFlag(int which) { fControlFlags.ClearBit( which ); }
	void	SetCursorX(hsScalar x);
	void	SetCursorY(hsScalar y);
	hsScalar GetCursorX() { return fXPos; }
	hsScalar GetCursorY() { return fYPos; }
	UInt32	GetButtonState() { return fButtonState; }
	hsScalar GetCursorOpacity() { return fOpacity; }
	void SetDisplayResolution(hsScalar Width, hsScalar Height);
	
	virtual hsBool MsgReceive(plMessage* msg);
	
	static plMouseDevice* Instance() { return plMouseDevice::fInstance; }
	
	static void SetMsgAlways(bool b) { plMouseDevice::bMsgAlways = b; }
	static void ShowCursor(hsBool override = false);
	static void NewCursor(int cursor);
	static void HideCursor(hsBool override = false);
	static bool GetHideCursor() { return plMouseDevice::bCursorHidden; }
	static void	SetCursorOpacity( hsScalar opacity = 1.f );
	static bool	GetInverted() { return plMouseDevice::bInverted; }
	static void SetInverted(bool inverted) { plMouseDevice::bInverted = inverted; }
	static void AddNameToCursor(const char* name);
	static void AddIDNumToCursor(UInt32 idNum);
	static void AddCCRToCursor();
	
protected:
	plInputEventMsg*	fXMsg;
	plInputEventMsg*	fYMsg;
	plInputEventMsg*	fB2Msg;

	// mouse button event queues (only hold 2)
	plInputEventMsg*	fLeftBMsg[2];
	plInputEventMsg*	fRightBMsg[2];
	plInputEventMsg*	fMiddleBMsg[2];

	hsScalar fXPos;
	hsScalar fYPos;
	int		 fWXPos; // the windows coordinates of the cursor
	int		 fWYPos;
	UInt32	 fButtonState;
	hsScalar fOpacity;
	hsBitVector		fControlFlags;
	
	
	plPlate	*fCursor;
	int		fCursorID;

	static plMouseDevice* fInstance;
	static plMouseInfo	fDefaultMouseControlMap[];
	void	CreateCursor( int cursor );
	void IUpdateCursorSize();
	static bool bMsgAlways;
	static bool bCursorHidden;
	static bool bCursorOverride;
	static bool bInverted;
	static hsScalar fWidth, fHeight;
};



#endif // PL_INPUT_DEVICE_H
