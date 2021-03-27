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

from __future__ import annotations
from dataclasses import dataclass
import re
from typing import Union
import webbrowser

from Plasma import *
from PlasmaConstants import *
from PlasmaTypes import *

from xCensor import xCensor

_HOST_REGEX = re.compile(R"https?:\/\/(?:www\.)?([^\/]+).*", re.IGNORECASE)
_URL_REGEX = re.compile(R"https?:\/\/[\S]+", re.IGNORECASE)

@dataclass
class _Link:
    url: str
    pos: int = 0


class xGUILinkHandler:
    def __init__(self, control: ptGUIControlMultiLineEdit):
        self._control = control
        self._lastColor = control.getForeColor()
        self._linkColor = ptColor().blue()
        self._nextLink = 0
        self._links = {}

        # Cheap inheritance
        for i in dir(control):
            if not hasattr(self, i):
                setattr(self, i, getattr(control, i))

    def __getitem__(self, index: int) -> str:
        if index in self._links:
            return self._links[index].url
        else:
            raise KeyError(index)

    def getCurrentURL(self) -> Union[None, str]:
        linkId = self._control.getCurrentLink()
        if linkId is None:
            PtDebugPrint("xGUILinkHandler.getCurrentURL():\tNo active link", level=kWarningLevel)
            return None
        try:
            link = self._links[linkId]
        except LookupError:
            PtDebugPrint(f"xGUILinkHandler.getCurrentURL():\tUnknown linkId {linkId}")
            return None
        else:
            return link.url

    @property
    def linkColor(self) -> ptColor:
        return self._linkColor

    @linkColor.setter
    def linkColor(self, value: ptColor):
        self._linkColor = value

    def openLink(self, prompt: bool = True) -> None:
        url = self.getCurrentURL()
        if url is None:
            PtDebugPrint("xGUILinkHandler.openLink():\tNo active link to open!")
            return

        if prompt:
            # Unparse the current URL as a hostname for prompting.
            hostname = _HOST_REGEX.sub(R"\1", url)

            def boom(result: int):
                if result == PtConfirmationResult.Yes:
                    PtDebugPrint(f"xGUILinkHandler.openLink():\tLink OK to open: {url}", level=kWarningLevel)
                    webbrowser.open_new_tab(url)
            PtLocalizedYesNoDialog(boom, "KI.Messages.OpenHyperlink", hostname)
        else:
            PtDebugPrint(f"xGUILinkHandler.openLink():\tUnconditional URL open: {url}")
            webbrowser.open_new_tab(url)

    # ======================================
    # ptGUIControlMultiLineEdit "overrides"
    # ======================================

    def clearBuffer(self) -> None:
        self._lastColor = self._control.getForeColor()
        self._nextLink = 0
        self._links.clear()
        self._control.clearBuffer()

    def deleteLinesFromTop(self, lines: int) -> None:
        oldBufferLen = self._control.getBufferSize()
        self._control.deleteLinesFromTop(lines)
        diff = self._control.getBufferSize() - oldBufferLen
        for linkId in list(self._links):
            self._links[linkId].pos -= diff
            if self._links[linkId].pos < 0:
                del self._links[linkId]

    def insertColor(self, color: ptColor) -> None:
        self._lastColor = color
        self._control.insertColor(color)

    def insertString(self, text: str, /, *, censorLevel: Union[int, None] = None, urlDetection: bool = True) -> None:
        lastInsert = 0
        control, linkColor, normalColor = self._control, self._linkColor, self._lastColor

        if censorLevel is None:
            censor = lambda x: x
        else:
            censor = lambda x: xCensor(x, censorLevel)

        if urlDetection:
            with PtBeginGUIUpdate(control):
                urls = ((i.span(), i[0]) for i in _URL_REGEX.finditer(text))
                for (start, end), url in urls:
                    if start > lastInsert:
                        control.insertStringW(censor(text[lastInsert:start]))
                    lastInsert = end
                    control.insertColor(linkColor)

                    self._links[self._nextLink] = _Link(url, control.getCursor())
                    control.insertLink(self._nextLink)
                    PtDebugPrint(f"xGUILinkHandler.insertString():\tInserting URL {url} as linkId {self._nextLink}", level=kDebugDumpLevel)
                    self._nextLink += 1

                    control.insertStringW(censor(url))
                    control.clearLink()
                    control.insertColor(normalColor)
                if lastInsert != len(text):
                    control.insertStringW(censor(text[lastInsert:]))
        else:
            control.insertStringW(censor(text))

    def insertStringW(self, text: str, /, *, censorLevel: Union[int, None] = None, urlDetection: bool = True) -> None:
        self.insertString(text, censorLevel=censorLevel, urlDetection=urlDetection)

    def setString(self, text: str, /, *, censorLevel: Union[int, None] = None, urlDetection: bool = True) -> None:
        with PtBeginGUIUpdate(self._control):
            self.clearBuffer()
            self.insertString(text, censorLevel=censorLevel, urlDetection=urlDetection)
            # Emulate the real behavior of setString() - this to ensure we don't get duplicate
            # cursors if the string is reset while the control is being focused (eg text notes).
            self._control.moveCursor(PtGUIMultiLineDirection.kBufferStart)

    def setStringW(self, text: str, /, *, censorLevel: Union[int, None] = None, urlDetection: bool = True) -> None:
        self.setString(text, censorLevel=censorLevel, urlDetection=urlDetection)
