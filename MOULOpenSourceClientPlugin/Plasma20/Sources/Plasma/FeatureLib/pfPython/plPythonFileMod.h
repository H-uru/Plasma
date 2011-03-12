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
#ifndef _plPythonFileMod_h_
#define _plPythonFileMod_h_

//////////////////////////////////////////////////////////////////////
//
// plPythonFileMod   - the 'special' Python File modifier
//
// This modifier will handle the interface to python code that has been file-ized.
//
//////////////////////////////////////////////////////////////////////

#include "../pnModifier/plMultiModifier.h"
#include "hsGeometry3.h"
#include "hsResMgr.h"

#include <python.h>

#include "plPythonParameter.h"

class PythonVaultCallback;
class plPythonSDLModifier;
class pyKey;
class pfPythonKeyCatcher;
class plKeyEventMsg;
class plPipeline;

class plPythonFileMod	: public plMultiModifier
{
protected:
	friend class plPythonSDLModifier;

	plPythonSDLModifier* fSDLMod;

	hsBool IEval(double secs, hsScalar del, UInt32 dirty);

	void IMakeModuleName(char* modulename,plSceneObject* sobj);

	char*		fPythonFile;
	char*		fModuleName;

	// the list of receivers that want to be notified
	hsTArray<plKey>			fReceivers;

	PyObject*	fSelfKey;
	plPipeline	*fPipe;

	// the list of parameters (attributes)
	hsTArray<plPythonParameter>	fParameters;

	// internal data
	PyObject*	fModule;		// python module object
	PyObject*	fInstance;		// python object that the instance of the class to run
	static hsBool	fAtConvertTime;	// flag for when in convert time within Max, don't run code
	hsBool		fLocalNotify;	// True when This Mod was Notified by a local plNotify
	hsBool		fIsFirstTimeEval;	// flag to determine when the first time at the eval,
								// so the Python coders can hava a chance to run initialization
								// code after the system is up, but before things are displayed
	hsBool		fAmIAttachedToClone;	// is this python file mod attached to a cloned object
	
	// callback class for the KI
	PythonVaultCallback	*fVaultCallback;
	pfPythonKeyCatcher	*fKeyCatcher;

	struct NamedComponent
	{
		char*	name;
		Int32	id;
		bool	isActivator;
	};

	hsTArray<NamedComponent> fNamedCompQueue;

	virtual void IFindResponderAndAdd(const char *responderName, Int32 id);
	virtual void IFindActivatorAndAdd(const char *activatorName, Int32 id);
	void ISetKeyValue(const plKey& key, Int32 id);

	bool ILoadPythonCode();

	enum genref_whats
	{
		kNotSure = 0,
		kAddNotify
	};

public:

	plPythonFileMod();
	~plPythonFileMod();

	CLASSNAME_REGISTER( plPythonFileMod );
	GETINTERFACE_ANY( plPythonFileMod, plMultiModifier );

	plPythonSDLModifier* GetSDLMod() { return fSDLMod; }
	hsBool WasLocalNotify()	{ return fLocalNotify; }
	plPipeline* GetPipeline() { return fPipe; }
	virtual void SetSourceFile(const char* filename);
	virtual int getPythonOutput(std::string* line);
	virtual void ReportError();
	virtual void DisplayPythonOutput();
	static void SetAtConvertTime() { fAtConvertTime=true; }
	virtual hsBool AmIAttachedToClone() { return fAmIAttachedToClone; }

	virtual void AddToNotifyList(plKey pKey) { fReceivers.Append(pKey); }
	virtual Int32 NotifyListCount() { return fReceivers.Count(); }
	virtual plKey GetNotifyListItem(Int32 i) { return fReceivers[i]; }

	virtual void AddParameter(plPythonParameter param) { fParameters.Append(param); }
	virtual Int32 GetParameterListCount() { return fParameters.Count(); }
	virtual plPythonParameter GetParameterItem(Int32 i) { return fParameters[i]; }
	
	virtual void AddTarget(plSceneObject* sobj);
	virtual void RemoveTarget(plSceneObject* so); 

	virtual void EnableControlKeyEvents();
	virtual void DisableControlKeyEvents();
	
	virtual hsBool MsgReceive(plMessage* msg);

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	// this is to keep track of what python functions are available and working,
	// so there is no need to keep trying and banging our head until its bloody
	enum func_num
	{
		kfunc_FirstUpdate = 0,		// these enums _have_ to match the static names in fEventFunctionNames
		kfunc_Update,
		kfunc_Notify,		// OnNotify
		kfunc_AtTimer,
		kfunc_OnKeyEvent,
		kfunc_Load,
		kfunc_Save,
		kfunc_GUINotify,
		kfunc_PageLoad,
		kfunc_ClothingUpdate,
		kfunc_KIMsg,
		kfunc_MemberUpdate,
		kfunc_RemoteAvatarInfo,
		kfunc_RTChat,
		kfunc_VaultEvent,
		kfunc_AvatarPage,
		kfunc_SDLNotify,
		kfunc_OwnershipNotify,
		kfunc_AgeVaultEvent,
		kfunc_Init,
		kfunc_OnCCRMsg,
		kfunc_OnServerInitComplete,
		kfunc_OnVaultNotify,
		kfunc_OnDefaultKeyCaught,
		kfunc_OnMarkerMsg,
		kfunc_OnBackdoorMsg,
		kfunc_OnBehaviorNotify,
		kfunc_OnLOSNotify,
		kfunc_OnBeginAgeLoad,
		kfunc_OnMovieEvent,
		kfunc_OnScreenCaptureDone,
		kfunc_OnClimbBlockerEvent,
		kfunc_OnAvatarSpawn,
		kfunc_OnAccountUpdate,
		kfunc_gotPublicAgeList,
		kfunc_OnGameMgrMsg,
		kfunc_OnGameCliMsg,
		kfunc_OnAIMsg,
		kfunc_lastone
	};
	// array of matching Python instance where the functions are, if defined
	PyObject* fPyFunctionInstances[kfunc_lastone];
	// array of the names of the standard functions that can be called
	static char*	fFunctionNames[];

	// The konstant hard-coded name to be used for all global pythonFileMods
	static char	kGlobalNameKonstant[];

	// API for processing discarded keys as the deafult key catcher
	void	HandleDiscardedKey( plKeyEventMsg *msg );
};

#endif // _plPythonFileMod_h
