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

#include <thread>
#include <windows.h>

#include "pfRemoteConsole.h"
#include "pfMessage/pfRemoteConsoleMsg.h"
#include "plgDispatch.h"
#include "plStatusLog/plStatusLog.h"

#include <pfPython/cyPythonInterface.h>

void handlePipeServer();
void SendFullBuf(HANDLE pipe, const char* buf, size_t buflen);

pfRemoteConsole::~pfRemoteConsole()
{
    plgDispatch::Dispatch()->UnRegisterForExactType(pfRemoteConsoleMsg::Index(), GetKey());
}

void pfRemoteConsole::Init() {
    plStatusLog::AddLineSF("plasmadbg.log", "Starting pfRemoteConsole");
    fThread = std::thread(handlePipeServer);
    plgDispatch::Dispatch()->RegisterForExactType(pfRemoteConsoleMsg::Index(), GetKey());
}

bool    pfRemoteConsole::MsgReceive( plMessage *msg )
{
    plStatusLog::AddLineSF("plasmadbg.log", "plRemoteConsole got msg");
    pfRemoteConsoleMsg *cmd = pfRemoteConsoleMsg::ConvertNoRef( msg );
    if (cmd) {
        plStatusLog::AddLineSF("plasmadbg.log", "Msg is pfRemoteConsoleMsg");
        PyObject* mymod = PythonInterface::FindModule("__main__");
        PythonInterface::RunStringInteractive(cmd->GetCommand().c_str(), mymod);
        std::string output;
        // get the messages
        PythonInterface::getOutputAndReset(&output);
        cmd->GetOutput().get()->fOutputData = ST::string::from_std_string(output);
        {
            std::lock_guard<std::mutex> lk(cmd->GetOutput().get()->fOutputDataSetLock);
            cmd->GetOutput().get()->fOutputDataSet = true;
        }
        cmd->GetOutput().get()->fCondvar.notify_one();

        return true;
    }
    return hsKeyedObject::MsgReceive(msg);
}

bool canRun(ST::string& script) {
    return script.contains("\n");
}

void handlePipeServer() {
    SetThreadDescription(
        GetCurrentThread(),
        L"PipeServer!"
    );

    // First, initialize the socket
    DWORD pid = GetCurrentProcessId();
    ST::string pipename = ST::format(R"(\\.\pipe\URU-PYTHON-{})", pid);

    HANDLE pipe = CreateNamedPipeW(
        pipename.to_std_wstring().c_str(),
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
        1,
        // 1MiB
        1 * 1024 * 1024,
        1 * 1024 * 1024,
        0,
        nullptr
    );
    if (pipe == nullptr) {
        // TODO: Log an error.
        return;
    }

    if (!ConnectNamedPipe(pipe, nullptr) && GetLastError() != ERROR_PIPE_CONNECTED) {
        DWORD err = GetLastError();
        // TODO: Log an error
        CloseHandle(pipe);
        return;
    }


    char buf;

    ST::string curScript;

    SendFullBuf(pipe, ">>> ", 4);

    bool isMultiline = false;
    while (true) {
        DWORD readSize;
        if (!ReadFile(pipe, &buf, sizeof(buf), &readSize, nullptr)) {
            DWORD err = GetLastError();
            CloseHandle(pipe);
            // TODO: Log the error.
            return;
        }

        if (readSize == 0) {
            // TODO: Log that pipe is closed.
            CloseHandle(pipe);
            return;
        }

        if (buf != '\r') {
            SendFullBuf(pipe, &buf, 1);
        } else {
            SendFullBuf(pipe, "\r\n", 2);
        }

        // Turn our string into ST
        ST::string readData(&buf, readSize);

        readData = readData.replace("\r", "\n");
        curScript += readData;

        if (buf != '\r') {
            continue;
        }

        if (!isMultiline && curScript.ends_with(":\n")) {
            isMultiline = true;
            SendFullBuf(pipe, "... ", 4);
            continue;
        }

        if (isMultiline && !curScript.ends_with("\n\n")) {
            SendFullBuf(pipe, "... ", 4);
            continue;
        }

        // Send message.
        pfRemoteConsoleMsg *cMsg = new pfRemoteConsoleMsg(curScript);
        std::shared_ptr<pfRemoteConsoleMsgOutput> output(cMsg->GetOutput());

        cMsg->Send(nullptr, true);

        // Wait for answer.
        {
            std::unique_lock<std::mutex> lk(output->fOutputDataSetLock);
            output->fCondvar.wait(lk, [output] {return output->fOutputDataSet;});
        }

        ST::string finalOutput = output.get()->fOutputData.replace("\n", "\r\n");

        SendFullBuf(pipe, finalOutput.c_str(), finalOutput.size());
        curScript.clear();
        isMultiline = false;

        SendFullBuf(pipe, ">>> ", 4);
    }
}

void SendFullBuf(HANDLE pipe, const char* buf, size_t buflen) {
    const char* endbuf = buf + buflen;
    DWORD writeSize;
    while (buf < endbuf) {
        WriteFile(pipe, buf, endbuf - buf, &writeSize, nullptr);
        buf += writeSize;
    }
}