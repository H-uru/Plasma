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
#include "hsGeometry3.h"
#include "plWinAudible.h"
#include "hsMatrix44.h"
#include "plAudio/plSound.h"
#include "plAudio/plWin32Sound.h"
#include "plAudio/plVoiceChat.h"
#include "plAudio/plAudioSystem.h"
#include "plAudio/plWin32StreamingSound.h"
#include "pnMessage/plSoundMsg.h"
#include "pnMessage/plTimeMsg.h"
#include "pnMessage/plAudioSysMsg.h"
#include "hsResMgr.h"
#include "pnKeyedObject/plKey.h"

#include "plgDispatch.h"
#include "pnMessage/plNodeRefMsg.h"
#include "pnMessage/plCmdIfaceModMsg.h"
#include "pnMessage/plProxyDrawMsg.h"
#include "plMessage/plInputEventMsg.h"
#include "plModifier/plSoundSDLModifier.h"
#include "pnSceneObject/plSceneObject.h"
#include "plStatusLog/plStatusLog.h"

// Visualization
#include "plWinAudibleProxy.h"

#define SND_INDEX_CHECK( index, ret )           \
    if ((size_t)index >= fSoundObjs.size())     \
    {                                           \
        hsStatusMessageF("ERROR: Sound index out of range (index %d, count %zu)\n", index, fSoundObjs.size()); \
        return ret;                             \
    }

#define SND_APPLY_LOOP( index, func, ret )      \
    if( index != -1 )                           \
    {                                           \
        SND_INDEX_CHECK( index, ret );          \
        if (fSoundObjs[index] != nullptr)       \
            fSoundObjs[index]->func;            \
    }                                           \
    else                                        \
    {                                           \
        for (plSound* sound : fSoundObjs)       \
        {                                       \
            if (sound != nullptr)               \
                sound->func;                    \
        }                                       \
    }                                           

plWinAudible::plWinAudible()
:   fSDLMod()
{
    fProxyGen = new plWinAudibleProxy;
    fProxyGen->Init(this);

    fLocalToWorld.Reset();
}

plWinAudible::~plWinAudible()
{
    // delete SDL modifier
    if (fSceneObj)
    {
        plSceneObject* so=plSceneObject::ConvertNoRef(fSceneObj->ObjectIsLoaded());
        if (so)
            so->RemoveModifier(fSDLMod);        
    }
    delete fSDLMod;

    delete fProxyGen;
    for (plSound* sound : fSoundObjs)
        delete sound;
    fSoundObjs.clear();
}

void plWinAudible::SetSceneObject(plKey obj)
{
    plKey oldKey;
    // remove old SDL mod
    if (fSDLMod && fSceneObj && fSceneObj != obj)
    {
        oldKey = fSceneObj;
        plSceneObject* so=plSceneObject::ConvertNoRef(fSceneObj->ObjectIsLoaded());
        if (so)
            so->RemoveModifier(fSDLMod);
        delete fSDLMod;
        fSDLMod = nullptr;
    }

    fSceneObj = obj;
    plSceneObject* so = plSceneObject::ConvertNoRef(obj ? obj->ObjectIsLoaded() : nullptr);
    if (so)
    {
        so->RemoveModifier(fSDLMod);        
        delete fSDLMod;
        fSDLMod=new plSoundSDLModifier;
        so->AddModifier(fSDLMod);
    }

    for (plSound* sound : fSoundObjs)
    {
        if (sound != nullptr && sound->GetKey() != nullptr)
        {
            if (obj != nullptr)
            {
                plGenRefMsg *replaceMsg = new plGenRefMsg(sound->GetKey(), plRefMsg::kOnReplace, 0, plSound::kRefParentSceneObject);
                hsgResMgr::ResMgr()->AddViaNotify( obj, replaceMsg, plRefFlags::kPassiveRef );
            }
            else if (oldKey != nullptr)
                sound->GetKey()->Release(oldKey);
        }
    }
}

void plWinAudible::SetSceneNode(plKey newNode)
{
    plKey oldNode = GetSceneNode();
    if( oldNode == newNode )
        return;

    if( !oldNode )
        Activate();

    if( newNode )
    {
        plNodeRefMsg* refMsg = new plNodeRefMsg(newNode, plNodeRefMsg::kOnRequest, -1, plNodeRefMsg::kAudible);
        hsgResMgr::ResMgr()->AddViaNotify(GetKey(), refMsg, plRefFlags::kPassiveRef);

    }
    if( oldNode )
    {
        oldNode->Release(GetKey());
    }

    if( !newNode )
    {
        DeActivate();
    }
    fSceneNode = std::move(newNode);
}

void plWinAudible::GetStatus(plSoundMsg* pMsg)
{
    if (pMsg->fIndex == -1)
    {
        for (size_t i = 0; i < fSoundObjs.size(); i++)
        {
            plSoundMsg* pReply = fSoundObjs[i]->GetStatus(pMsg);
            pReply->fIndex = (int)i;
            plgDispatch::MsgSend(pReply);
        }
    }
    else if ((size_t)pMsg->fIndex < fSoundObjs.size())
    {
        plSoundMsg* pReply = fSoundObjs[pMsg->fIndex]->GetStatus(pMsg);
        pReply->fIndex = pMsg->fIndex;
        plgDispatch::MsgSend(pReply);
    }
}

bool plWinAudible::AddSound( plSound *pSnd, int index, bool is3D )
{
    hsAssert(pSnd->GetKey() != nullptr, "Adding a new sound with no key.");
    if (plgAudioSys::Active())
    {
        hsgResMgr::ResMgr()->AddViaNotify( pSnd->GetKey(), new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, index, 0 ), plRefFlags::kActiveRef );
        return true;
    }

    else
    {
        pSnd->SetProperty( plSound::kPropIs3DSound, is3D );
        hsgResMgr::ResMgr()->AddViaNotify( pSnd->GetKey(), new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, index, 0 ), plRefFlags::kActiveRef );
        return true;
    }

}

/* Unused
int plWinAudible::AddSoundFromResource(plSound *pSnd, void* addr, int32_t size, bool is3D )
{
    //plWin32Sound* pSnd = new plWin32Sound;
    //IAssignSoundKey( pSnd, GetKey() ? GetKeyName() : "", fSoundObjs.Count() - 1 );    

    if (plgAudioSys::Active())
    {
        int ret = pSnd->GetSoundFromMemory(addr, size, is3D );
        if (ret)
        {
            fSoundObjs.Append(pSnd);
            return (fSoundObjs.Count() -1 );
        }
    }

    delete pSnd;
    return -1;
}
*/
plAudible& plWinAudible::SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l, int index)
{
    fLocalToWorld = l2w;
    plAudible::SetTransform(l2w, w2l);
    hsVector3 v = l2w.GetAxis(hsMatrix44::kUp);
    v*=-1;
    hsPoint3 pos = l2w.GetTranslate();
    
    if (index != -1)
    {
        SND_INDEX_CHECK( index, (*this) );
        if (fSoundObjs[index] != nullptr)
        {
            fSoundObjs[index]->SetPosition( pos );
            fSoundObjs[index]->SetConeOrientation( v.fX,v.fY,v.fZ );
        }
    }
    else
    {
        for (plSound* sound : fSoundObjs)
        {
            if (sound != nullptr)
            {
                sound->SetConeOrientation(v.fX, v.fY, v.fZ);
                sound->SetPosition(pos);
            }
        }
    }
    fProxyGen->SetTransform(l2w, w2l);

    return (*this);

}

void plWinAudible::SetTime(double t, int index)
{
    SND_APPLY_LOOP( index, SetTime( t ), ; );
}

void plWinAudible::SynchedPlay(int index)
{
    for (plSound* sound : fSoundObjs)
    {
        if (sound->IsPlaying())
        {
            fSoundObjs[index]->SynchedPlay(sound->GetByteOffset());
            return;
        }
    }
}

void plWinAudible::Play(int index )
{   
    SND_APPLY_LOOP( index, Play(), ; );
}

void plWinAudible::FastForwardPlay(int index /* = -1 */)
{
    SND_APPLY_LOOP(index, FastForwardPlay(), ; );
}

void plWinAudible::FastForwardToggle(int index /* = -1 */)
{
    SND_APPLY_LOOP(index, FastForwardToggle(), ; );
}

void plWinAudible::SetOuterVol(const int v, int index)
{
    SND_APPLY_LOOP( index, SetOuterVolume( v ), ; );
}

void plWinAudible::SetConeAngles(int inner, int outer, int index)
{
    SND_APPLY_LOOP( index, SetConeAngles( inner, outer ), ; );
}

void plWinAudible::Stop(int index)
{
    SND_APPLY_LOOP( index, Stop(), ; );
}

void plWinAudible::SetMin(const float m,int index)
{
    SND_APPLY_LOOP( index, SetMin( (int)m ), ; );
}

void plWinAudible::SetMax(const float m,int index)
{
    SND_APPLY_LOOP( index, SetMax( (int)m ), ; );
}

// Takes a 0-1.f as the volume (platform independent)
void    plWinAudible::SetVolume(const float volume,int index )
{
    SND_APPLY_LOOP( index, SetVolume( volume ), ; );
}

void    plWinAudible::SetMuted( bool muted, int index )
{
    SND_APPLY_LOOP( index, SetMuted( muted ), ; );
}

void    plWinAudible::ToggleMuted( int index )
{
    if( index != -1 )
    {
        SND_INDEX_CHECK( index, ; );
        if (fSoundObjs[index] != nullptr)
            fSoundObjs[ index ]->SetMuted( !fSoundObjs[ index ]->IsMuted() );
    }
    else
    {
        for (plSound* sound : fSoundObjs)
        {
            if (sound != nullptr)
                sound->SetMuted(!sound->IsMuted());
        }
    }
}

float plWinAudible::GetMin(int index) const
{
    return (float)(fSoundObjs[index]->GetMin());
}

float plWinAudible::GetMax(int index) const
{
    return (float)(fSoundObjs[index]->GetMax());
}

void plWinAudible::SetVelocity(const hsVector3 vel,int index)
{
    SND_APPLY_LOOP( index, SetVelocity( vel ), ; );
}

hsVector3 plWinAudible::GetVelocity(int index) const
{
    return(fSoundObjs[index]->GetVelocity());
}

hsPoint3 plWinAudible::GetPosition(int index)
{
    return( fSoundObjs[index]->GetPosition() );
}

void plWinAudible::SetLooping(bool loop,int index)
{
    SND_APPLY_LOOP( index, SetProperty( plSound::kPropLooping, loop ), ; );
}

void    plWinAudible::SetFadeIn( const int type, const float length, int index )
{
    SND_APPLY_LOOP( index, SetFadeInEffect( (plSound::plFadeParams::Type)type, length ), ; );
}

void    plWinAudible::SetFadeOut( const int type, const float length, int index ) 
{
    SND_APPLY_LOOP( index, SetFadeOutEffect( (plSound::plFadeParams::Type)type, length ), ; );
}

void plWinAudible::SetFilename(int index, const char *filename, bool isCompressed)
{
    SND_INDEX_CHECK(index, ; )

    plWin32StreamingSound *pStreamingSound = plWin32StreamingSound::ConvertNoRef(fSoundObjs[ index ]);
    if(pStreamingSound)
    {
        pStreamingSound->SetFilename(filename, isCompressed);
    }
    else
    {
        plStatusLog::AddLineSF("audio.log", "Cannot set filename of non-streaming sound. {}",
                               fSoundObjs[ index ]->GetKeyName());
    }
}

bool plWinAudible::IsPlaying(int index)
{
    if (index < (int)fSoundObjs.size())
    {
        if (index == -1)
        {
            for (plSound* sound : fSoundObjs)
            {
                if (sound != nullptr && sound->IsPlaying())
                    return true;
            }
            return false;
        }

        if (fSoundObjs[index] == nullptr)
            return false;

        return(fSoundObjs[index]->IsPlaying());
    } else {
        return false;
    }
}

void plWinAudible::Read(hsStream* s, hsResMgr* mgr)
{
    plAudible::Read(s, mgr);
    uint32_t n = s->ReadLE32();
    fSoundObjs.resize(n);
    for (uint32_t i = 0; i < n; i++)
    {
        plGenRefMsg* msg = new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, i, 0);
        mgr->ReadKeyNotifyMe(s, msg, plRefFlags::kActiveRef);
    }

    plKey pSceneKey = mgr->ReadKey(s);
    plNodeRefMsg* refMsg = new plNodeRefMsg(pSceneKey, plRefMsg::kOnCreate, -1, plNodeRefMsg::kAudible); 
    mgr->AddViaNotify(GetKey(), refMsg, plRefFlags::kPassiveRef);
}

void plWinAudible::IAssignSoundKey( plSound *sound, const char *name, uint32_t i )
{
    ST::string keyName = ST::format("{}_{}", name, i);
    hsgResMgr::ResMgr()->NewKey( keyName, sound, GetKey() ? GetKey()->GetUoid().GetLocation() : plLocation::kGlobalFixedLoc );
}

void plWinAudible::Write(hsStream* s, hsResMgr* mgr)
{
    plAudible::Write(s, mgr);
    s->WriteLE32((uint32_t)fSoundObjs.size());
    for (plSound* sound : fSoundObjs)
        mgr->WriteKey(s, sound);

    mgr->WriteKey(s, fSceneNode);
}

void plWinAudible::Activate()
{
    for (plSound* sound : fSoundObjs)
        if (sound != nullptr)
            sound->Activate();
}


void plWinAudible::DeActivate()
{
    for (plSound* sound : fSoundObjs)
    {
        if (sound != nullptr)
            sound->DeActivate();
    }
}

bool plWinAudible::MsgReceive(plMessage* msg)
{
    plGenRefMsg *refMsg = plGenRefMsg::ConvertNoRef(msg);
    if (refMsg)
    {
        plSound *snd = plSound::ConvertNoRef(refMsg->GetRef());
        if (snd)
        {
            int index = refMsg->fWhich;
            if( refMsg->GetContext() & (plRefMsg::kOnCreate | plRefMsg::kOnRequest) )
            {
                if (index >= (int)fSoundObjs.size())
                    fSoundObjs.resize(index + 1);
                fSoundObjs[index] = plSound::ConvertNoRef(refMsg->GetRef());
                if (plgAudioSys::Active())
                    fSoundObjs[index]->Activate();
            }
            else if( refMsg->GetContext() & (plRefMsg::kOnRemove | plRefMsg::kOnDestroy) )
            {
                if ((size_t)index < fSoundObjs.size())
                    fSoundObjs[index] = nullptr;
            }
            return true;
        }
    }

    plAudioSysMsg *sysMsg = plAudioSysMsg::ConvertNoRef( msg );
    if( sysMsg )
    {
        if( sysMsg->GetAudFlag() == plAudioSysMsg::kChannelVolChanged )
        {
            SND_APPLY_LOOP( -1, RefreshVolume(), false; );
        }
    }

    // proxyDrawMsg handling--just pass it on to the proxy object
    plProxyDrawMsg *pdMsg = plProxyDrawMsg::ConvertNoRef( msg );
    if (pdMsg != nullptr)
    {
        if( fProxyGen )
            return fProxyGen->MsgReceive( pdMsg );

        return true;
    }

    return plAudible::MsgReceive(msg);
}

void plWinAudible::RemoveCallbacks(plSoundMsg* pMsg)
{
    // sanity check
    if (pMsg->fIndex >= (int)fSoundObjs.size())
        return;
    if( pMsg->fIndex < 0 )
    {
        for (plSound* sound : fSoundObjs)
            sound->RemoveCallbacks(pMsg);
    }
    else
    {
        fSoundObjs[pMsg->fIndex]->RemoveCallbacks(pMsg);
    }
}

void plWinAudible::AddCallbacks(plSoundMsg* pMsg)
{
    // sanity check
    if (pMsg->fIndex >= (int)fSoundObjs.size())
        return;
    for (size_t i = 0; i < pMsg->GetNumCallbacks(); i++)
    {
        pMsg->GetEventCallback(i)->fIndex = pMsg->fIndex;
        pMsg->GetEventCallback(i)->SetSender(GetKey());
    }
    if( pMsg->fIndex < 0 )
    {
        for (plSound* sound : fSoundObjs)
            sound->AddCallbacks(pMsg);
    }
    else
    {
        fSoundObjs[pMsg->fIndex]->AddCallbacks(pMsg);
    }
}

int plWinAudible::GetSoundIndex(const char *keyname) const
{
    for (size_t i = 0; i < fSoundObjs.size(); i++)
    {
        if (!fSoundObjs[i])
            continue;
        if (fSoundObjs[i]->GetKeyName() == keyname)
            return int(i);
    }
    return -1;
}

plSound* plWinAudible::GetSound(size_t i) const
{ 
    return fSoundObjs[i];
}

// Visualization
plDrawableSpans* plWinAudible::CreateProxy(hsGMaterial* mat, std::vector<uint32_t>& idx, plDrawableSpans* addTo)
{
    plDrawableSpans* myDraw = addTo;
    for (plSound* sound : fSoundObjs)
        myDraw = sound->CreateProxy(fLocalToWorld, mat, idx, myDraw);
    return myDraw;
}


//
// 2-way win audible (for local players)
//

pl2WayWinAudible::pl2WayWinAudible() :
fVoicePlayer(),
fVoiceRecorder(),
fActive()
{
}

pl2WayWinAudible::~pl2WayWinAudible()
{
    DeActivate();
    if (fVoicePlayer)
        delete fVoicePlayer;
    if(fVoiceRecorder)
        delete fVoiceRecorder;
}

bool pl2WayWinAudible::MsgReceive(plMessage* msg)
{
    plEvalMsg* pMsg = plEvalMsg::ConvertNoRef(msg);
    if (pMsg && fVoiceRecorder)
    {
        fVoiceRecorder->Update(pMsg->GetTimeStamp());
//      fVoxIO->Update();
        return true;
    }

    plControlEventMsg* pCtrlMsg = plControlEventMsg::ConvertNoRef(msg);
    if (pCtrlMsg)
    {
        if (pCtrlMsg->GetControlCode() == S_PUSH_TO_TALK && fVoiceRecorder)
            fVoiceRecorder->SetMicOpen(pCtrlMsg->ControlActivated());
        return true;
    }

    return plWinAudible::MsgReceive(msg);
}

void pl2WayWinAudible::Init(bool isLocal)
{
    if (!fVoicePlayer)
        fVoicePlayer = new plVoicePlayer;
    if (!fVoiceRecorder) {
        if (isLocal)
            fVoiceRecorder = new plVoiceRecorder;
    }
    Activate();
}

void pl2WayWinAudible::Activate()
{
    if(fActive)
        return;

    plWinAudible::Activate();
    if(fVoicePlayer)
    {
        fVoicePlayer->GetSoundPtr()->Activate();
        fActive = true;
    }
    if (fVoiceRecorder)
    {
        plCmdIfaceModMsg* pModMsg = new plCmdIfaceModMsg;
        pModMsg->SetBCastFlag(plMessage::kBCastByExactType);
        pModMsg->SetSender(GetKey());
        pModMsg->SetCmd(plCmdIfaceModMsg::kAdd);
        plgDispatch::MsgSend(pModMsg);
        fActive = true;
    }
}

void pl2WayWinAudible::DeActivate()
{
    if(!fActive)
        return;

    plWinAudible::DeActivate();
    if(fVoicePlayer)
    {
        fVoicePlayer->GetSoundPtr()->DeActivate();
        fActive = false;
    }
    if (fVoiceRecorder)
    {           
        plCmdIfaceModMsg* pModMsg = new plCmdIfaceModMsg;
        pModMsg->SetBCastFlag(plMessage::kBCastByExactType);
        pModMsg->SetSender(GetKey());
        pModMsg->SetCmd(plCmdIfaceModMsg::kRemove);
        plgDispatch::MsgSend(pModMsg);
        fActive = false;
    }
}

void pl2WayWinAudible::Read(hsStream* s, hsResMgr* mgr)
{
    plWinAudible::Read(s, mgr);
    plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());
}

void pl2WayWinAudible::PlayNetworkedSpeech(const char* addr, size_t size, int numFrames, unsigned char flags)
{
    if (fVoicePlayer)
        fVoicePlayer->PlaybackVoiceMessage((uint8_t*)addr, size, numFrames, flags);
}

plAudible& pl2WayWinAudible::SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l, int index)
{
    plWinAudible::SetTransform(l2w, w2l, index);
    if (fVoicePlayer)
    {
        hsPoint3 pt = l2w.GetTranslate();

        // adjust to head level
        pt.fZ += 6.33f;
        fVoicePlayer->SetPosition( pt );
        hsPoint3 v(l2w.GetAxis(hsMatrix44::kView));
        fVoicePlayer->SetOrientation(v);
    }
    return (*this);
}

void pl2WayWinAudible::SetVelocity(const hsVector3 vel,int index)
{
    plWinAudible::SetVelocity(vel, index);
    if (fVoicePlayer)
        fVoicePlayer->SetVelocity( vel );
}

void pl2WayWinAudible::SetTalkIcon(int index, uint32_t str)
{
    if (fVoicePlayer)
        fVoicePlayer->SetTalkIcon(index,str);
}

void pl2WayWinAudible::ClearTalkIcon()
{
    if (fVoicePlayer)
        fVoicePlayer->ClearTalkIcon();
}

