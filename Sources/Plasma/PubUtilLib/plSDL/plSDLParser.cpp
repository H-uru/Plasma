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
#include "hsStlUtils.h"
#include "plSDL.h"
#include "../plFile/hsFiles.h"
#include "../plFile/plStreamSource.h"
#include "../pnNetCommon/pnNetCommon.h"
#include "../pnNetCommon/plNetApp.h"

static const int kTokenLen=256;

void plSDLParser::DebugMsg(char* fmt, ...) const
{
	return;
	plNetApp* netApp = plSDLMgr::GetInstance()->GetNetApp();

	va_list args;
	va_start(args, fmt);
	
	if (netApp)
	{
		hsLogEntry(netApp->DebugMsgV(fmt, args));
	}
	else
		DebugMsgV(fmt, args);
	va_end(args);
}

void plSDLParser::DebugMsgV(char* fmt, va_list args) const
{
	if (strlen(fmt)==nil)
		return;
	hsStatusMessage(xtl::formatv(fmt,args).c_str());
}

//
// parsing stateDesc
// read name, version
// return true to skip the next token read
//
bool plSDLParser::IParseStateDesc(const char* fileName, hsStream* stream, char token[], plStateDescriptor*& curDesc) const
{	
	plSDL::DescriptorList* descList = &plSDLMgr::GetInstance()->fDescriptors;

	bool ok = true;

	//
	// NAME
	//
//	curDesc=plSDLMgr::GetInstance()->FindDescriptor(token, plSDL::kLatestVersion);
//	if (!curDesc)
	{
		curDesc = TRACKED_NEW plStateDescriptor;
		curDesc->SetName(token);

		DebugMsg("SDL: DESC name=%s", token);
	}
	
	//
	// {
	//
	stream->GetToken(token, kTokenLen);	// skip '{'
	
	//
	// VERSION
	//
	if (stream->GetToken(token, kTokenLen))	
	{
		if (!strcmp(token, "VERSION"))
		{
			// read desc version
			hsAssert(curDesc, xtl::format("Syntax problem with .sdl file, fileName=%s", fileName).c_str());
			if (stream->GetToken(token, kTokenLen))
			{
				int v=atoi(token);
				curDesc->SetVersion(v);
				DebugMsg("\tVersion=%d", v);
			}				
		}
		else
		{
			hsAssert(false, xtl::format("Error parsing state desc, missing VERSION, fileName=%s", 
				fileName).c_str());
			ok = false;
		}
	}
	else
	{
		hsAssert(false, xtl::format("Error parsing state desc, fileName=%s", fileName).c_str());
		ok = false;
	}

	if ( ok )
	{
		ok = ( plSDLMgr::GetInstance()->FindDescriptor(curDesc->GetName(), curDesc->GetVersion())==nil );
		if ( !ok )
		{
			std::string err = xtl::format( "Found duplicate SDL descriptor for %s version %d.\nFailed to parse file: %s", curDesc->GetName(), curDesc->GetVersion(), fileName );
			plNetApp::StaticErrorMsg( err.c_str() );
			hsAssert( false, err.c_str() );
		}
	}

	if ( ok )
	{
		descList->push_back(curDesc);
	}
	else
	{
		delete curDesc;
		curDesc = nil;
	}

	return false;
}

//
// Parse a variable descriptor.
// read type, name, count [default]
// return true to skip the next token read
//
bool plSDLParser::IParseVarDesc(const char* fileName, hsStream* stream, char token[], plStateDescriptor*& curDesc, 
								plVarDescriptor*& curVar) const
{
	hsAssert(curDesc, xtl::format("Syntax problem with .sdl file, fileName=%s", fileName).c_str());
	if ( !curDesc )
		return false;

	bool skipNext=false;
	std::string dbgStr;
	static char	seps[] = "( ,)[]";
	// read type, name, cnt, [default]
	
	//
	// TYPE
	// create new state var, make current
	//
	if (*token == '$')
	{
		// nested sdls
		char* sdlName = token+1;
		plStateDescriptor* stateDesc = plSDLMgr::GetInstance()->FindDescriptor(sdlName, plSDL::kLatestVersion);
		hsAssert(stateDesc, xtl::format("can't find nested state desc reference %s, fileName=%s", 
				sdlName, fileName).c_str());
		curVar = TRACKED_NEW plSDVarDescriptor(stateDesc);
	}
	else
		curVar = TRACKED_NEW plSimpleVarDescriptor;
	
	curDesc->AddVar(curVar);
	bool ok=curVar->SetType(token);
	hsAssert(ok, xtl::format("Variable 'type' syntax problem with .sdl file, type=%s, fileName=%s", token, fileName).c_str());
	dbgStr = xtl::format("\tVAR Type=%s ", token).c_str();
	
	//
	// NAME (foo[1])
	//			
	if (stream->GetToken(token, kTokenLen))
	{
		hsAssert(strstr(token, "[") && strstr(token, "]"), xtl::format("invalid var syntax, missing [x], fileName=%s", 
				fileName).c_str());
		char* ptr = strtok( token, seps );	// skip [
		
		hsAssert(curVar, xtl::format("Missing current var.  Syntax problem with .sdl file, fileName=%s", fileName).c_str());
		curVar->SetName(token);
		//
		// COUNT
		//
		char* cntTok=strtok(nil, seps);		// kill ]
		int cnt = cntTok ? atoi(cntTok) : 0;
		curVar->SetCount(cnt);
		if (cnt==0)
			curVar->SetVariableLength(true);
		dbgStr += xtl::format("Name=%s[%d]", curVar->GetName(), cnt).c_str();
	}
	
	//
	// optional tokens: DEFAULT, INTERNAL
	//
	while (stream->GetToken(token, kTokenLen))
	{
		if (!strcmp(token, "DEFAULT"))
		{
			hsAssert(curVar, xtl::format("Syntax problem with .sdl file, fileName=%s", fileName).c_str());
			// read state var type
			
			std::string defaultStr;
			plSimpleVarDescriptor* sVar=(plSimpleVarDescriptor*)curVar;
			if (sVar)
			{
				int i;
				for(i=0;i<sVar->GetAtomicCount();i++)
				{
					if (stream->GetToken(token, kTokenLen))
					{
						defaultStr += token;
						if (i!=sVar->GetAtomicCount()-1)
							defaultStr += ",";
					}
				}
			}
			if (defaultStr.size())
			{
				curVar->SetDefault(defaultStr.c_str());				
				dbgStr += std::string(" DEFAULT=") + defaultStr;
			}
		}
		else
		if (!strcmp(token, "DISPLAYOPTION"))
		{
			hsAssert(curVar, xtl::format("Syntax problem with .sdl file, fileName=%s", fileName).c_str());
			dbgStr += std::string(" ") + token;

			hsBool read=stream->GetToken(token, kTokenLen);
			if (read)
			{
				std::string oldOptions=curVar->GetDisplayOptions();
				if (oldOptions.size())
					oldOptions += std::string(",");
				oldOptions += token;
				curVar->SetDisplayOptions(oldOptions.c_str());
				dbgStr += std::string("=") + token;
				if (!stricmp(token, "hidden"))
					curVar->SetInternal(true);
			}
			else
			{
				hsAssert(false, xtl::format("missing displayOption string, fileName=%s", fileName).c_str());
			}
		}
		else
		if (!strcmp(token, "DEFAULTOPTION"))
		{
			hsAssert(curVar, xtl::format("Syntax problem with .sdl file, fileName=%s", fileName).c_str());
			dbgStr += std::string(" ") + token;

			hsBool read=stream->GetToken(token, kTokenLen);
			if (read)
			{
				dbgStr += std::string("=") + token;
				if (!stricmp(token, "vault"))
					curVar->SetAlwaysNew(true);
			}
			else
			{
				hsAssert(false, xtl::format("missing defaultOption string, fileName=%s", fileName).c_str());
			}
		}

#if 1	// delete me in May 2003
		else
		if (!strcmp(token, "INTERNAL"))
		{
			hsAssert(curVar, xtl::format("Syntax problem with .sdl file, fileName=%s", fileName).c_str());
			curVar->SetInternal(true);
			dbgStr += std::string(" ") + token;
		}
		else
		if (!strcmp(token, "PHASED"))
		{
			hsAssert(curVar, xtl::format("Syntax problem with .sdl file, fileName=%s", fileName).c_str());
			curVar->SetAlwaysNew(true);
			dbgStr += std::string(" ") + token;
		}
#endif
		else
		{
			skipNext=true;
			break;
		}
	}

	DebugMsg((char*)dbgStr.c_str());

	return skipNext;
}

//
// create state descriptor from sdl file.
// return false on err.
//
bool plSDLParser::ILoadSDLFile(const char* fileName) const
{
	DebugMsg("Parsing SDL file %s", fileName);
	
	wchar_t* temp = hsStringToWString(fileName);
	hsStream* stream = plStreamSource::GetInstance()->GetFile(temp);
	delete [] temp;
	if (!stream)
		return false;

	stream->Rewind();

	plVarDescriptor* curVar=nil;
	plStateDescriptor* curDesc=nil;
	char token[kTokenLen];
	bool parsingStateDesc=false;
	bool skip=false;
	while (1)
	{		
		if (!skip)
		{
			if (!stream->GetToken(token, kTokenLen))
				break;
		}
		skip=false;

		if (!strcmp(token, "VAR"))
		{
			parsingStateDesc=false;
			curVar=nil;		// start fresh
			continue;
		}
		
		if (!strcmp(token, "STATEDESC"))
		{
			parsingStateDesc=true;
			curDesc=nil;	// start fresh
			continue;
		}

		if (!strcmp(token, "}"))
		{
			if ( curDesc )
				curDesc->SetFilename( fileName );
			parsingStateDesc=false;
			continue;
		}

		if (parsingStateDesc)
		{
			skip=IParseStateDesc(fileName, stream, token, curDesc);
			if ( !curDesc )
				break;	// failed to parse state desc
		}
		else
		{
			skip=IParseVarDesc(fileName, stream, token, curDesc, curVar);
		}
	}

	// If the very last char is a } without a \n, then it won't be handled above for some reason, so we have to catch it here.
	if ( curDesc )
		curDesc->SetFilename( fileName );

	// do not close or delete the stream, we do not own it
	return true;
}

//
// load all .sdl files in sdl directory, and create descriptors for each.
// return false on error
//
bool plSDLParser::IReadDescriptors() const
{
	std::string sdlDir = plSDLMgr::GetInstance()->GetSDLDir();
	DebugMsg("SDL: Reading latest descriptors from directory %s", sdlDir.c_str());

	wchar_t* temp = hsStringToWString(sdlDir.c_str());
	std::wstring wSDLDir = temp;
	delete [] temp;

	// Get the names of all the sdl files
	std::vector<std::wstring> files = plStreamSource::GetInstance()->GetListOfNames(wSDLDir, L".sdl");

	bool ret=true;
	int cnt=0;
	for (int i = 0; i < files.size(); i++)
	{
		char* str = hsWStringToString(files[i].c_str());
		if (!ILoadSDLFile(str))
		{
			plNetApp* netApp = plSDLMgr::GetInstance()->GetNetApp();
			if (netApp)
				netApp->ErrorMsg("Error loading SDL file %s", str);
			else
				hsStatusMessageF("Error loading SDL file %s", str);
			ret=false;
		}
		else
			cnt++;
		delete [] str;
	}
	DebugMsg("Done reading SDL files");	

	if (!cnt)
		ret=false;

	return ret;
}


//
// reads sdl folder, creates descriptor list
//
bool plSDLParser::Parse() const
{
	return IReadDescriptors();
}
