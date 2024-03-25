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

#ifndef plClusterGroup_inc
#define plClusterGroup_inc

#include <vector>

#include "hsBitVector.h"
#include "plRenderLevel.h"
#include "pnKeyedObject/hsKeyedObject.h"

class hsStream;
class hsResMgr;
class plSpanTemplate;
class plCluster;
class hsGMaterial;
class plVisRegion;
class plLightInfo;
class plMessage;
class plGenRefMsg;
class plDrawableSpans;

class plLODDist
{
public:
    float fMinDist;
    float fMaxDist;

    plLODDist(float minDist, float maxDist) : fMinDist(minDist), fMaxDist(maxDist) {}
    plLODDist() : fMinDist(0), fMaxDist(0) {}

    plLODDist& Set(float minDist, float maxDist) { fMinDist = minDist; fMaxDist = maxDist; return *this; }

    plLODDist& operator=(int d) { fMinDist = float(d); fMaxDist = float(d); return *this; }

    int operator==(const plLODDist& d) const { return (fMinDist == d.fMinDist)&&(fMaxDist == d.fMaxDist); }

    void Read(hsStream* s);
    void Write(hsStream* s) const;

};

class plClusterGroup : public hsKeyedObject
{
public:
    enum RefType {
        kRefMaterial,
        kRefRegion,
        kRefLight
    };
protected:
    plSpanTemplate*                 fTemplate;

    hsGMaterial*                    fMaterial;

    std::vector<plVisRegion*>       fRegions;
    hsBitVector                     fVisSet;
    hsBitVector                     fVisNot;

    std::vector<plLightInfo*>       fLights;

    plLODDist                       fLOD;

    std::vector<plCluster*>         fClusters;
    uint32_t                        fUnPacked;

    plKey                           fSceneNode;
    plKey                           fDrawable;

    plRenderLevel                   fRenderLevel;

    bool        IAddVisRegion(plVisRegion* reg);
    bool        IRemoveVisRegion(plVisRegion* reg);
    bool        IAddLight(plLightInfo* li);
    bool        IRemoveLight(plLightInfo* li);
    bool        IOnRef(plGenRefMsg* ref);
    bool        IOnRemove(plGenRefMsg* ref);
    bool        IOnReceive(plGenRefMsg* ref);
    void        ISetVisBits();
    void        ISendToSelf(RefType t, hsKeyedObject* ref);

    plCluster*  IAddCluster();
    plCluster*  IGetCluster(int i) const;

    friend class plClusterUtil;
public:
    plClusterGroup();
    ~plClusterGroup();

    CLASSNAME_REGISTER( plClusterGroup );
    GETINTERFACE_ANY( plClusterGroup, hsKeyedObject );

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

    bool MsgReceive(plMessage* msg) override;

    hsGMaterial* GetMaterial() const { return fMaterial; }
    const hsBitVector& GetVisSet() const { return fVisSet; }
    const hsBitVector& GetVisNot() const { return fVisNot; }
    const std::vector<plLightInfo*>& GetLights() const { return fLights; }
    const plLODDist& GetLOD() const { return fLOD; }

    const plSpanTemplate* GetTemplate() const { return fTemplate; }

    const plCluster* GetCluster(size_t i) const;
    size_t GetNumClusters() const { return fClusters.size(); }
    size_t NumInst() const;

    // The drawable needs us to be able to convert our data
    // into, well, drawable stuff.
    void UnPack();

    void SetVisible(bool visible=true);

    void SetSceneNode(const plKey& key) { fSceneNode = key; }
    plKey GetSceneNode() const { return fSceneNode; }
    
    plKey GetDrawable() const { return fDrawable; }

    plRenderLevel GetRenderLevel() const { return fRenderLevel; }
};

#endif // plClusterGroup_inc
