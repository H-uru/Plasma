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

plConfigInfoLogging::plConfigInfoLogging()
{
}

plConfigInfoLogging::~plConfigInfoLogging()
{
}

bool plConfigInfoLogging::GetValue(std::string& retval, const std::string & section, const std::string & key, const std::string & desc, const std::string& defval)
{
	std::string descwdef;
	xtl::format(descwdef,"%s  # %s",defval.c_str(),desc.c_str());
	fLog.AddValue(section,key,descwdef,kReplaceIfExists);

	bool found;
	retval = fConfigInfo.GetValue(section,key,defval,&found);
	return found;
}

bool plConfigInfoLogging::GetValue(int& retval, const std::string & section, const std::string & key, const std::string & desc, int defval)
{
	std::string descwdef;
	xtl::format(descwdef,"%d  # %s",defval,desc.c_str());
	fLog.AddValue(section,key,descwdef,kReplaceIfExists);

	bool found;
	retval = fConfigInfo.GetValue(section,key,defval,&found);
	return found;
}

bool plConfigInfoLogging::GetValue(bool& retval, const std::string & section, const std::string & key, const std::string & desc, bool defval)
{
	std::string descwdef;
	xtl::format(descwdef,"%d  # %s",defval,desc.c_str());
	fLog.AddValue(section,key,descwdef,kReplaceIfExists);

	bool found;
	retval = ( fConfigInfo.GetValue(section,key,(int)defval,&found)!=0 );
	return found;
}

bool plConfigInfoLogging::GetValue(float& retval, const std::string & section, const std::string & key, const std::string & desc, float defval)
{
	std::string descwdef;
	xtl::format(descwdef,"%f  # %s",defval,desc.c_str());
	fLog.AddValue(section,key,descwdef,kReplaceIfExists);

	bool found;
	double retvald = fConfigInfo.GetValue(section,key,defval,&found);
	retval = (float)retvald;
	return found;
}

bool plConfigInfoLogging::GetValue(double& retval, const std::string & section, const std::string & key, const std::string & desc, double defval)
{
	std::string descwdef;
	xtl::format(descwdef,"%f  # %s",defval,desc.c_str());
	fLog.AddValue(section,key,descwdef,kReplaceIfExists);

	bool found;
	retval = fConfigInfo.GetValue(section,key,defval,&found);
	return found;
}

bool plConfigInfoLogging::GetAllValues(std::vector<std::string>& values, const std::string & section, const std::string & key, const std::string & desc)
{
	std::string descwdef;
	xtl::format(descwdef,"%s  # %s","\"Multiple Entries\"",desc.c_str());
	fLog.AddValue(section,key,descwdef,kReplaceIfExists);

	values = fConfigInfo.GetAllValues(section,key);
	return values.size() != 0;
}

#if USE_MULT_SECTIONS

bool plConfigInfoLogging::GetValueAny(std::string& retval, const std::string & key, const std::string & desc, const std::string & defval)
{
	fLog.AddValue("ANY SECTION",key,desc,kReplaceIfExists);

	bool found;
	retval = fConfigInfo.GetValueAny(key,defval,&found);
	return found;
}

bool plConfigInfoLogging::GetValueAny(int &retval, const std::string & key, const std::string & desc, int defval)
{
	fLog.AddValue("ANY SECTION",key,desc,kReplaceIfExists);

	bool found;
	retval = fConfigInfo.GetValueAny(key,defval,&found);
	return found;
}

bool plConfigInfoLogging::GetValueAny(bool &retval, const std::string & key, const std::string & desc, bool defval)
{
	fLog.AddValue("ANY SECTION",key,desc,kReplaceIfExists);

	bool found;
	retval = ( fConfigInfo.GetValueAny(key,(int)defval,&found)!=0 );
	return found;
}

bool plConfigInfoLogging::GetValueAny(float& retval, const std::string & key, const std::string & desc, float defval)
{
	fLog.AddValue("ANY SECTION",key,desc,kReplaceIfExists);

	bool found;
	retval = fConfigInfo.GetValueAny(key,defval,&found);
	return found;
}

bool plConfigInfoLogging::GetValueAny(double& retval, const std::string & key, const std::string & desc, double defval)
{
	fLog.AddValue("ANY SECTION",key,desc,kReplaceIfExists);

	bool found;
	retval = fConfigInfo.GetValueAny(key,defval,&found);
	return found;
}

bool plConfigInfoLogging::GetAllValuesAny(std::vector<std::string>& values, const std::string & key, const std::string & desc)
{
	fLog.AddValue("ANY SECTION",key,desc,kReplaceIfExists);

	values = fConfigInfo.GetAllValuesAny(key);
	return values.size() != 0;
}

bool plConfigInfoLogging::GetValueIn(std::string& retval, const std::string & key,  const std::string & desc, const std::string & defval, const char * section1, ... /*, nil*/)
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

	return GetValueIn(retval,key,desc,defval,sections);
}

bool plConfigInfoLogging::GetValueIn(std::string& retval, const std::string & key,  const std::string & desc, const std::string & defval, std::vector<std::string> & sections )
{
	std::vector<std::string>::iterator si = sections.begin();
	while (si != sections.end())
	{
		fLog.AddValue(*si,key,desc,kReplaceIfExists);
		si++;
	}

	bool found;
	retval = fConfigInfo.GetValueIn(key,defval,&found,sections);
	return found;
}

bool plConfigInfoLogging::GetValueIn(int& retval, const std::string & key,  const std::string & desc, int defval, const char * section1, ... /*, nil*/)
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

	return GetValueIn(retval,key,desc,defval,sections);
}

bool plConfigInfoLogging::GetValueIn(int& retval, const std::string & key,  const std::string & desc, int defval, std::vector<std::string> & sections )
{
	std::vector<std::string>::iterator si = sections.begin();
	while (si != sections.end())
	{
		fLog.AddValue(*si,key,desc,kReplaceIfExists);
		si++;
	}

	bool found;
	retval = fConfigInfo.GetValueIn(key,defval,&found,sections);
	return found;
}

bool plConfigInfoLogging::GetValueIn(bool& retval, const std::string & key,  const std::string & desc, bool defval, const char * section1, ... /*, nil*/)
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

	return GetValueIn(retval,key,desc,defval,sections);
}

bool plConfigInfoLogging::GetValueIn(bool& retval, const std::string & key,  const std::string & desc, bool defval, std::vector<std::string> & sections )
{
	std::vector<std::string>::iterator si = sections.begin();
	while (si != sections.end())
	{
		fLog.AddValue(*si,key,desc,kReplaceIfExists);
		si++;
	}

	bool found;
	retval = ( fConfigInfo.GetValueIn(key,(int)defval,&found,sections)!=0 );
	return found;
}

bool plConfigInfoLogging::GetValueIn(float& retval, const std::string & key,  const std::string & desc, double defval, const char * section1, ... /*, nil*/)
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

	return GetValueIn(retval,key,desc,defval,sections);
}

bool plConfigInfoLogging::GetValueIn(float& retval, const std::string & key,  const std::string & desc, double defval, std::vector<std::string> & sections )
{
	std::vector<std::string>::iterator si = sections.begin();
	while (si != sections.end())
	{
		fLog.AddValue(*si,key,desc,kReplaceIfExists);
		si++;
	}

	bool found;
	retval = fConfigInfo.GetValueIn(key,defval,&found,sections);
	return found;
}

bool plConfigInfoLogging::GetValueIn(double& retval, const std::string & key,  const std::string & desc, double defval, const char * section1, ... /*, nil*/)
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

	return GetValueIn(retval,key,desc,defval,sections);
}

bool plConfigInfoLogging::GetValueIn(double& retval, const std::string & key,  const std::string & desc, double defval, std::vector<std::string> & sections )
{
	std::vector<std::string>::iterator si = sections.begin();
	while (si != sections.end())
	{
		fLog.AddValue(*si,key,desc,kReplaceIfExists);
		si++;
	}

	bool found;
	retval = fConfigInfo.GetValueIn(key,defval,&found,sections);
	return found;
}

#endif