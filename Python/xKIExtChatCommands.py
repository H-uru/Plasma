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
This module contains the extended chat commands for the KI
"""

import Plasma
#import xKILocalization
import xLocalization
import xInvite

# enter new commands in the dictionary below
# the 'key' is a lowercase version of what the user types after the '/'
# the 'value' is a string that is a console command
#            or a python function that will be executed

xChatExtendedChat = {
    Plasma.PtGetLocalizedString("KI.Commands.Sit"):         Plasma.PtAvatarSitOnGround,
    Plasma.PtGetLocalizedString("KI.Commands.Afk"):         Plasma.PtAvatarEnterAFK,
    Plasma.PtGetLocalizedString("KI.Commands.Invite"):      xInvite.CreateInvitation,
    Plasma.PtGetLocalizedString("KI.Commands.Uninvite"):    xInvite.DeleteInvitation,
    Plasma.PtGetLocalizedString("KI.Commands.Accept"):      xInvite.AcceptInvitation,
    Plasma.PtGetLocalizedString("KI.Commands.ShowInvites"): xInvite.ShowInvitations,
    Plasma.PtGetLocalizedString("KI.Commands.Me"):          xInvite.MeChat
}

# enter new emote commands in the dictionary below
# the 'key' is a lowercase version of what the user types after the '/'
# the 'value' is a string that is the engine emote name
#
#   command:                                    (emoteName(ie. system name), loc path for string)
xChatEmoteXlate = {
    Plasma.PtGetLocalizedString("KI.Commands.Wave"):   ("Wave",    "KI.EmoteStrings.Wave"),
    Plasma.PtGetLocalizedString("KI.Commands.Sneeze"): ("Sneeze",  "KI.EmoteStrings.Sneeze"),
    Plasma.PtGetLocalizedString("KI.Commands.Clap"):   ("Clap",    "KI.EmoteStrings.Clap"),
    Plasma.PtGetLocalizedString("KI.Commands.Laugh"):  ("Laugh",   "KI.EmoteStrings.Laugh"),
    Plasma.PtGetLocalizedString("KI.Commands.LOL"):    ("Laugh",   "KI.EmoteStrings.LOL"),
    Plasma.PtGetLocalizedString("KI.Commands.ROTFL"):  ("Laugh",   "KI.EmoteStrings.ROTFL"),
    Plasma.PtGetLocalizedString("KI.Commands.Dance"):  ("Dance",   "KI.EmoteStrings.Dance"),
    Plasma.PtGetLocalizedString("KI.Commands.Yes"):    ("Agree",   "KI.EmoteStrings.Yes"),
    Plasma.PtGetLocalizedString("KI.Commands.No"):     ("ShakeHead","KI.EmoteStrings.No"),
    Plasma.PtGetLocalizedString("KI.Commands.Yawn"):   ("Yawn",    "KI.EmoteStrings.Yawn"),
    Plasma.PtGetLocalizedString("KI.Commands.Cheer"):  ("Cheer",   "KI.EmoteStrings.Cheer"),
    Plasma.PtGetLocalizedString("KI.Commands.Thanks"): ("Thank",   "KI.EmoteStrings.Thanks"),
    Plasma.PtGetLocalizedString("KI.Commands.Thx"):    ("Thank",   "KI.EmoteStrings.Thx"),
    Plasma.PtGetLocalizedString("KI.Commands.Cry"):    ("Cry",     "KI.EmoteStrings.Cry"),
    Plasma.PtGetLocalizedString("KI.Commands.Cries"):  ("Cry",     "KI.EmoteStrings.Cries"),
    Plasma.PtGetLocalizedString("KI.Commands.DontKnow"): ("Shrug", "KI.EmoteStrings.DontKnow"),
    Plasma.PtGetLocalizedString("KI.Commands.Shrug"):  ("Shrug",   "KI.EmoteStrings.Shrug"),
    Plasma.PtGetLocalizedString("KI.Commands.Dunno"):  ("Shrug",   "KI.EmoteStrings.Dunno"),
    Plasma.PtGetLocalizedString("KI.Commands.Point"):  ("Point",   "KI.EmoteStrings.Point"),
    Plasma.PtGetLocalizedString("KI.Commands.BeckonBig"):       ("BeckonBig",       "KI.EmoteStrings.BeckonBig"),
    Plasma.PtGetLocalizedString("KI.Commands.BeckonSmall"):     ("BeckonSmall",     "KI.EmoteStrings.BeckonSmall"),
    Plasma.PtGetLocalizedString("KI.Commands.BlowKiss"):        ("BlowKiss",        "KI.EmoteStrings.BlowKiss"),
    Plasma.PtGetLocalizedString("KI.Commands.CallMe"):          ("CallMe",          "KI.EmoteStrings.CallMe"),
    Plasma.PtGetLocalizedString("KI.Commands.Cower"):           ("Cower",           "KI.EmoteStrings.Cower"),
    Plasma.PtGetLocalizedString("KI.Commands.Groan"):           ("Groan",           "KI.EmoteStrings.Groan"),
    Plasma.PtGetLocalizedString("KI.Commands.LeanLeft"):        ("LeanLeft",        "KI.EmoteStrings.LeanLeft"),
    Plasma.PtGetLocalizedString("KI.Commands.LeanRight"):       ("LeanRight",       "KI.EmoteStrings.LeanRight"),    
    Plasma.PtGetLocalizedString("KI.Commands.Okay"):            ("Okay",            "KI.EmoteStrings.Okay"),
    Plasma.PtGetLocalizedString("KI.Commands.OverHere"):        ("OverHere",        "KI.EmoteStrings.OverHere"),
    Plasma.PtGetLocalizedString("KI.Commands.Stop"):            ("Stop",            "KI.EmoteStrings.Stop"),
    Plasma.PtGetLocalizedString("KI.Commands.TalkHand"):   ("TalkHand",   "KI.EmoteStrings.TalkHand"),
    Plasma.PtGetLocalizedString("KI.Commands.TapFoot"):         ("TapFoot",         "KI.EmoteStrings.TapFoot"),
    Plasma.PtGetLocalizedString("KI.Commands.ThumbsDown"):      ("ThumbsDown",      "KI.EmoteStrings.ThumbsDown"),
    Plasma.PtGetLocalizedString("KI.Commands.ThumbsUp"):        ("ThumbsUp",        "KI.EmoteStrings.ThumbsUp"),
    Plasma.PtGetLocalizedString("KI.Commands.Amazed"):            ("Amazed",            "KI.EmoteStrings.Amazed"),
    Plasma.PtGetLocalizedString("KI.Commands.Bow"):            ("Bow",            "KI.EmoteStrings.Bow"),
    Plasma.PtGetLocalizedString("KI.Commands.Crazy"):            ("Crazy",            "KI.EmoteStrings.Crazy"),
    Plasma.PtGetLocalizedString("KI.Commands.Cringe"):            ("Cringe",            "KI.EmoteStrings.Cringe"),
    Plasma.PtGetLocalizedString("KI.Commands.CrossArms"):            ("CrossArms",            "KI.EmoteStrings.CrossArms"),
    Plasma.PtGetLocalizedString("KI.Commands.Doh"):            ("Doh",            "KI.EmoteStrings.Doh"),
    Plasma.PtGetLocalizedString("KI.Commands.Flinch"):            ("Flinch",            "KI.EmoteStrings.Flinch"),
    Plasma.PtGetLocalizedString("KI.Commands.Kneel"):            ("Kneel",            "KI.EmoteStrings.Kneel"),
    Plasma.PtGetLocalizedString("KI.Commands.LookAround"):            ("LookAround",            "KI.EmoteStrings.LookAround"),
    Plasma.PtGetLocalizedString("KI.Commands.Peer"):            ("Peer",            "KI.EmoteStrings.Peer"),
    Plasma.PtGetLocalizedString("KI.Commands.Salute"):            ("Salute",            "KI.EmoteStrings.Salute"),
    Plasma.PtGetLocalizedString("KI.Commands.ScratchHead"):            ("ScratchHead",            "KI.EmoteStrings.ScratchHead"),
    Plasma.PtGetLocalizedString("KI.Commands.ShakeFist"):            ("ShakeFist",            "KI.EmoteStrings.ShakeFist"),
    Plasma.PtGetLocalizedString("KI.Commands.Shoo"):            ("Shoo",            "KI.EmoteStrings.Shoo"),
    Plasma.PtGetLocalizedString("KI.Commands.SlouchSad"):            ("SlouchSad",            "KI.EmoteStrings.SlouchSad"),
    Plasma.PtGetLocalizedString("KI.Commands.WaveLow"):            ("WaveLow",            "KI.EmoteStrings.WaveLow"),
    Plasma.PtGetLocalizedString("KI.Commands.WaveBye"):            ("Wave",            "KI.EmoteStrings.WaveBye"),
    Plasma.PtGetLocalizedString("KI.Commands.ThumbsDown2"):      ("ThumbsDown2",      "KI.EmoteStrings.ThumbsDown2"),
    Plasma.PtGetLocalizedString("KI.Commands.ThumbsUp2"):        ("ThumbsUp2",        "KI.EmoteStrings.ThumbsUp2"),
    Plasma.PtGetLocalizedString("KI.Commands.Taunt"):        ("Taunt",        "KI.EmoteStrings.Taunt"),
    Plasma.PtGetLocalizedString("KI.Commands.Cough"):        ("Cough",        "KI.EmoteStrings.Cough"),
    Plasma.PtGetLocalizedString("KI.Commands.AskQuestion"):        ("AskQuestion",        "KI.EmoteStrings.AskQuestion"),
    Plasma.PtGetLocalizedString("KI.Commands.Winded"):            ("Winded",            "KI.EmoteStrings.Winded")
}

# xChatSpecialHandledCommands
#  list of the special handled extended chat commands
#
xChatSpecialHandledCommands = [
    Plasma.PtGetLocalizedString("KI.Commands.ChatAllAge"),
    Plasma.PtGetLocalizedString("KI.Commands.ChatReply"),
    Plasma.PtGetLocalizedString("KI.Commands.ChatPrivate"),
    Plasma.PtGetLocalizedString("KI.Commands.ChatNeighbors"),
    Plasma.PtGetLocalizedString("KI.Commands.ChatBuddies"),
]
