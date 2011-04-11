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
//
// 3DSMax HeadSpin exporter
//

#include "HeadSpin.h"
#include "SimpleExport.h"
#include "notify.h"

#include "plExportErrorMsg.h"
#include "plExportLogErrorMsg.h"

#include "../MaxConvert/UserPropMgr.h"
#include "hsExceptionStack.h"
#include "../MaxConvert/hsConverterUtils.h"
#include "../MaxConvert/plBitmapCreator.h"
#include "../pfPython/plPythonFileMod.h"

#include "../MaxMain/plPluginResManager.h"
#include "../plResMgr/plRegistryHelpers.h"
#include "../plResMgr/plRegistryNode.h"
#include "hsStream.h"
#include "../MaxConvert/plConvert.h"
#include "../MaxConvert/hsMaterialConverter.h"

#include "../plPhysX/plSimulationMgr.h"
#include "../plSDL/plSDL.h"
#include "../MaxMain/plMaxCFGFile.h"

// For texture export/cleanup
#include "../MaxMain/plTextureExportLog.h"
#include "../pnKeyedObject/plKey.h"
#include "../pnKeyedObject/plUoid.h"
#include "../plGImage/plCubicEnvironmap.h"
#include "../plGImage/plDynamicTextMap.h"
#include "../plGImage/plMipmap.h"
#include "../plScene/plSceneNode.h"

#include "plExportDlg.h"

#include "../plStatusLog/plStatusLog.h"
#include "../plFile/plFileUtils.h"

#include "../plAvatar/plAvatarMgr.h"

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
const TCHAR *HSExport2::Ext(int n) 
{
static	char str[64];
	switch(n) 
	{
		case 0:
			return "";
		case 1:
			return "prd";
	}
	return _T("");
}

//
// Long ASCII description (i.e. "Targa 2.0 Image File")
//
const TCHAR *HSExport2::LongDesc() 
{
	return "Plasma 2.0";
}

//
// Short ASCII description (i.e. "Targa")	
//
const TCHAR *HSExport2::ShortDesc() 
{
#ifdef HS_DEBUGGING
	return "Plasma 2.0 Debug";
#else
	return "Plasma 2.0";
#endif
}

//
// ASCII Author name
//
const TCHAR *HSExport2::AuthorName() 
{
	return "Billy Bob";
}

//
// ASCII Copyright message
//
const TCHAR *HSExport2::CopyrightMessage() 
{
	return "Copyright 1997 HeadSpin Technology Inc.";
}

//
// Other message #1
//
const TCHAR *HSExport2::OtherMessage1() 
{
	return _T("");
}

//
// Other message #2
//
const TCHAR *HSExport2::OtherMessage2() 
{
	return _T("");
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

void IGetPath(const char* name, char* path)
{
	int i;
	// find the last backslash in the full path
	for ( i=strlen(name)-1; i>=0 ; i-- )
	{
		if ( name[i] == '\\' )
			break;
	}
	if ( i >= 0 && i < 256)		// if either we couldn't the backslash or the path was too big
	{
		strncpy(path,name,i+1);
		path[i+1] = '\0';		//null terminate string (cause strncpy might not)
	}
	else
		path[0] = '\0';			// otherwise just make it a null string
}

// Another little helper class to help write out a list of textures to a log file
class plTextureLoggerCBack : public plRegistryKeyIterator
{
protected:
	plTextureExportLog* fTELog;

public:
	plTextureLoggerCBack(plTextureExportLog* teLog) { fTELog = teLog; }

	virtual hsBool EatKey(const plKey& key)
	{
		plBitmap* bmap = plBitmap::ConvertNoRef(key->ObjectIsLoaded());
		if (bmap != nil)
			fTELog->AddTexture(bmap);
		return true;	// Always continue
	}
};

// Yet another key iterator, this one to call OptimizeDrawables() on each sceneNode
class plOptimizeIterator : public plRegistryKeyIterator
{
public:
	virtual hsBool EatKey(const plKey& key)
	{
		if (key->GetUoid().GetClassType() == plSceneNode::Index())
		{
			plSceneNode* sn = plSceneNode::ConvertNoRef(key->ObjectIsLoaded());
			if (sn != nil)
				sn->OptimizeDrawables();
		}
		return true;	// Always continue
	}
};

//
//
// 
int	HSExport2::DoExport(const TCHAR *name,ExpInterface *ei,Interface *gi, BOOL suppressPrompts, DWORD options)
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
	char out_path[256];
	IGetPath(name, out_path);
	// Apparently this was implied by the open dialog, but not if you call Max's ExportToFile() func
	SetCurrentDirectory(out_path);

	// 
	// Setup ErrorMsg
	//
    // Needs to be outside try/catch so it doesn't stack unwind...
    plExportErrorMsg hituser_errorMessage;	// This is the errorMessage that slaps user

    TSTR filename = gi->GetCurFileName();
	hsStrncpy(fName, filename, 128);
	char *dot = strrchr(fName, '.');
	if (dot)
		*dot = 0;
    char ErrorLogName[512];
    sprintf(ErrorLogName, "%s%s.err", out_path, fName);
	plExportLogErrorMsg logonly_errorMessage(ErrorLogName);		// This errorMessage just writes it all to a file

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
	char buf[MAX_PATH];
	strcpy(buf, plMaxConfig::GetClientPath());
	strcat(buf, "sdl");
	plSDLMgr::GetInstance()->SetSDLDir(buf);
	plSDLMgr::GetInstance()->DeInit();
	plSDLMgr::GetInstance()->Init();

	// Add disk source for writing
	char datPath[MAX_PATH];
	strcpy(datPath, out_path);
	plFileUtils::AddSlash(datPath);
	strcat(datPath, "dat\\");
	CreateDirectory(datPath, NULL);
	plPluginResManager::ResMgr()->SetDataPath(datPath);

	if (hsgResMgr::Reset())
	{
		plSimulationMgr::Init();
		plAvatarMgr::GetInstance();

		// Verify the pages here manually, since it's a separate step now
		plPluginResManager::ResMgr()->VerifyPages();

		plPythonFileMod::SetAtConvertTime();

		// Convert!!!
		hsBool convertOK = plConvert::Instance().Convert();

		// Free the material cache.  This will delete unused materials.
		hsMaterialConverter::Instance().FreeMaterialCache(out_path);

		if (convertOK)
		{
			// Optimize the drawables
			plOptimizeIterator	optIterator;
			plPluginResManager::ResMgr()->IterateKeys( &optIterator );

			// And save.
			plPluginResManager::ResMgr()->WriteAllPages();

			// Write out a texture log file
		    char textureLog[MAX_PATH];
		    sprintf(textureLog, "log\\exportedTextures_%s.log", fName);
			plTextureExportLog		textureExportLog( textureLog );
			plTextureLoggerCBack	loggerCallback( &textureExportLog );
			
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
	dbLog.Open(name,"at");
	char str[256];
	exportTime = (timeGetTime() - exportTime) / 1000;
	sprintf(str,"Export from Max File \"%s\" on %02d/%02d/%4d took %d:%02d\n",filename,tm.wMonth,tm.wDay,tm.wYear, exportTime/60, exportTime%60);
	dbLog.WriteString(str);
	dbLog.Close();

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
