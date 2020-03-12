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
// plInputManager.h

#ifndef PL_INPUT_MANAGER_H
#define PL_INPUT_MANAGER_H


#include "HeadSpin.h"
#include "hsTemplates.h"
#include "pnKeyedObject/hsKeyedObject.h"
#include "pnInputCore/plInputMap.h"
#include <locale>

class plDInputMgr;
class plInputDevice;
class plDInputDevice;
class plInputInterfaceMgr;
class plPipeline;

class plInputManager :public hsKeyedObject
{
private:
    static bool fUseDInput;
public:
    plInputManager();
    plInputManager( hsWindowHndl hWnd );
    ~plInputManager();
    
    CLASSNAME_REGISTER( plInputManager );
    GETINTERFACE_ANY( plInputManager, hsKeyedObject );


    void AddInputDevice(plInputDevice* pDev);
    void InitDInput(hsWindowInst hInst, hsWindowHndl hWnd);

    static void UseDInput(bool b) { fUseDInput = b; }
    void Update();
    static plInputManager*  GetInstance() { return fInstance; }
    static plInputManager*  fInstance;
    virtual bool MsgReceive(plMessage* msg);
    static bool RecenterMouse() { return bRecenterMouse > 0; }
    static void SetRecenterMouse(bool b); 
    static void RecenterCursor();
    void CreateInterfaceMod(plPipeline* p);

    void    Activate( bool activating );

    float    GetMouseScale() const { return fMouseScale; }
    void        SetMouseScale( float s );
    
    static plKeyDef UntranslateKey(plKeyDef key, bool extended);
    
protected:
    
    hsTArray<plInputDevice*>    fInputDevices;
    plDInputMgr*                fDInputMgr;
    plInputInterfaceMgr         *fInterfaceMgr;
    bool                        fActive, fFirstActivated;       

    float                    fMouseScale;
    static uint8_t           bRecenterMouse;
    static hsWindowHndl      fhWnd;
	std::locale localeC;
    
public:
#if HS_BUILD_FOR_WIN32
    // event handlers
    void HandleWin32ControlEvent(UINT message, WPARAM Wparam, LPARAM Lparam, HWND hWnd);
#endif
};

#if HS_BUILD_FOR_WIN32
// {049DE53E-23A2-4d43-BF68-36AC1B57E357}
static const GUID PL_ACTION_GUID = { 0x49de53e, 0x23a2, 0x4d43, { 0xbf, 0x68, 0x36, 0xac, 0x1b, 0x57, 0xe3, 0x57 } };
#endif

#endif
