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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
// plDInputManager.cpp

#include "plDInputManager.h"
#include "plDInputDevice.h"

#include "pnInputCore/plControlEventCodes.h"
#include "plMessage/plInputEventMsg.h"

#define NUM_ACTIONS     17

// function pointers to dinput callbacks
typedef int (__stdcall * Pfunc1) (const DIDEVICEINSTANCE* device, void* pRef);
// I should need these...
//typedef int (__stdcall * Pfunc2) (const DIDEVICEOBJECTINSTANCE* device, void* pRef);
//typedef int (__stdcall * Pfunc3) (const struct DIDEVICEINSTANCEA* devInst, struct IDirectInputDevice8* dev, unsigned long why, unsigned long devRemaining, void* pRef); 


//
//
// dinput manager
//
//


plDInputMgr::plDInputMgr() :
fDI(nil)
{
    fDI = new plDInput;
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
    fDI->fActionFormat = new DIACTIONFORMAT;
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
//  Pfunc3 fPtr3 = &plDInputMgr::EnumSuitableDevices;
//  hr = fDI->fDInput->EnumDevicesBySemantics(NULL, fDI->fActionFormat, EnumSuitableDevices, fDI, NULL);

    
    for (i = 0; i < fDI->fSticks.Count(); i++)
    {
        fDI->fSticks[i]->fCaps = new DIDEVCAPS; 
        fDI->fSticks[i]->fCaps->dwSize = sizeof(DIDEVCAPS);
        hr = fDI->fSticks[i]->fDevice->GetCapabilities(fDI->fSticks[i]->fCaps);
        hsAssert(!hr, "Unable to acquire devcaps in DInput Device!"); 
        hr = fDI->fSticks[i]->fDevice->Acquire();
        hsAssert(!hr, "Unable to acquire DInput Device!"); 
    }

    fhWnd = hWnd;
    
    for (i = 0; i < fDI->fSticks.Count(); i++)
        fInputDevice.Append( new plDInputDevice );
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
    ::ShowCursor( TRUE );

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

    ::ShowCursor( FALSE );
}

bool plDInputMgr::MsgReceive(plMessage* msg)
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
        pDI->fSticks.Append(new plDIDevice(fStick));
        
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
            dipW.diph.dwSize        = sizeof(DIPROPDWORD); 
            dipW.diph.dwHeaderSize  = sizeof(DIPROPHEADER); 
            dipW.diph.dwHow         = DIPH_DEVICE; 
            dipW.diph.dwObj         = 0;
            dipW.dwData             = 500; // 5% of axis range for deadzone

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
    {A_CONTROL_MOVE,            DIAXIS_TPS_MOVE,        0,  "Walk Forward-Backward" ,},
    {A_CONTROL_TURN,            DIAXIS_TPS_TURN,        0,  "Turn Left-Right"       ,},
    {A_CONTROL_MOUSE_X,         DIAXIS_ANY_1,           0,  "Move Camera Left-Right",},
    {A_CONTROL_MOUSE_Y,         DIAXIS_ANY_2,           0,  "Move Camera Up-Down"   ,},
    {B_CONTROL_ACTION,          DIBUTTON_TPS_ACTION,    0,  "Action"                ,},
    {B_CONTROL_JUMP,            DIBUTTON_TPS_JUMP,      0,  "Jump"                  ,},
    {B_CONTROL_STRAFE_LEFT,     DIBUTTON_TPS_STEPLEFT,  0,  "Strafe Left"           ,},
    {B_CONTROL_STRAFE_RIGHT,    DIBUTTON_TPS_STEPRIGHT, 0,  "Strafe Right"          ,},
    {B_CONTROL_MODIFIER_FAST,   DIBUTTON_TPS_RUN,       0,  "Run"                   ,},
    {B_CONTROL_EQUIP,           DIBUTTON_TPS_SELECT,    0,  "Equip Item"            ,},
    {B_CONTROL_DROP,            DIBUTTON_TPS_USE,       0,  "Drop Item"             ,},
    {B_CONTROL_MOVE_FORWARD,    DIBUTTON_ANY(0),        0,  "Walk Forward"          ,},
    {B_CONTROL_MOVE_BACKWARD,   DIBUTTON_ANY(1),        0,  "Walk Backward"         ,},
    {B_CONTROL_ROTATE_LEFT,     DIBUTTON_ANY(2),        0,  "Turn Left"             ,},
    {B_CONTROL_ROTATE_RIGHT,    DIBUTTON_ANY(3),        0,  "Turn Right"            ,},
    {B_CONTROL_TURN_TO,         DIBUTTON_ANY(4),        0,  "Pick Item"             ,},
    {B_CAMERA_RECENTER,         DIBUTTON_ANY(5),        0,  "Recenter Camera"       ,},
};
