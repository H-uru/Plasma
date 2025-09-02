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

#include "plComponent.h"
#include "plComponentReg.h"
#include "plMiscComponents.h"
#include "MaxMain/plMaxNodeBase.h"

#include "MaxMain/MaxAPI.h"

#include <map>
#include <vector>
#include "resource.h"

#include "BipedKiller.h"
#include "plTransform/hsAffineParts.h"

//////////////
//
// LOCAL TYPES
//
//////////////

// NODETMINFO
// A local handy thing to remember a matrix and the time we sampled it
struct nodeTMInfo
{
    TimeValue fTime;
    Matrix3 fMat3;
};

// PLSAMPLEVEC
// A vector of matrix samples
typedef std::vector<nodeTMInfo *> plSampleVec;

/////////////
//
// PROTOTYPES
//
/////////////

void ProcessNodeRecurse(INode *node, INode *parent, Interface *theInterface);
void ProcessBipedNodeRecurse(INode *bipNode, INode *newParent, Interface *theInterface);
void ProcessNonBipedNodeRecurse(INode *node, INode *parent, Interface *theInterface);

int LimitTransform(INode* node, Matrix3* nodeTM);
void GetParts(int32_t i, std::vector<nodeTMInfo *>& mat3Array,  hsAffineParts* parts);

Quat GetRotKey(int32_t i, std::vector<nodeTMInfo *>& mat3Array, hsAffineParts* parts);
Point3 GetPosKey(int32_t i, std::vector<nodeTMInfo *>& mat3Array, hsAffineParts* parts);
ScaleValue GetScaleKey(int32_t i, std::vector<nodeTMInfo *>& mat3Array, hsAffineParts* parts);

Quat MakeRotKey(INode *node, INode *parent, TimeValue t);
Point3 MakePosKey(INode *node, INode *parent, TimeValue t);
ScaleValue MakeScaleKey(INode *node, INode *parent, TimeValue t);

AffineParts GetLocalNodeParts(INode *node, INode *parent, TimeValue t);

bool ExportableAnimationController(INode* node);
bool HasBipController(INode* node);
Quat GetRotKey(int32_t i, std::vector<nodeTMInfo *>& mat3Array);


plSampleVec * SampleNodeMotion(INode* node, INode* parent, int sampleRate, Interface *theInterface);
plSampleVec * SampleNodeMotion(INode * node, INode* parent, int sampleRate, TimeValue start, TimeValue end);
void ReapplyAnimation(INode *node, plSampleVec *samples);
void FreeMotionSamples(plSampleVec *samples);

/////////////////
//
// IMPLEMENTATION
//
/////////////////

// REMOVEBIPED
void RemoveBiped(INode *bipRoot, Interface *theInterface)
{
    SuspendAnimate();   
    AnimateOn();
    
    // remember Max's default controllers (for the user)
    ClassDesc* defaultRotCtrl=GetDefaultController(CTRL_ROTATION_CLASS_ID);
    ClassDesc* defaultPosCtrl=GetDefaultController(CTRL_POSITION_CLASS_ID);
    ClassDesc* defaultScaleCtrl=GetDefaultController(CTRL_SCALE_CLASS_ID);
    
    // change default controllers to linear to create linear controllers
    // since we have no tan info
    DllDir* dllDir=&theInterface->GetDllDir();
    ClassDirectory* classDir=&dllDir->ClassDir();

    ClassDesc* rotCtrl = classDir->FindClass( SClass_ID(CTRL_ROTATION_CLASS_ID),
                                Class_ID(TCBINTERP_ROTATION_CLASS_ID,0));  // was Class_ID(LININTERP_ROTATION_CLASS_ID,0));

    ClassDesc* posCtrl = classDir->FindClass( SClass_ID(CTRL_POSITION_CLASS_ID),
                                    Class_ID(LININTERP_POSITION_CLASS_ID, 0));

    ClassDesc* scaleCtrl = classDir->FindClass( SClass_ID(CTRL_SCALE_CLASS_ID),
                                            Class_ID(LININTERP_SCALE_CLASS_ID, 0));

    SetDefaultController(CTRL_ROTATION_CLASS_ID, rotCtrl);
    SetDefaultController(CTRL_POSITION_CLASS_ID, posCtrl);
    SetDefaultController(CTRL_SCALE_CLASS_ID, scaleCtrl);

    ProcessNodeRecurse(bipRoot, nullptr, theInterface);

    //deinit
    ResumeAnimate();
    
    // remember Max's default controllers (for the user)
    SetDefaultController(CTRL_ROTATION_CLASS_ID, defaultRotCtrl);
    SetDefaultController(CTRL_POSITION_CLASS_ID, defaultPosCtrl);
    SetDefaultController(CTRL_SCALE_CLASS_ID, defaultScaleCtrl);
}

// PROCESSNODERECURSE
void ProcessNodeRecurse(INode *node, INode *parent, Interface *theInterface)
{
    if(HasBipController(node))
    {
        ProcessBipedNodeRecurse(node, parent, theInterface);
    } else {
        ProcessNonBipedNodeRecurse(node, parent, theInterface);
    }
}

// PROCESSBIPNODERECURSE
// When we find a Biped-controlled node in our hierarchy, we need to find one non-biped
// child and promote it to the place of the biped node in the hierarchy. The siblings
// of the promoted node will become its children, as will the original children from the
// biped node.
void ProcessBipedNodeRecurse(INode *bipNode, INode *parent, Interface *theInterface)
{
    int numChildren = bipNode->NumberOfChildren();
    auto bipName = bipNode ? bipNode->GetName() : nullptr;
    INode *replacement = nullptr;

    for (int i = 0; i < numChildren; i++)
    {
        INode *child = bipNode->GetChildNode(i);
        auto childName = child ? child->GetName() : nullptr;

        if( ! HasBipController(child) )
        {
            replacement = child;                    // this child is going to be our replacement for this bipnode

            // sample the animation (into global space)
            plSampleVec *samples = SampleNodeMotion(replacement, bipNode, 1, theInterface);

            // detach from the parent (this blows away the animation)
            replacement->Detach(0);

            // attach the node to the biped's parent.
            parent->AttachChild(replacement);   

            ReapplyAnimation(child, samples);
            FreeMotionSamples(samples);

            // we only need one replacement for the bip node
            break;
        }
    }
    
    if(replacement)
    {
        // reparent the siblings to the newly promoted replacement node
        numChildren = bipNode->NumberOfChildren();
        for (int i = 0; i < numChildren; i++)
        {
            INode *child = bipNode->GetChildNode(i);

            if( HasBipController(child) )
            {
                ProcessBipedNodeRecurse(child, replacement, theInterface);
            } else {
                child->Detach(0);                   // remove the (non-bip) child from the bip node
                replacement->AttachChild(child);    // attach it to the non-bip parent

                ProcessNonBipedNodeRecurse(child, replacement, theInterface);
            }
        }
    } else {
        // this is an error condition: we've got a bip node that has no non-bip child for us to promote
        TCHAR buf[256];
        _sntprintf(buf, std::size(buf), _T("Couldn't find non-bip node to transfer motion to for bip node %s\n"), bipNode->GetName());
        OutputDebugString(buf);
    }
}

// PROCESSNONBIPEDNODERECURSE
// Sample motion for a hierarchy that does not have any Biped controllers in it.
void ProcessNonBipedNodeRecurse(INode *node, INode *parent, Interface *theInterface)
{
    if( ! ExportableAnimationController(node) )
    {
        plSampleVec *samples = SampleNodeMotion(node, parent, 2, theInterface);
        ReapplyAnimation(node, samples);
        FreeMotionSamples(samples);
    }

    int numChildren = node->NumberOfChildren();
    for (int i = 0; i < numChildren; i++)
    {
        INode *child = node->GetChildNode(i);

        ProcessNodeRecurse(child, node, theInterface);
    }
}

// ADJUSTROTKEYS
void AdjustRotKeys(INode *node)
{
    Control *controller = node->GetTMController();
    Control *rotControl = controller->GetRotationController();
    IKeyControl *rotKeyCont = GetKeyControlInterface(rotControl);
    int numKeys = rotKeyCont->GetNumKeys();

    for(int i = 0; i < numKeys; i++)
    {
        ITCBKey key;
        rotKeyCont->GetKey(i, &key);

        key.cont = 0;
        rotKeyCont->SetKey(i, &key);

    }
    
}

#define boolTrue = (0 == 0);
#define boolFalse = (0 == 1);

// *** todo: generalize this for rotation keys as well.
int CompareKeys(ILinPoint3Key &a, ILinPoint3Key &b)
{
    int result = a.val.Equals(b.val, .001f);
#if 0
    hsStatusMessage(ST::format("COMPAREKEYS(point): ({} {} {}) vs ({}, {}, {}) = {}", a.val.x, a.val.y, a.val.z, b.val.x, b.val.y, b.val.z, result ? "yes" : "no").c_str());
#endif
    return result;
}

template<class T>
void ReduceKeys(INode *node, IKeyControl *keyCont)
{

    keyCont->SortKeys();        // ensure the keys are sorted by time
    
    int to;         // the next key we're setting
    int from;       // the next key we're examining
    int origNumKeys = keyCont->GetNumKeys();
    int finalNumKeys = origNumKeys;
    
    for (to = 1, from = 1; from < origNumKeys - 1; to++, from++)
    {
        T prevKey, curKey, nextKey;

        keyCont->GetKey(from - 1, &prevKey);
        keyCont->GetKey(from, &curKey);
        keyCont->GetKey(from + 1, &nextKey);

        if (CompareKeys(curKey, prevKey) && CompareKeys(curKey, nextKey))
            finalNumKeys--; // skip it
        else
            keyCont->SetKey(to, &curKey); // copy current key
    }
    // copy the last one without peeking ahead
    T lastKey;
    keyCont->GetKey(from, &lastKey);
    keyCont->SetKey(to, &lastKey);

    keyCont->SetNumKeys(finalNumKeys);
    keyCont->SortKeys();
}

void EliminateScaleKeys(INode *node, IKeyControl *keyCont)
{
    int numKeys = keyCont->GetNumKeys();
    ILinScaleKey last;
    keyCont->GetKey(numKeys - 1, &last);
    keyCont->SetKey(1, &last);      // move the last to the second
    keyCont->SetNumKeys(2);
}

// REAPPLYANIMATION
// Now that we've reparented a node within the hierarchy, re-apply all its animation.
void ReapplyAnimation(INode *node, plSampleVec *samples)
{
    Control *controller = node->GetTMController();

    Control *rotControl = NewDefaultRotationController();   // we set the default rotation controller type above in RemoveBiped()
    Control *posControl = NewDefaultPositionController();   // '' ''
    Control *scaleControl = NewDefaultScaleController();    // '' ''
    
    controller->SetRotationController(rotControl);
    controller->SetPositionController(posControl);
    controller->SetScaleController(scaleControl);

    for(int i = 0; i < samples->size(); i++)
    {
        nodeTMInfo *info = (*samples)[i];
        Matrix3 m = info->fMat3;
        TimeValue t = info->fTime;

#if 1
        node->SetNodeTM(t, m);
#else
        AffineParts parts;

        INode *parent = node->GetParentNode();
        Matrix3 parentTM = parent->GetNodeTM(t);
        Matrix3 invParentTM = Inverse(parentTM);
        m *= invParentTM;

        decomp_affine(m, &parts);

        Quat q(parts.q.x, parts.q.y, parts.q.z, parts.q.w);
        Point3 p(parts.t.x, parts.t.y, parts.t.z);

        rotControl->SetValue(t, q);
        posControl->SetValue(t, p);
#endif
    }

    IKeyControl *posKeyCont = GetKeyControlInterface(posControl);
    IKeyControl *scaleKeyCont = GetKeyControlInterface(scaleControl);

    ReduceKeys<ILinPoint3Key>(node, posKeyCont);
    EliminateScaleKeys(node, scaleKeyCont);
    // grrrr ReduceKeys<ILinScaleKey>(node, scaleKeyCont);
}

// HASBIPCONTROLLER
bool HasBipController(INode* node)
{
    if (!node)
        return false;
    Control* c = node->GetTMController();
    if (c && ((c->ClassID()== BIPDRIVEN_CONTROL_CLASS_ID) ||
        (c->ClassID()== BIPBODY_CONTROL_CLASS_ID) || 
        (c->ClassID()== FOOTPRINT_CLASS_ID)) )
        return true;
    return false;

}

// EXPORTABLEANIMATIONCONTROLLER
bool ExportableAnimationController(INode* node)
{
    bool result = false;

    if(node)
    {
        Control *c = node->GetTMController();
        if(c)
        {
            Class_ID id = c->ClassID();
            if(id == Class_ID(LININTERP_ROTATION_CLASS_ID, 0)
                || id == Class_ID(PRS_CONTROL_CLASS_ID, 0)
                || id == Class_ID(LININTERP_POSITION_CLASS_ID, 0)
                || id == Class_ID(TCBINTERP_FLOAT_CLASS_ID, 0)
                || id == Class_ID(TCBINTERP_POSITION_CLASS_ID, 0)
                || id == Class_ID(TCBINTERP_ROTATION_CLASS_ID, 0)
                || id == Class_ID(TCBINTERP_POINT3_CLASS_ID, 0)
                || id == Class_ID(TCBINTERP_SCALE_CLASS_ID, 0))
            {
                result = true;
            }
        }
    }
    return result;
}

// SAMPLENODEMOTION
// top level function for sampling all the motion on a single node
plSampleVec * SampleNodeMotion(INode* node, INode* parent, int sampleRate, Interface *theInterface)
{
    Interval interval = theInterface->GetAnimRange();
    TimeValue start = interval.Start();                 // in ticks
    TimeValue end = interval.End();

    sampleRate *= GetTicksPerFrame();                   // convert sample rate to ticks

    return SampleNodeMotion(node, parent, sampleRate, start, end);
}

// SAMPLENODEMOTION
// sample all the motion on a single node
// intended for use in the context of a full tree traversal
plSampleVec * SampleNodeMotion(INode * node, INode* parent, int sampleRate, TimeValue start, TimeValue end)
{
    plSampleVec *result = new plSampleVec;

    bool done = false;
    
    for(int i = start; ! done; i += sampleRate)
    {
        if (i > end) i = end;
        if (i == end) done = true;

        // Get key time
        TimeValue keyTime = i;
        int frameNum= keyTime / GetTicksPerFrame();

        // get localTM
        nodeTMInfo * nti = new nodeTMInfo;
        nti->fTime = keyTime;
        Matrix3 localTM = node->GetNodeTM(keyTime);

        nti->fMat3 = localTM;
        result->push_back(nti);
    }
    return result;
}

// FREEMOTIONSAMPLES
void FreeMotionSamples(plSampleVec *samples)
{
    int count = samples->size();
    for(int i = 0; i < count; i++)
    {
        delete (*samples)[i];
    }
    delete samples;
}

// LIMITTRANSFORM
// Check if this node is marked as having a constrained transform.
// Meaning ignore part of the transform for this node and push it down to its kids.
int LimitTransform(INode* node, Matrix3* nodeTM)
{
/* NOT sure if we want to support this functionality: probably eventually.
    bool32 noRotX=false,noRotY=false,noRotZ=false;
    bool32 noRot=gUserPropMgr.UserPropExists(node,"BEHNoRot") || MatWrite::HasToken(node->GetName(), "norot");
    if (!noRot)
    {
        noRotX=gUserPropMgr.UserPropExists(node,"BEHNoRotX") || MatWrite::HasToken(node->GetName(), "norotx");
        noRotY=gUserPropMgr.UserPropExists(node,"BEHNoRotY") || MatWrite::HasToken(node->GetName(), "noroty");
        noRotZ=gUserPropMgr.UserPropExists(node,"BEHNoRotZ") || MatWrite::HasToken(node->GetName(), "norotz");
    }

    bool32 noTransX=false,noTransY=false,noTransZ=false;
    bool32 noTrans=gUserPropMgr.UserPropExists(node,"BEHNoTrans") || MatWrite::HasToken(node->GetName(), "notrans");
    if (!noTrans)
    {
        noTransX=gUserPropMgr.UserPropExists(node,"BEHNoTransX") || MatWrite::HasToken(node->GetName(), "notransx");
        noTransY=gUserPropMgr.UserPropExists(node,"BEHNoTransY") || MatWrite::HasToken(node->GetName(), "notransy");
        noTransZ=gUserPropMgr.UserPropExists(node,"BEHNoTransZ") || MatWrite::HasToken(node->GetName(), "notransz");
    }

    if (noRot || noTrans || 
        noRotX || noRotY || noRotZ ||
        noTransX || noTransY || noTransZ)
    {
        Matrix3 tm(true);           // identity
        
        Quat q(*nodeTM);            // matrix to quat
        float eulerAng[3];
        QuatToEuler(q, eulerAng);   // to euler
        
        // rotation
        if (!noRot && !noRotX)
            tm.RotateX(eulerAng[0]);
        if (!noRot && !noRotY)
            tm.RotateY(eulerAng[1]);
        if (!noRot && !noRotZ)
            tm.RotateZ(eulerAng[2]);

        // translation
        Point3 trans=nodeTM->GetTrans();
        if (noTrans || noTransX)
            trans.x=0;
        if (noTrans || noTransY)
            trans.y=0;
        if (noTrans || noTransZ)
            trans.z=0;
        tm.Translate(trans);

        // copy back
        *nodeTM = tm;
        return true;
    }
*/
    return false;
}
