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

#include "hsWindows.h"
#include <commdlg.h>

#include "Max.h"
#include "stdmat.h"
#include "bmmlib.h"
#include "iparamb2.h"
#include "meshdlib.h" 

#include "hsTypes.h"

#include <vector>
#include <algorithm>

#include "../MaxMain/plMaxNode.h"
#include "../MaxComponent/plComponent.h"
#include "../MaxComponent/plLightGrpComponent.h"
#include "../MaxComponent/plSoftVolumeComponent.h"

#include "plClusterUtil.h"

#include "../plDrawable/plClusterGroup.h"
#include "../plDrawable/plCluster.h"
#include "../plDrawable/plSpanTemplate.h"
#include "../plDrawable/plSpanInstance.h"
#include "../plDrawable/plGeometrySpan.h"

#include "../plSurface/hsGMaterial.h"
#include "../plSurface/plLayer.h"
#include "../plScene/plVisRegion.h"
#include "../plGLight/plLightInfo.h"

#include "plMeshConverter.h"
#include "hsVertexShader.h"
#include "plLightMapGen.h"

#include "hsResMgr.h"
#include "../pnKeyedObject/plUoid.h"

#include "../pnMessage/plNodeRefMsg.h"

#include "plTweak.h"

plConst(int) kDefMinFaces(200);
plConst(int) kDefMaxFaces(1000);
plConst(hsScalar) kDefMinSize(50.f);

plClusterUtil::plClusterUtil()
:	fGroup(nil),
	fTemplNode(nil),
	fTemplate(nil),
	fMinFaces(kDefMinFaces),
	fMaxFaces(kDefMaxFaces),
	fMinSize(kDefMinSize),
	fIdx(0)
{
}

plClusterUtil::~plClusterUtil()
{
}

plClusterGroup* plClusterUtil::CreateGroup(plMaxNode* templNode, const char* name)
{
	plClusterGroup* retVal = TRACKED_NEW plClusterGroup;
	
	char buff[256];
	sprintf(buff, "%s_%s_%d", name, templNode->GetName(), fIdx++);
	hsgResMgr::ResMgr()->NewKey(buff, retVal, templNode->GetLocation(), templNode->GetLoadMask());

	plKey sceneNode = templNode->GetRoomKey();
	retVal->SetSceneNode(sceneNode);

	plNodeRefMsg* refMsg = TRACKED_NEW plNodeRefMsg(sceneNode, plRefMsg::kOnCreate, -1, plNodeRefMsg::kGeneric);
	hsgResMgr::ResMgr()->AddViaNotify(retVal->GetKey(), refMsg, plRefFlags::kActiveRef);

	return retVal;
}

plClusterGroup* plClusterUtil::SetupGroup(plClusterGroup *group, plMaxNode* templNode, plSpanTemplateB* templ)
{
	fTemplNode = templNode;
	fGroup = group;
	fTemplate = templ;

	fGroup->fTemplate = templ;
	fGroup->ISendToSelf(plClusterGroup::kRefMaterial, templ->fMaterial);

	fGroup->fRenderLevel = templ->fRenderLevel;

	fMinInsts = fMinFaces / templ->NumTris();
	fMaxInsts = fMaxFaces / templ->NumTris();
	if( fMinInsts < 1 )
		fMinInsts = 1;
	if( fMaxInsts <= fMinInsts )
		fMaxInsts = fMinInsts+1;

	// STUB
	// Finish setting up the group here (lights, visregions, LOD), extracting all info
	// from the template node.
	ISetupGroupFromTemplate(templNode);

	return fGroup;
}

void plClusterUtil::ISetupGroupFromTemplate(plMaxNode* templ)
{
	plLightGrpComponent* liGrp = plLightGrpComponent::GetComp(templ);
	if( liGrp )
	{
		const hsTArray<plLightInfo*>& lights = liGrp->GetLightInfos();
		int i;
		for( i = 0; i < lights.GetCount(); i++ )
		{
			fGroup->ISendToSelf(plClusterGroup::kRefLight, lights[i]);
		}
	}
	if( templ->HasFade() )
	{
		hsScalar maxDist = 0;
		hsScalar minDist = 0;

		Box3 fade = templ->GetFade();
		const hsScalar kMaxMaxDist = 1.e10f;
		if( fade.Min()[2] < 0 )
		{
			minDist = fade.Min()[0];
			maxDist = kMaxMaxDist;
		}

		if( fade.Max()[2] > 0 )
			maxDist = fade.Max()[0];

		if( maxDist > minDist )
		{
			fGroup->fLOD.Set(minDist, maxDist);
		}
	}
	hsTArray<plVisRegion*> regions;
	plVisRegionComponent::CollectRegions(templ, regions);
	plEffVisSetComponent::CollectRegions(templ, regions);
	if( regions.GetCount() )
	{
		int i;
		for( i = 0; i < regions.GetCount(); i++ )
		{
			fGroup->ISendToSelf(plClusterGroup::kRefRegion, regions[i]);
		}
	}
}

class sortData
{
public:
	UInt16			fIdx0;
	UInt16			fIdx1;
	UInt16			fIdx2;
	hsScalar		fDist;

	sortData() {}
	sortData(UInt16 idx0, UInt16 idx1, UInt16 idx2, hsScalar dist)
		: fIdx0(idx0), fIdx1(idx1), fIdx2(idx2), fDist(dist)
	{
	}

	bool operator<(const sortData& ot) const { return fDist < ot.fDist; }
	bool operator>(const sortData& ot) const { return fDist > ot.fDist; }
	bool operator==(const sortData& ot) const { return fDist == ot.fDist; }
	bool operator!=(const sortData& ot) const { return fDist != ot.fDist; }
};

void plClusterUtil::ISortTemplate(plSpanTemplateB* templ) const
{
	UInt16* indexData = templ->fIndices;
	const int numTris = templ->NumTris();
	typedef std::vector<sortData> sortVec;
	sortVec vec;
	vec.resize(numTris);
	sortVec::iterator iter;
	for( iter = vec.begin(); iter != vec.end(); iter++ )
	{
		iter->fIdx0 = indexData[0];
		iter->fIdx1 = indexData[1];
		iter->fIdx2 = indexData[2];
		hsPoint3 pos;
		pos = *templ->Position(indexData[0]);
		float dist0 = pos.fX * pos.fX + pos.fY * pos.fY;
		pos = *templ->Position(indexData[1]);
		float dist1 = pos.fX * pos.fX + pos.fY * pos.fY;
		pos = *templ->Position(indexData[2]);
		float dist2 = pos.fX * pos.fX + pos.fY * pos.fY;

		iter->fDist = dist0 > dist1
			? (dist0 > dist2
				? dist0
				: dist2)
			: (dist1 > dist2
				? dist1
				: dist2);

		indexData += 3;
	}

	std::sort(vec.begin(), vec.end(), std::less<sortData>());

	indexData = templ->fIndices;
	for( iter = vec.begin(); iter != vec.end(); iter++ )
	{
		indexData[0] = iter->fIdx0;
		indexData[1] = iter->fIdx1;
		indexData[2] = iter->fIdx2;
		
		indexData += 3;
	}
}

void plClusterUtil::ITemplateFromGeo(plSpanTemplateB* templ, plGeometrySpan* geo)
{
	UInt16 format = plSpanTemplate::MakeFormat(
		true, // hasColor
		geo->GetNumUVs(), // UVW count
		geo->fFormat & plGeometrySpan::kSkinIndices, // hasWgtIdx
		(geo->fFormat & plGeometrySpan::kSkinWeightMask) >> 4, // NumWeights
		true, // hasNorm
		true // hasPos;
		);
	

	UInt32 numVerts = geo->fNumVerts;
	UInt32 numTris = geo->fNumIndices / 3;
	
	// Alloc it.
	templ->Alloc(format, numVerts, numTris);
	templ->AllocColors();

	UInt32 numPos = templ->NumPos();
	UInt32 numNorm = templ->NumNorm();
	UInt32 numUVWs = templ->NumUVWs();
	UInt32 numWeights = templ->NumWeights();
	UInt32 numColor = templ->NumColor();
	UInt32 numColor2 = templ->NumColor2();
	UInt32 numWgtIdx = templ->NumWgtIdx();

	// Fill in the data.
	memcpy(templ->fIndices, geo->fIndexData, templ->IndexSize());

	int i;
	for( i = 0; i < templ->NumVerts(); i++ )
	{
		float wgt[4];
		UInt32 wgtIdx;

		geo->ExtractInitColor(i, templ->MultColor(i), templ->AddColor(i));

		hsColorRGBA color;
		geo->ExtractVertex(i, templ->Position(i), templ->Normal(i), &color);

		if( templ->NumColor() )
			*templ->Color(i) = color.ToARGB32();
		if( templ->NumColor2() )
			*templ->Color2(i) = 0; 

		int k;
		for( k = 0; k < templ->NumUVWs(); k++ )
		{
			geo->ExtractUv(i, k, templ->UVWs(i, k));
		}
		
		if( templ->NumWeights() )
		{
			geo->ExtractWeights(i, wgt, &wgtIdx);
			int j;
			for( j = 0; j < templ->NumWeights(); j++ )
				*templ->Weight(i, j) = wgt[j];
			if( templ->NumWgtIdx() )
				*templ->WgtIdx(i) = wgtIdx;
		}
	}

	// Compute the local bounds.
	templ->ComputeBounds();

	ISortTemplate(templ);
}

plSpanTemplateB* plClusterUtil::IAddTemplate(plMaxNode* templNode, plGeometrySpan* geo)
{
	// Shade our mesh.
	// STUB

	// Create our blank template
	plSpanTemplateB* templ = TRACKED_NEW plSpanTemplateB(templNode);

	templ->fRenderLevel = templNode->GetRenderLevel(!templNode->GetNoDeferDraw());

	ITemplateFromGeo(templ, geo);

	return templ;
}

void plClusterUtil::IAddTemplates(plMaxNode* templNode, plSpanTemplTab& templs)
{
	// STUB
	// Get the Mesh

	// Figure the format and total number of verts.
	// If we're lazy or pressed for time we could use MeshConverter.
	// But we'd probably spend more time undoing MeshConverter hacks than if we start
	// from scratch.
	// But, here we go descending cheerully into hell.
	// At least with this interface we can bail and do it right later without to much
	// bloodshed.
	hsTArray<plGeometrySpan*> spanArray;
	if( !plMeshConverter::Instance().CreateSpans(templNode, spanArray, false) )
		return;

	plLightMapGen::Instance().Open(::GetCOREInterface(), ::GetCOREInterface()->GetTime(), false);
	hsVertexShader::Instance().Open();

	hsVertexShader::Instance().ShadeNode(templNode, 
				templNode->GetLocalToWorld44(), templNode->GetWorldToLocal44(), 
				spanArray);

	plLightMapGen::Instance().Close();
	hsVertexShader::Instance().Close();

	int i;
	for( i = 0; i < spanArray.GetCount(); i++ )
	{
		plSpanTemplateB* templ = IAddTemplate(templNode, spanArray[i]);
		templs.Append(1, &templ);
		templ->fMaterial = spanArray[i]->fMaterial;

		delete spanArray[i];
	}
}

Box3 plClusterUtil::IBound(const plL2WTab& src) const
{
	Box3 box;
	int i;
	for( i = 0; i < src.Count(); i++ )
	{
		box += src[i].GetTrans();
	}
	return box;
}

Point3 plClusterUtil::ILength(const plL2WTab& src) const
{
	Box3 box = IBound(src);
	return box.Max() - box.Min();
}

int plClusterUtil::ISelectAxis(const plL2WTab& src) const
{
	Box3 box = IBound(src);
	Point3 del = box.Max() - box.Min();
	if( del.x > del.y )
	{
		if( del.x > del.z )
			return 0;
		else
			return 2;
	}
	if( del.y > del.z )
		return 1;

	return 2;
}

static int sortAxis = 0;
static int cmp(const void *elem1, const void *elem2) 
{
	Matrix3* m1 = (Matrix3*) elem1;
	Matrix3* m2 = (Matrix3*) elem2;

	float d1 = m1->GetTrans()[sortAxis];
	float d2 = m2->GetTrans()[sortAxis];

	if( d1 < d2 )
		return -1;
	else if( d1 > d2 )
		return 1;

	return 0;
}


hsBool plClusterUtil::ISplitCluster(plSpanTemplateB* templ, plL2WTab& src, plL2WTab& lo, plL2WTab& hi)
{
	// Tried this, seems to work pretty well, but a more even grid is probably wiser at
	// this point.
#if 0 // MAX_SEP
	if( src.Count() <= fMinInsts)
		return false;

	// Pick an axis
	sortAxis = ISelectAxis(src);

	if( src.Count() < fMaxInsts)
	{
		Point3 len = ILength(src);
		if( len[sortAxis] < fMinSize )
			return false;
	}

	// Sort by that axis
	src.Sort(cmp);

	// Find the biggest gap
	float maxDist = 0;
	int pivot = 0;
	int i;
	for( i = 1; i < src.Count(); i++ )
	{
		float dist = src[i].GetTrans()[sortAxis] - src[i-1].GetTrans()[sortAxis];
		if( dist > maxDist )
		{
			maxDist = dist;
			pivot = i;
		}
	}
	hsAssert((pivot > 0) && (pivot < src.Count()), "Invalid pivot found");

	// Put everyone above it in hi, below it in lo
	lo.Append(pivot, src.Addr(0));
	hi.Append(src.Count()-pivot, src.Addr(pivot));

#else // MAX_SEP

	if( src.Count() <= fMinInsts )
		return false;

	// Pick an axis
	sortAxis = ISelectAxis(src);

	if( src.Count() < fMaxInsts)
	{
		Point3 len = ILength(src);
		if( len[sortAxis] < fMinSize )
			return false;
	}

	// Sort by that axis
	src.Sort(cmp);

	int pivot = src.Count() >> 1;
	lo.Append(pivot, src.Addr(0));
	hi.Append(src.Count()-pivot, src.Addr(pivot));
#endif // MAX_SEP

	return true;
}

void plClusterUtil::IFindClustersRecur(plSpanTemplateB* templ, plL2WTab& src, plL2WTabTab& dst)
{
	plL2WTab lo;
	plL2WTab hi;

	if( ISplitCluster(templ, src, lo, hi) )
	{
		// Keep going
		IFindClustersRecur(templ, lo, dst);
		IFindClustersRecur(templ, hi, dst);
	}
	else
	{
		plL2WTab* tab = TRACKED_NEW plL2WTab(src);
		dst.Append(1, &tab);
	}
}

void plClusterUtil::IFreeClustersRecur(plL2WTabTab& dst) const
{
	int i;
	for( i = 0; i < dst.Count(); i++ )
		delete dst[i];
}

inline hsScalar inlGetAlpha(UInt32* color)
{
	return hsScalar(*color >> 24) / 255.99f;
}

plSpanEncoding plClusterUtil::ISelectEncoding(plPoint3TabTab& delPosTab, plColorTabTab& colorsTab)
{
	hsBool hasColor = false;
	hsBool hasAlpha = false;
	hsScalar maxLenSq = 0;
	hsScalar maxX = 0;
	hsScalar maxY = 0;
	hsScalar maxZ = 0;
	int i;
	for( i = 0; i < delPosTab.Count(); i++ )
	{
		int j;
		if( delPosTab[i] )
		{
			plPoint3Tab& delPos = *delPosTab[i];
			for( j = 0; j < delPos.Count(); j++ )
			{
				hsScalar lenSq = delPos[j].MagnitudeSquared();
				if( lenSq > maxLenSq )
					maxLenSq = lenSq;
				hsScalar d = fabs(delPos[j].fX);
				if( d > maxX )
					maxX = d;
				d = fabs(delPos[j].fY);
				if( d > maxY )
					maxY = d;
				d = fabs(delPos[j].fZ);
				if( d > maxZ )
					maxZ = d;

			}
		}

		if( colorsTab[i] )
		{
			plColorTab& color = *colorsTab[i];
			for( j = 0; j < color.Count(); j++ )
			{
				UInt32 col = color[j];
				if( (col & 0x00ffffff) != 0x00ffffff )
					hasColor = true;
				if( (col & 0xff000000) != 0xff000000 )
					hasAlpha = true;
			}
		}
	}

	UInt32 code = 0;
	hsScalar posScale = 1.f;

	if( hasColor && hasAlpha )
		code |= plSpanEncoding::kColAI88;
	else if( hasColor )
		code |= plSpanEncoding::kColI8;
	else if( hasAlpha )
		code |= plSpanEncoding::kColA8;

	plConst(hsScalar) kPosQuantum(0.5 / 12.f); // 1/2 inch.
	hsScalar maxLen = hsSquareRoot(maxLenSq);
	if( maxLen > kPosQuantum )
	{
		if( (maxX < kPosQuantum) && (maxY < kPosQuantum) )
		{
			code |= plSpanEncoding::kPos008;
			posScale = maxLen / 255.9f;
		}
		else if( (maxLen / 255.9f) < kPosQuantum )
		{
			code |= plSpanEncoding::kPos888;
			posScale = maxLen / 255.9f;
		}
		else if( (maxLen / hsScalar(1 << 10)) < kPosQuantum )
		{
			code |= plSpanEncoding::kPos101010;
			posScale = maxLen / hsScalar(1 << 10);
		}
		else
		{
			code |= plSpanEncoding::kPos161616;
			posScale = maxLen / hsScalar(1 << 16);
		}
	}
	return plSpanEncoding(code, posScale);
}

static int CompTemplates(const void *elem1, const void *elem2) 
{
	plSpanTemplateB* templA = *((plSpanTemplateB**)elem1);
	plSpanTemplateB* templB = *((plSpanTemplateB**)elem2);

	hsScalar hA = templA->GetLocalBounds().GetMaxs().fZ;
	hsScalar hB = templB->GetLocalBounds().GetMaxs().fZ;

	if( hA < hB )
		return -1;

	if( hA > hB )
		return 1;

	return 0;
}

void plClusterUtil::ISortTemplates(plSpanTemplTab& templs) const
{
	templs.Sort(CompTemplates);

	float maxZ = -1.e33f;

	int i;
	for( i = 1; i < templs.Count(); i++ )
	{
		templs[i]->fRenderLevel.Set(templs[i-1]->fRenderLevel.Level() + 1);
	}
}

plSpanTemplTab plClusterUtil::MakeTemplates(INode* templNode)
{
	plSpanTemplTab templs;
	IAddTemplates((plMaxNode*)templNode, templs);
	ISortTemplates(templs);
	return templs;
}

void plClusterUtil::AddClusters(plL2WTab& insts, plDeformVert* def, plShadeVert* shade)
{
	plPoint3TabTab delPos;
	plColorTabTab colors;

	plL2WTabTab clusters;
	IFindClustersRecur(fTemplate, insts, clusters);

	int j;
	for( j = 0; j < clusters.Count(); j++ )
	{
		// Create a plCluster to hold them all.
		plCluster* cluster = fGroup->IAddCluster();

		// Get the delPositions and colors for all the instances
		IAllocPosAndColor(fTemplate, *clusters[j], delPos, colors);

		IDelPosAndColor(fTemplate, 
							*clusters[j],
							def, shade,
							delPos, colors);


		// Look through the results and pick out a proper encoding
		plSpanEncoding code = ISelectEncoding(delPos, colors);
		cluster->SetEncoding(code);

		// Now create, encode and add all the insts to the cluster.
		IAddInstsToCluster(cluster, fTemplate, *clusters[j], delPos, colors);

		IFreePosAndColor(delPos, colors);
	}
	
	IFreeClustersRecur(clusters);
}

void plClusterUtil::IAddInstsToCluster(plCluster* cluster, plSpanTemplateB* templ, 
									   const plL2WTab& insts, 
									   plPoint3TabTab& delPos, 
									   plColorTabTab& colors)
{
	int i;
	for( i = 0; i < insts.Count(); i++ )
	{
		plSpanInstance* span = TRACKED_NEW plSpanInstance;
		span->Alloc(cluster->GetEncoding(), templ->NumVerts());

		span->SetLocalToWorld(plMaxNodeBase::Matrix3ToMatrix44(insts[i]));

		span->Encode(cluster->GetEncoding(), templ->NumVerts(),
			delPos[i] ? delPos[i]->Addr(0) : nil,
			colors[i] ? colors[i]->Addr(0) : nil);

		cluster->IAddInst(span);
	}
}


void plClusterUtil::IAllocPosAndColor(plSpanTemplateB* templ, const plL2WTab& insts,
									plPoint3TabTab& delPos, plColorTabTab& colors)
{
	delPos.SetCount(insts.Count());
	colors.SetCount(insts.Count());

	const int numVerts = templ->NumVerts();
	int i;
	for( i = 0; i < insts.Count(); i++ )
	{
		delPos[i] = nil;
		colors[i] = nil;
	}
}

void plClusterUtil::IFreePosAndColor(plPoint3TabTab& delPos, plColorTabTab& colors) const
{
	int i;
	for( i = 0; i < delPos.Count(); i++ )
		delete delPos[i];
	for( i = 0; i < colors.Count(); i++ )
		delete colors[i];
}

void plClusterUtil::IDelPosAndColor(plSpanTemplateB* templ, 
									const plL2WTab& insts, plDeformVert* def, plShadeVert* shade,
									plPoint3TabTab& delPos, plColorTabTab& colors)
{

	hsBool doDef = def != nil;
	hsBool doCol = shade != nil;
	// For each inst
	int i;
	for( i = 0; i < insts.Count(); i++ )
	{
		hsBounds3Ext wBnd = templ->GetLocalBounds();
		hsMatrix44 l2w = plMaxNodeBase::Matrix3ToMatrix44(insts[i]);
		hsMatrix44 w2l;
		l2w.GetInverse(&w2l);
		hsMatrix44 w2lT;
		w2l.GetTranspose(&w2lT);

		wBnd.Transform(&l2w);

		if( doDef )
		{
			def->Begin(templ->GetSrcNode(), wBnd);

			delPos[i] = TRACKED_NEW plPoint3Tab;
			delPos[i]->SetCount(templ->NumVerts());
			int j;
			for( j = 0; j < templ->NumVerts(); j++ )
			{
				hsPoint3 p = l2w * *templ->Position(j);
				plPoint3Tab& dp = *delPos[i];
				dp[j] = def->GetDel(p);
				dp[j] = w2l * dp[j];
			}
			
			def->End();
		}



		// Make the stored colors the actual output UInt32.
		// templ has the mult and add colors, apply them here.
		if( doCol )
		{
			shade->Begin(templ->GetSrcNode(), wBnd);

			colors[i] = TRACKED_NEW plColorTab;
			colors[i]->SetCount(templ->NumVerts());
			int j;
			for( j = 0; j < templ->NumVerts(); j++ )
			{
				hsPoint3 pos = *templ->Position(j);
				pos += (*delPos[i])[j];
				pos = l2w * pos;

				hsVector3 norm = *templ->Normal(j);
				norm = w2lT * norm;
				
				Color rgb = shade->GetShade(pos, norm);

				rgb *= Color(templ->MultColor(j)->r, templ->MultColor(j)->g, templ->MultColor(j)->b);
				rgb += Color(templ->AddColor(j)->r, templ->AddColor(j)->g, templ->AddColor(j)->b);

				(*colors[i])[j] = hsColorRGBA().Set(rgb.r, rgb.g, rgb.b, templ->MultColor(j)->a).ToARGB32();
			}
			shade->End();
		}

	}
}

