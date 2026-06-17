# PythonFileMod callback methods

Every PythonFileMod's script must define a class with the same name as the
script. This class must subclass `ptModifier` or one of its subclasses,
`ptMultiModifier` or `ptResponder` (all from the `PlasmaTypes` module).

This modifier class can define callback methods with specific fixed names, which
will be called by the engine in response to various events.

## Overview

Lifecycle callbacks:

| Name | Parameters | C++ message class | Called when |
|---|---|---|---|
| [`__init__`](#__init__) | `(self)` | (none) | Python object created (in-game or in 3ds Max) |
| [`OnInit`](#oninit) | `(self)` | (none) | `plPythonFileMod` loaded in-game |
| [`OnFirstUpdate`](#onfirstupdate) | `(self)` | first `plEvalMsg` | before first [`OnUpdate`](#onupdate) |
| [`OnServerInitComplete`](#onserverinitcomplete) | `(self)` | `plInitialAgeStateLoadedMsg` | age loading about to end (all SDL states received) |
| [`BeginAgeUnLoad`](#beginageunload) | `(self, avatar: ptSceneobject)` | `plAgeBeginLoadingMsg` | age about to unload |

Event handlers:

| Name | Parameters | C++ message class | Called when |
|---|---|---|---|
| [`AvatarPage`](#avatarpage) | `(self, avatar: ptSceneobject, loading: bool, lastOut: bool)` | `plPlayerPageMsg` | avatar links in/out |
| [`Load`](#load) | `(self)` | `plSDLModifierStateMsg` | `self.SDL` was updated from the network |
| [`OnAIMsg`](#onaimsg) | `(self, brain: ..., msgType: int, userStr: str, args: ...)` | `plAIMsg` | "critter brain" NPC (Ahnonay quab) events |
| [`OnAvatarSpawn`](#onavatarspawn) | `(self, arg: bool)` | `plAvatarSpawnNotifyMsg` | armature spawned or warped |
| [`OnBackdoorMsg`](#onbackdoormsg) | `(self, target: str, param: str)` | `pfBackdoorMsg` | `Python.Backdoor` console command |
| [`OnBehaviorNotify`](#onbehaviornotify) | `(self, type: int, avatar: ..., state: bool)` | `plAvatarBehaviorNotifyMsg` | avatar behavior started/stopped |
| [`OnClimbingBlockerEvent`](#onclimbingblockerevent) | `(self, blocker: ptSceneobject)` | `plClimbEventMsg` | Gahreesen wall blocker touched |
| [`OnControlKeyEvent`](#oncontrolkeyevent) | `(self, controlKey: int, activeFlag: bool)` | `plControlEventMsg` | standard key presses |
| [`OnGUINotify`](#onguinotify) | `(self, id: int, control: ..., event: int)` | `pfGUINotifyMsg` | GUI events |
| [`OnNotify`](#onnotify) | `(self, state: float, id: int, event: Sequence[tuple])` | `plNotifyMsg` | many different events |
| [`OnOwnershipChanged`](#onownershipchanged) | `(self)` | `plNetOwnershipMsg` | client starts/stops being the synched object owner |
| [`OnPageLoad`](#onpageload) | `(self, what: int, who: str)` | `plRoomLoadNotifyMsg` | page un-/loaded |
| [`OnUpdate`](#onupdate) | `(self, secs: float, delta: float)` | `plEvalMsg` | every main update loop iteration |
| [`OnVaultEvent`](#onvaultevent) | `(self, event: int, tupData: tuple)` | (none) | vault node changed or ref added/removed |
| [`OnVaultNotify`](#onvaultnotify) | `(self, event: int, tupData: tuple)` | `plVaultNotifyMsg` | events related to age instances/links |

Asynchronous function callbacks:

| Name | Parameters | C++ message class | Called asynchronously by |
|---|---|---|---|
| [`gotPublicAgeList`](#gotpublicagelist) | `(self, ages: Sequence[tuple])` | `plNetCommPublicAgeListMsg` | `PtGetPublicAgeList` |
| [`OnGameScoreMsg`](#ongamescoremsg) | `(self, msg: ptGameScoreMsg)` | `pfGameScoreMsg` | `ptGameScore` methods |
| [`OnLOSNotify`](#onlosnotify) | `(self, id: int, noHitFlag: bool, sceneobject: ..., hitPoint: ..., distance: float)` | `plLOSHitMsg` | `PtRequestLOSScreen` |
| [`OnSDLNotify`](#onsdlnotify) | `(self, varName: str, sdlName: str, playerID: int, tag: str)` | `plSDLNotificationMsg` | `ptSDL.setNotify` |
| [`OnTimer`](#ontimer) | `(self, id: int)` | `plTimerCallbackMsg` | `PtAtTimeCallback` |

Internal KI/GUI event handlers:

| Name | Parameters | C++ message class | Called when |
|---|---|---|---|
| [`OnAccountUpdate`](#onaccountupdate) | `(self, updateType: int, result: int, playerID: int)` | `plAccountUpdateMsg` | avatar-/account-related events |
| [`OnClothingUpdate`](#onclothingupdate) | `(self)` | `plClothingUpdateBCMsg` | avatar clothing changed |
| [`OnDefaultKeyCaught`](#ondefaultkeycaught) | `(self, ch: str, isDown: bool, isRepeat: bool, isShift: bool, isCtrl: bool, keycode: int)` | `plKeyEventMsg` | unhandled key press |
| [`OnMarkerMsg`](#onmarkermsg) | `(self, msgType: int, tupData: tuple)` | `pfMarkerMsg` | marker captured |
| [`OnMemberUpdate`](#onmemberupdate) | `(self)` | `plMemberUpdateMsg` | "Age Players" list needs updating |
| [`OnMovieEvent`](#onmovieevent) | `(self, movieName: str, reason: int)` | `pfMovieEventMsg` | movie finished playing |
| [`OnRemoteAvatarInfo`](#onremoteavatarinfo) | `(self, player: ...)` | `plRemoteAvatarInfoMsg` | player clicked another avatar |
| [`OnRTChat`](#onrtchat) | `(self, player: ptPlayer, message: str, flags: int)` | `pfKIMsg` | chat message received |
| [`OnScreenCaptureDone`](#onscreencapturedone) | `(self, image: ptImage)` | `plCaptureRenderMsg` | `PtStartScreenCapture` sceeenshot finished |
| [`OnSubtitleMsg`](#onsubtitlemsg) | `(self, text: str, speaker: str)` | `plSubtitleMsg` | subtitle line needs to be displayed |

Unused:

| Name | Parameters | C++ message class |
|---|---|---|
| [`OnAgeVaultEvent`](#onagevaultevent) | `(self, event: int, tupData: tuple)` | (none) |
| [`OnCCRMsg`](#onccrmsg) | `(self, type: int, message: str, playerID: int)` | `plCCRCommunicationMsg` |

## `__init__`

Not technically a callback method, just the standard Python initializer method.

Every modifier's `__init__` method must call the superclass `__init__` and then
set at least the `id` and `version` attributes. If the modifier has its own
custom attributes, `__init__` should also set those to reasonable defaults.

Avoid putting non-trivial code in `__init__`, especially anything that might
raise an exception. If `__init__` fails, Plasma won't have a Python object for
the modifier and none of the callback methods will be called.

The `__init__` methods of **all** installed Python modifiers also run every time
the 3ds Max plugin starts up, before the resource manager is initialized. This
makes it unsafe to call most Plasma APIs from `__init__` - many need the
resource manager to work and so can fail or even cause a hard crash when called
from `__init__` by the Max plugin!

Do not call any Plasma APIs from `__init__`, except for the following, which are
known to work:

* `Plasma.PtDebugPrint`
* `Plasma.PtGetLocalizedString`

### Default attributes

After `__init__` returns, the engine adds the following attributes to `self`:

* `isInitialStateLoaded`: `bool` as `int` - Whether all SDL states for the age
  instance have been received yet, meaning that it's safe to use `self.SDL` and
  `PtGetAgeSDL()`. Initially 0, changes to 1 when
  [`OnServerInitComplete`](#onserverinitcomplete) is called.
* `key`: `ptKey` - The `plPythonFileMod`'s key.
* `sceneobject`: `ptSceneobject` - Target of the Python modifier.
* `SDL`: `ptSDL` - SDL variables for this Python modifier. Only present if there
  is a `STATEDESC` with the same name as the Python script.

## `OnFirstUpdate`

Called once per modifier instance, immediately before [`OnUpdate`](#onupdate) is
called for the first time. This normally happens before
[`OnServerInitComplete`](#onserverinitcomplete) is called.

## `OnUpdate`

Corresponds to `plEvalMsg`, or the C++ `plModifier::IEval` method. Called on
every iteration of the main client update loop.

### Parameters

* `secs`: `float` - `plEvalMsg::DSeconds()`.
* `delta`: `float` - `plEvalMsg::DelSeconds()`.

## `OnNotify`

Corresponds to `plNotifyMsg`. Called in response to a variety of events - see
[below](#notify-event-types-and-tuple-elements).

### Parameters

* `state`: `float` - `plNotifyMsg::fState`.
* `id`: `int` - If the `plNotifyMsg` was sent by an object passed into one of
  this modifier's parameters/attributes, then `id` is the ID number of that
  attribute. Otherwise, `id` is `-1`.
* `events`: `Sequence[tuple]` - The `plNotifyMsg`'s event
  records, translated to a sequence of tuples. Each tuple corresponds to one
  event record. In each tuple, the first element (index 0) is the event type as
  an `int` (see `PtEventType`) and the other elements vary depending on the
  specific event type (see [below](#notify-event-types-and-tuple-elements)). The
  entire `events` sequence can also be passed to `PlasmaTypes.PtFindAvatar` to
  easily get the `ptSceneobject` for the avatar that caused this notification.

### Notify event types and tuple elements

#### `kCollision`

* \[1] `enter`: `bool` as `int` - `proCollisionEventData::fEnter`.
* \[2] `hitter`: `ptSceneobject` - `proCollisionEventData::fHitter`.
* \[3] `hittee`: `ptSceneobject` - `proCollisionEventData::fHittee`.

#### `kSpawned`

* \[1] `spawner`: `ptSceneobject` - `proSpawnedEventData::fSpawner`.
* \[2] `spawnee`: `ptSceneobject` - `proSpawnedEventData::fSpawnee`.

#### `kPicked`

* \[1] `enabled`: `bool` as `int` - `proPickedEventData::fEnabled`.
* \[2] `picker`: `ptSceneobject` - `proPickedEventData::fPicker`.
* \[3] `picked`: `ptSceneobject` - `proPickedEventData::fPicked`.
* \[4] `hitPoint`: `ptPoint3` - `proPickedEventData::fHitPoint`, which is in
  world coordinates.
* \[5] `hitPointLocal`: `ptPoint3` - `proPickedEventData::fHitPoint` converted
  to the `picked` object's local coordinate space.

#### `kControlKey`

* \[1] `controlKey`: `int` - `proControlKeyEventData::fControlKey`.
* \[2] `down`: `bool` as `int` - `proControlKeyEventData::fDown`.

#### `kVariable`

* \[1] `name`: `str` - `proVariableEventData::fName`.
* \[2] `dataType`: `PtNotifyDataType` as `int` -
  `proVariableEventData::fDataType`.
* \[3] `value`: `float | ptKey | int | None` (depending on `dataType`) - The
  variable value.

#### `kFacing`

* \[1] `enabled`: `bool` as `int` - `proFacingEventData::enabled`.
* \[2] `facer`: `ptSceneobject` - `proFacingEventData::fFacer`.
* \[3] `facee`: `ptSceneobject` - `proFacingEventData::fFacee`.
* \[4] `dotProduct`: `float` - `proFacingEventData::dot`.

#### `kContained`

* \[1] `entering`: `bool` as `int` - `proContainedEventData::fEntering`.
* \[2] `contained`: `ptSceneobject` - `proContainedEventData::fContained`.
* \[3] `container`: `ptSceneobject` - `proContainedEventData::fContainer`.

#### `kActivate`

* \[1] `active`: `bool` as `int` - `proActivateEventData::fActive`.
* \[2] `activate`: `bool` as `int` - `proActivateEventData::fActivate`.

#### `kCallback`

* \[1] `eventType`: `int` - `proCallbackEventData::fEventType`.

#### `kResponderState`

* \[1] `state`: `int` - `proResponderStateEventData::fState`.

#### `kMultiStage`

* \[1] `stage`: `int` - `proMultiStageEventData::fStage`.
* \[2] `event`: `PtMultiStageEventType` as `int` -
  `proMultiStageEventData::fEvent`.
* \[3] `avatar`: `ptSceneobject` - `proMultiStageEventData::fAvatar`.

#### `kOfferLinkingBook`

* \[1] `offerer`: `ptSceneobject` - `proOfferLinkingBookEventData::offerer`.
* \[2] `event`: `int` - `proOfferLinkingBookEventData::targetAge`. The C++ name
  is misleading - this does not identify the target age.
* \[3] `offeree`: `int` - `proOfferLinkingBookEventData::offeree`.

#### `kBook`

* \[1] `event`: `int` - `proBookEventData::fEvent`.
* \[2] `linkID`: `int` - `proBookEventData::fLinkID`.

## `OnTimer`

Corresponds to `plTimerCallbackMsg`. Called in response to `PtAtTimeCallback`.
The `selfKey` passed to `PtAtTimeCallback` must be this Python modifier's
`self.key`, otherwise it will not receive this callback!

### Parameters

* `id`: `int` - `plTimerCallbackMsg::fID`. ID originally passed to
  `PtAtTimeCallback`.

## `OnControlKeyEvent`

Corresponds to `plControlEventMsg`.

### Parameters

* `controlKey`: `int` - `plControlEventMsg::GetControlCode()`.
* `activeFlag`: `bool` - `plControlEventMsg::ControlActivated()`.

## `Load`

Corresponds to `plSDLModifierStateMsg` (with type `plSDLModifierMsg::kRecv`), or
the C++ `plSDLModifier::ISetCurrentStateFrom` method. Any time the state of
`self.SDL` is updated from the network, this callback is called immediately
afterwards. This happens both for the initial SDL states received while loading
into the age, and for any later SDL updates while in the age.

## `OnGUINotify`

Corresponds to `pfGUINotifyMsg`.

### Parameters

* `id`: `int` - If the `pfGUINotifyMsg` was sent by an object passed into one of
  this modifier's parameters/attributes, then `id` is the ID number of that
  attribute. Otherwise, `id` is `-1`.
* `control`: `ptGUIDialog | ptKey | None` - `pfGUINotifyMsg::GetControlKey()`.
  If the key points to a `pfGUIDialogMod` or subclass, `control` is that
  object (converted to a `ptGUIDialog` or subclass). If the key points to
  something else, `control` is the key itself. If the key is nil, `control` is
  `None`. Most scripts assume that `control` is a `ptGUIDialog`.
* `event`: `int` - `pfGUINotifyMsg::GetEvent()`.

## `OnPageLoad`

Corresponds to `plRoomLoadNotifyMsg`. Called whenever a page is loaded/unloaded.

* `what`: `int` - `plRoomLoadNotifyMsg::GetWhatHappen()`. Either
  `PlasmaTypes.kLoaded` or `PlasmaTypes.kUnloaded`.
* `who`: `str` - The name of the `plRoomLoadNotifyMsg::GetRoom()` key (which
  should point to a `plSceneNode`).

## `OnClothingUpdate`

Corresponds to `plClothingUpdateBCMsg`.

This callback is only expected to be used by the avatar customization (closet)
code.

## `OnMemberUpdate`

Corresponds to `plMemberUpdateMsg`. Called when the KI "Age Players" list needs
to update - that is:

* when initially linking into an age
* when another avatar links in or out
* when another avatar comes in or out of "talk range"

This callback is only expected to be used by the KI code. Other scripts that
want to react to avatars linking in/out should use [`AvatarPage`](#avatarpage)
instead.

## `OnRemoteAvatarInfo`

Corresponds to `plRemoteAvatarInfoMsg`. Called when the player clicks another
avatar in the world to select it in the KI.

This callback is only expected to be used by the KI code.

### Parameters

* `player`: `ptPlayer | int` - Avatar information about
  `plRemoteAvatarInfoMsg::GetAvatarKey()`, which is the avatar that was clicked.
  Can also be the integer `0` if that key is nil or the avatar info lookup
  failed for some reason.

## `OnRTChat`

Corresponds to `pfKIMsg` with type `kHACKChatMsg` - that is, KI chat messages.

This callback is only expected to be used by the KI code.

### Parameters

* `player`: `ptPlayer` - Information about the avatar with KI number
  `pfKIMsg::GetPlayerID()`, which is the avatar that sent the chat message.
* `message`: `str` - `pfKIMsg::GetString()`. The text of the chat message.
* `flags`: `pfKIMsg::Flags` as `int` - `pfKIMsg::GetFlags()`.

## `OnVaultEvent`

Behaves like a callback registered via `VaultRegisterCallback` in C++. Called
when the server notifies the client that a vault node ref was added or removed,
or a vault node's data changed.

Not to be confused with [`OnVaultNotify`](#onvaultnotify), which is specifically
for changes related to age instances and links.

### Parameters

* `event`: `PtVaultCallbackTypes` as `int` - The type of vault event that
  occurred. Currently, this can only be `kVaultNodeRefAdded`,
  `kVaultRemovingNodeRef`, or `kVaultNodeSaved` (changed), even though other
  constants are also defined.
* `tupData`: `tuple[ptVaultNode | ptVaultNodeRef]` - A 1-element tuple
  containing the `ptVaultNodeRef` (for added/removed events) or `ptVaultNode`
  (for changed events) for the changed node or node ref.

## `AvatarPage`

Corresponds to `plPlayerPageMsg`. Called when an avatar links in or out of the
current age instance.

### Parameters

* `avatar`: `ptSceneobject` - Avatar object for `plPlayerPageMsg::fPlayer`, which
  is the avatar that linked in or out.
* `loading`: `bool` - Negation of `plPlayerPageMsg::fUnload`. Whether the avatar
  is linking in or out.
* `lastOut`: `bool` - `plPlayerPageMsg::fLastOut`. Whether the avatar that is
  linking out was the only other avatar left in the age instance, aside from the
  local player's avatar. Can only be true for link-outs, not link-ins.

## `OnSDLNotify`

Corresponds to `plSDLNotificationMsg`. Receives SDL change notifications
requested via `ptSDL.setNotify`.

### Parameters

* `varName`: `str` - Name of the SDL variable that changed
  (`plStateVariable::GetName()` for `plSDLNotificationMsg::fVar`).
* `sdlName`: `str` - Name of the SDL descriptor that defines the variable that
  changed (`plSDLNotificationMsg::fSDLName`).
* `playerID`: `int` - `plSDLNotificationMsg::fPlayerID`. KI number of the avatar
  that caused the state change, or 0 for certain special cases, such as
  resetting age SDL via the console.
* `tag`: `str` - `plSDLNotificationMsg::fHintString`. The "tag string" provided
  via `ptSDL.setTagString` by the code that changed the SDL variable, or empty
  if no tag string was set (which is the usual case).

## `OnOwnershipChanged`

Corresponds to `plNetOwnershipMsg`. Called when the client becomes, or stops
being, the owner of some network-synchronized objects in the current age
instance. The callback doesn't indicate which objects have changed ownership.
The Python modifier needs to call `ptSceneobject.isLocallyOwned` on objects it
cares about to determine if the client now owns those objects.

## `OnAgeVaultEvent`

Unused. A few scripts define it anyway, with the same parameters as
`OnVaultEvent`. This may be a remnant from an older engine version.

## `OnInit`

Called once per modifier instance, immediately after it has been loaded and
initialized, but before any other callback methods are called.

Unlike `__init__`, this callback method is only called during gameplay and not
in the 3ds Max plugin, so it's safe to call Plasma APIs inside `OnInit` that
would not be safe to call inside `__init__`.

## `OnCCRMsg`

Corresponds to `plCCRCommunicationMsg`. Seems to be unused. It might have been
intended only for Cyan's internal CCR clients, which would have received CCR
requests from players.

### Parameters

* `type`: `plCCRCommunicationMsg::Type` as `int` - `plCCRCommunicationMsg::GetType()`.
* `message`: `str` - `plCCRCommunicationMsg::GetMessageText()`.
* `playerID`: `int` - `plCCRCommunicationMsg::GetCCRPlayerID()`.

## `OnServerInitComplete`

Corresponds to `plInitialAgeStateLoadedMsg`. Called when the age loading process
finishes, which happens once the client has received all SDL states for the
current age instance. From this point on, `self.SDL` and `PtGetAgeSDL()` contain
usable values. Before `OnServerInitComplete` is called, Python modifiers
shouldn't try to interact with age SDL values.

Normally, [`OnFirstUpdate`](#onfirstupdate) has already been called by the time
`OnServerInitComplete` runs. For Python modifiers loaded after the normal age
loading process has already finished (that is, after SDL states have already
been received), `OnServerInitComplete` is instead called immediately after
[`OnInit`](#oninit) and before [`OnFirstUpdate`](#onfirstupdate).

The Python modifier's [`isInitialStateLoaded`](#default-attributes) attribute
also changes from 0 to 1 just before `OnServerInitComplete` is called.

## `OnVaultNotify`

Corresponds to `plVaultNotifyMsg`. Called when an age link belonging to the
current avatar (either an owned age or a visit invite) is added or removed, or
when an age instance's public/private status changes.

Not to be confused with [`OnVaultEvent`](#onvaultevent), which is for general
vault node/ref changes.

### Parameters

* `event`: `PtVaultNotifyTypes` as `int` - `plVaultNotifyMsg::GetType()`.
* `tupData`: `tuple[ptVaultAgeLinkNode | str]` - A 1-element tuple
  with information about the age to which the change applies. For
  `kRegisteredOwnedAge`, `kUnRegisteredOwnedAge`, `kRegisteredVisitAge`, and
  `kUnRegisteredVisitAge`, this is the `ptVaultAgeLinkNode` that was added to or
  removed from the current avatar. For `kPublicAgeCreated` and
  `kPublicAgeRemoved`, this is the age file name for the instance whose status
  changed (the exact instance is not identified).

## `OnDefaultKeyCaught`

Behaves like a `plDefaultKeyCatcher` registered via
`plInputInterfaceMgr::SetDefaultKeyCatcher` in C++. Called for every
`plKeyEventMsg` that wasn't handled by anything else.

This callback is only expected to be used by the KI code.

### Parameters

* `ch`: `str` - `plKeyEventMsg::GetKeyChar()`.
* `isDown`: `bool` - `plKeyEventMsg::GetKeyDown()`.
* `isRepeat`: `bool` - `plKeyEventMsg::GetRepeat()`.
* `isShift`: `bool` - `plKeyEventMsg::GetShiftKeyDown()`.
* `isCtrl`: `bool` - `plKeyEventMsg::GetCtrlKeyDown()`.
* `keycode`: `plKeyDef` as `int` - `plKeyEventMsg::GetKeyCode()`.

## `OnMarkerMsg`

Corresponds to `pfMarkerMsg`. Called when the current avatar captures (collides
with) a marker.

This callback is only expected to be used by the KI/marker mission code.

### Parameters

* `msgType`: `PtMarkerMsgType` as `int` - `pfMarkerMsg::fType`. Currently always
  `kMarkerCaptured`.
* `tupData`: `tuple[int]` - A 1-element tuple containing the ID of the captured
  marker.

## `OnBackdoorMsg`

Corresponds to `pfBackdoorMsg`. Called by the `Python.Backdoor` console command,
executed by either the local player or another player in the same age instance.
Only supported by internal clients.

### Parameters

* `target`: `str` - `pfBackdoorMsg::GetTarget()`. First argument of the `Python.Backdoor` command.
* `param`: `str` - `pfBackdoorMsg::GetString()`. Second argument of the `Python.Backdoor` command,
  if any, otherwise an empty string.

## `OnBehaviorNotify`

Corresponds to `plAvatarBehaviorNotifyMsg`.

### Parameters

* `type`: `PtBehaviorTypes` as `int` - `plAvatarBehaviorNotifyMsg::fType`.
* `avatar`: `ptSceneobject | None` - Avatar object whose `plArmatureMod`
  caused the notification, or `None` if the object couldn't be determined for
  some reason.
* `state`: `bool` - `plAvatarBehaviorNotifyMsg::state`. Whether the behavior is
  starting (`True`) or stopping (`False`).

## `OnLOSNotify`

Corresponds to `plLOSHitMsg`. Called in response to `PtRequestLOSScreen`. The
`selfKey` passed to `PtRequestLOSScreen` must be this Python modifier's
`self.key`, otherwise it will not receive this callback!

### Parameters

* `id`: `int` - `plLOSHitMsg::fRequestID`.
* `noHitFlag`: `bool` - `plLOSHitMsg::fNoHit`.
* `sceneobject`: `ptSceneobject | None` - `plLOSHitMsg::fObj`. If this key is
  nil, `sceneobject` is `None`.
* `hitPoint`: `ptPoint3 | None` - `plLOSHitMsg::fHitPoint`. If `sceneobject` is
  `None`, then `hitPoint` is also `None`.
* `distance`: `float` - `plLOSHitMsg::fDistance`.

## `BeginAgeUnLoad`

Corresponds to `plAgeBeginLoadingMsg`. Called when the game begins unloading the
current age, which happens after the link-out animation has finished and after
the client has already disconnected from the game server.

Despite the name of the callback method, the underlying `plAgeBeginLoadingMsg`
is sent not just before unloading an age, but also before *loading* an age. In
principle, `BeginAgeUnLoad` is called for both cases, and there's no way to tell
the two apart, because the `plAgeBeginLoadingMsg::fLoading` field isn't passed
to the callback method. However, most Python modifiers will only receive unload
notifications, because when `plAgeBeginLoadingMsg` is sent at the start of the
loading process, none of the `plPythonFileMod` objects from the age have been
loaded yet.

### Parameters

* `avatar`: `ptSceneobject` - The current avatar object.

## `OnMovieEvent`

Corresponds to `pfMovieEventMsg`. Called when a `ptMoviePlayer` finishes playing
a movie.

This callback is only expected to be used by the global GUI code.

### Parameters

* `movieName`: `str` - `pfMovieEventMsg::fMovieName`. Movie file path originally
  passed to `ptMoviePlayer`.
* `reason`: `PtMovieEventReason` as `int` - `pfMovieEventMsg::fReason`.
  Currently always `kMovieDone`.

## `OnScreenCaptureDone`

Corresponds to `plCaptureRenderMsg`. Called when a screenshot requested via
`PtStartScreenCapture` has been captured. The `selfKey` passed to
`PtStartScreenCapture` must be this Python modifier's `self.key`, otherwise it
will not receive this callback!

This callback is only expected to be used by the KI or global GUI code.

### Parameters

* `image`: `ptImage` - `plCaptureRenderMsg::GetMipmap()`. The screenshot image.

## `OnClimbingBlockerEvent`

Corresponds to `plClimbEventMsg`. Called when an avatar climbing the Gahreesen
wall touches a blocker and falls off.

### Parameters

* `blocker`: `ptSceneobject` - Sender of the message. The blocker object that
  was hit.

## `OnAvatarSpawn`

Corresponds to `plAvatarSpawnNotifyMsg`. Called when an armature (player avatar
or NPC) has spawned at, or been warped to, a spawn point. No details are
provided about which armature was spawned/warped or where.

### Parameters

* `arg`: `bool` - Currently always true.

## `OnAccountUpdate`

Corresponds to `plAccountUpdateMsg`. Called when the player selects, creates, or
deletes an avatar, or when a password change requested via `PtChangePassword`
finishes. Theoretically also called when one of the player's avatars is upgraded
from visitor to explorer, but this is no longer relevant now that all avatars
are always explorers.

This callback is only expected to be used by the KI or global GUI code.

### Parameters

* `updateType`: `PtAccountUpdateType` as `int` -
  `plAccountUpdateMsg::GetUpdateType()`. Type of event that happened.
* `result`: `ENetError` as `int` - `plAccountUpdateMsg::GetResult()`. 0 on
  success, non-zero on error.
* `playerID`: `int` - `plAccountUpdateMsg::GetPlayerInt()`. KI number of the
  avatar in question, or 0 for events not related to an avatar.

## `gotPublicAgeList`

Corresponds to `plNetCommPublicAgeListMsg`. Called when the server replies to a
`PtGetPublicAgeList` query.

Currently, all Python modifiers receive `gotPublicAgeList` callbacks for all
`PtGetPublicAgeList` calls - there is no way to direct one call's callback to
one particular Python modifier.

This callback is mainly used by the Nexus.

### Parameters

* `ages`: `Sequence[tuple[ptAgeInfoStruct, int, int]]` - Public age instances
  found for the age name passed to `PtGetPublicAgeList`. Each tuple corresponds
  to one public age instance and has the following elements:
  
  * \[0] `ageInfo`: `ptAgeInfoStruct` - Static information about the instance.
  * \[1] `population`: `int` - `NetAgeInfo::currentPopulation`. How many avatars
    are currently in the instance.
  * \[2] `owners`: `int` - `NetAgeInfo::population`. How many owners the
    instance has. For neighborhood instances, this is the number of members the
    neighborhood has. For other public instances, this number is meaningless.

## `OnAIMsg`

Corresponds to the subclasses of `plAIMsg`. Called for events related to NPC
armatures with a "critter brain". Currently, this is only used for Ahnonay
quabs.

### Parameters

* `brain`: `ptCritterBrain | None` - The "critter brain" of the armature that
  caused the event, or `None` if the responsible brain couldn't be found.
* `msgType`: `PtAIMsgType` as `int` - Indicates the specific message class - see
  [below](#aimsg-types-and-arguments).
* `userStr`: `str` - `plAIMsg::BrainUserString()`. The "user string" of the
  armature that caused the event, which is either passed to `PtLoadAvatarModel`
  or set in a `plLoadAvatarMsg` saved in a PRP. Can be used to easily identify a
  specific armature. Defaults to an empty string if no "user string" was set.
* `args`: `tuple[Any, ...] | None` - Depends on `msgType` - see
  [below](#aimsg-types-and-arguments).

### AIMsg types and arguments

#### `kBrainCreated`

Corresponds to `plAIBrainCreatedMsg`. No `args`.

#### `kArrivedAtGoal`

Corresponds to `plAIArrivedAtGoalMsg`.

* \[0] `goal`: `ptPoint3` - `plAIArrivedAtGoalMsg::Goal()`.

#### `kBrainDestroyed`

Corresponds to `plAIBrainDestroyedMsg`. No `args`.

#### `kGoToGoal`

Corresponds to `plAIGoToGoalMsg`.

* \[0] `goal`: `ptPoint3` - `plAIGoToGoalMsg::Goal()`.
* \[1] `avoidingAvatars`: `bool` - `plAIGoToGoalMsg::AvoidingAvatars()`.

## `OnGameScoreMsg`

Corresponds to the subclasses of `pfGameScoreMsg`. Called in response to
some `ptGameScore` methods. The `key` passed to the `ptGameScore` method must be
this Python modifier's `self.key`, otherwise it will not receive this callback!

### Parameters

* `msg`: `ptGameScoreMsg` - The received message. The exact class depends on the
  `ptGameScore` method.

## `OnSubtitleMsg`

Corresponds to `plSubtitleMsg`. Called when a line of subtitles needs to be
displayed. Only called if the player has subtitles enabled in the settings and
when the sound object in question is close enough to be audible.

This callback is only expected to be used by the KI code.

### Parameters

* `text`: `str` - `plSubtitleMsg::GetText()`. The subtitle text line.
* `speaker`: `str` - `plSubtitleMsg::GetSpeaker()`. Name of the person speaking.
