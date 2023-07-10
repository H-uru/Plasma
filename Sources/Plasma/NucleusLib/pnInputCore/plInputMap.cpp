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
// plInputMap.cpp
#include <algorithm>
#include <cctype>
#include <utility>

#include "plInputMap.h"
#include "plKeyMap.h"

#include "plResMgr/plLocalization.h"

ControlEventCode plInputMap::ConvertCharToControlCode(const ST::string& c)
{
    for (const auto& [code, desc] : fCmdConvert)
    {
        if (desc.compare_i(c) == 0)
            return code;
    }
    return (END_CONTROLS);
}

ST::string plInputMap::ConvertControlCodeToString(ControlEventCode code)
{
    auto it = fCmdConvert.find(code);
    if (it == fCmdConvert.end()) {
        return {};
    } else {
        return it->second;
    }
}

//
// plMouseMap
//


plMouseMap::~plMouseMap()
{
    for (plMouseInfo* info : fMap)
        delete info;
    fMap.clear();
}

hsSsize_t plMouseMap::AddMapping(plMouseInfo* pNfo)
{
    const auto idx = std::find(fMap.begin(), fMap.end(), pNfo);
    if (idx != fMap.end())
        return -1;

    fMap.emplace_back(pNfo);
    return hsSsize_t(fMap.size() - 1);
}


//// plKeyBinding and Smaller Classes ////////////////////////////////////////

plKeyCombo::plKeyCombo()
{
    fKey = KEY_UNMAPPED;
    fFlags = 0;
}

bool plKeyCombo::IsSatisfiedBy(const plKeyCombo &combo) const
{
    if (fKey != combo.fKey)
        return false;
    if ((fFlags & kShift) && !(combo.fFlags & kShift))
        return false;
    if ((fFlags & kCtrl) && !(combo.fFlags & kCtrl))
        return false;

    return true;
}

// Handy konstant for plKeyCombos
plKeyCombo  plKeyCombo::kUnmapped = plKeyCombo( KEY_UNMAPPED, 0 );



plKeyBinding::plKeyBinding()
{
    fCode = END_CONTROLS;
    fCodeFlags = 0;
    fKey1 = plKeyCombo::kUnmapped;
    fKey2 = plKeyCombo::kUnmapped;
}

plKeyBinding::plKeyBinding(ControlEventCode code, uint32_t codeFlags, const plKeyCombo &key1, const plKeyCombo &key2, ST::string string)
{
    fCode = code;
    fCodeFlags = codeFlags;
    fKey1 = key1;
    fKey2 = key2;
    fString = std::move(string);
}

const plKeyCombo &plKeyBinding::GetMatchingKey( plKeyDef keyDef ) const
{
    if (fKey1.fKey == keyDef)
        return fKey1;
    if (fKey2.fKey == keyDef)
        return fKey2;

    return plKeyCombo::kUnmapped;
}

void    plKeyBinding::SetKey1( const plKeyCombo &newCombo )
{
    fKey1 = newCombo;
}

void    plKeyBinding::SetKey2( const plKeyCombo &newCombo )
{
    fKey2 = newCombo;
}

void    plKeyBinding::ClearKeys()
{
    fKey1 = fKey2 = plKeyCombo::kUnmapped;
}

bool    plKeyBinding::HasUnmappedKey() const
{
    return fKey1.fKey == KEY_UNMAPPED || fKey2.fKey == KEY_UNMAPPED;
}

//////////////////////////////////////////////////////////////////////////////
//// plKeyMap Implementation /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

static const std::map<plLocalization::Language, const std::map<uint32_t, ST::string>*> keyConversions = {
    { plLocalization::kEnglish, &plKeyMap::fKeyConversionEnglish },
    { plLocalization::kFrench, &plKeyMap::fKeyConversionFrench },
    { plLocalization::kGerman, &plKeyMap::fKeyConversionGerman },
    { plLocalization::kSpanish, &plKeyMap::fKeyConversionSpanish },
    { plLocalization::kItalian, &plKeyMap::fKeyConversionItalian },
};

plKeyMap::~plKeyMap()
{
    ClearAll();
}

void    plKeyMap::ClearAll()
{
    for (plKeyBinding* binding : fBindings)
        delete binding;
    fBindings.clear();
}

//// AddCode //////////////////////////////////////////////////////////////////////////
//  Adds a given control code to the map. Once you add it, you can't change its flags. 
//  Returns false if the code is already present

bool    plKeyMap::AddCode( ControlEventCode code, uint32_t codeFlags )
{
    if (IFindBinding(code) != nullptr)
        return false;

    fBindings.emplace_back(new plKeyBinding(code, codeFlags, plKeyCombo::kUnmapped, plKeyCombo::kUnmapped));
    return true;
}

//// AddConsoleCommand ///////////////////////////////////////////////////////
//  Same but for console commands. No flags b/c console commands always use 
//  the same flags.

bool    plKeyMap::AddConsoleCommand(ST::string command)
{
    if (IFindConsoleBinding(command) != nullptr)
        return false;

    fBindings.emplace_back(new plKeyBinding(B_CONTROL_CONSOLE_COMMAND,
                                            kControlFlagDownEvent | kControlFlagNoRepeat | kControlFlagNoDeactivate,
                                            plKeyCombo::kUnmapped, plKeyCombo::kUnmapped,
                                            std::move(command)));
    return true;
}

//// IFindBinding ////////////////////////////////////////////////////////////
//  Find the binding for a given code.

plKeyBinding    *plKeyMap::IFindBinding( ControlEventCode code ) const
{
    for (plKeyBinding* binding : fBindings)
    {
        if (binding->GetCode() == code)
            return binding;
    }

    return nullptr;
}

//// IFindBindingByKey ///////////////////////////////////////////////////////
//  Find the binding for a given key.

plKeyBinding    *plKeyMap::IFindBindingByKey( const plKeyCombo &combo ) const
{
    for (plKeyBinding* binding : fBindings)
    {
        if (binding->GetKey1() == combo || binding->GetKey2() == combo)
            return binding;
    }

    return nullptr;
}

// Find ALL bindings that could be triggered by this combo. Meaning that if we have multiple
// bindings for a key with different shift/ctrl combinations, we want any that are satisfied with
// the given combo.
// We guarantee that the first binding in the result array is that one with priority.
void plKeyMap::IFindAllBindingsByKey(const plKeyCombo &combo, std::vector<plKeyBinding*> &result) const
{
    uint8_t bestScore = 0;
    for (plKeyBinding* binding : fBindings)
    {
        bool s1 = binding->GetKey1().IsSatisfiedBy(combo);
        bool s2 = binding->GetKey2().IsSatisfiedBy(combo);
        if (s1 || s2)
        {
            uint8_t myScore = 0;
            if (s1)
                myScore = binding->GetKey1().fFlags;
            if (s2 && (binding->GetKey2().fFlags > myScore))
                myScore = binding->GetKey2().fFlags;

            if (myScore >= bestScore)
                result.emplace(result.begin(), binding);
            else
                result.emplace_back(binding);
        }
    }
}

//// IFindConsoleBinding /////////////////////////////////////////////////////
//  You should be able to figure this out by now.

plKeyBinding    *plKeyMap::IFindConsoleBinding(const ST::string& command) const
{
    for (plKeyBinding* binding : fBindings)
    {
        if (binding->GetCode() == B_CONTROL_CONSOLE_COMMAND)
        {
            if (binding->GetExtendedString().compare_i(command) == 0)
                return binding;
        }
    }

    return nullptr;
}

//// IActuallyBind ///////////////////////////////////////////////////////////
//  Does the nitty gritty of binding by pref

void    plKeyMap::IActuallyBind( plKeyBinding *binding, const plKeyCombo &combo, BindPref pref )
{
    // Bind according to preference
    switch( pref )
    {
        case kNoPreference:
            // Pick a free slot, or 1st one if none
            if( binding->GetKey1() == plKeyCombo::kUnmapped )
                binding->SetKey1( combo );
            else if( binding->GetKey2() == plKeyCombo::kUnmapped )
                binding->SetKey2( combo );
            else
                binding->SetKey1( combo );
            break;

        case kNoPreference2nd:
            // Pick a free slot, or 2nd one if none
            if( binding->GetKey1() == plKeyCombo::kUnmapped )
                binding->SetKey1( combo );
            else if( binding->GetKey2() == plKeyCombo::kUnmapped )
                binding->SetKey2( combo );
            else
                binding->SetKey2( combo );
            break;

        case kFirstAlways:
            // Always bind to the first key
            binding->SetKey1( combo );
            break;

        case kSecondAlways:
            // You get the idea
            binding->SetKey2( combo );
            break;

        default:
            hsAssert( false, "Invalid bind preference in IActuallyBind()" );
    }
}

//// BindKey /////////////////////////////////////////////////////////////////
//  Adds a key binding to a given code. Returns false if the code isn't in 
//  this map or if key is already mapped.

bool    plKeyMap::BindKey( const plKeyCombo &combo, ControlEventCode code, BindPref pref /*= kNoPreference*/ )
{
    plKeyBinding* binding = nullptr;

    // Control combos are ok. Binding directly to control is not.
    if ( combo.fKey == KEY_CTRL )
        return false;

    // unless we are bindind to unmappped...
    if ( combo.fKey != KEY_UNMAPPED)
    {
        // Make sure key isn't already used
        binding = IFindBindingByKey( combo );
        if (binding != nullptr)
            return false;
    }

    // Get binding for this code
    binding = IFindBinding( code );
    if (binding == nullptr)
        return false;

    IActuallyBind( binding, combo, pref );
    return true;
}

//// BindKeyToConsoleCmd /////////////////////////////////////////////////////
//  Console command version

bool    plKeyMap::BindKeyToConsoleCmd(const plKeyCombo &combo, const ST::string& command, BindPref pref /*= kNoPreference*/)
{
    plKeyBinding* binding = nullptr;

    // Control combos are ok. Binding directly to control is not.
    if ( combo.fKey == KEY_CTRL )
        return false;

    // unless we are bindind to unmappped...
    if ( combo.fKey != KEY_UNMAPPED)
    {
        // Make sure key isn't already used
        binding = IFindBindingByKey( combo );
        if (binding != nullptr)
            return false;
    }

    // Get binding for this code
    binding = IFindConsoleBinding( command );
    if (binding == nullptr)
        return false;

    IActuallyBind( binding, combo, pref );
    return true;
}

//// FindBinding /////////////////////////////////////////////////////////////
//  Searches for the binding for a given code. Returns nullptr if not found

const plKeyBinding  *plKeyMap::FindBinding( ControlEventCode code ) const
{
    return IFindBinding( code );
}

//// FindBindingByKey ////////////////////////////////////////////////////////
//  Same thing but by key

const plKeyBinding  *plKeyMap::FindBindingByKey( const plKeyCombo &combo ) const
{
    return IFindBindingByKey( combo );
}

//  Same thing, but returns multiple matches (see IFindAllBindingsByKey)
void plKeyMap::FindAllBindingsByKey(const plKeyCombo &combo, std::vector<const plKeyBinding*> &result) const
{
    std::vector<plKeyBinding*> bindings;
    IFindAllBindingsByKey(combo, bindings);

    result.reserve(result.size() + bindings.size());
    result.insert(result.end(), bindings.begin(), bindings.end());
}


const plKeyBinding* plKeyMap::FindConsoleBinding(const ST::string& command) const
{
    return IFindConsoleBinding(command);
}

//// EnsureKeysClear /////////////////////////////////////////////////////////
//  Basically UnmapKey(), but for two keys at once and won't assert if you 
//  give it unmapped keys

void    plKeyMap::EnsureKeysClear( const plKeyCombo &key1, const plKeyCombo &key2 )
{
    if( key1 != plKeyCombo::kUnmapped )
        UnmapKey( key1 );
    if( key2 != plKeyCombo::kUnmapped )
        UnmapKey( key2 );
}

//// UnmapKey ////////////////////////////////////////////////////////////////
//  Unmaps the given key, no matter what binding it is in

void    plKeyMap::UnmapKey( const plKeyCombo &combo )
{
    if( combo == plKeyCombo::kUnmapped )
    {
        hsAssert( false, "Trying to unbind invalid key" );
        return;
    }

    plKeyBinding *binding;

    // Just in case we're in multiple bindings, even tho we are guarding against
    // that
    while ((binding = IFindBindingByKey(combo)) != nullptr)
    {
        if( binding->GetKey1() == combo )
            binding->SetKey1( plKeyCombo::kUnmapped );
        if( binding->GetKey2() == combo )
            binding->SetKey2( plKeyCombo::kUnmapped );
    }
}

//// UnmapBinding ////////////////////////////////////////////////////////////
//  Unmaps the keys for a single binding

void    plKeyMap::UnmapBinding( ControlEventCode code )
{
    plKeyBinding *binding = IFindBinding( code );
    if (binding != nullptr)
        binding->ClearKeys();
}

//// UnmapAllBindings ////////////////////////////////////////////////////////
//  Unmaps all the bindings, but leaves the code records themselves

void    plKeyMap::UnmapAllBindings()
{
    for (plKeyBinding* binding : fBindings)
        binding->ClearKeys();
}

//// EraseBinding ////////////////////////////////////////////////////////////
//  Erases the given code binding. Note: should never really be used, but 
//  provided here for completeness

void    plKeyMap::EraseBinding( ControlEventCode code )
{
    for (auto iter = fBindings.begin(); iter != fBindings.end(); ++iter)
    {
        if ((*iter)->GetCode() == code)
        {
            delete *iter;
            fBindings.erase(iter);
            return;
        }
    }
}


const std::map<uint32_t, ST::string>& plKeyMap::GetKeyConversion()
{
    auto langIter = keyConversions.find(plLocalization::GetLanguage());
    if (langIter == keyConversions.end()) {
        // default is English
        return fKeyConversionEnglish;
    } else {
        return *langIter->second;
    }
}

ST::string plKeyMap::ConvertVKeyToChar(uint32_t vk)
{
    const std::map<uint32_t, ST::string>& keyConvert = GetKeyConversion();
    auto it = keyConvert.find(vk);
    if (it == keyConvert.end()) {
        return {};
    } else {
        return it->second;
    }
}

plKeyDef plKeyMap::ConvertCharToVKey(const ST::string& c)
{
    const std::map<uint32_t, ST::string>& keyConvert = GetKeyConversion();
    for (const auto& [vKey, keyName] : keyConvert)
    {
        if (keyName.compare_i(c) == 0)
            return (plKeyDef)vKey;
    }

    // Is it just a single character?
    // This intentionally only detects and handles ASCII characters.
    // Non-ASCII characters are handled via the fKeyConversion maps.
    if (c.size() == 1 && isalnum(static_cast<unsigned char>(c[0])))
        return (plKeyDef)c.to_upper()[0];

    // if we didn't find anything yet...
    // ...then look thru all the other language mappings that we know about,
    // ...just in case they keep switching languages on us
    for (const auto& [lang, langKeyConvert] : keyConversions) {
        if (lang != plLocalization::GetLanguage()) {
            for (const auto& [vKey, keyName] : *langKeyConvert) {
                if (keyName.compare_i(c) == 0)
                    return (plKeyDef)vKey;
            }
        }
    }

    // finally, just give up... unmapped!
    return KEY_UNMAPPED;
}

ST::string plKeyMap::KeyComboToString(const plKeyCombo &combo)
{
    ST::string str = ConvertVKeyToChar(combo.fKey);
    if (str.empty()) {
        if (combo.fKey < 0x80 && isalnum(combo.fKey)) {
            char c = (char)combo.fKey;
            str = ST::string(&c, 1);
        } else {
            return ST_LITERAL("(unmapped)");
        }
    }
    if (combo.fFlags & plKeyCombo::kCtrl) {
        str += "_C";
    }
    if (combo.fFlags & plKeyCombo::kShift) {
        str += "_S";
    }
    return str;
}

plKeyCombo plKeyMap::StringToKeyCombo(const ST::string& keyStr)
{
    plKeyCombo combo;
    
    // Find modifiers to set flags with
    combo.fFlags = 0;
    if (keyStr.find("_S", ST::case_insensitive) != -1) {
        combo.fFlags |= plKeyCombo::kShift;
    }
    if (keyStr.find("_C", ST::case_insensitive) != -1) {
        combo.fFlags |= plKeyCombo::kCtrl;
    }
    
    // Convert raw key without modifiers
    combo.fKey = plKeyMap::ConvertCharToVKey(keyStr.before_first('_'));
    if (combo.fKey == KEY_UNMAPPED)
        combo = plKeyCombo::kUnmapped;
    
    return combo;
}

ST::string plKeyMap::GetStringCtrl()
{
    switch (plLocalization::GetLanguage())
    {
        case plLocalization::kFrench:
            return ST_LITERAL("Ctrl+");
            break;
        case plLocalization::kGerman:
            return ST_LITERAL("Strg+");
            break;
        case plLocalization::kSpanish:
            return ST_LITERAL("Ctrl+");
            break;
        case plLocalization::kItalian:
            return ST_LITERAL("Ctrl+");
            break;

        // default is English
        default:
            break;
    }
    return ST_LITERAL("Ctrl+");
}

ST::string plKeyMap::GetStringShift()
{
    switch (plLocalization::GetLanguage())
    {
        case plLocalization::kFrench:
            return ST_LITERAL("Maj+");
            break;
        case plLocalization::kGerman:
            return ST_LITERAL("Umschalt+");
            break;
        case plLocalization::kSpanish:
            return ST_LITERAL("Mayúsculas+");
            break;
        case plLocalization::kItalian:
            return ST_LITERAL("Shift+");
            break;

        // default is English
        default:
            break;
    }
    return ST_LITERAL("Shift+");
}

ST::string plKeyMap::GetStringUnmapped()
{
    switch (plLocalization::GetLanguage())
    {
        case plLocalization::kFrench:
            return ST_LITERAL("(NonDéfini)");
            break;
        case plLocalization::kGerman:
            return ST_LITERAL("(NichtZugewiesen)");
            break;
        case plLocalization::kSpanish:
            return ST_LITERAL("(SinMapear)");
            break;
        case plLocalization::kItalian:
            return ST_LITERAL("(NonAssegnato)");
            break;

        // default is English
        default:
            break;
    }
    return ST_LITERAL("(unmapped)");
}

// If the binding has one of these keys, but not the other, go and bind the other
// (if there's an unmapped space for it).
void plKeyMap::HandleAutoDualBinding(plKeyDef key1, plKeyDef key2)
{
    ICheckAndBindDupe(key1, key2);
    ICheckAndBindDupe(key2, key1);
}

void plKeyMap::ICheckAndBindDupe(plKeyDef origKey, plKeyDef dupeKey)
{
    std::vector<plKeyBinding*> bindings;
    plKeyCombo combo;
    combo.fKey = origKey;
    IFindAllBindingsByKey(combo, bindings);

    for (plKeyBinding *binding : bindings)
    {
        if (binding->HasUnmappedKey())
        {
            combo = binding->GetMatchingKey(origKey);
            combo.fKey = dupeKey;
            if (IFindBindingByKey(combo) == nullptr)
                IActuallyBind(binding, combo, kNoPreference);
        }
    }   
}

const std::map<uint32_t, ST::string> plKeyMap::fKeyConversionEnglish =
{ 
#ifdef HS_BUILD_FOR_WIN32
    { VK_F1,    ST_LITERAL("F1") },
    { VK_F2,    ST_LITERAL("F2") },
    { VK_F3,    ST_LITERAL("F3") },
    { VK_F4,    ST_LITERAL("F4") },
    { VK_F5,    ST_LITERAL("F5") },
    { VK_F6,    ST_LITERAL("F6") },
    { VK_F7,    ST_LITERAL("F7") }, 
    { VK_F8,    ST_LITERAL("F8") },
    { VK_F9,    ST_LITERAL("F9") },
    { VK_F10,   ST_LITERAL("F10") },
    { VK_F11,   ST_LITERAL("F11") },
    { VK_F12,   ST_LITERAL("F12") },
    { VK_ESCAPE, ST_LITERAL("Esc") },
    { VK_TAB,   ST_LITERAL("Tab") },
    { VK_UP,    ST_LITERAL("UpArrow") },
    { VK_DOWN,  ST_LITERAL("DownArrow") },
    { VK_LEFT,  ST_LITERAL("LeftArrow") },
    { VK_RIGHT, ST_LITERAL("RightArrow") },
    { VK_BACK,  ST_LITERAL("Backspace") },
    { VK_RETURN, ST_LITERAL("Enter") },
    { VK_PAUSE, ST_LITERAL("Pause") },
    { VK_CAPITAL, ST_LITERAL("CapsLock") },
    { VK_PRIOR, ST_LITERAL("PageUp") },
    { VK_NEXT,  ST_LITERAL("PageDn") },
    { VK_END,   ST_LITERAL("End") },
    { VK_HOME,  ST_LITERAL("Home") },
    { VK_SNAPSHOT,  ST_LITERAL("PrintScrn") },
    { VK_INSERT,    ST_LITERAL("Insert") },
    { VK_DELETE,    ST_LITERAL("Delete") },
    { VK_NUMPAD0,   ST_LITERAL("NumPad0") },
    { VK_NUMPAD1,   ST_LITERAL("NumPad1") },
    { VK_NUMPAD2,   ST_LITERAL("NumPad2") },
    { VK_NUMPAD3,   ST_LITERAL("NumPad3") },
    { VK_NUMPAD4,   ST_LITERAL("NumPad4") },
    { VK_NUMPAD5,   ST_LITERAL("NumPad5") },
    { VK_NUMPAD6,   ST_LITERAL("NumPad6") },
    { VK_NUMPAD7,   ST_LITERAL("NumPad7") },
    { VK_NUMPAD8,   ST_LITERAL("NumPad8") },
    { VK_NUMPAD9,   ST_LITERAL("NumPad9") },
    { VK_MULTIPLY,  ST_LITERAL("NumPad*") },
    { VK_ADD,       ST_LITERAL("NumPad+") },
    { VK_SUBTRACT,  ST_LITERAL("NumPad-") },
    { VK_DECIMAL,   ST_LITERAL("NumPad.") },
    { VK_DIVIDE,    ST_LITERAL("NumPad/") },
    { VK_SPACE,     ST_LITERAL("SpaceBar") },
    { VK_OEM_COMMA, ST_LITERAL("Comma") },
    { VK_OEM_PERIOD,ST_LITERAL("Period") },
    { VK_OEM_MINUS, ST_LITERAL("Minus") },
    { VK_OEM_PLUS,  ST_LITERAL("Plus") },
    { VK_SHIFT,     ST_LITERAL("Shift") },
    // not valid outside USA
    { VK_OEM_1,     ST_LITERAL("Semicolon") },
    { VK_OEM_2,     ST_LITERAL("ForewardSlash") },
    { VK_OEM_3,     ST_LITERAL("Tilde") },
    { VK_OEM_4,     ST_LITERAL("LeftBracket") },
    { VK_OEM_5,     ST_LITERAL("Backslash") },
    { VK_OEM_6,     ST_LITERAL("RightBracket") },
    { VK_OEM_7,     ST_LITERAL("Quote") },
#endif
};

const std::map<uint32_t, ST::string> plKeyMap::fKeyConversionFrench =
{
#ifdef HS_BUILD_FOR_WIN32
    { VK_F1,    ST_LITERAL("F1") },
    { VK_F2,    ST_LITERAL("F2") },
    { VK_F3,    ST_LITERAL("F3") },
    { VK_F4,    ST_LITERAL("F4") },
    { VK_F5,    ST_LITERAL("F5") },
    { VK_F6,    ST_LITERAL("F6") },
    { VK_F7,    ST_LITERAL("F7") },
    { VK_F8,    ST_LITERAL("F8") },
    { VK_F9,    ST_LITERAL("F9") },
    { VK_F10,   ST_LITERAL("F10") },
    { VK_F11,   ST_LITERAL("F11") },
    { VK_F12,   ST_LITERAL("F12") },
    { VK_ESCAPE, ST_LITERAL("Échap") },
    { VK_TAB,   ST_LITERAL("Tab") },
    { VK_UP,    ST_LITERAL("FlècheHaut") },
    { VK_DOWN,  ST_LITERAL("FlècheBas") },
    { VK_LEFT,  ST_LITERAL("FlècheGauche") },
    { VK_RIGHT, ST_LITERAL("FlècheDroite") },
    { VK_BACK,  ST_LITERAL("Retour") },
    { VK_RETURN, ST_LITERAL("Entrée") },
    { VK_PAUSE, ST_LITERAL("Pause") },
    { VK_CAPITAL, ST_LITERAL("CapsLock") },
    { VK_PRIOR, ST_LITERAL("PagePréc") },
    { VK_NEXT,  ST_LITERAL("PageSuiv") },
    { VK_END,   ST_LITERAL("Fin") },
    { VK_HOME,  ST_LITERAL("Origine") },
    { VK_SNAPSHOT,  ST_LITERAL("ImprÉcran") },
    { VK_INSERT,    ST_LITERAL("Inser") },
    { VK_DELETE,    ST_LITERAL("Suppr") },
    { VK_NUMPAD0,   ST_LITERAL("PavNum0") },
    { VK_NUMPAD1,   ST_LITERAL("PavNum1") },
    { VK_NUMPAD2,   ST_LITERAL("PavNum2") },
    { VK_NUMPAD3,   ST_LITERAL("PavNum3") },
    { VK_NUMPAD4,   ST_LITERAL("PavNum4") },
    { VK_NUMPAD5,   ST_LITERAL("PavNum5") },
    { VK_NUMPAD6,   ST_LITERAL("PavNum6") },
    { VK_NUMPAD7,   ST_LITERAL("PavNum7") },
    { VK_NUMPAD8,   ST_LITERAL("PavNum8") },
    { VK_NUMPAD9,   ST_LITERAL("PavNum9") },
    { VK_MULTIPLY,  ST_LITERAL("PavNum*") },
    { VK_ADD,       ST_LITERAL("PavNum+") },
    { VK_SUBTRACT,  ST_LITERAL("PavNum-") },
    { VK_DECIMAL,   ST_LITERAL("PavNum.") },
    { VK_DIVIDE,    ST_LITERAL("PavNum/") },
    { VK_SPACE,     ST_LITERAL("Espace") },
    { VK_OEM_COMMA, ST_LITERAL("Virgule") },
    { VK_OEM_PERIOD,ST_LITERAL("Point") },
    { VK_OEM_MINUS, ST_LITERAL("Moins") },
    { VK_OEM_PLUS,  ST_LITERAL("Plus") },
    { VK_SHIFT,     ST_LITERAL("Maj") },
    // not valid outside USA
    { VK_OEM_1,     ST_LITERAL("Point-virgule") },
    { VK_OEM_2,     ST_LITERAL("BarreOblique") },
    { VK_OEM_3,     ST_LITERAL("Tilde") },
    { VK_OEM_4,     ST_LITERAL("ParenthèseG") },
    { VK_OEM_5,     ST_LITERAL("BarreInverse") },
    { VK_OEM_6,     ST_LITERAL("ParenthèseD") },
    { VK_OEM_7,     ST_LITERAL("Guillemet") },
#endif
};

const std::map<uint32_t, ST::string> plKeyMap::fKeyConversionGerman =
{
#ifdef HS_BUILD_FOR_WIN32
    { VK_F1,    ST_LITERAL("F1") },
    { VK_F2,    ST_LITERAL("F2") },
    { VK_F3,    ST_LITERAL("F3") },
    { VK_F4,    ST_LITERAL("F4") },
    { VK_F5,    ST_LITERAL("F5") },
    { VK_F6,    ST_LITERAL("F6") },
    { VK_F7,    ST_LITERAL("F7") },
    { VK_F8,    ST_LITERAL("F8") },
    { VK_F9,    ST_LITERAL("F9") },
    { VK_F10,   ST_LITERAL("F10") },
    { VK_F11,   ST_LITERAL("F11") },
    { VK_F12,   ST_LITERAL("F12") },
    { VK_ESCAPE, ST_LITERAL("Esc") },
    { VK_TAB,   ST_LITERAL("Tab") },
    { VK_UP,    ST_LITERAL("PfeilHoch") },
    { VK_DOWN,  ST_LITERAL("PfeilRunter") },
    { VK_LEFT,  ST_LITERAL("PfeilLinks") },
    { VK_RIGHT, ST_LITERAL("PfeilRechts") },
    { VK_BACK,  ST_LITERAL("Backspace") },
    { VK_RETURN, ST_LITERAL("Enter") },
    { VK_PAUSE, ST_LITERAL("Pause") },
    { VK_CAPITAL, ST_LITERAL("Feststelltaste") },
    { VK_PRIOR, ST_LITERAL("BildHoch") },
    { VK_NEXT,  ST_LITERAL("BildRunter") },
    { VK_END,   ST_LITERAL("Ende") },
    { VK_HOME,  ST_LITERAL("Pos1") },
    { VK_SNAPSHOT,  ST_LITERAL("Druck") },
    { VK_INSERT,    ST_LITERAL("Einf") },
    { VK_DELETE,    ST_LITERAL("Entf") },
    { VK_NUMPAD0,   ST_LITERAL("ZB0") },
    { VK_NUMPAD1,   ST_LITERAL("ZB1") },
    { VK_NUMPAD2,   ST_LITERAL("ZB2") },
    { VK_NUMPAD3,   ST_LITERAL("ZB3") },
    { VK_NUMPAD4,   ST_LITERAL("ZB4") },
    { VK_NUMPAD5,   ST_LITERAL("ZB5") },
    { VK_NUMPAD6,   ST_LITERAL("ZB6") },
    { VK_NUMPAD7,   ST_LITERAL("ZB7") },
    { VK_NUMPAD8,   ST_LITERAL("ZB8") },
    { VK_NUMPAD9,   ST_LITERAL("ZB9") },
    { VK_MULTIPLY,  ST_LITERAL("ZB*") },
    { VK_ADD,       ST_LITERAL("ZB+") },
    { VK_SUBTRACT,  ST_LITERAL("ZB-") },
    { VK_DECIMAL,   ST_LITERAL("ZB.") },
    { VK_DIVIDE,    ST_LITERAL("ZB/") },
    { VK_SPACE,     ST_LITERAL("Leertaste") },
    { VK_OEM_COMMA, ST_LITERAL("Komma") },
    { VK_OEM_PERIOD,ST_LITERAL("Punkt") },
    { VK_OEM_MINUS, ST_LITERAL("Minus") },
    { VK_OEM_PLUS,  ST_LITERAL("Plus") },
    { VK_SHIFT,     ST_LITERAL("Umschalt") },
    // not valid outside USA
    { VK_OEM_1,     ST_LITERAL("Ü") },
    { VK_OEM_2,     ST_LITERAL("#") },
    { VK_OEM_3,     ST_LITERAL("Ö") },
    { VK_OEM_4,     ST_LITERAL("ß") },
    { VK_OEM_5,     ST_LITERAL("Backslash") },
    { VK_OEM_6,     ST_LITERAL("Akzent") },
    { VK_OEM_7,     ST_LITERAL("Ä") },
#endif
};

const std::map<uint32_t, ST::string> plKeyMap::fKeyConversionSpanish =
{
#ifdef HS_BUILD_FOR_WIN32
    { VK_F1,    ST_LITERAL("F1") },
    { VK_F2,    ST_LITERAL("F2") },
    { VK_F3,    ST_LITERAL("F3") },
    { VK_F4,    ST_LITERAL("F4") },
    { VK_F5,    ST_LITERAL("F5") },
    { VK_F6,    ST_LITERAL("F6") },
    { VK_F7,    ST_LITERAL("F7") },
    { VK_F8,    ST_LITERAL("F8") },
    { VK_F9,    ST_LITERAL("F9") },
    { VK_F10,   ST_LITERAL("F10") },
    { VK_F11,   ST_LITERAL("F11") },
    { VK_F12,   ST_LITERAL("F12") },
    { VK_ESCAPE, ST_LITERAL("Esc") },
    { VK_TAB,   ST_LITERAL("Tabulador") },
    { VK_UP,    ST_LITERAL("CursorArriba") },
    { VK_DOWN,  ST_LITERAL("CursorAbajo") },
    { VK_LEFT,  ST_LITERAL("CursorIzquierdo") },
    { VK_RIGHT, ST_LITERAL("CursorDerecho") },
    { VK_BACK,  ST_LITERAL("Retroceso") },
    { VK_RETURN, ST_LITERAL("Intro") },
    { VK_PAUSE, ST_LITERAL("Pausa") },
    { VK_CAPITAL, ST_LITERAL("BloqMayús") },
    { VK_PRIOR, ST_LITERAL("RePág") },
    { VK_NEXT,  ST_LITERAL("AVPág") },
    { VK_END,   ST_LITERAL("Fin") },
    { VK_HOME,  ST_LITERAL("Inicio") },
    { VK_SNAPSHOT,  ST_LITERAL("ImprPetSis") },
    { VK_INSERT,    ST_LITERAL("Insert") },
    { VK_DELETE,    ST_LITERAL("Supr") },
    { VK_NUMPAD0,   ST_LITERAL("TecNum0") },
    { VK_NUMPAD1,   ST_LITERAL("TecNum1") },
    { VK_NUMPAD2,   ST_LITERAL("TecNum2") },
    { VK_NUMPAD3,   ST_LITERAL("TecNum3") },
    { VK_NUMPAD4,   ST_LITERAL("TecNum4") },
    { VK_NUMPAD5,   ST_LITERAL("TecNum5") },
    { VK_NUMPAD6,   ST_LITERAL("TecNum6") }, 
    { VK_NUMPAD7,   ST_LITERAL("TecNum7") },
    { VK_NUMPAD8,   ST_LITERAL("TecNum8") },
    { VK_NUMPAD9,   ST_LITERAL("TecNum9") },
    { VK_MULTIPLY,  ST_LITERAL("TecNum*") },
    { VK_ADD,       ST_LITERAL("TecNum+") },
    { VK_SUBTRACT,  ST_LITERAL("TecNum-") },
    { VK_DECIMAL,   ST_LITERAL("TecNum.") },
    { VK_DIVIDE,    ST_LITERAL("TecNum/") },
    { VK_SPACE,     ST_LITERAL("BarraEspacio") },
    { VK_OEM_COMMA, ST_LITERAL("Coma") },
    { VK_OEM_PERIOD,ST_LITERAL("Punto") },
    { VK_OEM_MINUS, ST_LITERAL("Menos") },
    { VK_OEM_PLUS,  ST_LITERAL("Más") },
    { VK_SHIFT,     ST_LITERAL("Mayúsculas") },
    // not valid outside USA
    { VK_OEM_1,     ST_LITERAL("PuntoYComa") },
    { VK_OEM_2,     ST_LITERAL("Barra") },
    { VK_OEM_3,     ST_LITERAL("Tilde") },
    { VK_OEM_4,     ST_LITERAL("AbrirParéntesis") },
    { VK_OEM_5,     ST_LITERAL("BarraInvertida") },
    { VK_OEM_6,     ST_LITERAL("CerrarParéntesis") },
    { VK_OEM_7,     ST_LITERAL("Comillas") },
#endif
};

const std::map<uint32_t, ST::string> plKeyMap::fKeyConversionItalian =
{
#ifdef HS_BUILD_FOR_WIN32
    { VK_F1,    ST_LITERAL("F1") },
    { VK_F2,    ST_LITERAL("F2") },
    { VK_F3,    ST_LITERAL("F3") },
    { VK_F4,    ST_LITERAL("F4") },
    { VK_F5,    ST_LITERAL("F5") },
    { VK_F6,    ST_LITERAL("F6") },
    { VK_F7,    ST_LITERAL("F7") },
    { VK_F8,    ST_LITERAL("F8") },
    { VK_F9,    ST_LITERAL("F9") },
    { VK_F10,   ST_LITERAL("F10") },
    { VK_F11,   ST_LITERAL("F11") },
    { VK_F12,   ST_LITERAL("F12") },
    { VK_ESCAPE, ST_LITERAL("Esc") },
    { VK_TAB,   ST_LITERAL("Tab") },
    { VK_UP,    ST_LITERAL("FrecciaSu") },
    { VK_DOWN,  ST_LITERAL("FrecciaGiù") },
    { VK_LEFT,  ST_LITERAL("FrecciaSx") },
    { VK_RIGHT, ST_LITERAL("FrecciaDx") },
    { VK_BACK,  ST_LITERAL("Backspace") },
    { VK_RETURN, ST_LITERAL("Invio") },
    { VK_PAUSE, ST_LITERAL("Pausa") },
    { VK_CAPITAL, ST_LITERAL("BlocMaiusc") },
    { VK_PRIOR, ST_LITERAL("PagSu") },
    { VK_NEXT,  ST_LITERAL("PagGiù") },
    { VK_END,   ST_LITERAL("Fine") },
    { VK_HOME,  ST_LITERAL("Home") },
    { VK_SNAPSHOT,  ST_LITERAL("Stamp") },
    { VK_INSERT,    ST_LITERAL("Ins") },
    { VK_DELETE,    ST_LITERAL("Canc") },
    { VK_NUMPAD0,   ST_LITERAL("TastNum0") },
    { VK_NUMPAD1,   ST_LITERAL("TastNum1") },
    { VK_NUMPAD2,   ST_LITERAL("TastNum2") },
    { VK_NUMPAD3,   ST_LITERAL("TastNum3") },
    { VK_NUMPAD4,   ST_LITERAL("TastNum4") },
    { VK_NUMPAD5,   ST_LITERAL("TastNum5") },
    { VK_NUMPAD6,   ST_LITERAL("TastNum6") },
    { VK_NUMPAD7,   ST_LITERAL("TastNum7") },
    { VK_NUMPAD8,   ST_LITERAL("TastNum8") },
    { VK_NUMPAD9,   ST_LITERAL("TastNum9") },
    { VK_MULTIPLY,  ST_LITERAL("TastNum*") },
    { VK_ADD,       ST_LITERAL("TastNum+") },
    { VK_SUBTRACT,  ST_LITERAL("TastNum-") },
    { VK_DECIMAL,   ST_LITERAL("TastNum.") },
    { VK_DIVIDE,    ST_LITERAL("TastNum/") },
    { VK_SPACE,     ST_LITERAL("Spazio") },
    { VK_OEM_COMMA, ST_LITERAL("Virgola") },
    { VK_OEM_PERIOD,ST_LITERAL("Punto") },
    { VK_OEM_MINUS, ST_LITERAL("Meno") },
    { VK_OEM_PLUS,  ST_LITERAL("QuadraDx") },
    { VK_SHIFT,     ST_LITERAL("Shift") },
    // not valid outside USA
    { VK_OEM_1,     ST_LITERAL("QuadraSx") },
    { VK_OEM_2,     ST_LITERAL("ù") },
    { VK_OEM_3,     ST_LITERAL("ò") },
    { VK_OEM_4,     ST_LITERAL("Apostrofo") },
    { VK_OEM_5,     ST_LITERAL("\\") },
    { VK_OEM_6,     ST_LITERAL("ì") },
    { VK_OEM_7,     ST_LITERAL("à") },
#endif
};


const std::map<ControlEventCode, ST::string> plInputMap::fCmdConvert =
{

    { B_CONTROL_ACTION, ST_LITERAL("Use Key") },
    { B_CONTROL_JUMP, ST_LITERAL("Jump Key") },
    { B_CONTROL_DIVE, ST_LITERAL("Dive Key") },
    { B_CONTROL_MOVE_FORWARD, ST_LITERAL("Walk Forward") },
    { B_CONTROL_MOVE_BACKWARD, ST_LITERAL("Walk Backward") },
    { B_CONTROL_STRAFE_LEFT, ST_LITERAL("Strafe Left") },
    { B_CONTROL_STRAFE_RIGHT, ST_LITERAL("Strafe Right") },
    { B_CONTROL_MOVE_UP, ST_LITERAL("Move Up") },
    { B_CONTROL_MOVE_DOWN, ST_LITERAL("Move Down") },
    { B_CONTROL_ROTATE_LEFT, ST_LITERAL("Turn Left") },
    { B_CONTROL_ROTATE_RIGHT, ST_LITERAL("Turn Right") },
    { B_CONTROL_ROTATE_UP, ST_LITERAL("Turn Up") },
    { B_CONTROL_ROTATE_DOWN, ST_LITERAL("Turn Down") },
    { B_CONTROL_MODIFIER_FAST, ST_LITERAL("Fast Modifier") },
    { B_CONTROL_EQUIP, ST_LITERAL("PickUp Item") },
    { B_CONTROL_DROP, ST_LITERAL("Drop Item") },
    { B_TOGGLE_DRIVE_MODE, ST_LITERAL("Drive") },
    { B_CONTROL_ALWAYS_RUN, ST_LITERAL("Always Run") },
    { B_CAMERA_MOVE_FORWARD, ST_LITERAL("Camera Forward") },
    { B_CAMERA_MOVE_BACKWARD, ST_LITERAL("Camera Backward") },
    { B_CAMERA_MOVE_UP, ST_LITERAL("Camera Up") },
    { B_CAMERA_MOVE_DOWN, ST_LITERAL("Camera Down") },
    { B_CAMERA_MOVE_LEFT, ST_LITERAL("Camera Left") },
    { B_CAMERA_MOVE_RIGHT, ST_LITERAL("Camera Right") },
    { B_CAMERA_MOVE_FAST, ST_LITERAL("Camera Fast") },
    { B_CAMERA_ROTATE_RIGHT, ST_LITERAL("Camera Yaw Right") },
    { B_CAMERA_ROTATE_LEFT, ST_LITERAL("Camera Yaw Left") },
    { B_CAMERA_ROTATE_UP, ST_LITERAL("Camera Pitch Up") },
    { B_CAMERA_ROTATE_DOWN, ST_LITERAL("Camera Pitch Down") },
    { B_CAMERA_PAN_UP, ST_LITERAL("Camera Pan Up") },
    { B_CAMERA_PAN_DOWN, ST_LITERAL("Camera Pan Down") },
    { B_CAMERA_PAN_LEFT, ST_LITERAL("Camera Pan Left") },
    { B_CAMERA_PAN_RIGHT, ST_LITERAL("Camera Pan Right") },
    { B_CAMERA_PAN_TO_CURSOR, ST_LITERAL("Camera Pan To Cursor") },
    { B_CAMERA_RECENTER, ST_LITERAL("Camera Recenter") },
    { B_SET_CONSOLE_MODE, ST_LITERAL("Console") },
    { B_CAMERA_DRIVE_SPEED_UP, ST_LITERAL("Decrease Camera Drive Speed") },
    { B_CAMERA_DRIVE_SPEED_DOWN, ST_LITERAL("Increase Camera Drive Speed") },
    { S_INCREASE_MIC_VOL, ST_LITERAL("Increase Microphone Sensitivity") },
    { S_DECREASE_MIC_VOL, ST_LITERAL("Decrease Microphone Sensitivity") },
    { S_PUSH_TO_TALK, ST_LITERAL("Push to talk") },
    { S_SET_WALK_MODE, ST_LITERAL("Set Walk Mode") },
    { B_CONTROL_TURN_TO, ST_LITERAL("Turn To Click") },
    { B_CONTROL_TOGGLE_PHYSICAL, ST_LITERAL("Toggle Physical") },
    { S_SET_FIRST_PERSON_MODE, ST_LITERAL("Toggle First Person") },
    { B_CAMERA_ZOOM_IN, ST_LITERAL("Camera Zoom In") },
    { B_CAMERA_ZOOM_OUT, ST_LITERAL("Camera Zoom Out") },
    { B_CONTROL_EXIT_MODE, ST_LITERAL("Exit Mode") },
    { B_CONTROL_OPEN_KI, ST_LITERAL("Open KI") },
    { B_CONTROL_OPEN_BOOK, ST_LITERAL("Open Player Book") },
    { B_CONTROL_EXIT_GUI_MODE, ST_LITERAL("Exit GUI Mode") },
    { B_CONTROL_MODIFIER_STRAFE, ST_LITERAL("Strafe Modifier") },
};
