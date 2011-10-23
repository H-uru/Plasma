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
#ifndef cyAnimation_h
#define cyAnimation_h

/////////////////////////////////////////////////////////////////////////////
//
// NAME: cyAnimation
//
// PURPOSE: Class wrapper to map animation functions to plasma2 message
//

#include "pyKey.h"
#include "hsTemplates.h"

#include <Python.h>
#include "pyGlueHelpers.h"

class cyAnimation
{

    plKey           fSender;
    hsTArray<plKey> fRecvr;
    char*           fAnimName;
    hsBool          fNetForce;

    virtual void IRunOneCmd(int cmd);

protected:
    cyAnimation();
    cyAnimation(pyKey& sender);

    // copy constructor
    cyAnimation(const cyAnimation& anim);
public:

    // clean up on the way out
    ~cyAnimation();

    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptAnimation);
    PYTHON_CLASS_NEW_DEFINITION;
    static PyObject *New(PyObject *sender);
    static PyObject *New(cyAnimation &obj);
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a cyAnimation object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(cyAnimation); // converts a PyObject to a cyAnimation (throws error if not correct type)

    static void AddPlasmaClasses(PyObject *m);

    // setters
    virtual void SetSender(pyKey& sender);
    virtual void AddRecvr(pyKey& recvr);
    virtual void SetAnimName(const char* name);

    virtual PyObject* GetFirstRecvr();

    virtual void SetNetForce(hsBool state);

    //  Play animation from start to end (whatever is already set)
    //
    virtual void Play();

    //  Stop an animation
    //
    virtual void Stop();

    //  Continue playing animation from wherever it last stopped
    //
    virtual void Resume();

    //  Play an animation only from specific time start to end
    //
    virtual void PlayRange(hsScalar start, hsScalar end);

    //  Play (continue) an animation until the specified time is reached
    //
    virtual void PlayToTime(hsScalar time);

    //  Play (continue) an animation until the specified point is reached
    //
    virtual void PlayToPercentage(hsScalar zeroToOne);  

    //  Jump the animation to the specified time
    //  Doesn't start or stop playing of animation
    //
    virtual void SkipToTime(hsScalar time);

    //  Set whether the animation is to be looped or not
    //
    virtual void Looped(hsBool looped);

    //  Sets the backwards state for the animation
    //
    virtual void Backwards(hsBool backwards);


    // Sets the start and end of the looping points in the animation
    //
    virtual void SetLoopStart(hsScalar start);
    virtual void SetLoopEnd(hsScalar end);
    
    //  Sets the speed of the animation
    //  Doesn't start or stop playing animation
    //
    virtual void Speed(hsScalar speed);


    //  Jump the animation to the specified time
    //  Doesn't start or stop playing of animation
    //
    virtual void SkipToBegin();
    virtual void SkipToEnd();
    virtual void SkipToLoopBegin();
    virtual void SkipToLoopEnd();

    //  Bump the animation ahead one frame (whatever deltime is)
    //
    virtual void IncrementForward();

    //  Bump the animation back one frame (whatever deltime is)
    //
    virtual void IncrementBackward();
};


#endif  // cyAnimation_h
