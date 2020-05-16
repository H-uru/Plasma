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
from PlasmaKITypes import *
from PlasmaVaultConstants import *

gMaxInviteCount = 20

def SplitParams(params):
    # parse the string, splitting on whitespace, unless a part of a quoted string
    # make sure to handle \" and \\ correctly
    args = []
    curArg = ""
    inQuotes = 0
    isEscaped = 0
    for i in range(len(params)):
        if params[i] == '\\':
            if isEscaped:
                curArg += '\\'
                isEscaped = 0
            else:
                isEscaped = 1
        elif inQuotes:
            if params[i] == '"' and not isEscaped:
                inQuotes = 0
                # curArg += '"'
                args.append(curArg)
                curArg = ""
            else:
                curArg += params[i]
            isEscaped = 0
        else:
            if params[i] == '"':
                inQuotes = 1
                # curArg += '"'
            elif params[i] == ' ' or params[i] == '\t' or params[i] == '\n' or params[i] == '\r':
                if not curArg == "":
                    args.append(curArg)
                    curArg = ""
            else:
                curArg += params[i]
            isEscaped = 0
    if not curArg == "":
        args.append(curArg)
    return args


def CreateInvitation(params=None):
    "create an invitation"
    PtDebugPrint("xInvite: create invitation",level=kDebugDumpLevel)
    passkey = params
    if isinstance(passkey, str):
        invites = ptVault().getInviteFolder()
        if invites is not None:
            if invites.getChildNodeCount() <= gMaxInviteCount:
                # create the note
                note = ptVaultTextNoteNode(PtVaultNodePermissionFlags.kDefaultPermissions)
                note.noteSetTitle(passkey)
                invites.addNode(note)
                return PtGetLocalizedString("KI.Invitation.InviteKeyAdded", [str(passkey)])
            else:
                return (1,PtGetLocalizedString("KI.Invitation.MaxInvites"))
        else:
            return (1,PtGetLocalizedString("KI.Invitation.MissingInviteFolder"))
    else:
        return (1,PtGetLocalizedString("KI.Invitation.InviteUsage"))

def AcceptInvitation(params=""):
    "accept an invitation"
    PtDebugPrint("xInvite: accept invitation",level=kDebugDumpLevel)
    paramlist = SplitParams(params)
    if len(paramlist) == 2:
        PtAcceptInviteInGame(paramlist[0],paramlist[1])
        return PtGetLocalizedString("KI.Invitation.InviteAccepted", [str(paramlist[0]), str(paramlist[1])])
    else:
        return (1,PtGetLocalizedString("KI.Invitation.AcceptUsage"))

def ShowInvitations(params=None):
    "show invitations"
    PtDebugPrint("xInvite: show invitations",level=kDebugDumpLevel)
    passkeys = ""
    invites = ptVault().getInviteFolder()
    if invites is not None:
        l = invites.getChildNodeRefList()
        i = 0;
        for ref in l:
            if i != 0:
                passkeys += ", "
            child = ref.getChild()
            child = child.upcastToTextNoteNode()
            if child is not None:
                passkeys += child.noteGetTitle()
            else:
                PtDebugPrint("xInvite: Couldn't cast list item to note",level=kErrorLevel)
                pass
            i += 1
    else:
        return (1,PtGetLocalizedString("KI.Invitation.MissingInviteFolder"))
    return PtGetLocalizedString("KI.Invitation.Keys") + str(passkeys)

def DeleteInvitation(params=None):
    "delete invitation"
    PtDebugPrint("xInvite: delete invitation",level=kDebugDumpLevel)
    passkey = params
    if isinstance(passkey, str):
        invites = ptVault().getInviteFolder()
        if invites is not None:
            l = invites.getChildNodeRefList()
            removed = 0
            for ref in l:
                child = ref.getChild()
                child = child.upcastToTextNoteNode()
                if child is not None:
                    if passkey == child.noteGetTitle():
                        try:
                            invites.removeNode(child)
                        except:
                            PtDebugPrint("xInvite: Remove Node Failed",level=kErrorLevel)
                            return (1,PtGetLocalizedString("KI.Invitation.InviteNotFound"))
                        removed = 1
                        break
                else:
                    PtDebugPrint("xInvite: Couldn't cast list item to note",level=kErrorLevel)
                    return (1,PtGetLocalizedString("KI.Invitation.InviteNotFound"))
            if removed == 0:
                return (1,PtGetLocalizedString("KI.Invitation.InviteNotFound"))
        else:
            return (1,PtGetLocalizedString("KI.Invitation.MissingInviteFolder"))
    else:
        return (1, PtGetLocalizedString("KI.Invitation.UninviteUsage"))
    return PtGetLocalizedString("KI.Invitation.DeletedInvitation") + str(passkey)

def MeChat(params=None):
    if (params == None):
        PtDebugPrint('xChatExtend:MeCmd: If you have nothing to say, why say anything at all?')
        return 
    PtSendKIMessage(kKIChatStatusMsg, ('%s %s' % (PtGetLocalPlayer().getPlayerName(), params)))
