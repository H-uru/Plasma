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
#include "plConfigInfo.h"

#include "HeadSpin.h"
#include <fstream>
#include <algorithm>
#include <sstream>

const plString& plConfigInfo::GlobalSection()
{
    static plString section("global");
    return section;
}

plConfigInfo::plConfigInfo()
{
}

plConfigInfo::plConfigInfo(const plConfigInfo & src)
:   fSections(src.fSections)
{
}

plConfigInfo & plConfigInfo::operator =(const plConfigInfo & src)
{
    fSections = src.fSections;
    return *this;
}

void plConfigInfo::Clear()
{
    fSections.clear();
}

void plConfigInfo::RemoveSection(const plString & section)
{
    fSections.erase(section);
}

void plConfigInfo::RemoveKey(const plString & section, const plString & key)
{
    Sections::iterator si = fSections.find(section);
    if (si != fSections.end())
        fSections[section].RemoveKey(key);
}

bool plConfigInfo::HasSection(const plString & section) const
{
    return fSections.find(section)!=fSections.end();
}

bool plConfigInfo::HasKey(const plString & section, const plString & key)
{
    Sections::iterator si = fSections.find(section);
    if (si == fSections.end())
        return false;
    return (si->second.HasKey(key));
}

bool plConfigInfo::HasKeyAny(const plString & key)
{
    for (Sections::iterator si=fSections.begin(); si!=fSections.end(); ++si)
    {
        if (si->second.HasKey(key))
            return true;
    }
    return false;
}

bool plConfigInfo::HasKeyIn(const plString & key, const char * section1, ...)
{
    const char * section = section1;
    va_list va;
    va_start(va,section1);
    std::vector<plString> sections;
    while (section)
    {
        sections.push_back( section );
        section = va_arg(va,const char *);
    }
    va_end(va);
    return HasKeyIn( key, sections );
}

bool plConfigInfo::HasKeyIn(const plString & key, const std::vector<plString> & sections )
{
    for ( int i=0; i<sections.size(); i++ )
    {
        if (HasSection(sections[i]))
        {
            if (fSections[sections[i]].HasKey(key))
                return true;
        }
    }
    return false;
}

bool plConfigInfo::KeyHasValue(const plString & section, const plString & key, const plString & value)
{
    Sections::iterator si = fSections.find(section);
    if (si == fSections.end())
        return false;
    return si->second.KeyHasValue(key,value);
}

bool plConfigInfo::KeyHasValue(const plString & section, const plString & key, int value)
{
    Sections::iterator si = fSections.find(section);
    if (si == fSections.end())
        return false;
    return si->second.KeyHasValue(key,value);
}

bool plConfigInfo::KeyHasValue(const plString & section, const plString & key, double value)
{
    Sections::iterator si = fSections.find(section);
    if (si == fSections.end())
        return false;
    return si->second.KeyHasValue(key,value);
}

bool plConfigInfo::AddValue(const plString & section, const plString & key, const plString & value, KAddValueMode mode)
{
    return fSections[section].AddValue(key,value,mode);
}

bool plConfigInfo::AddValue(const plString & section, const plString & key, int value, KAddValueMode mode)
{
    return fSections[section].AddValue(key,value,mode);
}

bool plConfigInfo::AddValue(const plString & section, const plString & key, double value, KAddValueMode mode)
{
    return fSections[section].AddValue(key,value,mode);
}

bool plConfigInfo::AddValues(const plString & section, const plString & key, const std::vector<plString> & values, KAddValueMode mode)
{
    return fSections[section].AddValues(key,values);
}

plKeysAndValues plConfigInfo::GetSection(const plString & section, bool & found)
{
    found = HasSection(section);
    if (found)
        return fSections[section];
    else
        return plKeysAndValues(); // empty
}

std::vector<plString> plConfigInfo::GetSectionNames()
{
    std::vector<plString> results;
    for (Sections::const_iterator ii=fSections.begin(); ii!=fSections.end(); ++ii)
        results.push_back(ii->first);
    return results;
}

plString plConfigInfo::GetValue(const plString & section, const plString & key, const plString & defval, bool * outFound) const
{
    return fSections[section].GetValue(key,defval,outFound);
}

int plConfigInfo::GetValue(const plString & section, const plString & key, int defval, bool * outFound) const
{
    return fSections[section].GetValue(key,defval,outFound);
}

double plConfigInfo::GetValue(const plString & section, const plString & key, double defval, bool * outFound) const
{
    return fSections[section].GetValue(key,defval,outFound);
}

std::vector<plString> plConfigInfo::GetAllValues(const plString & section, const plString & key) const
{
    Sections::iterator si = fSections.find(section);
    if (si != fSections.end())
        return si->second.GetAllValues(key);
    return std::vector<plString>();
}


plString plConfigInfo::GetValueAny(const plString & key, const plString & defval, bool * outFound) const
{
    if (outFound) *outFound=false;
    for (Sections::iterator si=fSections.begin(); si!=fSections.end(); ++si)
        if (si->second.HasKey(key))
            return si->second.GetValue(key,defval,outFound);
    return defval;
}

int plConfigInfo::GetValueAny(const plString & key, int defval, bool * outFound) const
{
    if (outFound) *outFound=false;
    for (Sections::iterator si=fSections.begin(); si!=fSections.end(); ++si)
        if (si->second.HasKey(key))
            return si->second.GetValue(key,defval,outFound);
    return defval;
}

double plConfigInfo::GetValueAny(const plString & key, double defval, bool * outFound) const
{
    if (outFound) *outFound=false;
    for (Sections::iterator si=fSections.begin(); si!=fSections.end(); ++si)
        if (si->second.HasKey(key))
            return si->second.GetValue(key,defval,outFound);
    return defval;
}

std::vector<plString> plConfigInfo::GetAllValuesAny(const plString & key) const
{
    for (Sections::iterator si=fSections.begin(); si!=fSections.end(); ++si)
        if (si->second.HasKey(key))
            return si->second.GetAllValues(key);
    return std::vector<plString>();
}


plString plConfigInfo::GetValueIn(const plString & key, const plString & defval, bool * outFound, const char * section1, ...) const
{
    if (outFound) *outFound=false;
    const char * section = section1;
    va_list sections;
    va_start(sections,section1);
    while (section)
    {
        if (HasSection(section))
        {
            plKeysAndValues & kv = fSections[section];
            if (kv.HasKey(key))
                return kv.GetValue(key,defval,outFound);
        }
        section = va_arg(sections,const char *);
    }
    va_end(sections);
    return defval;
}

plString plConfigInfo::GetValueIn(const plString & key, const plString & defval, bool * outFound, const std::vector<plString> & sections ) const
{
    if (outFound) *outFound=false;
    for ( int i=0; i<sections.size(); i++ )
    {
        if (HasSection(sections[i]))
        {
            plKeysAndValues & kv = fSections[sections[i]];
            if (kv.HasKey(key))
                return kv.GetValue(key,defval,outFound);
        }
    }
    return defval;
}

int plConfigInfo::GetValueIn(const plString & key, int defval, bool * outFound, const char * section1, ...) const
{
    if (outFound) *outFound=false;
    const char * section = section1;
    va_list sections;
    va_start(sections,section1);
    while (section)
    {
        if (HasSection(section))
        {
            plKeysAndValues & kv = fSections[section];
            if (kv.HasKey(key))
                return kv.GetValue(key,defval,outFound);
        }
        section = va_arg(sections,const char *);
    }
    va_end(sections);
    return defval;
}

int plConfigInfo::GetValueIn(const plString & key, int defval, bool * outFound, const std::vector<plString> & sections ) const
{
    if (outFound) *outFound=false;
    for ( int i=0; i<sections.size(); i++ )
    {
        if (HasSection(sections[i]))
        {
            plKeysAndValues & kv = fSections[sections[i]];
            if (kv.HasKey(key))
                return kv.GetValue(key,defval,outFound);
        }
    }
    return defval;
}

double plConfigInfo::GetValueIn(const plString & key, double defval, bool * outFound, const char * section1, ...) const
{
    if (outFound) *outFound=false;
    const char * section = section1;
    va_list sections;
    va_start(sections,section1);
    while (section)
    {
        if (HasSection(section))
        {
            plKeysAndValues & kv = fSections[section];
            if (kv.HasKey(key))
                return kv.GetValue(key,defval,outFound);
        }
        section = va_arg(sections,const char *);
    }
    va_end(sections);
    return defval;
}

double plConfigInfo::GetValueIn(const plString & key, double defval, bool * outFound, const std::vector<plString> & sections ) const
{
    if (outFound) *outFound=false;
    for ( int i=0; i<sections.size(); i++ )
    {
        if (HasSection(sections[i]))
        {
            plKeysAndValues & kv = fSections[sections[i]];
            if (kv.HasKey(key))
                return kv.GetValue(key,defval,outFound);
        }
    }
    return defval;
}

std::vector<plString> plConfigInfo::GetAllValuesIn(const plString & key, const char * section1, ...)
{
    const char * section = section1;
    va_list sections;
    va_start(sections,section1);
    std::vector<plString> result;
    while (section)
    {
        if (HasSection(section))
        {
            plKeysAndValues & kv = fSections[section];
            if (kv.HasKey(key))
            {
                std::vector<plString> values = kv.GetAllValues(key);
                result.insert(result.end(),values.begin(),values.end());
            }
        }
        section = va_arg(sections,const char *);
    }
    va_end(sections);
    return result;
}

bool plConfigInfo::GetSectionIterators(Sections::const_iterator & iter, Sections::const_iterator & end) const
{
    iter = fSections.begin();
    end = fSections.end();
    return true;
}

bool plConfigInfo::GetKeyIterators(const plString & section, Keys::const_iterator & iter, Keys::const_iterator & end) const
{
    Sections::const_iterator si = fSections.find(section);
    if (si==fSections.end())
        return false;
    return fSections[section].GetKeyIterators(iter, end);
}

bool plConfigInfo::GetValueIterators(const plString & section, const plString & key, Values::const_iterator & iter, Values::const_iterator & end) const
{
    Sections::const_iterator si = fSections.find(section);
    if (si==fSections.end())
        return false;
    return fSections[section].GetValueIterators(key, iter, end);
}

bool plConfigInfo::ReadFrom(plConfigSource * src, KAddValueMode mode)
{
    return src->ReadInto(*this,mode);
}

bool plConfigInfo::WriteTo(plConfigSource * src)
{
    return src->WriteOutOf(*this);
}


////////////////////////////////////////////////

void plConfigSource::SplitAt(plString & key, plString & value, char splitter, plString & in)
{
    if (in.IsEmpty())
        return;
    
    int t = in.Find(splitter);
    if (t < 0)
    {
        key = in;
        return;
    }
    
    key = in.Left(t);
    value = in.Substr(t+1,in.GetSize()-t-1);
}


bool plConfigSource::ReadString(const plString & in)
{
    plString work = in.Trim();
    
    // comment
    if (work.CharAt(0) == '#')
        return true;
    
    // comment
    if (work.CharAt(0) == ';')
        return true;
    
    // section
    if (work.CharAt(0) == '[')
    {
        int close = work.Find("]");
        if (close < 0)
            return false;
        fCurrSection = work.Substr(1, close-1);
        fEffectiveSection = fCurrSection;
        return true;
    }

    // key=value
    plString key, value;
    SplitAt(key, value, '=', work);

    // dot notation makes section change for this key=value only.
    int t = key.Find('.');
    if (t>0 && t<key.GetSize()-1)
    {
        fEffectiveSection = key.Left(t);
        key = key.Substr(t+1);
    }

    bool ret=ReadPair(key, value);
    fEffectiveSection = fCurrSection;

    if(ret && key.Compare("LoadIni") == 0)
    {
        ret = ReadSubSource( value.c_str() );
    }
    
    return ret;
}

bool plConfigSource::ReadPair(plString & key, plString & value)
{
    hsAssert(fConfigInfo, "plConfigSource::ProcessPair: fConfigInfo not set.");

    key = key.Trim();
    value = value.Trim().Trim("\"'");
    
    if (key.IsEmpty())
        return true;
    
    return fConfigInfo->AddValue(fEffectiveSection, key, value, fAddMode);
}


bool plConfigSource::ReadList(char ** l)
{
    while(*l != NULL)
    {
        ReadString(*l);
        l++;
    }
    return true;
}


bool plConfigSource::ReadInto(plConfigInfo & configInfo, KAddValueMode mode)
{
    fConfigInfo = &configInfo;
    fAddMode = mode;
    return true;
}


bool plConfigSource::WriteOutOf(plConfigInfo & configInfo)
{
    fConfigInfo = &configInfo;
    return true;
}


/////////////////////////////////////////////////

plCmdLineConfigSource::plCmdLineConfigSource(int argc, char ** argv, const char * mySection)
:   fArgc(argc)
,   fArgv(argv)
,   fMySection(mySection)
{}


bool plCmdLineConfigSource::ReadInto(plConfigInfo & configInfo, KAddValueMode mode)
{
    int argc        = fArgc;
    char ** argv    = fArgv;
    
    if (!plConfigSource::ReadInto(configInfo, mode))
        return false;
    
    fCurrSection = fMySection;
    fEffectiveSection = fCurrSection;
    
    if(argc < 1)
        return true;
    
    fConfigInfo->AddValue(fEffectiveSection, "ARGV0", *argv, fAddMode);
    argc--;
    argv++;
    
    while(argc > 0)
    {
        if(ReadString(*argv) != true)
        {
            // TODO: log error here
            return false;
        }
        argv++;
        argc--;
    }

    return true;
}

/////////////////////////////////////////////////

plEnvConfigSource::plEnvConfigSource(char ** envp, const char * mySection)
:   fEnvp(envp)
,   fMySection(mySection)
{}


bool plEnvConfigSource::ReadInto(plConfigInfo & configInfo, KAddValueMode mode)
{
    if (!plConfigSource::ReadInto(configInfo, mode))
        return false;
    
    if (fEnvp != NULL)
    {
        fCurrSection = fMySection;
        fEffectiveSection = fCurrSection;
        return ReadList(fEnvp);
    }
    
    return true;
}


/////////////////////////////////////////////////

plIniConfigSource::plIniConfigSource(const char * iniFileName)
:   fFileName(iniFileName)
{}


bool plIniConfigSource::ReadInto(plConfigInfo & configInfo, KAddValueMode mode)
{
    if (!plConfigSource::ReadInto(configInfo, mode))
        return false;
    
    fCurrSection = plConfigInfo::GlobalSection();
    fEffectiveSection = fCurrSection;
    
    if(fFileName.GetSize() < 2)
        return false;
    
    
    std::ifstream    file;
    file.open(fFileName.c_str());
    
    if(!file.is_open())
    {
        // TODO log error here
        return false;
    }
    
    char buf[4096];
    
    while (!file.eof())
    {
        file.getline(buf, 4096);
        
        if(!ReadString(buf))
        {
            // TODO log warning here
        }
    }
    file.close();
    
    return true;
}

bool plIniConfigSource::WriteOutOf(plConfigInfo & configInfo)
{
    if (!plConfigSource::WriteOutOf(configInfo))
        return false;

    std::ofstream    file;
    file.open(fFileName.c_str());
    
    if(!file.is_open())
    {
        // TODO log error here
        return false;
    }

    file
        << "# This is an auto-generated file." << std::endl
        << std::endl
        ;

    plConfigInfo::Sections::const_iterator  si, se;
    plConfigInfo::Keys::const_iterator      ki, ke;
    plConfigInfo::Values::const_iterator    vi, ve;

    fConfigInfo->GetSectionIterators(si,se);
    for (; si!=se; ++si)
    {
        file << std::endl << "[" << si->first.c_str() << "]"<< std::endl;
        if (fConfigInfo->GetKeyIterators(si->first, ki, ke))
            for (; ki!=ke; ++ki)
            {
                if (fConfigInfo->GetValueIterators(si->first, ki->first, vi, ve))
                    for (; vi!=ve; ++vi)
                    {
                        file << ki->first.c_str() << "=" << vi->c_str() << std::endl;
                    }
            }
    }

    file.close();

    return true;
}


/////////////////////////////////////////////////

plIniStreamConfigSource::plIniStreamConfigSource(hsStream * stream)
:   fStream(stream)
{}


bool plIniStreamConfigSource::ReadInto(plConfigInfo & configInfo, KAddValueMode mode)
{
    if (!plConfigSource::ReadInto(configInfo, mode))
        return false;
    
    fCurrSection = "global";
    fEffectiveSection = fCurrSection;

    if ( !fStream )
        return false;
    
    char buf[4096];
    
    while (!fStream->AtEnd())
    {
        fStream->ReadLn( buf, sizeof(buf) );
        
        if(!ReadString(buf))
        {
            // TODO log warning here
        }
    }
    
    return true;
}

bool plIniStreamConfigSource::WriteOutOf(plConfigInfo & configInfo)
{
    if (!plConfigSource::WriteOutOf(configInfo))
        return false;

    if ( !fStream )
        return false;

    std::stringstream ss;

    plConfigInfo::Sections::const_iterator  si, se;
    plConfigInfo::Keys::const_iterator      ki, ke;
    plConfigInfo::Values::const_iterator    vi, ve;

    fConfigInfo->GetSectionIterators(si,se);
    for (; si!=se; ++si)
    {
        ss << std::endl << "[" << si->first.c_str() << "]"<< std::endl;
        if (fConfigInfo->GetKeyIterators(si->first, ki, ke))
            for (; ki!=ke; ++ki)
            {
                if (fConfigInfo->GetValueIterators(si->first, ki->first, vi, ve))
                    for (; vi!=ve; ++vi)
                    {
                        ss << ki->first.c_str() << "=" << vi->c_str() << std::endl;
                    }
            }
    }

    fStream->WriteString( ss.str().c_str() );

    return true;
}


/////////////////////////////////////////////////

plIniSectionConfigSource::plIniSectionConfigSource(const char * iniFileName, std::vector<plString> & sections)
:   plIniConfigSource(iniFileName)
{
    for (int i=0; i<sections.size(); i++)
        fSections.push_back(sections[i]);
}


bool plIniSectionConfigSource::ReadPair(plString & key, plString & value)
{
    hsAssert(fConfigInfo, "plConfigSource::ProcessPair: fConfigInfo not set.");

    // the current section must be in list of sections.
    Sections::iterator ii = std::find_if(fSections.begin(), fSections.end(),
        [this](const plString &v) { return v.CompareI(fCurrSection) == 0; }
    );

    if (ii==fSections.end())
        return true;

    key = key.Trim();
    value = value.Trim().Trim("\"'");
    
    if (key.IsEmpty())
        return true;

    if (key.CompareI("section") == 0)
        fSections.push_back(value);
    
    return fConfigInfo->AddValue(fEffectiveSection, key, value, fAddMode);
}


bool plIniSectionConfigSource::ReadSubSource( const char * name )
{
    plIniSectionConfigSource src(name, fSections);
    return fConfigInfo->ReadFrom(&src);
}

/////////////////////////////////////////////////

plIniNoSectionsConfigSource::plIniNoSectionsConfigSource(const char * filename)
:   fFileName(filename)
{
    fEffectiveSection = fCurrSection = "";
}

bool plIniNoSectionsConfigSource::ReadString(const plString & in)
{
    plString work = in.Trim();
    
    // ignore comments
    if (work.CharAt(0)=='#' || work.CharAt(0)==';')
        return true;
    
    // ignore sections
    if (work.CharAt(0) == '[')
        return true;

    // parse key value
    plString key, value;
    SplitAt(key, value, '=', work);

    return ReadPair(key, value);
}

bool plIniNoSectionsConfigSource::ReadInto(plConfigInfo & configInfo, KAddValueMode mode)
{
    if (!plConfigSource::ReadInto(configInfo, mode))
        return false;
    
    if (fFileName.GetSize() < 2)
        return false;
    
    std::ifstream    file;
    file.open(fFileName.c_str());
    
    if(!file.is_open())
    {
        // TODO log error here
        return false;
    }
    
    char buf[4096];
    
    while (!file.eof())
    {
        file.getline(buf, 4096);
        
        if(!ReadString(buf))
        {
            // TODO log warning here
        }
    }
    file.close();
    
    return true;
}


bool plIniNoSectionsConfigSource::WriteOutOf(plConfigInfo & configInfo)
{
    if (!plConfigSource::WriteOutOf(configInfo))
        return false;

    std::ofstream    file;
    file.open(fFileName.c_str());
    
    if(!file.is_open())
    {
        // TODO log error here
        return false;
    }

    file
        << "# This is an auto-generated file." << std::endl
        << std::endl
        ;

    plConfigInfo::Sections::const_iterator  si, se;
    plConfigInfo::Keys::const_iterator      ki, ke;
    plConfigInfo::Values::const_iterator    vi, ve;

    fConfigInfo->GetSectionIterators(si,se);
    for (; si!=se; ++si)
    {
        if (fConfigInfo->GetKeyIterators(si->first, ki, ke))
            for (; ki!=ke; ++ki)
            {
                if (fConfigInfo->GetValueIterators(si->first, ki->first, vi, ve))
                    for (; vi!=ve; ++vi)
                    {
                        file << ki->first.c_str() << "=" << vi->c_str() << std::endl;
                    }
            }
    }

    file.close();

    return true;
}


/////////////////////////////////////////////////

bool plDebugConfigSource::WriteOutOf(plConfigInfo & configInfo)
{
    if (!plConfigSource::WriteOutOf(configInfo))
        return false;

    plConfigInfo::Sections::const_iterator  si, se;
    plConfigInfo::Keys::const_iterator      ki, ke;
    plConfigInfo::Values::const_iterator    vi, ve;

    char buf[1024];
    fConfigInfo->GetSectionIterators(si,se);
    for (; si!=se; ++si)
    {
        sprintf(buf,"\n[%s]\n",si->first.c_str());
        hsStatusMessage(buf);
        if (fConfigInfo->GetKeyIterators(si->first, ki, ke))
            for (; ki!=ke; ++ki)
            {
                if (fConfigInfo->GetValueIterators(si->first, ki->first, vi, ve))
                    for (; vi!=ve; ++vi)
                    {
                        sprintf(buf,"%s=%s\n",ki->first.c_str(),vi->c_str());
                        hsStatusMessage(buf);
                    }
            }
    }

    return true;
}

////////////////////////////////////////////////////////////////////

void plConfigValueBase::ConfigRead(plConfigInfo * opts)
{
    if (fReadEvaluate())
    {
        plString value;
        bool found;
        value = opts->GetValue(GetConfigGroup(),GetConfigName(),"",&found);
        if (found)
            SetValue(fReadModify(value));
    }
}

void plConfigValueBase::ConfigWrite(plConfigInfo * opts)
{
    if (fWriteEvaluate())
    {
        opts->AddValue(GetConfigGroup(),GetConfigName(),fWriteModify(GetValue()),kAlwaysAdd);
    }
}

void plConfigValueBase::SetValue(const plString & value)
{
    ISetValue(fSetModify(value));
}

plString plConfigValueBase::GetValue() const
{
    return fGetModify(IGetValue());
}

void plConfigValueBase::SetReadEvaluate(plClass * targetObj, TEvaluate evalFunc)
{
    fReadEvaluate = plEvaluate(targetObj,evalFunc);
}

void plConfigValueBase::SetWriteEvaluate(plClass * targetObj, TEvaluate evalFunc)
{
    fWriteEvaluate = plEvaluate(targetObj,evalFunc);
}

void plConfigValueBase::SetWriteEvaluate(plClass * targetObj, TEvaluateConst evalFunc)
{
    fWriteEvaluate = plEvaluate(targetObj,evalFunc);
}

void plConfigValueBase::SetReadModify(plClass * targetObj, TModify modifyFunc)
{
    fReadModify = plModify(targetObj,modifyFunc);
}

void plConfigValueBase::SetWriteModify(plClass * targetObj, TModify modifyFunc)
{
    fWriteModify = plModify(targetObj,modifyFunc);
}

void plConfigValueBase::SetGetModify(plClass * targetObj, TModify modifyFunc)
{
    fGetModify = plModify(targetObj,modifyFunc);
}

void plConfigValueBase::SetSetModify(plClass * targetObj, TModify modifyFunc)
{
    fSetModify = plModify(targetObj,modifyFunc);
}


////////////////////////////////////////////////////////////////////

plConfigGroup::plConfigGroup(const char * groupName)
: fGroupName(groupName)
{}

bool plConfigGroup::Read(plConfigSource * src)
{
    if (!fOpts.ReadFrom(src))
        return false;
    for (int i=0; i<fItems.size(); i++)
        fItems[i]->ConfigRead(&fOpts);
    return true;
}

bool plConfigGroup::Write(plConfigSource * src)
{
    for (int i=0; i<fItems.size(); i++)
        fItems[i]->ConfigWrite(&fOpts);
    return fOpts.WriteTo(src);
}

void plConfigGroup::AddItem(plConfigValueBase * item, const char * name)
{
    item->SetConfigGroup(fGroupName);
    if (name)
        item->SetConfigName(name);
    fItems.push_back(item);
}

////////////////////////////////////////////////////////////////////

plConfigAggregateValue::plConfigAggregateValue(
    const char * name,
    plConfigValueBase * item1,
    plConfigValueBase * item2,
    plConfigValueBase * item3,
    plConfigValueBase * item4,
    plConfigValueBase * item5,
    plConfigValueBase * item6,
    plConfigValueBase * item7)
{
    SetConfigName(name);
    AddItems(item1,item2,item3,item4,item5,item6,item7);
}

void plConfigAggregateValue::AddItems(
    plConfigValueBase * item1,
    plConfigValueBase * item2,
    plConfigValueBase * item3,
    plConfigValueBase * item4,
    plConfigValueBase * item5,
    plConfigValueBase * item6,
    plConfigValueBase * item7)
{
    fItems.clear();
    if (item1) AddItem(item1);
    if (item2) AddItem(item2);
    if (item3) AddItem(item3);
    if (item4) AddItem(item4);
    if (item5) AddItem(item5);
    if (item6) AddItem(item6);
    if (item7) AddItem(item7);
}

void plConfigAggregateValue::ISetValue(const plString & value)
{
    plString work = value.Trim();
    int p=0,i=0;
    do
    {
        p = work.Find(" ");
        fItems[i]->SetValue(work.Left(p));
        work = work.Substr(p).TrimLeft();
        i++;
    } while (i<fItems.size() && p>=0);
}

plString plConfigAggregateValue::IGetValue() const
{
    plStringStream value;
    for (int i=0; i<fItems.size(); i++)
    {
        value << fItems[i]->GetValue();
        value << ' ';
    }
    return value.GetString().Trim();
}

void plConfigAggregateValue::AddItem(plConfigValueBase * item)
{
    fItems.push_back(item);
}

////////////////////////////////////////////////////////////////////

plWWWAuthenticateConfigSource::plWWWAuthenticateConfigSource(const plString& auth)
:   fAuth(auth)
{
    fEffectiveSection = fCurrSection = "";
}

bool plWWWAuthenticateConfigSource::ReadInto(plConfigInfo & configInfo, KAddValueMode mode)
{
    if (!plConfigSource::ReadInto(configInfo, mode))
        return false;
    
    fCurrSection = "global";
    fEffectiveSection = fCurrSection;
    
    unsigned int i = 0;

    while (i < fAuth.GetSize())
    {
        bool inQuote = false;
        unsigned int begin = i,end;
        while (i < fAuth.GetSize()
            && ((fAuth.CharAt(i) != ',' && !inQuote) || inQuote))
        {
                if (fAuth.CharAt(i) == '"')
                    inQuote = ! inQuote;
                i++;
        }
        end = i;

        plString buf = fAuth.Substr(begin, end-begin);
        if (!ReadString(buf))
        {
            // TODO log warning here
        }
        i++;
    }
    
    return true;
}
