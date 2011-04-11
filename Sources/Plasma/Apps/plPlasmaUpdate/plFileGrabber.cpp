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
#include "plFileGrabber.h"

/* Not needed currently - if we want it again we'll have to reimplement HTTP comm
plHttpFileGrabber::plHttpFileGrabber()
{
	fRequestMgr.SetHostname("");
}

bool plHttpFileGrabber::FileToStream(const char* path, hsStream* stream)
{
	std::string pathStr(path);
	bool retVal = fRequestMgr.GetFileToStream(path, stream);
	stream->SetPosition(0);

	return retVal;
}

void plHttpFileGrabber::SetServer(const char* server)
{
	std::string serverPath(server);

	fRequestMgr.SetHostname(serverPath);
}

void plHttpFileGrabber::MakeProperPath(char* path)
{
	char* slash = NULL;
	do {
		slash = strchr(path, '\\');
		if (slash)
			*slash = '/';
	} while(slash != NULL);
}

void plHttpFileGrabber::SetUsernamePassword(const std::string& username, const std::string& password)
{
	fRequestMgr.SetUsername(username);
	fRequestMgr.SetPassword(password);
}

bool plHttpFileGrabber::IsServerAvailable(const char* serverName)
{
	bool retVal = false;

	HINTERNET hInternet = InternetOpen("Parable Patcher",INTERNET_OPEN_TYPE_PRECONFIG,NULL,NULL,0);
	if (hInternet)
	{
		HINTERNET hHttp = InternetConnect(hInternet,serverName,8080,fUserName.c_str(),fPassword.c_str(),INTERNET_SERVICE_HTTP,0,0);
		if (hHttp)
		{
			HINTERNET hRequest = HttpOpenRequest(hHttp, "GET", "/Current/Current.txt", NULL, NULL, NULL, INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_KEEP_CONNECTION, 0);
			if (hRequest)
			{
				DWORD dwCode;
				DWORD dwSize = sizeof(dwCode);
				HttpSendRequest(hRequest, NULL, 0, NULL, 0);
				HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &dwCode, &dwSize, NULL);
				if (dwCode >= 200 && dwCode < 300)
				{
					retVal = true;
				}

				InternetCloseHandle(hRequest);
			}

			InternetCloseHandle(hHttp);
		}
		InternetCloseHandle(hInternet);
	}

	return retVal;
}
*/

plNetShareFileGrabber::plNetShareFileGrabber()
{
}

#define BUFFER_SIZE 1024*1024
bool plNetShareFileGrabber::FileToStream(const char* path, hsStream* stream)
{
	hsUNIXStream fileStream;
	std::string filePath = fServerName + path;
	
	if (fileStream.Open(filePath.c_str()))
	{
		char* buffer = new char[BUFFER_SIZE];
		UInt32 streamSize = fileStream.GetSizeLeft();
		while (streamSize > (BUFFER_SIZE))
		{
			fileStream.Read(BUFFER_SIZE, buffer);
			stream->Write(BUFFER_SIZE, buffer);

			streamSize = fileStream.GetSizeLeft();
		}

		if (streamSize > 0)
		{
			fileStream.Read(streamSize, buffer);
			stream->Write(streamSize, buffer);
		}

		stream->Rewind();

		fileStream.Close();
		delete [] buffer;

		return true;
	}

	return false;
}

void plNetShareFileGrabber::SetServer(const char* server)
{
	fServerName = "\\\\";
	fServerName += server;
}

void plNetShareFileGrabber::MakeProperPath(char* path)
{
	char* slash = NULL;
	do {
		slash = strchr(path, '/');
		if (slash)
			*slash = '\\';
	} while(slash != NULL);
}

bool plNetShareFileGrabber::IsServerAvailable(const char* serverName, const char* currentDir)
{
	bool retVal = false;

	char serverPath[MAX_PATH];
	sprintf(serverPath, "\\\\%s\\%s\\Current.txt", serverName, currentDir);

	hsUNIXStream si;
	if (si.Open(serverPath, "rb"))
	{
		retVal = true;
		si.Close();
	}

	return retVal;
}
