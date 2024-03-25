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
#ifndef plConfigInfo_h_inc
#define plConfigInfo_h_inc

#include "plKeysAndValues.h"

#include <string_theory/format>
#include <vector>

typedef std::vector<ST::string>     plStringList;

/////////////////////////////////////////////////

class plConfigSource;
class plConfigInfo
{
public:
    typedef plKeysAndValues::Keys
        Keys;
    typedef plKeysAndValues::Values
        Values;
    typedef std::map<ST::string, plKeysAndValues, ST::less_i>
        Sections;

    static const ST::string& GlobalSection();
    
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
    void RemoveSection(const ST::string & section);
    // remove key from section
    void RemoveKey(const ST::string & section, const ST::string & key);
    // QUERY
    // does this section exist?
    bool HasSection(const ST::string & section) const;
    // does the given section contain this key?
    bool HasKey(const ST::string & section, const ST::string & key);
    // does any section contain this key?
    bool HasKeyAny(const ST::string & key);
    // does any of the given sections contain this key?
    bool HasKeyIn(const ST::string & key, const char * section1, ... /*, nullptr*/);
    bool HasKeyIn(const ST::string & key, const std::vector<ST::string> & sections );
    // does key in section have this value?
    bool KeyHasValue(const ST::string & section, const ST::string & key, const ST::string & value);
    bool KeyHasValue(const ST::string & section, const ST::string & key, int value);
    bool KeyHasValue(const ST::string & section, const ST::string & key, double value);
    // ADD
    // add key=value to the section
    bool AddValue(const ST::string & section, const ST::string & key, const ST::string & value, KAddValueMode mode=kAlwaysAdd);
    bool AddValue(const ST::string & section, const ST::string & key, int value, KAddValueMode mode=kAlwaysAdd);
    bool AddValue(const ST::string & section, const ST::string & key, double value, KAddValueMode mode=kAlwaysAdd);
    bool AddValues(const ST::string & section, const ST::string & key, const std::vector<ST::string> & values, KAddValueMode mode=kAlwaysAdd);
    // GET
    plKeysAndValues GetSection(const ST::string & section, bool & found);
    std::vector<ST::string> GetSectionNames();
    // get value for key from given section
    ST::string GetValue(const ST::string & section, const ST::string & key, const ST::string & defval={}, bool * outFound=nullptr) const;
    int GetValue(const ST::string & section, const ST::string & key, int defval, bool * outFound=nullptr) const;
    double GetValue(const ST::string & section, const ST::string & key, double defval, bool * outFound=nullptr) const;
    std::vector<ST::string> GetAllValues(const ST::string & section, const ST::string & key) const;
    // get value for key from any section
    ST::string GetValueAny(const ST::string & key, const ST::string & defval={}, bool * outFound=nullptr) const;
    int GetValueAny(const ST::string & key, int defval, bool * outFound=nullptr) const;
    double GetValueAny(const ST::string & key, double defval, bool * outFound=nullptr) const;
    std::vector<ST::string> GetAllValuesAny(const ST::string & key) const;
    // get value for key from one of the given sections
    ST::string GetValueIn(const ST::string & key, const ST::string & defval, bool * outFound, const char * section1, ... /*, nullptr*/) const;
    ST::string GetValueIn(const ST::string & key, const ST::string & defval, bool * outFound, const std::vector<ST::string> & sections ) const;
    int GetValueIn(const ST::string & key, int defval, bool * outFound, const char * section1, ... /*, nullptr*/) const;
    int GetValueIn(const ST::string & key, int defval, bool * outFound, const std::vector<ST::string> & sections ) const;
    double GetValueIn(const ST::string & key, double defval, bool * outFound, const char * section1, ... /*, nullptr*/) const;
    double GetValueIn(const ST::string & key, double defval, bool * outFound, const std::vector<ST::string> & sections ) const;
    std::vector<ST::string> GetAllValuesIn(const ST::string & key, const char * section1, ... /*, nullptr*/);
    // ITERATORS
    bool GetSectionIterators(Sections::const_iterator & iter, Sections::const_iterator & end) const;
    bool GetKeyIterators(const ST::string & section, Keys::const_iterator & iter, Keys::const_iterator & end) const;
    bool GetValueIterators(const ST::string & section, const ST::string & key, Values::const_iterator & iter, Values::const_iterator & end) const;
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

    template <typename _Type>
    bool GetValue(_Type& retval, const ST::string & section, const ST::string & key, const ST::string & desc, const _Type& defval = _Type())
    {
        ST::string descwdef = ST::format("{}  # {}", defval, desc);
        fLog.AddValue(section, key, descwdef, kReplaceIfExists);

        bool found;
        retval = fConfigInfo.GetValue(section, key, defval, &found);
        return found;
    }

    bool GetAllValues(std::vector<ST::string>& values, const ST::string & section, const ST::string & key, const ST::string & desc);

#if USE_MULT_SECTIONS
    // get value for key from any section
    bool GetValueAny(ST::string& retval, const ST::string & key, const ST::string & desc, const ST::string & defval);
    bool GetValueAny(int &retval, const ST::string & key, const ST::string & desc, int defval);
    bool GetValueAny(bool& retval, const ST::string & key, const ST::string & desc, bool defval);
    bool GetValueAny(float& retval, const ST::string & key, const ST::string & desc, float defval);
    bool GetValueAny(double& retval, const ST::string & key, const ST::string & desc, double defval);
    bool GetAllValuesAny(std::vector<ST::string>& values, const ST::string & key, const ST::string & desc);

    // get value for key from one of the given sections
    bool GetValueIn(ST::string& retval, const ST::string & key,  const ST::string & desc, const ST::string & defval, const char * section1, ... /*, nullptr*/);
    bool GetValueIn(ST::string& retval, const ST::string & key,  const ST::string & desc, const ST::string & defval, std::vector<ST::string> & sections );
    bool GetValueIn(int& retval, const ST::string & key,  const ST::string & desc, int defval, const char * section1, ... /*, nullptr*/);
    bool GetValueIn(int& retval, const ST::string & key,  const ST::string & desc, int defval, std::vector<ST::string> & sections );
    bool GetValueIn(bool& retval, const ST::string & key,  const ST::string & desc, bool defval, const char * section1, ... /*, nullptr*/);
    bool GetValueIn(bool& retval, const ST::string & key,  const ST::string & desc, bool defval, std::vector<ST::string> & sections );
    bool GetValueIn(float& retval, const ST::string & key,  const ST::string & desc, double defval, const char * section1, ... /*, nullptr*/);
    bool GetValueIn(float& retval, const ST::string & key,  const ST::string & desc, double defval, std::vector<ST::string> & sections );
    bool GetValueIn(double& retval, const ST::string & key,  const ST::string & desc, double defval, const char * section1, ... /*, nullptr*/);
    bool GetValueIn(double& retval, const ST::string & key,  const ST::string & desc, double defval, std::vector<ST::string> & sections );
#endif
};

/////////////////////////////////////////////////

class plConfigSource
{
protected:
    ST::string      fCurrSection;       // used in parsing
    ST::string      fEffectiveSection;  // used in parsing
    KAddValueMode   fAddMode;           // used in parsing
    plConfigInfo *  fConfigInfo;
    
    void SplitAt(ST::string & key, ST::string & value, char splitter, const ST::string & in);
    virtual bool ReadString(const ST::string & in);
    virtual bool ReadPair(ST::string & key, ST::string & value);
    virtual bool ReadList(char ** list);
    virtual bool ReadSubSource( const char * name ) { return true; }
    
protected:
    virtual bool ReadInto(plConfigInfo & configInfo, KAddValueMode mode=kAlwaysAdd);
    virtual bool WriteOutOf(plConfigInfo & configInfo);

public:
    plConfigSource() : fAddMode(kAlwaysAdd), fConfigInfo() { }
    virtual ~plConfigSource() { }

    friend class plConfigInfo;
};


/////////////////////////////////////////////////

class plCmdLineConfigSource : public plConfigSource
{
    int fArgc;
    char ** fArgv;
    ST::string fMySection;
protected:
    bool ReadInto(plConfigInfo & configInfo, KAddValueMode mode=kAlwaysAdd) override;
public:
    plCmdLineConfigSource(int argc, char ** argv, const char * mySection="CmdLine");
};


/////////////////////////////////////////////////

class plEnvConfigSource : public plConfigSource
{
    char ** fEnvp;
    ST::string fMySection;
protected:
    bool ReadInto(plConfigInfo & configInfo, KAddValueMode mode=kAlwaysAdd) override;
public:
    plEnvConfigSource(char ** envp, const char * mySection="Environment");
};


/////////////////////////////////////////////////

class plIniConfigSource : public plConfigSource
{
protected:
    ST::string fFileName;
    bool ReadInto(plConfigInfo & configInfo, KAddValueMode mode=kAlwaysAdd) override;
    bool WriteOutOf(plConfigInfo & configInfo) override;
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
    bool ReadInto(plConfigInfo & configInfo, KAddValueMode mode=kAlwaysAdd) override;
    bool WriteOutOf(plConfigInfo & configInfo) override;
public:
    plIniStreamConfigSource(hsStream * stream);
};

/////////////////////////////////////////////////

// an ini file reader/writer that ignores section headers and puts all
// data in an unnamed section, or more accurately, in a section named "".
class plIniNoSectionsConfigSource : public plConfigSource
{
    ST::string fFileName;
protected:
    bool ReadString(const ST::string & in) override;
    bool ReadInto(plConfigInfo & configInfo, KAddValueMode mode=kAlwaysAdd) override;
    bool WriteOutOf(plConfigInfo & configInfo) override;
public:
    plIniNoSectionsConfigSource(const char * iniFileName);
};

/////////////////////////////////////////////////

// an ini file reader that only reads specified sections
class plIniSectionConfigSource : public plIniConfigSource
{
    typedef std::vector<ST::string>
        Sections;
protected:
    Sections    fSections;
    bool ReadPair(ST::string & key, ST::string & value) override;
    bool ReadSubSource(const char * name) override;
public:
    plIniSectionConfigSource(const char * iniFileName, std::vector<ST::string> & sections);
};

/////////////////////////////////////////////////

// TODO: This would be a cool thing. not needed right now though.
class plDatabaseConfigSource : public plConfigSource
{
protected:
    bool ReadInto(plConfigInfo & configInfo, KAddValueMode mode=kAlwaysAdd) override;
    bool WriteOutOf(plConfigInfo & configInfo) override;
public:
    plDatabaseConfigSource(const char * connectString);
};


/////////////////////////////////////////////////

class plDebugConfigSource : public plConfigSource
{
protected:
    ST::string fFileName;
    bool WriteOutOf(plConfigInfo & configInfo) override;
public:
    plDebugConfigSource(){}
};

/////////////////////////////////////////////////

class plWWWAuthenticateConfigSource : public plConfigSource
{
    const ST::string& fAuth;
protected:
    bool ReadInto(plConfigInfo & configInfo, KAddValueMode mode=kAlwaysAdd) override;
public:
    plWWWAuthenticateConfigSource(const ST::string& auth);
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
    plEvaluate(plClass * target=nullptr, TEvaluate evaluate=nullptr)
    : fTarget(target)
    , fEvaluate(evaluate)
    , fEvaluateConst()
    {}
    plEvaluate( plClass * target, TEvaluateConst evaluate )
    : fTarget(target)
    , fEvaluateConst(evaluate)
    , fEvaluate()
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

typedef ST::string (plClass::*TModify)(const ST::string & value);

struct plModify
{
    plClass * fTarget;
    ST::string (plClass::*fModify)(const ST::string & value);
    plModify(plClass * target=nullptr, TModify modify=nullptr)
    : fTarget(target)
    , fModify(modify)
    {}
    ST::string operator()(const ST::string & value) { return (fTarget)?(fTarget->*fModify)(value):value;}
    ST::string operator()(const ST::string & value) const { return (fTarget)?(fTarget->*fModify)(value):value;}
};

////////////////////////////////////////////////////////////////////

class plConfigValueBase
{
public:
    ST::string  fConfigName;
    ST::string  fConfigGroup;
    plEvaluate  fReadEvaluate;      // returns true if we want to read this value from options
    plEvaluate  fWriteEvaluate;     // returns true if we want to write this value to options
    plModify    fReadModify;        // may modify the value being read from options
    plModify    fWriteModify;       // may modify the value being written to options
    plModify    fGetModify;         // may modify the value when being assigned
    plModify    fSetModify;         // may modify the value when being accessed
    plConfigValueBase( const char * configName="", const char * configGroup="" )
    : fConfigName(configName)
    , fConfigGroup(configGroup)
    {}
    void SetConfigName(const ST::string & name) { fConfigName=name;}
    ST::string GetConfigName() const { return fConfigName;}
    void SetConfigGroup(const ST::string & group) { fConfigGroup=group;}
    ST::string GetConfigGroup() const { return fConfigGroup;}
    bool HasConfigName() { return !fConfigName.empty();}
    bool HasConfigGroup() { return !fConfigGroup.empty();}
    virtual void ConfigRead(plConfigInfo * opts);
    virtual void ConfigWrite(plConfigInfo * opts);
    void SetValue(const ST::string & value);
    ST::string GetValue() const;
    virtual void ISetValue(const ST::string & value) = 0;
    virtual ST::string IGetValue() const = 0;

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
    ST::string fConfigValue;
    void ISetValue(const ST::string & value) override { fConfigValue=value; }
    ST::string IGetValue() const override { return fConfigValue; }
};

////////////////////////////////////////////////////////////////////

class plConfigAggregateValue : public plConfigValueBase
{
public:
    std::vector<plConfigValueBase*> fItems;
    plConfigAggregateValue(
        const char * name=nullptr,
        // A vararg here would not work because of a
        // multiple inheritance issue. Classes that are
        // plConfigValueBase are likely to be derived from
        // plClass also. Since plClass must be first in
        // the inheritance list, the vararg would not
        // point to the plConfigValueBase vtable that we
        // need to access.
        plConfigValueBase * item1=nullptr,
        plConfigValueBase * item2=nullptr,
        plConfigValueBase * item3=nullptr,
        plConfigValueBase * item4=nullptr,
        plConfigValueBase * item5=nullptr,
        plConfigValueBase * item6=nullptr,
        plConfigValueBase * item7=nullptr);
    void ISetValue(const ST::string & value) override;
    ST::string IGetValue() const override;
    void AddItem(plConfigValueBase * item);
    void AddItems(
        plConfigValueBase * item1=nullptr,
        plConfigValueBase * item2=nullptr,
        plConfigValueBase * item3=nullptr,
        plConfigValueBase * item4=nullptr,
        plConfigValueBase * item5=nullptr,
        plConfigValueBase * item6=nullptr,
        plConfigValueBase * item7=nullptr);
};

////////////////////////////////////////////////////////////////////

class plConfigValueProxy : public plConfigValueBase
{
    plConfigValueBase * fConfigurable;
public:
    plConfigValueProxy(plConfigValueBase * item=nullptr)
    : fConfigurable(item)
    {}
    void Set(plConfigValueBase * item) { fConfigurable=item;}
    void ISetValue(const ST::string & value) override { fConfigurable->ISetValue(value); }
    ST::string IGetValue() const override { return fConfigurable->IGetValue(); }
};

////////////////////////////////////////////////////////////////////

class plConfigGroup
{
public:
    plConfigInfo fOpts;
    ST::string fGroupName;
    std::vector<plConfigValueBase*> fItems;
    plConfigGroup(const char * groupName="");
    bool Read(plConfigSource * src);
    bool Write(plConfigSource * src);
    void AddItem(plConfigValueBase * item, const char * name=nullptr);
};

////////////////////////////////////////////////////////////////////

#endif  // plConfigInfo_h_inc
