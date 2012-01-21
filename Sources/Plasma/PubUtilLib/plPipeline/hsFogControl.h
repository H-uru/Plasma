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

#ifndef hsFogControl_inc
#define hsFogControl_inc

//#include "hsSceneObject.h"
//#include "hsGEnviron.h"

class hsSceneNode;
class hsG3DDevice;
class hsGView3;
class hsPortal;

class hsFogControl : public plCreatable {
public:
    enum {
        kFogCtlPortal
    };
protected:

    plKey                   fNodeKey;
    hsDynamicArray<plKey>   fFogNodes;
//  hsGEnvironment::FogState            fAvgFog;

    virtual float    IGetStrength(hsSceneNode* node) = 0;

    void                IPopNodes();
    void                IPushNodes();

    void                IZeroAvgFog();
    void                IAverageNodes();
//  void                IAccumFog(hsGEnvironment* env, float wgt);

public:
    hsFogControl() {}
    ~hsFogControl() {}

    CLASSNAME_REGISTER( hsFogControl );
    GETINTERFACE_ANY( hsFogControl, plCreatable );

    virtual uint32_t      GetType() = 0;

//  virtual hsGEnvironment* GetHomeEnv() = 0;
    virtual void Init(hsSceneNode* node);

    virtual void Blend() = 0;

    virtual void Restore() = 0; 

    virtual void Read(hsStream *stream, hsResMgr* mgr) = 0;
    virtual void Write(hsStream *stream, hsResMgr* mgr) = 0;

};

#if 0 // Move up to FeatureLevel
class hsNodeFogControl : public hsFogControl {
public:
    hsNodeFogControl();
    ~hsNodeFogControl();

    hsSceneNode* GetFogNode(int i);
    hsSceneNode *GetHomeNode();
    virtual void Init(hsSceneNode* node);
    
    virtual hsGEnvironment* GetHomeEnv();

    virtual void Blend();

    virtual void Restore(); 

    virtual void Read(hsStream *stream);
    virtual void Write(hsStream *stream);
};

class hsPortalFogControl : public hsNodeFogControl {
protected:
    enum {
        kStatusNone             = 0x0,
        kStatusNodesSet         = 0x1
    };

    hsDynamicArray<plKey>       fPortals;
    float                                fDefRadius;

    uint32_t                                  fStatus;

    void                    IFindFogNodes();
    virtual float        IGetStrength(hsSceneNode* node);
public:
    hsPortalFogControl();

    void SetDefaultRadius(float r) { fDefRadius = r; }
    float GetDefaultRadius() { return fDefRadius; }

    hsPortal* GetPortal(int i);

    virtual uint32_t      GetType() { return kFogCtlPortal; }

    virtual void Init(hsSceneNode* node);
    virtual void Blend();

    int GetNumPortalKeys() { return fPortals.GetCount(); }
    void AddPortalKey(plKey key) { fPortals.Append(key); }
    plKey GetPortalKey(int i) { return fPortals[i]; }

    virtual void Read(hsStream *stream);
    virtual void Write(hsStream *stream);
};
#endif // Move up to FeatureLevel

#endif hsFogControl_inc
