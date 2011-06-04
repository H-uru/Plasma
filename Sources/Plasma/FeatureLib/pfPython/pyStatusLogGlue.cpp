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
#include "pyStatusLog.h"
#include "pyEnum.h"

#include "../plStatusLog/plStatusLog.h"
#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptStatusLog, pyStatusLog);

PYTHON_DEFAULT_NEW_DEFINITION(ptStatusLog, pyStatusLog)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptStatusLog)

PYTHON_INIT_DEFINITION(ptStatusLog, args, keywords)
{
	PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION(ptStatusLog, open, args)
{
	char* logName;
	unsigned long numLines, flags;
	if (!PyArg_ParseTuple(args, "sll", &logName, &numLines, &flags))
	{
		PyErr_SetString(PyExc_TypeError, "open expects a string and two unsigned longs");
		PYTHON_RETURN_ERROR;
	}
	PYTHON_RETURN_BOOL(self->fThis->Open(logName, numLines, flags));
}

PYTHON_METHOD_DEFINITION(ptStatusLog, write, args)
{
	char* text;
	PyObject* colorObj = NULL;
	if (!PyArg_ParseTuple(args, "s|O", &text, &colorObj))
	{
		PyErr_SetString(PyExc_TypeError, "write expects a string and an optional ptColor");
		PYTHON_RETURN_ERROR;
	}
	if (colorObj)
	{
		if (!pyColor::Check(colorObj))
		{
			PyErr_SetString(PyExc_TypeError, "write expects a string and an optional ptColor");
			PYTHON_RETURN_ERROR;
		}
		pyColor* color = pyColor::ConvertFrom(colorObj);
		PYTHON_RETURN_BOOL(self->fThis->WriteColor(text, *color));
	}
	PYTHON_RETURN_BOOL(self->fThis->Write(text));
}

PYTHON_BASIC_METHOD_DEFINITION(ptStatusLog, close, Close);

PYTHON_METHOD_DEFINITION_NOARGS(ptStatusLog, isOpen)
{
	PYTHON_RETURN_BOOL(self->fThis->IsOpen());
}

PYTHON_START_METHODS_TABLE(ptStatusLog)
	PYTHON_METHOD(ptStatusLog, open, "Params: logName,numLines,flags\nOpen a status log for writing to\n"
				"'logname' is the name of the log file (example: special.log)\n"
				"'numLines' is the number of lines to display on debug screen\n"
				"'flags' is a PlasmaConstants.PtStatusLogFlags"),
	PYTHON_METHOD(ptStatusLog, write, "Params: text,color=None\nIf the status log is open, write 'text' to log\n"
				"'color' is the display color in debug screen"),
	PYTHON_BASIC_METHOD(ptStatusLog, close, "Close the status log file"),
	PYTHON_METHOD_NOARGS(ptStatusLog, isOpen, "Returns whether the status log is currently opened"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptStatusLog, "A status log class");

// required functions for PyObject interoperability
PYTHON_CLASS_NEW_IMPL(ptStatusLog, pyStatusLog)

PyObject *pyStatusLog::New(plStatusLog* log)
{
	ptStatusLog *newObj = (ptStatusLog*)ptStatusLog_type.tp_new(&ptStatusLog_type, NULL, NULL);
	newObj->fThis->fLog = log;
	newObj->fThis->fICreatedLog = false;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptStatusLog, pyStatusLog)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptStatusLog, pyStatusLog)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyStatusLog::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptStatusLog);
	PYTHON_CLASS_IMPORT_END(m);
}

void pyStatusLog::AddPlasmaConstantsClasses(PyObject *m)
{
	PYTHON_ENUM_START(PtStatusLogFlags);
	PYTHON_ENUM_ELEMENT(PtStatusLogFlags, kFilledBackground,	plStatusLog::kFilledBackground);
	PYTHON_ENUM_ELEMENT(PtStatusLogFlags, kAppendToLast,		plStatusLog::kAppendToLast);
	PYTHON_ENUM_ELEMENT(PtStatusLogFlags, kDontWriteFile,		plStatusLog::kDontWriteFile);
	PYTHON_ENUM_ELEMENT(PtStatusLogFlags, kDeleteForMe,			plStatusLog::kDeleteForMe);
	PYTHON_ENUM_ELEMENT(PtStatusLogFlags, kAlignToTop,			plStatusLog::kAlignToTop);
	PYTHON_ENUM_ELEMENT(PtStatusLogFlags, kDebugOutput,			plStatusLog::kDebugOutput);
	PYTHON_ENUM_ELEMENT(PtStatusLogFlags, kTimestamp,			plStatusLog::kTimestamp);
	PYTHON_ENUM_ELEMENT(PtStatusLogFlags, kStdout,				plStatusLog::kStdout);
	PYTHON_ENUM_ELEMENT(PtStatusLogFlags, kTimeInSeconds,		plStatusLog::kTimeInSeconds);
	PYTHON_ENUM_ELEMENT(PtStatusLogFlags, kTimeAsDouble,		plStatusLog::kTimeAsDouble);
	PYTHON_ENUM_END(m, PtStatusLogFlags);
}