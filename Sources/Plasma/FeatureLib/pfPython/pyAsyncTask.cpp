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

#include "pyAsyncTask.h"

#include "plgDispatch.h"

#include "pyObjectRef.h"
#include "plPythonCallable.h"

#include "plNetGameLib/plNetGameLib.h"

#include <thread>

// ==========================================================================

pyAsyncTask::~pyAsyncTask()
{
    // If no one has waited on us, that's clearly some kind of problem,
    // so raise a warning about it.
    if (!hsTestBits(fFlags, kAwaited)) {
        PyErr_WarnEx(PyExc_RuntimeWarning, "ptAsyncTask was never awaited.", 1);
        PyErr_WriteUnraisable(nullptr);
    }

    Py_XDECREF(fResult);
}

// ==========================================================================

void pyAsyncTask::ClearException()
{
    // The result field is both "exception value" and "the real result", so only
    // clear if we think an exception was actually set. We don't want to
    // accidentally blow away the result.
    if (hsTestBits(fFlags, kException)) {
        hsClearBits(fFlags, kException);
        Py_CLEAR(fException);
        Py_CLEAR(fResult);
    }
}

void pyAsyncTask::SetException(PyObject* exc, PyObject* value)
{
    hsSetBits(fFlags, kException);
    PyObject* oldExc = fException;
    fException = exc;
    Py_XINCREF(fException);
    Py_XDECREF(oldExc);

    PyObject* oldResult = fResult;
    fResult = value ? value : Py_None;
    Py_INCREF(fResult);
    Py_XDECREF(oldResult);
}

void pyAsyncTask::SetCurrentException()
{
    PyObject* excType, * excValue, * traceback;
    PyErr_Fetch(&excType, &excValue, &traceback);
    PyErr_NormalizeException(&excType, &excValue, &traceback);
    SetException(excType, excValue);
    Py_XDECREF(excType);
    Py_XDECREF(excValue);
    Py_XDECREF(traceback);
}

void pyAsyncTask::SetResult(PyObject* result)
{
    if (!hsTestBits(fFlags, kException) && !hsTestBits(fFlags, kComplete)) {
        hsSetBits(fFlags, kComplete);
        PyObject* oldResult = fResult;
        if (result != nullptr) {
            fResult = result;
        } else {
            Py_INCREF(Py_None);
            fResult = Py_None;
        }
        Py_XDECREF(oldResult);
    } else {
        // Someone else has already set the result or failed the task with an
        // exception, so discard this result.
        Py_XDECREF(result);
    }
}

// ==========================================================================

void pyAsyncTask::Wait()
{
    if (!IsAwaited()) {
        IAwait();
        SetAwaited();
    }

    // The first iteration is block free.
    if (!HasException())
        IEval();

    // We were asked to block until the result is available, so do that.
    // This means that we return the actual result and no one is `await`ing us.
    while (!IsComplete() && !HasException()) {
        NetClientUpdate();
        plgDispatch::Dispatch()->MsgQueueProcess();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        IEval();
    }
}

PyObject* pyAsyncTask::Yield(const pyObjectRef& task)
{
    return Yield(task.Get());
}

std::tuple<bool, pyObjectRef> pyAsyncTask::Pump(PyObject* iter)
{
#if PY_VERSION_HEX >= 0x030A0000
    // Python 3.10 makes this really easy; just call PyIter_Send() on whatever it is.
    PyObject* result;
    switch (PyIter_Send(iter, Py_None, &result)) {
    case PYGEN_RETURN:
        return std::make_tuple(true, result);
    case PYGEN_NEXT:
        return std::make_tuple(false, result);
    case PYGEN_ERROR:
        // result should be null...
        return std::make_tuple(true, nullptr);
    DEFAULT_FATAL(PyIter_Send);
    }
#else
    // A lazy reimplementation of PyIter_Send(), keeping in mind that
    // tp_as_async->am_send was added in 3.10, and we'd like to avoid
    // doing too much expensive work here.
    pyObjectRef result;
    if (Py_TYPE(iter)->tp_iternext) {
        result = Py_TYPE(iter)->tp_iternext(iter);
    } else if (PyCoro_CheckExact(iter) || PyGen_Check(iter)) {
        pyObjectRef sendFunc = PyObject_GetAttrString(iter, "send");
        hsAssert(sendFunc, "Coroutine doesn't have a send method?");
        result = plPythonCallable::CallObject(sendFunc, nullptr);
    } else {
        hsAssert(0, "Can't pump unknown async type.");
        return std::make_tuple(true, nullptr);
    }
    if (PyErr_Occurred()) {
        if (PyErr_ExceptionMatches(PyExc_StopIteration)) {
            PyObject* excType, * excValue, * excTraceback;
            PyErr_Fetch(&excType, &excValue, &excTraceback);
            PyErr_NormalizeException(&excType, &excValue, &excTraceback);
            if (excValue)
                result = excValue;
            else
                result = { Py_None, pyObjectNewRef };
            Py_XDECREF(excType);
            Py_XDECREF(excTraceback);
        }
        return std::make_tuple(true, std::move(result));
    }

    // Didn't finish
    return std::make_tuple(false, std::move(result));
#endif // PY_VERSION_HEX >= 0x030A0000
}

PyObject* pyAsyncTask::GetAsyncIter(PyObject* aw)
{
#if PY_VERSION_HEX >= 0x030A0000
    if (Py_TYPE(aw)->tp_as_async && Py_TYPE(aw)->tp_as_async->am_send) {
#else
    if (PyCoro_CheckExact(aw) || PyGen_CheckExact(aw)) {
#endif // PY_VERSION_HEX >= 0x030A0000
        Py_INCREF(aw);
        return aw;
    } else if (Py_TYPE(aw)->tp_as_async && Py_TYPE(aw)->tp_as_async->am_await) {
        PyObject* iter = Py_TYPE(aw)->tp_as_async->am_await(aw);
        return iter;
    }
    return nullptr;
}
