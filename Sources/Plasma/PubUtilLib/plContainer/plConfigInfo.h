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
#ifndef plConfigInfo_h_inc
#define plConfigInfo_h_inc

#include "plKeysAndValues.h"
#include "hsStlUtils.h"
#include <stdarg.h>

/////////////////////////////////////////////////

typedef std::vector<std::string>	plStringList;
typedef std::vector<std::wstring>	plWStringList;

/////////////////////////////////////////////////

class plConfigSource;
class plConfigInfo
{
public:
	typedef plKeysAndValues::Keys
		Keys;
	typedef plKeysAndValues::Values
		Values;
	typedef std::map<xtl::istring, plKeysAndValues>
		Sections;

	static const std::string& GlobalSection();
	
private:
	mutable Sections fSections;
	
public:
	plConfigInfo();
	plConfigInfo(const plConfigInfo & src);
	virtual ~plConfigInfo() {}
	plConfigInfo & operator =(const plConfigInfo & src);

	// REMOVE
	// remove all sections
	void Clear();
	// remove section
	void RemoveSection(const std::string & section);
	// remove key from section
	void RemoveKey(const std::string & section, const std::string & key);
	// QUERY
	// does this section exist?
	bool HasSection(const std::string & section) const;
	// does the given section contain this key?
	bool HasKey(const std::string & section, const std::string & key);
	// does any section contain this key?
	bool HasKeyAny(const std::string & key);
	// does any of the given sections contain this key?
	bool HasKeyIn(const std::string & key, const char * section1, ... /*, nil*/);
	bool HasKeyIn(const std::string & key, const std::vector<std::string> & sections );
	// does key in section have this value?
	bool KeyHasValue(const std::string & section, const std::string & key, const std::string & value);
	bool KeyHasValue(const std::string & section, const std::string & key, int value);
	bool KeyHasValue(const std::string & section, const std::string & key, double value);
	// ADD
	// add key=value to the section
	bool AddValue(const std::string & section, const std::string & key, const std::string & value, KAddValueMode mode=kAlwaysAdd);
	bool AddValue(const std::string & section, const std::string & key, int value, KAddValueMode mode=kAlwaysAdd);
	bool AddValue(const std::string & section, const std::string & key, double value, KAddValueMode mode=kAlwaysAdd);
	bool AddValues(const std::string & section, const std::string & key, const std::vector<std::string> & values, KAddValueMode mode=kAlwaysAdd);
	// GET
	plKeysAndValues GetSection(const std::string & section, bool & found);
	std::vector<std::string> GetSectionNames();
	// get value for key from given section
	std::string GetValue(const std::string & section, const std::string & key, const std::string & defval="", bool * outFound=nil) const;
	int GetValue(const std::string & section, const std::string & key, int defval, bool * outFound=nil) const;
	double GetValue(const std::string & section, const std::string & key, double defval, bool * outFound=nil) const;
	std::vector<std::string> GetAllValues(const std::string & section, const std::string & key) const;
	// get value for key from any section
	std::string GetValueAny(const std::string & key, const std::string & defval="", bool * outFound=nil) const;
	int GetValueAny(const std::string & key, int defval, bool * outFound=nil) const;
	double GetValueAny(const std::string & key, double defval, bool * outFound=nil) const;
	std::vector<std::string> GetAllValuesAny(const std::string & key) const;
	// get value for key from one of the given sections
	std::string GetValueIn(const std::string & key, const std::string & defval, bool * outFound, const char * section1, ... /*, nil*/) const;
	std::string GetValueIn(const std::string & key, const std::string & defval, bool * outFound, const std::vector<std::string> & sections ) const;
	int GetValueIn(const std::string & key, int defval, bool * outFound, const char * section1, ... /*, nil*/) const;
	int GetValueIn(const std::string & key, int defval, bool * outFound, const std::vector<std::string> & sections ) const;
	double GetValueIn(const std::string & key, double defval, bool * outFound, const char * section1, ... /*, nil*/) const;
	double GetValueIn(const std::string & key, double defval, bool * outFound, const std::vector<std::string> & sections ) const;
	std::vector<std::string> GetAllValuesIn(const std::string & key, const char * section1, ... /*, nil*/);
	// ITERATORS
	bool GetSectionIterators(Sections::const_iterator & iter, Sections::const_iterator & end) const;
	bool GetKeyIterators(const xtl::istring & section, Keys::const_iterator & iter, Keys::const_iterator & end) const;
	bool GetValueIterators(const xtl::istring & section, const xtl::istring & key, Values::const_iterator & iter, Values::const_iterator & end) const;
	// CONFIG SOURCE
	virtual bool ReadFrom(plConfigSource * src, KAddValueMode mode=kAlwaysAdd);
	virtual bool WriteTo(plConfigSource * src);
};

class plConfigInfoLogging
{
private:
	plConfigInfo fConfigInfo;

	plConfigInfo fLog;
public:
	plConfigInfoLogging(); 
	~plConfigInfoLogging(); 

	plConfigInfo* GetConfigInfo() { return &fConfigInfo; }
	plConfigInfo* GetConfigInfoLog() { return &fLog; }

	bool GetValue(std::string& retval, const std::string & section, const std::string & key, const std::string & desc, const std::string& defval = "");
	bool GetValue(int& retval, const std::string & section, const std::string & key, const std::string & desc, int defval);
	bool GetValue(bool& retval, const std::string & section, const std::string & key, const std::string & desc, bool defval);
	bool GetValue(float& retval, const std::string & section, const std::string & key, const std::string & desc, float defval);
	bool GetValue(double& retval, const std::string & section, const std::string & key, const std::string & desc, double defval);
	bool GetAllValues(std::vector<std::string>& values, const std::string & section, const std::string & key, const std::string & desc);

#if USE_MULT_SECTIONS
	// get value for key from any section
	bool GetValueAny(std::string& retval, const std::string & key, const std::string & desc, const std::string & defval);
	bool GetValueAny(int &retval, const std::string & key, const std::string & desc, int defval);
	bool GetValueAny(bool& retval, const std::string & key, const std::string & desc, bool defval);
	bool GetValueAny(float& retval, const std::string & key, const std::string & desc, float defval);
	bool GetValueAny(double& retval, const std::string & key, const std::string & desc, double defval);
	bool GetAllValuesAny(std::vector<std::string>& values, const std::string & key, const std::string & desc);

	// get value for key from one of the given sections
	bool GetValueIn(std::string& retval, const std::string & key,  const std::string & desc, const std::string & defval, const char * section1, ... /*, nil*/);
	bool GetValueIn(std::string& retval, const std::string & key,  const std::string & desc, const std::string & defval, std::vector<std::string> & sections );
	bool GetValueIn(int& retval, const std::string & key,  const std::string & desc, int defval, const char * section1, ... /*, nil*/);
	bool GetValueIn(int& retval, const std::string & key,  const std::string & desc, int defval, std::vector<std::string> & sections );
	bool GetValueIn(bool& retval, const std::string & key,  const std::string & desc, bool defval, const char * section1, ... /*, nil*/);
	bool GetValueIn(bool& retval, const std::string & key,  const std::string & desc, bool defval, std::vector<std::string> & sections );
	bool GetValueIn(float& retval, const std::string & key,  const std::string & desc, double defval, const char * section1, ... /*, nil*/);
	bool GetValueIn(float& retval, const std::string & key,  const std::string & desc, double defval, std::vector<std::string> & sections );
	bool GetValueIn(double& retval, const std::string & key,  const std::string & desc, double defval, const char * section1, ... /*, nil*/);
	bool GetValueIn(double& retval, const std::string & key,  const std::string & desc, double defval, std::vector<std::string> & sections );
#endif
};

/////////////////////////////////////////////////

class plConfigSource
{
protected:
	std::string		fCurrSection;		// used in parsing
	std::string		fEffectiveSection;	// used in parsing
	KAddValueMode	fAddMode;			// used in parsing
	plConfigInfo *	fConfigInfo;
	
	void SplitAt(std::string & key, std::string & value, char splitter, std::string & in);
	virtual bool ReadString(const std::string & in);
	virtual bool ReadPair(std::string & key, std::string & value);
	virtual bool ReadList(char ** list);
	virtual bool ReadSubSource( const char * name ) { return true; }
	
protected:
	virtual bool ReadInto(plConfigInfo & configInfo, KAddValueMode mode=kAlwaysAdd);
	virtual bool WriteOutOf(plConfigInfo & configInfo);

public:
	plConfigSource() {}
	virtual ~plConfigSource() {}

	friend class plConfigInfo;
};


/////////////////////////////////////////////////

class plCmdLineConfigSource : public plConfigSource
{
	int fArgc;
	char ** fArgv;
	std::string fMySection;
protected:
	bool ReadInto(plConfigInfo & configInfo, KAddValueMode mode=kAlwaysAdd);
public:
	plCmdLineConfigSource(int argc, char ** argv, const char * mySection="CmdLine");
};


/////////////////////////////////////////////////

class plEnvConfigSource : public plConfigSource
{
	char ** fEnvp;
	std::string fMySection;
protected:
	bool ReadInto(plConfigInfo & configInfo, KAddValueMode mode=kAlwaysAdd);
public:
	plEnvConfigSource(char ** envp, const char * mySection="Environment");
};


/////////////////////////////////////////////////

class plIniConfigSource : public plConfigSource
{
protected:
	std::string fFileName;
	bool ReadInto(plConfigInfo & configInfo, KAddValueMode mode=kAlwaysAdd);
	bool WriteOutOf(plConfigInfo & configInfo);
public:
	plIniConfigSource(const char * iniFileName);
};

/////////////////////////////////////////////////
// just like above, but works with hsStream-derived classes

class hsStream;
class plIniStreamConfigSource : public plConfigSource
{
protected:
	hsStream * fStream;
	bool ReadInto(plConfigInfo & configInfo, KAddValueMode mode=kAlwaysAdd);
	bool WriteOutOf(plConfigInfo & configInfo);
public:
	plIniStreamConfigSource(hsStream * stream);
};

/////////////////////////////////////////////////

// an ini file reader/writer that ignores section headers and puts all
// data in an unnamed section, or more accurately, in a section named "".
class plIniNoSectionsConfigSource : public plConfigSource
{
	std::string fFileName;
protected:
	bool ReadString(const std::string & in);
	bool ReadInto(plConfigInfo & configInfo, KAddValueMode mode=kAlwaysAdd);
	bool WriteOutOf(plConfigInfo & configInfo);
public:
	plIniNoSectionsConfigSource(const char * iniFileName);
};

/////////////////////////////////////////////////

// an ini file reader that only reads specified sections
class plIniSectionConfigSource : public plIniConfigSource
{
	typedef std::vector<xtl::istring>
		Sections;
protected:
	Sections	fSections;
	bool ReadPair(std::string & key, std::string & value);
	bool ReadSubSource( const char * name );
public:
	plIniSectionConfigSource(const char * iniFileName, std::vector<std::string> & sections);
};

/////////////////////////////////////////////////

// TODO: This would be a cool thing. not needed right now though.
class plDatabaseConfigSource : public plConfigSource
{
protected:
	bool ReadInto(plConfigInfo & configInfo, KAddValueMode mode=kAlwaysAdd);
	bool WriteOutOf(plConfigInfo & configInfo);
public:
	plDatabaseConfigSource(const char * connectString);
};


/////////////////////////////////////////////////

class plDebugConfigSource : public plConfigSource
{
protected:
	std::string fFileName;
	bool WriteOutOf(plConfigInfo & configInfo);
public:
	plDebugConfigSource(){}
};

/////////////////////////////////////////////////

class plWWWAuthenticateConfigSource : public plConfigSource
{
	const std::string& fAuth;
protected:
	bool ReadInto(plConfigInfo & configInfo, KAddValueMode mode=kAlwaysAdd);
public:
	plWWWAuthenticateConfigSource(const std::string& auth);
};

////////////////////////////////////////////////////////////////////

// NOTE: plClass _must_ appear first in a multiple-inheritance list.
class plClass
{
public:
	virtual void Unused(){}
};

////////////////////////////////////////////////////////////////////

typedef bool (plClass::*TEvaluate)();
typedef bool (plClass::*TEvaluateConst)() const;
struct plEvaluate
{
	plClass * fTarget;
	bool (plClass::*fEvaluate)();
	bool (plClass::*fEvaluateConst)() const;
	plEvaluate( plClass * target=nil, TEvaluate evaluate=nil )
	: fTarget(target)
	, fEvaluate(evaluate)
	, fEvaluateConst(nil)
	{}
	plEvaluate( plClass * target, TEvaluateConst evaluate )
	: fTarget(target)
	, fEvaluateConst(evaluate)
	, fEvaluate(nil)
	{}
	bool operator()() 
	{ 
		if (fEvaluate)
			return (fTarget)?(fTarget->*fEvaluate)():true;
		else
		if (fEvaluateConst)
			return (fTarget)?(fTarget->*fEvaluateConst)():true;
		else
			return true;
	}
};

////////////////////////////////////////////////////////////////////

typedef std::string (plClass::*TModify)(const std::string & value);

struct plModify
{
	plClass * fTarget;
	std::string (plClass::*fModify)(const std::string & value);
	plModify( plClass * target=nil, TModify modify=nil )
	: fTarget(target)
	, fModify(modify)
	{}
	std::string operator()(const std::string & value) { return (fTarget)?(fTarget->*fModify)(value):value;}
	std::string operator()(const std::string & value) const { return (fTarget)?(fTarget->*fModify)(value):value;}
};

////////////////////////////////////////////////////////////////////

class plConfigValueBase
{
public:
	std::string fConfigName;
	std::string fConfigGroup;
	plEvaluate  fReadEvaluate;		// returns true if we want to read this value from options
	plEvaluate  fWriteEvaluate;		// returns true if we want to write this value to options
	plModify	fReadModify;		// may modify the value being read from options
	plModify	fWriteModify;		// may modify the value being written to options
	plModify	fGetModify;			// may modify the value when being assigned
	plModify	fSetModify;			// may modify the value when being accessed
	plConfigValueBase( const char * configName="", const char * configGroup="" )
	: fConfigName(configName)
	, fConfigGroup(configGroup)
	{}
	void SetConfigName(const char * name) { fConfigName=(name)?name:"";}
	std::string GetConfigName() const { return fConfigName;}
	void SetConfigGroup(const char * group) { fConfigGroup=group;}
	std::string GetConfigGroup() const { return fConfigGroup;}
	bool HasConfigName() { return fConfigName.length()>0;}
	bool HasConfigGroup() { return fConfigGroup.length()>0;}
	virtual void ConfigRead(plConfigInfo * opts);
	virtual void ConfigWrite(plConfigInfo * opts);
	void SetValue(const char * value);
	std::string GetValue() const;
	virtual void ISetValue(const char * value) = 0;
	virtual std::string IGetValue() const = 0;

	void SetReadEvaluate(plClass * targetObj, TEvaluate evalFunc);
	void SetWriteEvaluate(plClass * targetObj, TEvaluate evalFunc);
	void SetWriteEvaluate(plClass * targetObj, TEvaluateConst evalFunc);
	void SetReadModify(plClass * targetObj, TModify modifyFunc);
	void SetWriteModify(plClass * targetObj, TModify modifyFunc);
	void SetGetModify(plClass * targetObj, TModify modifyFunc);
	void SetSetModify(plClass * targetObj, TModify modifyFunc);
};

////////////////////////////////////////////////////////////////////

class plConfigValue : public plConfigValueBase
{
public:
	plConfigValue( const char * configName="", const char * configGroup="" )
	: plConfigValueBase(configName, configGroup)
	{}
	std::string fConfigValue;
	void ISetValue(const char * value) { fConfigValue=value;}
	std::string IGetValue() const { return fConfigValue;}
};

////////////////////////////////////////////////////////////////////

class plConfigAggregateValue : public plConfigValueBase
{
public:
	std::vector<plConfigValueBase*>	fItems;
	plConfigAggregateValue(
		const char * name=nil,
		// A vararg here would not work because of a
		// multiple inheritance issue. Classes that are
		// plConfigValueBase are likely to be derived from
		// plClass also. Since plClass must be first in
		// the inheritance list, the vararg would not
		// point to the plConfigValueBase vtable that we
		// need to access.
		plConfigValueBase * item1=nil,
		plConfigValueBase * item2=nil,
		plConfigValueBase * item3=nil,
		plConfigValueBase * item4=nil,
		plConfigValueBase * item5=nil,
		plConfigValueBase * item6=nil,
		plConfigValueBase * item7=nil);
	void ISetValue(const char * value);
	std::string IGetValue() const;
	void AddItem(plConfigValueBase * item);
	void AddItems(
		plConfigValueBase * item1=nil,
		plConfigValueBase * item2=nil,
		plConfigValueBase * item3=nil,
		plConfigValueBase * item4=nil,
		plConfigValueBase * item5=nil,
		plConfigValueBase * item6=nil,
		plConfigValueBase * item7=nil);
};

////////////////////////////////////////////////////////////////////

class plConfigValueProxy : public plConfigValueBase
{
	plConfigValueBase * fConfigurable;
public:
	plConfigValueProxy(plConfigValueBase * item=nil)
	: fConfigurable(item)
	{}
	void Set(plConfigValueBase * item) { fConfigurable=item;}
	void ISetValue(const char * value) { fConfigurable->ISetValue(value);}
	std::string IGetValue() const { return fConfigurable->IGetValue();}
};

////////////////////////////////////////////////////////////////////

class plConfigGroup
{
public:
	plConfigInfo fOpts;
	std::string fGroupName;
	std::vector<plConfigValueBase*> fItems;
	plConfigGroup(const char * groupName="");
	bool Read(plConfigSource * src);
	bool Write(plConfigSource * src);
	void AddItem(plConfigValueBase * item, const char * name=nil);
};

////////////////////////////////////////////////////////////////////

#endif	// plConfigInfo_h_inc
