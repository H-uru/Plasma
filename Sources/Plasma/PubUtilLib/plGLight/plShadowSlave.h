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

#ifndef plShadowSlave_inc
#define plShadowSlave_inc

#include "hsMatrix44.h"
#include "hsBounds.h"
#include "plViewTransform.h"
#include "hsGeometry3.h"

class plShadowCaster;
class plVolumeIsect;
class plPipeline;

class plShadowSlave
{
public:

    uint32_t              fIndex;

    hsMatrix44          fWorldToLight;
    hsMatrix44          fLightToWorld;

    hsPoint3            fLightPos;
    hsVector3           fLightDir;

    hsMatrix44          fWorldToTexture;
    hsMatrix44          fCastLUT;
    hsMatrix44          fRcvLUT;

    plViewTransform     fView;

    float            fPower;
    float            fBlurScale;

    hsBounds3Ext        fCasterWorldBounds;
    hsBounds3Ext        fWorldBounds;

    plVolumeIsect*      fIsect;

    uint32_t              fWidth;
    uint32_t              fHeight;

    float            fAttenDist;

    float            fPriority;

    uint32_t              fFlags;

    enum SlaveFlag
    {
        kObeysLightGroups       = 0x1,
        kIncludesChars          = 0x2,
        kSelfShadow             = 0x4,
        kCastInCameraSpace      = 0x8,
        kReverseZ               = 0x10,
        kTwoSided               = 0x20,
        kReverseCull            = 0x40,
        kPositional             = 0x80
    };

    virtual ~plShadowSlave() { }

    void SetFlag(SlaveFlag f, bool on) { if(on) fFlags |= f; else fFlags &= ~f; }
    bool HasFlag(SlaveFlag f) const { return 0 != (fFlags & f); }

    bool ObeysLightGroups() const { return HasFlag(kObeysLightGroups); }
    bool IncludesChars() const { return HasFlag(kIncludesChars); }
    bool SelfShadow() const { return HasFlag(kSelfShadow); }
    bool CastInCameraSpace() const { return HasFlag(kCastInCameraSpace); }
    bool ReverseZ() const { return HasFlag(kReverseZ); }
    bool TwoSided() const { return HasFlag(kTwoSided); }
    bool ReverseCull() const { return HasFlag(kReverseCull); }
    bool Positional() const { return HasFlag(kPositional); }

    virtual void Init() { fFlags = 0; }

    const plShadowCaster*       fCaster;

    // Internal to pipeline, no touch!!!
    int                         fLightIndex;
    int                         fLightRefIdx;
    int                         fSelfShadowOn;
    void*                       fPipeData; 

    bool                ISetupOrthoViewTransform();
    bool                ISetupPerspViewTransform();

    virtual bool        SetupViewTransform(plPipeline* pipe) = 0;
};

class plDirectShadowSlave : public plShadowSlave
{
    bool        SetupViewTransform(plPipeline* pipe) override { return ISetupOrthoViewTransform(); }
};

class plPointShadowSlave : public plShadowSlave
{
    bool        SetupViewTransform(plPipeline* pipe) override { return ISetupPerspViewTransform(); }
};

#endif // plShadowSlave_inc
