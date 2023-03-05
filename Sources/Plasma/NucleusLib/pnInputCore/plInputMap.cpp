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
#include <string>

#include "plInputMap.h"
#include "plKeyMap.h"

#include "plResMgr/plLocalization.h"

ControlEventCode plInputMap::ConvertCharToControlCode(const char* c)
{
    for (int i = 0; fCmdConvert[i].fCode != END_CONTROLS; i++)
    {
        if (stricmp(fCmdConvert[i].fDesc, c) == 0)
            return (fCmdConvert[i].fCode);
    }
    return (END_CONTROLS);
}

const char      *plInputMap::ConvertControlCodeToString( ControlEventCode code )
{
    for( int i = 0; fCmdConvert[ i ].fCode != END_CONTROLS; i++ )
    {
        if( fCmdConvert[ i ].fCode == code )
            return fCmdConvert[ i ].fDesc;
    }
    return nullptr;
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
    fString = nullptr;
}

plKeyBinding::plKeyBinding(ControlEventCode code, uint32_t codeFlags, const plKeyCombo &key1, const plKeyCombo &key2, const char *string /*= nullptr*/)
{
    fCode = code;
    fCodeFlags = codeFlags;
    fKey1 = key1;
    fKey2 = key2;
    fString = (string == nullptr) ? nullptr : hsStrcpy(string);
}

plKeyBinding::~plKeyBinding()
{
    delete [] fString;
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

bool    plKeyMap::AddConsoleCommand( const char *command )
{
    if (IFindConsoleBinding(command) != nullptr)
        return false;

    fBindings.emplace_back(new plKeyBinding(B_CONTROL_CONSOLE_COMMAND,
                                            kControlFlagDownEvent | kControlFlagNoRepeat | kControlFlagNoDeactivate,
                                            plKeyCombo::kUnmapped, plKeyCombo::kUnmapped,
                                            command));
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

plKeyBinding    *plKeyMap::IFindConsoleBinding( const char *command ) const
{
    for (plKeyBinding* binding : fBindings)
    {
        if (binding->GetCode() == B_CONTROL_CONSOLE_COMMAND)
        {
            if (stricmp(binding->GetExtendedString(), command) == 0)
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

bool    plKeyMap::BindKeyToConsoleCmd( const plKeyCombo &combo, const char *command, BindPref pref /*= kNoPreference*/ )
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


const plKeyBinding* plKeyMap::FindConsoleBinding( const char *command ) const
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


const char* plKeyMap::ConvertVKeyToChar( uint32_t vk )
{
    Win32keyConvert* keyConvert = &fKeyConversionEnglish[0];
    switch (plLocalization::GetLanguage())
    {
        case plLocalization::kFrench:
            keyConvert = &fKeyConversionFrench[0];
            break;
        case plLocalization::kGerman:
            keyConvert = &fKeyConversionGerman[0];
            break;
        case plLocalization::kSpanish:
            keyConvert = &fKeyConversionSpanish[0];
            break;
        case plLocalization::kItalian:
            keyConvert = &fKeyConversionItalian[0];
            break;

        // default is English
        default:
            break;
    }
    for (int i = 0; keyConvert[i].fVKey != 0xffffffff; i++)
    {
        if (keyConvert[i].fVKey == vk)
            return (keyConvert[i].fKeyName);
    }

    return nullptr;
}

plKeyDef plKeyMap::ConvertCharToVKey( const char *c )
{
    Win32keyConvert* keyConvert = &fKeyConversionEnglish[0];
    switch (plLocalization::GetLanguage())
    {
        case plLocalization::kFrench:
            keyConvert = &fKeyConversionFrench[0];
            break;
        case plLocalization::kGerman:
            keyConvert = &fKeyConversionGerman[0];
            break;
        case plLocalization::kSpanish:
            keyConvert = &fKeyConversionSpanish[0];
            break;
        case plLocalization::kItalian:
            keyConvert = &fKeyConversionItalian[0];
            break;

        // default is English
        default:
            break;
    }
    for (int i = 0; keyConvert[i].fVKey != 0xffffffff; i++)
    {
        if (stricmp(keyConvert[i].fKeyName, c) == 0)
            return (plKeyDef)(keyConvert[i].fVKey);
    }

    // Is it just a single character?
    if( isalnum( *c ) && strlen( c ) == 1 )
        return (plKeyDef)toupper( *c );

    // if we didn't find anything yet...
    // ...then look thru all the other language mappings that we know about,
    // ...just in case they keep switching languages on us
    if ( plLocalization::GetLanguage() != plLocalization::kEnglish)
    {
        for (int i = 0; fKeyConversionEnglish[i].fVKey != 0xffffffff; i++)
        {
            if (stricmp(fKeyConversionEnglish[i].fKeyName, c) == 0)
                return (plKeyDef)(fKeyConversionEnglish[i].fVKey);
        }
    }
    if ( plLocalization::GetLanguage() != plLocalization::kFrench)
    {
        for (int i = 0; fKeyConversionFrench[i].fVKey != 0xffffffff; i++)
        {
            if (stricmp(fKeyConversionFrench[i].fKeyName, c) == 0)
                return (plKeyDef)(fKeyConversionFrench[i].fVKey);
        }
    }
    if ( plLocalization::GetLanguage() != plLocalization::kGerman)
    {
        for (int i = 0; fKeyConversionGerman[i].fVKey != 0xffffffff; i++)
        {
            if (stricmp(fKeyConversionGerman[i].fKeyName, c) == 0)
                return (plKeyDef)(fKeyConversionGerman[i].fVKey);
        }
    }
    if ( plLocalization::GetLanguage() != plLocalization::kSpanish)
    {
        for (int i = 0; fKeyConversionSpanish[i].fVKey != 0xffffffff; i++)
        {
            if (stricmp(fKeyConversionSpanish[i].fKeyName, c) == 0)
                return (plKeyDef)(fKeyConversionSpanish[i].fVKey);
        }
    }
    if ( plLocalization::GetLanguage() != plLocalization::kItalian)
    {
        for (int i = 0; fKeyConversionItalian[i].fVKey != 0xffffffff; i++)
        {
            if (stricmp(fKeyConversionItalian[i].fKeyName, c) == 0)
                return (plKeyDef)(fKeyConversionItalian[i].fVKey);
        }
    }

    // finally, just give up... unmapped!
    return KEY_UNMAPPED;
}

const char* plKeyMap::GetStringCtrl()
{
    switch (plLocalization::GetLanguage())
    {
        case plLocalization::kFrench:
            return "Ctrl+";
            break;
        case plLocalization::kGerman:
            return "Strg+";
            break;
        case plLocalization::kSpanish:
            return "Ctrl+";
            break;
        case plLocalization::kItalian:
            return "Ctrl+";
            break;

        // default is English
        default:
            break;
    }
    return "Ctrl+";
}

const char* plKeyMap::GetStringShift()
{
    switch (plLocalization::GetLanguage())
    {
        case plLocalization::kFrench:
            return "Maj+";
            break;
        case plLocalization::kGerman:
            return "Umschalt+";
            break;
        case plLocalization::kSpanish:
            return "Mayúsculas+";
            break;
        case plLocalization::kItalian:
            return "Shift+";
            break;

        // default is English
        default:
            break;
    }
    return "Shift+";
}

const char* plKeyMap::GetStringUnmapped()
{
    switch (plLocalization::GetLanguage())
    {
        case plLocalization::kFrench:
            return "(NonDéfini)";
            break;
        case plLocalization::kGerman:
            return "(NichtZugewiesen)";
            break;
        case plLocalization::kSpanish:
            return "(SinMapear)";
            break;
        case plLocalization::kItalian:
            return "(NonAssegnato)";
            break;

        // default is English
        default:
            break;
    }
    return "(unmapped)";
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

Win32keyConvert plKeyMap::fKeyConversionEnglish[] =
{ 
#ifdef HS_BUILD_FOR_WIN32
    { VK_F1,    "F1"}, 
    { VK_F2,    "F2"}, 
    { VK_F3,    "F3"}, 
    { VK_F4,    "F4"},
    { VK_F5,    "F5"},
    { VK_F6,    "F6"},
    { VK_F7,    "F7"}, 
    { VK_F8,    "F8"},
    { VK_F9,    "F9"},
    { VK_F10,   "F10"},
    { VK_F11,   "F11"},
    { VK_F12,   "F12"},
    { VK_ESCAPE, "Esc"},
    { VK_TAB,   "Tab"},
    { VK_UP,    "UpArrow"}, 
    { VK_DOWN,  "DownArrow"}, 
    { VK_LEFT,  "LeftArrow"},
    { VK_RIGHT, "RightArrow"},
    { VK_BACK,  "Backspace"},
    { VK_RETURN, "Enter"}, 
    { VK_PAUSE, "Pause"},
    { VK_CAPITAL, "CapsLock"},
    { VK_PRIOR, "PageUp"},
    { VK_NEXT,  "PageDn"},
    { VK_END,   "End"},
    { VK_HOME,  "Home"},
    { VK_SNAPSHOT,  "PrintScrn"},
    { VK_INSERT,    "Insert"},
    { VK_DELETE,    "Delete"},
    { VK_NUMPAD0,   "NumPad0"}, 
    { VK_NUMPAD1,   "NumPad1"}, 
    { VK_NUMPAD2,   "NumPad2"}, 
    { VK_NUMPAD3,   "NumPad3"},
    { VK_NUMPAD4,   "NumPad4"},
    { VK_NUMPAD5,   "NumPad5"},
    { VK_NUMPAD6,   "NumPad6"}, 
    { VK_NUMPAD7,   "NumPad7"},
    { VK_NUMPAD8,   "NumPad8"},
    { VK_NUMPAD9,   "NumPad9"},
    { VK_MULTIPLY,  "NumPad*"},
    { VK_ADD,       "NumPad+"},
    { VK_SUBTRACT,  "NumPad-"},
    { VK_DECIMAL,   "NumPad."},
    { VK_DIVIDE,    "NumPad/"},
    { VK_SPACE,     "SpaceBar"},
    { VK_OEM_COMMA, "Comma"},
    { VK_OEM_PERIOD,"Period"},
    { VK_OEM_MINUS, "Minus"},
    { VK_OEM_PLUS,  "Plus"},
    { VK_SHIFT,     "Shift" },
    // not valid outside USA
    { VK_OEM_1,     "Semicolon"},
    { VK_OEM_2,     "ForewardSlash"},
    { VK_OEM_3,     "Tilde"},
    { VK_OEM_4,     "LeftBracket"},
    { VK_OEM_5,     "Backslash"},   
    { VK_OEM_6,     "RightBracket"},
    { VK_OEM_7,     "Quote"},
#endif
                
    { 0xffffffff,   "Unused"},
};

Win32keyConvert  plKeyMap::fKeyConversionFrench[] =
{
#ifdef HS_BUILD_FOR_WIN32
    { VK_F1,    "F1"}, 
    { VK_F2,    "F2"}, 
    { VK_F3,    "F3"}, 
    { VK_F4,    "F4"},
    { VK_F5,    "F5"},
    { VK_F6,    "F6"},
    { VK_F7,    "F7"}, 
    { VK_F8,    "F8"},
    { VK_F9,    "F9"},
    { VK_F10,   "F10"},
    { VK_F11,   "F11"},
    { VK_F12,   "F12"},
    { VK_ESCAPE, "Échap"},
    { VK_TAB,   "Tab"},
    { VK_UP,    "FlècheHaut"}, 
    { VK_DOWN,  "FlècheBas"}, 
    { VK_LEFT,  "FlècheGauche"},
    { VK_RIGHT, "FlècheDroite"},
    { VK_BACK,  "Retour"},
    { VK_RETURN, "Entrée"}, 
    { VK_PAUSE, "Pause"},
    { VK_CAPITAL, "CapsLock"},
    { VK_PRIOR, "PagePréc"},
    { VK_NEXT,  "PageSuiv"},
    { VK_END,   "Fin"},
    { VK_HOME,  "Origine"},
    { VK_SNAPSHOT,  "ImprÉcran"},
    { VK_INSERT,    "Inser"},
    { VK_DELETE,    "Suppr"},
    { VK_NUMPAD0,   "PavNum0"}, 
    { VK_NUMPAD1,   "PavNum1"}, 
    { VK_NUMPAD2,   "PavNum2"}, 
    { VK_NUMPAD3,   "PavNum3"},
    { VK_NUMPAD4,   "PavNum4"},
    { VK_NUMPAD5,   "PavNum5"},
    { VK_NUMPAD6,   "PavNum6"}, 
    { VK_NUMPAD7,   "PavNum7"},
    { VK_NUMPAD8,   "PavNum8"},
    { VK_NUMPAD9,   "PavNum9"},
    { VK_MULTIPLY,  "PavNum*"},
    { VK_ADD,       "PavNum+"},
    { VK_SUBTRACT,  "PavNum-"},
    { VK_DECIMAL,   "PavNum."},
    { VK_DIVIDE,    "PavNum/"},
    { VK_SPACE,     "Espace"},
    { VK_OEM_COMMA, "Virgule"},
    { VK_OEM_PERIOD,"Point"},
    { VK_OEM_MINUS, "Moins"},
    { VK_OEM_PLUS,  "Plus"},
    { VK_SHIFT,     "Maj"   },
    // not valid outside USA
    { VK_OEM_1,     "Point-virgule"},
    { VK_OEM_2,     "BarreOblique"},
    { VK_OEM_3,     "Tilde"},
    { VK_OEM_4,     "ParenthèseG"},
    { VK_OEM_5,     "BarreInverse"},    
    { VK_OEM_6,     "ParenthèseD"},
    { VK_OEM_7,     "Guillemet"},
#endif
                
    { 0xffffffff,   "Unused"},
};

Win32keyConvert  plKeyMap::fKeyConversionGerman[] =
{
#ifdef HS_BUILD_FOR_WIN32
    { VK_F1,    "F1"}, 
    { VK_F2,    "F2"}, 
    { VK_F3,    "F3"}, 
    { VK_F4,    "F4"},
    { VK_F5,    "F5"},
    { VK_F6,    "F6"},
    { VK_F7,    "F7"}, 
    { VK_F8,    "F8"},
    { VK_F9,    "F9"},
    { VK_F10,   "F10"},
    { VK_F11,   "F11"},
    { VK_F12,   "F12"},
    { VK_ESCAPE, "Esc"},
    { VK_TAB,   "Tab"},
    { VK_UP,    "PfeilHoch"}, 
    { VK_DOWN,  "PfeilRunter"}, 
    { VK_LEFT,  "PfeilLinks"},
    { VK_RIGHT, "PfeilRechts"},
    { VK_BACK,  "Backspace"},
    { VK_RETURN, "Enter"}, 
    { VK_PAUSE, "Pause"},
    { VK_CAPITAL, "Feststelltaste"},
    { VK_PRIOR, "BildHoch"},
    { VK_NEXT,  "BildRunter"},
    { VK_END,   "Ende"},
    { VK_HOME,  "Pos1"},
    { VK_SNAPSHOT,  "Druck"},
    { VK_INSERT,    "Einf"},
    { VK_DELETE,    "Entf"},
    { VK_NUMPAD0,   "ZB0"}, 
    { VK_NUMPAD1,   "ZB1"}, 
    { VK_NUMPAD2,   "ZB2"}, 
    { VK_NUMPAD3,   "ZB3"},
    { VK_NUMPAD4,   "ZB4"},
    { VK_NUMPAD5,   "ZB5"},
    { VK_NUMPAD6,   "ZB6"}, 
    { VK_NUMPAD7,   "ZB7"},
    { VK_NUMPAD8,   "ZB8"},
    { VK_NUMPAD9,   "ZB9"},
    { VK_MULTIPLY,  "ZB*"},
    { VK_ADD,       "ZB+"},
    { VK_SUBTRACT,  "ZB-"},
    { VK_DECIMAL,   "ZB."},
    { VK_DIVIDE,    "ZB/"},
    { VK_SPACE,     "Leertaste"},
    { VK_OEM_COMMA, "Komma"},
    { VK_OEM_PERIOD,"Punkt"},
    { VK_OEM_MINUS, "Minus"},
    { VK_OEM_PLUS,  "Plus"},
    { VK_SHIFT,     "Umschalt"  },
    // not valid outside USA
    { VK_OEM_1,     "Ü"},
    { VK_OEM_2,     "#"},
    { VK_OEM_3,     "Ö"},
    { VK_OEM_4,     "ß"},
    { VK_OEM_5,     "Backslash"},   
    { VK_OEM_6,     "Akzent"},
    { VK_OEM_7,     "Ä"},
#endif
                
    { 0xffffffff,   "Unused"},
};

Win32keyConvert  plKeyMap::fKeyConversionSpanish[] =
{
#ifdef HS_BUILD_FOR_WIN32
    { VK_F1,    "F1"}, 
    { VK_F2,    "F2"}, 
    { VK_F3,    "F3"}, 
    { VK_F4,    "F4"},
    { VK_F5,    "F5"},
    { VK_F6,    "F6"},
    { VK_F7,    "F7"}, 
    { VK_F8,    "F8"},
    { VK_F9,    "F9"},
    { VK_F10,   "F10"},
    { VK_F11,   "F11"},
    { VK_F12,   "F12"},
    { VK_ESCAPE, "Esc"},
    { VK_TAB,   "Tabulador"},
    { VK_UP,    "CursorArriba"}, 
    { VK_DOWN,  "CursorAbajo"}, 
    { VK_LEFT,  "CursorIzquierdo"},
    { VK_RIGHT, "CursorDerecho"},
    { VK_BACK,  "Retroceso"},
    { VK_RETURN, "Intro"}, 
    { VK_PAUSE, "Pausa"},
    { VK_CAPITAL, "BloqMayús"},
    { VK_PRIOR, "RePág"},
    { VK_NEXT,  "AVPág"},
    { VK_END,   "Fin"},
    { VK_HOME,  "Inicio"},
    { VK_SNAPSHOT,  "ImprPetSis"},
    { VK_INSERT,    "Insert"},
    { VK_DELETE,    "Supr"},
    { VK_NUMPAD0,   "TecNum0"}, 
    { VK_NUMPAD1,   "TecNum1"}, 
    { VK_NUMPAD2,   "TecNum2"}, 
    { VK_NUMPAD3,   "TecNum3"},
    { VK_NUMPAD4,   "TecNum4"},
    { VK_NUMPAD5,   "TecNum5"},
    { VK_NUMPAD6,   "TecNum6"}, 
    { VK_NUMPAD7,   "TecNum7"},
    { VK_NUMPAD8,   "TecNum8"},
    { VK_NUMPAD9,   "TecNum9"},
    { VK_MULTIPLY,  "TecNum*"},
    { VK_ADD,       "TecNum+"},
    { VK_SUBTRACT,  "TecNum-"},
    { VK_DECIMAL,   "TecNum."},
    { VK_DIVIDE,    "TecNum/"},
    { VK_SPACE,     "BarraEspacio"},
    { VK_OEM_COMMA, "Coma"},
    { VK_OEM_PERIOD,"Punto"},
    { VK_OEM_MINUS, "Menos"},
    { VK_OEM_PLUS,  "Más"},
    { VK_SHIFT,     "Mayúsculas"    },
    // not valid outside USA
    { VK_OEM_1,     "PuntoYComa"},
    { VK_OEM_2,     "Barra"},
    { VK_OEM_3,     "Tilde"},
    { VK_OEM_4,     "AbrirParéntesis"},
    { VK_OEM_5,     "BarraInvertida"},  
    { VK_OEM_6,     "CerrarParéntesis"},
    { VK_OEM_7,     "Comillas"},
#endif
                
    { 0xffffffff,   "Unused"},
};

Win32keyConvert  plKeyMap::fKeyConversionItalian[] =
{
#ifdef HS_BUILD_FOR_WIN32
    { VK_F1,    "F1"}, 
    { VK_F2,    "F2"}, 
    { VK_F3,    "F3"}, 
    { VK_F4,    "F4"},
    { VK_F5,    "F5"},
    { VK_F6,    "F6"},
    { VK_F7,    "F7"}, 
    { VK_F8,    "F8"},
    { VK_F9,    "F9"},
    { VK_F10,   "F10"},
    { VK_F11,   "F11"},
    { VK_F12,   "F12"},
    { VK_ESCAPE, "Esc"},
    { VK_TAB,   "Tab"},
    { VK_UP,    "FrecciaSu"}, 
    { VK_DOWN,  "FrecciaGiù"}, 
    { VK_LEFT,  "FrecciaSx"},
    { VK_RIGHT, "FrecciaDx"},
    { VK_BACK,  "Backspace"},
    { VK_RETURN, "Invio"}, 
    { VK_PAUSE, "Pausa"},
    { VK_CAPITAL, "BlocMaiusc"},
    { VK_PRIOR, "PagSu"},
    { VK_NEXT,  "PagGiù"},
    { VK_END,   "Fine"},
    { VK_HOME,  "Home"},
    { VK_SNAPSHOT,  "Stamp"},
    { VK_INSERT,    "Ins"},
    { VK_DELETE,    "Canc"},
    { VK_NUMPAD0,   "TastNum0"}, 
    { VK_NUMPAD1,   "TastNum1"}, 
    { VK_NUMPAD2,   "TastNum2"}, 
    { VK_NUMPAD3,   "TastNum3"},
    { VK_NUMPAD4,   "TastNum4"},
    { VK_NUMPAD5,   "TastNum5"},
    { VK_NUMPAD6,   "TastNum6"}, 
    { VK_NUMPAD7,   "TastNum7"},
    { VK_NUMPAD8,   "TastNum8"},
    { VK_NUMPAD9,   "TastNum9"},
    { VK_MULTIPLY,  "TastNum*"},
    { VK_ADD,       "TastNum+"},
    { VK_SUBTRACT,  "TastNum-"},
    { VK_DECIMAL,   "TastNum."},
    { VK_DIVIDE,    "TastNum/"},
    { VK_SPACE,     "Spazio"},
    { VK_OEM_COMMA, "Virgola"},
    { VK_OEM_PERIOD,"Punto"},
    { VK_OEM_MINUS, "Meno"},
    { VK_OEM_PLUS,  "QuadraDx"},
    { VK_SHIFT,     "Shift" },
    // not valid outside USA
    { VK_OEM_1,     "QuadraSx"},
    { VK_OEM_2,     "ù"},
    { VK_OEM_3,     "ò"},
    { VK_OEM_4,     "Apostrofo"},
    { VK_OEM_5,     "\\"},  
    { VK_OEM_6,     "ì"},
    { VK_OEM_7,     "à"},
#endif
                
    { 0xffffffff,   "Unused"},
};



CommandConvert plInputMap::fCmdConvert[] =
{

    { B_CONTROL_ACTION,         "Use Key"   },
    { B_CONTROL_JUMP,               "Jump Key"  },
    { B_CONTROL_DIVE,               "Dive Key"  },
    { B_CONTROL_MOVE_FORWARD,       "Walk Forward"  },
    { B_CONTROL_MOVE_BACKWARD,  "Walk Backward" },
    { B_CONTROL_STRAFE_LEFT,        "Strafe Left"   },
    { B_CONTROL_STRAFE_RIGHT,       "Strafe Right"  },
    { B_CONTROL_MOVE_UP,            "Move Up"       },
    { B_CONTROL_MOVE_DOWN,      "Move Down"     },
    { B_CONTROL_ROTATE_LEFT,        "Turn Left"     },
    { B_CONTROL_ROTATE_RIGHT,       "Turn Right"        },
    { B_CONTROL_ROTATE_UP,      "Turn Up"       },
    { B_CONTROL_ROTATE_DOWN,        "Turn Down"     },
    { B_CONTROL_MODIFIER_FAST,              "Fast Modifier" },
    { B_CONTROL_EQUIP,          "PickUp Item"   },
    { B_CONTROL_DROP,               "Drop Item"     },
    { B_TOGGLE_DRIVE_MODE,      "Drive" },
    { B_CONTROL_ALWAYS_RUN,     "Always Run" },
    { B_CAMERA_MOVE_FORWARD,        "Camera Forward"},
    { B_CAMERA_MOVE_BACKWARD,       "Camera Backward"},
    { B_CAMERA_MOVE_UP,         "Camera Up"},
    { B_CAMERA_MOVE_DOWN,           "Camera Down"},
    { B_CAMERA_MOVE_LEFT,           "Camera Left"},
    { B_CAMERA_MOVE_RIGHT,      "Camera Right"},
    { B_CAMERA_MOVE_FAST,           "Camera Fast"},
    { B_CAMERA_ROTATE_RIGHT,        "Camera Yaw Right"},
    { B_CAMERA_ROTATE_LEFT,     "Camera Yaw Left"},
    { B_CAMERA_ROTATE_UP,           "Camera Pitch Up"},
    { B_CAMERA_ROTATE_DOWN,     "Camera Pitch Down"},
    { B_CAMERA_PAN_UP,          "Camera Pan Up"},
    { B_CAMERA_PAN_DOWN,        "Camera Pan Down"},
    { B_CAMERA_PAN_LEFT,        "Camera Pan Left"},
    { B_CAMERA_PAN_RIGHT,       "Camera Pan Right"},
    { B_CAMERA_PAN_TO_CURSOR,   "Camera Pan To Cursor"},
    { B_CAMERA_RECENTER,        "Camera Recenter"},
    { B_SET_CONSOLE_MODE,           "Console"},
    { B_CAMERA_DRIVE_SPEED_UP,      "Decrease Camera Drive Speed"   },
    { B_CAMERA_DRIVE_SPEED_DOWN,    "Increase Camera Drive Speed"   },
    { S_INCREASE_MIC_VOL,       "Increase Microphone Sensitivity"   },
    { S_DECREASE_MIC_VOL,       "Decrease Microphone Sensitivity"   },
    { S_PUSH_TO_TALK,           "Push to talk" },
    { S_SET_WALK_MODE,          "Set Walk Mode" },          
    { B_CONTROL_TURN_TO,            "Turn To Click" },
    { B_CONTROL_TOGGLE_PHYSICAL,    "Toggle Physical" },
    { S_SET_FIRST_PERSON_MODE,      "Toggle First Person" },
    { B_CAMERA_ZOOM_IN,             "Camera Zoom In" },
    { B_CAMERA_ZOOM_OUT,            "Camera Zoom Out" },
    { B_CONTROL_EXIT_MODE,          "Exit Mode" },
    { B_CONTROL_OPEN_KI,            "Open KI" },
    { B_CONTROL_OPEN_BOOK,          "Open Player Book" },
    { B_CONTROL_EXIT_GUI_MODE,      "Exit GUI Mode" },
    { B_CONTROL_MODIFIER_STRAFE,    "Strafe Modifier" },


    { END_CONTROLS,             ""},
 
};
