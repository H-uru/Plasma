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
#ifndef _pySceneObject_h_
#define _pySceneObject_h_

//////////////////////////////////////////////////////////////////////
//
// pySceneObject   - a wrapper class to provide interface to modifier
//                   attached to a SceneObject
//
//////////////////////////////////////////////////////////////////////

#include <vector>

#include "pnKeyedObject/plKey.h"

#include "pyGlueDefinitions.h"
#include "pyKey.h"

class pyMatrix44;
namespace ST { class string; }

class pySceneObject
{
private:
    std::vector<plKey>  fSceneObjects;
    plKey               fSenderKey;     // the holder of the who (the modifier) we are
    plKey               fPyMod;         // pyKey that points to modifier
    bool                fNetForce;

    void IAddObjKeyToAll(const plKey& key);
    void ISetAllSenderKeys();

protected:
    pySceneObject();
    pySceneObject(const pyKey& objkey, const pyKey& selfkey);
    pySceneObject(const plKey& objkey, const pyKey& selfkey);
    pySceneObject(const plKey& objkey);

public:
    ~pySceneObject();

    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptSceneobject);
    static PyObject *New(const plKey& objKey, PyObject *selfKeyObj);
    static PyObject *New(const plKey& objKey, pyKey &selfKey);
    static PyObject *New(const plKey& objKey, const plKey& selfkey);
    static PyObject *New(const plKey& objKey);
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pySceneObject object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pySceneObject); // converts a PyObject to a pySceneObject (throws error if not correct type)

    static void AddPlasmaClasses(PyObject *m);

    // override the equals to operator
    bool operator==(const pySceneObject &sobj) const;
    bool operator!=(const pySceneObject &sobj) const { return !(sobj == *this);   }

    PyObject*               fDraw; // cyDraw
    PyObject*               fPhysics; // cyPhysics
    PyObject*               fAvatar; // cyAvatar
    PyObject*               fParticle; // cyParticleSys

    // getter and setters
    void addObjKey(const plKey& key);
    void addObjPyKey(const pyKey& objkey);
    plKey getObjKey() const;
    PyObject* getObjPyKey() const; // pyKey

    void setSenderKey(plKey key);
    void setPyMod(const pyKey& pymod) { fPyMod = pymod.getKey(); }
    void setPyMod(const plKey& key) { fPyMod = key; }

    void SetNetForce(bool state);

    PyObject* findObj(const ST::string& name); // pySceneObject

    ST::string GetName();
    std::vector<PyObject*> GetResponders(); // pyKey list
    std::vector<PyObject*> GetPythonMods(); // pyKey list
    std::vector<PyObject*> GetImageLibMods(); // pyKey list
    //
    // deteremine if this object (or the first object in the list)
    // ...is locally owned
    bool IsLocallyOwned();
    
    //
    // get the local to world matrix
    PyObject* GetLocalToWorld();

    //
    // get the local to world matrix
    PyObject* GetWorldToLocal();

    //
    // get the local to world matrix
    PyObject* GetLocalToParent();

    //
    // get the local to world matrix
    PyObject* GetParentToLocal();

    //
    // set the local to world matrix
    void SetTransform(pyMatrix44& l2w, pyMatrix44& w2l);

    //
    // find the position of this object (if there are more than one, just the first one)
    PyObject* GetWorldPosition(); // pyPoint3

    //
    // find the view vector for this object (if there are more than one, just the first one)
    PyObject* GetViewVector(); // pyVector3

    //
    // find the up vector for this object (if there are more than one, just the first one)
    PyObject* GetUpVector(); // pyVector3

    //
    // find the up vector for this object (if there are more than one, just the first one)
    PyObject* GetRightVector(); // pyVector3

    //
    // deteremine if this object (or any of the object attached)
    // ...is an avatar, of any type
    bool IsAvatar();

    PyObject* GetAvatarVelocity(); // pyVector3

    //
    // deteremine if this object (or the first object in the list)
    // ...is a human avatar
    bool IsHumanAvatar();

    //
    // switch to / from this camera (if it is a camera)
    //
    void PushCamera(pyKey& avKey);
    void PushCameraCut(pyKey& avKey);
    void PopCamera(pyKey& avKey);
    void PushCutsceneCamera(bool cut, pyKey& avKey);
    void PopCutsceneCamera(pyKey& avKey);

    void Animate();
    
    // return responder state (if responder modifier found)
    int8_t GetResponderState();
    
    // some animation commands for s.o.'s w/ multiple animations attached

    void RewindAnim(const ST::string& animName);
    void PlayAnim(const ST::string& animName);
    void StopAnim(const ST::string& animName);

    void RunResponder(int state);
    void FFResponder(int state);
    
    void SetSoundFilename(int index, const ST::string& filename, bool isCompressed);
    int GetSoundObjectIndex(const ST::string& sndObj);

    // hack for garrison
    void VolumeSensorIgnoreExtraEnters(bool ignore);

    /** More SubWorld hacks */
    void VolumeSensorNoArbitration(bool noArbitration);
};

#endif // _pySceneObject_h_
