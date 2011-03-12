""" *==LICENSE==*

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

 *==LICENSE==* """
# glue code in python for the glue code in C++
# This assumes that this will be loaded into the module that we are trying to do
# with an execfile('.\\python\\system\\glue.py') at the end of the module (after everything is defined)
# SPECIAL WARNING(1): This glue code returns the attributes in reverse ID order!
glue_cl = None       # the class of the modifier
glue_inst = None     # instance of the class modifier
glue_params = None   # parameters dictionary: mapped id to instance
glue_paramKeys = None # this is the parameter ID list, that should be sorted
try:
    x = glue_verbose
except NameError:
    glue_verbose = 0
def glue_getClass():
    global glue_cl
    if glue_cl == None:
        try:
            cl = eval(glue_name)
            if issubclass(cl,ptModifier):
                glue_cl = cl
            else:
                if glue_verbose:
                    print "Class %s is not derived from modifier" % (cl.__name__)
        except NameError:
            if glue_verbose:
                try:
                    print "Could not find class %s" % (glue_name)
                except NameError:
                    print "Filename/classname not set!"
    return glue_cl
def glue_getInst():
    global glue_inst
    if type(glue_inst) == type(None):
        cl = glue_getClass()
        if cl != None:
            glue_inst = cl()
    return glue_inst
def glue_delInst():
    global glue_inst
    global glue_cl
    global glue_params
    global glue_paramKeys
    if type(glue_inst) != type(None):
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
    if isinstance(obj,ptAttribute):
        if glue_params.has_key(obj.id):
            if glue_verbose:
                print "WARNING: Duplicate attribute ids!"
                print "%s has id %d which is already defined in %s" % (obj.name, obj.id, glue_params[obj.id].name)
        else:
            glue_params[obj.id] = obj
    elif type(obj) == type([]):
        for o in obj:
            glue_findAndAddAttribs(o, glue_params)
    elif type(obj) == type({}):
        for o in obj.values():
            glue_findAndAddAttribs(o, glue_params)
    elif type(obj) == type( () ):
        for o in obj:
            glue_findAndAddAttribs(o, glue_params)
            
def glue_getParamDict():
    global glue_params
    global glue_paramKeys
    if type(glue_params) == type(None):
        glue_params = {}
        gd = globals()
        for obj in gd.values():
            glue_findAndAddAttribs(obj, glue_params)
        # rebuild the parameter sorted key list
        glue_paramKeys = glue_params.keys()
        glue_paramKeys.sort()
        glue_paramKeys.reverse()    # reserve the order because PlasmaMax will ask for them in reverse order
    return glue_params
def glue_getClassName():
    cl = glue_getClass()
    if cl != None:
        return cl.__name__
    if glue_verbose:
        print "Class not found in %s.py" % (glue_name)
    return None
def glue_getBlockID():
    inst = glue_getInst()
    if inst != None:
        return inst.id
    if glue_verbose:
        print "Instance could not be created in %s.py" % (glue_name)
    return None
def glue_getNumParams():
    pd = glue_getParamDict()
    if pd != None:
        return len(pd)
    if glue_verbose:
        print "No attributes found in %s.py" % (glue_name)
    return 0
def glue_getParam(number):
    global glue_paramKeys
    pd = glue_getParamDict()
    if pd != None:
        # see if there is a paramKey list
        if type(glue_paramKeys) == type([]):
            if number >= 0 and number < len(glue_paramKeys):
                return pd[glue_paramKeys[number]].getdef()
            else:
                print "glue_getParam: Error! %d out of range of attribute list" % (number)
        else:
            pl = pd.values()
            if number >= 0 and number < len(pl):
                return pl[number].getdef()
            else:
                if glue_verbose:
                    print "glue_getParam: Error! %d out of range of attribute list" % (number)
    if glue_verbose:
        print "GLUE: Attribute list error"
    return None
def glue_setParam(id,value):
    pd = glue_getParamDict()
    if pd != None:
        if pd.has_key(id):
            # first try to set the attribute via function call (if there is one)
            try:
                pd[id].__setvalue__(value)
            except AttributeError:
                if isinstance(pd[id],ptAttributeList):
                    try:
                        if type(pd[id].value) != type([]):
                            pd[id].value = []   # make sure that the value starts as an empty list
                    except AttributeError:
                        pd[id].value = []   # or if value hasn't been defined yet, then do it now
                    pd[id].value.append(value)   # add in new value to list
                else:
                    pd[id].value = value
        else:
            if glue_verbose:
                print "setParam: can't find id=",id
    else:
        print "setParma: Something terribly has gone wrong. Head for the cover."
def glue_isNamedAttribute(id):
    pd = glue_getParamDict()
    if pd != None:
        try:
            if isinstance(pd[id],ptAttribNamedActivator):
                return 1
            if isinstance(pd[id],ptAttribNamedResponder):
                return 2
        except KeyError:
            if glue_verbose:
                print "Could not find id=%d attribute" % (id)
    return 0
def glue_isMultiModifier():
    inst = glue_getInst()
    if isinstance(inst,ptMultiModifier):
        return 1
    return 0
def glue_getVisInfo(number):
    global glue_paramKeys
    pd = glue_getParamDict()
    if pd != None:
        # see if there is a paramKey list
        if type(glue_paramKeys) == type([]):
            if number >= 0 and number < len(glue_paramKeys):
                return pd[glue_paramKeys[number]].getVisInfo()
            else:
                print "glue_getVisInfo: Error! %d out of range of attribute list" % (number)
        else:
            pl = pd.values()
            if number >= 0 and number < len(pl):
                return pl[number].getVisInfo()
            else:
                if glue_verbose:
                    print "glue_getVisInfo: Error! %d out of range of attribute list" % (number)
    if glue_verbose:
        print "GLUE: Attribute list error"
    return None
