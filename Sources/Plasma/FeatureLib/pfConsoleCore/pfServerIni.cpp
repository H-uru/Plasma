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

#include "pfServerIni.h"

#include <string_theory/codecs>
#include <string_theory/format>

#include "pfConsoleParser.h"

#include "pnNetBase/pnNetBase.h"

#include "plFile/plEncryptedStream.h"

static void ParseIntegerInto(const ST::string& value, unsigned int& out)
{
    ST::conversion_result res;
    out = value.to_uint(res);
    if (!res.ok() || !res.full_match()) {
        throw pfServerIniParseException(ST::format("Invalid integer value: {}", value));
    }
}

static void ParseBase64KeyInto(const ST::string& base64key, NetDhKey& output)
{
    ST_ssize_t base64len = ST::base64_decode(base64key, nullptr, 0);
    if (base64len < 0) {
        throw pfServerIniParseException("Invalid key: Invalid base-64 data (did you forget to put quotes around the value?)");
    } else if (sizeof(output) != base64len) {
        throw pfServerIniParseException(ST::format("Invalid key: Should be exactly {} bytes, not {}", sizeof(output), base64len));
    }

    ST_ssize_t bytes = ST::base64_decode(base64key, output, sizeof(output));
    if (bytes < 0) {
        throw pfServerIniParseException("Invalid key: Invalid base-64 data");
    }
}

void pfServerIni::IParseOption(const std::vector<ST::string>& name, const ST::string& value)
{
    if (name.empty() || name[0].compare_i("Server") != 0) {
        throw pfServerIniParseException("Unknown option name");
    }

    if (name.size() == 2 && name[1].compare_i("Status") == 0) {
        fStatusUrl = value;
    } else if (name.size() == 2 && name[1].compare_i("Signup") == 0) {
        fSignupUrl = value;
    } else if (name.size() == 2 && name[1].compare_i("DispName") == 0) {
        fDisplayName = value;
    } else if (name.size() == 2 && name[1].compare_i("Port") == 0) {
        ParseIntegerInto(value, fPort);
    } else if (name.size() == 3 && name[1].compare_i("File") == 0 && name[2].compare_i("Host") == 0) {
        fFileHostname = value;
    } else if (name.size() == 3 && name[1].compare_i("Auth") == 0) {
        if (name[2].compare_i("Host") == 0) {
            fAuthHostname = value;
        } else if (name[2].compare_i("N") == 0) {
            ParseBase64KeyInto(value, fAuthDhConstants.n);
        } else if (name[2].compare_i("X") == 0) {
            ParseBase64KeyInto(value, fAuthDhConstants.x);
        } else if (name[2].compare_i("G") == 0) {
            ParseIntegerInto(value, fAuthDhConstants.g);
        } else {
            throw pfServerIniParseException("Unknown option name");
        }
    } else if (name.size() == 3 && name[1].compare_i("Game") == 0) {
        if (name[2].compare_i("N") == 0) {
            ParseBase64KeyInto(value, fGameDhConstants.n);
        } else if (name[2].compare_i("X") == 0) {
            ParseBase64KeyInto(value, fGameDhConstants.x);
        } else if (name[2].compare_i("G") == 0) {
            ParseIntegerInto(value, fGameDhConstants.g);
        } else {
            throw pfServerIniParseException("Unknown option name");
        }
    } else if (name.size() == 3 && name[1].compare_i("Gate") == 0) {
        if (name[2].compare_i("Host") == 0) {
            fGateKeeperHostname = value;
        } else if (name[2].compare_i("N") == 0) {
            ParseBase64KeyInto(value, fGateKeeperDhConstants.n);
        } else if (name[2].compare_i("X") == 0) {
            ParseBase64KeyInto(value, fGateKeeperDhConstants.x);
        } else if (name[2].compare_i("G") == 0) {
            ParseIntegerInto(value, fGateKeeperDhConstants.g);
        } else {
            throw pfServerIniParseException("Unknown option name");
        }
    } else {
        throw pfServerIniParseException("Unknown option name");
    }
}

void pfServerIni::Parse(const plFileName& fileName)
{
    std::unique_ptr<hsStream> stream = plEncryptedStream::OpenEncryptedFile(fileName);
    if (!stream) {
        throw pfServerIniParseException(ST::format("File not found or could not be opened: {}", fileName));
    }

    ST::string line;
    for (int lineno = 1; stream->ReadLn(line); lineno++) {
        // Note: comments are already handled by hsStream::ReadLn.
        pfConsoleParser parser(line);
        auto name = parser.ParseUnknownCommandName();
        if (name.empty()) {
            if (!parser.GetErrorMsg().empty()) {
                throw pfServerIniParseException(ST::format("Line {}: {}", lineno, parser.GetErrorMsg()));
            } else {
                // No error, just an empty line.
                continue;
            }
        }

        auto args = parser.ParseArguments();
        if (!args) {
            throw pfServerIniParseException(ST::format("Line {}: {}", lineno, parser.GetErrorMsg()));
        }

        // TODO Warn/error if there isn't exactly one argument after the name?
        ST::string value = args->empty() ? ST::string() : std::move((*args)[0]);
        IParseOption(name, value);
    }
}

void pfServerIni::Apply()
{
    SetServerStatusUrl(fStatusUrl);
    SetServerSignupUrl(fSignupUrl);
    SetServerDisplayName(fDisplayName);
    SetClientPort(fPort);
    SetFileSrvHostname(fFileHostname);
    SetAuthSrvHostname(fAuthHostname);
    gNetAuthDhConstants = fAuthDhConstants;
    gNetGameDhConstants = fGameDhConstants;
    SetGateKeeperSrvHostname(fGateKeeperHostname);
    gNetGateKeeperDhConstants = fGateKeeperDhConstants;
}

void pfServerIni::Load(const plFileName& fileName)
{
    pfServerIni ini;
    ini.Parse(fileName);
    ini.Apply();
}
