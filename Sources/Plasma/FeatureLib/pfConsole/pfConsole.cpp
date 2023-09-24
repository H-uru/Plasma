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
//                                                                          //
//  pfConsole Functions                                                     //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "pfConsole.h"

#include "HeadSpin.h"
#include "plgDispatch.h"
#include "plFileSystem.h"
#include "plPipeline.h"
#include "plProduct.h"
#include "hsTimer.h"

#include <set>
#include <string_theory/format>
#include <string_theory/string>
#include <vector>

#include "pnInputCore/plKeyDef.h"
#include "pnInputCore/plKeyMap.h"
#include "pnKeyedObject/plFixedKey.h"
#include "pnNetCommon/plNetApp.h"

#include "plClipboard/plClipboard.h"
#include "plGImage/plPNG.h"
#include "plInputCore/plInputDevice.h"
#include "plInputCore/plInputInterface.h"
#include "plInputCore/plInputInterfaceMgr.h"
#include "plMessage/plCaptureRenderMsg.h"
#include "plMessage/plConfirmationMsg.h"
#include "plMessage/plConsoleMsg.h"
#include "plMessage/plInputEventMsg.h"
#include "plMessage/plInputIfaceMgrMsg.h"
#include "plMessageBox/hsMessageBox.h"
#include "plPipeline/plDebugText.h"

#include "pfConsoleCore/pfConsoleEngine.h"
#include "pfPython/cyPythonInterface.h"


//// Static Class Stuff //////////////////////////////////////////////////////

pfConsole   *pfConsole::fTheConsole = nullptr;
uint32_t      pfConsole::fConsoleTextColor = 0xff00ff00;
plPipeline  *pfConsole::fPipeline = nullptr;


//////////////////////////////////////////////////////////////////////////////
//// pfConsoleInputInterface - Input interface layer for the console /////////
//////////////////////////////////////////////////////////////////////////////

class pfConsoleInputInterface : public plInputInterface
{
    protected:

        pfConsole   *fConsole;



        bool    IHandleCtrlCmd(plCtrlCmd *cmd) override
        {
            if( cmd->fControlCode == B_SET_CONSOLE_MODE )
            {
                if( cmd->fControlActivated )
                {
                    // Activate/deactivate
                    switch( fConsole->fMode )
                    {
                        case pfConsole::kModeHidden:
                            fConsole->ISetMode( pfConsole::kModeSingleLine );
                            break;
                        case pfConsole::kModeSingleLine:
                            fConsole->ISetMode( pfConsole::kModeFull );
                            break;
                        case pfConsole::kModeFull:
                            fConsole->ISetMode( pfConsole::kModeHidden );
                            break;
                    }
                }
                return true;
            }
            return false;
        }

    public:

        pfConsoleInputInterface( pfConsole *console )
        {
            fConsole = console; 
            SetEnabled( true );         // Always enabled

            // Add our control codes to our control map. Do NOT add the key bindings yet.
            // Note: HERE is where you specify the actions for each command, i.e. net propagate and so forth.
            // This part basically declares us master of the bindings for these commands.
            
            // IF YOU ARE LOOKING TO CHANGE THE DEFAULT KEY BINDINGS, DO NOT LOOK HERE. GO TO
            // RestoreDefaultKeyMappings()!!!!

#ifndef PLASMA_EXTERNAL_RELEASE
            fControlMap->AddCode( B_SET_CONSOLE_MODE, kControlFlagNormal | kControlFlagNoRepeat );
#endif

            // IF YOU ARE LOOKING TO CHANGE THE DEFAULT KEY BINDINGS, DO NOT LOOK HERE. GO TO
            // RestoreDefaultKeyMappings()!!!!
        }

        uint32_t  GetPriorityLevel() const override { return kConsolePriority; }
        uint32_t  GetCurrentCursorID() const override { return kCursorHidden; }
        bool    HasInterestingCursorID() const override { return false; }

        bool    InterpretInputEvent(plInputEventMsg *pMsg) override
        {
            plKeyEventMsg   *keyMsg = plKeyEventMsg::ConvertNoRef( pMsg );
            if (keyMsg != nullptr)
            {
                if( fConsole->fMode )
                {
                    fConsole->IHandleKey( keyMsg );
                    return true;
                }
            }

            return false;
        }

        void    RefreshKeyMap() override
        {
        }

        void    RestoreDefaultKeyMappings() override
        {
            if (fControlMap != nullptr)
            {
                fControlMap->UnmapAllBindings();
#ifndef PLASMA_EXTERNAL_RELEASE
                fControlMap->BindKey( KEY_TILDE, B_SET_CONSOLE_MODE );
#endif
            }
        }
};

//// Constructor & Destructor ////////////////////////////////////////////////

pfConsole::pfConsole()
    : fNumDisplayLines(32), fDisplayBuffer(), fFXEnabled(true), fEffectCounter(), fLastTime(),
      fHelpTimer(), fMode(), fInited(), fHelpMode(), fCursorTicks(), fMsgTimeoutTimer(),
      fPythonMode(), fPythonFirstTime(true), fPythonMultiLines(), fHistory(), fWorkingCursor(),
      fInputInterface(), fEngine()
{
    fTheConsole = this;
}

pfConsole::~pfConsole()
{
    if (fInputInterface != nullptr)
    {
        plInputIfaceMgrMsg *msg = new plInputIfaceMgrMsg( plInputIfaceMgrMsg::kRemoveInterface );
        msg->SetIFace( fInputInterface );
        plgDispatch::MsgSend( msg );

        hsRefCnt_SafeUnRef( fInputInterface );
        fInputInterface = nullptr;
    }

    if (fDisplayBuffer != nullptr)
    {
        delete [] fDisplayBuffer;
        fDisplayBuffer = nullptr;
    }

    fTheConsole = nullptr;

    plgDispatch::Dispatch()->UnRegisterForExactType( plConsoleMsg::Index(), GetKey() );
    plgDispatch::Dispatch()->UnRegisterForExactType( plControlEventMsg::Index(), GetKey() );
}

pfConsole * pfConsole::GetInstance () {

    return fTheConsole;
}

ST::string pfConsole::IGetWorkingLine()
{
    // FIXME: Support more Unicode than just Latin-1
    return ST::string::from_latin_1(fWorkingLine);
}

void pfConsole::ISetWorkingLine(const ST::string& workingLine)
{
    // FIXME: Support more Unicode than just Latin-1
    ST::char_buffer buf = workingLine.to_latin_1();
    size_t size;
    if (buf.size() >= sizeof(fWorkingLine)) {
        size = sizeof(fWorkingLine);
        buf[size - 1] = 0;
    } else {
        size = buf.size() + 1;
    }
    hsAssert(size <= sizeof(fWorkingLine), "Miscalculated size of buffer for working line!");
    memcpy(fWorkingLine, buf.c_str(), size);
    fWorkingCursor = size - 1;
}

void pfConsole::IClearWorkingLine()
{
    fWorkingLine[0] = 0;
    fWorkingCursor = 0;
}

//// Init ////////////////////////////////////////////////////////////////////

void    pfConsole::Init( pfConsoleEngine *engine )
{
    fDisplayBuffer = new char[ fNumDisplayLines * kMaxCharsWide ];
    memset( fDisplayBuffer, 0, fNumDisplayLines * kMaxCharsWide );

    IClearWorkingLine();

    for (size_t i = 0; i < kNumHistoryTypes; i++) {
        fHistory[i] = {};
    }

    fEffectCounter = 0;
    fMode = 0;
    fMsgTimeoutTimer = 0;
    fHelpMode = false;
    fPythonMode = false;
    fPythonFirstTime = true;
    fPythonMultiLines = 0;
    fHelpTimer = 0;
    fCursorTicks = 0;
    fLastHelpMsg.clear();
    fEngine = engine;

    fInputInterface = new pfConsoleInputInterface( this );
    plInputIfaceMgrMsg *msg = new plInputIfaceMgrMsg( plInputIfaceMgrMsg::kAddInterface );
    msg->SetIFace( fInputInterface );
    plgDispatch::MsgSend( msg );

    // Register for keyboard event messages
    plgDispatch::Dispatch()->RegisterForExactType( plConsoleMsg::Index(), GetKey() );
    plgDispatch::Dispatch()->RegisterForExactType( plControlEventMsg::Index(), GetKey() );
}

//// ISetMode ////////////////////////////////////////////////////////////////

void    pfConsole::ISetMode( uint8_t mode )
{
    fMode = mode; 
    fEffectCounter = ( fFXEnabled ? kEffectDivisions : 0 ); 
    fMsgTimeoutTimer = 0; 
    fInputInterface->RefreshKeyMap();
}

//// MsgReceive //////////////////////////////////////////////////////////////

bool    pfConsole::MsgReceive( plMessage *msg )
{
    // Handle screenshot saving...
    plCaptureRenderMsg* capMsg = plCaptureRenderMsg::ConvertNoRef(msg);
    if (capMsg) {
        plFileName screenshots = plFileName::Join(plFileSystem::GetUserDataPath(), "Screenshots");
        plFileSystem::CreateDir(screenshots, false); // just in case...
        ST::string prefix = plProduct::ShortName();

        // List all of the PNG indices we have taken up already...
        ST::string pattern = ST::format("{}*.png", prefix);
        std::vector<plFileName> images = plFileSystem::ListDir(screenshots, pattern.c_str());
        std::set<uint32_t> indices;
        for (const auto& fn : images) {
            ST::string idx = fn.GetFileNameNoExt().substr(prefix.size());
            indices.insert(idx.to_uint(10));
        }

        // Now that we have an ordered set of indices, save this screenshot to the first one we don't have.
        uint32_t num = 0;
        for (auto it = indices.begin(); it != indices.end(); ++it, ++num) {
            if (*it != num)
                break;
        }

        // Got our num, save the screenshot.
        plFileName fn = ST::format("{}{04}.png", prefix, num);
        plPNG::Instance().WriteToFile(plFileName::Join(screenshots, fn), capMsg->GetMipmap());

        AddLine(ST::format("Saved screenshot as '{}'", fn.AsString()));
        return true;
    }

    plControlEventMsg *ctrlMsg = plControlEventMsg::ConvertNoRef( msg );
    if (ctrlMsg != nullptr)
    {
        if( ctrlMsg->ControlActivated() && ctrlMsg->GetControlCode() == B_CONTROL_CONSOLE_COMMAND && plNetClientApp::GetInstance()->GetFlagsBit(plNetClientApp::kPlayingGame))
        {
            fEngine->RunCommand(ctrlMsg->GetCmdString(), IAddLineCallback);
            return true;
        }
        return false;
    }

    plConsoleMsg *cmd = plConsoleMsg::ConvertNoRef( msg );
    if (cmd != nullptr && !cmd->GetString().empty())
    {
        if( cmd->GetCmd() == plConsoleMsg::kExecuteFile )
        {
            if( !fEngine->ExecuteFile( cmd->GetString() ) )
            {
                // Change the following line once we have a better way of reporting
                // errors in the parsing
                ST::string str = ST::format("Error parsing {}", cmd->GetString());
                ST::string msg = ST::format("{}:\n\nCommand: '{}'\n{}", fEngine->GetErrorMsg(), fEngine->GetLastErrorLine(),
#ifdef HS_DEBUGGING
                        "" );

                hsAssert( false, msg.c_str() );
#else
                        "\nPress OK to continue parsing files." );

                hsMessageBox(msg, str, hsMessageBoxNormal);
#endif
            }
        }
        else if( cmd->GetCmd() == plConsoleMsg::kAddLine )
            IAddParagraph(cmd->GetString());
        else if( cmd->GetCmd() == plConsoleMsg::kExecuteLine )
        {
            if( !fEngine->RunCommand(cmd->GetString(), IAddLineCallback))
            {
                // Change the following line once we have a better way of reporting
                // errors in the parsing
                AddLine(ST::format("{}:\n\nCommand: '{}'\n", fEngine->GetErrorMsg(), fEngine->GetLastErrorLine()));
            }
        }
            
        return true;
    }

    return hsKeyedObject::MsgReceive(msg);
}

//// IHandleKey //////////////////////////////////////////////////////////////

void    pfConsole::IHandleKey( plKeyEventMsg *msg )
{
    char            *c;
    wchar_t         key;
    size_t          i,eol;
    static bool     findAgain = false;
    static uint32_t   findCounter = 0;

    // filter out keyUps and ascii control characters
    // as the control functions are handled on the keyDown event
    if( !msg->GetKeyDown() || (msg->GetKeyChar() > '\0' && msg->GetKeyChar() < ' '))
        return;

    if( msg->GetKeyCode() == KEY_ESCAPE )
    {
        IClearWorkingLine();
        findAgain = false;
        findCounter = 0;
        fHelpMode = false;
        fPythonMode = false;
        fPythonMultiLines = 0;
        IUpdateTooltip();
    }
    else if( msg->GetKeyCode() == KEY_TAB )
    {
        if ( fPythonMode )
        {
            // if we are in Python mode, then just add two spaces, tab over, sorta
            if ( strlen(fWorkingLine) < kWorkingLineSize+2 )
            {
                strcat(&fWorkingLine[fWorkingCursor], "  ");
                fWorkingCursor += 2;
            }
        }
        else
        {
            static ST::string lastSearch;

            if( !findAgain && findCounter == 0 )
                lastSearch = IGetWorkingLine();

            ST::string found;
            if (findCounter > 0) {
                // Not found the normal way; try using an unrestricted search
                found = fEngine->FindNestedPartialCmd(lastSearch, findCounter, true);
                if (!found.empty()) {
                    findCounter++;
                } else {
                    /// Try starting over...?
                    findCounter = 0;
                    found = fEngine->FindNestedPartialCmd(lastSearch, findCounter, true);
                    if (!found.empty()) {
                        findCounter++;
                    }
                }
            } else {
                found = fEngine->FindPartialCmd(lastSearch, findAgain, true);
                if (!found.empty()) {
                    findAgain = true;
                } else if (findAgain) {
                    /// Try starting over
                    found = fEngine->FindPartialCmd(lastSearch, findAgain = false, true);
                    if (!found.empty()) {
                        findAgain = true;
                    }
                } else {
                    // Not found the normal way; start an unrestricted search
                    found = fEngine->FindNestedPartialCmd(lastSearch, findCounter, true);
                    if (!found.empty()) {
                        findCounter++;
                    }
                }
            }

            if (!found.empty()) {
                ISetWorkingLine(found);
            }
            IUpdateTooltip();
        }
    }
    else if( msg->GetKeyCode() == KEY_UP )
    {
        i = ( fHistory[ fPythonMode ].fRecallCursor > 0 ) ? fHistory[ fPythonMode ].fRecallCursor - 1 : kNumHistoryItems - 1;
        if (!fHistory[fPythonMode].fData[i].empty())
        {
            fHistory[ fPythonMode ].fRecallCursor = i;
            ISetWorkingLine(fHistory[fPythonMode].fData[fHistory[fPythonMode].fRecallCursor]);
            findAgain = false;
            findCounter = 0;
            IUpdateTooltip();
        }
    }
    else if( msg->GetKeyCode() == KEY_DOWN )
    {
        if( fHistory[ fPythonMode ].fRecallCursor != fHistory[ fPythonMode ].fCursor )
        {
            i = ( fHistory[ fPythonMode ].fRecallCursor < kNumHistoryItems - 1 ) ? fHistory[ fPythonMode ].fRecallCursor + 1 : 0;
            if( i != fHistory[ fPythonMode ].fCursor )
            {
                fHistory[ fPythonMode ].fRecallCursor = i;
                ISetWorkingLine(fHistory[fPythonMode].fData[fHistory[fPythonMode].fRecallCursor]);
            }
            else
            {
                IClearWorkingLine();
                fHistory[ fPythonMode ].fRecallCursor = fHistory[ fPythonMode ].fCursor;
            }
            findAgain = false;
            findCounter = 0;
            IUpdateTooltip();
        }
    }
    else if( msg->GetKeyCode() == KEY_LEFT )
    {
        if( fWorkingCursor > 0 )
            fWorkingCursor--;
    }
    else if( msg->GetKeyCode() == KEY_RIGHT )
    {
        if( fWorkingCursor < strlen( fWorkingLine ) )
            fWorkingCursor++;
    }
    else if( msg->GetKeyCode() == KEY_BACKSPACE )
    {
        if( fWorkingCursor > 0 )
        {
            fWorkingCursor--;

            c = &fWorkingLine[ fWorkingCursor ];
            do
            {
                *c = *( c + 1 );
                c++;
            } while( *c != 0 );

            findAgain = false;
            findCounter = 0;
        }
        else if( fHelpMode )
            fHelpMode = false;

        IUpdateTooltip();
    }
    else if( msg->GetKeyCode() == KEY_DELETE )
    {
        c = &fWorkingLine[ fWorkingCursor ];
        do
        {
            *c = *( c + 1 );
            c++;
        } while( *c != 0 );

        findAgain = false;
        findCounter = 0;
        IUpdateTooltip();
    }
    else if( msg->GetKeyCode() == KEY_ENTER )
    {
        IExecuteWorkingLine();
        findAgain = false;
        findCounter = 0;
        IUpdateTooltip();
    }
    else if( msg->GetKeyCode() == KEY_END )
    {
        fWorkingCursor = strnlen(fWorkingLine, std::size(fWorkingLine));
    }
    else if( msg->GetKeyCode() == KEY_HOME )
    {
        fWorkingCursor = 0;
    }
    else if (msg->GetCtrlKeyDown() && msg->GetKeyCode() == KEY_C)
    {
        plClipboard::GetInstance().SetClipboardText(IGetWorkingLine());
    }
    else if (msg->GetCtrlKeyDown() && msg->GetKeyCode() == KEY_X)
    {
        plClipboard::GetInstance().SetClipboardText(IGetWorkingLine());
        IClearWorkingLine();
        findAgain = false;
        findCounter = 0;
        IUpdateTooltip();
    }
    else if (msg->GetCtrlKeyDown() && msg->GetKeyCode() == KEY_V)
    {
        ST::string text = plClipboard::GetInstance().GetClipboardText();

        // Chop off leading or trailing newlines, which are probably not intended.
        text = text.trim("\r\n");
        if (text.empty())
            return;

        // FIXME: Unfortunately, the console is currently limited to single byte
        //        characters. Remove this when the console is unicode safe.
        ST::char_buffer buf = text.to_latin_1();

        // If there are any embedded newlines, we will need to execute the commands
        // or Python code, otherwise we will get a mess in the working line. However,
        // executing arbitrary commands without warning is not ideal, so ask the user
        // if this is OK.
        if (text.find('\n') != -1) {
            plConfirmationMsg* yesno = new plConfirmationMsg(
                ST_LITERAL(
                    "You are pasting text that contains multiple lines. "
                    "This may cause commands to execute. Should we continue?"
                ),
                plConfirmationMsg::Type::YesNo,
                [this, buf = std::move(buf)](plConfirmationMsg::Result result) -> void {
                    if (result == plConfirmationMsg::Result::Yes) {
                        for (const char c : buf)
                            IHandleCharacter(c);
                    }
                }
            );
            yesno->Send();
        } else {
            for (const char c : buf)
                IHandleCharacter(c);
        }

        findAgain = false;
        findCounter = 0;
        IUpdateTooltip();
    }
    else if (msg->GetKeyChar())
    {
        key = msg->GetKeyChar();
        // do they want to go into help mode?
        if( !fPythonMode && key == L'?' && fWorkingCursor == 0 )
        {
            /// Go into help mode
            fHelpMode = true;
        }
        // do they want to go into Python mode?
        else if( !fHelpMode && key == L'\\' && fWorkingCursor == 0 )
        {
            // toggle Python mode
            fPythonMode = fPythonMode ? false:true;
            if ( fPythonMode )
            {
                if ( fPythonFirstTime )
                {
                    IAddLine( "" );     // add a blank line
                    PythonInterface::RunStringInteractive("import sys;print(f'Python {sys.version}')", nullptr);
                    // get the messages
                    AddLine(PythonInterface::getOutputAndReset());
                    fPythonFirstTime = false;       // do this only once!
                }
            }
        }
        // or are they just typing in a working line
        else
        {
            // FIXME: make the console unicode friendly.
            IHandleCharacter((char)key);
            if (strnlen(fWorkingLine, std::size(fWorkingLine)) < kMaxCharsWide - 2 && key != 0) {
                findAgain = false;
                findCounter = 0;
                IUpdateTooltip();
            }
        }
    }
}

void    pfConsole::IHandleCharacter(const char c)
{
    // Properly handle embedded newlines as execution triggers.
    if (c == '\r')
        return;
    if (c == '\n') {
        IExecuteWorkingLine();
        return;
    }

    size_t i = strnlen(fWorkingLine, std::size(fWorkingLine));

    // The current working line is too long. Ignore this character.
    if (i >= kMaxCharsWide - 2 || c == '\0')
        return;

    // Advance any trailing characters one space to make room for the new character.
    for (i += 1; i > fWorkingCursor; --i)
        fWorkingLine[i] = fWorkingLine[i - 1];
    fWorkingLine[fWorkingCursor++] = c;
}

//// IAddLineCallback ////////////////////////////////////////////////////////

void pfConsole::IAddLineCallback(const ST::string& string)
{
    fTheConsole->IAddParagraph( string, 0 );
}

//// IAddLine ////////////////////////////////////////////////////////////////

void    pfConsole::IAddLine( const char *string, short leftMargin )
{
    int         i;
    char        *ptr;


    /// Advance upward
    for( i = 0, ptr = fDisplayBuffer; i < fNumDisplayLines - 1; i++ )
    {
        memcpy( ptr, ptr + kMaxCharsWide, kMaxCharsWide );
        ptr += kMaxCharsWide;
    }

    if( string[ 0 ] == '\t' )
    {
        leftMargin += 4;
        string++;
    }

    memset( ptr, 0, kMaxCharsWide );
    memset( ptr, ' ', leftMargin );
    strncpy( ptr + leftMargin, string, kMaxCharsWide - leftMargin - 1 );
    if( fMode == 0 )
    {
        /// Console is invisible, so show this line for a bit of time
        fMsgTimeoutTimer = kMsgHintTimeout;
    }
}

//// IAddParagraph ///////////////////////////////////////////////////////////

void pfConsole::IAddParagraph(const ST::string& s, short margin)
{
    ST::char_buffer buf = s.to_latin_1();
    char* string = buf.data();

    // Special character: if \i is in front of the string, indent it
    while( strncmp( string, "\\i", 2 ) == 0 )
    {
        margin += 3;
        string += 2;
    }

    for (char* ptr = string; ptr != nullptr && *ptr != 0; )
    {
        // Go as far as possible
        char* ptr2;
        if( strlen( ptr ) < kMaxCharsWide - margin - margin - 1 )
            ptr2 = ptr + strlen( ptr );
        else
        {
            // Back up until we hit a sep
            ptr2 = ptr + kMaxCharsWide - margin - margin - 1;
            for( ; ptr2 > string && *ptr2 != ' ' && *ptr2 != '\t' && *ptr2 != '\n'; ptr2-- );
        }

        // Check for carriage return
        char* ptr3 = strchr( ptr, '\n' );
        if( ptr3 == ptr )
        {
            IAddLine( "", margin );
            ptr++;
            continue;
        }
        if (ptr3 != nullptr && ptr3 < ptr2)
            ptr2 = ptr3;

        // Add this part
        if( ptr2 == ptr || *ptr2 == 0 )
        {
            IAddLine( ptr, margin );
            break;
        }

        *ptr2 = 0;
        IAddLine( ptr, margin );
        ptr = ptr2 + 1;
    }
}

//// IClear //////////////////////////////////////////////////////////////////

void    pfConsole::IClear()
{
    memset( fDisplayBuffer, 0, kMaxCharsWide * fNumDisplayLines );
}

//// Draw ////////////////////////////////////////////////////////////////////

void    pfConsole::Draw( plPipeline *p )
{
    int         i, yOff, y, x, eOffset, height;
    bool        showTooltip = false;
    float       thisTime;   // For making the console FX speed konstant regardless of framerate
    const float kEffectDuration = 0.5f;


    plDebugText&    drawText = plDebugText::Instance();

    thisTime = hsTimer::GetSeconds<float>();

    if( fMode == kModeHidden && fEffectCounter == 0 )
    {
        if( fMsgTimeoutTimer > 0 )
        {
            /// Message hint--draw the last line of the console for a bit
            drawText.DrawString(10, 4, ST::string::from_latin_1(fDisplayBuffer + kMaxCharsWide * (fNumDisplayLines - 1)), fConsoleTextColor);
            fMsgTimeoutTimer--;
        }
        fLastTime = thisTime;
        return;
    }

    drawText.SetDrawOnTopMode( true );

    yOff = drawText.GetFontHeight() + 2;
    if( fMode == kModeSingleLine )
        height = yOff * 3 + 14;
    else
        height = yOff * ( fNumDisplayLines + 2 ) + 14;

    if (fHelpTimer == 0 && !fHelpMode && !fLastHelpMsg.empty())
        showTooltip = true;
    
    if( fEffectCounter > 0 )
    {
        int numElapsed = (int)( (float)kEffectDivisions * ( ( thisTime - fLastTime ) / (float)kEffectDuration ) );
        if( numElapsed > fEffectCounter )
            numElapsed = fEffectCounter;
        else if( numElapsed < 0 )
            numElapsed = 0;

        if( fMode == kModeSingleLine )
            eOffset = fEffectCounter * height / kEffectDivisions;
        else if( fMode == kModeFull )
            eOffset = fEffectCounter * ( height - yOff * 3 - 14 ) / kEffectDivisions;
        else
            eOffset = ( kEffectDivisions - fEffectCounter ) * ( height - yOff * 3 - 14 ) / kEffectDivisions;
        fEffectCounter -= numElapsed;
    }
    else 
        eOffset = 0;
    fLastTime = thisTime;

    if( fMode == kModeSingleLine )
    {
        // Bgnd (TEMP ONLY)
        x = kMaxCharsWide * drawText.CalcStringWidth(ST_LITERAL("W")) + 4;
        y = height - eOffset;
        drawText.DrawRect( 4, 0, x, y, /*color*/0, 0, 0, 127 );

        /// Actual text
        if( fEffectCounter == 0 )
            drawText.DrawString(10, 4, ST_LITERAL("Plasma 2.0 Console"), 255, 255, 255, 255);

        if( !showTooltip )
            drawText.DrawString(10, 4 + yOff - eOffset, ST::string::from_latin_1(fDisplayBuffer + kMaxCharsWide * (fNumDisplayLines - 1)), fConsoleTextColor);

        y = 4 + yOff + yOff - eOffset;
    }
    else
    {
        // Bgnd (TEMP ONLY)
        x = kMaxCharsWide * drawText.CalcStringWidth(ST_LITERAL("W")) + 4;
        y = yOff * ( fNumDisplayLines + 2 ) + 14 - eOffset;
        drawText.DrawRect( 4, 0, x, y, /*color*/0, 0, 0, 127 );

        /// Actual text
        drawText.DrawString(10, 4, ST_LITERAL("Plasma 2.0 Console"), 255, 255, 255, 255);

        static int  countDown = 3000;
        if( fHelpTimer > 0 || fEffectCounter > 0 || fMode != kModeFull )
            countDown = 3000;
        else if( countDown > -720 )
            countDown--;

        // Resource data is encrypted so testers can't peer in to the EXE, plz don't decrypt
        static bool rezLoaded = false;
        static char tmpSrc[ kMaxCharsWide ];
        if( !rezLoaded )
        {
            memset( tmpSrc, 0, sizeof( tmpSrc ) );
            // Our concession to windows
#ifdef HS_BUILD_FOR_WIN32
            #include "../../Apps/plClient/win32/res/resource.h"
            HRSRC rsrc = FindResource(nullptr, MAKEINTRESOURCE(IDR_CNSL1), "CNSL");
            if (rsrc != nullptr)
            {
                HGLOBAL hdl = LoadResource(nullptr, rsrc);
                if (hdl != nullptr)
                {
                    uint8_t *ptr = (uint8_t *)LockResource( hdl );
                    if (ptr != nullptr)
                    {
                        for (i = 0; i < SizeofResource(nullptr, rsrc); i++)
                            tmpSrc[ i ] = ptr[ i ] + 26;
                        UnlockResource( hdl );
                    }
                }
            }
            rezLoaded = true;
#else
            // Need to define for other platforms?
#endif
        }

        if( countDown <= 0 )
        {
            y = 4 + yOff - eOffset;
            if( countDown <= -480 )
            {
                ST::string tmp = ST::string::from_latin_1(tmpSrc).left(((-countDown - 480) >> 4) + 1);
                drawText.DrawString( 10, y, tmp, fConsoleTextColor );
            }
            y += yOff * ( fNumDisplayLines - ( showTooltip ? 1 : 0 ) );
        }
        else
        {
            y = 4 + yOff - eOffset;
            const char* line = fDisplayBuffer;
            for (i = 0; i < fNumDisplayLines - (showTooltip ? 1 : 0); i++) {
                drawText.DrawString(10, y, ST::string::from_latin_1(line), fConsoleTextColor);
                y += yOff;
                line += kMaxCharsWide;
            }
        }

        if( showTooltip )
            y += yOff;
    }

    ST::string prompt;
    if (fHelpMode) {
        prompt = ST_LITERAL("Get Help On:");
    } else if (fPythonMode) {
        if (fPythonMultiLines == 0) {
            prompt = ST_LITERAL(">>>");
        } else {
            prompt = ST_LITERAL("...");
        }
    } else {
        prompt = ST_LITERAL("]");
    }

    drawText.DrawString(10, y, prompt, 255, 255, 255, 255);
    i = 19 + drawText.CalcStringWidth(prompt);
    drawText.DrawString(i, y, IGetWorkingLine(), fConsoleTextColor);

    if( fCursorTicks >= 0 )
    {
        ST::string lineUpToCursor = ST::string::from_latin_1(fWorkingLine, fWorkingCursor);
        x = drawText.CalcStringWidth(lineUpToCursor);
        drawText.DrawString(i + x, y + 2, ST_LITERAL("_"), 255, 255, 255);
    }
    fCursorTicks--;
    if( fCursorTicks < -kCursorBlinkRate )
        fCursorTicks = kCursorBlinkRate;

    if (showTooltip) {
        ST::char_buffer helpMsgBuf = fLastHelpMsg.to_latin_1();
        ST::string helpMsg = ST::string::from_latin_1(helpMsgBuf.data(), std::min(helpMsgBuf.size(), static_cast<size_t>(kMaxCharsWide - 2)));
        drawText.DrawString(i, y - yOff, helpMsg, 255, 255, 0);
    } else {
        fHelpTimer--;
    }

    drawText.SetDrawOnTopMode( false );
}

//// IUpdateTooltip //////////////////////////////////////////////////////////

void    pfConsole::IUpdateTooltip()
{
    ST::string c = fEngine->GetCmdSignature(IGetWorkingLine());
    if (c.empty() || c != fLastHelpMsg)
    {
        /// Different--update timer to wait
        fHelpTimer = kHelpDelay;
        fLastHelpMsg = std::move(c);
    }
}

//// IExecuteWorkingLine /////////////////////////////////////////////////////

void    pfConsole::IExecuteWorkingLine()
{
    ST::string line = IGetWorkingLine();

    // leave leading space for Python multi lines (need the indents!)
    if (fPythonMultiLines == 0) {
        // Clean up working line by removing any leading whitespace
        line = line.trim_left();
    }

    if (line.empty() && !fHelpMode && !fPythonMode) {
        // Blank line--just print a blank line to the console and skip
        IAddLine("");
        return;
    }

    // only save history line if there is something there
    if (!line.empty()) {
        // Save to history
        fHistory[fPythonMode].fData[fHistory[fPythonMode].fCursor] = line;
        fHistory[fPythonMode].fCursor = (fHistory[fPythonMode].fCursor < kNumHistoryItems - 1) ? fHistory[fPythonMode].fCursor + 1 : 0;
        fHistory[fPythonMode].fRecallCursor = fHistory[fPythonMode].fCursor;
    }

    // EXECUTE!!!
    if (fHelpMode) {
        if (line.empty()) {
            IPrintSomeHelp();
        } else if (line.compare_i("commands") == 0) {
            fEngine->PrintCmdHelp({}, IAddLineCallback);
        } else if (!fEngine->PrintCmdHelp(line, IAddLineCallback)) {
            AddLine(fEngine->GetErrorMsg());
        }

        fHelpMode = false;
    } else if (fPythonMode) {
        // are we in Python multi-line mode?
        if (fPythonMultiLines > 0) {
            // if there was a line then bump num lines
            if (!line.empty()) {
                AddLine(ST_LITERAL("... ") + line);
                fPythonMultiLines++;
            }

            // is it time to evaluate all the multi lines that are saved?
            if (line.empty() || fPythonMultiLines >= kNumHistoryItems) {
                if (fPythonMultiLines >= kNumHistoryItems)
                    AddLine(ST_LITERAL("Python Multi-line buffer full!"));
                // get the lines and stuff them in our buffer
                ST::string_stream biglines;
                for (size_t i = fPythonMultiLines; i > 0; i--) {
                    // reach back in the history and find this line and paste it in here
                    int recall = fHistory[fPythonMode].fCursor - i;
                    if (recall < 0)
                        recall += kNumHistoryItems;
                    biglines << fHistory[fPythonMode].fData[recall] << '\n';
                }
                // now evaluate this mess they made
                PyObject* mymod = PythonInterface::FindModule("__main__");
                PythonInterface::RunStringInteractive(biglines.to_string().c_str(), mymod);
                // get the messages
                AddLine(PythonInterface::getOutputAndReset());
                // all done doing multi lines...
                fPythonMultiLines = 0;
            }
        } else {
            // else we are just doing single lines
            // was there actually anything in the input buffer?
            if (!line.empty()) {
                AddLine(ST_LITERAL(">>> ") + line);
                // check to see if this is going to be a multi line mode ( a ':' at the end)
                if (line.ends_with(":")) {
                    fPythonMultiLines = 1;
                } else
                    // else if not the start of a multi-line then execute it
                {
                    PyObject* mymod = PythonInterface::FindModule("__main__");
                    PythonInterface::RunStringInteractive(line.c_str(), mymod);
                    // get the messages
                    AddLine(PythonInterface::getOutputAndReset());
                }
            } else {
                AddLine(ST_LITERAL(">>> "));
            }
        }
    } else {
        if (!fEngine->RunCommand(line, IAddLineCallback)) {
            ST::string errorMsg = fEngine->GetErrorMsg();
            if (!errorMsg.empty())
                AddLine(errorMsg);
        }
    }

    IClearWorkingLine();
}

//// IPrintSomeHelp //////////////////////////////////////////////////////////

void    pfConsole::IPrintSomeHelp()
{
    ST::string msg1 = ST_LITERAL("The console contains commands arranged under groups and subgroups. \
To use a command, you type the group name plus the command, such as 'Console.Clear' or \
'Console Clear'.");
    
    ST::string msg2 = ST_LITERAL("To get help on a command or group, type '?' followed by the command or \
group name. Typing '?' and just hitting enter will bring up this message. Typing '?' and \
then 'commands' will bring up a list of all base groups and commands.");

    ST::string msg3 = ST_LITERAL("You can also have the console auto-complete a command by pressing tab. \
This will search for a group or command that starts with what you have typed. If there is more \
than one match, pressing tab repeatedly will cycle through all the matches.");


    AddLine({});
    AddLine(ST_LITERAL("How to use the console:"));
    IAddParagraph(msg1, 2);
    AddLine({});
    IAddParagraph(msg2, 2);
    AddLine({});
    IAddParagraph(msg3, 2);
    AddLine({});
}
