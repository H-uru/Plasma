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

# Plasma Engine.
import plasma

# Uru modules.
import xInvite

# Enter new commands in the dictionary below.
# The key is a lowercase version of what the user types after the '/'.
# The value is a string that is a console command or a python function that
# will be executed.
xChatExtendedChat = {
    plasma.PtGetLocalizedString("KI.Commands.Sit") : plasma.PtAvatarSitOnGround,
    plasma.PtGetLocalizedString("KI.Commands.Afk") : plasma.PtAvatarEnterAFK,
    plasma.PtGetLocalizedString("KI.Commands.Invite") : xInvite.CreateInvitation,
    plasma.PtGetLocalizedString("KI.Commands.Uninvite") : xInvite.DeleteInvitation,
    plasma.PtGetLocalizedString("KI.Commands.Accept") : xInvite.AcceptInvitation,
    plasma.PtGetLocalizedString("KI.Commands.ShowInvites") : xInvite.ShowInvitations,
    plasma.PtGetLocalizedString("KI.Commands.Me") : xInvite.MeChat
}

# Enter new commands in the dictionary below.
# The key is a lowercase version of what the user types after the '/'.
# The value is a string that is the engine emote name.
xChatEmoteXlate = {
    plasma.PtGetLocalizedString("KI.Commands.Wave"): ("Wave", "KI.EmoteStrings.Wave"),
    plasma.PtGetLocalizedString("KI.Commands.Sneeze"): ("Sneeze", "KI.EmoteStrings.Sneeze"),
    plasma.PtGetLocalizedString("KI.Commands.Clap"): ("Clap", "KI.EmoteStrings.Clap"),
    plasma.PtGetLocalizedString("KI.Commands.Laugh"): ("Laugh", "KI.EmoteStrings.Laugh"),
    plasma.PtGetLocalizedString("KI.Commands.LOL"): ("Laugh", "KI.EmoteStrings.LOL"),
    plasma.PtGetLocalizedString("KI.Commands.ROTFL"): ("Laugh", "KI.EmoteStrings.ROTFL"),
    plasma.PtGetLocalizedString("KI.Commands.Dance"): ("Dance", "KI.EmoteStrings.Dance"),
    plasma.PtGetLocalizedString("KI.Commands.Yes"): ("Agree", "KI.EmoteStrings.Yes"),
    plasma.PtGetLocalizedString("KI.Commands.No"): ("ShakeHead", "KI.EmoteStrings.No"),
    plasma.PtGetLocalizedString("KI.Commands.Yawn"): ("Yawn", "KI.EmoteStrings.Yawn"),
    plasma.PtGetLocalizedString("KI.Commands.Cheer"): ("Cheer", "KI.EmoteStrings.Cheer"),
    plasma.PtGetLocalizedString("KI.Commands.Thanks"): ("Thank", "KI.EmoteStrings.Thanks"),
    plasma.PtGetLocalizedString("KI.Commands.Thx"): ("Thank", "KI.EmoteStrings.Thx"),
    plasma.PtGetLocalizedString("KI.Commands.Cry"): ("Cry", "KI.EmoteStrings.Cry"),
    plasma.PtGetLocalizedString("KI.Commands.Cries"): ("Cry", "KI.EmoteStrings.Cries"),
    plasma.PtGetLocalizedString("KI.Commands.DontKnow"): ("Shrug", "KI.EmoteStrings.DontKnow"),
    plasma.PtGetLocalizedString("KI.Commands.Shrug"): ("Shrug", "KI.EmoteStrings.Shrug"),
    plasma.PtGetLocalizedString("KI.Commands.Dunno"): ("Shrug", "KI.EmoteStrings.Dunno"),
    plasma.PtGetLocalizedString("KI.Commands.Point"): ("Point", "KI.EmoteStrings.Point"),
    plasma.PtGetLocalizedString("KI.Commands.BeckonBig"): ("BeckonBig", "KI.EmoteStrings.BeckonBig"),
    plasma.PtGetLocalizedString("KI.Commands.BeckonSmall"): ("BeckonSmall", "KI.EmoteStrings.BeckonSmall"),
    plasma.PtGetLocalizedString("KI.Commands.BlowKiss"): ("BlowKiss", "KI.EmoteStrings.BlowKiss"),
    plasma.PtGetLocalizedString("KI.Commands.CallMe"): ("CallMe", "KI.EmoteStrings.CallMe"),
    plasma.PtGetLocalizedString("KI.Commands.Cower"): ("Cower", "KI.EmoteStrings.Cower"),
    plasma.PtGetLocalizedString("KI.Commands.Groan"): ("Groan", "KI.EmoteStrings.Groan"),
    plasma.PtGetLocalizedString("KI.Commands.LeanLeft"): ("LeanLeft", "KI.EmoteStrings.LeanLeft"),
    plasma.PtGetLocalizedString("KI.Commands.LeanRight"): ("LeanRight", "KI.EmoteStrings.LeanRight"),    
    plasma.PtGetLocalizedString("KI.Commands.Okay"): ("Okay", "KI.EmoteStrings.Okay"),
    plasma.PtGetLocalizedString("KI.Commands.OverHere"): ("OverHere", "KI.EmoteStrings.OverHere"),
    plasma.PtGetLocalizedString("KI.Commands.Stop"): ("Stop", "KI.EmoteStrings.Stop"),
    plasma.PtGetLocalizedString("KI.Commands.TalkHand"): ("TalkHand", "KI.EmoteStrings.TalkHand"),
    plasma.PtGetLocalizedString("KI.Commands.TapFoot"): ("TapFoot", "KI.EmoteStrings.TapFoot"),
    plasma.PtGetLocalizedString("KI.Commands.ThumbsDown"): ("ThumbsDown", "KI.EmoteStrings.ThumbsDown"),
    plasma.PtGetLocalizedString("KI.Commands.ThumbsUp"): ("ThumbsUp", "KI.EmoteStrings.ThumbsUp"),
    plasma.PtGetLocalizedString("KI.Commands.Amazed"): ("Amazed", "KI.EmoteStrings.Amazed"),
    plasma.PtGetLocalizedString("KI.Commands.Bow"): ("Bow", "KI.EmoteStrings.Bow"),
    plasma.PtGetLocalizedString("KI.Commands.Crazy"): ("Crazy", "KI.EmoteStrings.Crazy"),
    plasma.PtGetLocalizedString("KI.Commands.Cringe"): ("Cringe", "KI.EmoteStrings.Cringe"),
    plasma.PtGetLocalizedString("KI.Commands.CrossArms"): ("CrossArms", "KI.EmoteStrings.CrossArms"),
    plasma.PtGetLocalizedString("KI.Commands.Doh"): ("Doh", "KI.EmoteStrings.Doh"),
    plasma.PtGetLocalizedString("KI.Commands.Flinch"): ("Flinch", "KI.EmoteStrings.Flinch"),
    plasma.PtGetLocalizedString("KI.Commands.Kneel"): ("Kneel", "KI.EmoteStrings.Kneel"),
    plasma.PtGetLocalizedString("KI.Commands.LookAround"): ("LookAround",            "KI.EmoteStrings.LookAround"),
    plasma.PtGetLocalizedString("KI.Commands.Peer"): ("Peer", "KI.EmoteStrings.Peer"),
    plasma.PtGetLocalizedString("KI.Commands.Salute"): ("Salute", "KI.EmoteStrings.Salute"),
    plasma.PtGetLocalizedString("KI.Commands.ScratchHead"): ("ScratchHead", "KI.EmoteStrings.ScratchHead"),
    plasma.PtGetLocalizedString("KI.Commands.ShakeFist"): ("ShakeFist", "KI.EmoteStrings.ShakeFist"),
    plasma.PtGetLocalizedString("KI.Commands.Shoo"): ("Shoo", "KI.EmoteStrings.Shoo"),
    plasma.PtGetLocalizedString("KI.Commands.SlouchSad"): ("SlouchSad", "KI.EmoteStrings.SlouchSad"),
    plasma.PtGetLocalizedString("KI.Commands.WaveLow"): ("WaveLow", "KI.EmoteStrings.WaveLow"),
    plasma.PtGetLocalizedString("KI.Commands.WaveBye"): ("Wave", "KI.EmoteStrings.WaveBye"),
    plasma.PtGetLocalizedString("KI.Commands.ThumbsDown2"): ("ThumbsDown2", "KI.EmoteStrings.ThumbsDown2"),
    plasma.PtGetLocalizedString("KI.Commands.ThumbsUp2"): ("ThumbsUp2", "KI.EmoteStrings.ThumbsUp2"),
    plasma.PtGetLocalizedString("KI.Commands.Taunt"): ("Taunt", "KI.EmoteStrings.Taunt"),
    plasma.PtGetLocalizedString("KI.Commands.Cough"): ("Cough", "KI.EmoteStrings.Cough"),
    plasma.PtGetLocalizedString("KI.Commands.AskQuestion"): ("AskQuestion", "KI.EmoteStrings.AskQuestion"),
    plasma.PtGetLocalizedString("KI.Commands.Winded"): ("Winded", "KI.EmoteStrings.Winded")
}

# Insert an emote animation from xChatEmoteXlate here to make it become a loop
xChatEmoteLoop = [
    "Dance"
]

## A list of the specially handled extended chat commands.
xChatSpecialHandledCommands = [
    plasma.PtGetLocalizedString("KI.Commands.ChatReply"),
    plasma.PtGetLocalizedString("KI.Commands.ChatPrivate"),
    plasma.PtGetLocalizedString("KI.Commands.ChatNeighbors"),
    plasma.PtGetLocalizedString("KI.Commands.ChatBuddies"),
    "/r",
]
