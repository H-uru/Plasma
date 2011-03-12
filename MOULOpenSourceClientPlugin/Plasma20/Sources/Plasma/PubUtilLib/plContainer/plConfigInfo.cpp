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
#include "plConfigInfo.h"

#include "hsStlUtils.h"
#include <fstream>
#include <algorithm>
#include <stdarg.h>
#include <sstream>

const std::string& plConfigInfo::GlobalSection()
{
	static std::string section("global");
	return section;
}

plConfigInfo::plConfigInfo()
{
}

plConfigInfo::plConfigInfo(const plConfigInfo & src)
:	fSections(src.fSections)
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

void plConfigInfo::RemoveSection(const std::string & section)
{
	fSections.erase(section.c_str());
}

void plConfigInfo::RemoveKey(const std::string & section, const std::string & key)
{
	Sections::iterator si = fSections.find(section.c_str());
	if (si != fSections.end())
		fSections[section.c_str()].RemoveKey(key);
}

bool plConfigInfo::HasSection(const std::string & section) const
{
	return fSections.find(section.c_str())!=fSections.end();
}

bool plConfigInfo::HasKey(const std::string & section, const std::string & key)
{
	Sections::iterator si = fSections.find(section.c_str());
	if (si == fSections.end())
		return false;
	return (si->second.HasKey(key));
}

bool plConfigInfo::HasKeyAny(const std::string & key)
{
	for (Sections::iterator si=fSections.begin(); si!=fSections.end(); ++si)
	{
		if (si->second.HasKey(key))
			return true;
	}
	return false;
}

bool plConfigInfo::HasKeyIn(const std::string & key, const char * section1, ...)
{
	const char * section = section1;
	va_list	va;
	va_start(va,section1);
	std::vector<std::string> sections;
	while (section)
	{
		sections.push_back( section );
		section = va_arg(va,const char *);
	}
	va_end(va);
	return HasKeyIn( key, sections );
}

bool plConfigInfo::HasKeyIn(const std::string & key, const std::vector<std::string> & sections )
{
	for ( int i=0; i<sections.size(); i++ )
	{
		const char * section = sections[i].c_str();
		if (HasSection(section))
		{
			if (fSections[section].HasKey(key))
				return true;
		}
	}
	return false;
}

bool plConfigInfo::KeyHasValue(const std::string & section, const std::string & key, const std::string & value)
{
	Sections::iterator si = fSections.find(section.c_str());
	if (si == fSections.end())
		return false;
	return si->second.KeyHasValue(key,value);
}

bool plConfigInfo::KeyHasValue(const std::string & section, const std::string & key, int value)
{
	Sections::iterator si = fSections.find(section.c_str());
	if (si == fSections.end())
		return false;
	return si->second.KeyHasValue(key,value);
}

bool plConfigInfo::KeyHasValue(const std::string & section, const std::string & key, double value)
{
	Sections::iterator si = fSections.find(section.c_str());
	if (si == fSections.end())
		return false;
	return si->second.KeyHasValue(key,value);
}

bool plConfigInfo::AddValue(const std::string & section, const std::string & key, const std::string & value, KAddValueMode mode)
{
	fSections[section.c_str()].AddValue(key,value,mode);
	return true;
}

bool plConfigInfo::AddValue(const std::string & section, const std::string & key, int value, KAddValueMode mode)
{
	char buf[20];
	sprintf(buf, "%d", value);
	std::string v(buf);
	return AddValue(section,key,v,mode);    
}

bool plConfigInfo::AddValue(const std::string & section, const std::string & key, double value, KAddValueMode mode)
{
	char buf[30];
	sprintf(buf, "%f", value);
	std::string v(buf);
	return AddValue(section,key,v,mode);    
}

bool plConfigInfo::AddValues(const std::string & section, const std::string & key, const std::vector<std::string> & values, KAddValueMode mode)
{
	return fSections[section.c_str()].AddValues(key,values);
}

plKeysAndValues plConfigInfo::GetSection(const std::string & section, bool & found)
{
	found = HasSection(section);
	if (found)
		return fSections[section.c_str()];
	else
		return plKeysAndValues(); // empty
}

std::vector<std::string> plConfigInfo::GetSectionNames()
{
	std::vector<std::string> results;
	for (Sections::const_iterator ii=fSections.begin(); ii!=fSections.end(); ++ii)
		results.push_back(ii->first.c_str());
	return results;
}

std::string plConfigInfo::GetValue(const std::string & section, const std::string & key, const std::string & defval, bool * outFound) const
{
	return fSections[section.c_str()].GetValue(key,defval,outFound);
}

int plConfigInfo::GetValue(const std::string & section, const std::string & key, int defval, bool * outFound) const
{
	return fSections[section.c_str()].GetValue(key,defval,outFound);
}

double plConfigInfo::GetValue(const std::string & section, const std::string & key, double defval, bool * outFound) const
{
	return fSections[section.c_str()].GetValue(key,defval,outFound);
}

std::vector<std::string> plConfigInfo::GetAllValues(const std::string & section, const std::string & key) const
{
	Sections::iterator si = fSections.find(section.c_str());
	if (si != fSections.end())
		return si->second.GetAllValues(key);
	std::vector<std::string> empty;
	return empty;
}


std::string plConfigInfo::GetValueAny(const std::string & key, const std::string & defval, bool * outFound) const
{
	if (outFound) *outFound=false;
	for (Sections::iterator si=fSections.begin(); si!=fSections.end(); ++si)
		if (si->second.HasKey(key))
			return si->second.GetValue(key,defval,outFound);
	return defval;
}

int plConfigInfo::GetValueAny(const std::string & key, int defval, bool * outFound) const
{
	if (outFound) *outFound=false;
	for (Sections::iterator si=fSections.begin(); si!=fSections.end(); ++si)
		if (si->second.HasKey(key))
			return si->second.GetValue(key,defval,outFound);
	return defval;
}

double plConfigInfo::GetValueAny(const std::string & key, double defval, bool * outFound) const
{
	if (outFound) *outFound=false;
	for (Sections::iterator si=fSections.begin(); si!=fSections.end(); ++si)
		if (si->second.HasKey(key))
			return si->second.GetValue(key,defval,outFound);
	return defval;
}

std::vector<std::string> plConfigInfo::GetAllValuesAny(const std::string & key) const
{
	for (Sections::iterator si=fSections.begin(); si!=fSections.end(); ++si)
		if (si->second.HasKey(key))
			return si->second.GetAllValues(key);
	std::vector<std::string> empty;
	return empty;
}


std::string plConfigInfo::GetValueIn(const std::string & key, const std::string & defval, bool * outFound, const char * section1, ...) const
{
	if (outFound) *outFound=false;
	const char * section = section1;
	va_list	sections;
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

std::string plConfigInfo::GetValueIn(const std::string & key, const std::string & defval, bool * outFound, const std::vector<std::string> & sections ) const
{
	if (outFound) *outFound=false;
	for ( int i=0; i<sections.size(); i++ )
	{
		if (HasSection(sections[i]))
		{
			plKeysAndValues & kv = fSections[sections[i].c_str()];
			if (kv.HasKey(key))
				return kv.GetValue(key,defval,outFound);
		}
	}
	return defval;
}

int plConfigInfo::GetValueIn(const std::string & key, int defval, bool * outFound, const char * section1, ...) const
{
	if (outFound) *outFound=false;
	const char * section = section1;
	va_list	sections;
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

int plConfigInfo::GetValueIn(const std::string & key, int defval, bool * outFound, const std::vector<std::string> & sections ) const
{
	if (outFound) *outFound=false;
	for ( int i=0; i<sections.size(); i++ )
	{
		if (HasSection(sections[i]))
		{
			plKeysAndValues & kv = fSections[sections[i].c_str()];
			if (kv.HasKey(key))
				return kv.GetValue(key,defval,outFound);
		}
	}
	return defval;
}

double plConfigInfo::GetValueIn(const std::string & key, double defval, bool * outFound, const char * section1, ...) const
{
	if (outFound) *outFound=false;
	const char * section = section1;
	va_list	sections;
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

double plConfigInfo::GetValueIn(const std::string & key, double defval, bool * outFound, const std::vector<std::string> & sections ) const
{
	if (outFound) *outFound=false;
	for ( int i=0; i<sections.size(); i++ )
	{
		if (HasSection(sections[i]))
		{
			plKeysAndValues & kv = fSections[sections[i].c_str()];
			if (kv.HasKey(key))
				return kv.GetValue(key,defval,outFound);
		}
	}
	return defval;
}

std::vector<std::string> plConfigInfo::GetAllValuesIn(const std::string & key, const char * section1, ...)
{
	const char * section = section1;
	va_list	sections;
	va_start(sections,section1);
	std::vector<std::string> result;
	while (section)
	{
		if (HasSection(section))
		{
			plKeysAndValues & kv = fSections[section];
			if (kv.HasKey(key))
			{
				std::vector<std::string> values = kv.GetAllValues(key);
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

bool plConfigInfo::GetKeyIterators(const xtl::istring & section, Keys::const_iterator & iter, Keys::const_iterator & end) const
{
	Sections::const_iterator si = fSections.find(section);
	if (si==fSections.end())
		return false;
	return fSections[section].GetKeyIterators(iter, end);
}

bool plConfigInfo::GetValueIterators(const xtl::istring & section, const xtl::istring & key, Values::const_iterator & iter, Values::const_iterator & end) const
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

void plConfigSource::SplitAt(std::string & key, std::string & value, char splitter, std::string & in)
{
	if(in.length() == 0)
		return;
	
	int t = in.find(splitter);
	if(t == std::string::npos)
	{
		key = in;
		return;
	}
	
	key.assign(in.substr(0,t));
	value.assign(in.substr(t+1,in.size()-t-1));        
}


bool plConfigSource::ReadString(const std::string & in)
{
	std::string work = in;
	xtl::trim(work);
	
	// comment
	if (work[0] == '#')
		return true;
	
	// comment
	if (work[0] == ';')
		return true;
	
	// section
	if (work[0] == '[')
	{
		int close = work.find_first_of("]");
		if(close == std::string::npos)
			return false;
		fCurrSection = work.substr(1,close-1);
		fEffectiveSection = fCurrSection;
		return true;
	}

	// key=value
	std::string  key, value;
	SplitAt(key, value, '=', work);

	// dot notation makes section change for this key=value only.
	int t = key.find('.');
	if (t>0 && t<key.size()-1)
	{
		fEffectiveSection.assign(key.substr(0,t));
		key.assign(key.substr(t+1));
	}

	bool ret=ReadPair(key, value);
	fEffectiveSection = fCurrSection;

	if(ret && strcmp("LoadIni",key.c_str()) == 0)
	{
		ret = ReadSubSource( value.c_str() );
	}
	
	return ret;
}

bool plConfigSource::ReadPair(std::string & key, std::string & value)
{
	hsAssert(fConfigInfo, "plConfigSource::ProcessPair: fConfigInfo not set.");

	xtl::trim(key);
	xtl::trim(value);
	xtl::trim(value,"\"'");
	
	if (key.size() == 0)
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
:	fArgc(argc)
,	fArgv(argv)
,	fMySection(mySection?mySection:"")
{}


bool plCmdLineConfigSource::ReadInto(plConfigInfo & configInfo, KAddValueMode mode)
{
	int argc		= fArgc;
	char ** argv	= fArgv;
	
	if (!plConfigSource::ReadInto(configInfo, mode))
		return false;
	
	fCurrSection = fMySection;
	fEffectiveSection = fCurrSection;
	
	if(argc < 1)
		return true;
	
	fConfigInfo->AddValue(fEffectiveSection.c_str(), "ARGV0", *argv, fAddMode);
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
:	fEnvp(envp)
,	fMySection(mySection?mySection:"")
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
:	fFileName(iniFileName)
{}


bool plIniConfigSource::ReadInto(plConfigInfo & configInfo, KAddValueMode mode)
{
	if (!plConfigSource::ReadInto(configInfo, mode))
		return false;
	
	fCurrSection = plConfigInfo::GlobalSection();
	fEffectiveSection = fCurrSection;
	
	if(fFileName.size() < 2)
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

	plConfigInfo::Sections::const_iterator	si, se;
	plConfigInfo::Keys::const_iterator		ki, ke;
	plConfigInfo::Values::const_iterator	vi, ve;

	fConfigInfo->GetSectionIterators(si,se);
	for (si; si!=se; ++si)
	{
		file << std::endl << "[" << si->first.c_str() << "]"<< std::endl;
		if (fConfigInfo->GetKeyIterators(si->first, ki, ke))
			for (ki; ki!=ke; ++ki)
			{
				if (fConfigInfo->GetValueIterators(si->first, ki->first, vi, ve))
					for (vi; vi!=ve; ++vi)
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
:	fStream(stream)
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

	plConfigInfo::Sections::const_iterator	si, se;
	plConfigInfo::Keys::const_iterator		ki, ke;
	plConfigInfo::Values::const_iterator	vi, ve;

	fConfigInfo->GetSectionIterators(si,se);
	for (si; si!=se; ++si)
	{
		ss << std::endl << "[" << si->first.c_str() << "]"<< std::endl;
		if (fConfigInfo->GetKeyIterators(si->first, ki, ke))
			for (ki; ki!=ke; ++ki)
			{
				if (fConfigInfo->GetValueIterators(si->first, ki->first, vi, ve))
					for (vi; vi!=ve; ++vi)
					{
						ss << ki->first.c_str() << "=" << vi->c_str() << std::endl;
					}
			}
	}

	fStream->WriteString( ss.str().c_str() );

	return true;
}


/////////////////////////////////////////////////

plIniSectionConfigSource::plIniSectionConfigSource(const char * iniFileName, std::vector<std::string> & sections)
:	plIniConfigSource(iniFileName)
{
	for (int i=0; i<sections.size(); i++)
		fSections.push_back(sections[i].c_str());
}


bool plIniSectionConfigSource::ReadPair(std::string & key, std::string & value)
{
	hsAssert(fConfigInfo, "plConfigSource::ProcessPair: fConfigInfo not set.");

	// the current section must be in list of sections.
	std::vector<xtl::istring>::iterator ii = std::find(fSections.begin(), fSections.end(), fCurrSection.c_str());

	if (ii==fSections.end())
		return true;

	xtl::trim(key);
	xtl::trim(value);
	xtl::trim(value,"\"'");
	
	if (key.size() == 0)
		return true;

	if (key == "section")
		fSections.push_back(value.c_str());
	
	return fConfigInfo->AddValue(fEffectiveSection, key, value, fAddMode);
}


bool plIniSectionConfigSource::ReadSubSource( const char * name )
{
	std::vector<std::string> sections;
	for ( int i=0; i<fSections.size(); i++ )
		sections.push_back( fSections[i].c_str() );
	return fConfigInfo->ReadFrom(&plIniSectionConfigSource( name, sections ));
}

/////////////////////////////////////////////////

plIniNoSectionsConfigSource::plIniNoSectionsConfigSource(const char * filename)
:	fFileName(filename)
{
	fEffectiveSection = fCurrSection = "";
}

bool plIniNoSectionsConfigSource::ReadString(const std::string & in)
{
	std::string work = in;
	xtl::trim(work);
	
	// ignore comments
	if (work[0]=='#' || work[0]==';')
		return true;
	
	// ignore sections
	if (work[0] == '[')
		return true;

	// parse key value
	std::string  key, value;
	SplitAt(key, value, '=', work);

	return ReadPair(key, value);
}

bool plIniNoSectionsConfigSource::ReadInto(plConfigInfo & configInfo, KAddValueMode mode)
{
	if (!plConfigSource::ReadInto(configInfo, mode))
		return false;
	
	if(fFileName.size() < 2)
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

	plConfigInfo::Sections::const_iterator	si, se;
	plConfigInfo::Keys::const_iterator		ki, ke;
	plConfigInfo::Values::const_iterator	vi, ve;

	fConfigInfo->GetSectionIterators(si,se);
	for (si; si!=se; ++si)
	{
		if (fConfigInfo->GetKeyIterators(si->first, ki, ke))
			for (ki; ki!=ke; ++ki)
			{
				if (fConfigInfo->GetValueIterators(si->first, ki->first, vi, ve))
					for (vi; vi!=ve; ++vi)
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

	plConfigInfo::Sections::const_iterator	si, se;
	plConfigInfo::Keys::const_iterator		ki, ke;
	plConfigInfo::Values::const_iterator	vi, ve;

	char buf[1024];
	fConfigInfo->GetSectionIterators(si,se);
	for (si; si!=se; ++si)
	{
		sprintf(buf,"\n[%s]\n",si->first.c_str());
		hsStatusMessage(buf);
		if (fConfigInfo->GetKeyIterators(si->first, ki, ke))
			for (ki; ki!=ke; ++ki)
			{
				if (fConfigInfo->GetValueIterators(si->first, ki->first, vi, ve))
					for (vi; vi!=ve; ++vi)
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
		std::string value;
		bool found;
		value = opts->GetValue(GetConfigGroup(),GetConfigName(),"",&found);
		if (found)
			SetValue(fReadModify(value).c_str());
	}
}

void plConfigValueBase::ConfigWrite(plConfigInfo * opts)
{
	if (fWriteEvaluate())
	{
		opts->AddValue(GetConfigGroup(),GetConfigName(),fWriteModify(GetValue()),kAlwaysAdd);
	}
}

void plConfigValueBase::SetValue(const char * value)
{
	ISetValue(fSetModify(value).c_str());
}

std::string plConfigValueBase::GetValue() const
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
	item->SetConfigGroup(fGroupName.c_str());
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

void plConfigAggregateValue::ISetValue(const char * value)
{
	std::string work = value;
	int p=0,i=0;
	do
	{
		xtl::trim(work);
		p = work.find(" ");
		fItems[i]->SetValue(work.substr(0,p).c_str());
		work.erase(0,p);
		i++;
	} while (i<fItems.size() && p!=std::string::npos);
}

std::string plConfigAggregateValue::IGetValue() const
{
	std::string value;
	for (int i=0; i<fItems.size(); i++)
	{
		value.append(fItems[i]->GetValue());
		value.append(" ");
	}
	return xtl::trim(value);
}

void plConfigAggregateValue::AddItem(plConfigValueBase * item)
{
	fItems.push_back(item);
}

////////////////////////////////////////////////////////////////////

plWWWAuthenticateConfigSource::plWWWAuthenticateConfigSource(const std::string& auth)
:	fAuth(auth)
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

	while (i < fAuth.size())
	{
		bool inQuote = false;
		unsigned int begin = i,end;
		while (i < fAuth.size() 
			&& ((fAuth[i] != ',' && !inQuote) || inQuote))
		{
				if (fAuth[i] == '"')
					inQuote = ! inQuote;
				i++;
		}
		end = i;

		std::string buf;
		buf.assign(fAuth,begin,end-begin);
		if(!ReadString(buf.c_str()))
		{
			// TODO log warning here
		}
		i++;
	}
	
	return true;
}
