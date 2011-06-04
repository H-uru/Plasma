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
#include "plOccluder.h"
#include "hsStream.h"
#include "plOccluderProxy.h"
#include "../plDrawable/plDrawableGenerator.h"
#include "../plSurface/hsGMaterial.h"
#include "../plSurface/plLayerInterface.h"
#include "../plSurface/plLayer.h"
#include "../pnMessage/plNodeRefMsg.h"
#include "../pnKeyedObject/plKey.h"
#include "hsResMgr.h"
#include "plgDispatch.h"
#include "plVisRegion.h"
#include "plVisMgr.h"

plOccluder::plOccluder()
:	fSceneNode(nil)
{
	fProxyGen = TRACKED_NEW plOccluderProxy;
	fProxyGen->Init(this);

	fVisSet.SetBit(0);
}

plOccluder::~plOccluder()
{
	delete fProxyGen;
}

hsBool plOccluder::MsgReceive(plMessage* msg)
{
	plGenRefMsg* refMsg = plGenRefMsg::ConvertNoRef(msg);
	if( refMsg )
	{
		switch( refMsg->fType )
		{
		case kRefVisRegion:
			if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
			{
				IAddVisRegion(plVisRegion::ConvertNoRef(refMsg->GetRef()));
			}
			else
			{
				IRemoveVisRegion(plVisRegion::ConvertNoRef(refMsg->GetRef()));
			}
			return true;
		default:
			break;
		}

	}
	return plObjInterface::MsgReceive(msg);
}

void plOccluder::IAddVisRegion(plVisRegion* reg)
{
	if( reg )
	{
		int idx = fVisRegions.Find(reg);
		if( fVisRegions.kMissingIndex == idx )
		{
			fVisRegions.Append(reg);
			if( reg->GetProperty(plVisRegion::kIsNot) )
				fVisNot.SetBit(reg->GetIndex());
			else
			{
				fVisSet.SetBit(reg->GetIndex());
				if( reg->ReplaceNormal() )
					fVisSet.ClearBit(plVisMgr::kNormal);
			}
		}
	}
}

void plOccluder::IRemoveVisRegion(plVisRegion* reg)
{
	if( reg )
	{
		int idx = fVisRegions.Find(reg);
		if( fVisRegions.kMissingIndex != idx )
		{
			fVisRegions.Remove(idx);
			if( reg->GetProperty(plVisRegion::kIsNot) )
				fVisNot.ClearBit(reg->GetIndex());
			else
				fVisSet.ClearBit(reg->GetIndex());
		}
	}
}

plDrawableSpans* plOccluder::CreateProxy(hsGMaterial* mat, hsTArray<UInt32>& idx, plDrawableSpans* addTo)
{
	hsTArray<hsPoint3>		pos;
	hsTArray<hsVector3>		norm;
	hsTArray<hsColorRGBA>	color;
	hsTArray<UInt16>		tris;

	plLayer* lay = plLayer::ConvertNoRef(mat->GetLayer(0)->BottomOfStack());
	if( lay )
		lay->SetMiscFlags(lay->GetMiscFlags() & ~hsGMatState::kMiscTwoSided);

	const hsTArray<plCullPoly>& polys = GetLocalPolyList();
	int i;
	for( i = 0; i < polys.GetCount(); i++ )
	{
		hsColorRGBA col;
		if( polys[i].IsHole() )
			col.Set(0,0,0,1.f);
		else
			col.Set(1.f, 1.f, 1.f, 1.f);

		int triStart = tris.GetCount();

		int idx0 = pos.GetCount();
		pos.Append(polys[i].fVerts[0]);
		norm.Append(polys[i].fNorm);
		color.Append(col);
		pos.Append(polys[i].fVerts[1]);
		norm.Append(polys[i].fNorm);
		color.Append(col);
		int j;
		for( j = 2; j < polys[i].fVerts.GetCount(); j++ )
		{
			int idxCurr = pos.GetCount();
			pos.Append(polys[i].fVerts[j]);
			norm.Append(polys[i].fNorm);
			color.Append(col);
			tris.Append(idx0);
			tris.Append(idxCurr-1);
			tris.Append(idxCurr);
		}
#if 1
		if( polys[i].IsTwoSided() )
		{
			int n = tris.GetCount();
			while( --n >= triStart )
			{
				int idx = tris[n];
				tris.Append(idx);
			}
		}
#endif
	}
	return plDrawableGenerator::GenerateDrawable(pos.GetCount(), 
										pos.AcquireArray(),
										norm.AcquireArray(),
										nil, 0,
										color.AcquireArray(),
										true,
										nil,
										tris.GetCount(),
										tris.AcquireArray(),
										mat,
										GetLocalToWorld(),
										true,
										&idx,
										addTo);
}

void plOccluder::SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l)
{
// Commenting out the following asserts. Although they are fundamentally correct, 
//essentially identity matrices which aren't so flagged (because of numerical
// precision) are triggering bogus asserts. mf
//	hsAssert(l2w.fFlags & hsMatrix44::kIsIdent, "Non-identity transform to non-movable Occluder");
//	hsAssert(w2l.fFlags & hsMatrix44::kIsIdent, "Non-identity transform to non-movable Occluder");
}

const hsMatrix44& plOccluder::GetLocalToWorld() const
{
	return hsMatrix44::IdentityMatrix();
}

const hsMatrix44& plOccluder::GetWorldToLocal() const
{
	return hsMatrix44::IdentityMatrix();
}

void plOccluder::ComputeFromPolys()
{
	IComputeBounds();
	IComputeSurfaceArea();
}

void plOccluder::IComputeBounds()
{
	fWorldBounds.MakeEmpty();

	const hsTArray<plCullPoly>& polys = GetLocalPolyList();
	int i;
	for( i =0 ; i < polys.GetCount(); i++ )
	{
		int j;
		for( j = 0; j < polys[i].fVerts.GetCount(); j++ )
			fWorldBounds.Union(&polys[i].fVerts[j]);
	}
}

hsScalar plOccluder::IComputeSurfaceArea()
{
	hsScalar area = 0;
	const hsTArray<plCullPoly>& polys = GetLocalPolyList();
	int i;
	for( i =0 ; i < polys.GetCount(); i++ )
	{
		int j;
		for( j = 2; j < polys[i].fVerts.GetCount(); j++ )
		{
			area += (hsVector3(&polys[i].fVerts[j], &polys[i].fVerts[j-2]) % hsVector3(&polys[i].fVerts[j-1], &polys[i].fVerts[j-2])).Magnitude();
		}
	}
	area *= 0.5f;

	return fPriority = area;
}

void plOccluder::SetPolyList(const hsTArray<plCullPoly>& list)
{
	UInt16 n = list.GetCount();
	fPolys.SetCount(n);
	int i;
	for( i = 0; i < n; i++ )
		fPolys[i] = list[i];
}

void plOccluder::ISetSceneNode(plKey node)
{
	if( fSceneNode != node )
	{
		if( node )
		{
			plNodeRefMsg* refMsg = TRACKED_NEW plNodeRefMsg(node, plRefMsg::kOnCreate, -1, plNodeRefMsg::kOccluder);
			hsgResMgr::ResMgr()->AddViaNotify(GetKey(), refMsg, plRefFlags::kPassiveRef);
		}
		if( fSceneNode )
		{
			fSceneNode->Release(GetKey());
		}
		fSceneNode = node;
	}
}

void plOccluder::Read(hsStream* s, hsResMgr* mgr)
{
	plObjInterface::Read(s, mgr);

	fWorldBounds.Read(s);
	fPriority = s->ReadSwapScalar();

	hsTArray<plCullPoly>& localPolys = IGetLocalPolyList();
	UInt16 n = s->ReadSwap16();
	localPolys.SetCount(n);
	int i;
	for( i = 0; i < n; i++ )
		localPolys[i].Read(s, mgr);

	plKey nodeKey = mgr->ReadKey(s);
	ISetSceneNode(nodeKey);

	n = s->ReadSwap16();
	fVisRegions.SetCountAndZero(n);
	for( i = 0; i < n; i++ )
		mgr->ReadKeyNotifyMe(s, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kRefVisRegion), plRefFlags::kActiveRef);
}

void plOccluder::Write(hsStream* s, hsResMgr* mgr)
{
	plObjInterface::Write(s, mgr);

	fWorldBounds.Write(s);
	s->WriteSwapScalar(fPriority);

	const hsTArray<plCullPoly>& localPolys = IGetLocalPolyList();
	s->WriteSwap16(localPolys.GetCount());
	int i;
	for( i = 0; i < localPolys.GetCount(); i++ )
		localPolys[i].Write(s, mgr);

	mgr->WriteKey(s, fSceneNode);

	s->WriteSwap16(fVisRegions.GetCount());
	for( i = 0; i < fVisRegions.GetCount(); i++ )
		mgr->WriteKey(s, fVisRegions[i]);
}

plMobileOccluder::plMobileOccluder()
{
	fLocalToWorld.Reset();
	fWorldToLocal.Reset();
}

plMobileOccluder::~plMobileOccluder()
{
}

void plMobileOccluder::IComputeBounds()
{
	plOccluder::IComputeBounds();
	fLocalBounds = fWorldBounds;
	fWorldBounds.Transform(&fLocalToWorld);
}

void plMobileOccluder::SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l)
{
	fLocalToWorld = l2w;
	fWorldToLocal = w2l;

	if( fPolys.GetCount() != fOrigPolys.GetCount() )
		fPolys.SetCount(fOrigPolys.GetCount());

	int i;
	for( i = 0; i < fPolys.GetCount(); i++ )
		fOrigPolys[i].Transform(l2w, w2l, fPolys[i]);

	if( fProxyGen )
		fProxyGen->SetTransform(l2w, w2l);
}

void plMobileOccluder::SetPolyList(const hsTArray<plCullPoly>& list)
{
	UInt16 n = list.GetCount();
	fOrigPolys.SetCount(n);
	fPolys.SetCount(n);

	int i;
	for( i = 0; i < n; i++ )
	{
		fPolys[i] = fOrigPolys[i] = list[i];
	}
}

void plMobileOccluder::Read(hsStream* s, hsResMgr* mgr)
{
	plOccluder::Read(s, mgr);

	fLocalToWorld.Read(s);
	fWorldToLocal.Read(s);

	fLocalBounds.Read(s);

	fPolys.SetCount(fOrigPolys.GetCount());

	SetTransform(fLocalToWorld, fWorldToLocal);
}

void plMobileOccluder::Write(hsStream* s, hsResMgr* mgr)
{
	plOccluder::Write(s, mgr);

	fLocalToWorld.Write(s);
	fWorldToLocal.Write(s);

	fLocalBounds.Write(s);
}

void plMobileOccluder::ComputeFromPolys()
{
	SetTransform(fLocalToWorld, fWorldToLocal);
	plOccluder::ComputeFromPolys();
}

