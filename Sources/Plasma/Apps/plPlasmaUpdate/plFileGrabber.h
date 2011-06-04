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
#ifndef plFileGrabber_h_inc
#define plFileGrabber_h_inc

#include <string>
#include "hsStream.h"

class plFileGrabber
{
public:
	virtual bool IsServerAvailable(const char* serverName, const char* currentDir) = 0;
	virtual bool FileToStream(const char* path, hsStream* stream) = 0;
	virtual void SetServer(const char* server) = 0;
	virtual void MakeProperPath(char* path) = 0;
	virtual bool NeedsAuth() { return false; }
	virtual void SetUsernamePassword(const std::string& username, const std::string& password) {}
};

/* Not needed currently - if we want it again we'll have to reimplement HTTP comm
class plHttpFileGrabber : public plFileGrabber
{
private:
	plHttpDiverseRequestMgr fRequestMgr;

public:
	plHttpFileGrabber();
	virtual bool IsServerAvailable(const char* serverName);
	virtual bool FileToStream(const char* path, hsStream* stream);
	virtual void SetServer(const char* server);
	virtual void MakeProperPath(char* path);
	virtual bool NeedsAuth() { return true; }
	virtual void SetUsernamePassword(const std::string& username, const std::string& password);
};
*/

class plNetShareFileGrabber : public plFileGrabber
{
private:
	std::string fServerName;

public:
	plNetShareFileGrabber();
	virtual bool IsServerAvailable(const char* serverName, const char* currentDir);
	virtual bool FileToStream(const char* path, hsStream* stream);
	virtual void SetServer(const char* server);
	virtual void MakeProperPath(char* path);
};

#endif