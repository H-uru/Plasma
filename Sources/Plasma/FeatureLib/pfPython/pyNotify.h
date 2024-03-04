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

#include <vector>

#include "pnKeyedObject/plKey.h"
#include "pnMessage/plNotifyMsg.h"

#include "pyGlueDefinitions.h"

class pyKey;
class pyPoint3;
namespace ST { class string; }

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
    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptNotify);
    static PyObject *New(const pyKey& selfkey);
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyNotify object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyNotify); // converts a PyObject to a pyNotify (throws error if not correct type)

    static void AddPlasmaClasses(PyObject *m);
    static void AddPlasmaConstantsClasses(PyObject *m);

    void SetSender(const pyKey& selfkey); // only used by python glue, do NOT call

    // methods that will be exposed to Python
    void ClearReceivers();
    void AddReceiver(pyKey* key);
    void SetNetPropagate(bool propagate) { fNetPropagate = propagate; }
    void SetNetForce(bool state) { fNetForce = state; }
    void SetActivateState(float state);
    void SetType(int32_t type);

    // add event record helpers
    void AddCollisionEvent(bool enter, pyKey* other, pyKey* self);
    void AddPickEvent(bool enabled, pyKey* other, pyKey* self, pyPoint3 hitPoint);
    void AddControlKeyEvent(int32_t key, bool down);
    void AddVarNumber(const ST::string& name, float number);
    void AddVarNumber(const ST::string& name, int32_t number);
    void AddVarNull(const ST::string& name);
    void AddVarKey(const ST::string& name, pyKey* key);
    void AddFacingEvent(bool enabled, pyKey* other, pyKey* self, float dot);
    void AddContainerEvent(bool entering, pyKey* container, pyKey* contained);
    void AddActivateEvent(bool active, bool activate);
    void AddCallbackEvent(int32_t event);
    void AddResponderState(int32_t state);

    void Send();

};

#endif // _pyNotify_h_
