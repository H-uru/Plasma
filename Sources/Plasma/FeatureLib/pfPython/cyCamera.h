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
#ifndef cyCamera_h
#define cyCamera_h

/////////////////////////////////////////////////////////////////////////////
//
// NAME: cyCamera
//
// PURPOSE: Class wrapper to map camera functions to plasma2 message
//

#include "HeadSpin.h"
#include "pyGlueHelpers.h"
#include "pnKeyedObject/plKey.h"

class cyCamera
{
protected:
    plKey       fSender;
    plKey       fTheCam;

    cyCamera();
public:
    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptCamera);
    PYTHON_CLASS_NEW_DEFINITION;
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a cyCamera object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(cyCamera); // converts a PyObject to a cyCamera (throws error if not correct type)

    static void AddPlasmaClasses(PyObject *m);

    // setters
    void SetSender(plKey &sender);

    // Save the current state of the virtual camera
    // NOTE: doesn't work by itself at the moment
    virtual void Push(pyKey& newCamKey);

    // Restore the state of the virtual camera with a previously saved setting
    virtual void Pop(pyKey& oldCamKey);

    // Send controlKey commands to the virtual camera (should be like a pass thru)
    virtual void ControlKey(int32_t controlKey, hsBool activated);


    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : TransitionTo
    //  PARAMETERS : newCamKey  - what to switch the camera to
    //             : time       - how long it takes to transition to new camera
    //
    //  PURPOSE    : Transition to a new camera (position and settings)
    //
    virtual void TransitionTo(pyKey& newCamKey, double time, hsBool save);
    
    virtual void SetEnableFirstPersonOverride(hsBool state);
    virtual void EnableFirstPersonOverride() { SetEnableFirstPersonOverride(true); }
    virtual void DisableFirstPersonOverride() { SetEnableFirstPersonOverride(false); }
    
    virtual void UndoFirstPerson();

    virtual float GetFOV();
    virtual void SetFOV(float fov, double t);

    virtual void SetSmootherCam(hsBool state);
    virtual hsBool IsSmootherCam();

    virtual void SetWalkAndVerticalPan(hsBool state);
    virtual hsBool IsWalkAndVerticalPan();

    virtual void SetStayInFirstPerson(hsBool state);
    virtual hsBool IsStayInFirstPerson();

    virtual void SetAspectRatio(float aspectratio);
    virtual float GetAspectRatio();
    virtual void RefreshFOV();
};


#endif  // cyCamera_h
