/*==LICENSE==*

CyanWorlds.com Engine - MMOG client, server and tools
Copyright (C) 2011 Cyan Worlds, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.

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

#include <functional>
#include <gtest/gtest.h>
#include <string_view>
#include <vector>

#include "hsEndian.h"

#include "pnNetProtocol/pnNpCli2Auth.h"

#include "plNetGameLib/Intern.h"
#include "plNetGameLib/plNglAuth.h"

using namespace std::literals::string_view_literals;

// Can't put Auth2Cli_FileListReply normally into a std::unique_ptr
// because of the variable-size array field,
// so we have to allocate it as a uint8_t[] and provide a custom deleter.
using FileListReplyDeleter = std::function<void(Auth2Cli_FileListReply*)>;
static FileListReplyDeleter s_fileListReplyDeleter = [](auto ptr) {
    delete[] reinterpret_cast<uint8_t*>(ptr);
};

static std::unique_ptr<Auth2Cli_FileListReply, FileListReplyDeleter> IMakeFileListReply(std::u16string_view fileData)
{
    size_t fileDataOffset = offsetof(Auth2Cli_FileListReply, fileData);
    size_t fileDataSize = fileData.size() * sizeof(fileData.front());
    size_t replySize = fileDataOffset + fileDataSize;
    uint8_t* replyData = new uint8_t[replySize];

    auto reply = reinterpret_cast<Auth2Cli_FileListReply*>(replyData);
    reply->messageId = kAuth2Cli_FileListReply;
    reply->transId = 42;
    reply->result = kNetSuccess;
    reply->wcharCount = fileData.size();

    for (size_t i = 0; i < fileData.size(); i++) {
        reply->fileData[i] = hsToLE16(fileData[i]);
    }

    return std::unique_ptr<Auth2Cli_FileListReply, FileListReplyDeleter>(reply, s_fileListReplyDeleter);
}

TEST(plNglAuth, IReceiveFileList_OneFile)
{
    std::vector<NetCliAuthFileInfo> fileInfoArray;

    auto reply = IMakeFileListReply(
        u"filename\0\u0012\u3456\0"sv
        u"\0"sv
    );
    EXPECT_TRUE(Ngl::Auth::IReceiveFileList(*reply, fileInfoArray));
    EXPECT_EQ(fileInfoArray.size(), 1);

    EXPECT_EQ(std::u16string_view(fileInfoArray[0].filename), u"filename"sv);
    EXPECT_EQ(fileInfoArray[0].filesize, 0x123456);
}

TEST(plNglAuth, IReceiveFileList_ThreeFiles)
{
    std::vector<NetCliAuthFileInfo> fileInfoArray;

    auto reply = IMakeFileListReply(
        u"filename0\0\u0010\u0020\0"sv
        u"filename1\0\u0011\u0021\0"sv
        u"filename2\0\u0012\u0022\0"sv
        u"\0"sv
    );
    EXPECT_TRUE(Ngl::Auth::IReceiveFileList(*reply, fileInfoArray));
    EXPECT_EQ(fileInfoArray.size(), 3);

    EXPECT_EQ(std::u16string_view(fileInfoArray[0].filename), u"filename0"sv);
    EXPECT_EQ(fileInfoArray[0].filesize, 0x100020);

    EXPECT_EQ(std::u16string_view(fileInfoArray[1].filename), u"filename1"sv);
    EXPECT_EQ(fileInfoArray[1].filesize, 0x110021);

    EXPECT_EQ(std::u16string_view(fileInfoArray[2].filename), u"filename2"sv);
    EXPECT_EQ(fileInfoArray[2].filesize, 0x120022);
}
