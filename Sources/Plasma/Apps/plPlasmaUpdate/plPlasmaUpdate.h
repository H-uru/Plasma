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
#include "jvBaseDlg.h"
#include <vector>
#include <string>
#include "plPlasmaServers.h"
#include "plFileGrabber.h"

class plManifest;
class jvDialogResizer;

class plPlasmaUpdate : public jvBaseDlg
{
protected:
	static BOOL CALLBACK ILoginWinProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static std::string fUserName;
	static std::string fPassword;

	std::vector<plManifest*> fManifests;
	char fIniPath[MAX_PATH];
	bool fCanExit;
	enum ProgressType { kValidating, kDownloading };
	ProgressType fProgressType;
	jvDialogResizer* fResizer;
	plPlasmaServers fServers;
	bool fAutoDownload;
	plFileGrabber* fFileGrabber;

	void IInit();
	void IShutdown();

	bool IReadServerInfo();

	void IEnableCtrls(bool enable);

	BOOL IDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

	static void ProgressFunc(const char* name, int delta);
	static void ILog(const char* format, ...);

	void IUpdateServer();

	void IDeleteManifests();
	bool IGetManifests(const char* serverRoot, bool external);
	void IDownloadUpdates();

public:
	plPlasmaUpdate();
	virtual ~plPlasmaUpdate();

	bool Create();

	void SetAutoDownload() { fAutoDownload = true; }
};
