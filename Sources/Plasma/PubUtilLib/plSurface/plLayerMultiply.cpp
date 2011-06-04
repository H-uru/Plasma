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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#include "plLayerMultiply.h"

plLayerMultiply::plLayerMultiply() : fDirtyChannels(0), fSrcOpacity(1.f)
{
	fSrcPreshadeColor.Set(1.f, 1.f, 1.f, 1.f);
	fSrcRuntimeColor.Set(1.f, 1.f, 1.f, 1.f);
	fSrcAmbientColor.Set(1.f, 1.f, 1.f, 1.f);
	fSrcTransform.Reset();
}

plLayerMultiply::~plLayerMultiply()
{
}

void plLayerMultiply::Read(hsStream* s, hsResMgr* mgr)
{
	plLayerInterface::Read(s, mgr);

	fOwnedChannels = s->ReadSwap32();
	if (fOwnedChannels & kOpacity)
	{
		fOpacity = TRACKED_NEW hsScalar;
		*fOpacity = fSrcOpacity = s->ReadSwapScalar();
		fDirtyChannels |= kOpacity;
	}
	
	if (fOwnedChannels & kPreshadeColor)
	{
		fPreshadeColor = TRACKED_NEW hsColorRGBA;
		fSrcPreshadeColor.Read(s);
		*fPreshadeColor = fSrcPreshadeColor;
		fDirtyChannels |= kPreshadeColor;
	}

	if (fOwnedChannels & kRuntimeColor)
	{
		fRuntimeColor = TRACKED_NEW hsColorRGBA;
		fSrcRuntimeColor.Read(s);
		*fRuntimeColor = fSrcRuntimeColor;
		fDirtyChannels |= kRuntimeColor;
	}

	if (fOwnedChannels & kAmbientColor)
	{
		fAmbientColor = TRACKED_NEW hsColorRGBA;
		fSrcAmbientColor.Read(s);
		*fAmbientColor = fSrcAmbientColor;
		fDirtyChannels |= kAmbientColor;
	}

	if (fOwnedChannels & kTransform)
	{
		fTransform = TRACKED_NEW hsMatrix44;
		fSrcTransform.Read(s);
		*fTransform = fSrcTransform;
		fDirtyChannels |= kTransform;
	}
}

void plLayerMultiply::Write(hsStream* s, hsResMgr* mgr)
{
	plLayerInterface::Write(s, mgr);

	s->WriteSwap32(fOwnedChannels);
	if (fOwnedChannels & kOpacity)
		s->WriteSwapScalar(fSrcOpacity);

	if (fOwnedChannels & kPreshadeColor)
		fSrcPreshadeColor.Write(s);

	if (fOwnedChannels & kRuntimeColor)
		fSrcRuntimeColor.Write(s);

	if (fOwnedChannels & kAmbientColor)
		fSrcAmbientColor.Write(s);

	if (fOwnedChannels & kTransform)
		fSrcTransform.Write(s);
}

plLayerInterface* plLayerMultiply::Attach(plLayerInterface* prev)
{
	return plLayerInterface::Attach(prev);
}

UInt32 plLayerMultiply::Eval(double wSecs, UInt32 frame, UInt32 ignore)
{
	UInt32 dirtyChannels = fDirtyChannels | plLayerInterface::Eval(wSecs, frame, ignore);
	UInt32 evalChannels = dirtyChannels & fOwnedChannels;

	if (evalChannels & kPreshadeColor)
		*fPreshadeColor = fSrcPreshadeColor * fUnderLay->GetPreshadeColor();

	if (evalChannels & kRuntimeColor)
		*fRuntimeColor = fSrcRuntimeColor * fUnderLay->GetRuntimeColor();

	if (evalChannels & kAmbientColor)
		*fAmbientColor = fSrcAmbientColor * fUnderLay->GetAmbientColor();

	if (evalChannels & kOpacity)
		*fOpacity = fSrcOpacity * fUnderLay->GetOpacity();

	if (evalChannels & kTransform)
		*fTransform = fSrcTransform * fUnderLay->GetTransform();

	fDirtyChannels = 0;
	return dirtyChannels;
}

hsBool plLayerMultiply::MsgReceive(plMessage* msg)
{
	return plLayerMultiply::MsgReceive(msg);
}

void plLayerMultiply::SetPreshadeColor(const hsColorRGBA& col)
{
	fSrcPreshadeColor = col;
	fDirtyChannels |= kPreshadeColor;
}

void plLayerMultiply::SetRuntimeColor(const hsColorRGBA& col)
{
	fSrcRuntimeColor = col;
	fDirtyChannels |= kRuntimeColor;
}

void plLayerMultiply::SetAmbientColor(const hsColorRGBA& col)
{
	fSrcAmbientColor = col;
	fDirtyChannels |= kAmbientColor;
}

void plLayerMultiply::SetOpacity(hsScalar a)
{
	fSrcOpacity = a;
	fDirtyChannels |= kOpacity;
}

void plLayerMultiply::SetTransform(const hsMatrix44& xfm)
{
	fSrcTransform = xfm;
	fDirtyChannels |= kTransform;
}