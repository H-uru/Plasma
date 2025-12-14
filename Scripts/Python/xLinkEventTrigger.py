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

"""
Link Event Trigger Module

This module provides functionality to send notifications based on avatar link and
spawn events. It can trigger notifications to either a plActivatorActivatorConditionalObject
or a responder when various avatar events occur, such as spawn in/out, link in/out.

The module supports configuration through dropdown attributes to specify when (Trigger On),
for whom (Trigger For), and at what point (Trigger At) the notifications should be sent.
This allows for flexible event handling such as setting SDL variables on link in, playing
sounds, or triggering other gameplay effects based on avatar entry and exit events from
the age.

The module sends notifications to either:
- A plActivatorActivatorConditionalObject (via actActivator attribute)
- A responder (via respRespond attribute)
- Both

Note: The plActivatorActivatorConditionalObject cannot be directly exported from
Korman or 3ds Max. This module provides direct responder triggering as an alternative
for users who want to draw their own logic nodes, while also supporting a planned
higher-level helper logic node for plActivatorActivatorConditionalObject export from Korman.
"""

from Plasma import *
from PlasmaTypes import *

import enum
import itertools

# Fire this "responder" on link in. In 3ds Max or Korman logic nodes, this can be a responder
# with some messages that you fire on link in. You need to be careful about doing that, though,
# because it's easy to craft something that won't synchronize well in multiplayer. In reality, my
# thinking is that, although this is labelled a responder, it should really hold a key to a kMultiTrigger
# plLogicModifier with a plActivatorActivatorConditionalObject set as its only condition. plAACO
# seems to not be exposed to Max, but it's very useful. It tells its LM to trigger when it receives
# a state==1.0 notification (if not already triggered) and to untrigger when it receives a state==0.0
# (if already triggered). Since this is a kMultiTrigger, any time we send a notify (state==1.0) to
# the LM, the plAACO will trigger the LM, sending off whatever notifies it has stored up. This can
# be used to activate other PFMs to do things like set SDL variables (xAgeSDLBoolSet) on link in...
#
# So why a responder attribute, you ask? Well, we *could* use a ``ptAttribActivator`` and manually
# build a plNotifyMsg and send to it. But ``ptAttribResponder`` already does that with its `run()`
# method. There is really nothing in the engine that will get mad at us for sending the "wrong" key
# type for any given attribute. At the end of the day, a key is a key, and we're just sending
# messages to the receiver owned by that key. So we're just saving some time and effort by abusing
# an already existing solution.
respRespond = ptAttribResponder(1, "Responder: Fire on Link/Spawn Event", netPropagate=False)

# These trigger settings are mirror images of the enums below. The reason why we have duplicated
# the enums is because Korman parses the Python scripts with regex to get the attributes. If we
# do not list the options inline with the code, Korman won't know what the valid options are if
# someone opts to manually craft their logic nodes with the script directly, versus the friendly
# helper nodes I'm planning.
strTriggerOn = ptAttribDropDownList(100, "Trigger On", ["Nothing", "Spawn In", "Spawn Out", "Link In", "Link Out"])
strTriggerFor = ptAttribDropDownList(101, "Trigger For", ["Me", "NPCs", "Player Avatars", "Everyone", "Nobody"])
strTriggerAt = ptAttribDropDownList(102, "Trigger At", ["Start", "End"])

class TriggerOn(enum.Enum):
    NOTHING = "Nothing"
    SPAWN_IN = "Spawn In"
    SPAWN_OUT = "Spawn Out"
    LINK_IN = "Link In"
    LINK_OUT = "Link Out"


class TriggerFor(enum.Enum):
    EVERYONE = "Everyone"
    HUMANS = "Player Avatars"
    ME = "Me"
    NPCS = "NPCs"
    NOBODY = "Nobody"


class TriggerAt(enum.Enum):
    START = "Start"
    END = "End"


class xLinkEventTrigger(ptModifier):
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 4010721418
        self.version = 1
        PtDebugPrint(f"xLinkEventTrigger.__init__(): v{self.version}", level=kWarningLevel)

        self._triggerFor = TriggerFor.NOBODY
        self._triggerOn = TriggerOn.NOTHING
        self._triggerAt = TriggerAt.START
        self._localAvatarHandled = False

    def BeginAgeUnLoad(self, avObj: ptSceneobject) -> None:
        # We aren't going to get an AvatarPage out for the local player. That happens after an
        # Age is unloaded when going to the character select screen.
        if not self.triggerForLocalAvatar:
            PtDebugPrint(f"xLinkEventTrigger.BeginAgeUnLoad(): {self.sceneObjectName=} ignoring (don't want local avatar notify)", level=kDebugDumpLevel)

        if self._triggerOn == TriggerOn.SPAWN_OUT:
            PtDebugPrint(f"xLinkEventTrigger.OnFirstUpdate(): {self.sceneObjectName=} sending trigger (local player spawn out)", level=kWarningLevel)
            respRespond.run(self.key, avatar=avObj)
        else:
            PtDebugPrint(f"xLinkEventTrigger.OnFirstUpdate(): {self.sceneObjectName=} ignoring", level=kDebugDumpLevel)

    def OnInit(self):
        self._triggerFor = TriggerFor(strTriggerFor.value)
        self._triggerOn = TriggerOn(strTriggerOn.value)
        self._triggerAt = TriggerAt(strTriggerAt.value)
        PtDebugPrint(f"xLinkEventTrigger.OnInit(): {self.sceneObjectName=} {self._triggerFor=} {self._triggerOn=}", level=kDebugDumpLevel)

    def AvatarPage(self, avObj: ptSceneobject, pageIn: bool, lastOut: bool):
        playerID = PtGetClientIDFromAvatarKey(avObj.getKey())
        inout = "in" if pageIn else "out"
        if self._triggerOn == TriggerOn.NOTHING:
            PtDebugPrint(f"xLinkEventTrigger.AvatarPage(): {self.sceneObjectName=} {playerID=} ignoring spawn {inout} (we trigger for nothing)", level=kDebugDumpLevel)
            return
        if self._triggerFor == TriggerFor.NOBODY:
            PtDebugPrint(f"xLinkEventTrigger.AvatarPage(): {self.sceneObjectName=} {playerID=} ignoring spawn {inout} (we trigger for nobody)", level=kDebugDumpLevel)
            return
        if self._triggerFor == TriggerFor.ME and not self.isLocalAvatar(avObj):
            PtDebugPrint(f"xLinkEventTrigger.AvatarPage(): {self.sceneObjectName=} {playerID=} ignoring spawn in (we trigger for the local player only)", level=kDebugDumpLevel)
            return
        if self._triggerFor == TriggerFor.HUMANS and not avObj.isHuman():
            PtDebugPrint(f"xLinkEventTrigger.AvatarPage(): {self.sceneObjectName=} {playerID=} ignoring spawn {inout} (we trigger for humans only", level=kDebugDumpLevel)
            return
        if self._triggerFor == TriggerFor.NPCS and avObj.isHuman():
            PtDebugPrint(f"xLinkEventTrigger.AvatarPage(): {self.sceneObjectName=} {playerID=} ignoring spawn {inout} (we trigger for NPCs only)", level=kDebugDumpLevel)
            return

        # AvatarPage() only gets called for the first time the local player loads. Meaning this
        # must be the very first Age the client loads. Unlikely. Handle the local player as a
        # special case.
        if self.isLocalAvatar(avObj) and self._localAvatarHandled:
            PtDebugPrint(f"xLinkEventTrigger.AvatarPage(): {self.sceneObjectName=} {playerID=} ignoring spawn {inout} (local avatar already handled)", level=kDebugDumpLevel)
            return

        # Now actually handle business. We may trigger now or when link effects play. There are
        # pros and cons to each. The spawn happens right before the link effects play, but that
        # may cause some effect you want to happen for the player to be partially cut off due to it
        # starting before they're actually in the Age. Further, if this is firing for a remote
        # player linking in, the remote player (for the most part) decides when they link in, which
        # means the delay is dependent on their loading speed. Relying on link trigger, however, is
        # probably not the *best* solution though because internal clients (CSR) can go into
        # "stealth mode," which might mean they never play the link in effects at all. You wouldn't
        # want to do gameplay logic in a link effects trigger. To get the best of both worlds, it
        # would be nice if we could trigger (state==1.0) when the interesting event starts and
        # untrigger (state==0.0) when it ends. So, for example, if we wanted to trigger on link in,
        # trigger (state==1.0) when the link effect starts and untrigger (state==0.0) when the
        # link in effect stops. Unfortunately, that's not going to work.
        #
        # Why won't it work, you ask? Well, gather 'round and let Hoikas explain. Because players
        # might be linking/spawning in/out simultaneously, if we're using an the actActivator
        # mechanism, that's pointing to a LogicModifier with the MultiTrigger flag set. If MultiTrigger
        # isn't set, then the logic mod can only trigger for one avatar's link process at a time.
        # So you might miss some link events. Further, in MultiTrigger mode, you can't untrigger
        # at all.
        #
        # Hopefully this all makes sense. :)
        if self._triggerOn == TriggerOn.SPAWN_IN and pageIn:
            PtDebugPrint(f"xLinkEventTrigger.AvatarPage(): {self.sceneObjectName=} {playerID=} sending trigger (spawn in)", level=kWarningLevel)
            respRespond.run(self.key, avatar=avObj)
        elif self._triggerOn == TriggerOn.SPAWN_OUT and not pageIn:
            PtDebugPrint(f"xLinkEventTrigger.AvatarPage(): {self.sceneObjectName=} {playerID=} sending trigger (spawn out)", level=kWarningLevel)
            respRespond.run(self.key, avatar=avObj)
        elif self._triggerOn in {TriggerOn.LINK_IN, TriggerOn.LINK_OUT} and pageIn:
            PtDebugPrint(f"xLinkEventTrigger.AvatarPage(): {self.sceneObjectName=} {playerID=} registering for beh notify", level=kDebugDumpLevel)
            avObj.avatar.registerForBehaviorNotify(self.key)
        elif self._triggerOn == TriggerOn.LINK_OUT and self._triggerAt == TriggerAt.END and not pageIn:
            # There isn't actually a post-link out notification. Rightfully so, IMO. But, if you
            # want to do something after everyone else has linked out, then spawn out is the best
            # match that we have for this condition. If a post link out notify is ever added,
            # remove this block.
            PtDebugPrint(f"xLinkEventTrigger.AvatarPage(): {self.sceneObjectName=} {playerID=} sending trigger (after link out)", level=kWarningLevel)
            respRespond.run(self.key, avatar=avObj)
        else:
            PtDebugPrint(f"xLinkEventTrigger.AvatarPage(): {self.sceneObjectName=} {playerID=} {pageIn=} ignoring", level=kDebugDumpLevel)

    def OnBehaviorNotify(self, behType: int, avObj: ptSceneobject, state: bool) -> None:
        if behType not in {PtBehaviorTypes.kBehaviorTypeLinkIn, PtBehaviorTypes.kBehaviorTypeLinkOut}:
            # Avatar is running, landing on the ground, etc. Who gives a rip?
            return

        # If we get here, we know we want a link trigger. So fire off whatever they want.
        playerID = PtGetClientIDFromAvatarKey(avObj.getKey())
        at_time_str = "before" if state else "after"
        at_time = (
            (state and self._triggerAt == TriggerAt.START) or
            (not state and self._triggerAt == TriggerAt.END)
        )
        if not at_time:
            PtDebugPrint(f"xLinkEventTrigger.OnBehaviorNotify(): {self.sceneObjectName=} {playerID=} ignoring {at_time_str} link event", level=kDebugDumpLevel)
            return

        if self._triggerOn == TriggerOn.LINK_IN and behType == PtBehaviorTypes.kBehaviorTypeLinkIn:
            PtDebugPrint(f"xLinkEventTrigger.OnBehaviorNotify(): {self.sceneObjectName=} {playerID=} sending trigger ({at_time_str} link in)", level=kWarningLevel)
            avObj.avatar.unRegisterForBehaviorNotify(self.key)
            respRespond.run(self.key, avatar=avObj)
        elif self._triggerOn == TriggerOn.LINK_OUT and behType == PtBehaviorTypes.kBehaviorTypeLinkOut:
            PtDebugPrint(f"xLinkEventTrigger.OnBehaviorNotify(): {self.sceneObjectName=} {playerID=} sending trigger ({at_time_str} link out)", level=kWarningLevel)
            avObj.avatar.unRegisterForBehaviorNotify(self.key)
            respRespond.run(self.key, avatar=avObj)
        else:
            # This is probably an error condition.
            PtDebugPrint(f"xLinkEventTrigger.OnBehaviorNotify(): {self.sceneObjectName=} {playerID=} ignoring {at_time_str} link event")

    def OnFirstUpdate(self) -> None:
        # Handle the local player because AvatarPage is untrustworthy.
        if not self.triggerForLocalAvatar:
            PtDebugPrint(f"xLinkEventTrigger.OnFirstUpdate(): {self.sceneObjectName=} ignoring (don't want local avatar notify)", level=kDebugDumpLevel)
            return
        if self._localAvatarHandled:
            # This really shouldn't happen, but we'll check for completeness.
            PtDebugPrint(f"xLinkEventTrigger.OnFirstUpdate(): {self.sceneObjectName=} ignoring (local avatar already handled)", level=kDebugDumpLevel)
            return

        try:
            avObj = PtGetLocalAvatar()
        except NameError:
            PtDebugPrint(f"xLinkEventTrigger.OnFirstUpdate(): {self.sceneObjectName=} ignoring (local avatar not spawned yet)", level=kDebugDumpLevel)
            return

        if self._triggerOn in {TriggerOn.LINK_IN, TriggerOn.LINK_OUT}:
            PtDebugPrint(f"xLinkEventTrigger.OnFirstUpdated(): {self.sceneObjectName=} registering local avatar for beh notify", level=kDebugDumpLevel)
            avObj.avatar.registerForBehaviorNotify(self.key)
        elif self._triggerOn == TriggerOn.SPAWN_IN:
            PtDebugPrint(f"xLinkEventTrigger.OnFirstUpdate(): {self.sceneObjectName=} sending trigger (local player spawn in)", level=kWarningLevel)
            respRespond.run(self.key, avatar=avObj)
        elif self._triggerOn == TriggerOn.SPAWN_OUT:
            PtDebugPrint(f"xLinkEventTrigger.OnFirstUpdate(): {self.sceneObjectName=} ignoring (want spawn out)", level=kDebugDumpLevel)
        else:
            raise ValueError(self._triggerOn)

        self._localAvatarHandled = True

    def isLocalAvatar(self, avObj: ptSceneobject) -> bool:
        return avObj == PtGetLocalAvatar()

    @property
    def sceneObjectName(self) -> str:
        return self.sceneobject.getName()

    @property
    def triggerForLocalAvatar(self) -> bool:
        return self._triggerFor in {TriggerFor.EVERYONE, TriggerFor.HUMANS, TriggerFor.ME}
