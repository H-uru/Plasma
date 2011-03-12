/*==LICENSE==*

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

*==LICENSE==*/
#include "hsWindows.h" // Damn, had to pass in a HWND
#include <map>
class ClassDesc2;
class IParamBlock2;
class plKey;
class plMessage;
class plResponderModifier;

namespace ResponderWait
{
	void SetDesc(ClassDesc2 *desc);

	void InitDlg(IParamBlock2 *curStatePB, int curCmd, HWND hList);

	IParamBlock2 *CreatePB();
	void FixupWaitBlock(IParamBlock2 *waitPB);

	bool ValidateCmdMove(IParamBlock2 *state, int oldIdx, int newIdx);
	void CmdMoved(IParamBlock2 *state, int oldIdx, int newIdx);
	void CmdRemoved(IParamBlock2 *state, int delIdx);
	
	bool		GetWaitOnMe(IParamBlock2* waitPB);
	int			GetWaitingOn(IParamBlock2* waitPB);
	const char*	GetWaitPoint(IParamBlock2* waitPB);
}
