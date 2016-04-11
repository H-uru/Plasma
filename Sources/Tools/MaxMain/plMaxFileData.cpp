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

#include <max.h>
#pragma hdrstop

/** \file This file contains a stub implementation for the ClassDescs left behind by
 *   Cyan's SceneViewer crap. This just keeps new versions of 3ds Max from crashing on
 *   null ClassDescs returned by the GUP. It also silences that "Dll Missing" dialog that
 *   everyone evar wants to whine about even though it means absolutely nothing.
 */

#define PLASMA_FILE_DATA_CID Class_ID(0x255a700a, 0x285279dc)
#define MAXFILE_DATA_CHUNK  1001
static const uint8_t kVersion = 1;

class plMaxFileDataControl : public StdControl
{
public:
    SYSTEMTIME fCodeBuildTime;
    char       fBranch[128];

    plMaxFileDataControl()
    {
        memset(&fCodeBuildTime, 0, sizeof(SYSTEMTIME));
        memset(&fBranch, 0, arrsize(fBranch));
    }

    // Animatable
    virtual void EditTrackParams(TimeValue t, ParamDimensionBase *dim, TCHAR *pname, HWND hParent, IObjParam *ip, DWORD flags) { }
    int TrackParamsType() { return TRACKPARAMS_WHOLE; }
    virtual void DeleteThis() { delete this; }

    // ReferenceMaker
    virtual RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message)
    {
        return REF_DONTCARE;
    }

    Class_ID ClassID() { return PLASMA_FILE_DATA_CID; }
    SClass_ID SuperClassID() { return CTRL_FLOAT_CLASS_ID; }
    void GetClassName(TSTR& s) { s = "DEAD - SceneViewer"; }

    // Control methods
    RefTargetHandle Clone(RemapDir& remap) { return new plMaxFileDataControl(); }
    void Copy(Control* from) { }
    virtual BOOL IsReplaceable() { return FALSE; }

    // StdControl methods
    void GetValueLocalTime(TimeValue t, void* val, Interval& valid, GetSetMethod method = CTRL_ABSOLUTE) { }
    void SetValueLocalTime(TimeValue t, void* val, int commit, GetSetMethod method) { }
    void Extrapolate(Interval range, TimeValue t, void* val, Interval& valid, int type) { }
    void *CreateTempValue() { return nullptr; }
    void DeleteTempValue(void *val) { }
    void ApplyValue(void* val, void* delta) { }
    void MultiplyValue(void* val, float m) { }

    // MyControl methods
    IOResult Load(ILoad *iload);
    IOResult Save(ISave *isave);
};

IOResult plMaxFileDataControl::Load(ILoad *iload)
{
    ULONG nb;
    IOResult res;
    while (IO_OK==(res=iload->OpenChunk()))
    {
        if (iload->CurChunkID() == MAXFILE_DATA_CHUNK)
        {
            uint8_t version = 0;
            res = iload->Read(&version, sizeof(uint8_t), &nb);
            res = iload->Read(&fCodeBuildTime, sizeof(SYSTEMTIME), &nb);

            int branchLen = 0;
            iload->Read(&branchLen, sizeof(int), &nb);
            iload->Read(&fBranch, branchLen, &nb);
        }

        iload->CloseChunk();
        if (res != IO_OK)
            return res;
    }

    return IO_OK;
}

IOResult plMaxFileDataControl::Save(ISave *isave)
{
    ULONG nb;
    isave->BeginChunk(MAXFILE_DATA_CHUNK);

    isave->Write(&kVersion, sizeof(kVersion), &nb);
    isave->Write(&fCodeBuildTime, sizeof(SYSTEMTIME), &nb);

    int branchLen = strlen(fBranch)+1;
    isave->Write(&branchLen, sizeof(int), &nb);
    isave->Write(&fBranch, branchLen, &nb);

    isave->EndChunk();
    return IO_OK;
}

class MaxFileDataClassDesc : public ClassDesc
{
public:
    int             IsPublic()              { return FALSE; }
    void*           Create(BOOL loading)    { return new plMaxFileDataControl; }
    const TCHAR*    ClassName()             { return _T("MaxFileData"); }
    SClass_ID       SuperClassID()          { return CTRL_FLOAT_CLASS_ID; }
    Class_ID        ClassID()               { return PLASMA_FILE_DATA_CID; }
    const TCHAR*    Category()              { return _T(""); }
};

MaxFileDataClassDesc gMaxFileDataClassDesc;
ClassDesc* GetMaxFileDataDesc() { return &gMaxFileDataClassDesc; }
