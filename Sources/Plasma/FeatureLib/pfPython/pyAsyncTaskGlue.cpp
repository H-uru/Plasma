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

#include "hsBitVector.h"
#include "plProfile.h"
#include "hsTimer.h"

#include "pyObjectRef.h"

#include <optional>
#include <string_view>
#include <type_traits>
#include <vector>

// ==========================================================================

plProfile_Extern(PythonAsyncTaskCounter);
plProfile_Extern(PythonAwaitedTaskCounter);

// ==========================================================================

PYTHON_CLASS_DEFINITION(ptAsyncTaskIter, pyAsyncTask);

static PyObject* ptAsyncTaskIter_new(PyTypeObject* type, PyObject*, PyObject*)
{
    plProfile_Inc(PythonAwaitedTaskCounter);
    ptAsyncTaskIter* self = (ptAsyncTaskIter*)type->tp_alloc(type, 0);
    self->fThis = nullptr;
    return (PyObject*)self;
}

static void ptAsyncTaskIter_dealloc(ptAsyncTaskIter* self)
{
    plProfile_Dec(PythonAwaitedTaskCounter);
    hsRefCnt_SafeUnRef(self->fThis);
    Py_TYPE(self)->tp_free(self);
}

PYTHON_NO_INIT_DEFINITION(ptAsyncTaskIter)

PYTHON_RICH_COMPARE_DEFINITION(ptAsyncTaskIter, lhs, rhs, op)
{
    if (!(pyAsyncTask::Check(lhs) && pyAsyncTask::Check(rhs)))
        PYTHON_RETURN_NOT_IMPLEMENTED;

    switch (op) {
    case Py_EQ:
        return PyBool_FromLong(((ptAsyncTaskIter*)lhs)->fThis == ((ptAsyncTaskIter*)rhs)->fThis);
    case Py_NE:
        return PyBool_FromLong(((ptAsyncTaskIter*)lhs)->fThis != ((ptAsyncTaskIter*)rhs)->fThis);
    default:
        PYTHON_RETURN_NOT_IMPLEMENTED;
    }
}

PYTHON_ITERNEXT_DEFINITION(ptAsyncTaskIter)
{
    if (!self->fThis->IsComplete())
        self->fThis->IEval();
    if (self->fThis->HasException()) {
        hsAssert(
            !PyErr_GivenExceptionMatches(self->fThis->GetException(), self->fThis->GetResult()),
            "StopIteration means success, are you sure about this?"
        );
        PyErr_SetObject(self->fThis->GetException(), self->fThis->GetResult());
        PYTHON_RETURN_ERROR;
    }
    if (self->fThis->IsComplete()) {
        PyErr_SetObject(PyExc_StopIteration, self->fThis->GetResult());
        PYTHON_RETURN_ERROR;
    }
    PYTHON_RETURN_NONE;
}

PYTHON_START_METHODS_TABLE(ptAsyncTaskIter)
PYTHON_END_METHODS_TABLE;

// Type structure definition
#define ptAsyncTaskIter_AS_ASYNC       PYTHON_NO_AS_ASYNC
#define ptAsyncTaskIter_AS_NUMBER      PYTHON_NO_AS_NUMBER
#define ptAsyncTaskIter_AS_SEQUENCE    PYTHON_NO_AS_SEQUENCE
#define ptAsyncTaskIter_AS_MAPPING     PYTHON_NO_AS_MAPPING
#define ptAsyncTaskIter_STR            PYTHON_NO_STR
#define ptAsyncTaskIter_GETATTRO       PYTHON_DEFAULT_GETATTRO
#define ptAsyncTaskIter_SETATTRO       PYTHON_DEFAULT_SETATTRO
#define ptAsyncTaskIter_RICH_COMPARE   PYTHON_DEFAULT_RICH_COMPARE(ptAsyncTaskIter)
#define ptAsyncTaskIter_ITER           PYTHON_NO_ITER
#define ptAsyncTaskIter_ITERNEXT       PYTHON_DEFAULT_ITERNEXT(ptAsyncTaskIter)
#define ptAsyncTaskIter_GETSET         PYTHON_NO_GETSET
#define ptAsyncTaskIter_BASE           PYTHON_NO_BASE
PLASMA_CUSTOM_TYPE(ptAsyncTaskIter, "Awaitable implementation for `ptAsyncTask`");

// ==========================================================================

static inline bool IIsAwaitable(PyObject* obj)
{
    // This is a pretty safe indication, no matter what version of Python
    // we are using. Even PyCoro has this slot filled in, we just don't
    // want to use it.
    return Py_TYPE(obj)->tp_as_async && Py_TYPE(obj)->tp_as_async->am_await;
}

class pyWrapperTask : public pyAsyncTask
{
    pyObjectRef fAwaitable;
    pyObjectRef fIter;

public:
    pyWrapperTask(pyObjectRef aw)
        : fAwaitable(std::move(aw))
    {
    }

    void IAwait() override
    {
        fIter = GetAsyncIter(fAwaitable.Get());
        if (!fIter)
            SetCurrentException();
    }

    void IEval() override
    {
        auto [complete, result] = Pump(fIter.Get());
        if (complete && !result) {
            SetCurrentException();
        } else if (complete) {
            SetResult(result.Release());
        }
    }
};

// ==========================================================================

// glue functions
PYTHON_CLASS_DEFINITION(ptAsyncTask, pyAsyncTask);

static PyObject* ptAsyncTask_new(PyTypeObject* type, PyObject*, PyObject*)
{
    plProfile_Inc(PythonAsyncTaskCounter);
    ptAsyncTask* self = (ptAsyncTask*)type->tp_alloc(type, 0);
    self->fThis = nullptr;
    return (PyObject*)self;
}

static void ptAsyncTask_dealloc(ptAsyncTask* self)
{
    plProfile_Dec(PythonAsyncTaskCounter);
    hsRefCnt_SafeUnRef(self->fThis);
    Py_TYPE(self)->tp_free(self);
}

PYTHON_INIT_DEFINITION(ptAsyncTask, args, keywords)
{
    constexpr std::string_view kTypeError = "ptAsyncTask.__init__() expects an optional awatiable object.";
    PyObject* argObj = nullptr;
    if (!PyArg_ParseTuple(args, "O", &argObj)) {
        PyErr_SetString(PyExc_TypeError, kTypeError.data());
        PYTHON_RETURN_INIT_ERROR;
    }
    if (argObj && !IIsAwaitable(argObj)) {
        PyErr_SetString(PyExc_TypeError, kTypeError.data());
        PYTHON_RETURN_INIT_ERROR;
    }
    if (argObj)
        self->fThis = new pyWrapperTask({ argObj, pyObjectNewRef });
    else
        self->fThis = new pyAsyncTask();
    self->fThis->Ref(); // ref count starts at 0, so this is required.
    PYTHON_RETURN_INIT_OK;
}

static PyObject* ptAsyncTask_await(ptAsyncTask* self)
{
    // This isn't a native coroutine object nor are there multiple loops,
    // double awaiting something is just weird, not really an error that
    // can cause the world to end.
    if (!self->fThis->IsAwaited()) {
        self->fThis->SetAwaited();
        self->fThis->IAwait();
    }
    if (self->fThis->HasException()) {
        PyErr_SetObject(self->fThis->GetException(), self->fThis->GetResult());
        PYTHON_RETURN_ERROR;
    }

    // Use another object as the iterator to prevent weird semantics like being able to
    // pump the task from outside the await point by calling `next(task)`. Note that
    // because ptAsyncTaskIter uses the same glue class as ptAsyncTask, we have to
    // call `tp_new` directly.
    ptAsyncTaskIter* iter = (ptAsyncTaskIter*)ptAsyncTaskIter_new(&ptAsyncTaskIter_type, nullptr, nullptr);
    iter->fThis = self->fThis;
    iter->fThis->Ref();
    return (PyObject*)iter;
}

PYTHON_START_AS_ASYNC_TABLE(ptAsyncTask)
    (unaryfunc)ptAsyncTask_await, /* am_await */
    nullptr,                      /* am_aiter */
    nullptr,                      /* am_anext */
PYTHON_END_AS_ASYNC_TABLE;

PYTHON_METHOD_DEFINITION_WKEY(ptAsyncTask, getResult, args, keywords)
{
    constexpr std::string_view kTypeError = "getResult() expects an optional bool";
    const char* kwdlist[] = { "block", nullptr };
    bool block = false;
    if (!PyArg_ParseTupleAndKeywords(args, keywords, "|$b", const_cast<char**>(kwdlist), &block)) {
        PyErr_SetString(PyExc_TypeError, kTypeError.data());
        PYTHON_RETURN_ERROR;
    }

    // Note that calling `.getResult(block=True)` doesn't make this an inherently blocking
    // task. The caller just wants the result before continuing. A subtle difference, but
    // it's why I'm not setting the block flag.
    if (block)
        self->fThis->Wait();
    if (self->fThis->HasException()) {
        PyErr_SetObject(self->fThis->GetException(), self->fThis->GetResult());
        PYTHON_RETURN_ERROR;
    }
    if (self->fThis->IsComplete()) {
        PyObject* result = self->fThis->GetResult();
        Py_INCREF(result);
        return result;
    }

    PyErr_SetString(PyExc_RuntimeError, "Task has not completed.");
    PYTHON_RETURN_ERROR;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAsyncTask, hasException)
{
    return PyBool_FromLong(self->fThis->HasException());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAsyncTask, isDone)
{
    return PyBool_FromLong(self->fThis->IsComplete() || self->fThis->HasException());
}

PYTHON_METHOD_DEFINITION(ptAsyncTask, setResult, args)
{
    PyObject* result;
    if (!PyArg_ParseTuple(args, "O", &result)) {
        PyErr_SetString(PyExc_TypeError, "setResult expects an object.");
        PYTHON_RETURN_ERROR;
    }

    Py_INCREF(result);
    self->fThis->SetResult(result);
    PYTHON_RETURN_NONE;
}

PYTHON_START_METHODS_TABLE(ptAsyncTask)
    PYTHON_METHOD_WKEY(ptAsyncTask, getResult, "Returns the result of this task. "
                                               "Raises `RuntimeError` if the task "
                                               "is not complete when non-blocking."),
    PYTHON_METHOD(ptAsyncTask, hasException, "Gets if the task has an exception set."),
    PYTHON_METHOD(ptAsyncTask, isDone, "Gets if the task is done (has a valid result or exception set)."),
    PYTHON_METHOD(ptAsyncTask, setResult, "Params: result\nSets the result of this task. "
                                          "NOTE: This will override and discard any result "
                                          "the engine would otherwise report."),
PYTHON_END_METHODS_TABLE;

// Type structure definition
#define ptAsyncTask_AS_ASYNC       PYTHON_DEFAULT_AS_ASYNC(ptAsyncTask)
#define ptAsyncTask_AS_NUMBER      PYTHON_NO_AS_NUMBER
#define ptAsyncTask_AS_SEQUENCE    PYTHON_NO_AS_SEQUENCE
#define ptAsyncTask_AS_MAPPING     PYTHON_NO_AS_MAPPING
#define ptAsyncTask_STR            PYTHON_NO_STR
#define ptAsyncTask_GETATTRO       PYTHON_DEFAULT_GETATTRO
#define ptAsyncTask_SETATTRO       PYTHON_DEFAULT_SETATTRO
#define ptAsyncTask_RICH_COMPARE   PYTHON_NO_RICH_COMPARE
#define ptAsyncTask_ITER           PYTHON_NO_ITER
#define ptAsyncTask_ITERNEXT       PYTHON_NO_ITERNEXT
#define ptAsyncTask_GETSET         PYTHON_NO_GETSET
#define ptAsyncTask_BASE           PYTHON_NO_BASE
PLASMA_CUSTOM_TYPE(ptAsyncTask, "An asynchronously completed task");

// required functions for PyObject interoperability
PyObject* pyAsyncTask::New(pyAsyncTask* task)
{
    ptAsyncTask* newObj = (ptAsyncTask*)ptAsyncTask_type.tp_new(&ptAsyncTask_type, nullptr, nullptr);
    newObj->fThis = task ? task : new pyAsyncTask();
    newObj->fThis->Ref(); // starting ref count is zero, so reffing is always needed.
    return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptAsyncTask, pyAsyncTask)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptAsyncTask, pyAsyncTask)

// ==========================================================================

class pySleepTask : public pyAsyncTask
{
    float fSleepTime;
    float fStartTime;

public:
    pySleepTask(float timeSec)
        : pyAsyncTask(),
          fSleepTime(timeSec),
          fStartTime()
    {
    }

    void IAwait() override
    {
        fStartTime = hsTimer::GetSeconds<float>();
    }

    void IEval() override
    {
        if (fStartTime + fSleepTime < hsTimer::GetSeconds<float>())
            SetResult(nullptr);
    }
};

PYTHON_GLOBAL_METHOD_DEFINITION_WKEY(PtSleep, args, keywords,
    "Params: time, block\n"
    "Delay script execution for an arbitrary amount of time, in seconds"
)
{
    constexpr std::string_view kTypeError = "PtSleep requires a number and an optional boolean.";
    const char* kwdlist[] = { "", "block", nullptr };
    PyObject* timeNumberObj;
    bool block = false;

    if (!PyArg_ParseTupleAndKeywords(args, keywords, "O|$b", const_cast<char**>(kwdlist), &timeNumberObj, &block)) {
        PyErr_SetString(PyExc_TypeError, kTypeError.data());
        PYTHON_RETURN_ERROR;
    }
    if (!PyNumber_Check(timeNumberObj)) {
        PyErr_SetString(PyExc_TypeError, kTypeError.data());
        PYTHON_RETURN_ERROR;
    }

    // We're accepting a PyNumber for the time so that `PtSleep(5)` and `PtSleep(5.0)` BOTH work.
    pyObjectRef timeFloatObj = PyNumber_Float(timeNumberObj);
    if (!timeFloatObj) {
        PyErr_SetNone(PyExc_ValueError);
        PYTHON_RETURN_ERROR;
    }

    float time = (float)PyFloat_AsDouble(timeFloatObj.Get());
    pyObjectRef taskObj = pyAsyncTask::New(new pySleepTask(time));
    if (block)
        ((ptAsyncTask*)taskObj.Get())->fThis->SetBlocking();
    return pyAsyncTask::Yield(taskObj);
}

// ==========================================================================

class pyWaitTask : public pyAsyncTask
{
public:
    struct Task
    {
        pyObjectRef fAwaitable;
        pyObjectRef fIter;

        Task(pyObjectRef aw, pyObjectRef iter = {})
            : fAwaitable(std::move(aw)),
              fIter(std::move(aw))
        {
        }
    };

private:
    std::vector<Task> fAwaitables;
    hsBitVector fCompletions;
    std::optional<float> fTimeout;
    float fStartTime;

    /**
     * Counts how many awaitables are either complete or pending.
     * \returns A tuple of (done, pending).
     */
    std::tuple<size_t, size_t> ICountProgress() const
    {
        size_t done = 0, pending = 0;
        // hsBitIterator es muy extraño. Entonces, no lo usamos.
        for (size_t i = 0; i < fAwaitables.size(); ++i) {
            if (fCompletions.IsBitSet(i))
                ++done;
            else
                ++pending;
        }
        return std::make_tuple(done, pending);
    }

public:
    pyWaitTask(std::vector<Task> aws, std::optional<float> timeout)
        : pyAsyncTask(),
          fAwaitables(std::move(aws)),
          fTimeout(std::move(timeout)),
          fStartTime()
    {
        fCompletions.SetSize(fAwaitables.size());
    }

    void IAwait() override
    {
        // Everything in fAwaitables is an object implementing `tp_as_async->am_await`.
        // Call all of the `__await__()` methods now that our `__await__()` method has
        // been called.
        for (auto& aw : fAwaitables) {
            aw.fIter = GetAsyncIter(aw.fAwaitable.Get());
            if (!aw.fIter) {
                SetCurrentException();
                return;
            }
        }

        fStartTime = hsTimer::GetSeconds<float>();
    }

    void IEval() override
    {
        // Pump all the incomplete iterators.
        for (size_t i = 0; i < fAwaitables.size(); ++i) {
            Task& aw = fAwaitables[i];
            if (fCompletions.IsBitSet(i))
                continue;

            auto [complete, result] = Pump(aw.fIter.Get());
            if (complete && !result) {
                // An actual fatal error occurred.
                SetCurrentException();
                return;
            } else if (complete) {
                fCompletions.SetBit(i);
            }
        }

        // If this fires, we either timed out or completed everything.
        // We'll return the same thing as `asyncio.wait()`: a tuple of tasks (done, pending)
        bool timeout = fTimeout && fStartTime + fTimeout.value() < hsTimer::GetSeconds<float>();
        auto [doneCount, pendingCount] = ICountProgress();
        bool test = fCompletions.IsBitSet(0);
        if (timeout || doneCount == fAwaitables.size()) {
            PyObject* doneObj = PyTuple_New(doneCount);
            PyObject* pendingObj = PyTuple_New(pendingCount);
            // This used to return the actual result, but I've changed it to return the task
            // instead. I'm keeping the result around for now.
            for (size_t awIdx = 0, doneIdx = 0, pendingIdx = 0; awIdx < fAwaitables.size(); ++awIdx) {
                PyObject* resultTup = fCompletions.IsBitSet(awIdx) ? doneObj : pendingObj;
                size_t& tupIdx = fCompletions.IsBitSet(awIdx) ? doneIdx : pendingIdx;
                pyObjectRef taskObj = fAwaitables[awIdx].fAwaitable; // intentional copy
                PyTuple_SET_ITEM(resultTup, tupIdx, taskObj.Release());
                ++tupIdx;
            }

            PyObject* resultObj = PyTuple_New(2);
            PyTuple_SET_ITEM(resultObj, 0, doneObj);
            PyTuple_SET_ITEM(resultObj, 1, pendingObj);
            SetResult(resultObj);
        }
    }
};

PYTHON_GLOBAL_METHOD_DEFINITION_WKEY(PtWaitFor, args, keywords,
    "Params: aws, /, timeout, block\n"
    "Wait on the awaitables in the iterable aws to complete."
)
{
    constexpr std::string_view kTypeError = "PtWaitFor requires an awaitable or iterable of awaitables, an optional number, and an optional boolean.";
    const char* kwdlist[] = { "", "timeout", "block", nullptr };
    PyObject* awsObj;
    PyObject* timeoutNumberObj = nullptr;
    bool block = false;

    if (!PyArg_ParseTupleAndKeywords(args, keywords, "O|$Ob", const_cast<char**>(kwdlist), &awsObj, &timeoutNumberObj, &block)) {
        PyErr_SetString(PyExc_TypeError, kTypeError.data());
        PYTHON_RETURN_ERROR;
    }

    std::vector<pyWaitTask::Task> aws;
    if (IIsAwaitable(awsObj)) {
        // Convenice feature to allow (a probably blocking) wait on a single awaitable.
        aws.emplace_back(pyObjectRef(awsObj, pyObjectNewRef));
    } else {
        pyObjectRef awsObjIter = PyObject_GetIter(awsObj);
        if (!awsObj) {
            PyErr_SetString(PyExc_TypeError, kTypeError.data());
            PYTHON_RETURN_ERROR;
        }
        while (pyObjectRef item = PyIter_Next(awsObjIter.Get())) {
            if (!IIsAwaitable(item.Get())) {
                PyErr_SetString(PyExc_TypeError, kTypeError.data());
                PYTHON_RETURN_ERROR;
            }
            aws.emplace_back(std::move(item));
        }
    }

    std::optional<float> timeout;
    if (timeoutNumberObj) {
        pyObjectRef timeoutFloatObj = PyNumber_Float(timeoutNumberObj);
        if (!timeoutFloatObj) {
            PyErr_SetNone(PyExc_ValueError);
            PYTHON_RETURN_ERROR;
        }
        timeout = (float)PyFloat_AsDouble(timeoutFloatObj.Get());
    }

    pyObjectRef taskObj = pyAsyncTask::New(new pyWaitTask(std::move(aws), std::move(timeout)));
    if (block)
        ((ptAsyncTask*)taskObj.Get())->fThis->SetBlocking();
    return pyAsyncTask::Yield(taskObj);
}

// ==========================================================================

PyObject* pyAsyncTask::Yield(PyObject* taskObj)
{
    hsAssert(pyAsyncTask::Check(taskObj), "Attempted to yield a non-task.");
    ptAsyncTask* task = (ptAsyncTask*)taskObj;
    if (task->fThis->IsBlocking()) {
        task->fThis->Wait(); // blocks for response
        if (task->fThis->HasException()) {
            PyErr_SetObject(task->fThis->GetException(), task->fThis->GetResult());
            PYTHON_RETURN_ERROR;
        }

        Py_INCREF(task->fThis->GetResult());
        return task->fThis->GetResult();
    }

    // The task is NOT blocking, so return ourselves. We implement the async protocol,
    // so someone will need to `await` us either in Python code or in C++.
    Py_INCREF(taskObj);
    return taskObj;
}

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyAsyncTask::AddPlasmaClasses(PyObject* m)
{
    PYTHON_CLASS_IMPORT_START(m);
    PYTHON_CLASS_IMPORT(m, ptAsyncTask);
    PYTHON_CLASS_IMPORT(m, ptAsyncTaskIter);
    PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaMethods - the python method definitions
//
void pyAsyncTask::AddPlasmaMethods(PyObject* m)
{
    PYTHON_START_GLOBAL_METHOD_TABLE(pyAsyncTask)
        PYTHON_GLOBAL_METHOD_WKEY(PtSleep)
        PYTHON_GLOBAL_METHOD_WKEY(PtWaitFor)
    PYTHON_END_GLOBAL_METHOD_TABLE(m, pyAsyncTask);
}
