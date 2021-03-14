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
#include "plSDL.h"

#include "hsStream.h"
#include "hsTimer.h"

#include "pnNetCommon/plNetApp.h"
#include "pnNetCommon/pnNetCommon.h"

#include "plNetMessage/plNetMessage.h"

const ST::string plSDL::kAgeSDLObjectName = ST_LITERAL("AgeSDLHook");

// static 
const uint8_t plStateDataRecord::kIOVersion=6;

//
// helper 
//
void plSDL::VariableLengthRead(hsStream* s, int size, int* val)
{
    // hsAssert(size, "unexpected size");
    
    if (size < (1<<8))
        *val = s->ReadByte();
    else
    if (size < (1<<16))
        *val = s->ReadLE16();
    else
        *val = s->ReadLE32();
}

//
// helper
//
void plSDL::VariableLengthWrite(hsStream* s, int size, int val)
{
    // hsAssert(size, "unexpected size");

    if (size < (1<<8))
    {
        hsAssert(val < (1<<8), "SDL data loss");
        s->WriteByte((uint8_t)val);
    }
    else
    if (size < (1<<16))
    {
        hsAssert(val < (1<<16), "SDL data loss");
        s->WriteLE16((uint16_t)val);
    }
    else
        s->WriteLE32(val);
}

/////////////////////////////////////////////////////////////////////////////////
// State Data
/////////////////////////////////////////////////////////////////////////////////
plStateDataRecord::plStateDataRecord(const ST::string& name, int version) : fFlags(0)
, fDescriptor()
{
    SetDescriptor(name, version);
}

plStateDataRecord::plStateDataRecord(plStateDescriptor* sd) : fFlags(0)
, fDescriptor()
{
    IInitDescriptor(sd);
}

plStateDataRecord::~plStateDataRecord() 
{ 
    IDeleteVarsList(fVarsList);
    IDeleteVarsList(fSDVarsList);
}

void plStateDataRecord::SetDescriptor(const ST::string& name, int version)
{
    IInitDescriptor(name, version);
}


void plStateDataRecord::IDeleteVarsList(VarsList& vars)
{
    for (plStateVariable* var : vars)
        delete var;
    vars.clear();
}

void plStateDataRecord::IInitDescriptor(const ST::string& name, int version)
{
    plStateDescriptor* sd = plSDLMgr::GetInstance()->FindDescriptor(name, version);
    //hsAssert( sd, ST::format("Failed to find sdl descriptor: {},{}. Missing legacy descriptor?", name, version ).c_str() );
    if (sd)
        IInitDescriptor(sd);
}

//
// Point to descriptor.
// setup state variables which correspond to the state descriptor.
//
void plStateDataRecord::IInitDescriptor(const plStateDescriptor* sd)
{
    // pt to state desc
    fDescriptor=sd;

    // delete old vars
    IDeleteVarsList(fVarsList);
    IDeleteVarsList(fSDVarsList);

    // create vars defined by state desc
    if (sd)
    {
        for(int i = 0; i < sd->GetNumVars(); ++i)
        {
            if (plVarDescriptor* vd = sd->GetVar(i))
            {
                if (vd->GetAsSDVarDescriptor())
                {   // it's a var which references another state descriptor.
                    fSDVarsList.push_back(new plSDStateVariable(vd->GetAsSDVarDescriptor()));
                }
                else
                {
                    hsAssert(vd->GetAsSimpleVarDescriptor(), "var class problem");
                    fVarsList.push_back(new plSimpleStateVariable(vd->GetAsSimpleVarDescriptor()));
                }
            }
        }
    }
}

///////////////////
// DIRTY VARS
///////////////////
int plStateDataRecord::IGetNumDirtyVars(const VarsList& vars) const
{
    int i, cnt=0;
    for(i=0;i<vars.size();i++)
        if (vars[i]->IsDirty())
            cnt++;
    return cnt;
}

int plStateDataRecord::IGetDirtyVars(const VarsList& varsIn, VarsList* varsOut) const
{
    int i;
    for(i=0;i<varsIn.size();i++)
        if (varsIn[i]->IsDirty())
            varsOut->push_back(varsIn[i]);
    return varsOut->size();
}

bool plStateDataRecord::IHasDirtyVars(const VarsList& vars) const
{
    int i;
    for(i=0;i<vars.size();i++)
        if (vars[i]->IsDirty())
            return true;
    return false;
}

////////////////
// USED VARS
////////////////
int plStateDataRecord::IGetNumUsedVars(const VarsList& vars) const
{
    int i, cnt=0;
    for(i=0;i<vars.size();i++)
        if (vars[i]->IsUsed())
            cnt++;
    return cnt;
}

int plStateDataRecord::IGetUsedVars(const VarsList& varsIn, VarsList* varsOut) const
{
    int i;
    for(i=0;i<varsIn.size();i++)
        if (varsIn[i]->IsUsed())
            varsOut->push_back(varsIn[i]);
    return varsOut->size();
}

bool plStateDataRecord::IHasUsedVars(const VarsList& vars) const
{
    int i;
    for(i=0;i<vars.size();i++)
        if (vars[i]->IsUsed())
            return true;
    return false;
}

///////////////////////////////////////
// IO
///////////////////////////////////////

//
// read state vars and indices, return true on success
//
bool plStateDataRecord::Read(hsStream* s, float timeConvert, uint32_t readOptions)
{
    fFlags = s->ReadLE16();
    uint8_t ioVersion = s->ReadByte();
    if (ioVersion != kIOVersion)
        return false;

    //
    // read simple var data
    //
    hsAssert(fDescriptor, "State Data Record has nil SDL descriptor");
    if (!fDescriptor)
        return false;

    int num;
    plSDL::VariableLengthRead(s, fDescriptor->GetNumVars(), &num );

    // if we are readeing the entire list, we don't need to read each index
    bool all = (num==fVarsList.size());

    int i;
    try
    {
        for(i=0;i<num;i++)
        {
            int idx;
            if (!all)
                plSDL::VariableLengthRead(s, fDescriptor->GetNumVars(), &idx );
            else
                idx=i;
            if (idx>=fVarsList.size() || !fVarsList[idx]->ReadData(s, timeConvert, readOptions))
            {
                if (plSDLMgr::GetInstance()->GetNetApp())
                    plSDLMgr::GetInstance()->GetNetApp()->ErrorMsg("Failed reading SDL, desc {}",
                            fDescriptor ? fDescriptor->GetName().c_str("?") : "?");
                return false;
            }
        }
    }
    catch (const std::exception &e)
    {
        hsAssert(false,
            ST::format("Something bad happened ({}) while reading simple var data, desc:{}",
                       e.what(), fDescriptor ? fDescriptor->GetName() : "?").c_str());
        return false;
    }
    catch (...)
    {
        hsAssert(false,
            ST::format("Something bad happened while reading simple var data, desc:{}",
                       fDescriptor ? fDescriptor->GetName() : "?").c_str());
        return false;
    }

    //
    // read nested var data
    //
    plSDL::VariableLengthRead(s, fDescriptor->GetNumVars(), &num );

    // if we are readeing the entire list, we don't need to write each index
    all = (num==fSDVarsList.size());

    try
    {
        for(i=0;i<num;i++)
        {
            int idx;
            if (!all)
                plSDL::VariableLengthRead(s, fDescriptor->GetNumVars(), &idx );
            else
                idx=i;
            if (idx>=fSDVarsList.size() || !fSDVarsList[idx]->ReadData(s, timeConvert, readOptions))    // calls plStateDataRecord::Read recursively
            {
                if (plSDLMgr::GetInstance()->GetNetApp())
                    plSDLMgr::GetInstance()->GetNetApp()->ErrorMsg("Failed reading nested SDL, desc {}",
                            fDescriptor ? fDescriptor->GetName().c_str("?") : "?");
                return false;
            }
        }
    }
    catch (const std::exception &e)
    {
        hsAssert(false,
            ST::format("Something bad happened ({}) while reading nested var data, desc:{}",
                       e.what(), fDescriptor ? fDescriptor->GetName() : "?").c_str());
        return false;
    }
    catch (...)
    {
        hsAssert(false,
            ST::format("Something bad happened while reading nested var data, desc:{}",
                       fDescriptor ? fDescriptor->GetName() : "?").c_str());
        return false;
    }

    // convert to latest descriptor
    // Only really need to do this the first time this descriptor is read...
    plStateDescriptor* latestDesc=plSDLMgr::GetInstance()->FindDescriptor(fDescriptor->GetName(), plSDL::kLatestVersion);
    hsAssert(latestDesc, ST::format("Failed to find latest sdl descriptor for: {}", fDescriptor->GetName()).c_str());
    bool forceConvert = (readOptions&plSDL::kForceConvert)!=0;
    if ( latestDesc && ( forceConvert || ( fDescriptor->GetVersion()!=latestDesc->GetVersion() ) ) )
    {
        DumpToObjectDebugger( "PreConvert" );
        ConvertTo( latestDesc, forceConvert );
        DumpToObjectDebugger( "PostConvert" );
    }

    return true;    // ok
}

//
// write out the state vars, along with their index
//
void plStateDataRecord::Write(hsStream* s, float timeConvert, uint32_t writeOptions) const
{
#ifdef HS_DEBUGGING
    if ( !plSDLMgr::GetInstance()->AllowTimeStamping() && (writeOptions & plSDL::kWriteTimeStamps) )
    {
        hsAssert( false, "SDL behavior flags disallow var timestamping on write.\nRemoving kWriteTimeStamps flag from writeOptions..." );
        writeOptions &= ~plSDL::kWriteTimeStamps;
    }
#endif

    s->WriteLE16((uint16_t)fFlags);
    s->WriteByte(kIOVersion);

    //
    // write simple vars
    //
    bool dirtyOnly = (writeOptions & plSDL::kDirtyOnly) != 0;
    int num = dirtyOnly ? GetNumDirtyVars() : GetNumUsedVars();
    plSDL::VariableLengthWrite(s, fDescriptor->GetNumVars(), num ); // write affected vars count

    // if we are writing he entire list, we don't need to write each index
    bool all = (num==fVarsList.size());

    int i;
    for(i=0;i<fVarsList.size(); i++)
    {
        if ( (dirtyOnly && fVarsList[i]->IsDirty()) ||
            (!dirtyOnly && fVarsList[i]->IsUsed()) )
        {
            if (!all)
                plSDL::VariableLengthWrite(s, fDescriptor->GetNumVars(), i );   // index
            fVarsList[i]->WriteData(s, timeConvert, writeOptions);              // data
        }
    }

    //
    // write nested vars
    //
    num = dirtyOnly ? GetNumDirtySDVars() : GetNumUsedSDVars();
    plSDL::VariableLengthWrite(s, fDescriptor->GetNumVars(), num ); // write affected vars count

    // if we are writing he entire list, we don't need to write each index
    all = (num==fSDVarsList.size());

    for(i=0;i<fSDVarsList.size(); i++)
    {
        if ( (dirtyOnly && fSDVarsList[i]->IsDirty()) ||
            (!dirtyOnly && fSDVarsList[i]->IsUsed()) )
        {
            if (!all)
                plSDL::VariableLengthWrite(s, fDescriptor->GetNumVars(), i );   // index
            fSDVarsList[i]->WriteData(s, timeConvert, writeOptions);            // data, calls plStateDataRecord::Write recursively
        }
    }
}

//
// STATIC - read prefix header.  returns true on success 
//
bool plStateDataRecord::ReadStreamHeader(hsStream* s, ST::string* name, int* version, plUoid* objUoid)
{
    uint16_t savFlags = s->ReadLE16();
    if (!(savFlags & plSDL::kAddedVarLengthIO))     // using to establish a new version in the header, can delete in 8/03
    {
        *name = "";
        return false;       // bad version
    }

    *name = s->ReadSafeString();
    *version = s->ReadLE16();
    
    if (objUoid)
    {
        hsAssert(savFlags & plSDL::kHasUoid, "SDL state data rec expecting to read a uoid, but there isn't one");
    }

    if (savFlags & plSDL::kHasUoid)
    {
        if (objUoid)
            objUoid->Read(s);
        else
        {
            plUoid tmp;
            tmp.Read(s);
        }
    }   
    
    return true;    // ok
}

//
// non-static - write prefix header. helper fxn
//
void plStateDataRecord::WriteStreamHeader(hsStream* s, plUoid* objUoid) const
{
    uint16_t savFlags=plSDL::kAddedVarLengthIO;       // using to establish a new version in the header, can delete in 8/03
    if (objUoid)
        savFlags |= plSDL::kHasUoid;

    s->WriteLE16(savFlags);
    s->WriteSafeString(GetDescriptor()->GetName());         
    s->WriteLE16((int16_t)GetDescriptor()->GetVersion());
    if (objUoid)
        objUoid->Write(s);
}

//
// create and prepare a net msg with this data
//
plNetMsgSDLState* plStateDataRecord::PrepNetMsg(float timeConvert, uint32_t writeOptions) const
{
    // save to stream
    hsRAMStream stream; 
    WriteStreamHeader(&stream);
    Write(&stream, timeConvert, writeOptions);
    
    // fill in net msg
    plNetMsgSDLState* msg;  
    if (writeOptions & plSDL::kBroadcast)
        msg = new plNetMsgSDLStateBCast;
    else
        msg = new plNetMsgSDLState;
    
    msg->StreamInfo()->CopyStream(&stream);
    return msg;
}

//
// Destroys 'this' and makes a total copy of other
//
void plStateDataRecord::CopyFrom(const plStateDataRecord& other, uint32_t writeOptions/*=0*/)
{
    fFlags = other.GetFlags();
    IInitDescriptor(other.GetDescriptor());
    int i;
    for(i=0;i<other.GetNumVars();i++)
    {
        if (other.GetVar(i)->IsUsed())
            GetVar(i)->CopyData(other.GetVar(i),writeOptions);
    }

    for(i=0;i<other.GetNumSDVars();i++)
    {
        if (other.GetSDVar(i)->IsUsed())
            GetSDVar(i)->CopyFrom(other.GetSDVar(i),writeOptions);
    }
}

//
// Find the data items which are dirty in 'other' and 
// copy them to my corresponding item.
// Requires that records have the same descriptor.
//
void plStateDataRecord::UpdateFrom(const plStateDataRecord& other, uint32_t writeOptions/*=0*/)
{
    if ( GetDescriptor()->GetVersion()!=other.GetDescriptor()->GetVersion() )
    {
        plStateDescriptor* sd=plSDLMgr::GetInstance()->FindDescriptor( other.GetDescriptor()->GetName(), other.GetDescriptor()->GetVersion() );
        hsAssert(sd, ST::format("Failed to find sdl descriptor {},{}. Missing legacy descriptor?",
                                other.GetDescriptor()->GetName(), other.GetDescriptor()->GetVersion()).c_str());
        ConvertTo( sd );
    }

    hsAssert(other.GetDescriptor()==fDescriptor, 
        ST::format("descriptor mismatch in UpdateFromDirty, SDL={},{} version {} {}",
            GetDescriptor()->GetName(), other.GetDescriptor()->GetName(),
            GetDescriptor()->GetVersion(), other.GetDescriptor()->GetVersion()).c_str());

    bool dirtyOnly = (writeOptions & plSDL::kDirtyOnly);

    int i;
    for(i=0;i<other.GetNumVars();i++)
    {
        if ( (dirtyOnly && other.GetVar(i)->IsDirty()) || (!dirtyOnly && other.GetVar(i)->IsUsed()) )
        {
            GetVar(i)->NotifyStateChange(other.GetVar(i), GetDescriptor()->GetName());  // see if there is enough difference to send state chg notification
            GetVar(i)->CopyData(other.GetVar(i), writeOptions );    // simple vars get copied completely, non-partial
        }
    }

    for(i=0;i<other.GetNumSDVars();i++)
    {
        if ( (dirtyOnly && other.GetSDVar(i)->IsDirty()) || (!dirtyOnly && other.GetSDVar(i)->IsUsed()) )
            GetSDVar(i)->UpdateFrom(other.GetSDVar(i), writeOptions);
    }
}

//
// dirty my items which are different from the corresponding one in 'other'.
// Requires that records have the same descriptor.
//
void plStateDataRecord::FlagDifferentState(const plStateDataRecord& other)
{
    if (other.GetDescriptor()==fDescriptor)
    {
        int i;
        for(i=0;i<other.GetNumVars();i++)
        {
            bool diff = (GetVar(i)->IsUsed() && ! (*other.GetVar(i) == *GetVar(i)) );
            GetVar(i)->SetDirty(diff);
        }

        for(i=0;i<other.GetNumSDVars();i++)
        {
            bool diff = (GetSDVar(i)->IsUsed() && ! (*other.GetSDVar(i) == *GetSDVar(i)) );
            GetSDVar(i)->SetDirty(diff);
        }
    }
    else
    {
        hsAssert(false, ST::format("descriptor mismatch in FlagDifferentState, mine {} {}, other {} {}",
            fDescriptor->GetName(), fDescriptor->GetVersion(),
            other.GetDescriptor()->GetName(), other.GetDescriptor()->GetVersion()).c_str());
    }
}

//
// dirty my items which are flagged as alwaysNew.
//
void plStateDataRecord::FlagAlwaysNewState()
{
    int i;
    for(i=0;i<GetNumVars();i++)
    {
        bool newer= (GetVar(i)->IsUsed() && GetVar(i)->GetVarDescriptor()->IsAlwaysNew());  // flagged to be always new
        GetVar(i)->SetDirty(newer);
    }

    for(i=0;i<GetNumSDVars();i++)
    {
        if (GetSDVar(i)->IsUsed())
        {
            GetSDVar(i)->FlagAlwaysNewState();
        }
    }
}

//
// dirty my items which are newer than the corresponding one in 'other'.
// Requires that records have the same descriptor.
//
void plStateDataRecord::FlagNewerState(const plStateDataRecord& other, bool respectAlwaysNew)
{
    if (other.GetDescriptor()==fDescriptor)
    {
        int i;
        for(i=0;i<other.GetNumVars();i++)
        {
            bool newer= (GetVar(i)->IsUsed() && 
                (GetVar(i)->GetTimeStamp() > other.GetVar(i)->GetTimeStamp() ||         // later timestamp
                (respectAlwaysNew && GetVar(i)->GetVarDescriptor()->IsAlwaysNew())) );  // flagged to be always new
            GetVar(i)->SetDirty(newer);
        }

        for(i=0;i<other.GetNumSDVars();i++)
        {
            if (GetSDVar(i)->IsUsed())
            {
                GetSDVar(i)->FlagNewerState(*other.GetSDVar(i), respectAlwaysNew);
            }
        }
    }
    else
    {
        hsAssert(false, ST::format("descriptor mismatch in FlagNewerState, mine {} {}, other {} {}",
            fDescriptor->GetName(), fDescriptor->GetVersion(),
            other.GetDescriptor()->GetName(), other.GetDescriptor()->GetVersion()).c_str());
    }
}

//
// assumes matching state descriptors
//
bool plStateDataRecord::operator==(const plStateDataRecord &other) const    
{
    // hsAssert(other.GetDescriptor()==fDescriptor, "descriptor mismatch in equality check");
    if (other.GetDescriptor()!=fDescriptor)
        return false;

    int i;
    for(i=0;i<other.GetNumVars();i++)
    {
        if (! (*other.GetVar(i) == *GetVar(i)) )
            return false;
    }

    for(i=0;i<other.GetNumSDVars();i++)
    {
        if (! (*other.GetSDVar(i) == *GetSDVar(i)) )
            return false;
    }

    return true;
}

//
// Converting from a var in an older dtor to a corresponding var in a newer dtor.
// If possible, use the value from fromVar (type converted, if necessary, to the type of toVar),
//      o/wise use toVar's default value.  
// Also handle the possiblity of different counts.
// Put the final value in the otherData buffer.
// return false on err
//
bool plStateDataRecord::IConvertVar(plSimpleStateVariable* fromVar, plSimpleStateVariable* toVar /* empty */, bool force)
{
    // first fill the dst using it's default value
    toVar->SetFromDefaults(false);
    
    if (!fromVar)
        return true;    // no corresponding var in the old state, done

    // Copy the value to the dst, converting if necessary
    fromVar->ConvertTo(toVar->GetSimpleVarDescriptor(),force);
    toVar->CopyData(fromVar, plSDL::kWriteTimeStamps | plSDL::kKeepDirty);

    return true;    // ok
}

plStateVariable* plStateDataRecord::IFindVar(const VarsList& vars, const ST::string& name) const
{
    for (int i = 0; i < vars.size(); i++)
    {
        if (!vars[i]->GetVarDescriptor()->GetName().compare_i(name))
            return vars[i];
    }

    if (plSDLMgr::GetInstance()->GetNetApp())
        plSDLMgr::GetInstance()->GetNetApp()->ErrorMsg("Failed to find SDL var {}", name);

    return nullptr;
}

//
// try to convert our data (old version) to other's (new) version.
// return false on err.
//
bool plStateDataRecord::ConvertTo( plStateDescriptor* other, bool force )
{
    if (!other && !force)
        return false;   // err

    hsAssert(!fDescriptor->GetName().compare_i(other->GetName()), "descriptor mismatch");

    if ( !force && (other == fDescriptor || other->GetVersion()==fDescriptor->GetVersion()))
        return true;    // ok, nothing to do

    hsAssert(other->GetVersion()>=fDescriptor->GetVersion(), "converting to an older state descriptor version?");

    hsLogEntry( plNetApp::StaticDebugMsg( "SDR(0x{x}) converting sdl record {} from version {} to {} (force:{})",
        uintptr_t(this), fDescriptor->GetName(), fDescriptor->GetVersion(), other->GetVersion(), force ) );

    // make other StateData to represent other descriptor, 
    // this will be the destination for the convert operation
    plStateDataRecord otherStateData(other);

    // for each var in the other dtor,
    //      use the corresponding value in mine (type converted if necessary),
    //      or use other's default value.  Put the final value in the otherData buffer
    int i;
    for(i=0;i<otherStateData.GetNumVars(); i++)
    {
        // get other var info
        plSimpleStateVariable* otherVar = otherStateData.GetVar(i);
        ST::string otherVarName = otherVar->GetVarDescriptor()->GetName();

        // find corresponding var in my data
        plSimpleStateVariable* myVar=FindVar(otherVarName);
        IConvertVar(myVar /* fromVar */, otherVar /* toVar */, force);
    }
    
    // for each nested stateDesc in the other guy,
    // run the convert operation on it recursively.
    for(i=0;i<otherStateData.GetNumSDVars(); i++)
    {
        plSDStateVariable* otherSDVar = otherStateData.GetSDVar(i);
        ST::string otherSDVarName = otherSDVar->GetVarDescriptor()->GetName();

        // find corresponding var in my data
        plSDStateVariable* mySDVar=FindSDVar(otherSDVarName);
        if (mySDVar)
        {
            mySDVar->ConvertTo( otherSDVar, force );
            otherSDVar->CopyFrom( mySDVar );
        }
    }

    // adopt new descriptor and data
    CopyFrom(otherStateData, plSDL::kWriteTimeStamps | plSDL::kKeepDirty);

    return true;
}

//
//
//
void plStateDataRecord::DumpToObjectDebugger(const char* msg, bool dirtyOnly, int level) const
{
    plNetObjectDebuggerBase* dbg = plNetObjectDebuggerBase::GetInstance();
    if (!dbg)
        return;

    ST::string pad = ST::string::fill(level * 3, ' ');

    int numVars = dirtyOnly ? GetNumDirtyVars() : GetNumUsedVars();
    int numSDVars = dirtyOnly ? GetNumDirtySDVars() : GetNumUsedSDVars();

    dbg->LogMsg(fAssocObject.IsValid() ? fAssocObject.GetObjectName().c_str() : " ");
    if (msg)
        dbg->LogMsg(ST::format("{}{}", pad, msg).c_str());

    dbg->LogMsg(ST::format("{}SDR({#x}), desc={}, showDirty={}, numVars={}, vol={}",
        pad, (uintptr_t)this, fDescriptor->GetName(), dirtyOnly, numVars+numSDVars, fFlags&kVolatile).c_str());

    // dump simple vars
    for (size_t i=0; i<fVarsList.size(); i++)
    {
        if ( (dirtyOnly && fVarsList[i]->IsDirty()) ||  (!dirtyOnly && fVarsList[i]->IsUsed()) )
        {
            fVarsList[i]->DumpToObjectDebugger(dirtyOnly, level+1);     
        }
    }
 
    // dump nested vars 
    for (size_t i=0; i<fSDVarsList.size(); i++)
    {
        if ( (dirtyOnly && fSDVarsList[i]->IsDirty()) || (!dirtyOnly && fSDVarsList[i]->IsUsed()) )
        {
            fSDVarsList[i]->DumpToObjectDebugger(dirtyOnly, level+1);       
        }
    }
}

void plStateDataRecord::DumpToStream(hsStream* stream, const char* msg, bool dirtyOnly, int level) const
{
    ST::string pad = ST::string::fill(level * 3, ' ');

    int numVars = dirtyOnly ? GetNumDirtyVars() : GetNumUsedVars();
    int numSDVars = dirtyOnly ? GetNumDirtySDVars() : GetNumUsedSDVars();

    ST::string logStr = fAssocObject.IsValid() ? fAssocObject.GetObjectName() : ST_LITERAL(" ");

    stream->Write(logStr.size(), logStr.c_str());
    if (msg)
    {
        logStr = ST::format("{}{}", pad, msg);
        stream->Write(logStr.size(), logStr.c_str());
    }

    logStr = ST::format("{}SDR({#x}), desc={}, showDirty={}, numVars={}, vol={}",
        pad, (uintptr_t)this, fDescriptor->GetName(), dirtyOnly, numVars+numSDVars, fFlags&kVolatile);
    stream->Write(logStr.size(), logStr.c_str());

    // dump simple vars
    for (size_t i=0; i<fVarsList.size(); i++)
    {
        if ( (dirtyOnly && fVarsList[i]->IsDirty()) ||  (!dirtyOnly && fVarsList[i]->IsUsed()) )
        {
            fVarsList[i]->DumpToStream(stream, dirtyOnly, level+1);     
        }
    }
 
    // dump nested vars
    for (size_t i=0; i<fSDVarsList.size(); i++)
    {
        if ( (dirtyOnly && fSDVarsList[i]->IsDirty()) || (!dirtyOnly && fSDVarsList[i]->IsUsed()) )
        {
            fSDVarsList[i]->DumpToStream(stream, dirtyOnly, level+1);       
        }
    }

    logStr = "\n";
    stream->Write(logStr.size(), logStr.c_str());
}

void plStateDataRecord::SetFromDefaults(bool timeStampNow)
{
    int i;
    // set simple vars
    for(i=0;i<fVarsList.size(); i++)
    {
        fVarsList[i]->SetFromDefaults(timeStampNow);
    }
 
    // set nested vars  
    for(i=0;i<fSDVarsList.size(); i++)
    {
        fSDVarsList[i]->SetFromDefaults(timeStampNow);
    }
}

void plStateDataRecord::TimeStampDirtyVars()
{
    int i;
    // set simple vars
    for(i=0;i<fVarsList.size(); i++)
    {
        if ( fVarsList[i]->IsDirty() )
            fVarsList[i]->TimeStamp();
    }
 
    // set nested vars  
    for(i=0;i<fSDVarsList.size(); i++)
    {
        if ( fSDVarsList[i]->IsDirty() )
            fSDVarsList[i]->TimeStamp();
    }
}
