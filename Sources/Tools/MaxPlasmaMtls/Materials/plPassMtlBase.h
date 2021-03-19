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
#ifndef PL_PASSMTLBASE_H
#define PL_PASSMTLBASE_H

#include <vector>

#include "plInterp/plAnimEaseTypes.h"

class plNoteTrackWatcher;
class plMtlChangeCallback;
class plAnimStealthNode;
class NoteTrack;
class plPassAnimDlgProc;
class plStealthNodeAccessor;
class plMaxNode;
class plErrorMsg;
namespace ST { class string; }

class plPassMtlBase : public Mtl
{
protected:

    friend class plPassAnimDlgProc;
    friend class plStealthNodeAccessor;
    friend class plNoteTrackWatcher;

    plNoteTrackWatcher  *fNTWatcher;

    IParamBlock2    *fBasicPB;
    IParamBlock2    *fAdvPB;
    IParamBlock2    *fLayersPB;
    IParamBlock2    *fAnimPB;

    Interval        fIValid;

    bool            fLoading;

    std::vector<NoteTrack *> fNotetracks;

    bool                               fStealthsChanged;
    std::vector<plMtlChangeCallback *> fChangeCallbacks;

    void                IUpdateAnimNodes();
    plAnimStealthNode   *IFindStealth( const ST::string &animName );
    plAnimStealthNode   *IVerifyStealthPresent( const ST::string &animName );

    int                 IGetNumStealths( bool update = true );
    plAnimStealthNode   *IGetStealth( int index, bool update = true );

    void                ICloneBase( plPassMtlBase *target, RemapDir &remap );
    virtual void        ICloneRefs( plPassMtlBase *target, RemapDir &remap ) = 0;

public:

    // mcn note: as far as I can tell, this is always the same pointer passed around to everyone.
    // So since we have trouble getting it all the time, just store the first version we get and 
    // use that forever and ever
    static IMtlParams *fIMtlParams;

    plPassMtlBase( BOOL loading );
    virtual ~plPassMtlBase();

    virtual bool HasAlpha() = 0;

    enum Refs
    {
        //kRefBasic,        // Can't do this, long ago they were different in the "derived" classes,
        //kRefAdv,          // and even tho MAX says it doesn't write refs out by their index, it does
        //kRefLayers,
        //kRefAnim,
        kRefNotetracks  = 4 // MUST BE THE LAST REF ID SPECIFIED
    };
    void    SetLoadingFlag( bool f ) { fLoading = f; }
    void    PostLoadAnimPBFixup();

    void    RegisterChangeCallback( plMtlChangeCallback *callback );
    void    UnregisterChangeCallback( plMtlChangeCallback *callback );

    // Change notifys from our ntWatcher
    virtual void    NoteTrackAdded();
    virtual void    NoteTrackRemoved();
    virtual void    NameChanged();

    // Loading/Saving
    IOResult Load(ILoad *iload) override;
    IOResult Save(ISave *isave) override;

    void    Reset() override;

    int             NumRefs() override;
    RefTargetHandle GetReference(int i) override;
    void            SetReference(int i, RefTargetHandle rtarg) override;
    RefResult       NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID &partID, RefMessage message) override;

    // Convert time, called on the setupProps pass for each material applied to a node in the scene
    virtual bool    SetupProperties( plMaxNode *node, plErrorMsg *pErrMsg );
    virtual bool    ConvertDeInit( plMaxNode *node, plErrorMsg *pErrMsg );

    int                 GetNumStealths();
    plAnimStealthNode   *GetStealth( int index );

    // Static convert to our plPassMtlBase type, if possible
    static plPassMtlBase    *ConvertToPassMtl( Mtl *mtl );

    // Blend types
    enum
    {
        kBlendNone,
        kBlendAlpha,
        kBlendAdd,
        kBlendMult
    };

    // Alpha blend types
    enum
    {
        kAlphaDiscard,
        kAlphaMultiply,
        kAlphaAdd
    };

    // Advanced Block
    virtual int     GetBasicWire() = 0;
    virtual int     GetMeshOutlines() = 0;
    virtual int     GetTwoSided() = 0;
    virtual int     GetSoftShadow() = 0;
    virtual int     GetNoProj() = 0;
    virtual int     GetVertexShade() = 0;
    virtual int     GetNoShade() = 0;
    virtual int     GetNoFog() = 0;
    virtual int     GetWhite() = 0;
    virtual int     GetZOnly() = 0;
    virtual int     GetZClear() = 0;
    virtual int     GetZNoRead() = 0;
    virtual int     GetZNoWrite() = 0;
    virtual int     GetZInc() = 0;
    virtual int     GetAlphaTestHigh() = 0;

    // Animation block
    virtual const char*  GetAnimName() = 0;
    virtual int          GetAutoStart() = 0;
    virtual int          GetLoop() = 0;
    virtual const char*  GetAnimLoopName() = 0;
    virtual int          GetEaseInType() { return plAnimEaseTypes::kNoEase; }
    virtual float        GetEaseInMinLength() { return 1; }
    virtual float        GetEaseInMaxLength() { return 1; }
    virtual float        GetEaseInNormLength() { return 1; }
    virtual int          GetEaseOutType() { return plAnimEaseTypes::kNoEase; }
    virtual float        GetEaseOutMinLength() { return 1; }
    virtual float        GetEaseOutMaxLength() { return 1; }
    virtual float        GetEaseOutNormLength() { return 1; }
    virtual const char*  GetGlobalVarName() { return nullptr; }
    virtual int          GetUseGlobal() { return 0; }

    // Basic block
    virtual int     GetColorLock() = 0;
    virtual Color   GetAmbColor() = 0;
    virtual Color   GetColor() = 0;
    virtual int     GetOpacity() = 0;
    virtual int     GetEmissive() = 0;
    virtual int     GetUseSpec() = 0;
    virtual int     GetShine() = 0;
    virtual Color   GetSpecularColor() = 0;
    virtual int     GetDiffuseColorLock() = 0;
    virtual Color   GetRuntimeColor() = 0;
    virtual Control *GetPreshadeColorController() = 0;
    virtual Control *GetAmbColorController() = 0;
    virtual Control *GetOpacityController() = 0;
    virtual Control *GetSpecularColorController() = 0;
    virtual Control *GetRuntimeColorController() = 0;
    
    // Layer block
    virtual Texmap *GetBaseLayer() = 0;
    virtual int     GetTopLayerOn() = 0;
    virtual Texmap *GetTopLayer() = 0;
    virtual int     GetLayerBlend() = 0;
    virtual int     GetOutputAlpha() = 0;
    virtual int     GetOutputBlend() = 0;
};


// Make sure min is less than normal, which is less than max
class plEaseAccessor : public PBAccessor
{
protected:
    bool fDoingUpdate;
    int fBlockID;
    int fEaseInMinID, fEaseInMaxID, fEaseInNormID, fEaseOutMinID, fEaseOutMaxID, fEaseOutNormID;

    void AdjustMin(IParamBlock2 *pb, ParamID minID, ParamID normalID, ParamID maxID, float value)
    {
        if (value > pb->GetFloat(normalID))
        {
            pb->SetValue(normalID, 0, value);
            if (value > pb->GetFloat(maxID))
                pb->SetValue(maxID, 0, value);
        }
    }
    void AdjustNormal(IParamBlock2 *pb, ParamID minID, ParamID normalID, ParamID maxID, float value)
    {
        if (value < pb->GetFloat(minID))
            pb->SetValue(minID, 0, value);
        if (value > pb->GetFloat(maxID))
            pb->SetValue(maxID, 0, value);
    }
    void AdjustMax(IParamBlock2 *pb, ParamID minID, ParamID normalID, ParamID maxID, float value)
    {
        if (value < pb->GetFloat(normalID))
        {
            pb->SetValue(normalID, 0, value);
            if (value < pb->GetFloat(minID))
                pb->SetValue(minID, 0, value);
        }
    }

public:
    plEaseAccessor(int blockID, int easeInMinID, int easeInMaxID, int easeInNormID, 
                   int easeOutMinID, int easeOutMaxID, int easeOutNormID)
    {
        fDoingUpdate = false;
        fBlockID = blockID;
        fEaseInMinID = easeInMinID; fEaseInMaxID = easeInMaxID; fEaseInNormID = easeInNormID; 
        fEaseOutMinID = easeOutMinID; fEaseOutMaxID = easeOutMaxID; fEaseOutNormID = easeOutNormID;
    }

    void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t) override
    {
        if (fDoingUpdate)
            return;
        fDoingUpdate = true;

        IParamBlock2 *pb = owner->GetParamBlockByID(fBlockID);

        if (id == fEaseInMinID)
            AdjustMin(pb, fEaseInMinID, fEaseInNormID, fEaseInMaxID, v.f);
        else if (id == fEaseInNormID)
            AdjustNormal(pb, fEaseInMinID, fEaseInNormID, fEaseInMaxID, v.f);
        else if (id == fEaseInMaxID)
            AdjustMax(pb, fEaseInMinID, fEaseInNormID, fEaseInMaxID, v.f);
        else if (id == fEaseOutMinID)
            AdjustMin(pb, fEaseOutMinID, fEaseOutNormID, fEaseOutMaxID, v.f);
        else if (id == fEaseOutNormID)
            AdjustNormal(pb, fEaseOutMinID, fEaseOutNormID, fEaseOutMaxID, v.f);
        else if (id == fEaseOutMaxID)
            AdjustMax(pb, fEaseOutMinID, fEaseOutNormID, fEaseOutMaxID, v.f);

        fDoingUpdate = false;
    }
};

//// plMtlChangeCallback /////////////////////////////////////////////////////
//  Interface class for receiving info about when things change on a material.

class plMtlChangeCallback
{
    public:
        virtual void    NoteTrackListChanged() { }
        virtual void    SegmentListChanged() { }
};

#endif // PL_PASSMTLBASE_H