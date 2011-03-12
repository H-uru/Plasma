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

#include "hsTypes.h"
#include "plShadowCaster.h"
#include "../plMessage/plShadowCastMsg.h"

#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plDrawInterface.h"

#include "../plDrawable/plDrawableSpans.h"
#include "../plDrawable/plSpanTypes.h"

#include "../plSurface/hsGMaterial.h"
#include "../plSurface/plLayerInterface.h"

#include "../plMessage/plRenderMsg.h"

#include "plgDispatch.h"

#include "plShadowMaster.h"

hsBool plShadowCaster::fShadowCastDisabled = false;
hsBool plShadowCaster::fCanShadowCast = true;

plShadowCaster::plShadowCaster()
:	fMaxOpacity(0),
	fCastFlags(0),
	fAttenScale(1.f),
	fBlurScale(0.f),
	fBoost(1.f)
{
}

plShadowCaster::~plShadowCaster()
{
	Deactivate();
}

void plShadowCaster::Read(hsStream* stream, hsResMgr* mgr)
{
	plMultiModifier::Read(stream, mgr);

	fCastFlags = stream->ReadByte();

	// HACKTESTPERSP
//	if( !(fCastFlags & kPerspective) )
//		fCastFlags |= kPerspective;
//	else
		fCastFlags &= ~kPerspective;

	fBoost = stream->ReadSwapScalar();
	fAttenScale = stream->ReadSwapScalar();
	fBlurScale = stream->ReadSwapScalar();

	Activate();
}

void plShadowCaster::Write(hsStream* stream, hsResMgr* mgr)
{
	plMultiModifier::Write(stream, mgr);

	stream->WriteByte(fCastFlags);

	stream->WriteSwapScalar(fBoost);
	stream->WriteSwapScalar(fAttenScale);
	stream->WriteSwapScalar(fBlurScale);
}

void plShadowCaster::Activate() const
{
	plgDispatch::Dispatch()->RegisterForExactType(plRenderMsg::Index(), GetKey());
}

void plShadowCaster::Deactivate() const
{
	plgDispatch::Dispatch()->UnRegisterForExactType(plRenderMsg::Index(), GetKey());
}

void plShadowCaster::ICollectAllSpans()
{
	fSpans.SetCount(0);
	int i;
	for( i = 0; i < GetNumTargets(); i++ )
	{
		plSceneObject* so = GetTarget(i);
		// Nil target? Shouldn't happen.
		if( so )
		{
			const plDrawInterface* di = so->GetDrawInterface();
			// Nil di- either it hasn't loaded yet, or we've been applied to something that isn't visible (oops).
			if( di && !di->GetProperty(plDrawInterface::kDisable) )
			{
				int j;
				for( j = 0; j < di->GetNumDrawables(); j++ )
				{
					plDrawableSpans* dr = plDrawableSpans::ConvertNoRef(di->GetDrawable(j));
					// Nil dr - it hasn't loaded yet.
					if( dr )
					{
						plDISpanIndex& diIndex = dr->GetDISpans(di->GetDrawableMeshIndex(j));
						if( !diIndex.IsMatrixOnly() )
						{
							int k;
							for( k = 0; k < diIndex.GetCount(); k++ )
							{
								const plSpan* span = dr->GetSpan(diIndex[k]);
//								if( !(span->fProps & plSpan::kPropNoShadowCast) )
								{
									fSpans.Append(DrawSpan().Set(dr, span, diIndex[k]));
								}
							}
						}
					}
				}
			}
		}
	}
}

hsBool plShadowCaster::IOnRenderMsg(plRenderMsg* msg)
{
	if( ShadowCastDisabled() )
		return true;

	const UInt8 shadowQuality = UInt8(plShadowMaster::GetGlobalShadowQuality() * 3.9f);
	if( !GetKey()->GetUoid().GetLoadMask().MatchesQuality(shadowQuality) )
		return true;

	// Don't really like having to gather these guys up every frame,
	// but with the avatar customization, it's all pretty volatile,
	// subject to infrequent change, but change without warning.
	// The number of actual targets (and hence shadow casting spans)
	// for any ShadowCasterModifier should always be on the order of
	// 10, so chances are we can get away with this. If not, we can
	// figure some way of caching, like a broadcast message warning us
	// that an avatar customization event has occurred.
	ICollectAllSpans();

	// Max opacity used to fade out shadows during link

	//find max opacity of all spans
	//clear shadowBits of all spans
	fMaxOpacity = 0.f;
	int i;
	for( i = 0; i < fSpans.GetCount(); i++ )
	{
		hsGMaterial* mat = fSpans[i].fDraw->GetSubMaterial(fSpans[i].fSpan->fMaterialIdx);
		if( mat )
		{
			plLayerInterface* baseLay = mat->GetLayer(0);
			if( baseLay && (baseLay->GetOpacity() > fMaxOpacity) )
				fMaxOpacity = baseLay->GetOpacity();
		}

		fSpans[i].fSpan->ClearShadowBits();
	}


	if( fMaxOpacity > 0 )
	{
		plShadowCastMsg* cast = TRACKED_NEW plShadowCastMsg(GetKey(), this, msg->Pipeline());
		cast->Send();
	}

	return true;
}


#include "plProfile.h"
plProfile_CreateTimer("ShadowCaster", "RenderSetup", ShadowCaster);

hsBool plShadowCaster::MsgReceive(plMessage* msg)
{
	plRenderMsg* rendMsg = plRenderMsg::ConvertNoRef(msg);
	if( rendMsg )
	{
		plProfile_BeginLap(ShadowCaster, this->GetKey()->GetUoid().GetObjectName());
		IOnRenderMsg(rendMsg);
		plProfile_EndLap(ShadowCaster, this->GetKey()->GetUoid().GetObjectName());
		return true;
	}

	return plMultiModifier::MsgReceive(msg);
}
