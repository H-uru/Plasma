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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#ifndef cyMisc_h
#define cyMisc_h

/////////////////////////////////////////////////////////////////////////////
//
// NAME: cyMisc
//
// PURPOSE: Class wrapper to map misc functions, such as the console
//
class pyKey;
class pySceneObject;
class pyPlayer;
class pyImage;
class pyDniCoordinates;
class pyColor;
class pyAgeInfoStruct;
class pyPoint3;

#include <python.h>
#include "hsTypes.h"
#include "hsStlUtils.h"
#include "../pnUtils/pnUtils.h"

class pyGUIDialog;
class plPipeline;
class plDisplayMode;
struct PipelineParams;

class cyMisc
{

	// game playing to get the pipeline from the client when we can't really
	// this is only for the C++ side
	// The pipeline is set in the plClient
	static plPipeline* fPipeline;
	static UInt32	fUniqueNumber;
	static UInt32	fPythonLoggingLevel;

public:
	// periodically do things
	static void Update( double secs );

	// the python definitions
	static void AddPlasmaMethods(std::vector<PyMethodDef> &methods);
	static void AddPlasmaMethods2(std::vector<PyMethodDef> &methods);
	static void AddPlasmaMethods3(std::vector<PyMethodDef> &methods);
	static void AddPlasmaMethods4(std::vector<PyMethodDef> &methods);

	static void AddPlasmaConstantsClasses(PyObject *m);


	static void			SetPipeline( plPipeline *pipe ) { fPipeline = pipe; }
	static plPipeline	*GetPipeline( void ) { return fPipeline; }


#if 1
	//
	// TEMP SCREEN PRINT CODE FOR NON-DBG TEXT DISPLAY
	//
public:
	static void PrintToScreen(const char* msg);
#endif

	enum PythonDebugPrintLevels
	{
		kNoLevel = 0,
		kDebugDump = 1,
		kWarningLevel = 2,
		kErrorLevel = 3,
		kAssertLevel = 4
	};

	enum LOSObjectType
	{
		kClickables,
		kCameraBlockers,
		kCustom,
		kShootable
	};

	static UInt32 GetPythonLoggingLevel();
	static void SetPythonLoggingLevel(UInt32 new_level);
	
	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : Console
	//  PARAMETERS : command   - string of console commmand to execute
	//
	//  PURPOSE    : Execute a console command from a python script,
	//					optionally propagate over the net
	//
	static void Console(const char* command);
	static void ConsoleNet(const char* command, hsBool netForce); 

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : FindSceneObject
	//  PARAMETERS : name   - string of name of the sceneobject
	//             : ageName - string of the name of the age to look in
	//
	//  PURPOSE    : Execute a console command from a python script,
	//					optionally propagate over the net
	//
	static PyObject* FindSceneObject(const char* name, const char* ageName); // returns pySceneObject
	static PyObject* FindActivator(const char* name); // returns pyKey

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : PopUpConsole
	//  PARAMETERS : command   - string of console commmand to execute
	//
	//  PURPOSE    : Execute a console command from a python script
	//
	static void PopUpConsole(const char* command);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : TimerCallback
	//  PARAMETERS : command   - string of console commmand to execute
	//
	//  PURPOSE    : Execute a console command from a python script
	//
	static void TimerCallback(pyKey& selfkey, hsScalar time, UInt32 id);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : ClearTimerCallbacks
	//  PARAMETERS : key and (optional) timer id
	//
	//  PURPOSE    : clears timer callbacks
	//
	static void ClearTimerCallbacks(pyKey& selfkey);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : AttachObject
	//  PARAMETERS : child object
	//             :   to be attached to parent object
	//
	//  PURPOSE    : Attach an object to another object, knowing only their pyKeys
	//
	static void AttachObject(pyKey& ckey, pyKey& pkey);
	static void AttachObjectSO(pySceneObject& cobj, pySceneObject& pobj);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : DetachObject
	//  PARAMETERS : child object
	//             :   to be attached to parent object
	//
	//  PURPOSE    : Attach an object to another object, knowing only their pyKeys
	//
	static void DetachObject(pyKey& ckey, pyKey& pkey);
	static void DetachObjectSO(pySceneObject& cobj, pySceneObject& pobj);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : LinkToAge
	//  PARAMETERS : 
	//
	//  PURPOSE    : LinkToAge
	//
	//  STATUS     : Depreciated. Use plNetLinkingMgr or pyNetLinkingMgr instead.
	//
///	static void LinkToAge(pyKey &selfkey, const char *AgeName,const char *SpawnPointName);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : SetDirtySyncStateServer
	//  PARAMETERS : 
	//
	//  PURPOSE    : set the Python modifier to be dirty and asked to be saved out
	//
	static void SetDirtySyncState(pyKey &selfkey, const char* SDLStateName, UInt32 sendFlags);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : SetDirtySyncStateClients
	//  PARAMETERS : 
	//
	//  PURPOSE    : set the Python modifier to be dirty and asked to be saved out
	//					specifies that state should be sent to other clients as well as server
	//
	static void SetDirtySyncStateWithClients(pyKey &selfkey, const char* SDLStateName, UInt32 sendFlags);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : EnableControlKeyEvents & DisableControlKeyEvents
	//  PARAMETERS : none
	//
	//  PURPOSE    : register and unregister for control key events
	//
	static void EnableControlKeyEvents(pyKey &selfkey);
	static void DisableControlKeyEvents(pyKey &selfkey);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : GetClientName
	//  PARAMETERS : avatar key
	//
	//  PURPOSE    : Return the net client (account) name of the player whose avatar
	//				key is provided.
	//
	static hsBool WasLocallyNotified(pyKey &selfkey);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : GetClientName
	//  PARAMETERS : avatar key
	//
	//  PURPOSE    : Return the net client (account) name of the player whose avatar
	//				key is provided.
	//
	static const char* GetClientName(pyKey &avKey);

	static PyObject* cyMisc::GetAvatarKeyFromClientID(int clientID); // returns pyKey
	static int cyMisc::GetLocalClientID();
	static int cyMisc::GetClientIDFromAvatarKey(pyKey& avatar);
	static hsBool cyMisc::ValidateKey(pyKey& key);



	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : GetClientName
	//  PARAMETERS : 
	//
	//  PURPOSE    : Return the local net client (account) name
	//
	static const char* GetLocalClientName();


	//
	// Get Current age information - DEPRECIATED. Use ptDniInfoSource() object instead.
	//
	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : GetAgeName
	//  Function   : GetAgeTime
	//  PARAMETERS : 
	//
	//  PURPOSE    : Return the age name of the current age the local player is in
	//             : Return the current coordinates of the player within this age
	//             : Return the current time with the current age the player is in
	//
	static const char* GetAgeName();
	static PyObject* GetAgeInfo(); // returns pyAgeInfoStruct
	static const char* GetPrevAgeName();
	static PyObject* GetPrevAgeInfo();
	// current time in current age
	static UInt32 GetAgeTime( void );
	static UInt32 GetDniTime(void);
	static UInt32 ConvertGMTtoDni(UInt32 time);
	static UInt32 GetServerTime( void ); // returns the current server time in GMT
	static float GetAgeTimeOfDayPercent(void);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : ExcludeRegionSet
	//  PARAMETERS : key - of the exclude region, ie. where to send the message
	//               state  - what state of to set at:
	//
	//  PURPOSE    : Sets the state of an exclude region
	//
	enum
	{
		kExRegRelease = 0,
		kExRegClear = 1,
	};
	static void ExcludeRegionSet(pyKey& sender, pyKey& exKey, UInt16 state);
	static void ExcludeRegionSetNow(pyKey& sender, pyKey& exKey, UInt16 state);


	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : GetSeconds
	//  PARAMETERS : 
	//
	//  PURPOSE    : Return the nunber of seconds elapsed
	//
	static double GetSeconds();
	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : GetSysSeconds
	//  PARAMETERS : 
	//
	//  PURPOSE    : Return the number of system seconds elapsed
	//
	static double GetSysSeconds();
	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : GetDelSysSeconds
	//  PARAMETERS : 
	//
	//  PURPOSE    : Return the frame delta seconds
	//
	static hsScalar GetDelSysSeconds();


	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : LoadDialog
	//  PARAMETERS : 
	//
	//  PURPOSE    : Loads the dialog by name
	//             : optionally sets the receiver key for the GUINotifyMsg
	//
	static void LoadDialog(const char* name);
	static void LoadDialogK(const char* name, pyKey& modKey);
	static void LoadDialogKA(const char* name, pyKey& rKey, const char* ageName);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : UnLoadDialog
	//  PARAMETERS : 
	//
	//  PURPOSE    : UnLoads the dialog by name
	//             : optionally sets the receiver key for the GUINotifyMsg
	//
	static void UnloadDialog(const char* name);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : IsDialogLoaded
	//  PARAMETERS : 
	//
	//  PURPOSE    : Test to see if a dialog is loaded (according to the dialog manager)
	//
	static hsBool IsDialogLoaded(const char* name);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : ShowDialog
	//  Function   : HideDialog
	//  PARAMETERS : 
	//
	//  PURPOSE    : Show or Hide a dialog by name
	//
	static void ShowDialog(const char* name);
	static void HideDialog(const char* name);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : GetDialogFromTagID
	//  PARAMETERS : 
	//
	//  PURPOSE    : Return the frame delta seconds
	//
	static PyObject* GetDialogFromTagID(UInt32 tag); // returns pyGUIDialog
	static PyObject* GetDialogFromString(const char* name); // returns pyGUIDialog

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : IsGUIModal
	//  PARAMETERS : 
	//
	//  PURPOSE    : Returns true if the GUI is currently modal (and therefore blocking input)
	//
	static bool IsGUIModal();

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : GetLocalAvatar
	//  PARAMETERS : 
	//
	//  PURPOSE    : Return a pySceneobject of the local Avatar
	//
	static PyObject* GetLocalAvatar(); // returns pySceneObject
	static PyObject* GetLocalPlayer(); // returns pyPlayer

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : GetPlayerList
	//  Function   : GetPlayerListDistanceSorted
	//  PARAMETERS : 
	//
	//  PURPOSE    : Get a list of players (other than self) that are playing the game
	//             : optionally get it sorted by distance
	//
	//  RETURNS    : Python object list of pyPlayer s
	//
	static std::vector<PyObject*> GetPlayerList(); // list of pyPlayer
	static std::vector<PyObject*> GetPlayerListDistanceSorted(); // list of pyPlayer

	static UInt32 GetMaxListenListSize();
	static hsScalar GetMaxListenDistSq();

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : SendRTChat
	//  PARAMETERS : from   - is a pyPlayer of the person who is sending this
	//             : tolist  - is a python list object, if empty then broadcast message
	//             : message  - text string to send to others
	//             : flags   - the flags of destiny... whatever that means
	//
	//  PURPOSE    : To send a real time chat message to a particualr user, a list of users
	//             : or broadcast it to everyone (within hearing distance?)
	//
	//  RETURNS    : the flags that were sent with the message (may be modified)
	//
	static UInt32 SendRTChat(pyPlayer& from, const std::vector<pyPlayer*> & tolist, const char* message, UInt32 flags);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : SendKIMessage
	//  PARAMETERS : command   - the command type
	//             : value     - extra value
	//
	//  PURPOSE    : Send message to the KI, to tell it things to do
	//
	//  RETURNS    : nothing
	//
	static void SendKIMessage(UInt32 command, hsScalar value);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : SendKIMessageS
	//  PARAMETERS : command   - the command type
	//             : value     - extra value as a string
	//
	//  PURPOSE    : Send message to the KI, to tell it things to do
	//
	//  RETURNS    : nothing
	//
	static void SendKIMessageS(UInt32 command, const wchar_t* value);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : SendKIMessageI
	//  PARAMETERS : command   - the command type
	//             : value     - extra value as an Int32
	//
	//  PURPOSE    : Send message to the KI, to tell it things to do
	//
	//  RETURNS    : nothing
	//
	static void SendKIMessageI(UInt32 command, Int32 value);
	static void SendKIGZMarkerMsg(Int32 markerNumber, pyKey& sender);
	static void SendKIRegisterImagerMsg(const char* imagerName, pyKey& sender);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : YesNoDialog
	//  PARAMETERS : sender    - sender's key (to get the reply)
	//             : value     - extra value
	//
	//  PURPOSE    : Send message to the KI, to tell it things to do
	//
	//  RETURNS    : nothing
	//
	static void YesNoDialog(pyKey& sender, const char* thestring);
	static void YesNoDialog(pyKey& sender, std::wstring thestring);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : RateIt
	//  PARAMETERS : chonicleName    - where to store the rating
	//             : thestring       - the message in the RateIt dialog
	//             : onceFlag        - flag to tell whether this is a onetime thing or ask everytime
	//
	//  PURPOSE    : Send message to the KI to tell it to ask the user to Rate something
	//
	//  RETURNS    : nothing
	//
	static void RateIt(const char* chronicleName, const char* thestring, hsBool onceFlag);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : SetPrivateChatList
	//  PARAMETERS : key			- who's joining
	//
	//  PURPOSE    : Lock the local avatar into private vox messaging, and / or add new memebers to his chat list
	//
	//  RETURNS    : nothing
	//
	static void SetPrivateChatList(const std::vector<pyPlayer*> & tolist);
	
	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : RemovePrivateChatMember
	//  PARAMETERS : key			- who's joining
	//
	//  PURPOSE    : Remove the local avatar from private vox messaging, and / or clear memebers from his chat list
	//
	//  RETURNS    : nothing
	//
	static void ClearPrivateChatList(pyKey& member);
	
	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : ClearCameraStack
	//  PARAMETERS : 
	//
	//  PURPOSE    : knocks all the cameras off the current stack
	//
	static void cyMisc::ClearCameraStack();

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : SendPetitionToCCR
	//  PARAMETERS : message  - message to send to the CCR ("please help me!")
	//
	//  PURPOSE    : Send a petition to the CCR for help or questions
	//
	static void SendPetitionToCCR(const char* message);
	static void SendPetitionToCCRI(const char* message, UInt8 reason,const char* title);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : SendChatToCCR
	//  PARAMETERS : message  - message to send to the CCR ("please help me!")
	//
	//  PURPOSE    : Send a petition to the CCR for help or questions
	//
	static void SendChatToCCR(const char* message,Int32 CCRPlayerID);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : GetNumRemotePlayers
	//  
	//  PURPOSE    : return the number of remote players connected
	//
	static int GetNumRemotePlayers();

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : Paging functions
	//  PARAMETERS : nodeName  - name of the page to load
	//  
	//  PURPOSE    : page in, or out a paritcular node
	//
	static void PageInNodes(const std::vector<std::string> & nodeNames, const char* age);
	static void PageOutNode(const char* nodeName);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : LimitAvatarLOD
	//  PARAMETERS : LODlimit - number of to limit the LOD to
	//  
	//  PURPOSE    : sets the avatar LOD limit
	//
	static void LimitAvatarLOD(int LODlimit);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : Set fog default functions
	//  PARAMETERS : floats  - the parameters
	//  
	//  PURPOSE    : sets the fog defaults
	//
	static void FogSetDefColor(pyColor& color);
	static void FogSetDefLinear(float start, float end, float density);
	static void FogSetDefExp(float end, float density);
	static void FogSetDefExp2(float end, float density);
	static void SetClearColor(float red, float green, float blue);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : Enable / disable cursor fade for avatar
	//  PARAMETERS : 
	//
	//  PURPOSE    : turns avatar fade out on / off 
	//
	static void EnableAvatarCursorFade();
	static void DisableAvatarCursorFade();
	static void FadeLocalPlayer(hsBool fade);

	
	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : Put the interface into 'offer book mode'
	//  PARAMETERS : 
	//
	//  PURPOSE    :  
	//
	static void EnableOfferBookMode(pyKey& selfkey, const char* ageFileName, const char* ageInstanceName);
	static void DisableOfferBookMode();
	static void NotifyOffererPublicLinkAccepted(UInt32 offerer);
	static void NotifyOffererPublicLinkRejected(UInt32 offerer);
	static void NotifyOffererPublicLinkCompleted(UInt32 offerer);
	static void ToggleAvatarClickability(hsBool on);
	static void SetShareSpawnPoint(const char* spawnPoint);
	static void SetShareAgeInstanceGuid(const Uuid& guid);
	
	//////////////////////////////////////////////////////////////////////////////
	//
	// Function   : IsCCRAwayStatus
	// PARAMETERS :
	//
	// PURPOSE	  : Returns current status of CCR dept
	//
	static hsBool IsCCRAwayStatus();

	//////////////////////////////////////////////////////////////////////////////
	//
	// Function   : AmCCR
	// PARAMETERS :
	//
	// PURPOSE	  : Returns true if local player is a CCR
	//
	static hsBool AmCCR();

	//////////////////////////////////////////////////////////////////////////////
	//
	// Function   : AcceptInviteInGame
	// PARAMETERS : Friend's Name, Invite Key
	//
	// PURPOSE	  : Send's a VaultTask to the server to perform the invite
	//
	static void AcceptInviteInGame(const char* friendName, const char* inviteKey);
	
	//////////////////////////////////////////////////////////////////////////////
	//
	// Function   : GetLanguage
	// PARAMETERS :
	//
	// PURPOSE    : Returns the current language the game is in
	//
	static int GetLanguage();
	
	//////////////////////////////////////////////////////////////////////////////
	//
	// Function   : UsingUnicode
	// PARAMETERS :
	//
	// PURPOSE    : Returns true if the current language uses Unicode (like Japanese)
	//
	static bool UsingUnicode();

	//////////////////////////////////////////////////////////////////////////////
	//
	// Function   : RequestLOSScreen
	// PARAMETERS : lots...
	//
	// PURPOSE    : To request an LOS from a point on the screen
	//
	static bool RequestLOSScreen(pyKey &selfkey, Int32 ID, hsScalar xPos, hsScalar yPos, hsScalar distance, int what, int reportType);

	//////////////////////////////////////////////////////////////////////////////
	//
	// Function   : CheckVisLOS
	// PARAMETERS : StartPoint, EndPoint
	//
	// PURPOSE    : Check is there is something visible in the path from StartPoint to EndPoint
	//
	static PyObject* CheckVisLOS(pyPoint3 startPoint, pyPoint3 endPoint);
	static PyObject* CheckVisLOSFromCursor();

	//////////////////////////////////////////////////////////////////////////////
	//
	// Function   : particle system management
	//
	static void TransferParticlesToKey(pyKey& fromKey, pyKey& toKey, int numParticles);
	static void SetParticleDissentPoint(float x, float y, float z, pyKey& particles);
	static void SetParticleOffset(float x, float y, float z, pyKey& particles);
	static void KillParticles(float time, float pct, pyKey& particles);
	static int  GetNumParticles(pyKey& host);
	static void SetLightColorValue(pyKey& light, std::string lightName, hsScalar r, hsScalar g, hsScalar b, hsScalar a);
	static void SetLightAnimationOn(pyKey& light, std::string lightName, hsBool start);
	//////////////////////////////////////////////////////////////////////////////
	//
	// Function   : RegisterForControlEventMessages
	// PARAMETERS : switch on or off, registrant
	//
	// PURPOSE	  : let you get control event messages at will (for pseudo-GUI's like the psnl bookshelf or clft imager)

	static void RegisterForControlEventMessages(hsBool on, pyKey& k);

	//////////////////////////////////////////////////////////////////////////////
	//
	// Function   : IsSinglePlayerMode
	// And        : IsDemoMode
	// PARAMETERS :
	//
	// PURPOSE    : Returns whether the game is in Single Player mode
	// And        : returns whether the game in in Demo mode
	//
	static bool IsSinglePlayerMode();
	static bool IsDemoMode();
	
	//////////////////////////////////////////////////////////////////////////////
	//
	// Function   : IsInternalRelease
	// PARAMETERS :
	//
	// PURPOSE    : Returns true if we are running an internal build
	//
	static bool IsInternalRelease();

	//////////////////////////////////////////////////////////////////////////////
	//
	// Function   : IsEnterChatModeKeyBound
	// PARAMETERS :
	//
	// PURPOSE    : Returns whether the EnterChatMode is bound to a key
	//
	static bool IsEnterChatModeKeyBound();

	//////////////////////////////////////////////////////////////////////////////
	//
	// Function   : ShootBulletFromScreen
	// PARAMETERS : lots...
	//
	// PURPOSE    : Shoots from screen coordinates, a bullet and makes a mark on objects that know about bullet holes
	//
	static void ShootBulletFromScreen(pyKey &selfkey, hsScalar xPos, hsScalar yPos, hsScalar radius, hsScalar range);

	//////////////////////////////////////////////////////////////////////////////
	//
	// Function   : ShootBulletFromObject
	// PARAMETERS : lots...
	//
	// PURPOSE    : Shoots from an object, a bullet and makes a mark on objects that know about bullet holes
	//
	static void ShootBulletFromObject(pyKey &selfkey, pySceneObject* sobj, hsScalar radius, hsScalar range);

	//////////////////////////////////////////////////////////////////////////////
	//
	// Function   : GetPublicAgeList
	// PARAMETERS : ageName, callback object
	//
	// PURPOSE    : Get the list of public ages for the given age name.
	//
	static void GetPublicAgeList( const char * ageName, PyObject * cbObject=nil );

	//////////////////////////////////////////////////////////////////////////////
	//
	// Function   : CreatePublicAge
	// PARAMETERS : ageInfo, callback object
	//
	// PURPOSE    : Add a public age to the list of available ones.
	//
	static void CreatePublicAge( pyAgeInfoStruct * ageInfo, PyObject * cbObject=nil );

	//////////////////////////////////////////////////////////////////////////////
	//
	// Function   : RemovePublicAge
	// PARAMETERS : ageInstanceGuid, callback object
	//
	// PURPOSE    : Remove a public age from the list of available ones.
	//
	static void RemovePublicAge( const char * ageInstanceGuid, PyObject * cbObject=nil );

	//////////////////////////////////////////////////////////////////////////////
	//
	// Function   : GetKILevel
	//
	// PURPOSE    : returns the ki level of the local player
	//
	static int GetKILevel(); // return local player's ki level

	//////////////////////////////////////////////////////////////////////////////
	//
	// 
	// PURPOSE    : these functions are for saving / restoring the camera stack
	//

	static int GetNumCameras();
	static const char* GetCameraNumber(int number);
	static void RebuildCameraStack(const char* name, const char* ageName);
	static void PyClearCameraStack();
	static void RecenterCamera();
	static bool IsFirstPerson();

	//////////////////////////////////////////////////////////////////////////////
	//
	// Function   : FadeIn and FadeOut
	//
	// PURPOSE    : fade screen in and out
	//
	static void FadeIn(float lenTime, bool holdFlag, bool noSound = 0);
	static void FadeOut(float lenTime, bool holdFlag, bool noSound = 0);


	//////////////////////////////////////////////////////////////////////////////
	//
	// Function   : Disable / Enable global clickability
	//
	// PURPOSE    : globally enable or disable ALL clickables on this local client
 	//
	static void SetClickability(hsBool b);

	
	//////////////////////////////////////////////////////////////////////////////
	//
	// Function   : Debug build only: Assert if condition is false.
	//
	// PURPOSE    : debugging
 	//
	static void DebugAssert( bool cond, const char * msg );


	//////////////////////////////////////////////////////////////////////////////
	//
	// Function   : Set a python object to be called back after a certain amount of time.
	//				Python object should have this method: def onAlarm(self,context)
	//
	// PURPOSE    : script can trigger itself over time w/o having to specify it in the dataset.
 	//
	static void	SetAlarm( float secs, PyObject * cb, UInt32 cbContext );
	
	//////////////////////////////////////////////////////////////////////////////
	//
	// Function   : Save Screen Shot
	//
	// PURPOSE    : captures the screen and saves it as a jpeg
 	//
	static void SaveScreenShot(const char* fileName, int x = 640, int y = 480, int quality = 75);

	//////////////////////////////////////////////////////////////////////////////
	//
	// Function   : Start a screen capture
	//
	// PURPOSE    : This starts a screen capture in motion. It will be capture on the next
	//              update and a plCaptureRenderMsg when its ready
	//
	static void StartScreenCapture(pyKey& selfkey);
	static void StartScreenCaptureWH(pyKey& selfkey, UInt16 width, UInt16 height);

	//////////////////////////////////////////////////////////////////////////////
	//
	// Function   : wear maintainer suit
	//
	// PURPOSE    : puts the maintainer suit on or off the avatar
	//
	static void WearMaintainerSuit(pyKey& key, hsBool wear);
	
	static void WearDefaultClothing(pyKey& key);
	static void WearDefaultClothingType(pyKey& key, UInt32 type);

	//////////////////////////////////////////////////////////////////////////////
	//
	// Function   : Fake link to object
	//
	// PURPOSE    : takes an avatar key and an object key and fake-links the avatar
	//				to that object's position.  appears to be a link to other players
	//
	static void FakeLinkToObject(pyKey& avatar, pyKey& object);	
	static void FakeLinkToObjectNamed(const char* name);	
	
	//////////////////////////////////////////////////////////////////////////////
	//
	// Function   : Spawn an avatar
	//
	// PURPOSE    : takes the name of an avatar model and a sceneobject key and
	//				spawns the avatar at that point
	//
	static PyObject* LoadAvatarModel(const char* modelName, pyKey& object, const char* userStr); // returns pyKey
	static void UnLoadAvatarModel(pyKey& avatar);
	
	///////////////////////////////////////////////////////////////////////////////
	//
	// Function	  :	Hide/show the mouse cursor
	//
	// PURPOSE	  :	forces the mouse cursor to hide/show, overrides pretty much
	//				everything and should only be used if "normal" methods don't
	//				work (like ptGUIMouseOff)
	static void ForceCursorHidden();
	static void ForceCursorShown();

	///////////////////////////////////////////////////////////////////////////////
	//
	// Function	  : GetLocalizedString
	//
	// PURPOSE	  : Returns the specified localized string with the parameters
	//				properly replaced (the list is a list of unicode strings) Name
	//				is in "Age.Set.Name" format
	//
	static std::wstring GetLocalizedString(std::wstring name, const std::vector<std::wstring> & arguments);

	static void EnablePlanarReflections(bool enable = true);
	static void SetGraphicsOptions(int Width, int Height, int ColorDepth, hsBool Windowed, int NumAASamples, int MaxAnisotropicSamples, hsBool VSync);
	static void GetSupportedDisplayModes(std::vector<plDisplayMode> *res);
	static int GetDesktopWidth();
	static int GetDesktopHeight();
	static int GetDesktopColorDepth();
	static PipelineParams *GetDefaultDisplayParams();

	static bool DumpLogs(const std::wstring & folder);

	static bool FileExists(const std::wstring & filename);
	static bool CreateDir(const std::wstring & directory);

	static std::wstring GetUserPath();
	static std::wstring GetInitPath();

	static void SetBehaviorNetFlags(pyKey & behKey, hsBool netForce, hsBool netProp);
	static void SendFriendInvite(const wchar email[], const wchar toName[]);
	static PyObject* PyGuidGenerate();
	static PyObject* GetAIAvatarsByModelName(const char* name);
	static void ForceVaultNodeUpdate(unsigned nodeId);
	static void VaultDownload(unsigned nodeId);
};

#endif  // cyMisc_h
