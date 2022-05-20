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

#include "pfConfirmationMgr.h"

#include <type_traits>

#include "plgDispatch.h"
#include "plTimerCallbackManager.h"

#include "pnMessage/plNotifyMsg.h"
#include "pnNetCommon/plNetApp.h"

#include "plMessage/plConfirmationMsg.h"
#include "plMessage/plConsoleMsg.h"
#include "plMessage/plLinkToAgeMsg.h"
#include "plMessage/plTimerCallbackMsg.h"

#include "pfGameGUIMgr/pfGameGUIMgr.h"
#include "pfGameGUIMgr/pfGUIDialogHandlers.h"
#include "pfGameGUIMgr/pfGUIDialogMod.h"
#include "pfGameGUIMgr/pfGUIControlMod.h"
#include "pfGameGUIMgr/pfGUITextBoxMod.h"
#include "pfLocalizationMgr/pfLocalizationMgr.h"
#include "pfMessage/pfGameGUIMsg.h"
#include "pfMessage/pfGUINotifyMsg.h"
#include "pfMessage/pfKIMsg.h"

using namespace ST::literals;

////////////////////////////////////////////////////////////////////////////////

// From GUI_District_KIYesNo.prp, so don't change them.
constexpr uint32_t kMessageTextTag = 12U;
constexpr uint32_t kLogoutTextTag = 63U; // Leftmost
constexpr uint32_t kYesTextTag = 60U; // Center
constexpr uint32_t kNoTextTag = 61U; // Rightmost
constexpr uint32_t kLogoutButtonTag = 62U;
constexpr uint32_t kYesButtonTag = 10U;
constexpr uint32_t kNoButtonTag = 11U;

////////////////////////////////////////////////////////////////////////////////

class pfConfirmationDialogProc : public pfGUIDialogProc
{
    friend class pfConfirmationMgr;
    pfConfirmationMgr* fMgr;

    inline void ISetText(ST::string text, uint32_t tagID)
    {
        pfGUITextBoxMod* mod = pfGUITextBoxMod::ConvertNoRef(fDialog->GetControlFromTag(tagID));
        hsAssert(mod != nullptr, "You sure about this, boss?");
        mod->SetText(std::move(text));
    }

    inline void ISetLocalizedText(const ST::string& path, uint32_t tagID)
    {
        ISetText(pfLocalizationMgr::Instance().GetString(path), tagID);
    }

    void ILayoutYesNo(ST::string text)
    {
        fDialog->GetControlFromTag(kLogoutTextTag)->SetVisible(false);
        fDialog->GetControlFromTag(kLogoutButtonTag)->SetVisible(false);
        fDialog->GetControlFromTag(kYesTextTag)->SetVisible(true);
        fDialog->GetControlFromTag(kYesButtonTag)->SetVisible(true);
        fDialog->GetControlFromTag(kNoTextTag)->SetVisible(true);
        fDialog->GetControlFromTag(kNoButtonTag)->SetVisible(true);
        ISetLocalizedText("KI.YesNoDialog.YESButton"_st, kYesTextTag);
        ISetLocalizedText("KI.YesNoDialog.NoButton"_st, kNoTextTag);
        ISetText(std::move(text), kMessageTextTag);
    }

    void ILayoutSingle(ST::string message, const ST::string& button)
    {
        fDialog->GetControlFromTag(kLogoutTextTag)->SetVisible(false);
        fDialog->GetControlFromTag(kLogoutButtonTag)->SetVisible(false);
        fDialog->GetControlFromTag(kYesTextTag)->SetVisible(false);
        fDialog->GetControlFromTag(kYesButtonTag)->SetVisible(false);
        fDialog->GetControlFromTag(kNoTextTag)->SetVisible(true);
        fDialog->GetControlFromTag(kNoButtonTag)->SetVisible(true);
        ISetLocalizedText(button, kNoTextTag);
        ISetText(std::move(message), kMessageTextTag);
    }

    void ILayoutQuit(ST::string text)
    {
        bool canLogout = plNetClientApp::GetInstance()->GetPlayerID() != 0;
        fDialog->GetControlFromTag(kLogoutTextTag)->SetVisible(canLogout);
        fDialog->GetControlFromTag(kLogoutButtonTag)->SetVisible(canLogout);
        fDialog->GetControlFromTag(kYesTextTag)->SetVisible(true);
        fDialog->GetControlFromTag(kYesButtonTag)->SetVisible(true);
        fDialog->GetControlFromTag(kNoTextTag)->SetVisible(true);
        fDialog->GetControlFromTag(kNoButtonTag)->SetVisible(true);
        if (canLogout)
            ISetText("Logout"_st, kLogoutTextTag); // FIXME: This is missing from the LOC files.
        ISetLocalizedText("KI.YesNoDialog.QuitButton"_st, kYesTextTag);
        ISetLocalizedText("KI.YesNoDialog.NoButton"_st, kNoTextTag);
        ISetText(std::move(text), kMessageTextTag);
    }

public:
    pfConfirmationDialogProc(pfConfirmationMgr* mgr)
        : fMgr(mgr)
    { }

    ~pfConfirmationDialogProc() = default;

    void OnInit() override
    {
        if (!fMgr->fPending.empty())
            fDialog->Show();
    }

    void OnShow() override
    {
        // Prevent dialog trolling...
        if (fMgr->fPending.empty()) {
            fDialog->Hide();
            return;
        }

        fMgr->fState = pfConfirmationMgr::State::WaitingForInput;

        const auto& msg = fMgr->fPending.front();
        ST::string text;
        if (auto locMsg = plLocalizedConfirmationMsg::ConvertNoRef(msg.Get()); locMsg != nullptr)
            text = pfLocalizationMgr::Instance().GetString(locMsg->GetText(), locMsg->GetArgs());
        else
            text = msg->GetText();

        switch (msg->GetType()) {
        case plConfirmationMsg::Type::ConfirmQuit:
            ILayoutQuit(text);
            break;
        case plConfirmationMsg::Type::ForceQuit:
            ILayoutSingle(text, "KI.YesNoDialog.QuitButton"_st);
            break;
        case plConfirmationMsg::Type::OK:
            ILayoutSingle(text, "KI.YesNoDialog.OKButton"_st);
            break;
        case plConfirmationMsg::Type::YesNo:
            ILayoutYesNo(text);
            break;
        DEFAULT_FATAL(msg->GetType());
        }
    }

    void OnHide() override
    {
        switch (fMgr->fState) {
        case pfConfirmationMgr::State::WaitingForInput:
            // Prevent dialog trolling...
            fDialog->Show();
            break;
        case pfConfirmationMgr::State::Ready:
            // If another confirmation is already available, we don't want to just show it now.
            // Instead, wait a short period of time, then re-process.
            plgTimerCallbackMgr::NewTimer(.5f, new plTimerCallbackMsg(fMgr->GetKey(), (int32_t)fMgr->fState));
            fMgr->fState = pfConfirmationMgr::State::Delaying;
            break;
        }
    }

    void DoSomething(pfGUIControlMod* ctrl) override
    {
        plConfirmationMsg::Result result;
        switch (ctrl->GetTagID()) {
        case kLogoutButtonTag:
            result = plConfirmationMsg::Result::Logout;
            break;
        case kYesButtonTag:
            result = plConfirmationMsg::Result::Yes;
            break;
        case kNoButtonTag:
            switch (fMgr->fPending.front()->GetType()) {
            case plConfirmationMsg::Type::ForceQuit:
            case plConfirmationMsg::Type::OK:
                result = plConfirmationMsg::Result::OK;
                break;
            default:
                result = plConfirmationMsg::Result::No;
                break;
            }
            break;
        DEFAULT_FATAL(ctrl->GetTagID());
        }

        fMgr->ISendResult(result, pfConfirmationMgr::State::Ready);
        fDialog->Hide();
    }

    void OnDestroy() override
    {
        // Crap... Someone has thrown the dialog away from underneath us.
        // If anybody is still around, cancel anything waiting on input.
        fMgr->ISendResult(plConfirmationMsg::Result::Cancel, pfConfirmationMgr::State::Alive);
    }

    void OnControlEvent(ControlEvt event) override
    {
        // There is only one control event, atm... Someone pressed escape
        // thereby closing the dialog. Therefore, just send a cancel.
        if (event == kExitMode) {
            fMgr->ISendResult(plConfirmationMsg::Result::Cancel, pfConfirmationMgr::State::Ready);
            fDialog->Hide();
        }
    }
};

////////////////////////////////////////////////////////////////////////////////

pfConfirmationMgr::pfConfirmationMgr()
    : fState(State::Alive),
      fProc(new pfConfirmationDialogProc(this))
{
    // Prevent the GUI system from killing us. Screw that comment in the GUI header.
    fProc->IncRef();
}

pfConfirmationMgr::~pfConfirmationMgr()
{
    // Any pending items that are callbacks fire now as being cancelled.
    while (!fPending.empty()) {
        const auto& msg = fPending.front();
        std::visit([](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::function<void(plConfirmationMsg::Result)>>)
                arg(plConfirmationMsg::Result::Cancel);
        }, msg->GetCallback());
        fPending.pop();
    }

    if (fProc->fDialog) {
        fProc->fDialog->SetHandler(nullptr);
        if (pfGameGUIMgr::GetInstance())
            pfGameGUIMgr::GetInstance()->UnloadDialog(fProc->fDialog);
    }

    if (fProc->DecRef())
        delete fProc;
}

////////////////////////////////////////////////////////////////////////////////

void pfConfirmationMgr::ISendResult(plConfirmationMsg::Result result, State newState)
{
    if (fPending.empty())
        return;

    fState = pfConfirmationMgr::State::ProcessingInput;

    const auto& msg = fPending.front();

    // If a quit was requested, don't rely on any downstream processing. Just post
    // a quit message to happen on the next dispatcher pump.
    bool wantQuit = (msg->GetType() == plConfirmationMsg::Type::ConfirmQuit &&
                     result == plConfirmationMsg::Result::Quit);
    bool forceQuit = msg->GetType() == plConfirmationMsg::Type::ForceQuit;
    if (wantQuit || forceQuit) {
        plConsoleMsg* quitMsg = new plConsoleMsg(plConsoleMsg::kExecuteLine, "App.Quit"_st);
        plgDispatch::Dispatch()->MsgQueue(quitMsg);
    }

    // Again, don't rely on bugprone Python code to handle critical functionality.
    bool wantLogout = (msg->GetType() == plConfirmationMsg::Type::ConfirmQuit &&
                       result == plConfirmationMsg::Result::Logout);
    if (wantLogout) {
        plLinkToAgeMsg* logoutMsg = new plLinkToAgeMsg();
        logoutMsg->AddReceiver(plNetClientApp::GetInstance()->GetKey());
        logoutMsg->PlayLinkSfx(false, false);
        logoutMsg->GetAgeLink()->GetAgeInfo()->SetAgeFilename("StartUp"_st);
        logoutMsg->GetAgeLink()->SpawnPoint().SetTitle("Default"_st);
        logoutMsg->GetAgeLink()->SpawnPoint().SetName("LinkInPointDefault"_st);
        plgDispatch::Dispatch()->MsgQueue(logoutMsg);
    }

    std::visit([result](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::function<void(plConfirmationMsg::Result)>>) {
            // New: High level, potentially stateful functor
            arg(result);
        } else if constexpr (std::is_same_v<T, plKey>) {
            // Old: Send a notify message to whoever called (probably) `PtYesNoDialog()`
            plNotifyMsg* notifyMsg = new plNotifyMsg;
            notifyMsg->AddReceiver(arg);
            notifyMsg->SetBCastFlag(plMessage::kNetPropagate, false);
            notifyMsg->SetState((float)result);
            notifyMsg->AddVariableEvent("YesNo"_st, (int32_t)result);
            notifyMsg->Send();
        } else {
            static_assert(std::is_same_v<T, std::monostate>, "non-exhaustive visitor");
        }
    }, msg->GetCallback());

    fPending.pop();
    fState = newState;
}

////////////////////////////////////////////////////////////////////////////////

void pfConfirmationMgr::ILoadDialog()
{
    // We need to be particularly careful that some old xKI.py doesn't run wild
    // over us.
    pfGameGUIMgr* gui = pfGameGUIMgr::GetInstance();
    pfGUIDialogMod* dialog = gui->GetDialogFromString("KIYesNo");
    if (dialog) {
        fState = State::Ready;
        dialog->SetHandler(fProc);

        // The default pfGUIDialogMod proc ate the OnInit() call. So, here's another one.
        fProc->OnInit();
    } else {
        fState = State::WaitingForDialogLoad;
        gui->LoadDialog("KIYesNo"_st, GetKey(), "GUI");
    }
}

////////////////////////////////////////////////////////////////////////////////

bool pfConfirmationMgr::MsgReceive(plMessage* msg)
{
    plConfirmationMsg* confirmMsg = plConfirmationMsg::ConvertNoRef(msg);
    if (confirmMsg) {
        fPending.emplace(confirmMsg);
        if (fState == State::Ready)
            fProc->fDialog->Show();
        else if (fState == State::Alive)
            ILoadDialog();
        return true;
    }

    pfKIMsg* kiMsg = pfKIMsg::ConvertNoRef(msg);
    if (kiMsg) {
        // This seems a little objectionable, IMO, but it keeps the behavior consistent.
        // When the disable KI message comes in, we force-cancel any confirmations, but
        // the old behavior did not "remember" this and allows any new dialogs to pop up,
        // even though the KI is supposedly disabled.
        if (kiMsg->GetCommand() == pfKIMsg::kDisableKIandBB) {
            if (fState == State::WaitingForInput) {
                while (!fPending.empty())
                    ISendResult(plConfirmationMsg::Result::Cancel, State::Ready);
                fProc->fDialog->Hide();
            }
        }
        return true;
    }

    pfGUINotifyMsg* guiNotifyMsg = pfGUINotifyMsg::ConvertNoRef(msg);
    if (guiNotifyMsg) {
        // Handle the dialog LOAD so we can insert our own dialog proc
        if (guiNotifyMsg->GetEvent() == pfGUINotifyMsg::kDialogLoaded) {
            hsAssert(fState == State::WaitingForDialogLoad, "Unexpected dialog load");
            fState = State::Ready;
            pfGUIDialogMod* dialog = pfGUIDialogMod::ConvertNoRef(guiNotifyMsg->GetControlKey()->VerifyLoaded());
            dialog->SetHandler(fProc);

            // The default pfGUIDialogMod proc ate the OnInit() call. So, here's another one.
            fProc->OnInit();
        } else {
            hsAssert(false, "Unexpected GUINotifyMsg");
        }

        // No other GUI notify messages should come though because we have
        // changed out the notify proc to one that does not send messages.
        return true;
    }

    plTimerCallbackMsg* timerMsg = plTimerCallbackMsg::ConvertNoRef(msg);
    if (timerMsg) {
        // Someone might delete the dialog out from under us eg by PtUnLoadDialog("KIYesNo")...
        if (fState == State::Delaying)
            fState = (State)timerMsg->fID;

        if (!fPending.empty()) {
            if (fState == State::Alive)
                ILoadDialog();
            else if (fState == State::Ready)
                fProc->fDialog->Show();
            else
                hsAssert(false, "Unexpected state on timer callback");
        }
        return true;
    }

    return hsKeyedObject::MsgReceive(msg);
}

////////////////////////////////////////////////////////////////////////////////

void pfConfirmationMgr::Init()
{
    if (s_instance == nullptr) {
        s_instance = new pfConfirmationMgr;
        s_instance->RegisterAs(kConfirmationMgr_KEY);

        plgDispatch::Dispatch()->RegisterForType(plConfirmationMsg::Index(), s_instance->GetKey());
        plgDispatch::Dispatch()->RegisterForExactType(pfKIMsg::Index(), s_instance->GetKey());
    }
}

void pfConfirmationMgr::Shutdown()
{
    if (s_instance) {
        plgDispatch::Dispatch()->UnRegisterForType(plConfirmationMsg::Index(), s_instance->GetKey());
        plgDispatch::Dispatch()->UnRegisterForExactType(pfKIMsg::Index(), s_instance->GetKey());

        s_instance->UnRegisterAs(kConfirmationMgr_KEY); // UnRefs us
        s_instance = nullptr;
    }
}

pfConfirmationMgr* pfConfirmationMgr::s_instance{};
