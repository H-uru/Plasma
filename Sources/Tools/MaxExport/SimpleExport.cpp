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
//
// 3DSMax HeadSpin exporter
//

#include "HeadSpin.h"
#include "hsExceptionStack.h"
#include "hsStream.h"

#include "MaxMain/MaxAPI.h"

#include "SimpleExport.h"
#include "plExportErrorMsg.h"
#include "plExportLogErrorMsg.h"

#include "MaxConvert/UserPropMgr.h"
#include "MaxConvert/hsConverterUtils.h"
#include "MaxConvert/plBitmapCreator.h"
#include "pfPython/plPythonFileMod.h"

#include "MaxMain/plPluginResManager.h"
#include "plResMgr/plRegistryHelpers.h"
#include "plResMgr/plRegistryNode.h"
#include "MaxConvert/plConvert.h"
#include "MaxConvert/hsMaterialConverter.h"

#include "plPhysX/plSimulationMgr.h"
#include "plSDL/plSDL.h"
#include "MaxMain/plMaxCFGFile.h"

// For texture export/cleanup
#include "MaxMain/plTextureExportLog.h"
#include "pnKeyedObject/plKey.h"
#include "pnKeyedObject/plUoid.h"
#include "plGImage/plCubicEnvironmap.h"
#include "plGImage/plDynamicTextMap.h"
#include "plGImage/plMipmap.h"
#include "plMessageBox/hsMessageBox.h"
#include "plScene/plSceneNode.h"

#include "plExportDlg.h"

#include "plStatusLog/plStatusLog.h"

#include "plAvatar/plAvatarMgr.h"

extern UserPropMgr gUserPropMgr;

#ifdef HS_DEBUGGING
#define HS_NO_TRY       
#endif

//
// .MSH export module functions follow:
//

HSExport2::HSExport2() 
{
}

HSExport2::~HSExport2() 
{
}

int HSExport2::ExtCount() 
{
    return 2;
}

//
// Extensions supported for import/export modules
//
const MCHAR* HSExport2::Ext(int n)
{
static  char str[64];
    switch(n) 
    {
        case 0:
            return _M("");
        case 1:
            return _M("prd");
    }
    return _T("");
}

//
// Long ASCII description (i.e. "Targa 2.0 Image File")
//
const MCHAR* HSExport2::LongDesc()
{
    return _M("Plasma 2.0");
}

//
// Short ASCII description (i.e. "Targa")   
//
const MCHAR* HSExport2::ShortDesc()
{
#ifdef HS_DEBUGGING
    return _M("Plasma 2.0 Debug");
#else
    return _M("Plasma 2.0");
#endif
}

//
// ASCII Author name
//
const MCHAR* HSExport2::AuthorName()
{
    return _M("Cyan, Inc.");
}

//
// ASCII Copyright message
//
const MCHAR* HSExport2::CopyrightMessage() 
{
    return _M("Copyright 1997 HeadSpin Technology Inc.");
}

//
// Other message #1
//
const MCHAR* HSExport2::OtherMessage1()
{
    return _M("");
}

//
// Other message #2
//
const MCHAR* HSExport2::OtherMessage2()
{
    return _M("");
}

//
// Version number * 100 (i.e. v3.01 = 301)
//
unsigned int HSExport2::Version() 
{
    return 100;
}

//
// Optional
//
void HSExport2::ShowAbout(HWND hWnd) 
{
}

// Another little helper class to help write out a list of textures to a log file
class plTextureLoggerCBack : public plRegistryKeyIterator
{
protected:
    plTextureExportLog* fTELog;

public:
    plTextureLoggerCBack(plTextureExportLog* teLog) { fTELog = teLog; }

    bool EatKey(const plKey& key) override
    {
        plBitmap* bmap = plBitmap::ConvertNoRef(key->ObjectIsLoaded());
        if (bmap != nullptr)
            fTELog->AddTexture(bmap);
        return true;    // Always continue
    }
};

// Yet another key iterator, this one to call OptimizeDrawables() on each sceneNode
class plOptimizeIterator : public plRegistryKeyIterator
{
public:
    bool EatKey(const plKey& key) override
    {
        if (key->GetUoid().GetClassType() == plSceneNode::Index())
        {
            plSceneNode* sn = plSceneNode::ConvertNoRef(key->ObjectIsLoaded());
            if (sn != nullptr)
                sn->OptimizeDrawables();
        }
        return true;    // Always continue
    }
};

//
//
// 
int HSExport2::DoExport(const MCHAR *name,ExpInterface *ei,Interface *gi, BOOL suppressPrompts, DWORD options)
{
    BOOL backupEnabled = gi->AutoBackupEnabled();
    gi->EnableAutoBackup(FALSE);

    BOOL bmmSilentMode = TheManager->SilentMode();
    TheManager->SetSilentMode(TRUE);

    bool mbSuppressPrompts = hsMessageBox_SuppressPrompts;
    hsMessageBox_SuppressPrompts = (suppressPrompts)?true:false;

    // Disable save so we don't crash in export or
    // otherwise screw database.
    SimpleExportExitCallback exitCB;
    gi->RegisterExitMAXCallback(&exitCB);

    gUserPropMgr.OpenQuickTable();
    hsConverterUtils::Instance().CreateNodeSearchCache();

    BroadcastNotification(NOTIFY_PRE_EXPORT);

    // get just the path (not the file) of where we are going to export to
    plFileName out_name = M2ST(name);
    plFileName out_path = out_name.StripFileName();

    // Apparently this was implied by the open dialog, but not if you call Max's ExportToFile() func
    SetCurrentDirectoryW(out_path.WideString().data());

    // 
    // Setup ErrorMsg
    //
    // Needs to be outside try/catch so it doesn't stack unwind...
    plExportErrorMsg hituser_errorMessage;  // This is the errorMessage that slaps user

    _tcsncpy(fName, gi->GetCurFileName(), std::size(fName));
    TCHAR* dot = _tcsrchr(fName, _T('.'));
    if (dot)
        *dot = 0;

    plFileName ErrorLogName = plFileName::Join(out_path, ST::format("{}.err", T2ST(fName)));
    plExportLogErrorMsg logonly_errorMessage(ErrorLogName);     // This errorMessage just writes it all to a file

    // now decide which errorMessage object to use
    plErrorMsg* errorMessage;
    if (suppressPrompts)
        errorMessage = &logonly_errorMessage;
    else
        errorMessage = &hituser_errorMessage;

    // For export time stats
    DWORD exportTime = timeGetTime();
    _SYSTEMTIME tm;
    GetSystemTime(&tm);

    //
    // Let's get cracking!  Convert the scene...
    //
    plConvertSettings settings;
    
    if (plExportDlg::Instance().IsExporting())
    {
        settings.fDoPreshade = plExportDlg::Instance().GetDoPreshade();
        settings.fPhysicalsOnly = plExportDlg::Instance().GetPhysicalsOnly();
        settings.fDoLightMap = plExportDlg::Instance().GetDoLightMap();
        settings.fExportPage = plExportDlg::Instance().GetExportPage();
    }

    plConvert::Instance().Init(gi, errorMessage, &settings);

    // We want to incorporate any SDL changes since the last export, so we DeInit()
    // and re-initialize.
    plSDLMgr::GetInstance()->SetSDLDir(plFileName::Join(plMaxConfig::GetClientPath(), "sdl"));
    plSDLMgr::GetInstance()->DeInit();
    plSDLMgr::GetInstance()->Init();

    // Add disk source for writing
    plFileName datPath = plFileName::Join(out_path, "dat");
    CreateDirectoryW(datPath.WideString().data(), nullptr);
    plPluginResManager::ResMgr()->SetDataPath(datPath);

    if (hsgResMgr::Reset())
    {
        plSimulationMgr::Init();
        plAvatarMgr::GetInstance();

        // Verify the pages here manually, since it's a separate step now
        plPluginResManager::ResMgr()->VerifyPages();

        plPythonFileMod::SetAtConvertTime();

        // Convert!!!
        bool convertOK = plConvert::Instance().Convert();

        // Free the material cache.  This will delete unused materials.
        hsMaterialConverter::Instance().FreeMaterialCache(out_path.AsString().c_str());

        if (convertOK)
        {
            // Optimize the drawables
            plOptimizeIterator  optIterator;
            plPluginResManager::ResMgr()->IterateKeys( &optIterator );

            // And save.
            plPluginResManager::ResMgr()->WriteAllPages();

            // Write out a texture log file
            plFileName textureLog = plFileName::Join("log", ST::format("exportedTextures_{}.log", fName));
            plTextureExportLog      textureExportLog( textureLog );
            plTextureLoggerCBack    loggerCallback( &textureExportLog );
            
            plPluginResManager::ResMgr()->IterateKeys( &loggerCallback );
            
            textureExportLog.Write();

            // Moving this to the end of writing the files out. Yes, this means that any unused mipmaps still get
            // written to disk, including ones loaded on preload, but it's the only way to get shared texture pages
            // to work without loading in the entire age worth of reffing objects. - 5.30.2002 mcn
            plBitmapCreator::Instance().DeInit();
        }

        // Have the resMgr clean up after export. This includes paging out any converted pages
        plPluginResManager::ResMgr()->EndExport();

        plSimulationMgr::Shutdown();
        plAvatarMgr::ShutDown();

        // Reset the resmgr so we free all the memory it allocated
        hsgResMgr::Reset();
    }


    //----------------------------------------------
    // Write a log entry to the Db file name for now
    //----------------------------------------------
    hsUNIXStream dbLog;
    dbLog.Open(out_name, "at");
    exportTime = (timeGetTime() - exportTime) / 1000;
    dbLog.WriteString(
        ST::format(
            "Export from Max File \"{}\" on {02d}/{02d}/{4d} took {d}:{02d}\n",
            out_name, tm.wMonth, tm.wDay, tm.wYear, exportTime / 60, exportTime % 60
        )
    );

    // Allow plugins to clean up after export
    BroadcastNotification(NOTIFY_POST_EXPORT);

    hsConverterUtils::Instance().DestroyNodeSearchCache();
    gUserPropMgr.CloseQuickTable();
    gi->UnRegisterExitMAXCallback(&exitCB);
    hsMessageBox_SuppressPrompts = mbSuppressPrompts;
    TheManager->SetSilentMode(bmmSilentMode);
    gi->EnableAutoBackup(backupEnabled);

    MessageBeep(MB_ICONASTERISK);

    return 1;
}
