
#ifndef hsGMatStateInline_inc
#define hsGMatStateInline_inc

#include "hsGMatState.h"
#include "hsStream.h"

void hsGMatState::Read(hsStream* s)
{
	fBlendFlags = s->ReadSwap32();
	fClampFlags = s->ReadSwap32();
	fShadeFlags = s->ReadSwap32();
	fZFlags = s->ReadSwap32();
	fMiscFlags = s->ReadSwap32();
}

void hsGMatState::Write(hsStream* s)
{
	s->WriteSwap32(fBlendFlags);
	s->WriteSwap32(fClampFlags);
	s->WriteSwap32(fShadeFlags);
	s->WriteSwap32(fZFlags);
	s->WriteSwap32(fMiscFlags);
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
