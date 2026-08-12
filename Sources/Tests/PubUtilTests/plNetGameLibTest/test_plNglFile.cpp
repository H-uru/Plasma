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

#include "pnNetProtocol/pnNpCli2File.h"

#include "plNetGameLib/Intern.h"
#include "plNetGameLib/plNglFile.h"

using namespace std::literals::string_view_literals;

// Can't put File2Cli_ManifestReply normally into a std::unique_ptr
// because of the variable-size array field,
// so we have to allocate it as a uint8_t[] and provide a custom deleter.
using ManifestReplyDeleter = std::function<void(File2Cli_ManifestReply*)>;
static ManifestReplyDeleter s_manifestReplyDeleter = [](auto ptr) {
    delete[] reinterpret_cast<uint8_t*>(ptr);
};

static std::unique_ptr<File2Cli_ManifestReply, ManifestReplyDeleter> IMakeManifestReply(uint32_t numFiles, std::u16string_view manifestData)
{
    size_t manifestOffset = offsetof(File2Cli_ManifestReply, manifestData);
    size_t manifestSize = manifestData.size() * sizeof(manifestData.front());
    size_t replySize = manifestOffset + manifestSize;
    uint8_t* replyData = new uint8_t[replySize];

    auto reply = reinterpret_cast<File2Cli_ManifestReply*>(replyData);
    reply->messageBytes = replySize;
    reply->messageId = kFile2Cli_ManifestReply;
    reply->transId = 42;
    reply->result = kNetSuccess;
    reply->readerId = 42;
    reply->numFiles = numFiles;
    reply->wcharCount = manifestData.size();
    memcpy(reply->manifestData, manifestData.data(), manifestSize);

    return std::unique_ptr<File2Cli_ManifestReply, ManifestReplyDeleter>(reply, s_manifestReplyDeleter);
}

TEST(plNglFile, IReceiveManifest_OneFile)
{
    std::vector<NetCliFileManifestEntry> manifest;
    unsigned numEntriesReceived = 0;

    auto reply = IMakeManifestReply(
        1,
        u"clientName\0downloadName\0d41d8cd98f00b204e9800998ecf8427e\0d1457b72c3fb323a2671125aef3eab5d\0\u0012\u3456\0\u0000\u789a\0\uffee\uccdd\0"sv
        u"\0"sv
    );
    EXPECT_TRUE(Ngl::File::IReceiveManifest(*reply, manifest, numEntriesReceived));
    EXPECT_EQ(numEntriesReceived, 1);
    EXPECT_EQ(manifest.size(), 1);

    EXPECT_EQ(std::u16string_view(manifest[0].clientName), u"clientName"sv);
    EXPECT_EQ(std::u16string_view(manifest[0].downloadName), u"downloadName"sv);
    EXPECT_EQ(std::u16string_view(manifest[0].md5, std::size(manifest[0].md5)), u"d41d8cd98f00b204e9800998ecf8427e"sv);
    EXPECT_EQ(std::u16string_view(manifest[0].md5compressed, std::size(manifest[0].md5compressed)), u"d1457b72c3fb323a2671125aef3eab5d"sv);
    EXPECT_EQ(manifest[0].fileSize, 0x123456);
    EXPECT_EQ(manifest[0].zipSize, 0x789a);
    EXPECT_EQ(manifest[0].flags, 0xffeeccdd);
}

TEST(plNglFile, IReceiveManifest_ThreeFiles)
{
    std::vector<NetCliFileManifestEntry> manifest;
    unsigned numEntriesReceived = 0;

    auto reply = IMakeManifestReply(
        3,
        u"client0\0download0\0ddddddddddddddddddddddddddddddd0\0ccccccccccccccccccccccccccccccc0\0\u0010\u0020\0\u0030\u0040\0\u0050\u0060\0"sv
        u"client1\0download1\0ddddddddddddddddddddddddddddddd1\0ccccccccccccccccccccccccccccccc1\0\u0011\u0021\0\u0031\u0041\0\u0051\u0061\0"sv
        u"client2\0download2\0ddddddddddddddddddddddddddddddd2\0ccccccccccccccccccccccccccccccc2\0\u0012\u0022\0\u0032\u0042\0\u0052\u0062\0"sv
        u"\0"sv
    );
    EXPECT_TRUE(Ngl::File::IReceiveManifest(*reply, manifest, numEntriesReceived));
    EXPECT_EQ(numEntriesReceived, 3);
    EXPECT_EQ(manifest.size(), 3);

    EXPECT_EQ(std::u16string_view(manifest[0].clientName), u"client0"sv);
    EXPECT_EQ(std::u16string_view(manifest[0].downloadName), u"download0"sv);
    EXPECT_EQ(std::u16string_view(manifest[0].md5, std::size(manifest[0].md5)), u"ddddddddddddddddddddddddddddddd0"sv);
    EXPECT_EQ(std::u16string_view(manifest[0].md5compressed, std::size(manifest[0].md5compressed)), u"ccccccccccccccccccccccccccccccc0"sv);
    EXPECT_EQ(manifest[0].fileSize, 0x100020);
    EXPECT_EQ(manifest[0].zipSize, 0x300040);
    EXPECT_EQ(manifest[0].flags, 0x500060);

    EXPECT_EQ(std::u16string_view(manifest[1].clientName), u"client1"sv);
    EXPECT_EQ(std::u16string_view(manifest[1].downloadName), u"download1"sv);
    EXPECT_EQ(std::u16string_view(manifest[1].md5, std::size(manifest[1].md5)), u"ddddddddddddddddddddddddddddddd1"sv);
    EXPECT_EQ(std::u16string_view(manifest[1].md5compressed, std::size(manifest[1].md5compressed)), u"ccccccccccccccccccccccccccccccc1"sv);
    EXPECT_EQ(manifest[1].fileSize, 0x110021);
    EXPECT_EQ(manifest[1].zipSize, 0x310041);
    EXPECT_EQ(manifest[1].flags, 0x510061);

    EXPECT_EQ(std::u16string_view(manifest[2].clientName), u"client2"sv);
    EXPECT_EQ(std::u16string_view(manifest[2].downloadName), u"download2"sv);
    EXPECT_EQ(std::u16string_view(manifest[2].md5, std::size(manifest[2].md5)), u"ddddddddddddddddddddddddddddddd2"sv);
    EXPECT_EQ(std::u16string_view(manifest[2].md5compressed, std::size(manifest[2].md5compressed)), u"ccccccccccccccccccccccccccccccc2"sv);
    EXPECT_EQ(manifest[2].fileSize, 0x120022);
    EXPECT_EQ(manifest[2].zipSize, 0x320042);
    EXPECT_EQ(manifest[2].flags, 0x520062);
}

TEST(plNglFile, IReceiveManifest_ThreeFilesChunked)
{
    std::vector<NetCliFileManifestEntry> manifest;
    unsigned numEntriesReceived = 0;

    auto reply_1 = IMakeManifestReply(
        3,
        u"client0\0download0\0ddddddddddddddddddddddddddddddd0\0ccccccccccccccccccccccccccccccc0\0\u0010\u0020\0\u0030\u0040\0\u0050\u0060\0"sv
        u"client1\0download1\0ddddddddddddddddddddddddddddddd1\0ccccccccccccccccccccccccccccccc1\0\u0011\u0021\0\u0031\u0041\0\u0051\u0061\0"sv
        u"\0"sv
    );
    EXPECT_TRUE(Ngl::File::IReceiveManifest(*reply_1, manifest, numEntriesReceived));
    EXPECT_EQ(numEntriesReceived, 2);
    EXPECT_EQ(manifest.size(), 3); // IReceiveManifest resizes the manifest vector ahead of time

    EXPECT_EQ(std::u16string_view(manifest[0].clientName), u"client0"sv);
    EXPECT_EQ(std::u16string_view(manifest[0].downloadName), u"download0"sv);
    EXPECT_EQ(std::u16string_view(manifest[0].md5, std::size(manifest[0].md5)), u"ddddddddddddddddddddddddddddddd0"sv);
    EXPECT_EQ(std::u16string_view(manifest[0].md5compressed, std::size(manifest[0].md5compressed)), u"ccccccccccccccccccccccccccccccc0"sv);
    EXPECT_EQ(manifest[0].fileSize, 0x100020);
    EXPECT_EQ(manifest[0].zipSize, 0x300040);
    EXPECT_EQ(manifest[0].flags, 0x500060);

    EXPECT_EQ(std::u16string_view(manifest[1].clientName), u"client1"sv);
    EXPECT_EQ(std::u16string_view(manifest[1].downloadName), u"download1"sv);
    EXPECT_EQ(std::u16string_view(manifest[1].md5, std::size(manifest[1].md5)), u"ddddddddddddddddddddddddddddddd1"sv);
    EXPECT_EQ(std::u16string_view(manifest[1].md5compressed, std::size(manifest[1].md5compressed)), u"ccccccccccccccccccccccccccccccc1"sv);
    EXPECT_EQ(manifest[1].fileSize, 0x110021);
    EXPECT_EQ(manifest[1].zipSize, 0x310041);
    EXPECT_EQ(manifest[1].flags, 0x510061);

    auto reply_2 = IMakeManifestReply(
        3,
        u"client2\0download2\0ddddddddddddddddddddddddddddddd2\0ccccccccccccccccccccccccccccccc2\0\u0012\u0022\0\u0032\u0042\0\u0052\u0062\0"sv
        u"\0"sv
    );
    EXPECT_TRUE(Ngl::File::IReceiveManifest(*reply_2, manifest, numEntriesReceived));
    EXPECT_EQ(numEntriesReceived, 3);
    EXPECT_EQ(manifest.size(), 3);

    EXPECT_EQ(std::u16string_view(manifest[2].clientName), u"client2"sv);
    EXPECT_EQ(std::u16string_view(manifest[2].downloadName), u"download2"sv);
    EXPECT_EQ(std::u16string_view(manifest[2].md5, std::size(manifest[2].md5)), u"ddddddddddddddddddddddddddddddd2"sv);
    EXPECT_EQ(std::u16string_view(manifest[2].md5compressed, std::size(manifest[2].md5compressed)), u"ccccccccccccccccccccccccccccccc2"sv);
    EXPECT_EQ(manifest[2].fileSize, 0x120022);
    EXPECT_EQ(manifest[2].zipSize, 0x320042);
    EXPECT_EQ(manifest[2].flags, 0x520062);
}
