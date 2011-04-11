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
#include "pyAudioControl.h"

#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptAudioControl, pyAudioControl);

PYTHON_DEFAULT_NEW_DEFINITION(ptAudioControl, pyAudioControl)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptAudioControl)

PYTHON_INIT_DEFINITION(ptAudioControl, args, keywords)
{
	PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION(ptAudioControl, setSoundFXVolume, args)
{
	float volume;
	if (!PyArg_ParseTuple(args, "f", &volume))
	{
		PyErr_SetString(PyExc_TypeError, "setSoundFXVolume expects a float");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetSoundFXVolume(volume);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAudioControl, setMusicVolume, args)
{
	float volume;
	if (!PyArg_ParseTuple(args, "f", &volume))
	{
		PyErr_SetString(PyExc_TypeError, "setMusicVolume expects a float");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetMusicVolume(volume);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAudioControl, setVoiceVolume, args)
{
	float volume;
	if (!PyArg_ParseTuple(args, "f", &volume))
	{
		PyErr_SetString(PyExc_TypeError, "setVoiceVolume expects a float");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetVoiceVolume(volume);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAudioControl, setAmbienceVolume, args)
{
	float volume;
	if (!PyArg_ParseTuple(args, "f", &volume))
	{
		PyErr_SetString(PyExc_TypeError, "setAmbienceVolume expects a float");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetAmbienceVolume(volume);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAudioControl, setGUIVolume, args)
{
	float volume;
	if (!PyArg_ParseTuple(args, "f", &volume))
	{
		PyErr_SetString(PyExc_TypeError, "setGUIVolume expects a float");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetGUIVolume(volume);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAudioControl, setNPCVoiceVolume, args)
{
	float volume;
	if (!PyArg_ParseTuple(args, "f", &volume))
	{
		PyErr_SetString(PyExc_TypeError, "setNPCVoiceVolume expects a float");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetNPCVoiceVolume(volume);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAudioControl, getSoundFXVolume)
{
	return PyFloat_FromDouble(self->fThis->GetSoundFXVolume());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAudioControl, getMusicVolume)
{
	return PyFloat_FromDouble(self->fThis->GetMusicVolume());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAudioControl, getVoiceVolume)
{
	return PyFloat_FromDouble(self->fThis->GetVoiceVolume());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAudioControl, getAmbienceVolume)
{
	return PyFloat_FromDouble(self->fThis->GetAmbienceVolume());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAudioControl, getGUIVolume)
{
	return PyFloat_FromDouble(self->fThis->GetGUIVolume());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAudioControl, getNPCVoiceVolume)
{
	return PyFloat_FromDouble(self->fThis->GetNPCVoiceVolume());
}


PYTHON_BASIC_METHOD_DEFINITION(ptAudioControl, enable, Enable)
PYTHON_BASIC_METHOD_DEFINITION(ptAudioControl, disable, Disable)

PYTHON_METHOD_DEFINITION_NOARGS(ptAudioControl, isEnabled)
{
	PYTHON_RETURN_BOOL(self->fThis->IsEnabled());
}

PYTHON_METHOD_DEFINITION(ptAudioControl, setLoadOnDemand, args)
{
	char stateFlag;
	if (!PyArg_ParseTuple(args, "b", &stateFlag))
	{
		PyErr_SetString(PyExc_TypeError, "setLoadOnDemand expects a boolean");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetLoadOnDemand(stateFlag != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAudioControl, setTwoStageLOD, args)
{
	char stateFlag;
	if (!PyArg_ParseTuple(args, "b", &stateFlag))
	{
		PyErr_SetString(PyExc_TypeError, "setTwoStageLOD expects a boolean");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetTwoStageLOD(stateFlag != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAudioControl, useHardwareAcceleration, args)
{
	char stateFlag;
	if (!PyArg_ParseTuple(args, "b", &stateFlag))
	{
		PyErr_SetString(PyExc_TypeError, "useHardwareAcceleration expects a boolean");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->UseHardwareAcceleration(stateFlag != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAudioControl, isHardwareAccelerated)
{
	PYTHON_RETURN_BOOL(self->fThis->IsHardwareAccelerated());
}

PYTHON_METHOD_DEFINITION(ptAudioControl, useEAXAcceleration, args)
{
	char stateFlag;
	if (!PyArg_ParseTuple(args, "b", &stateFlag))
	{
		PyErr_SetString(PyExc_TypeError, "useEAXAcceleration expects a boolean");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->UseEAXAcceleration(stateFlag != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAudioControl, isUsingEAXAcceleration)
{
	PYTHON_RETURN_BOOL(self->fThis->IsUsingEAXAcceleration());
}

PYTHON_METHOD_DEFINITION(ptAudioControl, supportsEAX, args)
{
	char *deviceName;
	if (!PyArg_ParseTuple(args, "s", &deviceName))
	{
		PyErr_SetString(PyExc_TypeError, "supportsEAX expects a string");
		PYTHON_RETURN_ERROR;
	}
	PYTHON_RETURN_BOOL(self->fThis->SupportEAX(deviceName));
}

PYTHON_BASIC_METHOD_DEFINITION(ptAudioControl, muteAll, MuteAll)
PYTHON_BASIC_METHOD_DEFINITION(ptAudioControl, unmuteAll, UnmuteAll)

PYTHON_METHOD_DEFINITION_NOARGS(ptAudioControl, isMuted)
{
	PYTHON_RETURN_BOOL(self->fThis->IsMuted());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAudioControl, canSetMicLevel)
{
	PYTHON_RETURN_BOOL(self->fThis->CanSetMicLevel());
}

PYTHON_METHOD_DEFINITION(ptAudioControl, setMicLevel, args)
{
	float volume;
	if (!PyArg_ParseTuple(args, "f", &volume))
	{
		PyErr_SetString(PyExc_TypeError, "setMicLevel expects a float");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetMicLevel(volume);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAudioControl, getMicLevel)
{
	return PyFloat_FromDouble(self->fThis->GetMicLevel());
}

PYTHON_METHOD_DEFINITION(ptAudioControl, enableVoiceRecording, args)
{
	char stateFlag;
	if (!PyArg_ParseTuple(args, "b", &stateFlag))
	{
		PyErr_SetString(PyExc_TypeError, "enableVoiceRecording expects a boolean");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->EnableVoiceRecording(stateFlag != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAudioControl, enableVoiceChat, args)
{
	char enable;
	if (!PyArg_ParseTuple(args, "b", &enable))
	{
		PyErr_SetString(PyExc_TypeError, "enableVoiceChat expects a boolean");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->EnableVoiceChat(enable != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAudioControl, isVoiceRecordingEnabled)
{
	PYTHON_RETURN_BOOL(self->fThis->IsVoiceRecordingEnabled());
}

PYTHON_METHOD_DEFINITION(ptAudioControl, enableVoiceCompression, args)
{
	char stateFlag;
	if (!PyArg_ParseTuple(args, "b", &stateFlag))
	{
		PyErr_SetString(PyExc_TypeError, "enableVoiceCompression expects a boolean");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->EnableVoiceCompression(stateFlag != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAudioControl, isVoiceCompressionEnabled)
{
	PYTHON_RETURN_BOOL(self->fThis->IsVoiceCompressionEnabled());
}

PYTHON_METHOD_DEFINITION(ptAudioControl, enableVoiceNetBroadcast, args)
{
	char stateFlag;
	if (!PyArg_ParseTuple(args, "b", &stateFlag))
	{
		PyErr_SetString(PyExc_TypeError, "enableVoiceNetBroadcast expects a boolean");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->EnableVoiceNetBroadcast(stateFlag != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAudioControl, isVoiceNetBroadcastEnabled)
{
	PYTHON_RETURN_BOOL(self->fThis->IsVoiceNetBroadcastEnabled());
}

PYTHON_BASIC_METHOD_DEFINITION(ptAudioControl, showIcons, ShowIcons)
PYTHON_BASIC_METHOD_DEFINITION(ptAudioControl, hideIcons, HideIcons)

PYTHON_METHOD_DEFINITION(ptAudioControl, pushToTalk, args)
{
	char stateFlag;
	if (!PyArg_ParseTuple(args, "b", &stateFlag))
	{
		PyErr_SetString(PyExc_TypeError, "pushToTalk expects a boolean");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->PushToTalk(stateFlag != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAudioControl, squelchLevel, args)
{
	float volume;
	if (!PyArg_ParseTuple(args, "f", &volume))
	{
		PyErr_SetString(PyExc_TypeError, "squelchLevel expects a float");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SquelchLevel(volume);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAudioControl, recordFrame, args)
{
	long frameSize;
	if (!PyArg_ParseTuple(args, "l", &frameSize))
	{
		PyErr_SetString(PyExc_TypeError, "recordFrame expects a long");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->RecordFrame(frameSize);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAudioControl, recordSampleRate, args)
{
	long sampleRate;
	if (!PyArg_ParseTuple(args, "l", &sampleRate))
	{
		PyErr_SetString(PyExc_TypeError, "recordSampleRate expects a long");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->RecordSampleRate(sampleRate);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAudioControl, getPriorityCutoff)
{
	return PyInt_FromLong(self->fThis->GetPriorityCutoff());
}

PYTHON_METHOD_DEFINITION(ptAudioControl, setPriorityCutoff, args)
{
	unsigned char cutoff;
	if (!PyArg_ParseTuple(args, "b", &cutoff))
	{
		PyErr_SetString(PyExc_TypeError, "setPriorityCutoff expects an unsigned 8-bit int");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetPriorityCutoff(cutoff);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAudioControl, setMode, args)
{
	int mode;
	if (!PyArg_ParseTuple(args, "i", &mode))
	{
		PyErr_SetString(PyExc_TypeError, "setMode expects an integer");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetAudioSystemMode(mode);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAudioControl, getMode)
{
	return PyInt_FromLong((long)self->fThis->GetAudioSystemMode());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAudioControl, getHighestMode)
{
	return PyInt_FromLong((long)self->fThis->GetHighestAudioMode());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAudioControl, getNumAudioDevices)
{
	return PyInt_FromLong(self->fThis->GetNumAudioDevices());
}

PYTHON_METHOD_DEFINITION(ptAudioControl, getAudioDeviceName, args)
{
	int index;
	if (!PyArg_ParseTuple(args, "i", &index))
	{
		PyErr_SetString(PyExc_TypeError, "getAudioDeviceName expects an int");
		PYTHON_RETURN_ERROR;
	}
	return PyString_FromString(self->fThis->GetAudioDeviceName(index));
}

PYTHON_METHOD_DEFINITION(ptAudioControl, setDeviceName, args)
{
	char *devicename = NULL;
	int restart;
	if (!PyArg_ParseTuple(args, "si", &devicename, &restart))
	{
		PyErr_SetString(PyExc_TypeError, "setDeviceName expects a string and a bool");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetDeviceName(devicename, restart != 0);
	PYTHON_RETURN_NONE;
}


PYTHON_METHOD_DEFINITION_NOARGS(ptAudioControl, getDeviceName)
{
	return PyString_FromString(self->fThis->GetDeviceName());
}

PYTHON_START_METHODS_TABLE(ptAudioControl)
	PYTHON_METHOD(ptAudioControl, setSoundFXVolume, "Params: volume\nSets the SoundFX volume (0.0 to 1.0) for the game.\n"
				"This only sets the volume for this game session."),
	PYTHON_METHOD(ptAudioControl, setMusicVolume, "Params: volume\nSets the Music volume (0.0 to 1.0) for the game.\n"
				"This only sets the volume for this game session."),
	PYTHON_METHOD(ptAudioControl, setVoiceVolume, "Params: volume\nSets the Voice volume (0.0 to 1.0) for the game.\n"
				"This only sets the volume for this game session."),
	PYTHON_METHOD(ptAudioControl, setAmbienceVolume, "Params: volume\nSets the Ambience volume (0.0 to 1.0) for the game.\n"
				"This only sets the volume for this game session."),
	PYTHON_METHOD(ptAudioControl, setGUIVolume, "Params: volume\nSets the GUI dialog volume (0.0 to 1.0) for the game.\n"
				"This only sets the volume for this game session."),
	PYTHON_METHOD(ptAudioControl, setNPCVoiceVolume, "Params: volume\nSets the NPC's voice volume (0.0 to 1.0) for the game.\n"
				"This only sets the volume for this game session."),
	PYTHON_METHOD_NOARGS(ptAudioControl, getSoundFXVolume, "Returns the volume (0.0 to 1.0) for the Sound FX."),
	PYTHON_METHOD_NOARGS(ptAudioControl, getMusicVolume, "Returns the volume (0.0 to 1.0) for the Music."),
	PYTHON_METHOD_NOARGS(ptAudioControl, getVoiceVolume, "Returns the volume (0.0 to 1.0) for the Voices."),
	PYTHON_METHOD_NOARGS(ptAudioControl, getAmbienceVolume, "Returns the volume (0.0 to 1.0) for the Ambiance."),
	PYTHON_METHOD_NOARGS(ptAudioControl, getGUIVolume, "Returns the volume (0.0 to 1.0) for the GUI dialogs."),
	PYTHON_METHOD_NOARGS(ptAudioControl, getNPCVoiceVolume, "Returns the volume (0.0 to 1.0) for the NPC's voice."),
	PYTHON_BASIC_METHOD(ptAudioControl, enable, "Enables audio"),
	PYTHON_BASIC_METHOD(ptAudioControl, disable, "Disabled audio"),
	PYTHON_METHOD_NOARGS(ptAudioControl, isEnabled, "Is the audio enabled? Returns 1 if true otherwise returns 0."),
	PYTHON_METHOD(ptAudioControl, setLoadOnDemand, "Params: state\nEnables or disables the load on demand for sounds."),
	PYTHON_METHOD(ptAudioControl, setTwoStageLOD, "Params: state\nEnables or disables two-stage LOD, where sounds can be loaded into RAM but not into sound buffers.\n"
				"...Less of a performance hit, harder on memory."),
	PYTHON_METHOD(ptAudioControl, useHardwareAcceleration, "Params: state\nEnables or disables audio hardware acceleration."),
	PYTHON_METHOD_NOARGS(ptAudioControl, isHardwareAccelerated, "Is audio hardware acceleration enabled? Returns 1 if true otherwise returns 0."),
	PYTHON_METHOD(ptAudioControl, useEAXAcceleration, "Params: state\nEnables or disables EAX sound acceleration (requires hardware acceleration)."),
	PYTHON_METHOD_NOARGS(ptAudioControl, isUsingEAXAcceleration, "Is EAX sound acceleration enabled? Returns 1 if true otherwise returns 0."),
	PYTHON_BASIC_METHOD(ptAudioControl, muteAll, "Mutes all sounds."),
	PYTHON_BASIC_METHOD(ptAudioControl, unmuteAll, "Unmutes all sounds."),
	PYTHON_METHOD_NOARGS(ptAudioControl, isMuted, "Are all sounds muted? Returns 1 if true otherwise returns 0."),
	PYTHON_METHOD_NOARGS(ptAudioControl, canSetMicLevel, "Can the microphone level be set? Returns 1 if true otherwise returns 0."),
	PYTHON_METHOD(ptAudioControl, setMicLevel, "Params: level\nSets the microphone recording level (0.0 to 1.0)."),
	PYTHON_METHOD_NOARGS(ptAudioControl, getMicLevel, "Returns the microphone recording level (0.0 to 1.0)."),
	PYTHON_METHOD(ptAudioControl, enableVoiceRecording, "Params: state\nEnables or disables voice recording."),
	PYTHON_METHOD_NOARGS(ptAudioControl, isVoiceRecordingEnabled, "Is voice recording enabled? Returns 1 if true otherwise returns 0."),
	PYTHON_METHOD(ptAudioControl, enableVoiceCompression, "Params: state\nEnables or disables voice compression."),
	PYTHON_METHOD_NOARGS(ptAudioControl, isVoiceCompressionEnabled, "Is voice compression enabled? Returns 1 if true otherwise returns 0."),
	PYTHON_METHOD(ptAudioControl, enableVoiceNetBroadcast, "Params: state\nEnables or disables voice over network broadcast."),
	PYTHON_METHOD_NOARGS(ptAudioControl, isVoiceNetBroadcastEnabled, "Is voice over net enabled? Returns 1 if true otherwise returns 0."),
	PYTHON_BASIC_METHOD(ptAudioControl, showIcons, "Shows (enables) the voice recording icons."),
	PYTHON_BASIC_METHOD(ptAudioControl, hideIcons, "Hides (disables) the voice recording icons."),
	PYTHON_METHOD(ptAudioControl, pushToTalk, "Params: state\nEnables or disables 'push-to-talk'."),
	PYTHON_METHOD(ptAudioControl, squelchLevel, "Params: level\nSets the squelch level."),
	PYTHON_METHOD(ptAudioControl, recordFrame, "Params: size\nSets the voice packet frame size."),
	PYTHON_METHOD(ptAudioControl, recordSampleRate, "Params: sampleRate\nSets the recording sample rate."),
	PYTHON_METHOD_NOARGS(ptAudioControl, getPriorityCutoff, "Returns current sound priority"),
	PYTHON_METHOD(ptAudioControl, setPriorityCutoff, "Params: priority\nSets the sound priority"),
	PYTHON_METHOD(ptAudioControl, setMode, "Params: mode\nSets the audio system mode"),
	PYTHON_METHOD_NOARGS(ptAudioControl, getMode, "Gets the audio system mode"),
	PYTHON_METHOD_NOARGS(ptAudioControl, getHighestMode, "Gets the highest possible audio system mode"),
	PYTHON_METHOD_NOARGS(ptAudioControl, getNumAudioDevices, "Returns the number of available audio devices."),
	PYTHON_METHOD(ptAudioControl, getAudioDeviceName, "Params: index\nGets the name of audio device for the given index"),
	PYTHON_METHOD(ptAudioControl, setDeviceName, "Params: devicename,restart\nSets the device name for the audio system, and optionally restarts it"),
	PYTHON_METHOD(ptAudioControl, getDeviceName, "Gets the name for the device being used by the audio system"),
	PYTHON_METHOD(ptAudioControl, supportsEAX, "Returns true or false based on whether or not a the device specified supports EAX"),
	PYTHON_METHOD(ptAudioControl, enableVoiceChat, "Params: state\nEnables or disables voice chat."),

PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptAudioControl, "Accessor class to the Audio controls");

// required functions for PyObject interoperability
PYTHON_CLASS_NEW_IMPL(ptAudioControl, pyAudioControl)

PYTHON_CLASS_CHECK_IMPL(ptAudioControl, pyAudioControl)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptAudioControl, pyAudioControl)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyAudioControl::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptAudioControl);
	PYTHON_CLASS_IMPORT_END(m);
}