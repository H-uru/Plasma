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

#ifndef plLayerAnimation_inc
#define plLayerAnimation_inc

#include <string_theory/string>

#include "plLayerInterface.h"

#include "plInterp/plAnimTimeConvert.h"

class plController;
class plLayerSDLModifier;
class plMessage;
class plSimpleStateVariable;

// LayerAnimations take advantage of the simplifying
// factor that they are write only. That is, it always
// overwrites the output of previous interfaces with
// the current value of the animation. Unanimated channels
// are unaffected.

class plLayerAnimationBase : public plLayerInterface
{
protected:
    ST::string      fSegmentID;
    double          fEvalTime;
    float           fCurrentTime;
    float           fLength;

    plController*   fPreshadeColorCtl;
    plController*   fRuntimeColorCtl;
    plController*   fAmbientColorCtl;
    plController*   fSpecularColorCtl;
    plController*   fOpacityCtl;
    plController*   fTransformCtl;

    float IMakeUniformLength();
    void IEvalConvertedTime(float secs, uint32_t passChans, uint32_t evalChans, uint32_t &dirty);

public:
    plLayerAnimationBase()
        : fPreshadeColorCtl(), fRuntimeColorCtl(), fAmbientColorCtl(), 
          fSpecularColorCtl(), fOpacityCtl(), fTransformCtl(),
          fEvalTime(-1.0), fCurrentTime(-1.f), fLength()
    { }
    virtual ~plLayerAnimationBase();

    CLASSNAME_REGISTER( plLayerAnimationBase );
    GETINTERFACE_ANY( plLayerAnimationBase, plLayerInterface );
    
    plLayerInterface*           Attach(plLayerInterface* prev) override;
    //virtual uint32_t                        Eval(double secs, uint32_t frame, uint32_t ignore) = 0;

    bool MsgReceive(plMessage* msg) override;

    void Read(hsStream* s, hsResMgr* mgr) override;
    void Write(hsStream* s, hsResMgr* mgr) override;

    // Specialized
    float GetLength() const { return fLength; }
    ST::string GetSegmentID() const { return fSegmentID; }
    void SetSegmentID(const ST::string &ID) { fSegmentID = ID; }

    // Export construction functions follow
    void SetPreshadeColorCtl(plController* colCtl);
    void SetRuntimeColorCtl( plController *colCtl );
    void SetAmbientColorCtl(plController* ambCtl);
    void SetSpecularColorCtl(plController* ambCtl);
    void SetOpacityCtl(plController* opaCtl);
    void SetTransformCtl(plController* xfmCtl);

    plController* GetPreshadeColorCtl() const { return fPreshadeColorCtl; }
    plController* GetRuntimeColorCtl() const { return fRuntimeColorCtl; }
    plController* GetAmbientColorCtl() const { return fAmbientColorCtl; }
    plController* GetSpecularColorCtl() const { return fSpecularColorCtl; }
    plController* GetOpacityCtl() const { return fOpacityCtl; }
    plController* GetTransformCtl() const { return fTransformCtl; }
};

class plLayerAnimation : public plLayerAnimationBase
{
    friend class plLayerSDLModifier;    

protected:
    plAnimTimeConvert           fTimeConvert;
    plLayerSDLModifier*         fLayerSDLMod;   // handles sending/recving sdl state

public:
    plLayerAnimation();
    virtual ~plLayerAnimation();

    CLASSNAME_REGISTER( plLayerAnimation );
    GETINTERFACE_ANY( plLayerAnimation, plLayerAnimationBase );

    plLayerInterface*           Attach(plLayerInterface* prev) override;
    uint32_t                      Eval(double wSecs, uint32_t frame, uint32_t ignore) override;

    bool MsgReceive(plMessage* msg) override;

    void Read(hsStream* s, hsResMgr* mgr) override;
    void Write(hsStream* s, hsResMgr* mgr) override;
    
    const plLayerSDLModifier* GetSDLModifier() const { return fLayerSDLMod; }
    plAnimTimeConvert& GetTimeConvert() { return fTimeConvert; }

    void DefaultAnimation();
};

class plLayerLinkAnimation : public plLayerAnimation   
{
protected:
    plKey fLinkKey;
    bool fEnabled;
    plEventCallbackMsg *fIFaceCallback;

    enum
    {
        kFadeLinkPrep   = 0x01,
        kFadeLinking    = 0x02,
        kFadeCamera     = 0x04,
        kFadeIFace      = 0x08,
        kFadeCCR        = 0x10,
    };
    uint8_t fFadeFlags;
    uint8_t fLastFadeFlag;
    bool fFadeFlagsDirty;
    
public:
    plLayerLinkAnimation();
    ~plLayerLinkAnimation();

    CLASSNAME_REGISTER( plLayerLinkAnimation );
    GETINTERFACE_ANY( plLayerLinkAnimation, plLayerAnimation );

    void SetLinkKey(plKey linkKey) { fLinkKey = std::move(linkKey); }
    plKey GetLinkKey() { return fLinkKey; }

    // NOTE: The link animation should NEVER NEVER NEVER send its state to the server.
    // NEVER!
    // If you think it should... talk to Bob. He will explain why it can't be, and beat you up.
    // If he can't remember, beat him up until he does (or ask Moose).
    bool DirtySynchState(const ST::string& sdlName, uint32_t sendFlags) override { return false; } // don't send link state

    void Read(hsStream* s, hsResMgr* mgr) override;
    void Write(hsStream* s, hsResMgr* mgr) override;
    uint32_t Eval(double wSecs, uint32_t frame, uint32_t ignore) override;
    bool MsgReceive(plMessage* pMsg) override;
    void Enable(bool b) { fEnabled = b; }
    void SetFadeFlag(uint8_t flag, bool val);

    bool fLeavingAge;
};

class plLayerSDLAnimation : public plLayerAnimationBase
{
protected:
    plSimpleStateVariable *fVar;
    ST::string fVarName;

public:
    plLayerSDLAnimation();

    CLASSNAME_REGISTER( plLayerSDLAnimation );
    GETINTERFACE_ANY( plLayerSDLAnimation, plLayerAnimationBase );

    uint32_t Eval(double wSecs, uint32_t frame, uint32_t ignore) override;

    bool MsgReceive(plMessage* msg) override;

    void Read(hsStream* s, hsResMgr* mgr) override;
    void Write(hsStream* s, hsResMgr* mgr) override;

    ST::string GetVarName() const { return fVarName; }
    void SetVarName(const ST::string &name) { fVarName = name; }
};

#endif // plLayerAnimation_inc
