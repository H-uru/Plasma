# /*==LICENSE==*
#
# CyanWorlds.com Engine - MMOG client, server and tools
# Copyright (C) 2011  Cyan Worlds, Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Additional permissions under GNU GPL version 3 section 7
#
# If you modify this Program, or any covered work, by linking or
# combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
# NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
# JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
# (or a modified version of those libraries),
# containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
# PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
# JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
# licensors of this Program grant you additional
# permission to convey the resulting work. Corresponding Source for a
# non-source form of such a combination shall include the source code for
# the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
# work.
#
# You can contact Cyan Worlds, Inc. by email legal@cyan.com
#  or by snail mail at:
#       Cyan Worlds, Inc.
#       14617 N Newport Hwy
#       Mead, WA   99021
#
# *==LICENSE==*/

# glue code in python for the glue code in C++
# SPECIAL WARNING(1): This glue code returns the attributes in reverse ID order!

from Plasma import PtDebugPrint
from PlasmaTypes import (
    ptAttribNamedActivator,
    ptAttribNamedResponder,
    ptAttribute,
    ptAttributeList,
    ptModifier,
    ptMultiModifier,
)

# The C++ side of the glue code sets the following attribute
# on the Python modifier module before passing it to the Python glue functions:
# * glue_name - name of the modifier module, as configured in the plPythonFileMod

# The modifier module itself can also define the following attribute
# to control the behavior of the glue functions:
# * glue_verbose - enables error prints in glue functions (defaults to False)

# The glue functions add the following attributes to the modifier module:
# * glue_cl - the class of the modifier
# * glue_inst - instance of the modifier class
# * glue_params - parameters dictionary: mapped id to instance
# * glue_paramKeys - this is the parameter ID list, that should be sorted

def _isVerbose(module):
    return getattr(module, "glue_verbose", False)

def getClass(module):
    try:
        return module.glue_cl
    except AttributeError:
        try:
            cl = getattr(module, module.glue_name)
            if issubclass(cl, ptModifier):
                module.glue_cl = cl
                return cl
            else:
                if _isVerbose(module):
                    PtDebugPrint(f"Class {cl.__name__} is not derived from modifier")
                return None
        except:
            if _isVerbose(module):
                try:
                    PtDebugPrint(f"Could not find class {module.glue_name}")
                except NameError:
                    PtDebugPrint("Filename/classname not set!")
            return None

def getInstance(module):
    try:
        return module.glue_inst
    except AttributeError:
        cl = getClass(module)
        if cl is not None:
            module.glue_inst = cl()
            return module.glue_inst

        return None

def delInstance(module):
    try:
        del module.glue_inst
    except AttributeError:
        pass

    try:
        del module.glue_cl
    except AttributeError:
        pass

    try:
        del module.glue_params
    except AttributeError:
        pass

    try:
        del module.glue_paramKeys
    except AttributeError:
        pass

def getVersion(module):
    inst = getInstance(module)
    ver = inst.version
    delInstance(module)
    return ver

def findAndAddAttribs(obj, params, *, verbose):
    if isinstance(obj, ptAttribute):
        if obj.id in params:
            if verbose:
                PtDebugPrint("WARNING: Duplicate attribute ids!")
                PtDebugPrint(f"{obj.name} has id {obj.id} which is already defined in {params[obj.id].name}")
        else:
            params[obj.id] = obj
    elif isinstance(obj, list):
        for o in obj:
            findAndAddAttribs(o, params, verbose=verbose)
    elif isinstance(obj, dict):
        for o in obj.values():
            findAndAddAttribs(o, params, verbose=verbose)
    elif isinstance(obj, tuple):
        for o in obj:
            findAndAddAttribs(o, params, verbose=verbose)

def getParamDict(module):
    try:
        return module.glue_params
    except AttributeError:
        params = {}
        for obj in module.__dict__.values():
            findAndAddAttribs(obj, params, verbose=_isVerbose(module))

        # rebuild the parameter sorted key list
        paramKeys = list(params.keys())
        paramKeys.sort()
        paramKeys.reverse() # reserve the order because PlasmaMax will ask for them in reverse order

        module.glue_params = params
        module.glue_paramKeys = paramKeys
        return params

def getClassName(module):
    cl = getClass(module)
    if cl is not None:
        return cl.__name__

    if _isVerbose(module):
        PtDebugPrint(f"Class not found in {module.glue_name}.py")

    return None

def getBlockID(module):
    inst = getInstance(module)
    if inst is not None:
        return inst.id

    if _isVerbose(module):
        PtDebugPrint(f"Instance could not be created in {module.glue_name}.py")

    return None

def getNumParams(module):
    pd = getParamDict(module)
    if pd is not None:
        return len(pd)

    if _isVerbose(module):
        PtDebugPrint(f"No attributes found in {module.glue_name}.py")

    return 0

def getParam(module, number):
    pd = getParamDict(module)
    if pd is not None:
        # see if there is a paramKey list
        if isinstance(module.glue_paramKeys, list):
            if number in range(len(module.glue_paramKeys)):
                return pd[module.glue_paramKeys[number]].getdef()
            else:
                PtDebugPrint(f"PlasmaGlue.getParam: Error! {number} out of range of attribute list")
        else:
            pl = list(pd.values())
            if number in range(len(pl)):
                return pl[number].getdef()
            else:
                if _isVerbose(module):
                    PtDebugPrint(f"PlasmaGlue.getParam: Error! {number} out of range of attribute list")

    if _isVerbose(module):
        PtDebugPrint("GLUE: Attribute list error")

    return None

def setParam(module, id, value):
    pd = getParamDict(module)
    if pd is not None:
        if id in pd:
            # first try to set the attribute via function call (if there is one)
            try:
                pd[id].__setvalue__(value)
            except AttributeError:
                if isinstance(pd[id], ptAttributeList):
                    try:
                        if not isinstance(pd[id].value, list):
                            pd[id].value = [] # make sure that the value starts as an empty list
                    except AttributeError:
                        pd[id].value = [] # or if value hasn't been defined yet, then do it now

                    pd[id].value.append(value) # add in new value to list
                else:
                    pd[id].value = value
        else:
            if _isVerbose(module):
                PtDebugPrint("PlasmaGlue.setParam: can't find id=", id)
    else:
        PtDebugPrint("PlasmaGlue.setParam: Something terribly has gone wrong. Head for the cover.")

def isNamedAttribute(module, id):
    pd = getParamDict(module)
    if pd is not None:
        try:
            if isinstance(pd[id], ptAttribNamedActivator):
                return 1
            if isinstance(pd[id], ptAttribNamedResponder):
                return 2
        except KeyError:
            if _isVerbose(module):
                PtDebugPrint(f"Could not find id={id} attribute")

    return 0

def isMultiModifier(module):
    inst = getInstance(module)
    return isinstance(inst, ptMultiModifier)

def getVisInfo(module, number):
    pd = getParamDict(module)
    if pd is not None:
        # see if there is a paramKey list
        if isinstance(module.glue_paramKeys, list):
            if number in range(len(module.glue_paramKeys)):
                return pd[module.glue_paramKeys[number]].getVisInfo()
            else:
                PtDebugPrint(f"PlasmaGlue.getVisInfo: Error! {number} out of range of attribute list")
        else:
            pl = list(pd.values())
            if number in range(len(pl)):
                return pl[number].getVisInfo()
            else:
                if _isVerbose(module):
                    PtDebugPrint(f"PlasmaGlue.getVisInfo: Error! {number} out of range of attribute list")

    if _isVerbose(module):
        PtDebugPrint("GLUE: Attribute list error")

    return None
