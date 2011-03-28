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
This module contains all the strings that need to localized for the Censor
"""

from ptWordFilter import *

xSentenceFilters = [\
    REFilter( r'fuck', Rating(xRatedX) ),\
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
            "poo"          : Rating(xRatedPG),\
            "pooper"          : Rating(xRatedPG),\
            "poopface"          : Rating(xRatedPG),\
            "poophead"          : Rating(xRatedPG),\
            "poopy"          : Rating(xRatedPG),\
            "turd"          : Rating(xRatedPG),\
            "turds"          : Rating(xRatedPG),\
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
            "analprobe"          : Rating(xRatedR),\
            "anal-retentive"          : Rating(xRatedR),\
            "anus"          : Rating(xRatedR),\
            "apeshit"          : Rating(xRatedR),\
            "arse"          : Rating(xRatedR),\
            "arsehole"          : Rating(xRatedR),\
            "ass"          : Rating(xRatedR),\
            "asscheek"          : Rating(xRatedR),\
            "asscheeks"          : Rating(xRatedR),\
            "assface"          : Rating(xRatedR),\
            "asshair"          : Rating(xRatedR),\
            "asshairs"          : Rating(xRatedR),\
            "asshole"          : Rating(xRatedR),\
            "assholes"          : Rating(xRatedR),\
            "asspoo"          : Rating(xRatedR),\
            "asswipe"          : Rating(xRatedR),\
            "ass-wipe"          : Rating(xRatedR),\
            "asswipes"          : Rating(xRatedR),\
            "azz"          : Rating(xRatedR),\
            "azzface"          : Rating(xRatedR),\
            "azzhole"          : Rating(xRatedR),\
            "azzholez"          : Rating(xRatedR),\
            "azzwipe"          : Rating(xRatedR),\
            "b@ll5"          : Rating(xRatedR),\
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
            "b4lls4ck"          : Rating(xRatedR),\
            "b4llz"          : Rating(xRatedR),\
            "ballsack"          : Rating(xRatedR),\
            "ball-sack"          : Rating(xRatedR),\
            "ballz"          : Rating(xRatedR),\
            "bastard"          : Rating(xRatedR),\
            "bastards"          : Rating(xRatedR),\
            "biatch"          : Rating(xRatedR),\
            "biches"          : Rating(xRatedR),\
            "bichez"          : Rating(xRatedR),\
            "bichzz"          : Rating(xRatedR),\
            "bitch"          : Rating(xRatedR),\
            "bitches"          : Rating(xRatedR),\
            "bitchez"          : Rating(xRatedR),\
            "bitchs"          : Rating(xRatedR),\
            "bitchslap"          : Rating(xRatedR),\
            "bitch-slap"          : Rating(xRatedR),\
            "bitch-slapper"          : Rating(xRatedR),\
            "bitchslaps"          : Rating(xRatedR),\
            "bitch-slaps"          : Rating(xRatedR),\
            "bitchz"          : Rating(xRatedR),\
            "blows"          : Rating(xRatedR),\
            "boner"          : Rating(xRatedR),\
            "boners"          : Rating(xRatedR),\
            "bonrz"          : Rating(xRatedR),\
            "boob"          : Rating(xRatedR),\
            "boobies"          : Rating(xRatedR),\
            "boobs"          : Rating(xRatedR),\
            "boobz"          : Rating(xRatedR),\
            "bullshit"          : Rating(xRatedR),\
            "bunghole"          : Rating(xRatedR),\
            "bunghole"          : Rating(xRatedR),\
            "bungholes"          : Rating(xRatedR),\
            "buthole"          : Rating(xRatedR),\
            "butthole"          : Rating(xRatedR),\
            "buttholes"          : Rating(xRatedR),\
            "buttplug"          : Rating(xRatedR),\
            "dickhead"          : Rating(xRatedR),\
            "dickheads"          : Rating(xRatedR),\
            "dicks"          : Rating(xRatedR),\
            "dipshit"          : Rating(xRatedR),\
            "dipshits"          : Rating(xRatedR),\
            "dogshit"          : Rating(xRatedR),\
            "dumbass"          : Rating(xRatedR),\
            "dumbass"          : Rating(xRatedR),\
            "dumbasses"          : Rating(xRatedR),\
            "dumbasses"          : Rating(xRatedR),\
            "dumbazz"          : Rating(xRatedR),\
            "dumbazzes"          : Rating(xRatedR),\
            "dumbshit"          : Rating(xRatedR),\
            "dumbshits"          : Rating(xRatedR),\
            "dumbshitz"          : Rating(xRatedR),\
            "dyke"          : Rating(xRatedR),\
            "dykes"          : Rating(xRatedR),\
            "dykes"          : Rating(xRatedR),\
            "dykez"          : Rating(xRatedR),\
            "f4g"          : Rating(xRatedR),\
            "f4gg0t"          : Rating(xRatedR),\
            "f4gz"          : Rating(xRatedR),\
            "f4rt"          : Rating(xRatedR),\
            "faeces"          : Rating(xRatedR),\
            "fag"          : Rating(xRatedR),\
            "fagg0t"          : Rating(xRatedR),\
            "faggot"          : Rating(xRatedR),\
            "faggots"          : Rating(xRatedR),\
            "fags"          : Rating(xRatedR),\
            "fagz"          : Rating(xRatedR),\
            "farging"          : Rating(xRatedR),\
            "fart"          : Rating(xRatedR),\
            "fartface"          : Rating(xRatedR),\
            "farthead"          : Rating(xRatedR),\
            "g0n4dz"          : Rating(xRatedR),\
            "g0nads"          : Rating(xRatedR),\
            "genital"          : Rating(xRatedR),\
            "genitals"          : Rating(xRatedR),\
            "gethead"          : Rating(xRatedR),\
            "get-laid"          : Rating(xRatedR),\
            "gets-head"          : Rating(xRatedR),\
            "getting-head"          : Rating(xRatedR),\
            "getting-laid"          : Rating(xRatedR),\
            "gon4ds"          : Rating(xRatedR),\
            "gonad"          : Rating(xRatedR),\
            "gonads"          : Rating(xRatedR),\
            "gonadz"          : Rating(xRatedR),\
            "got-laid"          : Rating(xRatedR),\
            "h0m0"          : Rating(xRatedR),\
            "h0mo"          : Rating(xRatedR),\
            "hooker"          : Rating(xRatedR),\
            "horny"          : Rating(xRatedR),\
            "jackass"          : Rating(xRatedR),\
            "jackasses"          : Rating(xRatedR),\
            "jackoff"          : Rating(xRatedR),\
            "jackoffs"          : Rating(xRatedR),\
            "jeckoffs"          : Rating(xRatedR),\
            "jerkhole"          : Rating(xRatedR),\
            "jerkholes"          : Rating(xRatedR),\
            "jerking"          : Rating(xRatedR),\
            "jerkoff"          : Rating(xRatedR),\
            "jerk-off"          : Rating(xRatedR),\
            "jerkwad"          : Rating(xRatedR),\
            "jerkwads"          : Rating(xRatedR),\
            "masturbate"          : Rating(xRatedR),\
            "pigshit"          : Rating(xRatedR),\
            "queef"          : Rating(xRatedR),\
            "queefs"          : Rating(xRatedR),\
            "queer"          : Rating(xRatedR),\
            "queers"          : Rating(xRatedR),\
            "rectum"          : Rating(xRatedR),\
            "scrotum"          : Rating(xRatedR),\
            "shag"          : Rating(xRatedR),\
            "shagadelic"          : Rating(xRatedR),\
            "shagged"          : Rating(xRatedR),\
            "shaggy"          : Rating(xRatedR),\
            "shagscab"          : Rating(xRatedR),\
            "shit"          : Rating(xRatedR),\
            "shitbag"          : Rating(xRatedR),\
            "shitbag"          : Rating(xRatedR),\
            "shite"          : Rating(xRatedR),\
            "shiteater"          : Rating(xRatedR),\
            "shit-eater"          : Rating(xRatedR),\
            "shitface"          : Rating(xRatedR),\
            "shitgivitus"          : Rating(xRatedR),\
            "shithead"          : Rating(xRatedR),\
            "shits"          : Rating(xRatedR),\
            "shitter"          : Rating(xRatedR),\
            "shitty"          : Rating(xRatedR),\
            "slut"          : Rating(xRatedR),\
            "sonofabitch "          : Rating(xRatedR),\
            "tagnut"          : Rating(xRatedR),\
            "tagnuts"          : Rating(xRatedR),\
            "tally-wack"          : Rating(xRatedR),\
            "tally-wacker"          : Rating(xRatedR),\
            "tally-whack"          : Rating(xRatedR),\
            "tally-whacker"          : Rating(xRatedR),\
            "testes"          : Rating(xRatedR),\
            "testicle"          : Rating(xRatedR),\
            "testicles"          : Rating(xRatedR),\
            "tightass"          : Rating(xRatedR),\
            "tit"          : Rating(xRatedR),\
            "tits"          : Rating(xRatedR),\
            "tittie"          : Rating(xRatedR),\
            "titties"          : Rating(xRatedR),\
            "titty"          : Rating(xRatedR),\
            "tity"          : Rating(xRatedR),\
            "twat"          : Rating(xRatedR),\
            "twatface"          : Rating(xRatedR),\
            "twathole"          : Rating(xRatedR),\
            "twats"          : Rating(xRatedR),\
            "vadge"          : Rating(xRatedR),\
            "vaginal"          : Rating(xRatedR),\
            "vaginas"          : Rating(xRatedR),\
            "vaginitis"          : Rating(xRatedR),\
            "vagscab"          : Rating(xRatedR),\
            "wh0r3"          : Rating(xRatedR),\
            "whore"          : Rating(xRatedR),\
            "whoreson"          : Rating(xRatedR),\
            "whoresons"          : Rating(xRatedR),\
            "4ck"          : Rating(xRatedX),\
            "4r53"          : Rating(xRatedX),\
            "asslick"          : Rating(xRatedX),\
            "asslicker"          : Rating(xRatedX),\
            "asslikker"          : Rating(xRatedX),\
            "blowjob"          : Rating(xRatedX),\
            "blowjobber"          : Rating(xRatedX),\
            "blowjobr"          : Rating(xRatedX),\
            "blowjobs"          : Rating(xRatedX),\
            "butlikr"          : Rating(xRatedX),\
            "buttlicker"          : Rating(xRatedX),\
            "buttlikker"          : Rating(xRatedX),\
            "c0ck"          : Rating(xRatedX),\
            "c0k"          : Rating(xRatedX),\
            "c0ksukkr"          : Rating(xRatedX),\
            "clit"          : Rating(xRatedX),\
            "clits"          : Rating(xRatedX),\
            "clitcommander"          : Rating(xRatedX),\
            "clit-commander"          : Rating(xRatedX),\
            "cock"          : Rating(xRatedX),\
            "cocksucker"          : Rating(xRatedX),\
            "cockface"          : Rating(xRatedX),\
            "cockhole"          : Rating(xRatedX),\
            "cocks"          : Rating(xRatedX),\
            "cocksmoker"          : Rating(xRatedX),\
            "cock-sockers"          : Rating(xRatedX),\
            "cock-Sucker"          : Rating(xRatedX),\
            "cocksuckers"          : Rating(xRatedX),\
            "cocktease"          : Rating(xRatedX),\
            "cockteaser"          : Rating(xRatedX),\
            "cokteesr"          : Rating(xRatedX),\
            "cornhole"          : Rating(xRatedX),\
            "cornhole"          : Rating(xRatedX),\
            "cuck"          : Rating(xRatedX),\
            "cum"          : Rating(xRatedX),\
            "dicklicker"          : Rating(xRatedX),\
            "dicklickers"          : Rating(xRatedX),\
            "dicksmoker"          : Rating(xRatedX),\
            "dicksmokers"          : Rating(xRatedX),\
            "dildo"          : Rating(xRatedX),\
            "dildoes"          : Rating(xRatedX),\
            "dildos"          : Rating(xRatedX),\
            "effing"          : Rating(xRatedX),\
            "fellatio"          : Rating(xRatedX),\
            "fisting"          : Rating(xRatedX),\
            "fudgepacker"          : Rating(xRatedX),\
            "fudgina"          : Rating(xRatedX),\
            "fugly"          : Rating(xRatedX),\
            "fukface"          : Rating(xRatedX),\
            "fukhead"          : Rating(xRatedX),\
            "fuking"          : Rating(xRatedX),\
            "fukyu"          : Rating(xRatedX),\
            "fuque"          : Rating(xRatedX),\
            "furburger"          : Rating(xRatedX),\
            "fur-burger"          : Rating(xRatedX),\
            "furk"          : Rating(xRatedX),\
            "furpie"          : Rating(xRatedX),\
            "fur-pie"          : Rating(xRatedX),\
            "gangbang"          : Rating(xRatedX),\
            "gang-bang"          : Rating(xRatedX),\
            "hairpie"          : Rating(xRatedX),\
            "hair-pie"          : Rating(xRatedX),\
            "handjob"          : Rating(xRatedX),\
            "hand-job"          : Rating(xRatedX),\
            "handjobs"          : Rating(xRatedX),\
            "hand-jobs"          : Rating(xRatedX),\
            "hardon"          : Rating(xRatedX),\
            "hard-on"          : Rating(xRatedX),\
            "headgiver"          : Rating(xRatedX),\
            "headgiving"          : Rating(xRatedX),\
            "head-giver"          : Rating(xRatedX),\
            "labia"          : Rating(xRatedX),\
            "lapdance"          : Rating(xRatedX),\
            "m0f0"          : Rating(xRatedX),\
            "m0fo"          : Rating(xRatedX),\
            "mofo"          : Rating(xRatedX),\
            "mofO"          : Rating(xRatedX),\
            "niggah"          : Rating(xRatedX),\
            "niggaz"          : Rating(xRatedX),\
            "nigger"          : Rating(xRatedX),\
            "niggers"          : Rating(xRatedX),\
            "niggr"          : Rating(xRatedX),\
            "nutlicker"          : Rating(xRatedX),\
            "pecker"          : Rating(xRatedX),\
            "pederast"          : Rating(xRatedX),\
            "pederaste"          : Rating(xRatedX),\
            "penile"          : Rating(xRatedX),\
            "penis"          : Rating(xRatedX),\
            "prick"          : Rating(xRatedX),\
            "push-shit"          : Rating(xRatedX),\
            "pussy"          : Rating(xRatedX),\
            "pussy"          : Rating(xRatedX),\
            "pussyfart"          : Rating(xRatedX),\
            "pussy-fart"          : Rating(xRatedX),\
            "putz"          : Rating(xRatedX),\
            "puzzy"          : Rating(xRatedX),\
            "rimjob"          : Rating(xRatedX),\
            "sillypussy"          : Rating(xRatedX),\
            "spick"          : Rating(xRatedX),\
            "w4nker"          : Rating(xRatedX),\
            "w4nkerz"          : Rating(xRatedX),\
            "w4nkr"          : Rating(xRatedX),\
            "w4nkrr"          : Rating(xRatedX),\
            "wank"          : Rating(xRatedX),\
            "wanker"          : Rating(xRatedX),\
            "wankers"          : Rating(xRatedX),\
            "wankerz"          : Rating(xRatedX),\
            "wanking"          : Rating(xRatedX),\
            "wop"          : Rating(xRatedX)\
        }\
    )\
]