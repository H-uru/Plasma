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
#include "cyMisc.h"
#include "pyGlueHelpers.h"
#include "pyKey.h"
#include "pyColor.h"
#include "pyPlayer.h"
#include "pyEnum.h"

// for enums
#include "..\plNetCommon\plNetCommon.h"
#include "..\plResMgr\plLocalization.h"
#include "..\plMessage\plLOSRequestMsg.h"

#include <python.h>

PYTHON_GLOBAL_METHOD_DEFINITION(PtYesNoDialog, args, "Params: selfkey,dialogMessage\nThis will display a Yes/No dialog to the user with the text dialogMessage\n"
			"This dialog _has_ to be answered by the user.\n"
			"And their answer will be returned in a Notify message.")
{
	PyObject* keyObj = NULL;
	PyObject* dialogMsgObj = NULL;
	if (!PyArg_ParseTuple(args, "OO", &keyObj, &dialogMsgObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtYesNoDialog expects a ptKey and a string or unicode string");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtYesNoDialog expects a ptKey and a string or unicode string");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(keyObj);
	if (PyUnicode_Check(dialogMsgObj))
	{
		int len = PyUnicode_GetSize(dialogMsgObj);
		wchar_t* text = TRACKED_NEW wchar_t[len + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)dialogMsgObj, text, len);
		text[len] = L'\0';
		cyMisc::YesNoDialog(*key, text);
		delete [] text;
		PYTHON_RETURN_NONE;
	}
	else if (PyString_Check(dialogMsgObj))
	{
		char* text = PyString_AsString(dialogMsgObj);
		cyMisc::YesNoDialog(*key, text);
		PYTHON_RETURN_NONE;
	}
	PyErr_SetString(PyExc_TypeError, "PtYesNoDialog expects a ptKey and a string or unicode string");
	PYTHON_RETURN_ERROR;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtRateIt, args, "Params: chronicleName,dialogPrompt,onceFlag\nShows a dialog with dialogPrompt and stores user input rating into chronicleName")
{
	char* chronicleName;
	char* dialogPrompt;
	char onceFlag;
	if (!PyArg_ParseTuple(args, "ssb", &chronicleName, &dialogPrompt, &onceFlag))
	{
		PyErr_SetString(PyExc_TypeError, "PtRateIt expects two strings and a boolean");
		PYTHON_RETURN_ERROR;
	}
	cyMisc::RateIt(chronicleName, dialogPrompt, onceFlag != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtExcludeRegionSet, args, "Params: senderKey,regionKey,state\nThis will set the state of an exclude region\n"
			"- 'senderKey' is a ptKey of the PythonFile component\n"
			"- 'regionKey' is a ptKey of the exclude region\n"
			"- 'state' is either kExRegRelease or kExRegClear")
{
	PyObject* senderObj = NULL;
	PyObject* regionObj = NULL;
	unsigned short stateVal;
	if (!PyArg_ParseTuple(args, "OOh", &senderObj, &regionObj, &stateVal))
	{
		PyErr_SetString(PyExc_TypeError, "PtExcludeRegionSet expects two ptKeys and a short");
		PYTHON_RETURN_ERROR;
	}
	if ((!pyKey::Check(senderObj)) || (!pyKey::Check(regionObj)))
	{
		PyErr_SetString(PyExc_TypeError, "PtExcludeRegionSet expects two ptKeys and a short");
		PYTHON_RETURN_ERROR;
	}
	pyKey* sender = pyKey::ConvertFrom(senderObj);
	pyKey* region = pyKey::ConvertFrom(regionObj);
	cyMisc::ExcludeRegionSet(*sender, *region, stateVal);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtExcludeRegionSetNow, args, "Params: senderKey,regionKey,state\nThis will set the state of an exclude region immediately on the server\n"
			"- 'senderKey' is a ptKey of the PythonFile component\n"
			"- 'regionKey' is a ptKey of the exclude region\n"
			"- 'state' is either kExRegRelease or kExRegClear")
{
	PyObject* senderObj = NULL;
	PyObject* regionObj = NULL;
	unsigned short stateVal;
	if (!PyArg_ParseTuple(args, "OOh", &senderObj, &regionObj, &stateVal))
	{
		PyErr_SetString(PyExc_TypeError, "PtExcludeRegionSetNow expects two ptKeys and a short");
		PYTHON_RETURN_ERROR;
	}
	if ((!pyKey::Check(senderObj)) || (!pyKey::Check(regionObj)))
	{
		PyErr_SetString(PyExc_TypeError, "PtExcludeRegionSetNow expects two ptKeys and a short");
		PYTHON_RETURN_ERROR;
	}
	pyKey* sender = pyKey::ConvertFrom(senderObj);
	pyKey* region = pyKey::ConvertFrom(regionObj);
	cyMisc::ExcludeRegionSetNow(*sender, *region, stateVal);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtAcceptInviteInGame, args, "Params: friendName,inviteKey\nSends a VaultTask to the server to perform the invite")
{
	char* friendName;
	char* inviteKey;
	if (!PyArg_ParseTuple(args, "ss", &friendName, &inviteKey))
	{
		PyErr_SetString(PyExc_TypeError, "PtAcceptInviteInGame expects two strings");
		PYTHON_RETURN_ERROR;
	}
	cyMisc::AcceptInviteInGame(friendName, inviteKey);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetTime, "Returns the number of seconds since the game was started.")
{
	return PyFloat_FromDouble(cyMisc::GetSeconds());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetGameTime, "Returns the system game time (frame based) in seconds.")
{
	return PyFloat_FromDouble(cyMisc::GetSysSeconds());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetFrameDeltaTime, "Returns the amount of time that has elapsed since last frame.")
{
	return PyFloat_FromDouble(cyMisc::GetDelSysSeconds());
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtPageInNode, args, "Params: nodeName, ageName=\"\"\nPages in node, or a list of nodes")
{
	PyObject* nodeNameObj = NULL;
	char* ageName = NULL;
	if (!PyArg_ParseTuple(args, "O|s", &nodeNameObj, &ageName))
	{
		PyErr_SetString(PyExc_TypeError, "PtPageInNode expects a string or list of strings, and optionally a string");
		PYTHON_RETURN_ERROR;
	}
	std::vector<std::string> nodeNames;
	if (PyString_Check(nodeNameObj))
	{
		nodeNames.push_back(PyString_AsString(nodeNameObj));
	}
	else if (PyList_Check(nodeNameObj))
	{
		int num = PyList_Size(nodeNameObj);
		for (int i = 0; i < num; i++)
		{
			PyObject* listItem = PyList_GetItem(nodeNameObj, i);
			if (!PyString_Check(listItem))
			{
				PyErr_SetString(PyExc_TypeError, "PtPageInNode expects a string or list of strings, and optionally a string");
				PYTHON_RETURN_ERROR;
			}
			nodeNames.push_back(PyString_AsString(listItem));
		}
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "PtPageInNode expects a string or list of strings, and optionally a string");
		PYTHON_RETURN_ERROR;
	}

	cyMisc::PageInNodes(nodeNames, ageName);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtPageOutNode, args, "Params: nodeName\nPages out a node")
{
	char* nodeName;
	if (!PyArg_ParseTuple(args, "s", &nodeName))
	{
		PyErr_SetString(PyExc_TypeError, "PtPageOutNode expects a string");
		PYTHON_RETURN_ERROR;
	}
	cyMisc::PageOutNode(nodeName);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtLimitAvatarLOD, args, "Params: LODlimit\nSets avatar's LOD limit")
{
	int lodLimit;
	if (!PyArg_ParseTuple(args, "i", &lodLimit))
	{
		PyErr_SetString(PyExc_TypeError, "PtLimitAvatarLOD expects an integer");
		PYTHON_RETURN_ERROR;
	}
	cyMisc::LimitAvatarLOD(lodLimit);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtFogSetDefColor, args, "Params: color\nSets default fog color")
{
	PyObject* colorObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &colorObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtFogSetDefColor expects a ptColor object");
		PYTHON_RETURN_ERROR;
	}
	if (!pyColor::Check(colorObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtFogSetDefColor expects a ptColor object");
		PYTHON_RETURN_ERROR;
	}
	pyColor* color = pyColor::ConvertFrom(colorObj);
	cyMisc::FogSetDefColor(*color);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtFogSetDefLinear, args, "Params: start,end,density\nSet linear fog values")
{
	float start, end, density;
	if (!PyArg_ParseTuple(args, "fff", &start, &end, &density))
	{
		PyErr_SetString(PyExc_TypeError, "PtFogSetDefLinear expects three floats");
		PYTHON_RETURN_ERROR;
	}
	cyMisc::FogSetDefLinear(start, end, density);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtFogSetDefExp, args, "Params: end,density\nSet exp fog values")
{
	float end, density;
	if (!PyArg_ParseTuple(args, "ff", &end, &density))
	{
		PyErr_SetString(PyExc_TypeError, "PtFogSetDefExp expects three floats");
		PYTHON_RETURN_ERROR;
	}
	cyMisc::FogSetDefExp(end, density);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtFogSetDefExp2, args, "Params: end,density\nSet exp2 fog values")
{
	float end, density;
	if (!PyArg_ParseTuple(args, "ff", &end, &density))
	{
		PyErr_SetString(PyExc_TypeError, "PtFogSetDefExp2 expects three floats");
		PYTHON_RETURN_ERROR;
	}
	cyMisc::FogSetDefExp2(end, density);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtLoadDialog, args, "Params: dialogName,selfKey=None,ageName=\"\"\nLoads a GUI dialog by name and optionally set the Notify proc key\n"
			"If the dialog is already loaded then it won't load it again")
{
	char* dialogName;
	PyObject* keyObj = NULL;
	char* ageName = NULL;
	if (!PyArg_ParseTuple(args, "s|Os", &dialogName, &keyObj, &ageName))
	{
		PyErr_SetString(PyExc_TypeError, "PtLoadDialog expects a string, and optionally a ptKey and second string");
		PYTHON_RETURN_ERROR;
	}
	if (keyObj)
	{
		if (!pyKey::Check(keyObj))
		{
			PyErr_SetString(PyExc_TypeError, "PtLoadDialog expects a string, and optionally a ptKey and second string");
			PYTHON_RETURN_ERROR;
		}
		pyKey* key = pyKey::ConvertFrom(keyObj);
		if (ageName)
			cyMisc::LoadDialogKA(dialogName, *key, ageName);
		else
			cyMisc::LoadDialogK(dialogName, *key);
	}
	else
		cyMisc::LoadDialog(dialogName);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtUnloadDialog, args, "Params: dialogName\nThis will unload the GUI dialog by name. If not loaded then nothing will happen")
{
	char* dialogName;
	if (!PyArg_ParseTuple(args, "s", &dialogName))
	{
		PyErr_SetString(PyExc_TypeError, "PtUnloadDialog expects a string");
		PYTHON_RETURN_ERROR;
	}
	cyMisc::UnloadDialog(dialogName);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtIsDialogLoaded, args, "Params: dialogName\nTest to see if a GUI dialog is loaded, by name")
{
	char* dialogName;
	if (!PyArg_ParseTuple(args, "s", &dialogName))
	{
		PyErr_SetString(PyExc_TypeError, "PtIsDialogLoaded expects a string");
		PYTHON_RETURN_ERROR;
	}
	PYTHON_RETURN_BOOL(cyMisc::IsDialogLoaded(dialogName));
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtShowDialog, args, "Params: dialogName\nShow a GUI dialog by name (does not load dialog)")
{
	char* dialogName;
	if (!PyArg_ParseTuple(args, "s", &dialogName))
	{
		PyErr_SetString(PyExc_TypeError, "PtShowDialog expects a string");
		PYTHON_RETURN_ERROR;
	}
	cyMisc::ShowDialog(dialogName);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtHideDialog, args, "Params: dialogName\nHide a GUI dialog by name (does not unload dialog)")
{
	char* dialogName;
	if (!PyArg_ParseTuple(args, "s", &dialogName))
	{
		PyErr_SetString(PyExc_TypeError, "PtHideDialog expects a string");
		PYTHON_RETURN_ERROR;
	}
	cyMisc::HideDialog(dialogName);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtGetDialogFromTagID, args, "Params: tagID\nReturns the dialog associated with the tagID")
{
	unsigned long tagID;
	if (!PyArg_ParseTuple(args, "l", &tagID))
	{
		PyErr_SetString(PyExc_TypeError, "PtGetDialogFromTagID expects a long");
		PYTHON_RETURN_ERROR;
	}
	return cyMisc::GetDialogFromTagID(tagID);
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtGetDialogFromString, args, "Params: dialogName\nGet a ptGUIDialog from its name")
{
	char* dialogName;
	if (!PyArg_ParseTuple(args, "s", &dialogName))
	{
		PyErr_SetString(PyExc_TypeError, "PtHideDialog expects a string");
		PYTHON_RETURN_ERROR;
	}
	return cyMisc::GetDialogFromString(dialogName);
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtIsGUIModal, "Returns true if the GUI is displaying a modal dialog and blocking input")
{
	PYTHON_RETURN_BOOL(cyMisc::IsGUIModal());
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSendPrivateChatList, args, "Params: chatList\nLock the local avatar into private vox messaging, and / or add new members to his chat list")
{
	PyObject* chatListObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &chatListObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtSendPrivateChatList expects a list of ptPlayers");
		PYTHON_RETURN_ERROR;
	}

	std::vector<pyPlayer*> chatList;
	if (PyList_Check(chatListObj))
	{
		int listSize = PyList_Size(chatListObj);
		for (int i = 0; i < listSize; i++)
		{
			PyObject* listItem = PyList_GetItem(chatListObj, i);
			if (!pyPlayer::Check(listItem))
			{
				PyErr_SetString(PyExc_TypeError, "PtSendPrivateChatList expects a list of ptPlayers");
				PYTHON_RETURN_ERROR;
			}
			chatList.push_back(pyPlayer::ConvertFrom(listItem));
		}
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "PtSendPrivateChatList expects a list of ptPlayers");
		PYTHON_RETURN_ERROR;
	}

	cyMisc::SetPrivateChatList(chatList);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtClearPrivateChatList, args, "Params: memberKey\nRemove the local avatar from private vox messaging, and / or clear members from his chat list")
{
	PyObject* keyObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtClearPrivateChatList expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtClearPrivateChatList expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(keyObj);
	cyMisc::ClearPrivateChatList(*key);
	PYTHON_RETURN_NONE;
}

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaMethods - the python method definitions
//

void cyMisc::AddPlasmaMethods2(std::vector<PyMethodDef> &methods)
{
	PYTHON_GLOBAL_METHOD(methods, PtYesNoDialog);
	PYTHON_GLOBAL_METHOD(methods, PtRateIt);
	
	PYTHON_GLOBAL_METHOD(methods, PtExcludeRegionSet);
	PYTHON_GLOBAL_METHOD(methods, PtExcludeRegionSetNow);

	PYTHON_GLOBAL_METHOD(methods, PtAcceptInviteInGame);

	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtGetTime);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtGetGameTime);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtGetFrameDeltaTime);

	PYTHON_GLOBAL_METHOD(methods, PtPageInNode);
	PYTHON_GLOBAL_METHOD(methods, PtPageOutNode);
	
	PYTHON_GLOBAL_METHOD(methods, PtLimitAvatarLOD);

	PYTHON_GLOBAL_METHOD(methods, PtFogSetDefColor);
	PYTHON_GLOBAL_METHOD(methods, PtFogSetDefLinear);
	PYTHON_GLOBAL_METHOD(methods, PtFogSetDefExp);
	PYTHON_GLOBAL_METHOD(methods, PtFogSetDefExp2);

	PYTHON_GLOBAL_METHOD(methods, PtLoadDialog);
	PYTHON_GLOBAL_METHOD(methods, PtUnloadDialog);
	PYTHON_GLOBAL_METHOD(methods, PtIsDialogLoaded);
	PYTHON_GLOBAL_METHOD(methods, PtShowDialog);
	PYTHON_GLOBAL_METHOD(methods, PtHideDialog);
	PYTHON_GLOBAL_METHOD(methods, PtGetDialogFromTagID);
	PYTHON_GLOBAL_METHOD(methods, PtGetDialogFromString);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtIsGUIModal);

	PYTHON_GLOBAL_METHOD(methods, PtSendPrivateChatList);
	PYTHON_GLOBAL_METHOD(methods, PtClearPrivateChatList);
}

void cyMisc::AddPlasmaConstantsClasses(PyObject *m)
{
	PYTHON_ENUM_START(PtCCRPetitionType);
	PYTHON_ENUM_ELEMENT(PtCCRPetitionType, kGeneralHelp,plNetCommon::PetitionTypes::kGeneralHelp);
	PYTHON_ENUM_ELEMENT(PtCCRPetitionType, kBug,		plNetCommon::PetitionTypes::kBug);
	PYTHON_ENUM_ELEMENT(PtCCRPetitionType, kFeedback,	plNetCommon::PetitionTypes::kFeedback);
	PYTHON_ENUM_ELEMENT(PtCCRPetitionType, kExploit,	plNetCommon::PetitionTypes::kExploit);
	PYTHON_ENUM_ELEMENT(PtCCRPetitionType, kHarass,		plNetCommon::PetitionTypes::kHarass);
	PYTHON_ENUM_ELEMENT(PtCCRPetitionType, kStuck,		plNetCommon::PetitionTypes::kStuck);
	PYTHON_ENUM_ELEMENT(PtCCRPetitionType, kTechnical,	plNetCommon::PetitionTypes::kTechnical);
	PYTHON_ENUM_END(m, PtCCRPetitionType);
	
	PYTHON_ENUM_START(PtLanguage);
	PYTHON_ENUM_ELEMENT(PtLanguage, kEnglish,		plLocalization::kEnglish);
	PYTHON_ENUM_ELEMENT(PtLanguage, kFrench,		plLocalization::kFrench);
	PYTHON_ENUM_ELEMENT(PtLanguage, kGerman,		plLocalization::kGerman);
	PYTHON_ENUM_ELEMENT(PtLanguage, kSpanish,		plLocalization::kSpanish);
	PYTHON_ENUM_ELEMENT(PtLanguage, kItalian,		plLocalization::kItalian);
	PYTHON_ENUM_ELEMENT(PtLanguage, kJapanese,		plLocalization::kJapanese);
	PYTHON_ENUM_ELEMENT(PtLanguage, kNumLanguages,	plLocalization::kNumLanguages);
	PYTHON_ENUM_END(m, PtLanguage);
	
	PYTHON_ENUM_START(PtLOSReportType);
	PYTHON_ENUM_ELEMENT(PtLOSReportType, kReportHit,		plLOSRequestMsg::kReportHit);
	PYTHON_ENUM_ELEMENT(PtLOSReportType, kReportMiss,		plLOSRequestMsg::kReportMiss);
	PYTHON_ENUM_ELEMENT(PtLOSReportType, kReportHitOrMiss,	plLOSRequestMsg::kReportHitOrMiss);
	PYTHON_ENUM_END(m, PtLOSReportType);
	
	PYTHON_ENUM_START(PtLOSObjectType);
	PYTHON_ENUM_ELEMENT(PtLOSObjectType, kClickables,		kClickables);
	PYTHON_ENUM_ELEMENT(PtLOSObjectType, kCameraBlockers,	kCameraBlockers);
	PYTHON_ENUM_ELEMENT(PtLOSObjectType, kCustom,			kCustom);
	PYTHON_ENUM_ELEMENT(PtLOSObjectType, kShootable,		kShootable);
	PYTHON_ENUM_END(m, PtLOSObjectType);
}