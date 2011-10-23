
#ifndef hsGMatStateInline_inc
#define hsGMatStateInline_inc

#include "hsGMatState.h"
#include "hsStream.h"

void hsGMatState::Read(hsStream* s)
{
    fBlendFlags = s->ReadLE32();
    fClampFlags = s->ReadLE32();
    fShadeFlags = s->ReadLE32();
    fZFlags = s->ReadLE32();
    fMiscFlags = s->ReadLE32();
}

void hsGMatState::Write(hsStream* s)
{
    s->WriteLE32(fBlendFlags);
    s->WriteLE32(fClampFlags);
    s->WriteLE32(fShadeFlags);
    s->WriteLE32(fZFlags);
    s->WriteLE32(fMiscFlags);
}

void hsGMatState::Clear(const hsGMatState& state)
{
    fBlendFlags &= ~state.fBlendFlags;
    fClampFlags &= ~state.fClampFlags;
    fShadeFlags &= ~state.fShadeFlags;
    fZFlags &= ~state.fZFlags;
    fMiscFlags &= ~state.fMiscFlags;
}

void hsGMatState::Composite(const hsGMatState& want, const hsGMatState& on, const hsGMatState& off)
{
    fBlendFlags = want.fBlendFlags & ~off.fBlendFlags;
    if( !(fBlendFlags & (kBlendMask & ~(kBlendAlpha|kBlendAntiAlias))) )
        fBlendFlags |= on.fBlendFlags;

    fClampFlags = (want.fClampFlags | on.fClampFlags) & ~off.fClampFlags;
    
    fShadeFlags = (want.fShadeFlags | on.fShadeFlags) & ~off.fShadeFlags;
#if 0 // This restriction is only valid for glide - handle in glideDevice
    if( fBlendFlags & (kBlendAntiAlias | kBlendAlpha) )
        fShadeFlags &= ~(kShadeSpecularAlpha | kShadeSpecularHighlight);
#endif // This restriction is only valid for glide - handle in glideDevice

    fZFlags = (want.fZFlags | on.fZFlags) & ~off.fZFlags;
    fMiscFlags = (want.fMiscFlags | on.fMiscFlags) & ~off.fMiscFlags;
}

#endif // hsGMatStateInline_inc
