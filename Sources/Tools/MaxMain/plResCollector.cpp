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
#include "HeadSpin.h"
#include "plResCollector.h"
#include "plMtlCollector.h"
#include "max.h"
#include "../MaxExport/plExportProgressBar.h"

void plResCollector::Collect()
{
	Interface *ip = GetCOREInterface();

	// Get the directory to copy files to
	char path[MAX_PATH];
	ip->ChooseDirectory(ip->GetMAXHWnd(),
						"Choose a folder to copy the resources to",
						path,
						NULL);
	if (!strcmp(path, ""))
		return;

	// Make sure the directory ends with a slash
	if (path[strlen(path)-1] != '\\' && path[strlen(path)-1] != '/')
		strcat(path, "\\");

	// Make a list of all the textures
	TexNameSet texNames;
	plMtlCollector::GetAllTextures(texNames);

	plExportProgressBar bar;
	bar.Start("Copy Files", texNames.size()+1);

	// Copy each texture to the output directory
	TexNameSet::iterator it = texNames.begin();
	for (; it != texNames.end(); it++)
	{
		const char *texName = *it;

		char outpath[MAX_PATH], name[_MAX_FNAME+_MAX_EXT], ext[_MAX_EXT];
		_splitpath(texName, NULL, NULL, name, ext);
		strcat(name, ext);

		if (bar.Update(name))
			return;

		strcpy(outpath, path);
		strcat(outpath, name);

		CopyFile(texName, outpath, TRUE);
	}

	// Get the filename to save to
	TSTR& maxFile = ip->GetCurFileName();
	TSTR& filePath =  ip->GetCurFilePath();

	if (!strcmp(maxFile, ""))
		return;

	if (bar.Update(maxFile))
		return;

	// If we need to save, do it now
	if (IsSaveRequired())
		ip->SaveToFile(filePath);

	// Copy the max file to the output directory
	strcat(path, maxFile);
	CopyFile(filePath, path, TRUE);
}
