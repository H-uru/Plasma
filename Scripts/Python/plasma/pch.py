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
""" pch - Plasma Console Helper
This module aids in the using the plasma console to debug PythonFileComponents.
"""
# plasma console helper
import Plasma
import PlasmaTypes
import sys

# globals for the outside and inside to grab
__pmods = []
__sel = 0
__selattr = 0

# help!
def help():
    "display help"
    print "functions:"
    print "  getmods()  - gets all the modules available"
    print "  showmods() - show the modules (and the current selected module)"
    print "  selmod(i)  - selects module 'i'; returns module object"
    print "  showmod()  - shows detail of the module selected"
    print "  showdoc()  - shows the doc field of the module selected"
    print "  showattribs() - shows all the plasma attributes of the module selected"
    print "  selattrib(id) - selects attribute 'id' in the selected module;"
    print "                  returns attribute object"
    print "  setattrib(value) - sets the selected attribute to 'value'"
    print "  showglobals() - shows the globals for the selected module"
    print "  setglobal(name,value) - set global 'name' to 'value' in selected module"
    print "  getglobal(name)  - returns global object"
    print "  showinst()  - shows the instance of the ptModifier class for the selected"
    print "                module"
    print "  getinst()   - returns the instance object"
    print "  showvars(inst)  - shows the instance variables of the ptModifier class of"
    print "                    the selected module"
    print "  showmethods(inst) - shows the methods of the ptModifier class of the"
    print "                      selected module"
    print "  showfunc(method)  - decompiles a method or function and shows source"
    print "  setvar(name,value) - sets instance variable 'name' to 'value' in the"
    print "                       selected module"
    print "  getvar(name) - returns the instance variable object (in selected module)"
# modules
def getmods():
    "get all the PythonFileComponent modules"
    global __pmods,__sel
    __pmods = []  # wipe the module list clean
    print "Plasma modules:"
    for modname in sys.modules.keys():
        mod = sys.modules[modname]
        if hasattr(mod,"glue_inst"):
            if __sel == len(__pmods):
                print "*%d. %s" % (len(__pmods),modname[:-13])
            else:
                print " %d. %s" % (len(__pmods),modname[:-13])
            __pmods.append([modname,mod])
def showmods():
    "show all the PythonFileComponent modules"
    global __pmods
    global __sel
    idx = 0
    print "Plasma modules:"
    for mod in __pmods:
        if idx == __sel:
            print "*%d. %s" % (idx,mod[0][:-13])
        else:
            print " %d. %s" % (idx,mod[0][:-13])
        idx += 1
def selmod(idx=None):
    "select a module from the list"
    global __pmods
    global __sel
    if type(idx) == type(None):
        idx = __sel
    elif type(idx) == type(""):
        # its a string, then find it by module name
        i = 0
        for mod in __pmods:
            if mod[0][:-13] == idx:
                break
            i += 1
        # if we didn't find the module
        if i == len(__pmods):
            print "Module %s not found" % idx
            return
        idx = i
    if idx < len(__pmods):
        __sel = idx
        print "%s selected" % (__pmods[idx][0][:-13])
        return __pmods[__sel][1]
    else:
        print "Error: index not valid. There are %d modules" % (len(__pmods))
# find attributes
def showmod():
    "show details of the selected module"
    global __pmods
    global __sel
    print "Module: %s" % (__pmods[__sel][0][:-13])
    showdoc()
    showattribs()
    showglobals()
    showinst()
def showdoc():
    "show the doc of the module selected"
    global __pmods
    global __sel
    print "Doc:"
    print __pmods[__sel][1].__doc__
def showattribs():
    "show the plasma attributes of the module selected"
    global __pmods
    global __sel
    global __selattr
    print "Attributes in %s:" % (__pmods[__sel][0][:-13])
    for name in __pmods[__sel][1].__dict__.keys():
        ist = __pmods[__sel][1].__dict__[name]
        if isinstance(ist,PlasmaTypes.ptAttribute):
            if __selattr == ist.id:
                print "*(%d) %s(%s) =" % (ist.id,name,ist.__class__.__name__),ist.value
            else:
                print " (%d) %s(%s) =" % (ist.id,name,ist.__class__.__name__),ist.value
def selattrib(id=None):
    "select a plasma attribute by id in the selected module"
    global __pmods
    global __sel
    global __selattr
    if type(id) == type(None):
        id = __selattr
    for name in __pmods[__sel][1].__dict__.keys():
        ist = __pmods[__sel][1].__dict__[name]
        if isinstance(ist,PlasmaTypes.ptAttribute):
            if id == ist.id:
                __selattr = ist.id
                print "%s(%s) =" % (name,ist.__class__.__name__),ist.value
                return ist
    print "Error: Attribute ID %d not found" % (id)
def setattrib(value):
    "set the value of the selected plasma attribute in the selected module"
    global __pmods
    global __sel
    global __selattr
    for name in __pmods[__sel][1].__dict__.keys():
        ist = __pmods[__sel][1].__dict__[name]
        if isinstance(ist,PlasmaTypes.ptAttribute):
            if __selattr == ist.id:
                if type(ist.value) == type(None) or type(ist.value) == type(value):
                    # see if there is a __setvalue__ method
                    try:
                        ist.__setvalue__(value)
                    except AttributeError:
                        ist.value = value
                else:
                    print "Error: value is not same type as attribute"
                return
    print "Error: Attribute ID %d not found" % (id)
# find globals
def showglobals():
    "show the global variables of the selected module"
    global __pmods
    global __sel
    print "Globals:"
    for name in __pmods[__sel][1].__dict__.keys():
        ist = __pmods[__sel][1].__dict__[name]
        # make sure that its not something we already know about
        if not hasattr(Plasma,name) and not hasattr(PlasmaTypes,name):
            if not isinstance(ist,PlasmaTypes.ptAttribute) and not isinstance(ist,PlasmaTypes.ptModifier):
                if name[:2] != '__' and name[:4] != 'glue':
                    if type(ist) != type(sys) and type(ist) != type(PlasmaTypes.ptAttribute):
                        print "  %s =" % (name),ist
def setglobal(name,value):
    "set a global variable to a value with in the selected module"
    global __pmods
    global __sel
    # first see if there is already a glabal by that name
    if not __pmods[__sel][1].__dict__.has_key(name):
        print "Warning: creating new global!"
    __pmods[__sel][1].__dict__[name] = value
    print "%s = " % (name),__pmods[__sel][1].__dict__[name]
def getglobal(name):
    "get a global variable with in the selected module"
    global __pmods
    global __sel
    return __pmods[__sel][1].__dict__[name]
# find instance
def showinst():
    "show details of the instance of the ptModifier class in the selected module"
    global __pmods
    global __sel
    for name in __pmods[__sel][1].__dict__.keys():
        ist = __pmods[__sel][1].__dict__[name]
        if isinstance(ist,PlasmaTypes.ptModifier):
            print "Instance of %s in module %s:" % (ist.__class__.__name__,__pmods[__sel][1].__name__[:-13])
            print "  Doc: ",ist.__doc__
            showvars(ist)
            showmethods(ist)
def getinst():
    "gets the instance of the ptModifier class in the selected module"
    global __pmods
    global __sel
    for name in __pmods[__sel][1].__dict__.keys():
        ist = __pmods[__sel][1].__dict__[name]
        if isinstance(ist,PlasmaTypes.ptModifier):
            return ist
def showvars(instance):
    "shows the variables of the instance"
    print "  Variables:"
    if len(instance.__dict__) > 0:
        for vname in instance.__dict__.keys():
            print "    %s =" % (vname),instance.__dict__[vname]
    else:
        print "    (none)"
def showmethods(instance):
    "shows the methods of the instance"
    print "  Methods:"
    for mname in instance.__class__.__dict__.keys():
        mist = instance.__class__.__dict__[mname]
        # is it a function... see if it has code
        if hasattr(mist,'func_code'):
            # gather arguments
            args = "("
            for i in range(mist.func_code.co_argcount):
                args += mist.func_code.co_varnames[i]
                if i+1 < mist.func_code.co_argcount:
                    args += ","
            args += ")"
            print "    %s%s" % (mist.__name__,args)
            print "      Doc:", mist.__doc__
def showfunc(f):
    "decompiles function"
    import decompyle
    if hasattr(f,'func_code'):
        # create the argument list
        argstr = "("
        argcount = 0
        for arg in f.func_code.co_varnames[:f.func_code.co_argcount]:
            argstr += arg
            argcount += 1
            if argcount < f.func_code.co_argcount:
                argstr += ","
        argstr += ")"
        print "%s%s" % (f.func_name,argstr)
        print "    Doc:",f.__doc__
        decompyle.decompyle(f.func_code)
def setvar(vname,value):
    "set a variable within the instance of the ptModifier class in the selected module"
    global __pmods
    global __sel
    for name in __pmods[__sel][1].__dict__.keys():
        ist = __pmods[__sel][1].__dict__[name]
        if isinstance(ist,PlasmaTypes.ptModifier):
            # first see if there is already a glabal by that name
            if not ist.__dict__.has_key(vname):
                print "Warning: creating new class variable!"
            ist.__dict__[vname] = value
            print "%s = " % (vname),ist.__dict__[vname]
def getvar(vname):
    "get the variable in the instance of the ptModifier class in the selected module"
    global __pmods
    global __sel
    for name in __pmods[__sel][1].__dict__.keys():
        ist = __pmods[__sel][1].__dict__[name]
        if isinstance(ist,PlasmaTypes.ptModifier):
            return ist.__dict__[vname]
