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

/////////////////////////////////////////////////

typedef std::vector<plString>       plStringList;

/////////////////////////////////////////////////

class plConfigSource;
class plConfigInfo
{
public:
    typedef plKeysAndValues::Keys
        Keys;
    typedef plKeysAndValues::Values
        Values;
    typedef std::map<plString, plKeysAndValues, plString::less_i>
        Sections;

    static const plString& GlobalSection();
    
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
    void RemoveSection(const plString & section);
    // remove key from section
    void RemoveKey(const plString & section, const plString & key);
    // QUERY
    // does this section exist?
    bool HasSection(const plString & section) const;
    // does the given section contain this key?
    bool HasKey(const plString & section, const plString & key);
    // does any section contain this key?
    bool HasKeyAny(const plString & key);
    // does any of the given sections contain this key?
    bool HasKeyIn(const plString & key, const char * section1, ... /*, nil*/);
    bool HasKeyIn(const plString & key, const std::vector<plString> & sections );
    // does key in section have this value?
    bool KeyHasValue(const plString & section, const plString & key, const plString & value);
    bool KeyHasValue(const plString & section, const plString & key, int value);
    bool KeyHasValue(const plString & section, const plString & key, double value);
    // ADD
    // add key=value to the section
    bool AddValue(const plString & section, const plString & key, const plString & value, KAddValueMode mode=kAlwaysAdd);
    bool AddValue(const plString & section, const plString & key, int value, KAddValueMode mode=kAlwaysAdd);
    bool AddValue(const plString & section, const plString & key, double value, KAddValueMode mode=kAlwaysAdd);
    bool AddValues(const plString & section, const plString & key, const std::vector<plString> & values, KAddValueMode mode=kAlwaysAdd);
    // GET
    plKeysAndValues GetSection(const plString & section, bool & found);
    std::vector<plString> GetSectionNames();
    // get value for key from given section
    plString GetValue(const plString & section, const plString & key, const plString & defval="", bool * outFound=nil) const;
    int GetValue(const plString & section, const plString & key, int defval, bool * outFound=nil) const;
    double GetValue(const plString & section, const plString & key, double defval, bool * outFound=nil) const;
    std::vector<plString> GetAllValues(const plString & section, const plString & key) const;
    // get value for key from any section
    plString GetValueAny(const plString & key, const plString & defval="", bool * outFound=nil) const;
    int GetValueAny(const plString & key, int defval, bool * outFound=nil) const;
    double GetValueAny(const plString & key, double defval, bool * outFound=nil) const;
    std::vector<plString> GetAllValuesAny(const plString & key) const;
    // get value for key from one of the given sections
    plString GetValueIn(const plString & key, const plString & defval, bool * outFound, const char * section1, ... /*, nil*/) const;
    plString GetValueIn(const plString & key, const plString & defval, bool * outFound, const std::vector<plString> & sections ) const;
    int GetValueIn(const plString & key, int defval, bool * outFound, const char * section1, ... /*, nil*/) const;
    int GetValueIn(const plString & key, int defval, bool * outFound, const std::vector<plString> & sections ) const;
    double GetValueIn(const plString & key, double defval, bool * outFound, const char * section1, ... /*, nil*/) const;
    double GetValueIn(const plString & key, double defval, bool * outFound, const std::vector<plString> & sections ) const;
    std::vector<plString> GetAllValuesIn(const plString & key, const char * section1, ... /*, nil*/);
    // ITERATORS
    bool GetSectionIterators(Sections::const_iterator & iter, Sections::const_iterator & end) const;
    bool GetKeyIterators(const plString & section, Keys::const_iterator & iter, Keys::const_iterator & end) const;
    bool GetValueIterators(const plString & section, const plString & key, Values::const_iterator & iter, Values::const_iterator & end) const;
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

    bool GetValue(plString& retval, const plString & section, const plString & key, const plString & desc, const plString& defval = "");
    bool GetValue(int& retval, const plString & section, const plString & key, const plString & desc, int defval);
    bool GetValue(bool& retval, const plString & section, const plString & key, const plString & desc, bool defval);
    bool GetValue(float& retval, const plString & section, const plString & key, const plString & desc, float defval);
    bool GetValue(double& retval, const plString & section, const plString & key, const plString & desc, double defval);
    bool GetAllValues(std::vector<plString>& values, const plString & section, const plString & key, const plString & desc);

#if USE_MULT_SECTIONS
    // get value for key from any section
    bool GetValueAny(plString& retval, const plString & key, const plString & desc, const plString & defval);
    bool GetValueAny(int &retval, const plString & key, const plString & desc, int defval);
    bool GetValueAny(bool& retval, const plString & key, const plString & desc, bool defval);
    bool GetValueAny(float& retval, const plString & key, const plString & desc, float defval);
    bool GetValueAny(double& retval, const plString & key, const plString & desc, double defval);
    bool GetAllValuesAny(std::vector<plString>& values, const plString & key, const plString & desc);

    // get value for key from one of the given sections
    bool GetValueIn(plString& retval, const plString & key,  const plString & desc, const plString & defval, const char * section1, ... /*, nil*/);
    bool GetValueIn(plString& retval, const plString & key,  const plString & desc, const plString & defval, std::vector<plString> & sections );
    bool GetValueIn(int& retval, const plString & key,  const plString & desc, int defval, const char * section1, ... /*, nil*/);
    bool GetValueIn(int& retval, const plString & key,  const plString & desc, int defval, std::vector<plString> & sections );
    bool GetValueIn(bool& retval, const plString & key,  const plString & desc, bool defval, const char * section1, ... /*, nil*/);
    bool GetValueIn(bool& retval, const plString & key,  const plString & desc, bool defval, std::vector<plString> & sections );
    bool GetValueIn(float& retval, const plString & key,  const plString & desc, double defval, const char * section1, ... /*, nil*/);
    bool GetValueIn(float& retval, const plString & key,  const plString & desc, double defval, std::vector<plString> & sections );
    bool GetValueIn(double& retval, const plString & key,  const plString & desc, double defval, const char * section1, ... /*, nil*/);
    bool GetValueIn(double& retval, const plString & key,  const plString & desc, double defval, std::vector<plString> & sections );
#endif
};

/////////////////////////////////////////////////

class plConfigSource
{
protected:
    plString        fCurrSection;       // used in parsing
    plString        fEffectiveSection;  // used in parsing
    KAddValueMode   fAddMode;           // used in parsing
    plConfigInfo *  fConfigInfo;
    
    void SplitAt(plString & key, plString & value, char splitter, plString & in);
    virtual bool ReadString(const plString & in);
    virtual bool ReadPair(plString & key, plString & value);
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
    plString fMySection;
protected:
    bool ReadInto(plConfigInfo & configInfo, KAddValueMode mode=kAlwaysAdd);
public:
    plCmdLineConfigSource(int argc, char ** argv, const char * mySection="CmdLine");
};


/////////////////////////////////////////////////

class plEnvConfigSource : public plConfigSource
{
    char ** fEnvp;
    plString fMySection;
protected:
    bool ReadInto(plConfigInfo & configInfo, KAddValueMode mode=kAlwaysAdd);
public:
    plEnvConfigSource(char ** envp, const char * mySection="Environment");
};


/////////////////////////////////////////////////

class plIniConfigSource : public plConfigSource
{
protected:
    plString fFileName;
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
    plString fFileName;
protected:
    bool ReadString(const plString & in);
    bool ReadInto(plConfigInfo & configInfo, KAddValueMode mode=kAlwaysAdd);
    bool WriteOutOf(plConfigInfo & configInfo);
public:
    plIniNoSectionsConfigSource(const char * iniFileName);
};

/////////////////////////////////////////////////

// an ini file reader that only reads specified sections
class plIniSectionConfigSource : public plIniConfigSource
{
    typedef std::vector<plString>
        Sections;
protected:
    Sections    fSections;
    bool ReadPair(plString & key, plString & value);
    bool ReadSubSource( const char * name );
public:
    plIniSectionConfigSource(const char * iniFileName, std::vector<plString> & sections);
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
    plString fFileName;
    bool WriteOutOf(plConfigInfo & configInfo);
public:
    plDebugConfigSource(){}
};

/////////////////////////////////////////////////

class plWWWAuthenticateConfigSource : public plConfigSource
{
    const plString& fAuth;
protected:
    bool ReadInto(plConfigInfo & configInfo, KAddValueMode mode=kAlwaysAdd);
public:
    plWWWAuthenticateConfigSource(const plString& auth);
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

typedef plString (plClass::*TModify)(const plString & value);

struct plModify
{
    plClass * fTarget;
    plString (plClass::*fModify)(const plString & value);
    plModify( plClass * target=nil, TModify modify=nil )
    : fTarget(target)
    , fModify(modify)
    {}
    plString operator()(const plString & value) { return (fTarget)?(fTarget->*fModify)(value):value;}
    plString operator()(const plString & value) const { return (fTarget)?(fTarget->*fModify)(value):value;}
};

////////////////////////////////////////////////////////////////////

class plConfigValueBase
{
public:
    plString    fConfigName;
    plString    fConfigGroup;
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
    void SetConfigName(const plString & name) { fConfigName=name;}
    plString GetConfigName() const { return fConfigName;}
    void SetConfigGroup(const plString & group) { fConfigGroup=group;}
    plString GetConfigGroup() const { return fConfigGroup;}
    bool HasConfigName() { return !fConfigName.IsEmpty();}
    bool HasConfigGroup() { return !fConfigGroup.IsEmpty();}
    virtual void ConfigRead(plConfigInfo * opts);
    virtual void ConfigWrite(plConfigInfo * opts);
    void SetValue(const plString & value);
    plString GetValue() const;
    virtual void ISetValue(const plString & value) = 0;
    virtual plString IGetValue() const = 0;

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
    plString fConfigValue;
    void ISetValue(const plString & value) { fConfigValue=value;}
    plString IGetValue() const { return fConfigValue;}
};

////////////////////////////////////////////////////////////////////

class plConfigAggregateValue : public plConfigValueBase
{
public:
    std::vector<plConfigValueBase*> fItems;
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
    void ISetValue(const plString & value);
    plString IGetValue() const;
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
    void ISetValue(const plString & value) { fConfigurable->ISetValue(value);}
    plString IGetValue() const { return fConfigurable->IGetValue();}
};

////////////////////////////////////////////////////////////////////

class plConfigGroup
{
public:
    plConfigInfo fOpts;
    plString fGroupName;
    std::vector<plConfigValueBase*> fItems;
    plConfigGroup(const char * groupName="");
    bool Read(plConfigSource * src);
    bool Write(plConfigSource * src);
    void AddItem(plConfigValueBase * item, const char * name=nil);
};

////////////////////////////////////////////////////////////////////

#endif  // plConfigInfo_h_inc
