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
"""
This module contains all the strings that need to localized for the Censor
"""

from ptWordFilter import *

xSentenceFilters = [\
    REFilter( r'fuck', Rating(xRatedX) ),\
    REFilter( r'fuuck', Rating(xRatedX) ),\
    REFilter( r'fuk', Rating(xRatedX) ),\
    REFilter( r'f.u.c.k', Rating(xRatedX) ),\
    REFilter( r'fukk', Rating(xRatedX) ),\
    REFilter( r'phuck', Rating(xRatedX) ),\
    REFilter( r'phuker', Rating(xRatedX) ),\
    REFilter( r'clitoris', Rating(xRatedX) ),\
    REFilter( r'vagina', Rating(xRatedX) ),\
    REFilter( r'cocksucker', Rating(xRatedX) ),\
    REFilter( r'cunt', Rating(xRatedX) ),\
    ExactMatchListFilter(\
        {
            "@$$"          : Rating(xRatedR),\
            "@$$hole"          : Rating(xRatedR),\
            "@55"          : Rating(xRatedR),\
            "@55#0l3"          : Rating(xRatedR),\
            "@55#0l35"          : Rating(xRatedR),\
            "@55h0l3"          : Rating(xRatedR),\
            "@nu5"          : Rating(xRatedR),\
            "@nus"          : Rating(xRatedR),\
            "@r53"          : Rating(xRatedR),\
            "@r53#0l3"          : Rating(xRatedR),\
            "@r53h0l3"          : Rating(xRatedR),\
            "@r5e"          : Rating(xRatedR),\
            "@r5eh0le"          : Rating(xRatedR),\
            "@rse"          : Rating(xRatedR),\
            "@rseh0le"          : Rating(xRatedR),\
            "@ss"          : Rating(xRatedR),\
            "@ssf@ce"          : Rating(xRatedR),\
            "@ssh0le"          : Rating(xRatedR),\
            "@zz"          : Rating(xRatedR),\
            "4ck"          : Rating(xRatedX),\
            "4r5e"          : Rating(xRatedR),\
            "4r53"          : Rating(xRatedR),\
            "5h1t"          : Rating(xRatedR),\
            "5hit"          : Rating(xRatedR),\
            "a$$"          : Rating(xRatedR),\
            "a$$hole"          : Rating(xRatedR),\
            "a55"          : Rating(xRatedR),\
            "analprobe"          : Rating(xRatedR),\
            "anal-retentive"          : Rating(xRatedR),\
            "anus"          : Rating(xRatedR),\
            "apeshit"          : Rating(xRatedR),\
            "ar5e"          : Rating(xRatedR),\
            "arse"          : Rating(xRatedR),\
            "arrse"          : Rating(xRatedR),\
            "arsehole"          : Rating(xRatedR),\
            "ass"          : Rating(xRatedR),\
            "asscheek"          : Rating(xRatedR),\
            "asscheeks"          : Rating(xRatedR),\
            "assface"          : Rating(xRatedR),\
            "asshair"          : Rating(xRatedR),\
            "asshairs"          : Rating(xRatedR),\
            "asshole"          : Rating(xRatedR),\
            "assholes"          : Rating(xRatedR),\
            "assjockey"          : Rating(xRatedX),\
            "asslick"          : Rating(xRatedX),\
            "asslicker"          : Rating(xRatedX),\
            "asslikker"          : Rating(xRatedX),\
            "assmuncher"          : Rating(xRatedX),\
            "asspacker"          : Rating(xRatedX),\
            "asspoo"          : Rating(xRatedR),\
            "assrammer"          : Rating(xRatedR),\
            "asswhole"          : Rating(xRatedR),\
            "asswhore"          : Rating(xRatedR),\
            "asswipe"          : Rating(xRatedR),\
            "ass-wipe"          : Rating(xRatedR),\
            "asswipes"          : Rating(xRatedR),\
            "azz"          : Rating(xRatedR),\
            "azzface"          : Rating(xRatedR),\
            "azzhole"          : Rating(xRatedR),\
            "azzholez"          : Rating(xRatedR),\
            "azzwipe"          : Rating(xRatedR),\
            "b!tch"          : Rating(xRatedR),\
            "b@ll5"          : Rating(xRatedR),\
            "b000b"          : Rating(xRatedR),\
            "b000bs"          : Rating(xRatedR),\
            "b00b"          : Rating(xRatedR),\
            "b00b5"          : Rating(xRatedR),\
            "b00beez"          : Rating(xRatedR),\
            "b00bies"          : Rating(xRatedR),\
            "b00bs"          : Rating(xRatedR),\
            "b00bz"          : Rating(xRatedR),\
            "b0ner"          : Rating(xRatedR),\
            "b0nerz"          : Rating(xRatedR),\
            "b1@tc#"          : Rating(xRatedR),\
            "b1@tch"          : Rating(xRatedR),\
            "b1ch"          : Rating(xRatedR),\
            "b17ch"          : Rating(xRatedR),\
            "b1tch"          : Rating(xRatedR),\
            "b4lls4ck"          : Rating(xRatedR),\
            "b4llz"          : Rating(xRatedR),\
            "ballsack"          : Rating(xRatedR),\
            "ball-sack"          : Rating(xRatedR),\
            "ballz"          : Rating(xRatedR),\
            "bastard"          : Rating(xRatedR),\
            "bastards"          : Rating(xRatedR),\
            "bi+ch"          : Rating(xRatedR),\
            "biatch"          : Rating(xRatedR),\
            "biches"          : Rating(xRatedR),\
            "bichez"          : Rating(xRatedR),\
            "bichzz"          : Rating(xRatedR),\
            "bitch"          : Rating(xRatedR),\
            "bitches"          : Rating(xRatedR),\
            "bitchez"          : Rating(xRatedR),\
            "bitchin"          : Rating(xRatedPG),\
            "bitchs"          : Rating(xRatedR),\
            "bitchslap"          : Rating(xRatedR),\
            "bitch-slap"          : Rating(xRatedR),\
            "bitch-slapper"          : Rating(xRatedR),\
            "bitchslaps"          : Rating(xRatedR),\
            "bitch-slaps"          : Rating(xRatedR),\
            "bitchy"          : Rating(xRatedPG),\
            "bitchz"          : Rating(xRatedR),\
            "blowjob"          : Rating(xRatedX),\
            "blowjobber"          : Rating(xRatedX),\
            "blowjobr"          : Rating(xRatedX),\
            "blowjobs"          : Rating(xRatedX),\
            "bollocks"          : Rating(xRatedR),\
            "bollox"          : Rating(xRatedR),\
            "boner"          : Rating(xRatedR),\
            "boners"          : Rating(xRatedR),\
            "bonrz"          : Rating(xRatedR),\
            "boob"          : Rating(xRatedR),\
            "boobies"          : Rating(xRatedR),\
            "boobs"          : Rating(xRatedR),\
            "boobz"          : Rating(xRatedR),\
            "booob"          : Rating(xRatedR),\
            "booobs"          : Rating(xRatedR),\
            "bullcrap"          : Rating(xRatedPG),\
            "bullshit"          : Rating(xRatedR),\
            "bunghole"          : Rating(xRatedR),\
            "bungholes"          : Rating(xRatedR),\
            "buthole"          : Rating(xRatedR),\
            "butlikr"          : Rating(xRatedX),\
            "buttbang"          : Rating(xRatedX),\
            "butt-bang"          : Rating(xRatedX),\
            "buttbanger"          : Rating(xRatedX),\
            "butthole"          : Rating(xRatedR),\
            "buttholes"          : Rating(xRatedR),\
            "buttlicker"          : Rating(xRatedX),\
            "buttlikker"          : Rating(xRatedX),\
            "buttmunch"          : Rating(xRatedPG),\
            "buttpirate"          : Rating(xRatedR),\
            "buttplug"          : Rating(xRatedR),\
            "c0ck"          : Rating(xRatedX),\
            "c0k"          : Rating(xRatedX),\
            "c0ksukkr"          : Rating(xRatedX),\
            "cawk"          : Rating(xRatedX),\
            "chink"          : Rating(xRatedX),\
            "cl1t"          : Rating(xRatedX),\
            "clit"          : Rating(xRatedX),\
            "clitcommander"          : Rating(xRatedX),\
            "clit-commander"          : Rating(xRatedX),\
            "clits"          : Rating(xRatedX),\
            "cnut"          : Rating(xRatedX),\
            "cock"          : Rating(xRatedX),\
            "cockface"          : Rating(xRatedX),\
            "cockhead"          : Rating(xRatedX),\
            "cockhole"          : Rating(xRatedX),\
            "cocklicker"          : Rating(xRatedX),\
            "cockrider"          : Rating(xRatedX),\
            "cocks"          : Rating(xRatedX),\
            "cocksmoker"          : Rating(xRatedX),\
            "cocksocker"          : Rating(xRatedX),\
            "cock-socker"          : Rating(xRatedX),\
            "cocksockers"          : Rating(xRatedX),\
            "cocksucka"          : Rating(xRatedX),\
            "cock-sucker"          : Rating(xRatedX),\
            "cocksuckers"          : Rating(xRatedX),\
            "cocksukka"          : Rating(xRatedX),\
            "cocktease"          : Rating(xRatedX),\
            "cockteaser"          : Rating(xRatedX),\
            "cokteesr"          : Rating(xRatedX),\
            "cooch"          : Rating(xRatedX),\
            "coochie"          : Rating(xRatedX),\
            "cooter"          : Rating(xRatedX),\
            "cornhole"          : Rating(xRatedR),\
            "crackwhore"          : Rating(xRatedR),\
            "cuck"          : Rating(xRatedX),\
            "cum"          : Rating(xRatedX),\
            "cunnilingus"          : Rating(xRatedX),\
            "d1ck"          : Rating(xRatedR),\
            "dago"          : Rating(xRatedX),\
            "datnigga"          : Rating(xRatedX),\
            "dego"          : Rating(xRatedX),\
            "dickhead"          : Rating(xRatedR),\
            "dickheads"          : Rating(xRatedR),\
            "dicklicker"          : Rating(xRatedX),\
            "dicklickers"          : Rating(xRatedX),\
            "dicks"          : Rating(xRatedR),\
            "dicksmoker"          : Rating(xRatedX),\
            "dicksmokers"          : Rating(xRatedX),\
            "dickwad"          : Rating(xRatedX),\
            "dickweed"          : Rating(xRatedX),\
            "dildo"          : Rating(xRatedX),\
            "dildoes"          : Rating(xRatedX),\
            "dildos"          : Rating(xRatedX),\
            "dipshit"          : Rating(xRatedR),\
            "dipshits"          : Rating(xRatedR),\
            "dlck"          : Rating(xRatedR),\
            "dogshit"          : Rating(xRatedR),\
            "doggiestyle"          : Rating(xRatedX),\
            "dripdick"          : Rating(xRatedX),\
            "dumb@ss"          : Rating(xRatedR),\
            "dumbass"          : Rating(xRatedR),\
            "dumb@sses"          : Rating(xRatedR),\
            "dumbasses"          : Rating(xRatedR),\
            "dumbazz"          : Rating(xRatedR),\
            "dumbazzes"          : Rating(xRatedR),\
            "dumbbitch"          : Rating(xRatedR),\
            "dumbshit"          : Rating(xRatedR),\
            "dumbshits"          : Rating(xRatedR),\
            "dumbshitz"          : Rating(xRatedR),\
            "dyke"          : Rating(xRatedR),\
            "dykes"          : Rating(xRatedR),\
            "dykez"          : Rating(xRatedR),\
            "effing"          : Rating(xRatedX),\
            "f4g"          : Rating(xRatedR),\
            "f4gg0t"          : Rating(xRatedR),\
            "f4gz"          : Rating(xRatedR),\
            "f4rt"          : Rating(xRatedR),\
            "faeces"          : Rating(xRatedR),\
            "fag"          : Rating(xRatedR),\
            "fagg0t"          : Rating(xRatedR),\
            "faggit"          : Rating(xRatedR),\
            "faggitt"          : Rating(xRatedR),\
            "faggot"          : Rating(xRatedR),\
            "faggots"          : Rating(xRatedR),\
            "fagot"          : Rating(xRatedR),\
            "fags"          : Rating(xRatedR),\
            "fagz"          : Rating(xRatedR),\
            "farging"          : Rating(xRatedR),\
            "fart"          : Rating(xRatedR),\
            "fartface"          : Rating(xRatedR),\
            "farthead"          : Rating(xRatedR),\
            "fatass"          : Rating(xRatedR),\
            "fcuk"          : Rating(xRatedX),\
            "felcher"          : Rating(xRatedX),\
            "fellatio"          : Rating(xRatedX),\
            "fisting"          : Rating(xRatedX),\
            "fudgepacker"          : Rating(xRatedX),\
            "fudgina"          : Rating(xRatedX),\
            "fugly"          : Rating(xRatedX),\
            "fuque"          : Rating(xRatedX),\
            "furburger"          : Rating(xRatedX),\
            "fur-burger"          : Rating(xRatedX),\
            "furk"          : Rating(xRatedX),\
            "furpie"          : Rating(xRatedX),\
            "fur-pie"          : Rating(xRatedX),\
            "fux"          : Rating(xRatedX),\
            "g0n4dz"          : Rating(xRatedR),\
            "g0nads"          : Rating(xRatedR),\
            "gangbang"          : Rating(xRatedX),\
            "gang-bang"          : Rating(xRatedX),\
            "genital"          : Rating(xRatedR),\
            "genitals"          : Rating(xRatedR),\
            "gethead"          : Rating(xRatedR),\
            "get-laid"          : Rating(xRatedR),\
            "gets-head"          : Rating(xRatedR),\
            "getting-head"          : Rating(xRatedR),\
            "getting-laid"          : Rating(xRatedR),\
            "givehead"          : Rating(xRatedR),\
            "goddamn"          : Rating(xRatedR),\
            "goddamned"          : Rating(xRatedR),\
            "gon4ds"          : Rating(xRatedR),\
            "gonad"          : Rating(xRatedR),\
            "gonads"          : Rating(xRatedR),\
            "gonadz"          : Rating(xRatedR),\
            "gook"          : Rating(xRatedX),\
            "got-laid"          : Rating(xRatedR),\
            "h0m0"          : Rating(xRatedR),\
            "h0mo"          : Rating(xRatedR),\
            "hairpie"          : Rating(xRatedX),\
            "hair-pie"          : Rating(xRatedX),\
            "handjob"          : Rating(xRatedX),\
            "hand-job"          : Rating(xRatedX),\
            "handjobs"          : Rating(xRatedX),\
            "hand-jobs"          : Rating(xRatedX),\
            "hardon"          : Rating(xRatedX),\
            "hard-on"          : Rating(xRatedX),\
            "headgiver"          : Rating(xRatedX),\
            "head-giver"          : Rating(xRatedX),\
            "headgiving"          : Rating(xRatedX),\
            "holestuffer"          : Rating(xRatedX),\
            "homo"          : Rating(xRatedR),\
            "horny"          : Rating(xRatedR),\
            "horseshit"          : Rating(xRatedR),\
            "hotdamn"          : Rating(xRatedPG),\
            "jackass"          : Rating(xRatedR),\
            "jackasses"          : Rating(xRatedR),\
            "jackoff"          : Rating(xRatedR),\
            "jackoffs"          : Rating(xRatedR),\
            "jap"          : Rating(xRatedX),\
            "jeckoffs"          : Rating(xRatedR),\
            "jerkhole"          : Rating(xRatedR),\
            "jerkholes"          : Rating(xRatedR),\
            "jerking"          : Rating(xRatedR),\
            "jerkoff"          : Rating(xRatedR),\
            "jerk-off"          : Rating(xRatedR),\
            "jerkwad"          : Rating(xRatedR),\
            "jerkwads"          : Rating(xRatedR),\
            "jism"          : Rating(xRatedX),\
            "jiz"          : Rating(xRatedX),\
            "jizz"          : Rating(xRatedX),\
            "kawk"          : Rating(xRatedX),\
            "kike"          : Rating(xRatedX),\
            "koon"          : Rating(xRatedX),\
            "kunt"          : Rating(xRatedX),\
            "l3i+ch"          : Rating(xRatedR),\
            "l3itch"          : Rating(xRatedR),\
            "labia"          : Rating(xRatedX),\
            "lapdance"          : Rating(xRatedX),\
            "lesbo"          : Rating(xRatedR),\
            "lezbo"          : Rating(xRatedR),\
            "limpdick"          : Rating(xRatedR),\
            "m0f0"          : Rating(xRatedX),\
            "m0fo"          : Rating(xRatedX),\
            "m45turbate"          : Rating(xRatedR),\
            "m4sturbate"          : Rating(xRatedR),\
            "masturb8"          : Rating(xRatedR),\
            "masturbate"          : Rating(xRatedR),\
            "masturbates"          : Rating(xRatedR),\
            "mof0"          : Rating(xRatedX),\
            "mofo"          : Rating(xRatedX),\
            "moulie"          : Rating(xRatedX),\
            "muffdiver"          : Rating(xRatedX),\
            "n1gga"          : Rating(xRatedX),\
            "n1ggah"          : Rating(xRatedX),\
            "n1gger"          : Rating(xRatedX),\
            "nigga"          : Rating(xRatedX),\
            "niggah"          : Rating(xRatedX),\
            "niggas"          : Rating(xRatedX),\
            "niggaz"          : Rating(xRatedX),\
            "nigger"          : Rating(xRatedX),\
            "niggers"          : Rating(xRatedX),\
            "niggerz"          : Rating(xRatedX),\
            "niggr"          : Rating(xRatedX),\
            "niggrz"          : Rating(xRatedX),\
            "nutlicker"          : Rating(xRatedX),\
            "nutsack"          : Rating(xRatedX),\
            "pecker"          : Rating(xRatedX),\
            "pederast"          : Rating(xRatedX),\
            "pederaste"          : Rating(xRatedX),\
            "penile"          : Rating(xRatedX),\
            "penis"          : Rating(xRatedX),\
            "pigshit"          : Rating(xRatedR),\
            "piss"          : Rating(xRatedR),\
            "pissflaps"          : Rating(xRatedX),\
            "porchmonkey"          : Rating(xRatedX),\
            "poo"          : Rating(xRatedPG),\
            "poon"          : Rating(xRatedX),\
            "poontang"          : Rating(xRatedX),\
            "pooper"          : Rating(xRatedPG),\
            "poopface"          : Rating(xRatedPG),\
            "poophead"          : Rating(xRatedPG),\
            "poopy"          : Rating(xRatedPG),\
            "prick"          : Rating(xRatedX),\
            "prickhead"          : Rating(xRatedX),\
            "push-shit"          : Rating(xRatedX),\
            "pusse"          : Rating(xRatedX),\
            "pussi"          : Rating(xRatedX),\
            "pussy"          : Rating(xRatedX),\
            "pussyfart"          : Rating(xRatedX),\
            "pussy-fart"          : Rating(xRatedX),\
            "pussylips"          : Rating(xRatedX),\
            "pussy-lips"          : Rating(xRatedX),\
            "putz"          : Rating(xRatedX),\
            "puzzy"          : Rating(xRatedX),\
            "queef"          : Rating(xRatedR),\
            "queefs"          : Rating(xRatedR),\
            "queer"          : Rating(xRatedR),\
            "queers"          : Rating(xRatedR),\
            "queerz"          : Rating(xRatedR),\
            "quim"          : Rating(xRatedR),\
            "rectum"          : Rating(xRatedR),\
            "retard"          : Rating(xRatedR),\
            "rimjob"          : Rating(xRatedX),\
            "rimming"          : Rating(xRatedX),\
            "schlong"          : Rating(xRatedX),\
            "schmuck"          : Rating(xRatedR),\
            "scrotum"          : Rating(xRatedR),\
            "sh!+"          : Rating(xRatedR),\
            "sh!t"          : Rating(xRatedR),\
            "sh1t"          : Rating(xRatedR),\
            "shag"          : Rating(xRatedR),\
            "shagadelic"          : Rating(xRatedR),\
            "shagged"          : Rating(xRatedR),\
            "shi+"          : Rating(xRatedR),\
            "shit"          : Rating(xRatedR),\
            "shit4brains"          : Rating(xRatedR),\
            "shitb@g"          : Rating(xRatedR),\
            "shitbag"          : Rating(xRatedR),\
            "shite"          : Rating(xRatedR),\
            "shiteater"          : Rating(xRatedR),\
            "shit-eater"          : Rating(xRatedR),\
            "shitface"          : Rating(xRatedR),\
            "shitfaced"          : Rating(xRatedR),\
            "shitforbrains"          : Rating(xRatedR),\
            "shitgivitus"          : Rating(xRatedR),\
            "shithead"          : Rating(xRatedR),\
            "shitoutofluck"          : Rating(xRatedR),\
            "shits"          : Rating(xRatedR),\
            "shitstain"          : Rating(xRatedR),\
            "shitter"          : Rating(xRatedR),\
            "shitty"          : Rating(xRatedR),\
            "sillypussy"          : Rating(xRatedX),\
            "skinflute"          : Rating(xRatedX),\
            "slut"          : Rating(xRatedR),\
            "sluts"          : Rating(xRatedR),\
            "slutz"          : Rating(xRatedR),\
            "sonofabitch "          : Rating(xRatedR),\
            "spic"          : Rating(xRatedX),\
            "spick"          : Rating(xRatedX),\
            "splooge"          : Rating(xRatedX),\
            "suckdick"          : Rating(xRatedX),\
            "suckoff"          : Rating(xRatedX),\
            "t1tt1es"          : Rating(xRatedR),\
            "tagnut"          : Rating(xRatedR),\
            "tagnuts"          : Rating(xRatedR),\
            "tallywack"          : Rating(xRatedR),\
            "tally-wack"          : Rating(xRatedR),\
            "tallywacker"          : Rating(xRatedR),\
            "tally-wacker"          : Rating(xRatedR),\
            "tallywhack"          : Rating(xRatedR),\
            "tally-whack"          : Rating(xRatedR),\
            "tallywhacker"          : Rating(xRatedR),\
            "tally-whacker"          : Rating(xRatedR),\
            "teez"          : Rating(xRatedR),\
            "testes"          : Rating(xRatedR),\
            "testicle"          : Rating(xRatedR),\
            "testicles"          : Rating(xRatedR),\
            "tightass"          : Rating(xRatedR),\
            "tit"          : Rating(xRatedR),\
            "tits"          : Rating(xRatedR),\
            "titt"          : Rating(xRatedR),\
            "tittie"          : Rating(xRatedR),\
            "tittie5"          : Rating(xRatedR),\
            "titties"          : Rating(xRatedR),\
            "titty"          : Rating(xRatedR),\
            "tittywank"          : Rating(xRatedX),\
            "tittywanker"          : Rating(xRatedX),\
            "tity"          : Rating(xRatedR),\
            "turd"          : Rating(xRatedPG),\
            "turds"          : Rating(xRatedPG),\
            "tw4t"          : Rating(xRatedR),\
            "twat"          : Rating(xRatedR),\
            "twatface"          : Rating(xRatedR),\
            "twathead"          : Rating(xRatedR),\
            "twathole"          : Rating(xRatedR),\
            "twats"          : Rating(xRatedR),\
            "twunt"          : Rating(xRatedR),\
            "vadge"          : Rating(xRatedR),\
            "vaginal"          : Rating(xRatedR),\
            "vaginas"          : Rating(xRatedR),\
            "vaginitis"          : Rating(xRatedR),\
            "vagscab"          : Rating(xRatedR),\
            "w4nker"          : Rating(xRatedX),\
            "w4nkerz"          : Rating(xRatedX),\
            "w4nkr"          : Rating(xRatedX),\
            "w4nkrr"          : Rating(xRatedX),\
            "wank"          : Rating(xRatedX),\
            "wanker"          : Rating(xRatedX),\
            "wankers"          : Rating(xRatedX),\
            "wankerz"          : Rating(xRatedX),\
            "wanking"          : Rating(xRatedX),\
            "wetback"          : Rating(xRatedX),\
            "wh0r3"          : Rating(xRatedR),\
            "whore"          : Rating(xRatedR),\
            "whoreson"          : Rating(xRatedR),\
            "whoresons"          : Rating(xRatedR),\
            "wop"          : Rating(xRatedX),\
        }\
    )\
]
