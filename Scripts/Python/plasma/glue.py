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
# This assumes that this will be loaded into the module that we are trying to do
# with an execfile('.\\python\\system\\glue.py') at the end of the module (after everything is defined)
# SPECIAL WARNING(1): This glue code returns the attributes in reverse ID order!

glue_cl = None       # the class of the modifier
glue_inst = None     # instance of the class modifier
glue_params = None   # parameters dictionary: mapped id to instance
glue_paramKeys = None # this is the parameter ID list, that should be sorted

try:
    glue_verbose
except NameError:
    glue_verbose = 0

def glue_getClass():
    global glue_cl

    if glue_cl is None:
        try:
            cl = globals()[glue_name]
            if issubclass(cl, ptModifier):
                glue_cl = cl
            else:
                if glue_verbose:
                    PtDebugPrint(f"Class {cl.__name__} is not derived from modifier")
        except:
            if glue_verbose:
                try:
                    PtDebugPrint(f"Could not find class {glue_name}")
                except NameError:
                    PtDebugPrint("Filename/classname not set!")

    return glue_cl

def glue_getInst():
    global glue_inst

    if glue_inst is None:
        cl = glue_getClass()
        if cl is not None:
            glue_inst = cl()

    return glue_inst

def glue_delInst():
    global glue_inst
    global glue_cl
    global glue_params
    global glue_paramKeys

    if glue_inst is not None:
        del glue_inst

    # remove our references
    glue_cl = None
    glue_params = None
    glue_paramKeys = None

def glue_getVersion():
    inst = glue_getInst()
    ver = inst.version
    glue_delInst()
    return ver

def glue_findAndAddAttribs(obj, glue_params):
    if isinstance(obj, ptAttribute):
        if obj.id in glue_params:
            if glue_verbose:
                PtDebugPrint("WARNING: Duplicate attribute ids!")
                PtDebugPrint(f"{obj.name} has id {obj.id} which is already defined in {glue_params[obj.id].name}")
        else:
            glue_params[obj.id] = obj
    elif isinstance(obj, list):
        for o in obj:
            glue_findAndAddAttribs(o, glue_params)
    elif isinstance(obj, dict):
        for o in obj.values():
            glue_findAndAddAttribs(o, glue_params)
    elif isinstance(obj, tuple):
        for o in obj:
            glue_findAndAddAttribs(o, glue_params)

def glue_getParamDict():
    global glue_params
    global glue_paramKeys

    if glue_params is None:
        glue_params = {}
        gd = globals()
        for obj in gd.values():
            glue_findAndAddAttribs(obj, glue_params)
        # rebuild the parameter sorted key list
        glue_paramKeys = list(glue_params.keys())
        glue_paramKeys.sort()
        glue_paramKeys.reverse() # reserve the order because PlasmaMax will ask for them in reverse order

    return glue_params

def glue_getClassName():
    cl = glue_getClass()
    if cl is not None:
        return cl.__name__

    if glue_verbose:
        PtDebugPrint(f"Class not found in {glue_name}.py")

    return None

def glue_getBlockID():
    inst = glue_getInst()
    if inst is not None:
        return inst.id

    if glue_verbose:
        PtDebugPrint(f"Instance could not be created in {glue_name}.py")

    return None

def glue_getNumParams():
    pd = glue_getParamDict()
    if pd is not None:
        return len(pd)

    if glue_verbose:
        PtDebugPrint(f"No attributes found in {glue_name}.py")

    return 0

def glue_getParam(number):
    global glue_paramKeys

    pd = glue_getParamDict()
    if pd is not None:
        # see if there is a paramKey list
        if isinstance(glue_paramKeys, list):
            if number in range(len(glue_paramKeys)):
                return pd[glue_paramKeys[number]].getdef()
            else:
                PtDebugPrint(f"glue_getParam: Error! {number} out of range of attribute list")
        else:
            pl = list(pd.values())
            if number in range(len(pl)):
                return pl[number].getdef()
            else:
                if glue_verbose:
                    PtDebugPrint(f"glue_getParam: Error! {number} out of range of attribute list")

    if glue_verbose:
        PtDebugPrint("GLUE: Attribute list error")

    return None

def glue_setParam(id, value):
    pd = glue_getParamDict()
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
            if glue_verbose:
                PtDebugPrint("setParam: can't find id=", id)
    else:
        PtDebugPrint("setParma: Something terribly has gone wrong. Head for the cover.")

def glue_isNamedAttribute(id):
    pd = glue_getParamDict()
    if pd is not None:
        try:
            if isinstance(pd[id], ptAttribNamedActivator):
                return 1
            if isinstance(pd[id], ptAttribNamedResponder):
                return 2
        except KeyError:
            if glue_verbose:
                PtDebugPrint(f"Could not find id={id} attribute")

    return 0

def glue_isMultiModifier():
    inst = glue_getInst()
    if isinstance(inst, ptMultiModifier):
        return 1
    return 0

def glue_getVisInfo(number):
    global glue_paramKeys

    pd = glue_getParamDict()
    if pd is not None:
        # see if there is a paramKey list
        if isinstance(glue_paramKeys, list):
            if number in range(len(glue_paramKeys)):
                return pd[glue_paramKeys[number]].getVisInfo()
            else:
                PtDebugPrint(f"glue_getVisInfo: Error! {number} out of range of attribute list")
        else:
            pl = list(pd.values())
            if number in range(len(pl)):
                return pl[number].getVisInfo()
            else:
                if glue_verbose:
                    PtDebugPrint(f"glue_getVisInfo: Error! {number} out of range of attribute list")

    if glue_verbose:
        PtDebugPrint("GLUE: Attribute list error")

    return None
