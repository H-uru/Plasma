# -*- coding: utf-8 -*-
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

 *==LICENSE==* """

from Plasma import *
from PlasmaTypes import *

solvedVariableName = ptAttribString(1, "SDL: Boolean Solved Variable")
butsInUseVariableName = ptAttribString(2, "SDL: Button Lock Variable")
numCorrectVariableName = ptAttribString(3, "SDL: Number of correct entries")
combination = ptAttribString(4, "Combination")
resetOnEmpty = ptAttribBoolean(5, "Reset when the Age shuts down", default=False)
disableOnSolve = ptAttribBoolean(6, "Disable activators on solve", default=True)
actButtons = ptAttribActivatorList(7, "Act: Buttons")
respButtonPush = ptAttribResponder(8, "Resp: Button Push")
allowSlidingSolution = ptAttribBoolean(9, "Allow solving by sliding solution rather than needing feedback on failure", default=False)

class xAgeSDLBoolActivatorComboSet(ptResponder):
    def __init__(self):
        ptResponder.__init__(self)
        self.version = 1
        self.id = 719

        # Defer setting up the handlers until PFM is initialized by the engine
        self._NotifyHandlers = {}
        self._HitActId = None

    def OnInit(self):
        self._ValidateCombination()
        if respButtonPush.value is not None:
            self._NotifyHandlers[actButtons.id] = self._RunPushResp
        else:
            self._NotifyHandlers[actButtons.id] = self._PickButton
        self._NotifyHandlers[respButtonPush.id] = self._RespPushComplete

    def OnServerInitComplete(self):
        ageSDL = PtGetAgeSDL()
        ageSDL.setFlags(solvedVariableName.value, True, True)
        ageSDL.sendToClients(solvedVariableName.value)
        ageSDL.setNotify(self.key, solvedVariableName.value, 0.0)
        ageSDL.setFlags(butsInUseVariableName.value, True, True)
        ageSDL.sendToClients(butsInUseVariableName.value)
        ageSDL.setNotify(self.key, butsInUseVariableName.value, 0.0)
        ageSDL.setFlags(numCorrectVariableName.value, True, True)
        ageSDL.sendToClients(numCorrectVariableName.value)
        self.SDL.setDefault("attemptCombo", ())
        self.SDL.sendToClients("attemptCombo")

        if not PtGetPlayerList():
            self._butsInUse = False
            if resetOnEmpty.value:
                self._solved = (False, "fastforward")
                self._numCorrect = (0, "fastforward")

        if self._butsInUse:
            actButtons.disable()

    # ======================================================================

    def OnSDLNotify(self, VARname, SDLname, playerID, tag):
        if VARname == solvedVariableName.value:
            if not self._solved:
                self._numCorrect = 0
        elif VARname == butsInUseVariableName.value:
            if self._butsInUse:
                actButtons.disable()
            else:
                actButtons.enable()

    def OnNotify(self, state, id, events):
        if not state or not PtWasLocallyNotified(self.key):
            return
        handler = self._NotifyHandlers.get(id, None)
        if handler is None:
            PtDebugPrint(f"xAgeSDLBoolActComboSet.OnNotify():\tUnhandled notify {id=} {events=}", level=kDebugDumpLevel)
        else:
            handler(events)

    # ======================================================================

    def _GetAgeSDL(self, attr):
        return PtGetAgeSDL()[attr.value][0]
    def _SetAgeSDL(self, attr, value):
        ageSDL = PtGetAgeSDL()
        if isinstance(value, tuple):
            state, hint = value
        else:
            state, hint = value, ""
        ageSDL.setTagString(attr.value, hint)
        ageSDL.setIndexNow(attr.value, 0, state)

    def _GetScriptSDL(self, name):
        return self.SDL[name]
    def _SetScriptSDL(self, name, value):
        self.SDL[name] = tuple(value)

    @property
    def _butsInUse(self):
        return self._GetAgeSDL(butsInUseVariableName)

    @_butsInUse.setter
    def _butsInUse(self, value):
        self._SetAgeSDL(butsInUseVariableName, value)

    @property
    def _solved(self):
        return self._GetAgeSDL(solvedVariableName)

    @_solved.setter
    def _solved(self, value):
        self._SetAgeSDL(solvedVariableName, value)

    @property
    def _numCorrect(self):
        return self._GetAgeSDL(numCorrectVariableName)

    @_numCorrect.setter
    def _numCorrect(self, value):
        self._SetAgeSDL(numCorrectVariableName, value)

    @property
    def _attemptCombo(self):
        return list(self._GetScriptSDL("attemptCombo"))

    @_attemptCombo.setter
    def _attemptCombo(self, value):
        self._SetScriptSDL("attemptCombo", value)

    # ======================================================================

    def _GetPickedActID(self, events):
        try:
            pickedSOKey = next((i[3].getKey() for i in events if i[0] == kPickedEvent))
            return next((i for i, actKey in enumerate(actButtons.value) if actKey.getParentKey() == pickedSOKey))
        except:
            PtDebugPrint("xAgeSDLBoolActivatorComboSet._GetPickedActID():\tUnable to determine picked activator ID")
            raise

    def _PickButton(self, events):
        # Run if there is no push responder
        actId = self._GetPickedActID(events)
        PtDebugPrint(f"xAgeSDLBoolActivatorComboSet._PickedButton():\tPicked button {actId}", level=kWarningLevel)
        self._TriggerButton(actId)

    def _RunPushResp(self, events):
        self._HitActId = self._GetPickedActID(events)
        PtDebugPrint(f"xAgeSDLBoolActivatorComboSet._RunPushResp():\tPushing button {self._HitActId}", level=kWarningLevel)
        self._butsInUse = True

        # Responders can't be fired by integer index, so we'll have to do this the hard way...
        notify = ptNotify(self.key)
        if isinstance(respButtonPush.value, (list, tuple)):
            for i in respButtonPush.value:
                notify.addReceiver(i)
        else:
            notify.addReceiver(respButtonPush.value)
        notify.netPropagate(True)
        notify.netForce(True)
        avKey = PtGetLocalAvatar().getKey()
        notify.addCollisionEvent(1, avKey, avKey)
        notify.setActivate(1.0)
        notify.addResponderState(self._HitActId)
        # Whoosh... off it goes...
        notify.send()

    def _RespPushComplete(self, events):
        self._butsInUse = False

        # Proxy over to the "wait, there is no responder" method
        self._TriggerButton(self._HitActId)
        self._HitActId = None

    def _TriggerButton(self, actId):
        self._AddToAttempt(actId)

        if self._solved:
            PtDebugPrint("xAgeSDLBoolActComboSet._TriggerButton():\tYou just unsolved it, moron.", level=kWarningLevel)
            self._solved = False
            self._numCorrect = 0
            self._attemptCombo = [actId]
            return

        if allowSlidingSolution.value:
            attempt = self._attemptCombo
            self._numCorrect = next((len(attempt) - i for i in range(len(attempt)) if self._IsAttemptCorrectAtIndex(i)), 0)
            PtDebugPrint(f"xAgeSDLBoolActComboSet._TriggerButton():\t Sliding attempt {attempt} has {self._numCorrect} correct.", level=kWarningLevel)
            self._CheckSolved()
            return

        try:
            desiredActId = self._combination[self._numCorrect]
        except IndexError:
            PtDebugPrint("xAgeSDLBoolActComboSet._TriggerButton():\tWoah, an index error... Let's pretend this is a bad input...")
            desiredActId = -1

        if desiredActId == actId:
            PtDebugPrint("xAgeSDLBoolActComboSet._TriggerButton():\tThat's right!", level=kWarningLevel)
            self._numCorrect += 1
            self._CheckSolved()
        else:
            PtDebugPrint("xAgeSDLBoolActComboSet._TriggerButton():\tWRONG! THAT'S ***WRONG***!", level=kWarningLevel)
            self._solved = False
            self._numCorrect = 0

    def _ValidateCombination(self):
        if not combination.value:
            PtDebugPrint("xAgeSDLBoolActComboSet._ValidateCombination():\tCombination is unset")
        if ',' not in combination.value:
            PtDebugPrint("xAgeSDLBoolActComboSet._ValidateCombination():\tCombination does not have separation delimiters")
        self._combination = [int(i) for i in combination.value.split(',')]

    def _CheckSolved(self):
        if self._numCorrect == len(self._combination):
            PtDebugPrint("xAgeSDLBoolActComboSet._CheckSolved():\tWoohoo! You have solved the puzzle!", level=kWarningLevel)
            self._solved = True
            if disableOnSolve.value:
                self._butsInUse = True

    def _AddToAttempt(self, actId):
        attempt = self._attemptCombo
        attempt.append(actId)
        if len(attempt) > len(self._combination):
            attempt = attempt[1:]
        PtDebugPrint(f"xAgeSDLBoolActComboSet._AddToAttempt():\t New attempt value: {attempt}", level=kWarningLevel)
        self._attemptCombo = attempt

    def _IsAttemptCorrectAtIndex(self, i):
        attempt = self._attemptCombo
        return attempt[i:] == self._combination[0:len(attempt)-i]
