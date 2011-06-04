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
#include "hsConfig.h"
#include "hsWindows.h"

// plInputManager.cpp
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include "hsTypes.h"
#include "plInputManager.h"
#include "plPipeline.h"
#include "plInputDevice.h"
#include "plDInputDevice.h"
#include "../plMessage/plInputEventMsg.h"
#include "plInputInterfaceMgr.h"
#include "hsStream.h"
#include "../pnKeyedObject/plKey.h"
#include "../pnKeyedObject/plFixedKey.h"
#include "hsResMgr.h"
#include "hsTimer.h"
#include "plgDispatch.h"
#include "../pnMessage/plTimeMsg.h"
#include "../pnMessage/plCmdIfaceModMsg.h"
#include "../pnMessage/plPlayerPageMsg.h"

hsBool	plInputManager::fUseDInput = false;
UInt8	plInputManager::bRecenterMouse = 0;
HWND	plInputManager::fhWnd = nil;
#define NUM_ACTIONS		17

struct plDIDevice
{
	plDIDevice() : fDevice(nil), fCaps(nil) {;}
	plDIDevice(IDirectInputDevice8* _device) : fCaps(nil) { fDevice = _device;}
	IDirectInputDevice8*	fDevice;
	DIDEVCAPS*				fCaps;
};

struct plDInput
{
	plDInput() :
	fDInput(nil),
	fActionFormat(nil)
	{;}
	IDirectInput8*			fDInput; 
	hsTArray<plDIDevice*>	fSticks;
	DIACTIONFORMAT*			fActionFormat;
};

class plDInputMgr 
{
public:
	plDInputMgr();
	~plDInputMgr();

	void Init(HINSTANCE hInst, HWND hWnd);
	void Update();
	void AddDevice(IDirectInputDevice8* device);
	void ConfigureDevice();
	virtual hsBool MsgReceive(plMessage* msg);
	
	// dinput callback functions
	static int __stdcall EnumGamepadCallback(const DIDEVICEINSTANCE* device, void* pRef);
// I should be using these but they don't work...
//	static int __stdcall SetAxisRange(const DIDEVICEOBJECTINSTANCE* obj, void* pRef);
//	static int __stdcall EnumSuitableDevices(const struct DIDEVICEINSTANCEA* devInst, struct IDirectInputDevice8* dev, unsigned long why, unsigned long devRemaining, void* pRef);

protected:
	plDInput*					fDI;
	hsTArray<plDInputDevice*>	fInputDevice;
	static DIACTION				fActionMap[];
	HWND						fhWnd;
};
// function pointers to dinput callbacks
typedef int (__stdcall * Pfunc1) (const DIDEVICEINSTANCE* device, void* pRef);
// I should need these...
//typedef int (__stdcall * Pfunc2) (const DIDEVICEOBJECTINSTANCE* device, void* pRef);
//typedef int (__stdcall * Pfunc3) (const struct DIDEVICEINSTANCEA* devInst, struct IDirectInputDevice8* dev, unsigned long why, unsigned long devRemaining, void* pRef); 


plInputManager* plInputManager::fInstance = nil;

plInputManager::plInputManager( HWND hWnd ) :
fDInputMgr(nil),
fInterfaceMgr(nil)
{
	fhWnd = hWnd;
	fInstance = this;
	fActive = false;
	fFirstActivated = false;
	fMouseScale = 1.f;
}

plInputManager::plInputManager() :
fDInputMgr(nil),
fInterfaceMgr(nil)
{
	fInstance = this;
	fActive = false;
	fFirstActivated = false;
	fMouseScale = 1.f;
}

plInputManager::~plInputManager() 
{
	fInterfaceMgr->Shutdown();
	fInterfaceMgr = nil;

	for (int i = 0; i < fInputDevices.Count(); i++)
	{
		fInputDevices[i]->Shutdown();
		delete(fInputDevices[i]);
	}
	if (fDInputMgr)
		delete fDInputMgr;
}

//static
void plInputManager::SetRecenterMouse(hsBool b)
{ 
	if (b)
		bRecenterMouse++;
	else if (bRecenterMouse > 0)
		bRecenterMouse--;
}


void plInputManager::RecenterCursor()
{
	RECT rect;
	GetClientRect(fhWnd, &rect);
	POINT pt;
//	pt.y = ( (rect.bottom - rect.top) / 2 ) / fInstance->fMouseScale;
//	pt.x = ( (rect.right - rect.left) / 2 ) / fInstance->fMouseScale;
	ClientToScreen(fhWnd, &pt);
	SetCursorPos( pt.x, pt.y );
}
void plInputManager::CreateInterfaceMod(plPipeline* p)
{
	fInterfaceMgr = TRACKED_NEW plInputInterfaceMgr();
	fInterfaceMgr->Init();
}

void plInputManager::InitDInput(HINSTANCE hInst, HWND hWnd)
{
	if (fUseDInput)
	{
		fDInputMgr = TRACKED_NEW plDInputMgr;
		fDInputMgr->Init(hInst, hWnd);
	}
}

hsBool plInputManager::MsgReceive(plMessage* msg)
{
	for (int i=0; i<fInputDevices.Count(); i++)
		if (fInputDevices[i]->MsgReceive(msg))
			return true;

	if (fDInputMgr)
		return fDInputMgr->MsgReceive(msg);

	return hsKeyedObject::MsgReceive(msg);
}

void plInputManager::Update()
{
	if (fDInputMgr)
		fDInputMgr->Update();
}

void plInputManager::SetMouseScale( hsScalar s )
{
/*	RECT	rect;
	POINT	currPos;


	// Gotta make sure to move the mouse to the correct new position for the scale
	GetClientRect( fhWnd, &rect );
	GetCursorPos( &currPos );
	ScreenToClient( fhWnd, &currPos );

	float x = (float)currPos.x / rect.right;
	float y = (float)currPos.y / rect.bottom;

	x *= fMouseScale; y *= fMouseScale;

	fMouseScale = s;

	// Refreshes all of the input devices so that they can reset mouse limits, etc
	RECT rect2 = rect;
	rect2.right /= fMouseScale;
	rect2.bottom /= fMouseScale;
	::MapWindowPoints( fhWnd, NULL, (POINT *)&rect2, 2 );
	BOOL ret = ::ClipCursor( &rect );

	// Now move the cursor to the right spot

	currPos.x = ( x / fMouseScale ) * rect.right;
	currPos.y = ( y / fMouseScale ) * rect.bottom;

	ClientToScreen( fhWnd, &currPos );
	SetCursorPos( currPos.x, currPos.y );
*/
}

// Sometimes the keyboard driver "helps" us translating a key involved in a key
// combo. For example pressing shif-numpad8 will actually generate a KEY_UP event,
// the same as the up arrow. This function undoes that translation.
plKeyDef plInputManager::UntranslateKey(plKeyDef key, hsBool extended)
{
	if (!extended)
	{
		if (key == KEY_UP)
			return KEY_NUMPAD8;
		if (key == KEY_DOWN)
			return KEY_NUMPAD2;
		if (key == KEY_LEFT)
			return KEY_NUMPAD4;
		if (key == KEY_RIGHT)
			return KEY_NUMPAD6;
	}

	return key;
}
		
void plInputManager::HandleWin32ControlEvent(UINT message, WPARAM Wparam, LPARAM Lparam, HWND hWnd)
{
	if( !fhWnd )
		fhWnd = hWnd;

	hsBool bExtended;

	switch (message)
	{
	case SYSKEYDOWN:
	case KEYDOWN:
		{
			bExtended = Lparam >> 24 & 1;
			hsBool bRepeat = ((Lparam >> 29) & 0xf) != 0;
			for (int i=0; i<fInputDevices.Count(); i++)
				fInputDevices[i]->HandleKeyEvent( KEYDOWN, UntranslateKey((plKeyDef)Wparam, bExtended), true, bRepeat ); 
		}
		break;
	case SYSKEYUP:
	case KEYUP:
		{
			bExtended = Lparam >> 24 & 1;
			for (int i=0; i<fInputDevices.Count(); i++)
				fInputDevices[i]->HandleKeyEvent( KEYUP, UntranslateKey((plKeyDef)Wparam, bExtended), false, false ); 
		}
		break;
	case MOUSEWHEEL:
		{
			plMouseEventMsg* pMsg = TRACKED_NEW plMouseEventMsg;
			int zDelta = GET_WHEEL_DELTA_WPARAM(Wparam);
			pMsg->SetWheelDelta((float)zDelta);
			if (zDelta < 0)
				pMsg->SetButton(kWheelNeg);
			else
				pMsg->SetButton(kWheelPos);

			RECT rect;
			GetClientRect(hWnd, &rect);
			pMsg->SetXPos(LOWORD(Lparam) / (float)rect.right);
			pMsg->SetYPos(HIWORD(Lparam) / (float)rect.bottom);

			pMsg->Send();
		}
		break;
	case MOUSEMOVE:
	case L_BUTTONDN:
	case L_BUTTONUP:
	case R_BUTTONDN:
	case R_BUTTONUP:
	case L_BUTTONDBLCLK:
	case R_BUTTONDBLCLK:
	case M_BUTTONDN:
	case M_BUTTONUP:
		{
			
			RECT rect;
			GetClientRect(hWnd, &rect);
		 
			plIMouseXEventMsg* pXMsg = TRACKED_NEW plIMouseXEventMsg;
			plIMouseYEventMsg* pYMsg = TRACKED_NEW plIMouseYEventMsg;
			plIMouseBEventMsg* pBMsg = TRACKED_NEW plIMouseBEventMsg;

			pXMsg->fWx = LOWORD(Lparam);
			pXMsg->fX = (float)LOWORD(Lparam) / (float)rect.right;
			pYMsg->fWy = HIWORD(Lparam);
			pYMsg->fY = (float)HIWORD(Lparam) / (float)rect.bottom;

			// Apply mouse scale
//			pXMsg->fX *= fMouseScale;
//			pYMsg->fY *= fMouseScale;
			
			if (Wparam & MK_LBUTTON && message != L_BUTTONUP)
			{
				pBMsg->fButton |= kLeftButtonDown;
			}
			else
			{
				pBMsg->fButton |= kLeftButtonUp;
			}
						
			if (Wparam & MK_RBUTTON && message != R_BUTTONUP)
			{
				pBMsg->fButton |= kRightButtonDown;
			}
			else
			{
				pBMsg->fButton |= kRightButtonUp;
			}
			
			if (Wparam & MK_MBUTTON && message != M_BUTTONUP)
			{
				pBMsg->fButton |= kMiddleButtonDown;
			}
			else
			{
				pBMsg->fButton |= kMiddleButtonUp;
			}
			
			if( message == L_BUTTONDBLCLK )
				pBMsg->fButton |= kLeftButtonDblClk;		// We send the double clicks separately
			if( message == R_BUTTONDBLCLK )
				pBMsg->fButton |= kRightButtonDblClk;

			for (int i=0; i<fInputDevices.Count(); i++)
			{
				fInputDevices[i]->MsgReceive(pXMsg);
				fInputDevices[i]->MsgReceive(pYMsg);
				fInputDevices[i]->MsgReceive(pBMsg);
			}
			POINT pt;
			
			if (RecenterMouse() && (pXMsg->fX <= 0.1 || pXMsg->fX >= 0.9) )
			{		
				pt.x = (rect.right - rect.left) / 2;
				pt.y = HIWORD(Lparam);
				ClientToScreen(hWnd, &pt);
				SetCursorPos( pt.x, pt.y );
			}
			else
			if (RecenterMouse() && (pYMsg->fY <= 0.1 || pYMsg->fY >= 0.9) )
			{		
				pt.y = (rect.bottom - rect.top) / 2;
				pt.x = LOWORD(Lparam);
				ClientToScreen(hWnd, &pt);
				SetCursorPos( pt.x, pt.y );
			}
			if (RecenterMouse() && Lparam == 0)
			{
				pt.y = (rect.bottom - rect.top) / 2;
				pt.x = (rect.right - rect.left) / 2;
				ClientToScreen(hWnd, &pt);
				SetCursorPos( pt.x, pt.y );
			}
			delete(pXMsg);
			delete(pYMsg);
			delete(pBMsg);

			

		}
		break;
	case WM_ACTIVATE:
		{
			bool activated = ( LOWORD( Wparam ) == WA_INACTIVE ) ? false : true;
			Activate( activated );
		}
		break;
	}

}

//// Activate ////////////////////////////////////////////////////////////////
//	Handles what happens when the app (window) activates/deactivates

void	plInputManager::Activate( bool activating )
{
	int		i;


	for( i = 0; i < fInputDevices.GetCount(); i++ )
		fInputDevices[ i ]->HandleWindowActivate( activating, fhWnd );

	fActive = activating;
	fFirstActivated = true;
}

//// AddInputDevice //////////////////////////////////////////////////////////

void	plInputManager::AddInputDevice( plInputDevice *pDev )
{
	fInputDevices.Append( pDev ); 
	if( fFirstActivated )
		pDev->HandleWindowActivate( fActive, fhWnd );
}

//
//
// dinput manager
//
//


plDInputMgr::plDInputMgr() :
fDI(nil)
{
	fDI = TRACKED_NEW plDInput;
}

plDInputMgr::~plDInputMgr()
{
	if (fDI)
	{
		for (int i = 0; i < fDI->fSticks.Count(); i++)
		{	
			plDIDevice* pD = fDI->fSticks[i];
			pD->fDevice->Release();
			delete(pD->fCaps);
			delete(pD);
		}
		fDI->fSticks.SetCountAndZero(0);
		delete(fDI->fActionFormat);
		fDI->fDInput->Release();
		for(int j = 0; j < fInputDevice.Count(); j++)
			delete(fInputDevice[j]);
		fInputDevice.SetCountAndZero(0);
		delete fDI;
	}
}


void plDInputMgr::Init(HINSTANCE hInst, HWND hWnd)
{
	
	HRESULT         hr; 
	hr = DirectInput8Create(hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&fDI->fDInput, NULL); 
	hsAssert(!hr, "failed to initialize directInput!"); 
	// enumerate game controllers
	Pfunc1 fPtr = &plDInputMgr::EnumGamepadCallback;
	int i = 0;
	

	// set up the action mapping
	fDI->fActionFormat = TRACKED_NEW DIACTIONFORMAT;
	fDI->fActionFormat->dwSize        = sizeof(DIACTIONFORMAT);
	fDI->fActionFormat->dwActionSize  = sizeof(DIACTION);
	fDI->fActionFormat->dwDataSize    = NUM_ACTIONS * sizeof(DWORD);
	fDI->fActionFormat->dwNumActions  = NUM_ACTIONS;
	fDI->fActionFormat->guidActionMap = PL_ACTION_GUID;
	fDI->fActionFormat->dwGenre       = DIVIRTUAL_FIGHTING_THIRDPERSON;
	fDI->fActionFormat->rgoAction     = fActionMap;
	fDI->fActionFormat->dwBufferSize  = 16;
	fDI->fActionFormat->lAxisMin      = -1000;
	fDI->fActionFormat->lAxisMax      = 1000;
	sprintf( fDI->fActionFormat->tszActionMap, "Plasma 2.0 Game Actions" );

	// this call should not work:
	fDI->fDInput->EnumDevices(DI8DEVCLASS_GAMECTRL, fPtr, fDI, DIEDFL_ATTACHEDONLY); 
	
	// apply the mapping to the game controller	
	// this is the correct <but broken> way to apply the action map:
//	Pfunc3 fPtr3 = &plDInputMgr::EnumSuitableDevices;
//	hr = fDI->fDInput->EnumDevicesBySemantics(NULL, fDI->fActionFormat, EnumSuitableDevices, fDI, NULL);

	
	for (i = 0; i < fDI->fSticks.Count(); i++)
	{
		fDI->fSticks[i]->fCaps = TRACKED_NEW DIDEVCAPS; 
		fDI->fSticks[i]->fCaps->dwSize = sizeof(DIDEVCAPS);
		hr = fDI->fSticks[i]->fDevice->GetCapabilities(fDI->fSticks[i]->fCaps);
		hsAssert(!hr, "Unable to acquire devcaps in DInput Device!"); 
		hr = fDI->fSticks[i]->fDevice->Acquire();
		hsAssert(!hr, "Unable to acquire DInput Device!"); 
	}

	fhWnd = hWnd;
	
	for (i = 0; i < fDI->fSticks.Count(); i++)
		fInputDevice.Append( TRACKED_NEW plDInputDevice );
}

void plDInputMgr::Update()
{
	HRESULT     hr;
 
    if (!fDI->fSticks.Count()) 
        return;

    // Poll the devices to read the current state
    for (int i = 0; i < fDI->fSticks.Count(); i++)
	{
 		hr = fDI->fSticks[i]->fDevice->Poll(); 
		if (FAILED(hr))  
		{
			// Attempt to reacquire joystick
			while(hr == DIERR_INPUTLOST) 
			{	
				hr = fDI->fSticks[i]->fDevice->Acquire();
				char str[256];
				sprintf(str, "DInput Device # %d connection lost - press Ignore to attempt to reacquire!", i);
				hsAssert(!hr, str); 
			}
		}
		
		DIDEVICEOBJECTDATA data; 
		ULONG size = 1;
		hr = fDI->fSticks[i]->fDevice->GetDeviceData(sizeof(DIDEVICEOBJECTDATA),&data,&size,0); 

		fInputDevice[i]->Update(&data);
	}
}

void plDInputMgr::AddDevice(IDirectInputDevice8* device)
{
	HRESULT hr = device->BuildActionMap(fDI->fActionFormat, NULL, NULL);
	if (!FAILED(hr))
		device->SetActionMap( fDI->fActionFormat, NULL, NULL );
}

void plDInputMgr::ConfigureDevice()
{
	::ClipCursor(nil);
	::ShowCursor( TRUE );
	ReleaseCapture();
		

	DICOLORSET dics;
    ZeroMemory(&dics, sizeof(DICOLORSET));
    dics.dwSize = sizeof(DICOLORSET);

    DICONFIGUREDEVICESPARAMS dicdp;
    ZeroMemory(&dicdp, sizeof(dicdp));
    dicdp.dwSize = sizeof(dicdp);
    dicdp.dwcUsers       = 1;
    dicdp.lptszUserNames = NULL;
    
    dicdp.dwcFormats     = 1;
    dicdp.lprgFormats    = fDI->fActionFormat;
    dicdp.hwnd           = fhWnd;
    dicdp.lpUnkDDSTarget = NULL;

	fDI->fDInput->ConfigureDevices(NULL, &dicdp, DICD_EDIT, NULL);
	for (int i = 0; i < fDI->fSticks.Count(); i++)
		fDI->fSticks[i]->fDevice->SetActionMap( fDI->fActionFormat, NULL, DIDSAM_FORCESAVE );

	RECT rect;
	::GetClientRect(fhWnd,&rect);
	::ClientToScreen(fhWnd,(LPPOINT)&rect);
	::ClipCursor(&rect);
	::ShowCursor( FALSE );
	SetCapture(fhWnd);

}

hsBool plDInputMgr::MsgReceive(plMessage* msg)
{
	plInputEventMsg* pMsg = plInputEventMsg::ConvertNoRef(msg);
	if (pMsg && pMsg->fEvent == plInputEventMsg::kConfigure)
	{
		ConfigureDevice();
	}
	return false;
}

// dinput required callback functions:

// enumerate the dinput devices 
int __stdcall plDInputMgr::EnumGamepadCallback(const DIDEVICEINSTANCE* device, void* pRef)
{
	HRESULT hr;
	
	plDInput* pDI = (plDInput*)pRef;
	IDirectInputDevice8* fStick = nil;
    hr = pDI->fDInput->CreateDevice(device->guidInstance, &fStick, NULL);
	
	if(!FAILED(hr)) 
	{
        pDI->fSticks.Append(TRACKED_NEW plDIDevice(fStick));
		
		// the following code pertaining to the action map shouldn't be here.
		// in fact this shouldn't work at all according to MS, but this is 
		// currently the only way this works.  Whatever - the correct
		// code is here and commented out in case this ever gets fixed by MS
		// in a future release of dinput.
		HRESULT hr = fStick->BuildActionMap(pDI->fActionFormat, NULL, NULL);
		if (!FAILED(hr))
		{
			hr = fStick->SetActionMap( pDI->fActionFormat, NULL, NULL );

			DIPROPDWORD dipW; 
			dipW.diph.dwSize		= sizeof(DIPROPDWORD); 
			dipW.diph.dwHeaderSize	= sizeof(DIPROPHEADER); 
			dipW.diph.dwHow			= DIPH_DEVICE; 
			dipW.diph.dwObj			= 0;
			dipW.dwData				= 500; // 5% of axis range for deadzone

			hr = fStick->SetProperty(DIPROP_DEADZONE , &dipW.diph);
		}
		return DIENUM_CONTINUE;
	}
    return DIENUM_STOP;
}

// look for axes on the controller and set the output range to +-100
// apparently not needed with action mapping:
/*
int __stdcall plDInputMgr::SetAxisRange(const DIDEVICEOBJECTINSTANCE* obj, void* pRef)
{
	HRESULT hr;
	DIPROPRANGE diprg; 

	diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
	diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
	diprg.diph.dwHow        = DIPH_BYID; 
	diprg.diph.dwObj        = obj->dwType; 
	diprg.lMin              = -100; 
	diprg.lMax              = +100; 

	plDInput* pDI = (plDInput*)pRef;
	for (int i = 0; i < pDI->fSticks.Count(); i++)
		hr = pDI->fSticks[i]->fDevice->SetProperty(DIPROP_RANGE, &diprg.diph);

	if(!FAILED(hr)) 
        return DIENUM_CONTINUE;
	
    return DIENUM_STOP;

}
*/

// apply mapping to controller
// not used.  why? no one really knows.
// leave this here in case dinput ever gets fixed...
/*
int __stdcall plDInputMgr::EnumSuitableDevices(const struct DIDEVICEINSTANCEA* devInst, struct IDirectInputDevice8* dev, unsigned long why, unsigned long devRemaining, void* pRef)
{
	plDInput* pDI = (plDInput*)pRef;
	HRESULT hr = dev->BuildActionMap(pDI->fActionFormat, NULL, NULL);
	if (!FAILED(hr))
	{
		hr = dev->SetActionMap( pDI->fActionFormat, NULL, NULL );
	}
	return DIENUM_STOP;
}
*/
DIACTION plDInputMgr::fActionMap[NUM_ACTIONS] =
{
	{A_CONTROL_MOVE,			DIAXIS_TPS_MOVE,		0,	"Walk Forward-Backward"	,},
	{A_CONTROL_TURN,			DIAXIS_TPS_TURN,		0,	"Turn Left-Right"		,},
	{A_CONTROL_MOUSE_X,			DIAXIS_ANY_1,			0,	"Move Camera Left-Right",},
	{A_CONTROL_MOUSE_Y,			DIAXIS_ANY_2,			0,	"Move Camera Up-Down"	,},
	{B_CONTROL_ACTION,			DIBUTTON_TPS_ACTION,	0,	"Action"				,},
	{B_CONTROL_JUMP,			DIBUTTON_TPS_JUMP,		0,	"Jump"					,},
	{B_CONTROL_STRAFE_LEFT,		DIBUTTON_TPS_STEPLEFT,	0,	"Strafe Left"			,},
	{B_CONTROL_STRAFE_RIGHT,	DIBUTTON_TPS_STEPRIGHT,	0,	"Strafe Right"			,},
	{B_CONTROL_MODIFIER_FAST,	DIBUTTON_TPS_RUN,		0,	"Run"					,},
	{B_CONTROL_EQUIP,			DIBUTTON_TPS_SELECT,	0,	"Equip Item"			,},
	{B_CONTROL_DROP,			DIBUTTON_TPS_USE,		0,	"Drop Item"				,},
	{B_CONTROL_MOVE_FORWARD,	DIBUTTON_ANY(0),		0,	"Walk Forward"			,},
	{B_CONTROL_MOVE_BACKWARD,	DIBUTTON_ANY(1),		0,	"Walk Backward"			,},
	{B_CONTROL_ROTATE_LEFT,		DIBUTTON_ANY(2),		0,	"Turn Left"				,},
	{B_CONTROL_ROTATE_RIGHT,	DIBUTTON_ANY(3),		0,	"Turn Right"			,},
	{B_CONTROL_TURN_TO,			DIBUTTON_ANY(4),		0,	"Pick Item"				,},
	{B_CAMERA_RECENTER,			DIBUTTON_ANY(5),		0,	"Recenter Camera"		,},
};
