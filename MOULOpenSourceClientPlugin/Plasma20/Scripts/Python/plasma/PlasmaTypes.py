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
"""
This module is contains the datatypes and constants for
interfacing with the Plasma 2.0 engine.
"""
from Plasma import *
from PlasmaConstants import *

####################################
# Utility functions
####################################
false=0
true=1

# OnNotify Event enums
kCollisionEvent=PtEventType.kCollision       # [1]=enter flag, [2]=hitter(probably avatar), [3]=hittee
kPickedEvent=PtEventType.kPicked             # [1]=enter flag, [2]=picker(probably avatar), [3]=pickee, [4]=hitpoint(world) [5]=hitpoint(local)
kControlKeyEvent=PtEventType.kControlKey     # [1]=key id, [2]=down flag
kVariableEvent=PtEventType.kVariable         # [1]=name, [2]=data type, [3]=data
kFacingEvent=PtEventType.kFacing             # [1]=enabled flag, [2]=facer(probably avatar), [3]=facee, [4]=dot product
kContainedEvent=PtEventType.kContained       # [1]=entering flag, [2]=contained(probably avatar), [3]=container
kActivateEvent=PtEventType.kActivate         # [1]=active flag, [2]=activate flag
kCallbackEvent=PtEventType.kCallback         # [1]=callback id
kResponderStateEvent=PtEventType.kResponderState  # [1]=state id
kMultiStageEvent=PtEventType.kMultiStage     # [1]=what stage, [2]=event(see below), [3]=avatar
kSpawnedEvent=PtEventType.kSpawned           # [1]=spawner, [2]=spawnee (usually avatar)
kClickDragEvent=PtEventType.kClickDrag       # not used yet
kOfferLinkingBook=PtEventType.kOfferLinkingBook	# [1]=offerer, [2]=link panel ID of age offered

    
# OnNotify Var Event Data Types
kVarNumberType=PtNotifyDataType.kNumber
kVarKeyType=PtNotifyDataType.kKey
# OnNotify MultiStageEvent - what event types
kEnterStage=PtMultiStageEventType.kEnterStage
kBeginingOfLoop=PtMultiStageEventType.kBeginingOfLoop
kAdvanceNextStage=PtMultiStageEventType.kAdvanceNextStage
kRegressPrevStage=PtMultiStageEventType.kRegressPrevStage
# Behavior - gotoStage
kStageExitBrain=-1      # sending ptAttribBehavior to stage -1 will exit brain

# OnGUINotify Control Types
kDialog=1
kButton=2
kDraggable=3
kListBox=4
kTextBox=5
kEditBox=6
kUpDownPair=7
kKnob=8
kDragBar=9
kCheckBox=10
kRadioGroup=11
kDynamicTextControl=12
kMultiLineEdit=13
# GUIControlListBox String Justify Types
kLeftJustify=1
kRightJustify=2
# GUIControlListBox String inherit flag
kNoInherit=0
kInheritFromNormal=1
kInheritFromSelect=2
kSelectDetermined=3
kSelectUseGUIColor=4
# GUIControlMultiLineEdit style flags (additive)
kFontBold=1
kFontItalic=2
kFontShadowed=4

# OnGUINotify Event Types
kShowHide=1             # show or hide change (only on kDialog)
kAction=2               # kButton clicked, kListBox item clicked on, kEditBox hit enter
kValueChanged=3         # value changed in control (could be from kUpDownPair,kKnob,kCheckBox,kRadioGroup
kDialogLoaded=4         # the dialog has just been loaded
kFocusChange=5          # the focus changed from one control to another, or none, within the dialog
kExitMode = 6		# Modal dialog received an exit mode command
kInterestingEvent = 7   # an interesting event happened

# OnRoomLoad 'what' types
kLoaded=1
kUnloaded=2

# Clothing group Types and clothing types
kMaleClothingGroup=0
kFemaleClothingGroup=1
kAnyClothingItem=-1
kPantsClothingItem=0
kShirtClothingItem=1
kLeftHandClothingItem=2
kRightHandClothingItem=3
kFaceClothingItem=4
kHairClothingItem=5
kLeftFootClothingItem=6
kRightFootClothingItem=7
kAccessoryClothingItem=8

#Debug print levels
kDebugDumpLevel = 1
kWarningLevel = 2
kErrorLevel = 3
kAssertLevel = 4


def PtAssert(cond, msg):
    "Plasma assert. Just like the Python one but we can set it to NOP in release"
    assert cond,msg

def PtDebugPrint(*msgs,**keywords):
    "Plasma debug print. Will be NOP'd when released"
    try:
        level = keywords['level']
    except LookupError:
        level = kErrorLevel
    if level >= PtGetPythonLoggingLevel():
        if level == 4:
            PtAssert(0,msgs[0])
        else:
            for msg in msgs:
                print msg

def PtGetObjectName(obj):
    "Given a ptSceneobject, return its name"
    if isinstance(obj,ptSceneobject):
        if type(obj.getKey()) != type(None):
            return obj.getKey().getName()
    return "nil"

# add an event that is in the form of a list
# ...to a notify message
def PtAddEvent(notify,event):
    "Add an event of any type to a ptNotify message object"
    if type(event) != type([]):
        print "Unrecognized event record structure"
        return
    if event[0] == kCollisionEvent:
        notify.addCollisionEvent(event[1],event[2].getKey(),event[3].getKey())
    elif event[0] == kPickedEvent:
        notify.addPickEvent(event[1],event[2].getKey(),event[3].getKey(),event[4])
    elif event[0] == kControlKeyEvent:
        notify.addControlKeyEvent(event[1],event[2])
    elif event[0] == kVariableEvent:
        if event[2] == kVarNumberType:
            notify.addVarNumber(event[1],event[3])
        elif event[2] == kVarKeyType:
            notify.addVarKey(event[1],event[3])
    elif event[0] == kFacingEvent:
        notify.addFacingEvent(event[1],event[2].getKey(),event[3].getKey(),event[4])
    elif event[0] == kContainedEvent:
        notify.addContainerEvent(event[1],event[2].getKey(),event[3].getKey())
    elif event[0] == kActivateEvent:   
        notify.addActivateEvent(event[1],event[2])
    elif event[0] == kCallbackEvent:   
        notify.addCallbackEvent(event[1])
    elif event[0] == kResponderStateEvent:
        notify.addResponderState(event[1])
    else:
        print "Unrecognized event type %d" % (event[0])
        
# add a list of events into a ptNotify message
def PtAddEvents(notify, events):
    "Add a list of events to a ptNotify message object"
    if type(events) == type([]):
        for event in events:
            PtAddEvent(notify,event)

# find the avatar in event record list
def PtFindAvatar(events):
    "Find the avatar in one of the event records"
    for event in events:
        if event[0]==kCollisionEvent or event[0]==kPickedEvent or event[0]==kFacingEvent or event[0]==kContainedEvent or event[0]==kSpawnedEvent:
            return event[2]
        if event[0] == kMultiStageEvent:
            return event[3]
    # didn't find one
    return None
    
####################################
# Exceptions
####################################
import exceptions
class ptResponderStateError(exceptions.Exception):
    def __init__(self,args=None):
        self.args = args

#
# Attributes that will be exposed in Max to be filled in by <someone>

####################################
# base class for all attributes
####################################
# (This makes it easy to find all the attributes in a module)
class ptAttribute:
    def __init__(self,id,name, vislistid = -1, visliststates = []):
        self.id = id
        self.name = name
        self.vis_id = vislistid
        self.vis_states = visliststates

    def setVisInfo(self, id, stateslist):
        self.vis_id = id
        self.vis_states = stateslist

    def getVisInfo(self):
        return (self.vis_id, self.vis_states)

# base class for all list attributes
# (This makes it easy to find all the attributes that are a list)
class ptAttributeList(ptAttribute):
    def __init__(self,id,name):
        ptAttribute.__init__(self,id,name)

# Boolean attribute (Checkbox)
class ptAttribBoolean(ptAttribute):
    def __init__(self,id,name=None, default=0):
        ptAttribute.__init__(self,id,name)
        self.value = default
    def getdef(self):
        return (self.id,self.name,1,self.value)

# Integer attribute (Spinner)
class ptAttribInt(ptAttribute):
    def __init__(self,id,name=None,default=0,rang=None):
        ptAttribute.__init__(self,id,name)
        self.value = default
        self.rang = rang
    def getdef(self):
        return (self.id,self.name,2,self.value,self.rang)

# Floating point number attribute (Spinner)
class ptAttribFloat(ptAttribute):
    def __init__(self,id,name=None,default=0.0,rang=None):
        ptAttribute.__init__(self,id,name)
        self.value = default
        self.rang = rang
    def getdef(self):
        return (self.id,self.name,3,self.value,self.rang)

# String attribute (Edit box)
class ptAttribString(ptAttribute):
    def __init__(self,id,name=None,default=""):
        ptAttribute.__init__(self,id,name)
        self.value = default
    def getdef(self):
        return (self.id,self.name,4,self.value)

# Drop-down list attribute (Drop-down list combo box)
class ptAttribDropDownList(ptAttribute):
    def __init__(self,id,name=None,options=None):
        ptAttribute.__init__(self,id,name)
        self.options = options
    def getdef(self):
        return (self.id,self.name,20,self.options)

# Sceneobject attribute (pick single sceneobject box)
class ptAttribSceneobject(ptAttribute):
    def __init__(self,id,name=None,netForce=0):
        ptAttribute.__init__(self,id,name)
        self.value = None
        self.sceneobject = None
        self.netForce = netForce
    def getdef(self):
        return (self.id,self.name,5)
    def __setvalue__(self,value):
        if self.netForce:
            value.netForce(1)
        self.value = value
        self.sceneobject = self.value

# Sceneobject list attribute (pick multiple sceneobjects box)
class ptAttribSceneobjectList(ptAttributeList):
    def __init__(self,id,name=None,byObject=0,netForce=0):
        ptAttributeList.__init__(self,id,name)
        self.value = []     # start as an empty list
        self.sceneobject = self.value
        self.netForce = netForce
        if byObject:
            self.byObject = {}
        else:
            self.byObject = None
    def getdef(self):
        return (self.id,self.name,6)
    def __setvalue__(self,value):
        if self.netForce:
            value.netForce(1)
        self.value.append(value)
        if type(self.byObject) == type({}):
            name = value.getName()
            self.byObject[name] = value

# attribute list of keys
class ptAttributeKeyList(ptAttributeList):
    def __init__(self,id,name=None,byObject=0,netForce=0):
        ptAttributeList.__init__(self,id,name)
        self.value = []
        self.netForce = netForce
        if byObject:
            self.byObject = {}
        else:
            self.byObject = None
    def enable(self,objectName=None):
        if self.value != None:
            if type(objectName) != type(None) and type(self.byObject) != type(None):
                pkey = self.byObject[objectName]
                if self.netForce:
                    pkey.netForce(1)
                pkey.enable()
            elif type(self.value)==type([]):
                for pkey in self.value:
                    if self.netForce:
                        pkey.netForce(1)
                    pkey.enable()
            else:
                if self.netForce:
                    self.value.netForce(1)
                self.value.enable()
    def disable(self,objectName=None):
        if self.value != None:
            if type(objectName) != type(None) and type(self.byObject) != type(None):
                pkey = self.byObject[objectName]
                if self.netForce:
                    pkey.netForce(1)
                pkey.disable()
            elif type(self.value)==type([]):
                for pkey in self.value:
                    if self.netForce:
                        pkey.netForce(1)
                    pkey.disable()
            else:
                if self.netForce:
                    self.value.netForce(1)
                self.value.disable()
    def __setvalue__(self,value):
        if self.netForce:
            value.netForce(1)
        self.value.append(value)
        if type(self.byObject) == type({}):
            name = value.getName()
            self.byObject[name] = value

# Activator attribute (pick activator types box)
class ptAttribActivator(ptAttributeKeyList):
    def getdef(self):
        return (self.id,self.name,8)
    def enableActivator(self):
        for key in self.value:
            key.getSceneObject().physics.enable()
    def disableActivator(self):
        for key in self.value:
            key.getSceneObject().physics.disable()
    def volumeSensorIgnoreExtraEnters(self,state):
        for key in self.value:
            key.getSceneObject().volumeSensorIgnoreExtraEnters(state)

# Activator attribute (pick activator types box)
class ptAttribActivatorList(ptAttributeKeyList):
    def getdef(self):
        return (self.id,self.name,7)

# Responder attribute (pick responder types box)
class ptAttribResponder(ptAttributeKeyList):
    def __init__(self,id,name=None,statelist=None,byObject=0,netForce=0):
        ptAttributeKeyList.__init__(self,id,name,byObject,netForce)
        self.state_list = statelist
    def getdef(self):
        return (self.id,self.name,9)
    def run(self,key,state=None,events=None,avatar=None,objectName=None,netForce=0,netPropagate=1,fastforward=0):
        # has the value been set?
        if type(self.value) != type(None):
            nt = ptNotify(key)
            nt.clearReceivers()
            # see if the value is a list or byObject or a single
            if type(objectName) != type(None) and type(self.byObject) != type(None):
                nt.addReceiver(self.byObject[objectName])
            elif type(self.value)==type([]):
                for resp in self.value:
                    nt.addReceiver(resp)
            else:
                nt.addReceiver(self.value)
            if not netPropagate:
                nt.netPropagate(0)
                # ptNotify defaults to netPropagate=1
            if netForce or self.netForce:
                nt.netForce(1)
            # see if the state is specified
            if type(state) == type(0):
                raise ptResponderStateError,"Specifying state as a number is no longer supported"
            elif type(state) == type(''):
                if type(self.state_list) != type(None):
                    try:
                        idx = self.state_list.index(state)
                        nt.addResponderState(idx)
                    except ValueError:
                        raise ptResponderStateError, "There is no state called '%s'"%(state)
                else:
                    raise ptResponderStateError,"There is no state list provided"
            # see if there are events to pass on
            if type(events) != type(None):
                PtAddEvents(nt,events)
            if type(avatar) != type(None):
                nt.addCollisionEvent(1,avatar.getKey(),avatar.getKey())
            if fastforward:
                nt.setType(PtNotificationType.kResponderFF)
                # if fast forwarding, then only do it on the local client
                nt.netPropagate(0)
                nt.netForce(0)
            nt.setActivate(1.0)
            nt.send()
    def setState(self,key,state,objectName=None,netForce=0,netPropagate=1):
        # has the value been set?
        if type(self.value) != type(None):
            nt = ptNotify(key)
            nt.clearReceivers()
            # see if the value is a list or byObject or a single
            if type(objectName) != type(None) and type(self.byObject) != type(None):
                nt.addReceiver(self.byObject[objectName])
            elif type(self.value)==type([]):
                for resp in self.value:
                    nt.addReceiver(resp)
            else:
                nt.addReceiver(self.value)
            if not netPropagate:
                nt.netPropagate(0)
                # ptNotify defaults to netPropagate=1
            if netForce or self.netForce:
                nt.netForce(1)
            # see if the state is specified
            if type(state) == type(0):
                raise ptResponderStateError,"Specifying state as a number is no longer supported"
            elif type(state) == type(''):
                if type(self.state_list) != type(None):
                    try:
                        idx = self.state_list.index(state)
                        nt.addResponderState(idx)
                    except ValueError:
                        raise ptResponderStateError, "There is no state called '%s'"%(state)
                else:
                    raise ptResponderStateError,"There is no state list provided"
            # see if there are events to pass on
            nt.setType(PtNotificationType.kResponderChangeState)
            nt.setActivate(1.0)
            nt.send()

    def getState(self):
        if (type(self.value) != type(None)):
            if type(self.value)==type([]):
                for resp in self.value:
                    obj = resp.getSceneObject()
                    idx = obj.getResponderState()
                    curState = self.state_list[idx]
                    return curState
            else:
                obj = self.value.getSceneObject()
                idx = obj.getResponderState()
                curState = self.state_list[idx]
                return curState


# Responder attribute List
class ptAttribResponderList(ptAttribResponder):
    def getdef(self):
        return (self.id,self.name,10)
 
# Activator attribute (pick activator types box)
class ptAttribNamedActivator(ptAttribActivator):
    def getdef(self):
        # get attribute as a string, then we will turn it into an activator later
        return (self.id,self.name,4,self.value)

# Responder attribute (pick responder types box)
class ptAttribNamedResponder(ptAttribResponder):
    def getdef(self):
        # get attribute as a string, then we will turn it into an responder later
        return (self.id,self.name,4,self.value)

# DynamicText attribute pick button
class ptAttribDynamicMap(ptAttribute):
    def __init__(self,id,name=None,netForce=0):
        ptAttribute.__init__(self,id,name)
        self.value = None
        self.textmap = None
        self.netForce = netForce
    # this is to set the value via method (only called if defined)
    def __setvalue__(self,value):
        # has a ptDynamicText already been made
        try:
            self.textmap.addKey(value)
        except AttributeError:
            self.textmap = ptDynamicMap(value)
            if self.netForce:
                self.textmap.netForce(1)
            self.value = self.textmap
    def getdef(self):
        return (self.id,self.name,11)

# a GUI Dialogbox attribute
class ptAttribGUIDialog(ptAttribute):
    def __init__(self,id,name=None):
        ptAttribute.__init__(self,id,name)
        self.dialog = None
        self.value = None
    def getdef(self):
        return (self.id,self.name,12)
    def __setvalue__(self,value):
        self.dialog = ptGUIDialog(value)
        self.value = self.dialog

# a Exclude region attribute
kExRegRelease = 0
kExRegClear = 1
class ptAttribExcludeRegion(ptAttribute):
    def __init__(self,id,name=None):
        ptAttribute.__init__(self,id,name)
        self.value = None
    def getdef(self):
        return (self.id,self.name,13)
    def clear(self,sender):
        if type(self.value) != type(None):
            PtExcludeRegionSet(sender,self.value,kExRegClear)
    def release(self,sender):
        if type(self.value) != type(None):
            PtExcludeRegionSet(sender,self.value,kExRegRelease)
    def enable(self):
        self.sceneobject.physics.enable()
    def disable(self):
        self.sceneobject.physics.disable()
    def clearNow(self,sender):
        if type(self.value) != type(None):
            PtExcludeRegionSetNow(sender,self.value,kExRegClear)
    def releaseNow(self,sender):
        if type(self.value) != type(None):
            PtExcludeRegionSetNow(sender,self.value,kExRegRelease)

class ptAttribWaveSet(ptAttribute):
    def __init__(self,id,name=None):
        ptAttribute.__init__(self,id,name)
        self.value = None
    def getdef(self):
        return (self.id,self.name,19)
    def __setvalue__(self,value):
        self.waveset = ptWaveSet(value)
        self.value = self.waveset

class ptAttribSwimCurrent(ptAttribute):
    def __init__(self,id,name=None):
        ptAttribute.__init__(self,id,name)
        self.value = None
        self.current = None
    def getdef(self):
        return (self.id,self.name,21)
    def __setvalue__(self,value):
        self.current = ptSwimCurrentInterface(value)
        self.value = self.current

class ptAttribClusterList(ptAttributeList):
    def __init__(self,id,name=None):
        ptAttribute.__init__(self,id,name)
        self.value = []
    def getdef(self):
        return (self.id,self.name,22)
    def __setvalue__(self,value):
        self.value.append(ptCluster(value))
    
# special class for byObject that gets the parents name of the values when someone first asks for them
class ptByAnimObject(dict):
    def __init__(self):
        dict.__init__(self)
        self.gotParentKeys = 0
    def getParentKeys(self):
        # if we haven't got the parent keys yet, then add them to the dict
        if not self.gotParentKeys:
            for anim in dict.values(self):
                # get the animation target key
                aKey = anim.getFirstKey()
                if aKey:
                    # get its parent key
                    pKey = aKey.getParentKey()
                    if pKey:
                        # only add this key once
                        if not pKey.getName() in self:
                            dict.__setitem__(self,pKey.getName(),anim)
            self.gotParentKeys = 1
    def __getitem__(self,key):
        self.getParentKeys()
        return dict.__getitem__(self,key)
    def keys(self):
        self.getParentKeys()
        return dict.keys(self)
    def get(self, key, *args):
        self.getParentKeys()
        return dict.get(self, key, *args)

# an Animation attribute
kAnimEaseNoEase = 0
kAnimEaseConstAccel = 1
kAnimEaseSpline = 2
class ptAttribAnimation(ptAttribute):
    def __init__(self,id,name=None,byObject=0,netForce=0):
        ptAttribute.__init__(self,id,name)
        self.value = None
        self.animName = ""
        self.netForce = netForce
        if byObject:
            self.byObject = ptByAnimObject()
        else:
            self.byObject = None
    # this is to set the value via method (only called if defined)
    def __setvalue__(self,value):
        # has a ptAnimation already been made
        if type(value) == type(""):
            self.animName = value
            try:
                self.animation.setAnimName(value)
                # then if there are animations by object then set those, too
                if isinstance(self.byObject,ptByAnimObject):
                    for anim in self.byObject.values():
                        anim.setAnimName(value)
            except AttributeError:
                self.animation = ptAnimation()
                self.animation.setAnimName(value)
                if self.netForce:
                    self.animation.netForce(1)
                self.value = self.animation
        elif isinstance(value,ptKey):
            try:
                self.animation.addKey(value)
            except AttributeError:
                self.animation = ptAnimation()
                self.animation.addKey(value)
                if self.netForce:
                    self.animation.netForce(1)
                self.value = self.animation
            if isinstance(self.byObject,ptByAnimObject):
                singleAnim = ptAnimation()
                singleAnim.addKey(value)
                if self.netForce:
                    singleAnim.netForce(1)
                # set name if known
                if self.animName != "":
                    singleAnim.setAnimName(self.animName)
                name = value.getName()
                self.byObject[name] = singleAnim
    def getdef(self):
        return (self.id,self.name,14)

# a Behavior attribute
class ptAttribBehavior(ptAttribute):
    "Attribute for specifying behaviors, including multistage Behaviors"
    def __init__(self,id,name=None,netForce=1,netProp=1):
        ptAttribute.__init__(self,id,name)
        self.value = None
        self.netForce = netForce
        self.netProp = netProp
    def __setvalue__(self,value):
        self.value = value
        PtSetBehaviorNetFlags(self.value, self.netForce, self.netProp)
    def getdef(self):
        return (self.id,self.name,15)
    def run(self,avatar):
        "This will run the behavior on said avatar"
        if type(self.value) != type(None):
            if self.netForce:
                self.value.netForce(1)
                avatar.avatar.netForce(1)
            avatar.avatar.runBehavior(self.value,self.netForce,self.netProp)
    def nextStage(self,avatar,transitionTime=1.0,setTimeFlag=1,newTime=0.0,dirFlag=0,isForward=1):
        "This will go to the next stage in a multi-stage behavior"
        if type(self.value) != type(None):
            if self.netForce:
                self.value.netForce(1)
                avatar.avatar.netForce(1)
            avatar.avatar.nextStage(self.value,transitionTime,setTimeFlag,newTime,dirFlag,isForward,self.netForce)
    def previousStage(self,avatar,transitionTime=1.0,setTimeFlag=1,newTime=0.0,dirFlag=0,isForward=1):
        "This will go to the next stage in a multi-stage behavior"
        if type(self.value) != type(None):
            if self.netForce:
                self.value.netForce(1)
                avatar.avatar.netForce(1)
            avatar.avatar.previousStage(self.value,transitionTime,setTimeFlag,newTime,dirFlag,isForward,self.netForce)
    def gotoStage(self,avatar,stage,transitionTime=1.0,setTimeFlag=1,newTime=0.0,dirFlag=0,isForward=1):
        "This will go to the next stage in a multi-stage behavior"
        if type(self.value) != type(None):
            if self.netForce:
                self.value.netForce(1)
                avatar.avatar.netForce(1)
            avatar.avatar.gotoStage(self.value,stage,transitionTime,setTimeFlag,newTime,dirFlag,isForward,self.netForce)
    def setLoopCount(self,stage,loopCount):
        "This will set the loop count for a stage"
        if type(self.value) != type(None):
            PtSetBehaviorLoopCount(self.value,stage,loopCount,self.netForce)

# Material texture attribute pick button
class ptAttribMaterial(ptAttribute):
    def __init__(self,id,name=None):
        ptAttribute.__init__(self,id,name)
        self.value = None
        self.map = None
    # this is to set the value via method (only called if defined)
    def __setvalue__(self,value):
        self.map = ptImage(value)
        self.value = self.map
    def getdef(self):
        return (self.id,self.name,16)

class ptAttribMaterialAnimation(ptAttribute):
    def __init__(self, id, name = None):
        ptAttribute.__init__(self, id, name)
        self.value = None
        self.animation = None

    def __setvalue__(self, value):
        if type(self.animation) == type(None):
            self.animation = ptAnimation()
            self.animation.addKey(value)
            self.value = self.animation
        else:
            self.animation.addKey(value)

    def getdef(self):
        return (self.id, self.name, 23)


# Sceneobject list attribute (pick multiple sceneobjects box)
class ptAttribMaterialList(ptAttributeList):
    def __init__(self,id,name=None,byObject=0,netForce=0):
        ptAttributeList.__init__(self,id,name)
        self.value = []     # start as an empty list
        self.map = self.value
        self.netForce = netForce
        if byObject:
            self.byObject = {}
        else:
            self.byObject = None
    def getdef(self):
        return (self.id,self.name,6)
    def __setvalue__(self,value):
        if self.netForce:
            value.netForce(1)
        self.value.append(value)
        if type(self.byObject) == type({}):
            name = value.getName()
            self.byObject[name] = value

# a GUI PopUpMenu attribute
class ptAttribGUIPopUpMenu(ptAttribute):
    def __init__(self,id,name=None):
        ptAttribute.__init__(self,id,name)
        self.value = None
    def getdef(self):
        return (self.id,self.name,17)
    def __setvalue__(self,value):
        self.menu = ptGUIPopUpMenu(value)
        self.value = self.menu

# a GUI Skin attribute
class ptAttribGUISkin(ptAttribute):
    def __init__(self,id,name=None):
        ptAttribute.__init__(self,id,name)
        self.value = None
    def getdef(self):
        return (self.id,self.name,18)
    def __setvalue__(self,value):
        self.skin = ptGUISkin(value)
        self.value = self.skin

# a Grass Shader attribute
class ptAttribGrassShader(ptAttribute):
    def __init__(self,id,name=None):
        ptAttribute.__init__(self,id,name)
        self.value = None
    def getdef(self):
        return (self.id,self.name,24)
    def __setvalue__(self,value):
        self.shader = ptGrassShader(value)
        self.value = self.shader


#
# ptModifier  - class for creating a Plasma modifier, such as a responder

# base class
class ptModifier:
    def __init__(self):
        self.key = None
        self.SDL = None
        self.version = 0

class ptResponder(ptModifier):
    # this modifier will get a plNotifyMsg as an OnNotify
    def __init__(self):
        ptModifier.__init__(self)
        self.sceneobject = None

class ptMultiModifier(ptModifier):
    # this modifier can be attached to multiple object, but only one module
    def __init__(self):
        ptModifier.__init__(self)
