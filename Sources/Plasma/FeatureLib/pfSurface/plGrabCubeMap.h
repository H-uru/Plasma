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

#ifndef plGrabCubeMap_inc
#define plGrabCubeMap_inc

#include "plScene/plRenderRequest.h"

class plSceneObject;
class plPipeline;
class plPageTreeMgr;

struct hsMatrix44;
struct hsPoint3;
struct hsColorRGBA;

class plGrabCubeRenderRequest : public plRenderRequest
{
public:
    plGrabCubeRenderRequest();

    char            fFileName[256];
    uint8_t           fQuality;

    // This function is called after the render request is processed by the client
    virtual void    Render(plPipeline* pipe, plPageTreeMgr* pageMgr);
};

class plGrabCubeMap
{
protected:
    void ISetupRenderRequests(plPipeline* pipe, const hsPoint3& center, const char* pref, const hsColorRGBA& clearColor, uint8_t q) const;

public:
    plGrabCubeMap() {}
    void GrabCube(plPipeline* pipe, plSceneObject* obj, const char* pref, const hsColorRGBA& clearColor, uint8_t q=75);
    void GrabCube(plPipeline* pipe, const hsPoint3& pos, const char* pref, const hsColorRGBA& clearColor, uint8_t q=75);
};


#endif // plGrabCubeMap_inc
