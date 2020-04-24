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
This module will censor a sentence, depending on censorLevel
"""
from ptWordFilter import *

import xCensorFilters

SpecialPunctuation = '#$%&*+-@_|~'

#
# Add filters and words to be censored in the SentenceFilters list below:
#
# Use the REFilter(regularExpression,rating) for words that need extra special handling.
#   Each of these filters are run against the entire sentence each time... so, use
#   sparingly (ie. for the really bad words). The regularExpression uses the same
#   format as the re function in python.
# Use ExactMatchListFilter for words that are exact matches (but case insensitive)
#  and do not include any white space or special characters. This filter will split
#  the sentence into words and do a quick match through the list.
#
# The Rating class takes a rating and a substitution string. If there is no substitution string
#   then a default string is used ('****')

SentenceFilters = xCensorFilters.xSentenceFilters

def xCensor(sentence,censorLevel):
    "censors sentence for words for above ratingLevel. Returns censored sentence"
    # make sure they stay within reasonable censorLevels
    if censorLevel > xRatedR:
        censorLevel = xRatedR
    for sfilter in SentenceFilters:
        sentence = sfilter.censor(sentence,censorLevel)
    return sentence

def xWhatRating(sentence):
    "Returns the censorship level of a sentence"
    rated = xRatedG   # assume rated lowest level
    for sfilter in SentenceFilters:
        thisRating = sfilter.test(sentence)
        if thisRating > rated:
            rated = thisRating
    return rated
