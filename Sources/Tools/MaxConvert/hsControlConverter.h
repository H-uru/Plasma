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
#ifndef __HSCONTROLCONVERTER_H
#define __HSCONTROLCONVERTER_H

#include <commdlg.h>
#include <math.h>

//#include "Max.h"
#include "stdmat.h"
#include "bmmlib.h"
#include "istdplug.h"
#include "texutil.h"
#include "plInterp/hsKeys.h"
#include "hsTemplates.h"

class plMaxNode;

enum ControllerType
{
    ctrlTypeUnknown,
    ctrlTypeFloat,
    ctrlTypePoint3,
    ctrlTypePosition,
    ctrlTypeRotation,
    ctrlTypeRotationX,
    ctrlTypeRotationY,
    ctrlTypeRotationZ,
    ctrlTypeScale,
    ctrlTypeTransform,
    ctrlTypeEase,
    ctrlTypeMult,
    ctrlTypeVert,   // CS Biped
    ctrlTypeHoriz,  // CS Biped
    ctrlTypeTurn,   // CS Biped
    ctrlTypeRollAngle,
};

class plController;
class plLeafController;
class plCompoundController;
struct hsKeyFrame;
class hsConverterUtils;
class plSceneObject;

class hsControlConverter
{
private:
    hsControlConverter();

public:
    ~hsControlConverter() { };
    static hsControlConverter& Instance();
    
    void Init(plErrorMsg* msg);
    void DeInit();

    // Used to pick correct controller by lights/materials
    hsBool GetControllerByName(Animatable* anim, TSTR &name, Control* &ctl);
    Control *GetControllerByID(IParamBlock2 *pblock, int paramID);

    /////////////////////////////////////////////////////////////////////////
    // 
    // Controller convert functions:
    // 
    // All convert functions must call ISetSegRange(start, end) at the beginning.
    // (ISetSegRange(-1, -1) will give you the entire anim.)
    plLeafController* MakeMatrix44Controller(StdUVGen* uvGen, const char* nodeName);
    plLeafController* MakeMatrix44Controller(Control* prsControl);
    plLeafController* MakeScalarController(Control* control, plMaxNode* node, float start = -1, float end = -1);
    plController* MakeColorController(Control* control, plMaxNode* node, float start = -1, float end = -1);
    plController* MakePosController(Control* control, plMaxNode* node, float start = -1, float end = -1);
    plController* MakeScaleController(Control* control, plMaxNode* node, float start = -1, float end = -1);
    plController* MakeRotController(Control* control, plMaxNode* node, hsBool camRot = false, float start = -1, float end = -1);
    plCompoundController* MakeTransformController(Control* control, plMaxNode* node, float start = -1, float end = -1);

    // This last one was in tempAnim.cpp on its own for some time, apparently created
    // as an initial attempt to get anims working in Max. It's still used, so I don't want
    // to nuke it, but it made sense to move it here.
    plController* ConvertTMAnim(plSceneObject *obj, plMaxNode *node, hsAffineParts *parts, float start = -1, float end = -1);
    //
    //
    //////////////////////////////////////////////////////////////////////////

    void    Matrix3ToHsMatrix44(Matrix3* m3, hsMatrix44* hsM);
    Matrix3 StdUVGenToMatrix3(StdUVGen* uvGen);
    bool    StdUVGenToHsMatrix44(hsMatrix44* hsMat, StdUVGen* uvGen, bool preserveOffset=false);
    void    MaxSampleAngles(const char* nodeName, Control* ctl, Tab<TimeValue>& kTimes, float maxRads);
    void    ScalePositionController(plController* ctl, float scale);

    void    ReduceKeys(Control *control, float threshold);
    hsBool  HasKeyTimes(Control* ctl);
    uint8_t       GetKeyType(Control* ctl, hsBool rotQuat = false);

    plMaxNode* GetXformParent(plMaxNode* node);
    hsBool ForceWorldSpace(plMaxNode* node);
    hsBool ForceOrigin(plMaxNode* node);
    hsBool ForceLocal(plMaxNode* node);
    hsBool IsAnimated(plMaxNode* node);
    hsBool OwnsMaterialCopy(plMaxNode* node);
    hsBool HasFrameEvents(plMaxNode *node);

    void CompositeKeyTimes(Control* ctl, Tab<TimeValue> &time);

    int GetTicksPerFrame()      { return fTicksPerFrame; }
    int GetFrameRate()          { return fFrameRate; }
    int GetTicksPerSec()        { return fTicksPerSec; }
    int GetStartFrame()         { return fStartFrame; }
    int GetEndFrame()           { return fEndFrame; }
    int GetNumFrames()          { return fNumFrames; }
    float GetAnimLength()       { return fAnimLength; }

private:
    void ISetSegRange(float start, float end);
    void IConvertSubTransform(Control *control, char *ctlName, plMaxNode *node, plCompoundController *tmc, float start, float end);

    plLeafController* ICreateSimpleRotController(plMaxNode* node, Control* control, hsBool camRot = false);
    plLeafController* ICreateSimpleScaleController(plMaxNode* node, Control* control);
    plLeafController* ICreateQuatController(plMaxNode* node, Control* control, bool rotation = true, hsBool camRot = false);
    plLeafController* ICreateScaleValueController(plMaxNode* node, Control* control);
    plLeafController* ICreateScalarController(plMaxNode* node, Control* control);
    plLeafController* ICreateSimplePosController(plMaxNode* node, Control* control);

    void    IEnableEaseCurves(Animatable* control, bool enable);
    void    IGetControlSampleTimes(Control* control, int iLo, int iHi, Tab<TimeValue>& kTimes, float maxRads);
    int     IAddPartsKeys(Control* control, hsTArray <hsG3DSMaxKeyFrame>* kfArray, plMaxNode* node);
    int32_t   ICreateHSInterpKey(Control* control, IKey* mKey, TimeValue keyTime, hsKeyFrame* baseKey, plMaxNode* node=nil, hsBool rotQuat = false);
    int32_t   IGetRangeCoverKeyIndices(char* nodeName, Control* cont, int32_t &start, int32_t &end);
    ControllerType IGetControlType(TSTR ctrlName);
    bool    IIsKeyTimeInRange(TimeValue time);
    bool    IIsKeyInRange(IKey* key);
    void    IGetUnEasedLocalTM(plMaxNode* node, Control* control, hsMatrix44* out, TimeValue time);
    Matrix3 IFlipY();
    bool    ISkinNode(plMaxNode* node);
    void    ISetForceLocal(bool f) { fForceLocal=f; }

    hsBool  IGetEditableMeshKeyTimes( plMaxNode *node, Tab<TimeValue> &times );
    hsBool  IGetGeomKeyTimes( plMaxNode *node, Tab<TimeValue> &times );
    void    IGetGeomKeyTimesRecur( Animatable *anim, Tab<TimeValue> &times );
    hsBool  IGetSubAnimByName( Animatable *anim, TSTR &name, Animatable *&subAnim );
    void    IExportAnimatedCameraFOV(plMaxNode* node, hsTArray <hsG3DSMaxKeyFrame>* kfArray);
    Interface* fInterface;

    hsConverterUtils& fConverterUtils;
    plErrorMsg * fErrorMsg;

    int32_t       fTicksPerFrame;
    int32_t       fFrameRate;
    int32_t       fTicksPerSec;
    int32_t       fStartFrame;
    int32_t       fEndFrame;
    int32_t       fNumFrames;
    float    fAnimLength;
    hsBool    fWarned;

    hsBool    fForceLocal;

    TimeValue   fSegStart;
    TimeValue   fSegEnd;
};

#endif