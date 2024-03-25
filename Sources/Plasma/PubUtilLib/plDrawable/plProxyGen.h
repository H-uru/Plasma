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

#ifndef plProxyGen_inc
#define plProxyGen_inc

#include "pnKeyedObject/hsKeyedObject.h"
#include "hsColorRGBA.h"

class plDrawableSpans;
class hsGMaterial;
struct hsMatrix44;

//
// Making your own ProxyGen suitable for visualizing class YourObject.
//
// Derive YourObjectProxy() from ProxyGen.
// YourObjectProxy() should set fProxyMsgType and the colors fAmbient,fColor, either
//      in the constructor or in Init().
// Implement IGetNode() (see below).
// Implement ICreateProxy() (see below).
// Implement a CreateProxy() member to YourObject (see below).
// Add DrawableType kYourObjectProxy to plDrawable.h
// Add console commands under Graphics.Show.YourObjectProxy.
// 
// See plLightSpace and plLightProxy for examples.
//
// More details.
// ============
// To make your own ProxyGen, you should subclass plProxyGen and override Init()
// because you'll probably want to keep a pointer to your owner (of type only you
// need to understand). 
// 
// The IGetNode() function lets your owner specify which plSceneNode to place the
// drawable in. That will normally just return your owner's GetSceneNode() call.
//
// ICreateProxy() let's your ProxyGen do any tweaking before calling CreateProxy()
// on your object. Your ProxyGen::ICreateProxy() could just do the work itself, but
// normally doesn't have enough protected information to really do the job.
//
// Typically your object's CreateProxy() function will just translate its internal
// data to a form suitable for the plDrawableGenerator suite of helper functions
// and return the result of a call into one of them.
//
class plProxyGen : public hsKeyedObject
{
protected:

    hsColorRGBA                 fAmbient; // opacity in ambient.a
    hsColorRGBA                 fColor;

    plDrawableSpans*            fProxyDraw;
    hsGMaterial*                fProxyMat;

    uint32_t                      fProxyMsgType;

    std::vector<uint32_t>       fProxyIndex;

    // These must be implemented by the specific type, so we know what to draw.
    virtual plDrawableSpans*    ICreateProxy(hsGMaterial* mat, std::vector<uint32_t>& idx, plDrawableSpans* addTo=nullptr) = 0; // called by IGenerate
    virtual plKey               IGetNode() const = 0;

    // Derived type should set fProxyMsgType as one of plProxyDrawMsg::types
    uint32_t                      IGetProxyMsgType() const { return fProxyMsgType; }

    // These are all fine by default.
    uint32_t                      IGetProxyIndex() const;
    uint32_t                      IGetDrawableType() const;

    virtual hsGMaterial*        IMakeProxyMaterial() const;
    virtual hsGMaterial*        IGetProxyMaterial() const; // will make material if needed.
    hsGMaterial* IFindProxyMaterial() const;

    virtual void                IGenerateProxy();
    virtual void                IApplyProxy(uint32_t drawIdx) const; // called by IGenerate 
    virtual void                IRemoveProxy(uint32_t drawIdx) const; 
    virtual void                IDestroyProxy();
public:
    plProxyGen(const hsColorRGBA& amb, const hsColorRGBA& dif, float opac);
    virtual ~plProxyGen();

    virtual void                SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l);
    virtual void                SetDisable(bool on);

    virtual void                Init(const hsKeyedObject* owner);

    bool                MsgReceive(plMessage* msg) override;

};


#endif // plProxyGen_inc
