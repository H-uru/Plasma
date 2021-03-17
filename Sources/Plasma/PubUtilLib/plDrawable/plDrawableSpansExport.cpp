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
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  plDrawableSpans Class Export-only Functions                             //
//                                                                          //
//// Version History /////////////////////////////////////////////////////////
//                                                                          //
//  Created 4.3.2001 mcn                                                    //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "plDrawableSpans.h"
#include "hsStream.h"
#include "hsResMgr.h"
#include "plPipeline.h"
#include "plGeometrySpan.h"
#include "plGBufferGroup.h"

#include "plSpaceTree.h"
#include "plSpaceTreeMaker.h"       // This is fun and amusing and wonderful to have here.
                                    // Keep it here forever.
#include "plSurface/hsGMaterial.h"
#include "plPipeline/plFogEnvironment.h"
#include "pnMessage/plRefMsg.h"
#include "pnMessage/plNodeRefMsg.h" // for NodeRefMsg
#include "plMessage/plDeviceRecreateMsg.h"
#include "plSurface/hsGMaterial.h"
#include "plSurface/plLayerInterface.h"
#include "plGImage/plBitmap.h"
#include "plGLight/plLightInfo.h"
#include "plgDispatch.h"

#include "plStatusLog/plStatusLog.h"

//#define VERT_LOG

//// Write ///////////////////////////////////////////////////////////////////

void    plDrawableSpans::Write( hsStream* s, hsResMgr* mgr )
{
    // Make sure we're optimized before we write (should be tho)
//  Optimize();

    // Make sure all the garbage is cleaned up
    if( fNeedCleanup )
        IRemoveGarbage();

    // Parent write
    plDrawable::Write(s, mgr);

    s->WriteLE32( fProps );
    s->WriteLE32( fCriteria );
    s->WriteLE32( fRenderLevel.fLevel );

    /// Write out the material keys
    s->WriteLE32((uint32_t)fMaterials.size());
    for (hsGMaterial* material : fMaterials)
        mgr->WriteKey(s, material);

    /// Write out the icicles
    s->WriteLE32((uint32_t)fIcicles.size());
    for (plIcicle& icicle : fIcicles)
        icicle.Write(s);

    /// Write out the patches
    // FIXME MAJOR VERSION
    // no more patches, remove this line
    s->WriteLE32(0);

    /// Write out the index table based on the pointer array
    s->WriteLE32((uint32_t)fSpans.size());
    for (plSpan* span : fSpans)
    {
        uint8_t   *icicle = (uint8_t *)span, *base = (uint8_t *)fIcicles.data();
        uint32_t j = (uint32_t)( icicle - base ) / sizeof( plIcicle );
        s->WriteLE32( j );
    }

    /// Write out the common keys
    for (plSpan* span : fSpans)
    {
        // The fog environ key
        mgr->WriteKey(s, span->fFogEnvironment);
    }

    /// Write out the bounds and stuff
    if (!fSpans.empty())
    {
        fLocalBounds.Write(s);
        fWorldBounds.Write(s);
        fMaxWorldBounds.Write(s);
    }

    for (plSpan* span : fSpans)
    {
        if (span->fProps & plSpan::kPropHasPermaLights)
        {
            s->WriteLE32((uint32_t)span->fPermaLights.size());
            for (plLightInfo* permaLight : span->fPermaLights)
                mgr->WriteKey(s, permaLight);
        }
        if (span->fProps & plSpan::kPropHasPermaProjs)
        {
            s->WriteLE32((uint32_t)span->fPermaProjs.size());
            for (plLightInfo* permaProj : span->fPermaProjs)
                mgr->WriteKey(s, permaProj);
        }
    }

    /// Write out the source spans if necessary
    s->WriteLE32((uint32_t)fSourceSpans.size());
    for (plGeometrySpan* geoSpan : fSourceSpans)
        geoSpan->Write(s);

    s->WriteLE32((uint32_t)fLocalToWorlds.size());
    for (size_t i = 0; i < fLocalToWorlds.size(); i++)
    {
        fLocalToWorlds[i].Write(s);
        fWorldToLocals[i].Write(s);

        fLocalToBones[i].Write(s);
        fBoneToLocals[i].Write(s);
    }

    // Write out the drawInterface index arrays
    s->WriteLE32((uint32_t)fDIIndices.size());
    for (plDISpanIndex* array : fDIIndices)
    {
        s->WriteLE32( array->fFlags );
        s->WriteLE32((uint32_t)array->GetCount());
        for (size_t j = 0; j < array->GetCount(); j++)
            s->WriteLE32( (*array)[ j ] );
    }

    // Write the groups out
    s->WriteLE32((uint32_t)fGroups.size());
    for (size_t i = 0; i < fGroups.size(); i++)
    {
#ifdef VERT_LOG

        hsUNIXStream log;
        log.Open("log\\GBuf.log", "ab");
        char buf[256];
        sprintf(buf, "Drawable Span: %s, GroupNum: %u\r\n", GetKeyName().c_str(), i);
        log.WriteString(buf);
        log.Close();
#endif
        fGroups[ i ]->Write( s );
    }

    /// Other stuff
    mgr->WriteCreatable(s, fSpaceTree);

    mgr->WriteKey(s, fSceneNode);

    /// All done!
}

//// AddDISpans //////////////////////////////////////////////////////////////
//  Adds a drawInterface's geometry spans to the list to be collapsed into 
//  buffers.

uint32_t plDrawableSpans::AddDISpans(std::vector<plGeometrySpan *> &spans, uint32_t index)
{
    hsBounds3Ext    bounds;


    /// Do garbage cleanup first
    if( fNeedCleanup )
        IRemoveGarbage();

    if (index == (uint32_t)-1) // need a new one
    {
        /// Create a lookup entry
        index = (uint32_t)fDIIndices.size();
        fDIIndices.emplace_back(new plDISpanIndex);
        fDIIndices[ index ]->fFlags = plDISpanIndex::kNone;
    }
    plDISpanIndex   *spanLookup = fDIIndices[ index ];


    /// Add the geometry spans to our list. Also add our internal span
    /// copies
    for (plGeometrySpan* geoSpan : spans)
    {
        spanLookup->Append(fSourceSpans.size());
        geoSpan->fSpanRefIndex = fSourceSpans.size();
        fSourceSpans.emplace_back(geoSpan);

        size_t spanIdx = fIcicles.size();
        plSpan* span = (plSpan *)&fIcicles.emplace_back();

        /// Set common stuff
        IAssignMatIdxToSpan(span, geoSpan->fMaterial);
        span->fLocalToWorld = geoSpan->fLocalToWorld;
        span->fWorldToLocal = geoSpan->fWorldToLocal;
        span->fProps |= (geoSpan->fProps & plGeometrySpan::kPropRunTimeLight) ? plSpan::kPropRunTimeLight : 0;
        if (geoSpan->fProps & plGeometrySpan::kPropNoShadowCast)
            span->fProps |= plSpan::kPropNoShadowCast;
        if (geoSpan->fProps & plGeometrySpan::kPropNoShadow)
            span->fProps |= plSpan::kPropNoShadow;
        if (geoSpan->fProps & plGeometrySpan::kPropForceShadow)
            span->fProps |= plSpan::kPropForceShadow;
        if (geoSpan->fProps & plGeometrySpan::kPropReverseSort)
            span->fProps |= plSpan::kPropReverseSort;
        if (geoSpan->fProps & plGeometrySpan::kPartialSort)
            span->fProps |= plSpan::kPartialSort;
        if (geoSpan->fProps & plGeometrySpan::kVisLOS)
        {
            span->fProps |= plSpan::kVisLOS;
            fProps |= plDrawable::kPropHasVisLOS;
        }

        span->fNumMatrices = geoSpan->fNumMatrices;
        span->fBaseMatrix = geoSpan->fBaseMatrix;
        span->fLocalUVWChans = geoSpan->fLocalUVWChans;
        span->fMaxBoneIdx = geoSpan->fMaxBoneIdx;
        span->fPenBoneIdx = (uint16_t)(geoSpan->fPenBoneIdx);

        bounds = geoSpan->fLocalBounds;
        span->fLocalBounds = bounds;
        bounds.Transform( &span->fLocalToWorld );
        span->fWorldBounds = bounds;
        span->fFogEnvironment = geoSpan->fFogEnviron;

        /// Add to our source indices
        fSpans.emplace_back(span);
        fSpanSourceIndices.emplace_back(spanIdx);
    }

    /// Rebuild the pointer array
    IRebuildSpanArray();

    SetSpaceTree(nullptr);

    fOptimized = false;

    return index;
}

//// Optimize ////////////////////////////////////////////////////////////////

void    plDrawableSpans::Optimize()
{
    if( fOptimized )
        return;

    /// Sort all the source spans
    if( !(fProps & kPropNoReSort) )
        ISortSourceSpans();

    /// Pack the source spans into spans and indices
    IPackSourceSpans();

    /// Now that we're done with the source spans, get rid of them! (BLEAH!)
    for (plGeometrySpan* geoSpan : fSourceSpans)
        delete geoSpan;
    fSourceSpans.clear();

    if( fCriteria & kCritSortSpans )
    {
        if( !(fProps & kPropNoReSort) )
            fProps |= kPropSortSpans;
    }
    if( fCriteria & kCritSortFaces )
    {
        fProps |= kPropSortFaces;
    }

    /// Now we do a pass at the buffer groups, asking them to tidy up
    for (plGBufferGroup* group : fGroups)
        group->TidyUp();

    // Look to see if we have any materials we aren't using anymore.
    for (size_t i = 0; i < fMaterials.size(); i++)
    {
        ICheckToRemoveMaterial(i);
    }

    fReadyToRender = false;

    // Make the space tree (hierarchical bounds).
    plSpaceTreeMaker maker;
    maker.Reset();
    for (plSpan* span : fSpans)
    {
        maker.AddLeaf(span->fWorldBounds, span->fProps & plSpan::kPropNoDraw);
    }
    plSpaceTree* tree = maker.MakeTree();
    SetSpaceTree(tree);

    /// All done!
    fOptimized = true;
}

#ifdef VERT_LOG
static plStatusLog* IStartLog(const char* name, int numSpans)
{
    ST::string filename = ST::format("x{}.log", name);
    plStatusLog* statusLog = plStatusLogMgr::GetInstance().CreateStatusLog(
        plStatusLogMgr::kDefaultNumLines,
        filename,
        plStatusLog::kFilledBackground | plStatusLog::kDeleteForMe);
    return statusLog;
}

static plStatusLog* IEndLog(plStatusLog* statusLog)
{
    delete statusLog;
    return nullptr;
}

static void ILogSpan(plStatusLog* statusLog, plGeometrySpan* geo, plVertexSpan* span, plGBufferGroup* group)
{
    ST::string owner = geo->fMaxOwner.empty() ? ST_LITERAL("<unknown>") : geo->fMaxOwner;
    ST::string material = geo->fMaterial->GetKeyName().empty() ? ST_LITERAL("<unknown>") : geo->fMaterial->GetKeyName();
    if( span->fTypeMask & plSpan::kIcicleSpan )
    {
        plIcicle* ice = (plIcicle*)span;
        if( geo->fProps & plGeometrySpan::kFirstInstance )
        {
            plGBufferCell* cell = group->GetCell(span->fVBufferIdx, span->fCellIdx);
            uint32_t stride = group->GetVertexSize();
            uint32_t ptr = cell->fVtxStart + span->fCellOffset * stride;

            statusLog->AddLine("From obj <{}> mat <{}> size {} bytes grp={} ({} offset)",
                owner,
                material,
                geo->GetVertexSize(geo->fFormat) * geo->fNumVerts + sizeof(uint16_t) * geo->fNumIndices,
                span->fGroupIdx,
                ptr);
//              span->fVBufferIdx,
//              span->fCellIdx,
//              span->fCellOffset,
//              span->fVStartIdx,
//              span->fVLength,
//              ice->fIBufferIdx,
//              ice->fIStartIdx,
//              ice->fILength
        }
        else
        {
            statusLog->AddLine("Instanced obj <{}> mat <{}> grp={} ({}/{}/{}/{}/{}/{}/{}/{})",
                owner,
                material,
                span->fGroupIdx,
                span->fVBufferIdx,
                span->fCellIdx,
                span->fCellOffset,
                span->fVStartIdx,
                span->fVLength,
                ice->fIBufferIdx,
                ice->fIStartIdx,
                ice->fILength);
        }
    }
    else
    {
        if( geo->fProps & plGeometrySpan::kFirstInstance )
        {
            statusLog->AddLine("From obj <{}> mat <{}> size {} bytes grp={} ({}/{}/{}/{}/{})",
                owner,
                material,
                geo->GetVertexSize(geo->fFormat) * geo->fNumVerts + sizeof(uint16_t) * geo->fNumIndices,
                span->fGroupIdx,
                span->fVBufferIdx,
                span->fCellIdx,
                span->fCellOffset,
                span->fVStartIdx,
                span->fVLength);
        }
        else
        {
            statusLog->AddLine("Instanced obj <{}> mat <{}> grp={} ({}/{}/{}/{}/{})",
                owner,
                material,
                span->fGroupIdx,
                span->fVBufferIdx,
                span->fCellIdx,
                span->fCellOffset,
                span->fVStartIdx,
                span->fVLength);
        }
    }
}
#endif

//// IPackSourceSpans ////////////////////////////////////////////////////////
//  Takes the array of source spans and converts them to our internal icicle
//  spans, vertex buffers and index buffers.

void    plDrawableSpans::IPackSourceSpans()
{
    hsBounds3Ext    bounds;
    hsBitVector     doneSpans;


    /// Calc bounds
    fLocalBounds.MakeEmpty();
    fWorldBounds.MakeEmpty();

    for (plGeometrySpan* geoSpan : fSourceSpans)
    {
        hsBounds3Ext bnd = geoSpan->fLocalBounds;
        bnd.Transform(&geoSpan->fLocalToWorld);
        fWorldBounds.Union( &bnd );
    }

    fLocalBounds = fWorldBounds;
    fLocalBounds.Transform( &fWorldToLocal );
    fMaxWorldBounds = fWorldBounds;

    // It could be that instance refs in spans that we (the drawable) own
    // are actually spans in some other drawable. That's not currently handled.
    // (Making that case handled would involve rewriting the instancing implementation
    // to not suck ass).
    // So for each instance set, we make two lists of instance refs, the ones also
    // in this drawable (refsHere), and ones not (refsThere). If there are refsThere,
    // we split out the refsHere into a new instance group, removing them from the
    // refsThere list. If refsThere still contains spans from separate drawables, 
    // that will be dealt with in those drawables' Optimize calls.
    doneSpans.Clear();
    for (size_t i = 0; i < fSourceSpans.size(); i++)
    {
        if( !doneSpans.IsBitSet(i) )
        {
            plGeometrySpan* span = fSourceSpans[i];
            if( span->fInstanceRefs )
            {
                hsTArray<plGeometrySpan*>& refs = *(span->fInstanceRefs);
                hsTArray<plGeometrySpan*> refsHere;
                hsTArray<plGeometrySpan*> refsThere;

                int k;
                for( k = 0; k < refs.GetCount(); k++ )
                {
                    plGeometrySpan* other = refs[k];
                    if( other != span )
                    {
                        auto iter = std::find(fSourceSpans.cbegin(), fSourceSpans.cend(), other);
                        if (iter == fSourceSpans.cend())
                            refsThere.Append(other);
                        else
                            refsHere.Append(other);
                    }
                }
                if( refsThere.GetCount() )
                {
                    if( refsHere.GetCount() )
                    {
                        span->BreakInstance();

                        // Okay, got to form a new instance group out of refsHere.
                        for( k = 0; k < refsHere.GetCount(); k++ )
                        {
                            plGeometrySpan* other = refsHere[k];

                            other->ChangeInstance(span);

                            doneSpans.SetBit(other->fSpanRefIndex);
                        }
                    }
                    else
                    {
                        span->UnInstance();
                    }
                    if( refsThere.GetCount() == 1 )
                    {
                        refsThere[0]->UnInstance();
                    }
                }
            }
            doneSpans.SetBit(i);
        }
    }

    /// Now pack the spans
    doneSpans.Clear();
    for (size_t i = 0; i < fSourceSpans.size(); i++)
    {
        // Now we fill the rest of the data in for our span
        if( !doneSpans.IsBitSet( i ) )
        {
            if( fSourceSpans[ i ]->fProps & plGeometrySpan::kInstanced )
            {
                // Instanced spans--convert the first as normal, then convert the rest
                // using the first as reference
                doneSpans.SetBit( i, true );

                plIcicle    *baseIcicle = (plIcicle *)fSpans[ i ];
                IConvertGeoSpanToIcicle( fSourceSpans[ i ], baseIcicle, 0, baseIcicle );

                // Loop through the rest
                for (int j = 0; j < fSourceSpans[i]->fInstanceRefs->GetCount(); j++)
                {
                    plGeometrySpan *other = (*fSourceSpans[ i ]->fInstanceRefs)[ j ];
                    if( other == fSourceSpans[ i ] )
                        continue;

#if 0 // What exactly is this supposed to be doing? My guess is, NADA.
                    if( IConvertGeoSpanToIcicle( other, (plIcicle *)fSpans[ other->fSpanRefIndex ], 0, baseIcicle ) )
                        baseIcicle = (plIcicle *)fSpans[ other->fSpanRefIndex ];
#else // What exactly is this supposed to be doing? My guess is, NADA.
                    IConvertGeoSpanToIcicle( other, (plIcicle *)fSpans[ other->fSpanRefIndex ], 0, baseIcicle );
#endif // What exactly is this supposed to be doing? My guess is, NADA.
                    doneSpans.SetBit( other->fSpanRefIndex, true );
                }
    
            }
            else
            {
                // Do normal, uninstanced conversion
                IConvertGeoSpanToIcicle( fSourceSpans[ i ], (plIcicle *)fSpans[ i ], 0 );   
                doneSpans.SetBit( i, true );
            }
        }
    }

#ifdef VERT_LOG
    hsTArray<plGeometrySpan*> order;
    order.SetCount(fSourceSpans.GetCount());
    for (plGeometrySpan* geoSpan : fSourceSpans)
    {
        order[geoSpan->fSpanRefIndex] = geoSpan;
    }

    plStatusLog* statusLog = IStartLog(GetKey()->GetName(), fSourceSpans.GetCount());
    for (int i = 0; i < order.GetCount(); i++)
    {
        plVertexSpan* vSpan = (plVertexSpan*)fSpans[i];
        ILogSpan(statusLog, order[i], vSpan, fGroups[vSpan->fGroupIdx]);
    }
    statusLog = IEndLog(statusLog);
#endif
}

//// ISortSourceSpans ////////////////////////////////////////////////////////
//  Does our actual optimization path by resorting all the spans into the
//  most efficient order possible. Also has to re-order the span lookup
//  table.

void    plDrawableSpans::ISortSourceSpans()
{
    hsTArray<uint32_t>    spanReorderTable, spanInverseTable;
    plGeometrySpan      *tmpSpan;
    uint32_t              tmpIdx;
    plSpan              *tmpSpanPtr;


    // Init the reorder table
    for (size_t i = 0; i < fSourceSpans.size(); i++)
        spanReorderTable.Append( i );

    // Do a nice, if naiive, sort by material (hehe by the pointers, no less)
    for (size_t i = 0; i < fSourceSpans.size() - 1; i++)
    {
        size_t j, idx;
        for (j = i + 1, idx = i; j < fSourceSpans.size(); j++)
        {
            if( ICompareSpans( fSourceSpans[ j ], fSourceSpans[ idx ] ) < 0 )
                idx = j;
        }

        // Swap idx with i, so we get the smallest pointer on top
        if( i != idx )
        {
            // Swap both the source span and our internal span
            tmpSpan = fSourceSpans[ i ];
            fSourceSpans[ i ] = fSourceSpans[ idx ];
            fSourceSpans[ idx ] = tmpSpan;

            fSourceSpans[ i ]->fSpanRefIndex = i;
            fSourceSpans[ idx ]->fSpanRefIndex = idx;

            tmpSpanPtr = fSpans[ i ];
            fSpans[ i ] = fSpans[ idx ];
            fSpans[ idx ] = tmpSpanPtr;

            // Also swap the entries in the reorder table
            tmpIdx = spanReorderTable[ i ];
            spanReorderTable[ i ] = spanReorderTable[ idx ];
            spanReorderTable[ idx ] = tmpIdx;
        }

        // Next!
    }


    /// Problem: our reorder table is inversed(y->x instead of x->y). Either we search for numbers,
    /// or we just flip it first...
    spanInverseTable.SetCountAndZero( spanReorderTable.GetCount() );
    for (int i = 0; i < spanReorderTable.GetCount(); i++)
        spanInverseTable[ spanReorderTable[ i ] ] = i;

    /// Now update our span xlate table
    for (plDISpanIndex* di : fDIIndices)
    {
        if (!di->IsMatrixOnly() )
        {
            for (size_t j = 0; j < di->GetCount(); j++)
            {
                uint32_t idx = (*di)[ j ];

                (*di)[ j ] = spanInverseTable[ idx ];
            }
        }
    }

    /// Use our pointer array to rebuild the icicle array (UUUUGLY)
    std::vector<plIcicle> tempIcicles;

    tempIcicles.resize(fIcicles.size());

    plIcicle* newIcicle = tempIcicles.data();
    for (plSpan*& span : fSpans)
    {
        *newIcicle = *(plIcicle *)span;
        span = newIcicle;
        newIcicle++;
    }

    /// Swap the two arrays out. This will basically swap the actual memory blocks, so be careful...
    fIcicles.swap(tempIcicles);
    tempIcicles.clear();
}

//// ICompareSpans ///////////////////////////////////////////////////////////
//  Sorting function for ISortSpans(). Kinda like strcmp(): returns -1 if 
//  span1 < span2, 1 if span1 > span2, 0 if "equal".

short   plDrawableSpans::ICompareSpans( plGeometrySpan *span1, plGeometrySpan *span2 )
{
    bool        b1, b2;
    plBitmap    *t1, *t2;


    /// Quick check--identical materials are easy to compare :)
    if( span1->fMaterial == span2->fMaterial )
        return 0;

    /// Compare features from most to least important...

    // Any decal span should come after a non-decal
    if( span1->fDecalLevel < span2->fDecalLevel )
        return -1;
    if( span1->fDecalLevel > span2->fDecalLevel )
        return 1;
    // Ok, they're equal decal-wise, so find something else to judge on

    // Most important: is one of the materials an alpha blend? (if so, gotta
    // put at end, so it's "bigger")
    if( span1->fMaterial->GetNumLayers() > 0 && 
        ( span1->fMaterial->GetLayer( 0 )->GetState().fBlendFlags & hsGMatState::kBlendMask ) != 0 )
        b1 = true;
    else
        b1 = false;

    if( span2->fMaterial->GetNumLayers() > 0 && 
        ( span2->fMaterial->GetLayer( 0 )->GetState().fBlendFlags & hsGMatState::kBlendMask ) != 0 )
        b2 = true;
    else
        b2 = false;

    if( b1 != b2 )
        return( b1 ? 1 : -1 );

    // Next is texture (name). We do this kinda like strings: compare the first layer's
    // textures and go upwards, so that we group materials together starting with the
    // base layer's texture and going upwards
    size_t numLayers = span1->fMaterial->GetNumLayers();
    if( span2->fMaterial->GetNumLayers() < numLayers )
        numLayers = span2->fMaterial->GetNumLayers();

    for (size_t i = 0; i < numLayers; i++)
    {
        t1 = span1->fMaterial->GetLayer( i )->GetTexture();
        t2 = span2->fMaterial->GetLayer( i )->GetTexture();

        if (t1 != nullptr && t2 == nullptr)
            return 1;
        else if (t1 == nullptr && t2 != nullptr)
            return -1;
        else if (t1 == nullptr && t2 == nullptr)
            break;  // Textures equal up to here--keep going with rest of tests
        
        if( !t1->GetKeyName().empty() && !t2->GetKeyName().empty() )
        {
            int r = t1->GetKeyName().compare(t2->GetKeyName(), ST::case_insensitive);
            if (r != 0)
                return (short)r;
        }
    }

    // Finally, by material itself.
    if( !span1->fMaterial->GetKeyName().empty() && !span2->fMaterial->GetKeyName().empty() )
    {
        int r = span1->fMaterial->GetKeyName().compare(span2->fMaterial->GetKeyName(), ST::case_insensitive);
        if (r != 0)
            return (short)r;
    }

    if( span1->fLocalToWorld.fFlags != span2->fLocalToWorld.fFlags )
    {
        if( span1->fLocalToWorld.fFlags )
            return -1;
        else
            return 1;
    }

    /// Equal in our book...
    return 0;
}
