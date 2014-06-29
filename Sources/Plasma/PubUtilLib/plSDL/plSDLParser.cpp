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
#include "plSDL.h"
#include "plFile/plStreamSource.h"
#include "pnNetCommon/pnNetCommon.h"
#include "pnNetCommon/plNetApp.h"

static const int kTokenLen=256;

void plSDLParser::DebugMsg(const char* fmt, ...) const
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

void plSDLParser::DebugMsgV(const char* fmt, va_list args) const
{
    if (strlen(fmt) == 0)
        return;
    hsStatusMessage(plString::IFormat(fmt,args).c_str());
}

//
// parsing stateDesc
// read name, version
// return true to skip the next token read
//
bool plSDLParser::IParseStateDesc(const plFileName& fileName, hsStream* stream, char token[],
                                  plStateDescriptor*& curDesc) const
{   
    plSDL::DescriptorList* descList = &plSDLMgr::GetInstance()->fDescriptors;

    bool ok = true;

    //
    // NAME
    //
//  curDesc=plSDLMgr::GetInstance()->FindDescriptor(token, plSDL::kLatestVersion);
//  if (!curDesc)
    {
        curDesc = new plStateDescriptor;
        curDesc->SetName(token);

        DebugMsg("SDL: DESC name=%s", token);
    }
    
    //
    // {
    //
    stream->GetToken(token, kTokenLen); // skip '{'
    
    //
    // VERSION
    //
    if (stream->GetToken(token, kTokenLen)) 
    {
        if (!strcmp(token, "VERSION"))
        {
            // read desc version
            hsAssert(curDesc, plFormat("Syntax problem with .sdl file, fileName={}", fileName).c_str());
            if (stream->GetToken(token, kTokenLen))
            {
                int v=atoi(token);
                curDesc->SetVersion(v);
                DebugMsg("\tVersion=%d", v);
            }               
        }
        else
        {
            hsAssert(false, plFormat("Error parsing state desc, missing VERSION, fileName={}", fileName).c_str());
            ok = false;
        }
    }
    else
    {
        hsAssert(false, plFormat("Error parsing state desc, fileName={}", fileName).c_str());
        ok = false;
    }

    if ( ok )
    {
        ok = ( plSDLMgr::GetInstance()->FindDescriptor(curDesc->GetName(), curDesc->GetVersion())==nil );
        if ( !ok )
        {
            plString err = plFormat("Found duplicate SDL descriptor for {} version {}.\nFailed to parse file: {}",
                                    curDesc->GetName(), curDesc->GetVersion(), fileName);
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
bool plSDLParser::IParseVarDesc(const plFileName& fileName, hsStream* stream, char token[],
                                plStateDescriptor*& curDesc, plVarDescriptor*& curVar) const
{
    hsAssert(curDesc, plFormat("Syntax problem with .sdl file, fileName={}", fileName).c_str());
    if ( !curDesc )
        return false;

    bool skipNext=false;
    plString dbgStr;
    static char seps[] = "( ,)[]";
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
        hsAssert(stateDesc, plFormat("can't find nested state desc reference {}, fileName={}",
                 sdlName, fileName).c_str());
        curVar = new plSDVarDescriptor(stateDesc);
    }
    else
        curVar = new plSimpleVarDescriptor;
    
    curDesc->AddVar(curVar);
    bool ok=curVar->SetType(token);
    hsAssert(ok, plFormat("Variable 'type' syntax problem with .sdl file, type={}, fileName={}",
                          token, fileName).c_str());
    dbgStr = plFormat("\tVAR Type={} ", token);
    
    //
    // NAME (foo[1])
    //          
    if (stream->GetToken(token, kTokenLen))
    {
        hsAssert(strstr(token, "[") != nullptr && strstr(token, "]") != nullptr,
                 plFormat("invalid var syntax, missing [x], fileName={}", fileName).c_str());
        char* ptr = strtok( token, seps );  // skip [
        
        hsAssert(curVar, plFormat("Missing current var.  Syntax problem with .sdl file, fileName={}", fileName).c_str());
        curVar->SetName(token);
        //
        // COUNT
        //
        char* cntTok=strtok(nil, seps);     // kill ]
        int cnt = cntTok ? atoi(cntTok) : 0;
        curVar->SetCount(cnt);
        if (cnt==0)
            curVar->SetVariableLength(true);
        dbgStr += plFormat("Name={}[{}]", curVar->GetName(), cnt);
    }
    
    //
    // optional tokens: DEFAULT, INTERNAL
    //
    while (stream->GetToken(token, kTokenLen))
    {
        if (!strcmp(token, "DEFAULT"))
        {
            hsAssert(curVar, plFormat("Syntax problem with .sdl file, fileName={}", fileName).c_str());
            // read state var type
            
            plString defaultStr;
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
            if (!defaultStr.IsEmpty())
            {
                curVar->SetDefault(defaultStr);
                dbgStr += " DEFAULT=" + defaultStr;
            }
        }
        else
        if (!strcmp(token, "DISPLAYOPTION"))
        {
            hsAssert(curVar, plFormat("Syntax problem with .sdl file, fileName={}", fileName).c_str());
            dbgStr += plString(" ") + token;

            bool read=stream->GetToken(token, kTokenLen);
            if (read)
            {
                plString oldOptions=curVar->GetDisplayOptions();
                if (!oldOptions.IsEmpty())
                    oldOptions += ",";
                oldOptions += token;
                curVar->SetDisplayOptions(oldOptions);
                dbgStr += plString("=") + token;
                if (!stricmp(token, "hidden"))
                    curVar->SetInternal(true);
            }
            else
            {
                hsAssert(false, plFormat("missing displayOption string, fileName={}", fileName).c_str());
            }
        }
        else
        if (!strcmp(token, "DEFAULTOPTION"))
        {
            hsAssert(curVar, plFormat("Syntax problem with .sdl file, fileName={}", fileName).c_str());
            dbgStr += plString(" ") + token;

            bool read=stream->GetToken(token, kTokenLen);
            if (read)
            {
                dbgStr += plString("=") + token;
                if (!stricmp(token, "vault"))
                    curVar->SetAlwaysNew(true);
            }
            else
            {
                hsAssert(false, plFormat("missing defaultOption string, fileName={}", fileName).c_str());
            }
        }

#if 1   // delete me in May 2003
        else
        if (!strcmp(token, "INTERNAL"))
        {
            hsAssert(curVar, plFormat("Syntax problem with .sdl file, fileName={}", fileName).c_str());
            curVar->SetInternal(true);
            dbgStr += plString(" ") + token;
        }
        else
        if (!strcmp(token, "PHASED"))
        {
            hsAssert(curVar, plFormat("Syntax problem with .sdl file, fileName={}", fileName).c_str());
            curVar->SetAlwaysNew(true);
            dbgStr += plString(" ") + token;
        }
#endif
        else
        {
            skipNext=true;
            break;
        }
    }

    DebugMsg(dbgStr.c_str());

    return skipNext;
}

//
// create state descriptor from sdl file.
// return false on err.
//
bool plSDLParser::ILoadSDLFile(const plFileName& fileName) const
{
    DebugMsg("Parsing SDL file %s", fileName.AsString().c_str());

    hsStream* stream = plStreamSource::GetInstance()->GetFile(fileName);
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
            curVar=nil;     // start fresh
            continue;
        }
        
        if (!strcmp(token, "STATEDESC"))
        {
            parsingStateDesc=true;
            curDesc=nil;    // start fresh
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
                break;  // failed to parse state desc
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
    plFileName sdlDir = plSDLMgr::GetInstance()->GetSDLDir();
    DebugMsg("SDL: Reading latest descriptors from directory %s", sdlDir.AsString().c_str());

    // Get the names of all the sdl files
    std::vector<plFileName> files = plStreamSource::GetInstance()->GetListOfNames(sdlDir, "sdl");

    bool ret=true;
    int cnt=0;
    for (int i = 0; i < files.size(); i++)
    {
        if (!ILoadSDLFile(files[i]))
        {
            plNetApp* netApp = plSDLMgr::GetInstance()->GetNetApp();
            if (netApp)
                netApp->ErrorMsg("Error loading SDL file %s", files[i].AsString().c_str());
            else
                hsStatusMessageF("Error loading SDL file %s", files[i].AsString().c_str());
            ret=false;
        }
        else
            cnt++;
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
