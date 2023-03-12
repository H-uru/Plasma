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
#ifndef _pyNotify_h_
#define _pyNotify_h_

//////////////////////////////////////////////////////////////////////
//
// pyNotify   - a wrapper class to provide interface to send a NotifyMsg
//
//////////////////////////////////////////////////////////////////////

#include "pnMessage/plNotifyMsg.h"
#include "pyGlueHelpers.h"

class pyPoint3;

class pyNotify
{
private:
    plKey           fSenderKey;     // the holder of the who (the modifier) we are
    // the list of receivers that want to be notified
    std::vector<plKey> fReceivers;

    bool            fNetPropagate;
    bool            fNetForce;

    // notify message build area
    plNotifyMsg     fBuildMsg;

protected:
    pyNotify(); // only used by python glue, do NOT call
    pyNotify(const pyKey& selfkey);

public:
    virtual ~pyNotify();

    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptNotify);
    static PyObject *New(const pyKey& selfkey);
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyNotify object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyNotify); // converts a PyObject to a pyNotify (throws error if not correct type)

    static void AddPlasmaClasses(PyObject *m);
    static void AddPlasmaConstantsClasses(PyObject *m);

    void SetSender(const pyKey& selfkey); // only used by python glue, do NOT call

    // methods that will be exposed to Python
    virtual void ClearReceivers();
    virtual void AddReceiver(pyKey* key);
    virtual void SetNetPropagate(bool propagate) { fNetPropagate = propagate; }
    virtual void SetNetForce(bool state) { fNetForce = state; }
    virtual void SetActivateState(float state);
    virtual void SetType(int32_t type);

    // add event record helpers
    virtual void AddCollisionEvent( bool enter, pyKey* other, pyKey* self );
    virtual void AddPickEvent(bool enabled, pyKey* other, pyKey* self, pyPoint3 hitPoint);
    virtual void AddControlKeyEvent( int32_t key, bool down );
    virtual void AddVarNumber(const ST::string& name, float number);
    virtual void AddVarNumber(const ST::string& name, int32_t number);
    virtual void AddVarNull(const ST::string& name);
    virtual void AddVarKey(const ST::string& name, pyKey* key);
    virtual void AddFacingEvent( bool enabled, pyKey* other, pyKey* self, float dot);
    virtual void AddContainerEvent( bool entering, pyKey* container, pyKey* contained);
    virtual void AddActivateEvent( bool active, bool activate );
    virtual void AddCallbackEvent( int32_t event );
    virtual void AddResponderState(int32_t state);

    virtual void Send();

};

#endif // _pyNotify_h_
