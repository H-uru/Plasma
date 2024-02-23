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
#ifndef plPythonSDLModifier_h_inc
#define plPythonSDLModifier_h_inc

class plPythonFileMod;
class plStateDataRecord;
class plSimpleStateVariable;
class pyKey;

#include <map>
#include "plModifier/plSDLModifier.h"

#include "pyGlueDefinitions.h"


// hack for plNetClientVNodeMgr single-player mode SDLHook stuff.
plStateDataRecord * GetAgeSDL();

//
// The fields of a SDL record in Python format.
// If the Python code changes a value an update is sent automatically
//
class plPythonSDLModifier : public plSDLModifier
{
protected:
    class SDLObj
    {
    public:
        PyObject* obj;
        int size;       // 0 for resizable
        bool sendToClients;
        bool skipLocalCheck;
        bool sendImmediate;
        ST::string hintString;
        SDLObj() : obj(), size(-1), sendToClients(), skipLocalCheck(), sendImmediate() { }
        SDLObj(PyObject* obj, int size, bool sendToClients)
            : obj(obj), size(size), sendToClients(sendToClients), skipLocalCheck(), sendImmediate() { }
    };
    typedef std::map<ST::string, SDLObj> SDLMap;
    SDLMap fMap;
    plPythonFileMod* fOwner;

    PyObject* ISDLVarToPython(plSimpleStateVariable* var);
    PyObject* ISDLVarIdxToPython(plSimpleStateVariable* var, int type, int idx);

    void IPythonVarToSDL(plStateDataRecord* state, const ST::string& name);
    bool IPythonVarIdxToSDL(plSimpleStateVariable* var, int varIdx, int type, PyObject* pyVar, const ST::string& hintstring);

    void ISetItem(const ST::string& key, PyObject* value);
    void IDirtySynchState(const ST::string& name, bool sendImmediate = false);

    void IPutCurrentStateIn(plStateDataRecord* dstState) override;
    void ISetCurrentStateFrom(const plStateDataRecord* srcState) override;
public:
    plPythonSDLModifier() : fOwner() {}
    plPythonSDLModifier(plPythonFileMod* owner);
    ~plPythonSDLModifier();

    CLASSNAME_REGISTER(plPythonSDLModifier);
    GETINTERFACE_ANY(plPythonSDLModifier, plSDLModifier);

    const char* GetSDLName() const override;

    static bool HasSDL(const ST::string& pythonFile);
    // find the Age global SDL guy... if there is one
    static plPythonSDLModifier* FindAgeSDL();
    static plKey FindAgeSDLTarget();

    void SetDefault(const ST::string& key, PyObject* value);
    void SendToClients(const ST::string& key);
    void SetNotify(pyKey& selfkey, const ST::string& key, float tolerance);

    PyObject* GetItem(const ST::string& key);

    template<typename T>
    void SetItem(const ST::string& key, int index, T value);
    void SetItem(const ST::string& key, PyObject* value);

    void SetItemIdx(const ST::string& key, int idx, PyObject* value, bool sendImmediate = false);
    void SetFlags(const ST::string& name, bool sendImmediate, bool skipOwnershipCheck);
    void SetTagString(const ST::string& name, const ST::string& tag);
};

// A wrapper for plPythonSDLModifier that Python uses
class pySDLModifier
{
protected:
    plPythonSDLModifier* fRecord;

    pySDLModifier(plPythonSDLModifier* sdlMod);
    pySDLModifier() : fRecord() { }
public:
    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptSDL);
    PYTHON_CLASS_NEW_DEFINITION;
    static PyObject *New(plPythonSDLModifier *sdlMod);
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pySDLModifier object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pySDLModifier); // converts a PyObject to a pySDLModifier (throws error if not correct type)

    static void AddPlasmaClasses(PyObject *m);
    static void AddPlasmaMethods(PyObject* m);

    // global function to get the GrandMaster Age SDL object
    static PyObject* GetAgeSDL();

    static void SetDefault(pySDLModifier& self, const ST::string& key, PyObject* value);
    static void SendToClients(pySDLModifier& self, const ST::string& key);
    static void SetNotify(pySDLModifier& self, pyKey& selfkey, const ST::string& key, float tolerance);
    
    static PyObject* GetItem(pySDLModifier& self, const ST::string& key);
    static void SetItem(pySDLModifier& self, const ST::string& key, PyObject* value);
    static void SetItemIdx(pySDLModifier& self, const ST::string& key, int idx, PyObject* value);
    static void SetItemIdxImmediate(pySDLModifier& self, const ST::string& key, int idx, PyObject* value);
    static void SetFlags(pySDLModifier& self, const ST::string& name, bool sendImmediate, bool skipOwnershipCheck);
    static void SetTagString(pySDLModifier& self, const ST::string& name, const ST::string& tag);
    
};

#endif // plPythonSDLModifier_h_inc
