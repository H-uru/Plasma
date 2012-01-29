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
#include "Max.h"
#include "istdplug.h"
#include "custcont.h"

#include "MaxExport/SimpleExport.h"
#include "MaxMain/MaxCompat.h"

#include "MaxComponent/plComponentMgr.h"
#include "MaxPlasmaMtls/plMtlImport.h"
extern ClassDesc* GetGUPDesc();
extern ClassDesc* GetComponentUtilDesc();
extern ClassDesc* GetComponentMgrDesc();
extern ClassDesc* GetMaxFileDataDesc();
extern ClassDesc* GetMaxUtilsDesc();

static HSClassDesc2 HSDesc;
static int controlsInit = FALSE;
HINSTANCE hInstance = NULL;

/*inline*/ TCHAR *GetString(int id)
{
    static TCHAR buf[256];
    if (hInstance)
        return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
    return NULL;
}

//
// return a string to be displayed if the DLL is not found
//
__declspec(dllexport) const TCHAR *LibDescription() 
{ 
    return "Plasma 2.0"; 
}

//
// the number of plugin classes in the dll
//
__declspec(dllexport) int LibNumberClasses() 
{ 
    return 7 + plComponentMgr::Inst().Count() + plPlasmaMtlImport::GetNumMtlDescs(); 
}

//
// return the i'th class descriptor defined by the plugin
//

// TEMP //
class plGeneralAttribClassDesc;
extern plGeneralAttribClassDesc theGeneralAttribClassDesc;
// TEMP //

__declspec(dllexport) ClassDesc *LibClassDesc(int i) 
{
    switch(i) 
    { 
        case 0: 
            return &HSDesc; 
        case 1:
            return GetGUPDesc();
        case 2:
            return (ClassDesc*)&theGeneralAttribClassDesc;
        case 3:
            return GetComponentUtilDesc();
        case 4:
            return GetComponentMgrDesc();
        case 5:
#ifdef MAXSCENEVIEWER_ENABLED
            return GetMaxFileDataDesc();
#else
            return 0;
#endif
        case 6:
            return GetMaxUtilsDesc();
        default: 
            {
                int numMtls = plPlasmaMtlImport::GetNumMtlDescs();
                if( i - 7 < numMtls )
                    return plPlasmaMtlImport::GetMtlDesc( i - 7 );
                return plComponentMgr::Inst().Get( i - 7 - numMtls );
            }
    }
}

//
// Return version so can detect obsolete DLLs
//
__declspec(dllexport) ULONG LibVersion() 
{ 
    return VERSION_3DSMAX; 
}

#include "plPythonMgr.h"
#include "plPluginResManager.h"
#include "plSDL/plSDL.h"
#include "plMaxCFGFile.h"
#include <direct.h>
#include "plFile/hsFiles.h"

//
// DLLMAIN
//
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) 
{
    hInstance = hinstDLL;

    if (!controlsInit) 
    {
        controlsInit = TRUE;
        
        // jaguar controls
        INIT_CUSTOM_CONTROLS(hInstance);

        // initialize Chicago controls
        InitCommonControls();

        plPythonMgr::Instance().LoadPythonFiles();

        const char *clientPath = plMaxConfig::GetClientPath(false, true);
        if (clientPath)
        {
            char oldCwd[kFolderIterator_MaxPath];
            _getcwd(oldCwd, sizeof(oldCwd));
            _chdir(clientPath);
            plSDLMgr::GetInstance()->Init();
            _chdir(oldCwd);
        }
        
        // Initialize the ResManager
        plResManager* pRmgr = new plPluginResManager;
        hsgResMgr::Init(pRmgr);
    }

    switch (fdwReason) 
    {
        case DLL_PROCESS_ATTACH:
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            break;
    }
    return(TRUE);
}

//////////////////////////////////////////////////////////////////////////////////
// TEMP
//////////////////////////////////////////////////////////////////////////////////
#include "CustAttrib.h"
#include "ICustAttribContainer.h"
#include "iparamb2.h"

#define PL_GEN_ATTRIB_CLASS_ID Class_ID(0x24c36e6e, 0x53ec2ce4)

class plGeneralAttrib : public CustAttrib
{
public:
    ClassDesc2  *fClassDesc;
    IParamBlock2 *fPBlock;

    plGeneralAttrib();

    virtual RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
                               PartID& partID,  RefMessage message){return REF_SUCCEED;}

    int NumParamBlocks() { return 1; }                  // return number of ParamBlocks in this instance
    IParamBlock2* GetParamBlock(int i) { return fPBlock; } // return i'th ParamBlock
    IParamBlock2* GetParamBlockByID(BlockID id) { return (fPBlock->ID() == id) ? fPBlock : NULL; } // return id'd ParamBlock

    int NumRefs() { return 1;}
    virtual RefTargetHandle GetReference(int i) { if(i == 0) return fPBlock; else return NULL; }
    virtual void SetReference(int i, RefTargetHandle rtarg) { if(i == 0) fPBlock = (IParamBlock2 *)rtarg; }

    virtual int NumSubs()  { return 1; }
    virtual Animatable* SubAnim(int i) { return fPBlock; }
    virtual TSTR SubAnimName(int i){ return fClassDesc->ClassName();} 


    void BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev);
    void EndEditParams(IObjParam *ip, ULONG flags, Animatable *next);
    SClass_ID       SuperClassID() {return CUST_ATTRIB_CLASS_ID;}
    Class_ID        ClassID() {return fClassDesc->ClassID();}

    ReferenceTarget *Clone(RemapDir &remap = DEFAULTREMAP);
    virtual bool CheckCopyAttribTo(ICustAttribContainer *to) { return true; }
    
    const TCHAR* GetName() { return (const TCHAR*)_T(fClassDesc->ClassName()); }
    void DeleteThis() { delete this; }
};


class plGeneralAttribClassDesc : public ClassDesc2
{
public:
    int             IsPublic()      { return 1; }
    void*           Create(BOOL loading) { return new plGeneralAttrib; }
    const TCHAR*    ClassName()     { return _T("Plasma Attrib"); }
    SClass_ID       SuperClassID()  { return CUST_ATTRIB_CLASS_ID; }
    Class_ID        ClassID()       { return PL_GEN_ATTRIB_CLASS_ID; }
    const TCHAR*    Category()      { return _T(""); }
    const TCHAR*    InternalName()  { return _T("PlasmaAttrib"); }
    HINSTANCE       HInstance()     { return hInstance; }
};
static plGeneralAttribClassDesc theGeneralAttribClassDesc;

//
// Parameter Block Description
//
enum
{
    kRoomName = 0
};

ParamBlockDesc2 generalAttribBlock
(
    1, _T("GeneralAttribs"), 0, &theGeneralAttribClassDesc, P_AUTO_CONSTRUCT, 0,

    // params
    kRoomName,      _T("roomName"),         TYPE_STRING,        0,  0,
    p_default,      "", 
    end,

    end
);

plGeneralAttrib::plGeneralAttrib() : fClassDesc(&theGeneralAttribClassDesc), fPBlock(NULL)
{
    fClassDesc->MakeAutoParamBlocks(this);
}

void plGeneralAttrib::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
    fClassDesc->BeginEditParams(ip,this,flags,prev);
}

void plGeneralAttrib::EndEditParams(IObjParam *ip, ULONG flags, Animatable *next)
{
    fClassDesc->EndEditParams(ip,this,flags,next);
}

ReferenceTarget *plGeneralAttrib::Clone(RemapDir &remap)
{
    plGeneralAttrib *pnew = (plGeneralAttrib*) fClassDesc->Create(false);
    pnew->SetReference(0, remap.CloneRef(fPBlock));
    BaseClone(this, pnew, remap);
    return pnew;
}
//////////////////////////////////////////////////////////////////////////////////
// TEMP
//////////////////////////////////////////////////////////////////////////////////
