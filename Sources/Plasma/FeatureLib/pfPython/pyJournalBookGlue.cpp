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

#include "pyJournalBook.h"

#include "pfJournalBook/pfJournalBook.h"

#include "pyEnum.h"
#include "pyGlueHelpers.h"
#include "pyImage.h"
#include "pyKey.h"

// glue functions
PYTHON_CLASS_DEFINITION(ptBook, pyJournalBook);

PYTHON_DEFAULT_NEW_DEFINITION(ptBook, pyJournalBook)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptBook)

PYTHON_INIT_DEFINITION(ptBook, args, keywords)
{
    const char* kwlist[] = {"esHTMLSource", "coverImage", "callbackKey", "guiName", nullptr};
    ST::string source;
    PyObject* coverObj = nullptr;
    PyObject* callbackObj = nullptr;
    ST::string guiName;
    if (!PyArg_ParseTupleAndKeywords(args, keywords, "O&|OOO&", const_cast<char **>(kwlist),
                                     PyUnicode_STStringConverter, &source, &coverObj, &callbackObj, PyUnicode_STStringConverter, &guiName))
    {
        PyErr_SetString(PyExc_TypeError, "__init__ expects a string and optionally a ptImage, ptKey, and string");
        PYTHON_RETURN_INIT_ERROR;
    }

    // convert all the optional arguments
    plKey coverKey;
    if (coverObj)
    {
        if (pyKey::Check(coverObj))
        {
            // this is really the callback key
            if (callbackObj) // callbackObj was already defined, can't have two keys
            {
                PyErr_SetString(PyExc_TypeError, "__init__ expects a string and optionally a ptImage, ptKey, and string");
                PYTHON_RETURN_INIT_ERROR;
            }
            callbackObj = coverObj;
            coverObj = nullptr;
        }
        else if (!pyImage::Check(coverObj))
        {
            PyErr_SetString(PyExc_TypeError, "__init__ expects a string and optionally a ptImage, ptKey, and string");
            PYTHON_RETURN_INIT_ERROR;
        }
        else
            coverKey = pyImage::ConvertFrom(coverObj)->GetKey();
    }

    plKey callbackKey;
    if (callbackObj)
    {
        if (!pyKey::Check(callbackObj))
        {
            PyErr_SetString(PyExc_TypeError, "__init__ expects a string and optionally a ptImage, ptKey, and string");
            PYTHON_RETURN_INIT_ERROR;
        }
        callbackKey = pyKey::ConvertFrom(callbackObj)->getKey();
    }

    self->fThis->MakeBook(source, coverKey, callbackKey, guiName);
    PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION(ptBook, show, args)
{
    char startOpened;
    if (!PyArg_ParseTuple(args, "b", &startOpened))
    {
        PyErr_SetString(PyExc_TypeError, "show expects a boolean");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->Show(startOpened != 0);
    PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptBook, hide, Hide)

PYTHON_METHOD_DEFINITION(ptBook, open, args)
{
    unsigned long startingPage;
    if (!PyArg_ParseTuple(args, "l", &startingPage))
    {
        PyErr_SetString(PyExc_TypeError, "open expects an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->Open(startingPage);
    PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptBook, close, Close)
PYTHON_BASIC_METHOD_DEFINITION(ptBook, closeAndHide, CloseAndHide)

PYTHON_BASIC_METHOD_DEFINITION(ptBook, nextPage, NextPage)
PYTHON_BASIC_METHOD_DEFINITION(ptBook, previousPage, PreviousPage)

PYTHON_METHOD_DEFINITION(ptBook, goToPage, args)
{
    unsigned long page;
    if (!PyArg_ParseTuple(args, "l", &page))
    {
        PyErr_SetString(PyExc_TypeError, "goToPage expects an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->GoToPage(page);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptBook, setSize, args)
{
    float width, height;
    if (!PyArg_ParseTuple(args, "ff", &width, &height))
    {
        PyErr_SetString(PyExc_TypeError, "setSize expects two floats");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetSize(width, height);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptBook, getCurrentPage)
{
    return PyLong_FromUnsignedLong(self->fThis->GetCurrentPage());
}

PYTHON_METHOD_DEFINITION(ptBook, allowPageTurning, args)
{
    char allow;
    if (!PyArg_ParseTuple(args, "b", &allow))
    {
        PyErr_SetString(PyExc_TypeError, "allowPageTurning expects a boolean");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->AllowPageTurning(allow != 0);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptBook, setPageMargin, args)
{
    unsigned long margin;
    if (!PyArg_ParseTuple(args, "l", &margin))
    {
        PyErr_SetString(PyExc_TypeError, "setPageMargin expects an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetPageMargin(margin);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptBook, setGUI, args)
{
    ST::string guiName;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &guiName))
    {
        PyErr_SetString(PyExc_TypeError, "setGUI expects a string");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetGUI(guiName);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptBook, getMovie, args)
{
    unsigned char index;
    if (!PyArg_ParseTuple(args, "b", &index))
    {
        PyErr_SetString(PyExc_TypeError, "getMovie expects a unsigned 8-bit int");
        PYTHON_RETURN_ERROR;
    }
    return self->fThis->GetMovie(index);
}

PYTHON_METHOD_DEFINITION(ptBook, setEditable, args)
{
    char editable;
    if (!PyArg_ParseTuple(args, "b", &editable))
    {
        PyErr_SetString(PyExc_TypeError, "setEditable expects a boolean");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetEditable(editable != 0);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptBook, getEditableText, args)
{
    return PyUnicode_FromSTString(self->fThis->GetEditableText());
}

PYTHON_METHOD_DEFINITION(ptBook, setEditableText, args)
{
    ST::string text;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &text))
    {
        PyErr_SetString(PyExc_TypeError, "setEditableText expects a string");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetEditableText(text);
    PYTHON_RETURN_NONE;
}

PYTHON_START_METHODS_TABLE(ptBook)
    PYTHON_METHOD(ptBook, show, "Params: startOpened\nShows the book closed, or open if the the startOpened flag is true"),
    PYTHON_BASIC_METHOD(ptBook, hide, "Hides the book"),
    PYTHON_METHOD(ptBook, open, "Params: startingPage\nOpens the book to the specified page"),
    PYTHON_BASIC_METHOD(ptBook, close, "Closes the book"),
    PYTHON_BASIC_METHOD(ptBook, closeAndHide, "Closes the book and hides it once it finishes animating"),
    PYTHON_BASIC_METHOD(ptBook, nextPage, "Flips the book to the next page"),
    PYTHON_BASIC_METHOD(ptBook, previousPage, "Flips the book to the previous page"),
    PYTHON_METHOD(ptBook, goToPage, "Params: page\nFlips the book to the specified page"),
    PYTHON_METHOD(ptBook, setSize, "Params: width,height\nSets the size of the book (width and height are floats from 0 to 1)"),
    PYTHON_METHOD_NOARGS(ptBook, getCurrentPage, "Returns the currently shown page"),
    PYTHON_METHOD(ptBook, allowPageTurning, "Params: allow\nTurns on and off the ability to flip the pages in a book"),
    PYTHON_METHOD(ptBook, setPageMargin, "Params: margin\nSets the text margin for the book"),
    PYTHON_METHOD(ptBook, setGUI, "Params: guiName\nSets the gui to be used by the book, if the requested gui is not loaded, it will use the default\nDo not call while the book is open!"),
    PYTHON_METHOD(ptBook, getMovie, "Params: index\nGrabs a ptAnimation object representing the movie indexed by index. The index is the index of the movie in the source code"),
    PYTHON_METHOD(ptBook, setEditable, "Params: editable\nTurn book editing on or off. If the book GUI does not support editing, nothing will happen"),
    PYTHON_METHOD(ptBook, getEditableText, "Returns the editable text currently contained in the book."),
    PYTHON_METHOD(ptBook, setEditableText, "Params: text\nSets the book's editable text."),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptBook, "Params: esHTMLSource,coverImage=None,callbackKey=None,guiName=''\nCreates a new book");

// required functions for PyObject interoperability
PyObject *pyJournalBook::New(const ST::string& htmlSource, plKey coverImageKey /* = {} */, plKey callbackKey /* = {} */, const ST::string &guiName /* = "" */)
{
    ptBook *newObj = (ptBook*)ptBook_type.tp_new(&ptBook_type, nullptr, nullptr);
    newObj->fThis->MakeBook(htmlSource, std::move(coverImageKey), std::move(callbackKey), guiName);
    return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptBook, pyJournalBook)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptBook, pyJournalBook)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyJournalBook::AddPlasmaClasses(PyObject *m)
{
    PYTHON_CLASS_IMPORT_START(m);
    PYTHON_CLASS_IMPORT(m, ptBook);
    PYTHON_CLASS_IMPORT_END(m);
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtLoadBookGUI, args, "Params: guiName\nLoads the gui specified, a gui must be loaded before it can be used. If the gui is already loaded, doesn't do anything")
{
    ST::string guiName;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &guiName))
    {
        PyErr_SetString(PyExc_TypeError, "PtLoadBookGUI expects a string");
        PYTHON_RETURN_ERROR;
    }
    pyJournalBook::LoadGUI(guiName);
    PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtUnloadBookGUI, args, "Params: guiName\nUnloads the gui specified. If the gui isn't loaded, doesn't do anything")
{
    ST::string guiName;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &guiName))
    {
        PyErr_SetString(PyExc_TypeError, "PtUnloadBookGUI expects a string");
        PYTHON_RETURN_ERROR;
    }
    pyJournalBook::UnloadGUI(guiName);
    PYTHON_RETURN_NONE;
}

PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtUnloadAllBookGUIs, pyJournalBook::UnloadAllGUIs, "Unloads all loaded guis except for the default one")

void pyJournalBook::AddPlasmaMethods(PyObject* m)
{
    PYTHON_START_GLOBAL_METHOD_TABLE(ptJournalBook)
        PYTHON_GLOBAL_METHOD(PtLoadBookGUI)
        PYTHON_GLOBAL_METHOD(PtUnloadBookGUI)
        PYTHON_BASIC_GLOBAL_METHOD(PtUnloadAllBookGUIs)
    PYTHON_END_GLOBAL_METHOD_TABLE(m, ptJournalBook)
}

void pyJournalBook::AddPlasmaConstantsClasses(PyObject *m)
{
    PYTHON_ENUM_START(PtBookEventTypes)
    PYTHON_ENUM_ELEMENT(PtBookEventTypes, kNotifyImageLink,         pfJournalBook::kNotifyImageLink)
    PYTHON_ENUM_ELEMENT(PtBookEventTypes, kNotifyShow,              pfJournalBook::kNotifyShow)
    PYTHON_ENUM_ELEMENT(PtBookEventTypes, kNotifyHide,              pfJournalBook::kNotifyHide)
    PYTHON_ENUM_ELEMENT(PtBookEventTypes, kNotifyNextPage,          pfJournalBook::kNotifyNextPage)
    PYTHON_ENUM_ELEMENT(PtBookEventTypes, kNotifyPreviousPage,      pfJournalBook::kNotifyPreviousPage)
    PYTHON_ENUM_ELEMENT(PtBookEventTypes, kNotifyCheckUnchecked,    pfJournalBook::kNotifyCheckUnchecked)
    PYTHON_ENUM_ELEMENT(PtBookEventTypes, kNotifyClose,             pfJournalBook::kNotifyClose)
    PYTHON_ENUM_END(m, PtBookEventTypes)
}
