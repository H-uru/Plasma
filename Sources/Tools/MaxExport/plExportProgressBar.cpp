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

*==LICENSE==*/
#include "HeadSpin.h"

#include "MaxMain/MaxAPI.h"

#include "plExportProgressBar.h"

namespace {
    DWORD WINAPI ProgressDummyFunc(LPVOID arg) 
    {
        return(0);
    }
}

plExportProgressBar::plExportProgressBar() :
    fTotalSteps(0),
    fCurStep(0)
{
    fInterface = GetCOREInterface();
}

plExportProgressBar::~plExportProgressBar()
{
   fInterface->ProgressEnd();
}

void plExportProgressBar::Start(char *name, uint32_t steps)
{
    fTotalSteps = steps;
    fCurStep = 0;

    fInterface->ProgressEnd();
    fInterface->ProgressStart(name, TRUE, ProgressDummyFunc, nullptr);
   
   GUP* exportServerGup = OpenGupPlugIn(Class_ID(470000004,99));
   if(exportServerGup && name)
   {
      exportServerGup->Control(-3); // means next control will be progress task
      exportServerGup->Control((DWORD)name);
   }   
}

bool plExportProgressBar::Update(MAX14_CONST MCHAR* name, uint32_t inc)
{
    fCurStep += inc;

   // Check to see if we are running an export server
   // If so, then pass the update on to the export server
   GUP* exportServerGup = OpenGupPlugIn(Class_ID(470000004,99));
   if(exportServerGup)
   {
      exportServerGup->Control(-2);  // means next control will be progress pct
      exportServerGup->Control((int)((fCurStep * 100) / fTotalSteps));  // send pct
      if(name)
      {
         exportServerGup->Control(-4); // means next control will be progress subtask
         exportServerGup->Control((DWORD)name);
      }
      exportServerGup->Control(-6); // signal that we're done sending this update sequence
   }   
   
   fInterface->ProgressUpdate((int)((fCurStep * 100) / fTotalSteps), FALSE, name);

    if (fInterface->GetCancel()) 
    {
        int retval = MessageBox(fInterface->GetMAXHWnd(), _T("Really Cancel?"),
            _T("Question"), MB_ICONQUESTION | MB_YESNO);
        if (retval == IDYES)
        {
            return true;
        }
        else if (retval == IDNO)
        {
            fInterface->SetCancel(FALSE);
        }
    }

    return false;
}

uint32_t plExportProgressBar::CountNodes()
{
    return INodeCount(GetCOREInterface()->GetRootNode());
}

uint32_t plExportProgressBar::INodeCount(INode *node)
{
    uint32_t count = 1;

    for (int i = 0; i < node->NumberOfChildren(); i++)
    {
        INode *child = node->GetChildNode(i);
        count += INodeCount(child);
    }

    return count;
}
