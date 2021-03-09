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


#ifndef plCluster_inc
#define plCluster_inc

#include <vector>

#include "plClusterGroup.h"
#include "plSpanInstance.h"

class hsGMaterial;
class hsStream;
class plSpanInstance;
class hsKeyedObject;
class plLightInfo;
class plSpanTemplate;
class plVisRegion;
class hsBounds3Ext;

class plCluster
{
public:
    enum
    {
        kNoIdx = uint8_t(-1)
    };
protected:
    plClusterGroup*             fGroup;

    std::vector<plSpanInstance*> fInsts;

    plSpanEncoding              fEncoding;

    friend class plClusterUtil;
    plSpanInstance*             IGetInst(size_t i) const { return fInsts[i]; }
    void                        IAddInst(plSpanInstance* inst) { fInsts.emplace_back(inst); }
public:

    plCluster();
    ~plCluster();

    void Read(hsStream* s, plClusterGroup* grp);
    void Write(hsStream* s) const;

    size_t NumInsts() const { return fInsts.size(); }
    const plSpanInstance& GetInst(size_t i) const { return *fInsts[i]; }

    void UnPack(uint8_t* vDst, uint16_t* iDst, int idxOffset, hsBounds3Ext& wBnd) const;

    // Getters and setters, mostly for export construction.
    const plSpanTemplate* GetTemplate() const { return fGroup->GetTemplate(); }

    void SetEncoding(const plSpanEncoding& c) { fEncoding = c; }
    plSpanEncoding GetEncoding() const { return fEncoding; }

    plClusterGroup* GetGroup() const { return fGroup; }
    void SetGroup(plClusterGroup* g) { fGroup = g; }

    hsGMaterial* GetMaterial() const { return fGroup->GetMaterial(); }

    const hsBitVector& GetVisSet() const { return fGroup->GetVisSet(); }
    const hsBitVector& GetVisNot() const { return fGroup->GetVisNot(); }

    const std::vector<plLightInfo*>& GetLights() const { return fGroup->GetLights(); }

    const plLODDist& GetLOD() const { return fGroup->GetLOD(); }
};

#endif // plCluster_inc
