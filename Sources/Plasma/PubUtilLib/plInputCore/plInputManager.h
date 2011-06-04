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
// plInputManager.h

#ifndef PL_INPUT_MANAGER_H
#define PL_INPUT_MANAGER_H

#include <windows.h>
#include "hsTypes.h"
#include "hsTemplates.h"
#include "../pnKeyedObject/hsKeyedObject.h"
#include "../pnInputCore/plInputMap.h"

class plDInputMgr;
class plInputDevice;
class plDInputDevice;
class plInputInterfaceMgr;
class plPipeline;

class plInputManager :public hsKeyedObject
{
private:
	static hsBool fUseDInput;
public:
	plInputManager();
	plInputManager( HWND hWnd );
	~plInputManager();
	
	CLASSNAME_REGISTER( plInputManager );
	GETINTERFACE_ANY( plInputManager, hsKeyedObject );


	void AddInputDevice(plInputDevice* pDev);
	void InitDInput(HINSTANCE hInst, HWND hWnd);

	static void UseDInput(hsBool b) { fUseDInput = b; }
	void Update();
	static plInputManager*	GetInstance() { return fInstance; }
	static plInputManager*	fInstance;
	virtual hsBool MsgReceive(plMessage* msg);
	static hsBool RecenterMouse() { return bRecenterMouse > 0; }
	static void SetRecenterMouse(hsBool b); 
	static void RecenterCursor();
	void CreateInterfaceMod(plPipeline* p);

	void	Activate( bool activating );

	hsScalar	GetMouseScale( void ) const { return fMouseScale; }
	void		SetMouseScale( hsScalar s );
	
	static plKeyDef UntranslateKey(plKeyDef key, hsBool extended);
	
protected:
	
	hsTArray<plInputDevice*>	fInputDevices;
	plDInputMgr*				fDInputMgr;
	plInputInterfaceMgr			*fInterfaceMgr;
	bool						fActive, fFirstActivated;		

	hsScalar					fMouseScale;
	static UInt8				bRecenterMouse;
	static HWND					fhWnd;
	
public:
	// event handlers
	void HandleWin32ControlEvent(UINT message, WPARAM Wparam, LPARAM Lparam, HWND hWnd);
};


// {049DE53E-23A2-4d43-BF68-36AC1B57E357}
static const GUID PL_ACTION_GUID = { 0x49de53e, 0x23a2, 0x4d43, { 0xbf, 0x68, 0x36, 0xac, 0x1b, 0x57, 0xe3, 0x57 } };

#endif
