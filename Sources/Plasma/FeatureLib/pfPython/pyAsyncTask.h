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

#ifndef _pyAsyncTask_h_
#define _pyAsyncTask_h_

#include "HeadSpin.h"
#include "hsRefCnt.h"

#include <tuple>

#include "pyGlueHelpers.h"

// For some reason, 3ds Max defines a "Yield" macro. What is up with that?
#ifdef Yield
#   undef Yield
#endif

class pyObjectRef;

class pyAsyncTask : public hsRefCnt
{
protected:
    enum
    {
        /** The task's __await__() method has been called. */
        kAwaited = (1 << 0),

        /** The `fResult` field is now valid and ready to be returned. */
        kComplete = (1 << 1),

        /** The task should block on a valid result. */
        kBlocking = (1 << 2),

        /** The task has an exception set. */
        kException = (1 << 3),
    };

    uint32_t fFlags;
    PyObject* fResult;
    PyObject* fException;

public:
    pyAsyncTask()
        : hsRefCnt(),
          fFlags(),
          fResult(),
          fException()
    { }
    pyAsyncTask(const pyAsyncTask& copy) = delete;
    pyAsyncTask(pyAsyncTask&& move) = delete;
    ~pyAsyncTask();

public:
    // required functions for PyObject interoperability
    static PyObject* New(pyAsyncTask* task = nullptr);
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyDniInfoSource object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyAsyncTask); // converts a PyObject to a pyAsyncTask (throws error if not correct type)

    static void AddPlasmaClasses(PyObject* m);
    static void AddPlasmaMethods(PyObject* m);

public:
    /** Clears any exception state. */
    void ClearException();

    /**
     * Gets the excception type set on this task.
     * \return A borrowed reference.
     */
    PyObject* GetException() const { return fException; }

    /**
     * Gets the result of this task.
     * \return A borrowed reference.
     */
    PyObject* GetResult() const
    {
        return fResult;
    }

    bool HasException() const { return hsTestBits(fFlags, kException); }
    bool IsAwaited() const { return hsTestBits(fFlags, kAwaited); }
    bool IsComplete() const { return hsTestBits(fFlags, kComplete); }
    bool IsBlocking() const { return hsTestBits(fFlags, kBlocking); }

    /**
     * Sets the result of the task to an exception.
     * \param exc A borrowed reference to the exception type.
     * \param value A borrowed reference to the exception value.
     * \note The parameters for this function matches the refcounting semantics of
     *       the builtin `PyErr_` function. That is, you do not need to give this
     *       function a new reference.
     */
    void SetException(PyObject* exc, PyObject* value = nullptr);

    /**
     * Sets the result of the task to the current Python exception state.
     * Note that this implicitly *clears* the current Python exception state.
     */
    void SetCurrentException();

    /**
     * Sets the successful result of the task.
     * \param A new reference to the result of the task.
     * \note This function steals the reference to result.
     */
    void SetResult(PyObject* result);

public:
    void SetAwaited() { hsSetBits(fFlags, kAwaited); }
    void SetBlocking() { hsSetBits(fFlags, kBlocking); }

    virtual void IAwait() {}

    /**
     * Evaluate the current status of the Task.
     * This is (generally) called when `plEvalMsg` triggers the plPythonFileMod to
     * pump the top-level script coroutine, which eventually trickles down into
     * the glue object's `tp_iternext`/`__next__()`.
     */
    virtual void IEval() {}

    /**
     * Busy-loop until a result is available.
     */
    void Wait();

public:
    /**
     * Gets the appropriate result and returns control to the caller.
     * \note This may block the main thread.
     * \returns A new reference to the result object.
     */
    static PyObject* Yield(PyObject* task);

    /**
     * Gets the appropriate result and returns control to the caller.
     * \note This may block the main thread.
     * \returns A new reference to the result object.
     */
    static PyObject* Yield(const pyObjectRef& task);

    /**
     * Pumps an arbitrary awaitable.
     * \returns An `std::tuple` of [bool complete, pyObjectRef result].
     */
    static std::tuple<bool, pyObjectRef> Pump(PyObject* iter);

    /**
     * Returns an iterator from the awaitable object `aw` that can be pumped
     * using `pyAsyncTask::Pump()`.
     * \param aw A Python awaitable.
     * \returns A new reference to an iterator.
     */
    static PyObject* GetAsyncIter(PyObject* aw);
};

#endif // _pyAsyncTask_h_
