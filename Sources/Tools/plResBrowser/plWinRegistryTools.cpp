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
//////////////////////////////////////////////////////////////////////////////
//
//  plWinRegistryTools
//  Utility class for doing various usefull things in Win32
//  Written by Mathew Burrack
//  4.23.2002
//
//////////////////////////////////////////////////////////////////////////////

#include "plWinRegistryTools.h"

#include "HeadSpin.h"
#include "hsWindows.h"

#include <QString>
#include <string>


//////////////////////////////////////////////////////////////////////////////
//// Static Utility Functions ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// ISetRegKey //////////////////////////////////////////////////////////////
//  Sets the given registry key to the given string value. If valueName = nil,
//  sets the (default) value

static bool ISetRegKey(const QString &keyName, const QString &value, const QString &valueName = QString())
{
    HKEY    regKey;
    DWORD   result;


    // Create the key (just opens if it already exists)
    if (::RegCreateKeyExW(HKEY_CLASSES_ROOT, keyName.toStdWString().c_str(), 0,
                          nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                          nullptr, &regKey, &result) != ERROR_SUCCESS)
    {
        hsStatusMessageF("Warning: Registry database open failed for key '{}'.", qUtf8Printable(keyName));
        return false;
    }

    // Assign the "default" subkey value
    std::wstring wValue = value.toStdWString();
    LONG lResult = ::RegSetValueExW(regKey, valueName.toStdWString().c_str(), 0,
                                    REG_SZ, reinterpret_cast<const BYTE *>(wValue.c_str()),
                                    (wValue.size() + 1) * sizeof(wchar_t));
    
    if (::RegCloseKey(regKey) == ERROR_SUCCESS && lResult == ERROR_SUCCESS)
        return true;

    hsStatusMessageF("Warning: Registry database update failed for key '{}'.", qUtf8Printable(keyName));
    return false;
}

//////////////////////////////////////////////////////////////////////////////
//// Public Utility Functions ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// AssociateFileType ///////////////////////////////////////////////////////
//  Associates a given file type in the Win32 registry with the given 
//  application. Also assigns a default icon if iconIndex != -1
//
//  To do this, we create a set of keys in the registry under CLASSES_ROOT that
//  looks like this:
//      fileTypeID  (value = fileTypeName)
//          |
//          |--- DefaultIcon (value = path,index) [omit this one if you don't 
//          |                                      want a default icon]
//          |
//          |--- shell
//                 |
//                 |--- open
//                        |
//                        |--- command (value = command line)
//

bool plWinRegistryTools::AssociateFileType(const QString &fileTypeID, const QString &fileTypeName,
                                           const QString &appPath, int iconIndex)
{
    // Root key
    if (!ISetRegKey(fileTypeID, fileTypeName))
        return false;

    // DefaultIcon key, if we want one
    if (iconIndex != -1)
    {
        QString keyName = QString(R"(%1\DefaultIcon)").arg(fileTypeID);
        QString keyValue = QString("%1,%2").arg(appPath).arg(iconIndex);
        if (!ISetRegKey(keyName, keyValue))
            return false;
    }

    // shell/open/command key
    QString keyName = QString(R"(%1\shell\open\command)").arg(fileTypeID);
    QString keyValue = QString(R"("%1" "%2")").arg(appPath, "%1");
    if (!ISetRegKey(keyName, keyValue))
        return false;

    // Success!
    return true;
}

//// AssociateFileExtension //////////////////////////////////////////////////
//  Assigns a given file extension to a previously registered Win32 file type 
//  (using the above function)
//
//  We do this by creating a key entry under CLASSES_ROOT of the following 
//  structure:
//
//          fileExtension (value = fileTypeID)
//
//  where fileExtension includes the leading . and fileTypeID is the same
//  typeID registered with the above function

bool plWinRegistryTools::AssociateFileExtension(const QString &fileExtension, const QString &fileTypeID)
{
    return ISetRegKey(fileExtension, fileTypeID);
}

//// GetCurrentFileExtensionAssociation //////////////////////////////////////
//  Obtains the current fileTypeID associated with the given file extension,
//  or a null string if it isn't yet associated.

QString plWinRegistryTools::GetCurrentFileExtensionAssociation(const QString &extension)
{
    long dataLen = 512;
    wchar_t buffer[512];
    buffer[0] = 0;

    LONG retVal = ::RegQueryValueW(HKEY_CLASSES_ROOT, extension.toStdWString().c_str(), buffer, &dataLen);
    if (retVal != ERROR_SUCCESS)
    {
        wchar_t msg[512];
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, retVal, 0, msg, std::size(msg), nullptr);
        hsStatusMessageF("Error querying registry key '{}' : {}", qUtf8Printable(extension), msg);
        return QString();
    }

    if (dataLen < 512 && buffer[dataLen] == 0)
        dataLen -= 1;

    return QString::fromWCharArray(buffer, dataLen);
}

