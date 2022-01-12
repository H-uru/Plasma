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

#include <vector>

#include "HeadSpin.h"
#include "plProxyGen.h"

#include "plSurface/hsGMaterial.h"
#include "plSurface/plLayer.h"
#include "plDrawableSpans.h"
#include "plDrawableGenerator.h"
#include "pnMessage/plProxyDrawMsg.h"
#include "pnKeyedObject/plKey.h"
#include "pnMessage/plRefMsg.h"
#include "pnMessage/plNodeRefMsg.h"
#include "plgDispatch.h"
#include "hsResMgr.h"

static std::vector<plDrawableSpans*>    fProxyDrawables;
static std::vector<hsGMaterial*>        fProxyMaterials;
static uint32_t                         fProxyKeyCounter = 0;

plProxyGen::plProxyGen(const hsColorRGBA& amb, const hsColorRGBA& dif, float opac)
:   fProxyMsgType(),
    fProxyDraw(),
    fProxyMat()
{
    fAmbient = amb;
    fColor = dif;
    fAmbient.a = opac;
}

plProxyGen::~plProxyGen()
{
    if( fProxyDraw )
        GetKey()->Release(fProxyDraw->GetKey());
    if( fProxyMat )
        GetKey()->Release(fProxyMat->GetKey());
}

void plProxyGen::Init(const hsKeyedObject* owner)
{
    if( !GetKey() )
    {
        ST::string buff;
        plLocation loc;
        if( owner->GetKey() )
        {
            buff = ST::format("{}_ProxyGen_{}_{}", owner->GetKey()->GetName(),
                              owner->GetKey()->GetUoid().GetClonePlayerID(),
                              fProxyKeyCounter++);
            loc = owner->GetKey()->GetUoid().GetLocation();
        }
        else
        {
            buff = ST::format("ProxyGen{}", fProxyKeyCounter++);
            loc = plLocation::kGlobalFixedLoc;
        }

        hsgResMgr::ResMgr()->NewKey( buff, this, loc );

        plgDispatch::Dispatch()->RegisterForExactType(plProxyDrawMsg::Index(), GetKey());
    }

}

uint32_t plProxyGen::IGetDrawableType() const
{
    switch( fProxyMsgType & plProxyDrawMsg::kAllTypes )
    {
    case plProxyDrawMsg::kLight:
        return plDrawable::kLightProxy;
    case plProxyDrawMsg::kPhysical:
        return plDrawable::kPhysicalProxy;
    case plProxyDrawMsg::kOccluder:
        return plDrawable::kOccluderProxy;
    case plProxyDrawMsg::kAudible:
        return plDrawable::kAudibleProxy;
    case plProxyDrawMsg::kCoordinate:
        return plDrawable::kCoordinateProxy;
    case plProxyDrawMsg::kCamera:
        return plDrawable::kCameraProxy;
    }
    hsAssert(false, "Unknown proxy type");
    return plDrawable::kGenericProxy;
}

uint32_t plProxyGen::IGetProxyIndex() const
{
    plKey sceneNode = IGetNode();
    uint32_t drawType = IGetDrawableType();

    hsSsize_t firstNil = -1;
    for (size_t i = 0; i < fProxyDrawables.size(); i++)
    {
        if( fProxyDrawables[i] )
        {
            if( (fProxyDrawables[i]->GetType() & drawType)
                &&(fProxyDrawables[i]->GetSceneNode() == sceneNode) )
            {
                return i;
            }
        }
        else if( firstNil < 0 )
            firstNil = i;
    }
    if( firstNil < 0 )
        firstNil = fProxyDrawables.size();
    if (firstNil >= fProxyDrawables.size())
        fProxyDrawables.resize(firstNil + 1);

    return (uint32_t)firstNil;
}

hsGMaterial* plProxyGen::IMakeProxyMaterial() const
{
    const hsColorRGBA& amb = fAmbient;
    const hsColorRGBA& dif = fColor;
    float opac = fAmbient.a;

    hsGMaterial* retVal = new hsGMaterial();

    ST::string buff;
    if( !GetKeyName().empty() )
        buff = ST::format("{}_Material", GetKeyName());
    else
        buff = "ProxyMaterial";
    hsgResMgr::ResMgr()->NewKey( buff, retVal, GetKey() ? GetKey()->GetUoid().GetLocation() : plLocation::kGlobalFixedLoc );

    plLayer *lay = retVal->MakeBaseLayer();
    lay->SetRuntimeColor(dif);
    lay->SetPreshadeColor(dif);
    lay->SetAmbientColor(amb);
    lay->SetOpacity(opac);
    if( opac < 1.f )
    {
        lay->SetBlendFlags(lay->GetBlendFlags() | hsGMatState::kBlendAlpha);
        lay->SetMiscFlags(hsGMatState::kMiscTwoSided);
        lay->SetZFlags(hsGMatState::kZNoZWrite);
    }

    return retVal;
}

hsGMaterial* plProxyGen::IFindProxyMaterial() const
{
    for (hsGMaterial* mat : fProxyMaterials)
    {
        if (!mat)
            continue;

        plLayer* lay = plLayer::ConvertNoRef(mat->GetLayer(0));
        if (lay &&
            lay->GetAmbientColor() == fAmbient &&
            lay->GetRuntimeColor() == fColor &&
            lay->GetOpacity() == fAmbient.a)
            return mat;
    }

    return nullptr;
}

hsGMaterial* plProxyGen::IGetProxyMaterial() const
{
    // If we already have a material, return that
    if (fProxyMat)
        return fProxyMat;

    // If there's one existing (for another proxy) that is setup the same as ours, use that
    hsGMaterial* mat = IFindProxyMaterial();
    if (mat)
        return mat;

    // Have to make a new one
    mat = IMakeProxyMaterial();
    fProxyMaterials.emplace_back(mat);
    return mat;
}

void plProxyGen::IGenerateProxy()
{
    if( !IGetNode() )
        return;

    uint32_t idx = IGetProxyIndex();

    hsGMaterial* mat = IGetProxyMaterial();
    hsAssert(mat, "Failed to create proxy material");

    fProxyIndex.clear();
    fProxyDrawables[idx] = ICreateProxy(mat, fProxyIndex, fProxyDrawables[idx]);

    if( fProxyDrawables[idx] && !fProxyDrawables[idx]->GetKey() )
    {
        ST::string buff;
        if( !GetKeyName().empty() )
            buff = ST::format("{}_ProxyDrawable", GetKeyName());
        else
            buff = "ProxyDrawable";

        hsgResMgr::ResMgr()->NewKey( buff, fProxyDrawables[ idx ], GetKey() ? GetKey()->GetUoid().GetLocation() : plLocation::kGlobalFixedLoc );
    }

    if( fProxyDrawables[idx] )
    {
        fProxyDrawables[idx]->SetType(IGetDrawableType());

        IApplyProxy(idx);

        plGenRefMsg* msg = new plGenRefMsg(GetKey(), plRefMsg::kOnRequest, idx, 0);
        hsgResMgr::ResMgr()->AddViaNotify(mat->GetKey(), msg, plRefFlags::kActiveRef);
        fProxyMat = mat;

        plNodeRefMsg* refMsg = new plNodeRefMsg( GetKey(), plNodeRefMsg::kOnRequest, (int8_t)idx, plNodeRefMsg::kDrawable );
        hsgResMgr::ResMgr()->AddViaNotify(fProxyDrawables[idx]->GetKey(), refMsg, plRefFlags::kActiveRef);
        fProxyDraw = fProxyDrawables[idx];
    }
}

//// IApplyProxy
// Add our proxy to our scenenode for rendering.
void plProxyGen::IApplyProxy(uint32_t idx) const
{
    if( fProxyDrawables[idx] && IGetNode() && (fProxyDrawables[idx]->GetSceneNode() != IGetNode()) )
    {
        fProxyDrawables[idx]->SetSceneNode(IGetNode());
    }
}

//// IRemoveProxy
// Remove our proxy from our scenenode.
void plProxyGen::IRemoveProxy(uint32_t idx) const
{
    if( fProxyDrawables[idx] )
    {
        fProxyDrawables[idx]->SetSceneNode(nullptr);
    }
}

/// Destroy the proxy. Duh.
void plProxyGen::IDestroyProxy()
{
    if( fProxyDraw )
    {
        if( fProxyDraw->GetSceneNode() )
            fProxyDraw->SetSceneNode(nullptr);
        GetKey()->Release(fProxyDraw->GetKey());
        fProxyDraw = nullptr;
    }
    if( fProxyMat )
    {
        GetKey()->Release(fProxyMat->GetKey());
        fProxyMat = nullptr;
    }
    fProxyDrawables.clear();
    fProxyMaterials.clear();
}

bool plProxyGen::MsgReceive(plMessage* msg)
{
    plProxyDrawMsg* pDraw = plProxyDrawMsg::ConvertNoRef(msg);
    if( pDraw && (pDraw->GetProxyFlags() & IGetProxyMsgType()) )
    {
        if( pDraw->GetProxyFlags() & plProxyDrawMsg::kCreate )
        {
            if (fProxyDraw == nullptr)
                IGenerateProxy();
        }
        else if( pDraw->GetProxyFlags() & plProxyDrawMsg::kDestroy )
        {
            if (fProxyDraw != nullptr)
                IDestroyProxy();
        }
        else if( pDraw->GetProxyFlags() & plProxyDrawMsg::kToggle )
        {
            if (fProxyDraw == nullptr)
                IGenerateProxy();
            else
                IDestroyProxy();
        }
        return true;
    }
    plNodeRefMsg* nodeRef = plNodeRefMsg::ConvertNoRef(msg);
    if( nodeRef )
    {
        if( nodeRef->GetContext() & (plRefMsg::kOnDestroy | plRefMsg::kOnRemove) )
        {
            if (nodeRef->fWhich < (hsSsize_t)fProxyDrawables.size())
                fProxyDrawables[nodeRef->fWhich] = nullptr;
        }
        return true;
    }
    plGenRefMsg* genMsg = plGenRefMsg::ConvertNoRef(msg);
    if( genMsg )
    {
        if( genMsg->GetContext() & (plRefMsg::kOnDestroy | plRefMsg::kOnRemove) )
        {
            if (genMsg->fWhich < (hsSsize_t)fProxyMaterials.size())
                fProxyMaterials[genMsg->fWhich] = nullptr;
        }
        return true;
    }

    return hsKeyedObject::MsgReceive(msg);
}


void plProxyGen::SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l)
{
    uint32_t idx = IGetProxyIndex();
    if( fProxyDrawables[idx] )
    {
        for (uint32_t proxyIndex : fProxyIndex)
            fProxyDrawables[idx]->SetTransform(proxyIndex, l2w, w2l);
    }
}

void plProxyGen::SetDisable(bool on)
{
    uint32_t idx = IGetProxyIndex();
    if( fProxyDrawables[idx] )
    {
        for (uint32_t proxyIndex : fProxyIndex)
            fProxyDrawables[idx]->SetNativeProperty(proxyIndex, plDrawable::kPropNoDraw, on);
    }
}
