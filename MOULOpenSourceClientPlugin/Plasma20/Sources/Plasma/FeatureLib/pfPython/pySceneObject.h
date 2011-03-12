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
#ifndef _pySceneObject_h_
#define _pySceneObject_h_

//////////////////////////////////////////////////////////////////////
//
// pySceneObject   - a wrapper class to provide interface to modifier
//                   attached to a SceneObject
//
//////////////////////////////////////////////////////////////////////

#include "pyKey.h"
#include "cyDraw.h"
#include "cyPhysics.h"
#include "cyAvatar.h"
#include "cyParticleSys.h"

#include "hsStlUtils.h"

#include <python.h>
#include "pyGlueHelpers.h"


class pySceneObject
{
private:
	hsTArray<plKey>		fSceneObjects;
	plKey				fSenderKey;		// the holder of the who (the modifier) we are
	plKey				fPyMod;			// pyKey that points to modifier
	
	hsBool			fNetForce;

	virtual void IAddObjKeyToAll(plKey key);
	virtual void ISetAllSenderKeys();

protected:
	pySceneObject();
	pySceneObject(pyKey& objkey, pyKey& selfkey);
	pySceneObject(plKey objkey,pyKey& selfkey);
	pySceneObject(plKey objkey);

public:
	~pySceneObject() {Py_XDECREF(fDraw); Py_XDECREF(fPhysics); Py_XDECREF(fAvatar); Py_XDECREF(fParticle);}

	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptSceneobject);
	static PyObject *New(plKey objKey, PyObject *selfKeyObj);
	static PyObject *New(plKey objKey, pyKey &selfKey);
	static PyObject *New(plKey objKey, plKey selfkey);
	static PyObject *New(plKey objKey);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pySceneObject object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pySceneObject); // converts a PyObject to a pySceneObject (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

	// override the equals to operator
	hsBool operator==(const pySceneObject &sobj) const;
	hsBool operator!=(const pySceneObject &sobj) const { return !(sobj == *this);	}

	PyObject*				fDraw; // cyDraw
	PyObject*				fPhysics; // cyPhysics
	PyObject*				fAvatar; // cyAvatar
	PyObject*				fParticle; // cyParticleSys

	// getter and setters
	virtual void addObjKey(plKey key);
	virtual void addObjPyKey(pyKey& objkey);
	virtual plKey getObjKey();
	virtual PyObject* getObjPyKey(); // pyKey

	virtual void setSenderKey(plKey key);
	virtual void setPyMod(pyKey& pymod);
	virtual void setPyMod(plKey& key);

	virtual void SetNetForce(hsBool state);

	virtual PyObject* findObj(const char* name); // pySceneObject

	virtual const char* GetName();
	virtual std::vector<PyObject*> GetResponders(); // pyKey list
	virtual std::vector<PyObject*> GetPythonMods(); // pyKey list
	//
	// deteremine if this object (or the first object in the list)
	// ...is locally owned
	virtual hsBool IsLocallyOwned();
	
	//
	// get the local to world matrix
	virtual PyObject* GetLocalToWorld();

	//
	// get the local to world matrix
	virtual PyObject* GetWorldToLocal();

	//
	// get the local to world matrix
	virtual PyObject* GetLocalToParent();

	//
	// get the local to world matrix
	virtual PyObject* GetParentToLocal();

	//
	// set the local to world matrix
	virtual void SetTransform(pyMatrix44& l2w, pyMatrix44& w2l);

	//
	// find the position of this object (if there are more than one, just the first one)
	virtual PyObject* GetWorldPosition(); // pyPoint3

	//
	// find the view vector for this object (if there are more than one, just the first one)
	virtual PyObject* GetViewVector(); // pyVector3

	//
	// find the up vector for this object (if there are more than one, just the first one)
	virtual PyObject* GetUpVector(); // pyVector3

	//
	// find the up vector for this object (if there are more than one, just the first one)
	virtual PyObject* GetRightVector(); // pyVector3

	//
	// deteremine if this object (or any of the object attached)
	// ...is an avatar, of any type
	virtual hsBool IsAvatar();

	virtual PyObject* GetAvatarVelocity(); // pyVector3

	//
	// deteremine if this object (or the first object in the list)
	// ...is a human avatar
	virtual hsBool IsHumanAvatar();

	//
	// switch to / from this camera (if it is a camera)
	//
	void PushCamera(pyKey& avKey);
	void PushCameraCut(pyKey& avKey);
	void PopCamera(pyKey& avKey);
	void PushCutsceneCamera(hsBool cut,pyKey& avKey);
	void PopCutsceneCamera(pyKey& avKey);

	void Animate();
	
	// return responder state (if responder modifier found)
	Int8 GetResponderState();
	
	// some animation commands for s.o.'s w/ multiple animations attached

	void RewindAnim(const char* animName);
	void PlayAnim(const char* animName);
	void StopAnim(const char* animName);

	void RunResponder(int state);
	void FFResponder(int state);
	
	void SetSoundFilename(int index, const char* filename, bool isCompressed);
	int GetSoundObjectIndex(const char* sndObj);

	// hack for garrison
	void VolumeSensorIgnoreExtraEnters(bool ignore);
};

#endif // _pySceneObject_h_
