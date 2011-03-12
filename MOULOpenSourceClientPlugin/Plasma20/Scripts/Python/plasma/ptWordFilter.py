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
This module is the word filter to be used...
"""
import string
import re

# Rating levels
xRatedG = 0
xRatedPG = 1
xRatedPG13 = 2
xRatedR = 3
xRatedX = 4

class LanguageFilter:
    def __init__(self):
        pass
    def test(self,sentence):
        "returns censored sentence"
        return xRatedG
    def censor(self,sentence,censorLevel):
        "returns censored sentence"
        return sentence

class ExactMatchListFilter(LanguageFilter):
    def __init__(self,wordlist):
        self.wordlist = wordlist
    def test(self,sentence):
        "return the rating of sentence in question"
        rated = xRatedG     # assume rated lowest level
        startidx = 0
        for endidx in range(len(sentence)):
            if sentence[endidx] in string.whitespace or sentence[endidx] in string.punctuation:
                if startidx != endidx:
                    try:
                        # find and get rating and substitute
                        rating = self.wordlist[string.lower(sentence[startidx:endidx])]
                    except LookupError:
                        # couldn't find word
                        rating = None
                    if rating != None and rating.rating > rated:
                        # substitute into string
                        rated = rating.rating
                startidx = endidx + 1
        if startidx < len(sentence):
            try:
                # find and get rating and substitute
                rating = self.wordlist[string.lower(sentence[startidx:])]
            except LookupError:
                # couldn't find word
                rating = None
            if rating != None and rating.rating > rated:
                # substitute into string
                rated = rating.rating
        return rated
        
    def censor(self,sentence,censorLevel):
        "censors a sentence to a rating"
        # break into words, but perserve original punctuation
        censored = ""
        startidx = 0
        for endidx in range(len(sentence)):
            if sentence[endidx] in string.whitespace or sentence[endidx] in string.punctuation:
                if startidx != endidx:
                    try:
                        # find and get rating and substitute
                        rating = self.wordlist[string.lower(sentence[startidx:endidx])]
                    except LookupError:
                        # couldn't find word
                        rating = None
                    if rating != None and rating.rating > censorLevel:
                        # substitute into string
                        censored += rating.substitute + sentence[endidx]
                    else:
                        censored += sentence[startidx:endidx] + sentence[endidx]
                else:
                    censored += sentence[startidx]
                startidx = endidx + 1
        if startidx < len(sentence):
            # Special after loop processing!
            try:
                # find and get rating and substitute
                rating = self.wordlist[string.lower(sentence[startidx:])]
            except LookupError:
                # couldn't find word
                rating = None
            if rating != None and rating.rating > censorLevel:
                # substitute into string
                censored += rating.substitute
            else:
                censored += sentence[startidx:]
        return censored

class REFilter(LanguageFilter):
    def __init__(self,regexp,rating):
        self.compiledRE = re.compile(regexp, re.IGNORECASE | re.MULTILINE )
        if not isinstance(rating,Rating):
            PtDebugPrint("ptWordFilter: rating for %s not of type Rating" % (regexp))
        self.rating = rating
    def test(self,sentence):
        "return the rating of sentence in question"
        if self.compiledRE.search(sentence) != None:
            return self.rating.rating
        return xRatedG
    def censor(self,sentence,censorLevel):
        "censors a sentence to a rating"
        if self.rating.rating > censorLevel:
            if self.compiledRE.search(sentence) != None:
                return self.compiledRE.sub(self.rating.substitute,sentence)
        return sentence

class Rating:
    "substitute can be string for exact substitute or number of splat replacement"
    def __init__(self,rating,subtitute="*****"):
        self.rating = rating
        self.substitute = subtitute
