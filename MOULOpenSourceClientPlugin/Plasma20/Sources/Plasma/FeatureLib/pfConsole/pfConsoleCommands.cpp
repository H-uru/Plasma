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
//////////////////////////////////////////////////////////////////////////////
//																			//
//	Actual Console Commands and Groups										//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifdef PLASMA_EXTERNAL_RELEASE
#define LIMIT_CONSOLE_COMMANDS 1
#endif


#include "pfConsoleCmd.h"
#include "plgDispatch.h"

#include "../plAgeLoader/plAgeLoader.h"
#include "../plNetClient/plNetClientMgr.h"
#include "../plPipeline/plDebugText.h"
#include "../plPipeline/plPipeDebugFlags.h"
#include "../plMessage/plMovieMsg.h"
#include "../plDrawable/plDrawableSpans.h"
#include "plPipeline.h"
#include "../pfCamera/plCameraModifier.h"
#include "../pfCamera/plVirtualCamNeu.h"
#include "../pfCamera/plCameraBrain.h"
#include "../plResMgr/plResManager.h"
#include "../pnKeyedObject/plFixedKey.h"
#include "../pnKeyedObject/plKey.h"
#include "../pnKeyedObject/plKeyImp.h"
#include "../pnModifier/plLogicModBase.h"
#include "../plModifier/plSDLModifier.h"
#include "../plSDL/plSDL.h"
#include "../pfCharacter/plPlayerModifier.h"
#include "../plSurface/plLayerDepth.h"
#include "../plSurface/plLayerOr.h"
#include "../plSurface/plLayerOr.h"
#include "../plAudio/plAudioSystem.h"
#include "../plAudio/plVoiceChat.h"
#include "../plAudio/plWinMicLevel.h"
#include "../plPipeline/plFogEnvironment.h"
#include "../plPipeline/plPlates.h"
#include "../plPipeline/plDynamicEnvMap.h"
#include "../../NucleusLib/inc/hsTimer.h"
#include "../pnMessage/plClientMsg.h"
#include "../pnMessage/plEnableMsg.h"
#include "../pnMessage/plAudioSysMsg.h"
#include "../plMessage/plListenerMsg.h"
#include "../pfAudio/plListener.h"
#include "../plMessage/plAvatarMsg.h"
#include "../plMessage/plOneShotMsg.h"
#include "../plVault/plVault.h"
#include "../../Apps/plClient/plClient.h"
#include "pfConsole.h"
#include "pfConsoleContext.h"
#include "../plResMgr/plKeyFinder.h"
#include "../plModifier/plSimpleModifier.h"
#include "../plAvatar/plAvatarMgr.h"
#include "../plAvatar/plAvatarTasks.h"
#include "../plAvatar/plAvBrainGeneric.h"
//#include "../plHavok1/plSimulationMgr.h"

#include "../plMessage/plConsoleMsg.h"
#include "../plMessage/plAnimCmdMsg.h"
#include "../pnMessage/plCameraMsg.h"
#include "../pnMessage/plSoundMsg.h"
#include "../pnMessage/plEventCallbackMsg.h"
#include "../pnMessage/plNotifyMsg.h"
#include "../pfAnimation/plAnimDebugList.h"

#include "../pnMessage/plNodeChangeMsg.h"
#include "../pnMessage/plProxyDrawMsg.h"
#include "../pnMessage/plObjRefMsg.h"
#include "../pnMessage/plAttachMsg.h"
#include "../plMessage/plSimInfluenceMsg.h"
#include "../plMessage/plSimStateMsg.h"
#include "../plMessage/plLinkToAgeMsg.h"
#include "../pfMessage/pfKIMsg.h"

#include "../plInputCore/plInputInterfaceMgr.h"
#include "../plInputCore/plInputManager.h"
#include "../plInputCore/plInputDevice.h"
#include "../plInputCore/plAvatarInputInterface.h"
#include "../plMessage/plInputEventMsg.h"
#include "../pnInputCore/plKeyMap.h"

#include "../plParticleSystem/plParticleSystem.h"
#include "../plParticleSystem/plConvexVolume.h"
#include "../plParticleSystem/plParticleEffect.h"
#include "../plParticleSystem/plParticleGenerator.h"
#include "../plSurface/hsGMaterial.h"
#include "../pnSceneObject/plDrawInterface.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "../plScene/plSceneNode.h"
#include "../plScene/plPageTreeMgr.h"
#include "../plScene/plPostEffectMod.h"
#include "../pnMessage/plNodeRefMsg.h"
//#include "../pnMessage/plWarpMsg.h"
#include "hsResMgr.h"

#include "../plParticleSystem/plParticleSystem.h"
#include "../plMessage/plParticleUpdateMsg.h"

#include "../plDrawable/plDynaBulletMgr.h"

#include "../plGImage/plMipmap.h"
#include "../plGImage/plTGAWriter.h"

#include "../plGLight/plShadowCaster.h"
#include "../plGLight/plShadowMaster.h"

// begin for agedefn test
#include "hsStream.h"
#include "../plAgeDescription/plAgeDescription.h"
#include "../plUnifiedTime/plUnifiedTime.h"
//end for agedefn test

#include "../../PubUtilLib/plFile/hsFiles.h"
#include "../../NucleusLib/pnSceneObject/plAudioInterface.h"

#include "../plStatusLog/plStatusLog.h"
#include "../pnTimer/pnBuildDates.h"

#include "hsStlUtils.h"
#include "hsTemplates.h"
#include "../Corelib/hsUtils.h"

#include "../pfPython/cyPythonInterface.h"
#include "../pfPython/plPythonSDLModifier.h"

#include "../plResMgr/plResManagerHelper.h"
#include "../plResMgr/plResMgrSettings.h"
#include "../plResMgr/plLocalization.h"


#define PF_SANITY_CHECK( cond, msg ) { if( !( cond ) ) { PrintString( msg ); return; } }

//// DO NOT REMOVE!!!!
//// This is here so Microsoft VC won't decide to "optimize" this file out
//// DO NOT REMOVE!!!!
void	pfConsoleCmdGroup::Dummy( void )
{
}
//// DO NOT REMOVE!!!!

//// Defining Console Commands ///////////////////////////////////////////////
//
//	You define console commands by using the PF_CONSOLE_CMD macro. The format
//	of the macro is:
//
//		PF_CONSOLE_CMD( groupName, functionName, "paramList", "Help string" )
//
//	Where:
//		- groupName is a string representing what command group the command
//		  is in. Subgroups are specified by an underscore, i.e. to put a command
//		  in the Draw subgroup of the Graphics group, you would specify
//		  "Graphics_Draw". Specifying "" means to put the command in the base
//		  command group--i.e. it has no group.
//		- functionName is required; it specifies the function name (really?!?!).
//		  Function names must be globally unique, so you can't have a Draw
//		  function in both the Graphics and SceneAPI groups. Sorry. :(
//		- paramList specifies the parameters and types to the function.
//		  The smallest list you can have is "", which means "no parameters".
//		  If you have parameters, it must be in a comma-delimited string.
//		  You can either specify types or labels and types, so you can say
//		  "int, float" or "int x, float value". Currently, the labels are only
//		  used when printing out usage strings, but they will be used later
//		  for auto-labeling GUI elements, so please put them in where viable.
//
//		  White space does not matter. Valid types are int, float, char, string
//		  bool (auto-conversion of "true"/"false" strings to 1 or 0) and "...".
//		  "..." is a special type that means the same as the C equivalent:
//		  "there can be zero or more parameters here and I don't care what the
//		  type is" is the gist of it.
//		- helpString is a short description of the function, which currently
//		  isn't used, but will be used in the future when implementing help
//		  (could you have guessed it? :) Please fill it in when the function
//		  name isn't obvious (i.e. SetFogColor doesn't really need one)
//	
//	The actual C code prototype looks like:
//		void	pfConsoleCmd_groupName_functionName( UInt32 numParams, pfConsoleCmdParam *params, 
//														void (*PrintString)( char * ) );
//
//	numParams is exactly what it sounds like. params is an array of console
//	parameter objects, each of which are rather nifty in that they can be cast
//	immediately to whatever type you asked for in your parameter list
//	("paramList" above). So if your paramList was "int", then params[ 0 ]
//	can be cast to an int immediately, such as int x = params[ 0 ];
//	If you attempt to cast a parameter to a type other than the one specified
//	in the paramList, you get an hsAssert saying so, so don't do it! Any
//	parameters that fall under "..." are automagically strings, but they can
//	be cast to any valid type without an assert. So basically, if you want
//	to still do your own conversion, just specify "..." as the entire paramList
//	and treat the params array as if it were an array of strings.
//
//	Thus, the net result of the paramList is that it lets the console engine
//	do the parameter parsing for you. If the paramters given to the function
//	do not match the list (this includes too many or too few parameters), a
//	usage string is printed out and the function is not called. Thus, the
//	ONLY parameter error you can possibly have is casting a parameter object
//	to a type other than you asked for. So don't do it!
//
//	(Note: this makes numParams almost obsolete; the only reason it still has
//	a use is for "...", which of course allows variable number of parameters.)
//
//	PrintString is a function that lets you print output to the on-screen
//	console. It is guaranteed to be non-null. Worst case is that it points
//	to a dummy function that does nothing, but it will *always* be valid.
//
//	To define console command groups, you use the macro:
//
//		PF_CONSOLE_GROUP( group )
//
//	where "group" is the name without quotes of the group you want to create.
//	To create a subgroup inside a group, use:
//
//		PF_CONSOLE_SUBGROUP( parent, group )
//
//	where "parent" is the parent group for the subgroup. "parent" can have
//	underscores in it just like the group of a CONSOLE_CMD, so you can say
//
//		PF_CONSOLE_SUBGROUP( Graphics_Render, Drawing )
//
//	to create the Graphics_Render_Drawing subgroup. All groups must be
//	defined before any commands that are in that group. Note that although
//	the 
//
//////////////////////////////////////////////////////////////////////////////

//
// utility functions
//
//////////////////////////////////////////////////////////////////////////////
plKey FindSceneObjectByName(const char* name, const char* ageName, char* statusStr, bool subString=false);
plKey FindObjectByName(const char* name, int type, const char* ageName, char* statusStr, bool subString=false);
plKey FindObjectByNameAndType(const char* name, const char* typeName, const char* ageName, 
							   char* statusStr, bool subString=false);

//
// Find an object from name, type (int), and optionally age.
// Name can be an alias specified by saying $foo
//
plKey FindObjectByName(const char* name, int type, const char* ageName, char* statusStr, bool subString)
{
	if (!name)
	{
		if (statusStr)
			sprintf(statusStr, "Object name is nil");
		return nil;
	}
	
	if (type<0 || type>=plFactory::GetNumClasses())
	{
		if (statusStr)
			sprintf(statusStr, "Illegal class type val");
		return nil;
	}

	plKey key=nil;
	// Try restricted to our age first, if we're not given an age name. This works
	// around most of the problems associated with unused keys in pages causing the pages to be marked
	// as not loaded and thus screwing up our searches
	if( ageName == nil && plNetClientMgr::GetInstance() != nil )
	{
		const char *thisAge = plAgeLoader::GetInstance()->GetCurrAgeDesc().GetAgeName();
		if (thisAge != nil)
		{
			key = plKeyFinder::Instance().StupidSearch(thisAge, nil, type, name, subString);
			if (key != nil)
			{
				if (statusStr)	
					sprintf(statusStr, "Found Object");
				return key;
			}
		}
	}
	// Fallback
	key = plKeyFinder::Instance().StupidSearch(ageName,nil,type, name, subString);

	if (!key)
	{
		if (statusStr)
			sprintf(statusStr, "Can't find object");
		return nil;
	}
	
	if (!key->ObjectIsLoaded())
	{
		if (statusStr)
			sprintf(statusStr, "Object is not loaded");
	}

	if (statusStr)	
		sprintf(statusStr, "Found Object");

	return key;
}

//
// Find a SCENEOBJECT from name, and optionally age.
// Name can be an alias specified by saying $foo.
// Will load the object if necessary.
//
plKey FindSceneObjectByName(const char* name, const char* ageName, char* statusStr, bool subString)
{
	plKey key=FindObjectByName(name, plSceneObject::Index(), ageName, statusStr, subString);
	
	if (!plSceneObject::ConvertNoRef(key ? key->ObjectIsLoaded() : nil))
	{
		if (statusStr)
			sprintf(statusStr, "Can't find SceneObject");
		return nil;
	}

	return key;
}

//
// Find an object from name, type (string) and optionally age.
// Name can be an alias specified by saying $foo
//
plKey FindObjectByNameAndType(const char* name, const char* typeName, const char* ageName, 
							   char* statusStr, bool subString)
{
	if (!typeName)
	{
		if (statusStr)
			sprintf(statusStr, "TypeName is nil");
		return nil;
	}
	
	return FindObjectByName(name, plFactory::FindClassIndex(typeName), ageName, statusStr, subString);
}

void PrintStringF(void pfun(const char *),const char * fmt, ...)
{
	va_list args;

	char buffy[512];
	va_start(args, fmt);
	vsprintf(buffy, fmt, args);
	va_end(args);
	pfun(buffy);
}

//// plDoesFileExist //////////////////////////////////////////////////////////
//	Utility function to determine whether the given file exists

static hsBool	plDoesFileExist( const char *path )
{
	hsUNIXStream	stream;


	if( !stream.Open( path, "rb" ) )
		return false;

	stream.Close();
	return true;
}


//////////////////////////////////////////////////////////////////////////////
//// Base Commands ///////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#ifndef LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_BASE_CMD( SampleCmd1, "", "Sample command #1" )
{
	// No parameters, enforced (i.e. this function won't get called unless
	// the calling line has no parameters)
}

PF_CONSOLE_BASE_CMD( SampleCmd2, "int myValue", "Sample command #2" )
{
	// One parameter, which is an int. Note that because of the console
	// engine, we no longer have to test for the number of parameters.

	// Since we said "int" as our type, this is perfectly valid
	int		myInt = (int)params[ 0 ] * 5;

	// This will assert on run-time, since it's not an int
	float	myFloat = params[ 0 ];

	// This is also BAD, since we only specified one parameter. It'll assert
	int		test = params[ 1 ];
}

PF_CONSOLE_BASE_CMD( SampleCmd3, "int, ...", "Sample command #3" )
{
	// One parameter, which is an int, plus zero or more paramters.

	// Still perfectly valid
	int		myInt = (int)params[ 0 ] * 5;

	// Note: we have to test numParams here because ... allows no extra
	// params, so we have to make sure we have one. Note: numParams
	// INCLUDES all parameters, in this case our int.
	if( numParams > 1 )
	{
		// This is okay--any parameters via ... are strings
		char	*str = params[ 1 ];

		// This is also okay, since ... allows the params to be cast
		// to any valid type. Note that if the parameter isn't actually
		// an int, it'll behave just like atoi()--i.e. return 0
		int		value = params[ 1 ];
	}
}

#endif // LIMIT_CONSOLE_COMMANDS


#ifndef LIMIT_CONSOLE_COMMANDS

#include "../plMessage/plTransitionMsg.h"

PF_CONSOLE_BASE_CMD( FadeIn, "float len, bool hold", "Sample command #1" )
{
	plTransitionMsg	*msg = TRACKED_NEW plTransitionMsg( plTransitionMsg::kFadeIn, (float)params[ 0 ], (bool)params[ 1 ] );
	plgDispatch::MsgSend( msg );
}

PF_CONSOLE_BASE_CMD( FadeOut, "float len, bool hold", "Sample command #1" )
{
	plTransitionMsg	*msg = TRACKED_NEW plTransitionMsg( plTransitionMsg::kFadeOut, (float)params[ 0 ], (bool)params[ 1 ] );
	plgDispatch::MsgSend( msg );
}

PF_CONSOLE_BASE_CMD( NextStatusLog, "", "Cycles through the status logs" )
{
	plStatusLogMgr::GetInstance().NextStatusLog();
}

PF_CONSOLE_BASE_CMD( PrevStatusLog, "", "Cycles backwards through the status logs" )
{
	plStatusLogMgr::GetInstance().PrevStatusLog();
}


PF_CONSOLE_BASE_CMD( ShowStatusLog, "string logName", "Advances the status log display to the given log" )
{
	plStatusLogMgr::GetInstance().SetCurrStatusLog( params[ 0 ] );
}

#endif // LIMIT_CONSOLE_COMMANDS


PF_CONSOLE_BASE_CMD( DisableLogging, "", "Turns off logging" )
{
	plStatusLog::fLoggingOff = true;
}


PF_CONSOLE_BASE_CMD( EnableLogging, "", "Turns on logging" )
{
	plStatusLog::fLoggingOff = false;
}

PF_CONSOLE_BASE_CMD( DumpLogs, "string folderName", "Dumps all current logs to the folder specified, relative to the log folder" )
{
	plStatusLogMgr::GetInstance().DumpLogs( params[ 0 ] );
}


//////////////////////////////////////////////////////////////////////////////
//// Stat Gather Commands ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#ifndef LIMIT_CONSOLE_COMMANDS


PF_CONSOLE_GROUP( Stats )

#include "../plStatGather/plProfileManagerFull.h"

PF_CONSOLE_CMD( Stats, Show,	// Group name, Function name
				"...",			// Params
				"Shows or hides a given category of statistics.\n"
				"List the valid categories using Stats.ListGroups")
{
	const char* group = nil;
	if (numParams > 0)
		group = params[0];

	if(numParams > 1)
		plProfileManagerFull::Instance().ShowLaps(params[0], params[1]);
	else
		plProfileManagerFull::Instance().ShowGroup(group);
}

PF_CONSOLE_CMD(Stats, ResetMax,	// Group name, Function name
				"",			// Params
				"Resets the max value for all stats")
{
	plProfileManagerFull::Instance().ResetMax();
}

PF_CONSOLE_CMD(Stats, ShowNext, "", "Shows the next group of stats")
{
	plProfileManagerFull::Instance().ShowNextGroup();
}

PF_CONSOLE_CMD(Stats, ShowLaps,
				"string group, string stat",
				"")
{
	plProfileManagerFull::Instance().ShowLaps(params[0], params[1]);
}

PF_CONSOLE_CMD(Stats, ListGroups, "", "Prints the names of all the stat groups to the console")
{
	plProfileManagerFull::GroupSet groups;
	plProfileManagerFull::Instance().GetGroups(groups);

	plProfileManagerFull::GroupSet::iterator it;
	for (it = groups.begin(); it != groups.end(); it++)
		PrintString((char*)*it);
}

PF_CONSOLE_CMD(Stats, ListLaps, "", "Prints the names of all the stats with laps to the console")
{
	plProfileManagerFull::LapNames laps;
	plProfileManagerFull::Instance().GetLaps(laps);

	for (int i = 0; i < laps.size(); i++)
	{
		char buf[256];
		sprintf(buf, "%s - %s", laps[i].group, laps[i].varName);
		PrintString(buf);
	}
}

PF_CONSOLE_CMD(Stats, SetLapMin, "int min", "Sets the minimum index of which lap will display")
{
	plProfileManagerFull::Instance().SetMinLap((int)params[0]);
}


PF_CONSOLE_CMD(Stats, PageDownLaps, "", "Show the next page of laps")
{
	plProfileManagerFull::Instance().PageDownLaps();
}


PF_CONSOLE_CMD(Stats, PageUpLaps, "", "Show the previous page of laps")
{
	plProfileManagerFull::Instance().PageUpLaps();
}

PF_CONSOLE_CMD(Stats, SetAvgTime, "int ms", "Sets the amount of time stats are averaged over")
{
	plProfileManager::Instance().SetAvgTime((int)params[0]);
}

PF_CONSOLE_CMD(Stats, Graph, "string stat, int min, int max", "Graphs the specified stat")
{
	plProfileManagerFull::Instance().CreateGraph(params[0], (int)params[1], (int)params[2]);
}

PF_CONSOLE_CMD(Stats, ShowDetail, "", "Shows the detail stat graph")
{
	plProfileManagerFull::Instance().ShowDetailGraph();
}

PF_CONSOLE_CMD(Stats, HideDetail, "", "Hides the detail stat graph")
{
	plProfileManagerFull::Instance().HideDetailGraph();
}

PF_CONSOLE_CMD(Stats, ResetDetailDefaults, "", "Resets the detail graph's defaults")
{
	plProfileManagerFull::Instance().ResetDefaultDetailVars();
}

PF_CONSOLE_CMD(Stats, AddDetailVar, "string stat", "Adds the specified var to the detail graph with the default range of 0->100")
{
	plProfileManagerFull::Instance().AddDetailVar(params[0], 0, 100);
}

PF_CONSOLE_CMD(Stats, AddDetailVarWithOffset, "string stat, int offset", "Adds the specified var to the detail graph with a offset and default range\n"
											  "of 0->(100-offset)")
{
	int offset = (int)params[1];
	plProfileManagerFull::Instance().AddDetailVar(params[0], -offset, 100-offset);
}

PF_CONSOLE_CMD(Stats, AddDetailVarWithRange, "string stat, int min, int max", "Adds the specified var to the detail graph")
{
	plProfileManagerFull::Instance().AddDetailVar(params[0], (int)params[1], (int)params[2]);
}

PF_CONSOLE_CMD(Stats, AddDetailVarWithOffsetAndRange, "string stat, int offset, int min, int max", "Adds the specified var to the detail graph with an\n"
													  "offset and a range of min->(max-offset)")
{
	int offset = (int)params[1];
	plProfileManagerFull::Instance().AddDetailVar(params[0], (int)params[2]-offset, (int)params[3]-offset);
}

PF_CONSOLE_CMD(Stats, RemoveDetailVar, "string stat", "Removes the specified var from the detail graph")
{
	plProfileManagerFull::Instance().RemoveDetailVar(params[0]);
}

#include "../plStatGather/plAutoProfile.h"

PF_CONSOLE_CMD(Stats, AutoProfile, "...", "Performs an automated profile in all the ages. Optional: Specify an age name to do just that age")
{
	const char* ageName = nil;
	if (numParams > 0)
		ageName = params[0];

	plAutoProfile::Instance()->StartProfile(ageName);
}

PF_CONSOLE_CMD(Stats, ProfileAllAgeLoads, "", "Turns on Registry.LogReadTimes and links to each age, then exits")
{
	((plResManager*)hsgResMgr::ResMgr())->LogReadTimes(true);
	plAutoProfile::Instance()->LinkToAllAges();
}

#endif // LIMIT_CONSOLE_COMMANDS


//////////////////////////////////////////////////////////////////////////////
//// Memory  Commands ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#ifndef LIMIT_CONSOLE_COMMANDS


PF_CONSOLE_GROUP( Memory )

#ifdef HS_FIND_MEM_LEAKS

PF_CONSOLE_CMD( Memory, DumpAllocReport,	// Group name, Function name
				"",			// Params
				"Dump heap allocations to file." )	// Help string
{
	MemDumpAllocReport();
}


PF_CONSOLE_CMD(Memory,
			   ValidateNow,
			   "",
			   "Validate all heap allocations")
{
    MemValidateNow();
}


PF_CONSOLE_CMD(Memory,
			   SetValidation,
			   "bool on",
			   "Validate all heap allocations each time memory is alloced or freed.")
{
    MemSetValidation((bool)params[0]);
}

PF_CONSOLE_CMD(Memory,
			   DumpUsage,
			   "",
			   "Dump heap usage to file")
{
    MemDumpUsageReport();
}


#endif

#endif // LIMIT_CONSOLE_COMMANDS

//////////////////////////////////////////////////////////////////////////////
//// Console Group Commands //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#ifndef LIMIT_CONSOLE_COMMANDS


PF_CONSOLE_GROUP( Console )

PF_CONSOLE_CMD( Console, Clear, "", "Clears the console" )
{
	pfConsole::Clear();
}

PF_CONSOLE_CMD( Console, EnableFX, "bool enable", "Enables flashy console effects" )
{
	pfConsole::EnableEffects( (hsBool)(bool)params[ 0 ] );
	if( pfConsole::AreEffectsEnabled() )
		PrintString( "Console effects enabled" );
	else
		PrintString( "Console effects disabled" );
}

PF_CONSOLE_CMD( Console, SetTextColor, "int r, int g, int b", 
				"Sets the color of normal console text" )
{
	UInt32		color = 0xff000000 | ( (int)params[ 0 ] << 16 ) | ( (int)params[ 1 ] << 8 ) | ( (int)params[ 2 ] );

	pfConsole::SetTextColor( color );
}

class DocGenIterator : public pfConsoleCmdIterator
{
	FILE *fFile;	

public:
	DocGenIterator(FILE *f) { fFile = f; }
	virtual void ProcessCmd(pfConsoleCmd* c, int depth) 
	{

		if(strncmp("SampleCmd",c->GetName(), 9) != 0)
		{
			fprintf(fFile, "<P><I>%s </I><BR>%s </P>\n",c->GetSignature(),
					c->GetHelp());
		}
	}
	virtual bool ProcessGroup(pfConsoleCmdGroup *g, int depth) 
	{
	//	if(g->GetFirstCommand() != nil)
		{
			fprintf(fFile, "<P><B><H%s>Command %sGroup %s </B></H2></P>\n",
				(depth > 0) ? "3" : "2",
				(depth > 0) ? "Sub" :"" ,
				g->GetName());
		}
		return true;
	}
};

class BriefDocGenIterator : public pfConsoleCmdIterator
{
	FILE *fFile;	
	char fGrpName[200];

public:
	BriefDocGenIterator(FILE *f) { fFile = f; strcpy(fGrpName,"");}
	virtual void ProcessCmd(pfConsoleCmd* c, int depth) 
	{

		if(strncmp("SampleCmd",c->GetName(), 9) != 0)
		{
				fprintf(fFile, "<I>%s.%s </I> - %s <BR>\n",fGrpName,c->GetSignature(),
						c->GetHelp());
		}
	}
	virtual bool ProcessGroup(pfConsoleCmdGroup *g, int depth) 
	{
	//	if(g->GetFirstCommand() != nil)
		{
			fprintf(fFile, "<BR>\n");
			if(depth <1)
				strcpy(fGrpName, g->GetName());
			else 
			{
				strcat(fGrpName,".");
				strcat(fGrpName,g->GetName());
			}

		}
		return true;
	}
};

PF_CONSOLE_CMD( Console, CreateDocumentation, "string fileName", 
				"Writes HTML documentation for the current console commands" )
{

	PrintString((char*)params[0]);


	pfConsoleCmdGroup	*group;
	FILE *f;


	f = fopen(params[0],"wt");
	if(f == nil)
	{
		PrintString("Couldn't Open File");
		return;
	}
	

	fprintf(f, "<CENTER> <H2> Console Commands for Plasma 2.0 Client </H2> <I>Built %s on %s.</I></CENTER><BR>", 
		pnBuildDates::fBuildTime, pnBuildDates::fBuildDate );

	DocGenIterator iter(f);
	group = pfConsoleCmdGroup::GetBaseGroup();
	group->IterateCommands(&iter);

	fclose(f);
	
}


PF_CONSOLE_CMD( Console, CreateBriefDocumentation, "string fileName", 
				"Writes brief HTML documentation for the current console commands" )
{

	PrintString((char*)params[0]);


	pfConsoleCmdGroup	*group;
	FILE *f;


	f = fopen(params[0],"wt");
	if(f == nil)
	{
		PrintString("Couldn't Open File");
		return;
	}

	fprintf(f, "<CENTER> <H3> Console Commands for Plasma 2.0 Client </H3> <I>Built %s on %s.</I></CENTER><BR>", 
		pnBuildDates::fBuildTime, pnBuildDates::fBuildDate );
	BriefDocGenIterator iter(f);
	group = pfConsoleCmdGroup::GetBaseGroup();
	group->IterateCommands(&iter);

	fclose(f);
	
}

PF_CONSOLE_CMD( Console, SetVar, "string name, string value", 
				"Sets the value of a given global console variable" )
{
	pfConsoleContext &ctx = pfConsoleContext::GetRootContext();


	hsBool oldF = ctx.GetAddWhenNotFound();
	ctx.SetAddWhenNotFound( true );
	ctx.SetVar( params[ 0 ], params[ 1 ] );
	ctx.SetAddWhenNotFound( oldF );
}

PF_CONSOLE_CMD( Console, PrintVar, "string name", "Prints the value of a given global console variable" )
{
	pfConsoleContext &ctx = pfConsoleContext::GetRootContext();

	Int32 idx = ctx.FindVar( params[ 0 ] );
	if( idx == -1 )
		PrintString( "Variable not found" );
	else
	{
		PrintStringF( PrintString, "The value of %s is %s", (const char *)params[ 0 ], (const char *)ctx.GetVarValue( idx ) );
	}
}

PF_CONSOLE_CMD( Console, PrintAllVars, "", "Prints the values of all global console variables" )
{
	pfConsoleContext &ctx = pfConsoleContext::GetRootContext();

	UInt32	i;

	PrintString( "Global console variables:" );
	for( i = 0; i < ctx.GetNumVars(); i++ )
	{
		PrintStringF( PrintString, "  %s: %s", (const char *)ctx.GetVarName( i ), (const char *)ctx.GetVarValue( i ) );
	}
}

PF_CONSOLE_CMD( Console, ClearAllVars, "", "Wipes the global console variable context" )
{
	pfConsoleContext &ctx = pfConsoleContext::GetRootContext();
	ctx.Clear();
	PrintString( "Global context wiped" );
}

PF_CONSOLE_CMD( Console, ExecuteFile, "string filename", "Runs the given file as if it were an .ini or .fni file" )
{
	plConsoleMsg *cMsg = TRACKED_NEW plConsoleMsg( plConsoleMsg::kExecuteFile, params[ 0 ] );
	cMsg->Send();
}

PF_CONSOLE_CMD( Console, ExecuteFileDelayed, "string filename, float timeInSecs", "Runs the given file as if it were an .ini or .fni file, but at some given point in the future" )
{
	plConsoleMsg *cMsg = TRACKED_NEW plConsoleMsg( plConsoleMsg::kExecuteFile, params[ 0 ] );
	cMsg->SetTimeStamp( hsTimer::GetSysSeconds() + (float)params[ 1 ] );
	cMsg->Send();
}

#endif // LIMIT_CONSOLE_COMMANDS


//////////////////////////////////////////////////////////////////////////////
//// Graphics Group Commands /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

PF_CONSOLE_GROUP( Graphics ) // Defines a main command group

#ifndef LIMIT_CONSOLE_COMMANDS

// NOTE ON THESE DEBUG FLAGS:
// Any unique prefix will work for a given flag (although the
// TAB won't fill out the rest of the command). So "noli" works
// as well as "noLightMaps". So try to make the first 4 or 5 letters of
// the string flag both unique AND meaningful. E.g. if the flag will
// disable XYZ, then "noXYZ..." is a good start for the command (as opposed
// to "disableXYZ..."). Note also, that no checking for uniqueness happens
// (because I don't feel like it), so "SetDebugFlag no" will grab the first
// of many things that start with no (currently noMultiTexture). Since verbs
// tend to get reused more than subjects, start commands with the noun instead
// of the verb. E.g. "showBufferData" and "showNormals" can be more easily distinguished
// as bufferDataShow and normalsShow.
PF_CONSOLE_CMD( Graphics,			// Group name
				SetDebugFlag,				// Function name
				"string flag, ...",		// Params
				"Sets or toggles a pipeline debug flag.\nValid flags are:\n\
\tbufferDataShow - Shows vertex/index buffer stats\n\
\tnoMultiTexture - Disables multitexturing\n\
\tnoLightMaps - Disables lightmaps\n\
\tnoRTLights - Disables runtime lighting\n\
\tnoAlphaBlending - Disables alpha blending\n\
\tnoDecals - Disables drawing of decals\n\
\tnoFaceSort - Disables alpha-blending face sorting\n\
\tnormalShow - Shows normals for all vertices\n\
\tnoShadows - Toggles shadow generation and display\n\
\tnoUpper - Toggles render of upper layers\n\
\tnoBump - Toggles bump mapping\n\
\tnoRender - Toggles all rendering\n\
\tnoLODPop - Toggles ignoring of LOD Pops\n\
\tnoPlates - Toggles rendering of plates\n\
\tmipmapColorize - Color-codes mipmap levels\n\
\tnoAnisotropy - Disables anisotropic filtering\n\
\tallBright - Overrides D3D lighting equations (forces everything emissive)\n\
\tnoProjLights - Turns off runtime projected lights\n\
\toneMaterial - Toggles using one material for the entire scene\n\
\treShaders - reload all shaders\n\
\treTex - reload all textures from sysmem\n\
\tonlyProjLights - Turns off runtime non-projected lights\n\
\tnoFog - Disable all fog" )	// Help string
{
	UInt32		flag;
	bool		on;
	char		string[ 128 ], name[ 64 ];
	int			i;

	struct 
	{ 
		char name[ 64 ]; UInt32 flag; 
	} flags[] = { { "reloadTextures", plPipeDbg::kFlagReload },
					{ "noPreShade", plPipeDbg::kFlagNoPreShade},
					{ "noMultitexture", plPipeDbg::kFlagNoMultitexture },
					{ "noLightMaps", plPipeDbg::kFlagNoLightmaps },
					{ "noRTLights", plPipeDbg::kFlagNoRuntimeLights },
					{ "noAlphaBlending", plPipeDbg::kFlagNoAlphaBlending },
					{ "noDecals", plPipeDbg::kFlagNoDecals },
					{ "noFaceSort", plPipeDbg::kFlagDontSortFaces },
					{ "noSpecular", plPipeDbg::kFlagDisableSpecular },
					{ "normalShow", plPipeDbg::kFlagShowNormals },
					{ "mipmapColorize", plPipeDbg::kFlagColorizeMipmaps },
					{ "noShadows", plPipeDbg::kFlagNoShadows },
					{ "noUpper", plPipeDbg::kFlagNoUpperLayers },
					{ "noRender", plPipeDbg::kFlagNoRender },
					{ "noLODPop", plPipeDbg::kFlagSkipVisDist },
					{ "noPlates", plPipeDbg::kFlagNoPlates },
					{ "noBump", plPipeDbg::kFlagNoBump },
					{ "noAnisotropy", plPipeDbg::kFlagNoAnisotropy },
					{ "allBright", plPipeDbg::kFlagAllBright },
					{ "noProjLights", plPipeDbg::kFlagNoApplyProjLights },
					{ "oneMaterial", plPipeDbg::kFlagSingleMat},
					{ "onlyProjLights", plPipeDbg::kFlagOnlyApplyProjLights },
					{ "noFog", plPipeDbg::kFlagNoFog }
	};
	int		numDebugFlags = sizeof( flags ) / sizeof( flags[ 0 ] );


	if( numParams > 2 )
	{
		PrintString( "Invalid parameters. Use 'SetDebugFlag flag [, true|flase]'." );
		return;
	}

	for( i = 0; i < numDebugFlags; i++ )
	{
		if( strnicmp( params[ 0 ], flags[ i ].name, strlen(params[0]) ) == 0 )
		{
			flag = flags[ i ].flag;
			strcpy( name, flags[ i ].name );
			break;
		}
	}
	if( i == numDebugFlags )
	{
		flag = atoi( params[ 0 ] );
#ifndef HS_DEBUGGING
		if( flag < 1 || flag > flags[ numDebugFlags ].flag )
		{
			PrintString( "Invalid Flag. Check help for valid flags." );
			return;
		}
#endif
		sprintf( name, "Flag %d", flag );
	}

	if( numParams == 1 )
		on = !pfConsole::GetPipeline()->IsDebugFlagSet( flag );
	else
		on = params[ 1 ];

	pfConsole::GetPipeline()->SetDebugFlag( flag, on );

	sprintf( string, "%s is now %s", name, on ? "enabled" : "disabled" );
	PrintString( string );
}



PF_CONSOLE_SUBGROUP( Graphics, VisSet )		// Creates a sub-group under a given group

PF_CONSOLE_CMD( Graphics_VisSet, Toggle, "", "Toggle using VisSets" )
{
	hsBool turnOn = !plPageTreeMgr::VisMgrEnabled();
	plPageTreeMgr::EnableVisMgr(turnOn);

	PrintStringF( PrintString, "Visibility Sets %s", turnOn ? "Enabled" : "Disabled" );
}

PF_CONSOLE_CMD( Graphics, BumpNormal, "", "Set bump mapping method to default for your hardware." )
{
	PF_SANITY_CHECK( pfConsole::GetPipeline(), "This command MUST be used in an .fni file (after pipeline initialization)" );

	if( pfConsole::GetPipeline()->GetMaxLayersAtOnce() > 3 )
	{
		pfConsole::GetPipeline()->SetDebugFlag(plPipeDbg::kFlagBumpUV, false);
		pfConsole::GetPipeline()->SetDebugFlag(plPipeDbg::kFlagBumpW, false);
	}
	else
	{
		pfConsole::GetPipeline()->SetDebugFlag(plPipeDbg::kFlagBumpUV, true);
		pfConsole::GetPipeline()->SetDebugFlag(plPipeDbg::kFlagBumpW, false);
	}
}

PF_CONSOLE_CMD( Graphics, BumpUV, "", "Set bump mapping method to GeForce2 compatible." )
{
	PF_SANITY_CHECK( pfConsole::GetPipeline(), "This command MUST be used in an .fni file (after pipeline initialization)" );

	pfConsole::GetPipeline()->SetDebugFlag(plPipeDbg::kFlagBumpUV, true);
	pfConsole::GetPipeline()->SetDebugFlag(plPipeDbg::kFlagBumpW, false);
}

PF_CONSOLE_CMD( Graphics, BumpW, "", "Set bump mapping method to cheapest available." )
{
	PF_SANITY_CHECK( pfConsole::GetPipeline(), "This command MUST be used in an .fni file (after pipeline initialization)" );

	pfConsole::GetPipeline()->SetDebugFlag(plPipeDbg::kFlagBumpUV, false);
	pfConsole::GetPipeline()->SetDebugFlag(plPipeDbg::kFlagBumpW, true);
}


PF_CONSOLE_CMD( Graphics, AllowWBuffering, "", "Enables the use of w-buffering\n(w-buffering is disabled by default)." )
{
	PF_SANITY_CHECK( pfConsole::GetPipeline() == nil, "This command MUST be used in an .ini file (before pipeline initialization)" );

	extern UInt32 fDbgSetupInitFlags;


	fDbgSetupInitFlags |= 0x00000001;
	PrintString( "W-buffering enabled." );
}

PF_CONSOLE_CMD( Graphics, ForceGeForce2Quality, "", "Forces higher-level hardware down to roughly the capabilities of a GeForce 2." )
{
	PF_SANITY_CHECK( pfConsole::GetPipeline() == nil, "This command MUST be used in an .ini file (before pipeline initialization)" );

	extern UInt32 fDbgSetupInitFlags;


	fDbgSetupInitFlags |= 0x00000004;
	PrintString( "Hardware caps forced down to GeForce 2 level." );
}

#endif // LIMIT_CONSOLE_COMMANDS



//// Graphics.Shadow SubGroup /////////////////////////////////////////////
PF_CONSOLE_SUBGROUP( Graphics, Shadow )		// Creates a sub-group under a given group

PF_CONSOLE_CMD( Graphics_Shadow,
			   Enable,
			   "bool enable",
			   "Enable shadows." )
{
	bool enable = (bool)params[0];

	if (enable)
	{
		plShadowCaster::EnableShadowCast();
		PrintString("Shadows Enabled");
	}
	else
	{
		plShadowCaster::DisableShadowCast();
		PrintString("Shadows Disabled");
	}
}

PF_CONSOLE_CMD( Graphics_Shadow, 
			   Disable,
			   "", 
			   "Disable shadows." )
{
	plShadowCaster::DisableShadowCast();
	PrintString("Shadows Disabled");
}

PF_CONSOLE_CMD( Graphics_Shadow, 
			   Toggle,
			   "", 
			   "Toggle shadows." )
{
	plShadowCaster::ToggleShadowCast();
	PrintString(plShadowCaster::ShadowCastDisabled() ? "Shadows Disabled" : "Shadows Enabled");
}

PF_CONSOLE_CMD( Graphics_Shadow, 
			   Show,
			   "", 
			   "Show shadows." )
{
	hsBool on = !pfConsole::GetPipeline()->IsDebugFlagSet(plPipeDbg::kFlagShowShadowBounds);
	pfConsole::GetPipeline()->SetDebugFlag( plPipeDbg::kFlagShowShadowBounds, on );

	char	str[ 256 ];
	sprintf( str, "Shadow bounds now %s", on ? "visible" : "invisible" );
	PrintString( str );
}

PF_CONSOLE_CMD( Graphics_Shadow, 
			   Apply,
			   "", 
			   "Toggles applying shadows (they are still computed)." )
{
	hsBool on = !pfConsole::GetPipeline()->IsDebugFlagSet(plPipeDbg::kFlagNoShadowApply);
	pfConsole::GetPipeline()->SetDebugFlag( plPipeDbg::kFlagNoShadowApply, on );

	char	str[ 256 ];
	sprintf( str, "Shadow apply now %s", on ? "disabled" : "enabled" );
	PrintString( str );
}

PF_CONSOLE_CMD( Graphics_Shadow, 
			   MaxSize,
			   "...", 
			   "Max shadowmap size." )
{
	int size;
	if( numParams > 0 )
	{
		size = atoi( params[ 0 ] );

		plShadowMaster::SetGlobalMaxSize(size);
	}
	else
	{
		size = plShadowMaster::GetGlobalMaxSize();
	}
	char str[256];
	sprintf(str, "Max shadowmap size %d", size);
	PrintString(str);
}

PF_CONSOLE_CMD( Graphics_Shadow, 
			   MaxDist,
			   "...", 
			   "Max shadowmap vis distance." )
{
	float dist;
	if( numParams > 0 )
	{
		dist = (float)atof( params[ 0 ] );

		plShadowMaster::SetGlobalMaxDist(dist);
	}
	else
	{
		dist = plShadowMaster::GetGlobalMaxDist();
	}
	char str[256];
	sprintf(str, "Max shadowmap vis dist %f", dist);
	PrintString(str);
}

PF_CONSOLE_CMD( Graphics_Shadow, 
			   VisibleDistance,
			   "...", 
			   "Shadow quality (0 to 1)." )
{
	float parm;
	if( numParams > 0 )
	{
		parm = (float)atof( params[ 0 ] );

		plShadowMaster::SetGlobalShadowQuality(parm);
	}
	else
	{
		parm = plShadowMaster::GetGlobalShadowQuality();
	}
	char str[256];
	sprintf(str, "Shadow quality %f", parm);
	PrintString(str);
}

PF_CONSOLE_CMD( Graphics_Shadow, 
			   Blur,
			   "...", 
			   "Max shadowmap blur size." )
{
	extern hsScalar blurScale;
	if( numParams > 0 )
	{
		blurScale = (hsScalar)atof( params[ 0 ] );
	}
	else
	{
	}
	char str[256];
	sprintf(str, "Max shadowmap Blur %f", blurScale);
	PrintString(str);
}

//// Graphics.DebugText SubGroup /////////////////////////////////////////////

#ifndef LIMIT_CONSOLE_COMMANDS


PF_CONSOLE_SUBGROUP( Graphics, DebugText )		// Creates a sub-group under a given group

PF_CONSOLE_CMD( Graphics_DebugText,			// Group name
				SetFont,					// Function name
				"string face, int size",	// Params
				"Sets the font face and size used for drawing debug text" )	// Help string
{
	plDebugText::Instance().SetFont( params[ 0 ], (UInt16)(int)params[ 1 ] );
}

PF_CONSOLE_CMD( Graphics_DebugText,			// Group name
				Enable,						// Function name
				"",							// Params
				"Enables debug text drawing" )	// Help string
{
	plDebugText::Instance().SetEnable( true );
}

PF_CONSOLE_CMD( Graphics_DebugText,			// Group name
				Disable,					// Function name
				"",							// Params
				"Disables debug text drawing" )	// Help string
{
	plDebugText::Instance().SetEnable( false );
}

#endif // LIMIT_CONSOLE_COMMANDS
	

PF_CONSOLE_SUBGROUP( Graphics, Renderer )		// Creates a sub-group under a given group

PF_CONSOLE_CMD( Graphics_Renderer, SetClearColor, "float r, float g, float b", "Sets the global clear color" )
{
	hsAssert( pfConsole::GetPipeline() != nil, "Cannot use this command before pipeline initialization" );

	hsColorRGBA		c;

	c.Set( params[ 0 ], params[ 1 ], params[ 2 ], 1.0f );
	plClient::GetInstance()->SetClearColor( c );
}


#ifndef LIMIT_CONSOLE_COMMANDS


PF_CONSOLE_CMD( Graphics_Renderer, mfTest, "int mfDbgTest", "Reserved for internal testing" )
{
	extern int mfCurrentTest;

	mfCurrentTest = (int) params[0];

	char	str[ 256 ];
	sprintf( str, "Current test %d.", mfCurrentTest );
	PrintString( str );
}

#endif // LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_CMD( Graphics_Renderer, Gamma, "float g, ...", "Set gamma value (g or (r,g,b))" )
{
	hsAssert( pfConsole::GetPipeline() != nil, "Cannot use this command before pipeline initialization" );

	hsScalar g = params[0];

	if( numParams == 1 )
	{
		pfConsole::GetPipeline()->SetGamma(g);

//		char	str[ 256 ];
//		sprintf(str, "Gamma set to %g.", g);
//		PrintString(str);
	}
	else
	{
		hsScalar eR = g;
		hsScalar eG = g;
		hsScalar eB = g;

		if( numParams > 2 )
			eB = params[2];
		eG = params[1];

		pfConsole::GetPipeline()->SetGamma(eR, eG, eB);

//		char	str[ 256 ];
//		sprintf(str, "Gamma set to (%g, %g, %g).", eR, eG, eB);
//		PrintString(str);
	}
}

PF_CONSOLE_CMD( Graphics_Renderer, Gamma2, "float g", "Set gamma value (alternative ramp)" )
{
	hsAssert( pfConsole::GetPipeline() != nil, "Cannot use this command before pipeline initialization" );

	hsTArray<UInt16> ramp;
	ramp.SetCount(256);

	hsScalar g = params[0];

	int i;
	for( i = 0; i < 256; i++ )
	{
		hsScalar t = hsScalar(i) / 255.f;
		hsScalar sinT = sin(t * hsScalarPI / 2.f);

		hsScalar remap = t + (sinT - t) * g;
		if( remap < 0 )
			remap = 0;
		else if( remap > 1.f )
			remap = 1.f;

		ramp[i] = UInt16(remap * hsScalar(UInt16(-1)) + 0.5f);
	}

	pfConsole::GetPipeline()->SetGamma(ramp.AcquireArray());

//	char	str[ 256 ];
//	sprintf(str, "Gamma set to <alt> %g.", g);
//	PrintString(str);
}

#ifndef LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_CMD( Graphics_Renderer, MaxCullNodes, "...", "Limit occluder processing" )
{
	hsAssert( pfConsole::GetPipeline() != nil, "Cannot use this command before pipeline initialization" );

	int maxCullNodes;
	if( numParams > 0 )
	{
		maxCullNodes = (int) params[0];
		pfConsole::GetPipeline()->SetMaxCullNodes(maxCullNodes);
	}
	else
	{
		maxCullNodes = pfConsole::GetPipeline()->GetMaxCullNodes();
	}

	char	str[ 256 ];
	sprintf( str, "Max Cull Nodes now %d.", maxCullNodes );
	PrintString( str );
}


#endif // LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_CMD( Graphics_Renderer, SetYon, "float yon, ...", "Sets the view yon" )
{
	hsAssert( pfConsole::GetPipeline() != nil, "Cannot use this command before pipeline initialization" );

	hsScalar		hither, yon;


	pfConsole::GetPipeline()->GetDepth( hither, yon );

	pfConsole::GetPipeline()->SetDepth( hither, (float)params[ 0 ] );
	pfConsole::GetPipeline()->RefreshMatrices();
	
	char	str[ 256 ];
	sprintf( str, "Yon set to %4.1f.", (float)params[ 0 ] );
	PrintString( str );
}

#ifndef LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_CMD( Graphics_Renderer, TweakZBiasScale, "float deltaScale", "Adjusts the device-dependent scale value for upper-layer z biasing" )
{
	hsAssert( pfConsole::GetPipeline() != nil, "Cannot use this command before pipeline initialization" );

	hsScalar		scale;

	scale = pfConsole::GetPipeline()->GetZBiasScale();
	scale += (float)params[ 0 ];
	pfConsole::GetPipeline()->SetZBiasScale( scale );
	
	char	str[ 256 ];
	sprintf( str, "Z bias scale now set to %4.2f.", (float)scale );
	PrintString( str );
}

PF_CONSOLE_CMD( Graphics_Renderer, SetZBiasScale, "float scale", "Sets the device-dependent scale value for upper-layer z biasing" )
{
	hsAssert( pfConsole::GetPipeline() != nil, "Cannot use this command before pipeline initialization" );

	pfConsole::GetPipeline()->SetZBiasScale( (float)params[ 0 ] );
	
	char	str[ 256 ];
	sprintf( str, "Z bias scale now set to %4.2f.", (float)params[ 0 ] );
	PrintString( str );
}



PF_CONSOLE_CMD( Graphics_Renderer, Overwire, "...", "Turn on (off) overlay wire rendering" )
{
	hsAssert( pfConsole::GetPipeline() != nil, "Cannot use this command before pipeline initialization" );

	hsBool on = false;
	UInt32 flag = plPipeDbg::kFlagOverlayWire;
	if( !numParams )
		on = !pfConsole::GetPipeline()->IsDebugFlagSet( flag );
	else
		on = (bool)params[ 0 ];

	pfConsole::GetPipeline()->SetDebugFlag( flag, on );

	char str[256];
	sprintf( str, "OverlayWire is now %s", on ? "enabled" : "disabled" );
	PrintString( str );
}

PF_CONSOLE_CMD( Graphics_Renderer, Overdraw, "bool on", "Turn on (off) overdraw rendering" )
{
	hsAssert( pfConsole::GetPipeline() != nil, "Cannot use this command before pipeline initialization" );

	static plLayerDepth ld;
	static bool ldOn = false;
	if( (bool)params[0] )
	{
		if( !ldOn )
		{
			pfConsole::GetPipeline()->AppendLayerInterface( &ld, true );
			pfConsole::GetPipeline()->SetDebugFlag(plPipeDbg::kFlagNoLightmaps, true);
			ldOn = true;
		}
	}
	else
	{
		if( ldOn )
		{
			pfConsole::GetPipeline()->RemoveLayerInterface( &ld, true );
			pfConsole::GetPipeline()->SetDebugFlag(plPipeDbg::kFlagNoLightmaps, false	);
			ldOn = false;
		}
	}
}

PF_CONSOLE_CMD( Graphics_Renderer, Wireframe, "...", "Toggle or set wireframe view mode." )
{
	hsAssert( pfConsole::GetPipeline() != nil, "Cannot use this command before pipeline initialization" );

	static bool			wireOn = false;
	static plLayerOr	wireLayer;


	if( numParams > 1 )
	{
		PrintString( "Invalid parameters. Use 'Wireframe [true|false]'." );
		return;
	}

	wireLayer.SetMiscFlags( hsGMatState::kMiscWireFrame );

	if( numParams == 0 )
		wireOn = !wireOn;
	else if( wireOn == (bool)params[ 0 ] )
		return;
	else
		wireOn = (bool)params[ 0 ];

	if( wireOn )
		pfConsole::GetPipeline()->AppendLayerInterface( &wireLayer );
	else
		pfConsole::GetPipeline()->RemoveLayerInterface( &wireLayer );

	char	str[ 256 ];
	sprintf( str, "Wireframe view mode is now %s.", wireOn ? "enabled" : "disabled" );
	PrintString( str );
}

PF_CONSOLE_CMD( Graphics_Renderer, TwoSided, "...", "Toggle or set force two-sided." )
{
	hsAssert( pfConsole::GetPipeline() != nil, "Cannot use this command before pipeline initialization" );

	static bool			twoSideOn = false;
	static plLayerOr	twoSideLayer;


	if( numParams > 1 )
	{
		PrintString( "Invalid parameters. Use 'TwoSided [true|false]'." );
		return;
	}

	twoSideLayer.SetMiscFlags( hsGMatState::kMiscTwoSided );

	if( numParams == 0 )
		twoSideOn = !twoSideOn;
	else if( twoSideOn == (bool)params[ 0 ] )
		return;
	else
		twoSideOn = (bool)params[ 0 ];

	if( twoSideOn )
		pfConsole::GetPipeline()->AppendLayerInterface( &twoSideLayer );
	else
		pfConsole::GetPipeline()->RemoveLayerInterface( &twoSideLayer );

	char	str[ 256 ];
	sprintf( str, "Two-sided mode is now %s.", twoSideOn ? "enabled" : "disabled" );
	PrintString( str );
}

PF_CONSOLE_CMD( Graphics_Renderer, ResetDevice,
			   "int width, int height, int colordepth, bool Windowed, int numAASamples, int MaxAnisotropicSamples",
			   "Reset Display Device")
{
	plClient::GetInstance()->ResetDisplayDevice((int)params[0], (int)params[1], (int)params[2], (bool)params[3], (int)params[4], (int)params[5]);
}
#endif // LIMIT_CONSOLE_COMMANDS


static bool MakeUniqueFileName(const char* prefix, const char* ext, char* fileName)
{
	for (UInt32 uniqueNumber = 1; uniqueNumber < 1000; uniqueNumber++)
	{
		sprintf(fileName, "%s%03d.%s", prefix, uniqueNumber, ext);

		if (!plDoesFileExist(fileName))
			return true;
	}

	return false;
}


#ifndef LIMIT_CONSOLE_COMMANDS


PF_CONSOLE_CMD( Graphics_Renderer, TakeScreenshot, "...", "Takes a shot of the current frame and saves it to the given file" )
{
	hsAssert( pfConsole::GetPipeline() != nil, "Cannot use this command before pipeline initialization" );

	plMipmap		myMipmap;
	char			fileName[ 512 ];


	if( numParams > 1 )
	{
		PrintString( "Too many parameters to TakeScreenshot" );
		return;
	}
	else if( numParams == 1 )
		strcpy( fileName, (char *)params[ 0 ] );
	else
	{
		// Think up a filename
		if (!MakeUniqueFileName("screen", "tga", fileName))
		{
			PrintString( "Out of filenames for TakeScreenshot" );
			return;
		}
	}

	if( !pfConsole::GetPipeline()->CaptureScreen( &myMipmap ) )
		PrintString( "Error capturing screenshot" );
	else
	{
		char	str[ 512 ];

		plTGAWriter::Instance().WriteMipmap( fileName, &myMipmap );
		sprintf( str, "Screenshot written to '%s'.", fileName );
		PrintString( str );
	}
}

#include "../pfSurface/plGrabCubeMap.h"

PF_CONSOLE_CMD( Graphics_Renderer, GrabCubeMap, 
			   "string sceneObject, string prefix", 
			   "Take cubemap from sceneObject's position and name it prefix_XX.jpg")
{
	char str[512];
	const char* objName = params[0];
	plKey key = FindSceneObjectByName(objName, nil, str);
	PrintString( str );
	if( !key )
		return;
	plSceneObject *obj = plSceneObject::ConvertNoRef(key->GetObjectPtr());
	if( !obj )
		return;

	hsColorRGBA clearColor = plClient::GetInstance()->GetClearColor();
	const char* pref = params[1];
	plGrabCubeMap grabCube;
	grabCube.GrabCube(pfConsole::GetPipeline(), obj, pref, clearColor);
}

PF_CONSOLE_CMD( Graphics_Renderer, GrabCubeCam, 
			   "string prefix", 
			   "Take cubemap from camera's position and name it prefix_XX.jpg")
{
	hsPoint3 pos = pfConsole::GetPipeline()->GetViewPositionWorld();

	hsColorRGBA clearColor = plClient::GetInstance()->GetClearColor();
	const char* pref = params[1];
	plGrabCubeMap grabCube;
	grabCube.GrabCube(pfConsole::GetPipeline(), pos, pref, clearColor);
}

#include "../plJPEG/plJPEG.h"

PF_CONSOLE_CMD( Graphics_Renderer, TakeJPEGScreenshot, "...", "Takes a shot of the current frame and saves it to the given file" )
{
	hsAssert( pfConsole::GetPipeline() != nil, "Cannot use this command before pipeline initialization" );

	plMipmap		myMipmap;
	char			fileName[ 512 ];


	if( numParams > 2 )
	{
		PrintString( "Too many parameters to TakeScreenshot" );
		return;
	}
	else if( numParams > 0 )
		strcpy( fileName, (char *)params[ 0 ] );
	else
	{
		// Think up a filename
		if (!MakeUniqueFileName("screen", "jpg", fileName))
		{
			PrintString( "Out of filenames for TakeScreenshot" );
			return;
		}
	}

	if( !pfConsole::GetPipeline()->CaptureScreen( &myMipmap ) )
		PrintString( "Error capturing screenshot" );
	else
	{
		char	str[ 512 ];
		UInt8	quality = 75;


		if( numParams == 2 )
			quality = (int)params[ 1 ];

		plJPEG::Instance().SetWriteQuality( quality );

		if( !plJPEG::Instance().WriteToFile( fileName, &myMipmap ) )
		{
			sprintf( str, "JPEG write failed (%s).", plJPEG::Instance().GetLastError() );
		}
		else
			sprintf( str, "Screenshot written to '%s', quality of %d%%.", fileName, quality );
		PrintString( str );
	}
}

#include "../plGImage/plAVIWriter.h"

PF_CONSOLE_CMD( Graphics_Renderer, AVIWrite, "...", "Saves each frame to an AVI file" )
{
	hsAssert( pfConsole::GetPipeline() != nil, "Cannot use this command before pipeline initialization" );

	char fileName[ 512 ];

	if( numParams > 2 )
	{
		PrintString( "Too many parameters to AVIWrite" );
		return;
	}
	else if( numParams > 0 )
		strcpy( fileName, (char *)params[ 0 ] );
	else
	{
		// Think up a filename
		if (!MakeUniqueFileName("movie", "avi", fileName))
		{
			PrintString( "Out of filenames for AVIWrite" );
			return;
		}
	}

	if (!plAVIWriter::Instance().Open(fileName, pfConsole::GetPipeline()))
		PrintString( "AVI file create failed");
}

PF_CONSOLE_CMD(Graphics_Renderer, AVIClose, "", "Stops the current AVI recording")
{
	plAVIWriter::Instance().Close();
	PrintString("Recording stopped");
}

/*
PF_CONSOLE_CMD( Graphics_Renderer, GenerateReflectMaps, "string baseObject, string baseFileName, int size", "Generates the six cubic faces of a reflection map centered on the given object" )
{
	hsAssert( pfConsole::GetPipeline() != nil, "Cannot use this command before pipeline initialization" );

	// First, create the renderTarget for the renderRequests
	plRenderTarget *target = TRACKED_NEW plRenderTarget( plRenderTarget::kIsProjected | plRenderTarget::kIsTexture, 
												params[ 2 ], params[ 2 ], 32, 24, 0 );

//	plMipmap *newMip = TRACKED_NEW plMipmap( size, size, plMipmap::kARGB32Config, 1 );


	c.Set( params[ 0 ], params[ 1 ], params[ 2 ], 1.0f );
	plClient::GetInstance()->SetClearColor( c );
}
*/

PF_CONSOLE_CMD( Graphics_Renderer, ToggleScene, "", "Toggles main scene drawing" )
{
	if( plClient::GetInstance() == nil )
	{
		PrintString( "Command invalid before client init" );
		return;
	}

	if( plClient::GetInstance()->HasFlag( plClient::kFlagDBGDisableRender ) )
	{
		plClient::GetInstance()->SetFlag( plClient::kFlagDBGDisableRender, false );
		PrintString( "Scene rendering enabled" );
	}
	else
	{
		plClient::GetInstance()->SetFlag( plClient::kFlagDBGDisableRender, true );
		PrintString( "Scene rendering disabled" );
	}
}

PF_CONSOLE_CMD( Graphics_Renderer, ToggleRenderRequests, "", "Toggles processing of pre- and post-render requests" )
{
	if( plClient::GetInstance() == nil )
	{
		PrintString( "Command invalid before client init" );
		return;
	}

	if( plClient::GetInstance()->HasFlag( plClient::kFlagDBGDisableRRequests ) )
	{
		plClient::GetInstance()->SetFlag( plClient::kFlagDBGDisableRRequests, false );
		PrintString( "Render requests enabled" );
	}
	else
	{
		plClient::GetInstance()->SetFlag( plClient::kFlagDBGDisableRRequests, true );
		PrintString( "Render requests disabled" );
	}
}

#endif // LIMIT_CONSOLE_COMMANDS

//// Graphics.Renderer.Fog Subgroup //////////////////////////////////////////

PF_CONSOLE_SUBGROUP( Graphics_Renderer, Fog )

PF_CONSOLE_CMD( Graphics_Renderer_Fog, SetDefColor, "float r, float g, float b", "Sets the default fog color" )
{
	hsAssert( pfConsole::GetPipeline() != nil, "Cannot use this command before pipeline initialization" );


	plFogEnvironment	env;
	hsColorRGBA			color;

	color.Set( params[ 0 ], params[ 1 ], params[ 2 ], 1.0f );
	env = pfConsole::GetPipeline()->GetDefaultFogEnviron();
	env.SetColor( color );
	pfConsole::GetPipeline()->SetDefaultFogEnviron( &env );
}

PF_CONSOLE_CMD( Graphics_Renderer_Fog, SetDefLinear, "float start, float end, float density", "Sets the default fog to linear" )
{
	hsAssert( pfConsole::GetPipeline() != nil, "Cannot use this command before pipeline initialization" );


	plFogEnvironment	env;

	env = pfConsole::GetPipeline()->GetDefaultFogEnviron();
	env.Set( params[ 0 ], params[ 1 ], params[ 2 ] ); 
	pfConsole::GetPipeline()->SetDefaultFogEnviron( &env );
}

PF_CONSOLE_CMD( Graphics_Renderer_Fog, SetDefExp, "float end, float density", "Sets the default fog to linear" )
{
	hsAssert( pfConsole::GetPipeline() != nil, "Cannot use this command before pipeline initialization" );


	plFogEnvironment	env;

	env = pfConsole::GetPipeline()->GetDefaultFogEnviron();
	env.SetExp( plFogEnvironment::kExpFog, params[ 0 ], params[ 1 ] ); 
	pfConsole::GetPipeline()->SetDefaultFogEnviron( &env );
}

PF_CONSOLE_CMD( Graphics_Renderer_Fog, SetDefExp2, "float end, float density", "Sets the default fog to exp^2" )
{
	hsAssert( pfConsole::GetPipeline() != nil, "Cannot use this command before pipeline initialization" );


	plFogEnvironment	env;

	env = pfConsole::GetPipeline()->GetDefaultFogEnviron();
	env.SetExp( plFogEnvironment::kExp2Fog, params[ 0 ], params[ 1 ] ); 
	pfConsole::GetPipeline()->SetDefaultFogEnviron( &env );
}

//// Graphics.Show Subgroups //////////////////////////////////////////

#ifndef LIMIT_CONSOLE_COMMANDS


PF_CONSOLE_SUBGROUP( Graphics, Show );

PF_CONSOLE_CMD( Graphics_Show, Bounds, "", "Toggle object bounds display")
{
	hsBool on = !pfConsole::GetPipeline()->IsDebugFlagSet( plPipeDbg::kFlagShowAllBounds );
	pfConsole::GetPipeline()->SetDebugFlag( plPipeDbg::kFlagShowAllBounds, on );

	char	str[ 256 ];
	sprintf( str, "Bounds now %s", on ? "visible" : "invisible" );
	PrintString( str );
}

PF_CONSOLE_CMD( Graphics_Show, Sound, "", "Toggle sound fields visible")
{
	static hsBool on = false;
	plProxyDrawMsg* msg = TRACKED_NEW plProxyDrawMsg(plProxyDrawMsg::kAudible | ((on = !on) ? plProxyDrawMsg::kCreate : plProxyDrawMsg::kDestroy));
	plgDispatch::MsgSend(msg);
	if( on )
		pfConsole::GetPipeline()->SetDrawableTypeMask(pfConsole::GetPipeline()->GetDrawableTypeMask() | plDrawableSpans::kAudibleProxy);
	else
		pfConsole::GetPipeline()->SetDrawableTypeMask(pfConsole::GetPipeline()->GetDrawableTypeMask() & ~plDrawableSpans::kAudibleProxy);

	char	str[ 256 ];
	sprintf( str, "Sounds now %s", on ? "visible" : "invisible" );
	PrintString( str );
}

PF_CONSOLE_CMD( Graphics_Show, SingleSound,
				"string sceneObject", "Toggles the proxy fields for a single sound object" )
{
	char	str[ 512 ];

	const char *ageName = plAgeLoader::GetInstance()->GetCurrAgeDesc().GetAgeName();

	plKey key = FindSceneObjectByName( params[ 0 ], ageName, str, true );
	plSceneObject *obj = ( key != nil ) ? plSceneObject::ConvertNoRef( key->GetObjectPtr() ) : nil;
	if( !obj )
	{
		sprintf( str, "Cannot find sceneObject %s", (char *)params[ 0 ] );
		PrintString( str );
		return;
	}

	const plAudioInterface	*ai = obj->GetAudioInterface();
	if( ai == nil )
	{
		sprintf( str, "sceneObject %s has no audio interface", (char *)params[ 0 ] );
		PrintString( str );
		return;
	}
	plKey	aiKey = ai->GetKey();

	plProxyDrawMsg* msg = TRACKED_NEW plProxyDrawMsg( plProxyDrawMsg::kAudible | plProxyDrawMsg::kToggle );
	msg->SetBCastFlag( plMessage::kBCastByExactType, false );
	msg->AddReceiver( aiKey );
	plgDispatch::MsgSend( msg );

	// Just in case we need to show them. Since we're toggling, we don't even know if it's being hidden and
	// thus we should turn this off. Since this is purely for debugging, the slight performance hit on this
	// is, imho, acceptable.
	pfConsole::GetPipeline()->SetDrawableTypeMask( pfConsole::GetPipeline()->GetDrawableTypeMask() | plDrawableSpans::kAudibleProxy );

	char str2[ 256 ];
	sprintf( str2, "Toggling proxies on sceneObject %s", (char *)params[ 0 ] );
	PrintString( str2 );
}

PF_CONSOLE_CMD( Graphics_Show, SoundOnly, "", "Toggle only sound fields visible")
{
	static hsBool on = false;
	plProxyDrawMsg* msg = TRACKED_NEW plProxyDrawMsg(plProxyDrawMsg::kAudible | ((on = !on) ? plProxyDrawMsg::kCreate : plProxyDrawMsg::kDestroy));
	plgDispatch::MsgSend(msg);
	static UInt32 oldMask = plDrawableSpans::kNormal;
	if( on )
	{
		oldMask = pfConsole::GetPipeline()->GetDrawableTypeMask();
		pfConsole::GetPipeline()->SetDrawableTypeMask(plDrawableSpans::kAudibleProxy);
	}
	else
	{
		pfConsole::GetPipeline()->SetDrawableTypeMask(oldMask);
	}
	char	str[ 256 ];
	sprintf( str, on ? "Now showing only sound proxies" : "Restoring previous render state" );
	PrintString( str );
}

PF_CONSOLE_CMD( Graphics_Show, OccSnap, "", "Take snapshot of current occlusion and render (or toggle)")
{
	UInt32 flag = plPipeDbg::kFlagOcclusionSnap;
	hsBool on = !pfConsole::GetPipeline()->IsDebugFlagSet(flag);

	pfConsole::GetPipeline()->SetDebugFlag( flag, on );
	if( on )
		pfConsole::GetPipeline()->SetDrawableTypeMask(pfConsole::GetPipeline()->GetDrawableTypeMask() | plDrawableSpans::kOccSnapProxy);
	else
		pfConsole::GetPipeline()->SetDrawableTypeMask(pfConsole::GetPipeline()->GetDrawableTypeMask() & ~plDrawableSpans::kOccSnapProxy);

	char	str[ 256 ];
	sprintf( str, "Active Occluders now %s", on ? "visible" : "invisible" );
	PrintString( str );
}

PF_CONSOLE_CMD( Graphics_Show, OccSnapOnly, "", "Take snapshot of current occlusion and render (or toggle)")
{
	UInt32 flag = plPipeDbg::kFlagOcclusionSnap;
	hsBool on = !pfConsole::GetPipeline()->IsDebugFlagSet(flag);

	static UInt32 oldMask = pfConsole::GetPipeline()->GetDrawableTypeMask();

	pfConsole::GetPipeline()->SetDebugFlag( flag, on );
	if( on )
		pfConsole::GetPipeline()->SetDrawableTypeMask(plDrawableSpans::kOccSnapProxy);
	else
		pfConsole::GetPipeline()->SetDrawableTypeMask(oldMask);

	char	str[ 256 ];
	sprintf( str, "Active Occluders now %s", on ? "visible" : "invisible" );
	PrintString( str );
}

PF_CONSOLE_CMD( Graphics_Show, Occluders, "", "Toggle occluder geometry visible")
{
	static hsBool on = false;
	plProxyDrawMsg* msg = TRACKED_NEW plProxyDrawMsg(plProxyDrawMsg::kOccluder | ((on = !on) ? plProxyDrawMsg::kCreate : plProxyDrawMsg::kDestroy));
	plgDispatch::MsgSend(msg);

	if( on )
		pfConsole::GetPipeline()->SetDrawableTypeMask(pfConsole::GetPipeline()->GetDrawableTypeMask() | plDrawableSpans::kOccluderProxy);
	else
		pfConsole::GetPipeline()->SetDrawableTypeMask(pfConsole::GetPipeline()->GetDrawableTypeMask() & ~plDrawableSpans::kOccluderProxy);

	char	str[ 256 ];
	sprintf( str, "Occluders now %s", on ? "visible" : "invisible" );
	PrintString( str );
}

PF_CONSOLE_CMD( Graphics_Show, OccludersOnly, "", "Toggle only occluder geometry visible")
{
	static hsBool on = false;
	plProxyDrawMsg* msg = TRACKED_NEW plProxyDrawMsg(plProxyDrawMsg::kOccluder | ((on = !on) ? plProxyDrawMsg::kCreate : plProxyDrawMsg::kDestroy));
	plgDispatch::MsgSend(msg);
	static UInt32 oldMask = plDrawableSpans::kNormal;
	if( on )
	{
		oldMask = pfConsole::GetPipeline()->GetDrawableTypeMask();
		pfConsole::GetPipeline()->SetDrawableTypeMask(plDrawableSpans::kOccluderProxy);
	}
	else
	{
		pfConsole::GetPipeline()->SetDrawableTypeMask(oldMask);
	}
	char	str[ 256 ];
	sprintf( str, on ? "Now showing only occluder proxies" : "Restoring previous render state" );
	PrintString( str );
}

PF_CONSOLE_CMD( Graphics_Show, Physicals, "", "Toggle Physical geometry visible")
{
	static hsBool on = false;
	plProxyDrawMsg* msg = TRACKED_NEW plProxyDrawMsg(plProxyDrawMsg::kPhysical | ((on = !on) ? plProxyDrawMsg::kCreate : plProxyDrawMsg::kDestroy));
	plgDispatch::MsgSend(msg);
	if( on )
		pfConsole::GetPipeline()->SetDrawableTypeMask(pfConsole::GetPipeline()->GetDrawableTypeMask() | plDrawableSpans::kPhysicalProxy);
	else
		pfConsole::GetPipeline()->SetDrawableTypeMask(pfConsole::GetPipeline()->GetDrawableTypeMask() & ~plDrawableSpans::kPhysicalProxy);

	char	str[ 256 ];
	sprintf( str, "Physicals now %s", on ? "visible" : "invisible" );
	PrintString( str );
}

PF_CONSOLE_CMD( Graphics_Show, PhysicalsOnly, "", "Toggle only Physical geometry visible")
{
	static hsBool on = false;
	plProxyDrawMsg* msg = TRACKED_NEW plProxyDrawMsg(plProxyDrawMsg::kPhysical | ((on = !on) ? plProxyDrawMsg::kCreate : plProxyDrawMsg::kDestroy));
	plgDispatch::MsgSend(msg);
	static UInt32 oldMask = plDrawableSpans::kNormal;
	if( on )
	{
		oldMask = pfConsole::GetPipeline()->GetDrawableTypeMask();
		pfConsole::GetPipeline()->SetDrawableTypeMask(plDrawableSpans::kPhysicalProxy);
	}
	else
	{
		pfConsole::GetPipeline()->SetDrawableTypeMask(oldMask);
	}
	char	str[ 256 ];
	sprintf( str, on ? "Now showing only physics proxies" : "Restoring previous render state" );
	PrintString( str );
}

PF_CONSOLE_CMD( Graphics_Show, Normal, "", "Toggle normal geometry visible")
{
	static hsBool on = true;
	if( on = !on )
		pfConsole::GetPipeline()->SetDrawableTypeMask(pfConsole::GetPipeline()->GetDrawableTypeMask() | plDrawableSpans::kNormal);
	else
		pfConsole::GetPipeline()->SetDrawableTypeMask(pfConsole::GetPipeline()->GetDrawableTypeMask() & ~plDrawableSpans::kNormal);

	char	str[ 256 ];
	sprintf( str, "Normal geometry now %s", on ? "visible" : "invisible" );
	PrintString( str );
}

PF_CONSOLE_CMD( Graphics_Show, NormalOnly, "", "Toggle only normal geometry visible")
{
	static hsBool on = false;
	static UInt32 oldMask = plDrawableSpans::kNormal;
	if( on = !on )
	{
		oldMask = pfConsole::GetPipeline()->GetDrawableTypeMask();
		pfConsole::GetPipeline()->SetDrawableTypeMask(plDrawableSpans::kNormal);
	}
	else
	{
		pfConsole::GetPipeline()->SetDrawableTypeMask(oldMask);
	}
	char	str[ 256 ];
	sprintf( str, on ? "Now showing only normal geometry" : "Restoring previous render state" );
	PrintString( str );
}

PF_CONSOLE_CMD( Graphics_Show, Lights, "", "Toggle visible proxies for lights")
{
	static hsBool on = false;
	plProxyDrawMsg* msg = TRACKED_NEW plProxyDrawMsg(plProxyDrawMsg::kLight | ((on = !on) ? plProxyDrawMsg::kCreate : plProxyDrawMsg::kDestroy));
	plgDispatch::MsgSend(msg);
	if( on )
		pfConsole::GetPipeline()->SetDrawableTypeMask(pfConsole::GetPipeline()->GetDrawableTypeMask() | plDrawableSpans::kLightProxy);
	else
		pfConsole::GetPipeline()->SetDrawableTypeMask(pfConsole::GetPipeline()->GetDrawableTypeMask() & ~plDrawableSpans::kLightProxy);

	char	str[ 256 ];
	sprintf( str, "Lights now %s", on ? "visible" : "invisible" );
	PrintString( str );
}

PF_CONSOLE_CMD( Graphics_Show, LightsOnly, "", "Toggle visible proxies for lights and everything else invisible")
{
	static hsBool on = false;
	plProxyDrawMsg* msg = TRACKED_NEW plProxyDrawMsg(plProxyDrawMsg::kLight | ((on = !on) ? plProxyDrawMsg::kCreate : plProxyDrawMsg::kDestroy));
	plgDispatch::MsgSend(msg);
	static UInt32 oldMask = plDrawableSpans::kNormal;
	if( on )
	{
		oldMask = pfConsole::GetPipeline()->GetDrawableTypeMask();
		pfConsole::GetPipeline()->SetDrawableTypeMask(plDrawableSpans::kLightProxy);
	}
	else
	{
		pfConsole::GetPipeline()->SetDrawableTypeMask(oldMask);
	}
	char	str[ 256 ];
	sprintf( str, on ? "Now showing only light proxies" : "Restoring previous render state" );
	PrintString( str );
}

PF_CONSOLE_CMD( Graphics_Show, Clicks, "", "Toggle visible proxies for clicks")
{
	static hsBool on = false;
	plProxyDrawMsg* msg = TRACKED_NEW plProxyDrawMsg(plProxyDrawMsg::kCamera | ((on = !on) ? plProxyDrawMsg::kCreate : plProxyDrawMsg::kDestroy));
	plgDispatch::MsgSend(msg);
	if( on )
		pfConsole::GetPipeline()->SetDrawableTypeMask(pfConsole::GetPipeline()->GetDrawableTypeMask() | plDrawableSpans::kCameraProxy);
	else
		pfConsole::GetPipeline()->SetDrawableTypeMask(pfConsole::GetPipeline()->GetDrawableTypeMask() & ~plDrawableSpans::kCameraProxy);

	char	str[ 256 ];
	sprintf( str, "Clicks now %s", on ? "visible" : "invisible" );
	PrintString( str );
}


PF_CONSOLE_CMD( Graphics_Show, ClickOnly, "", "Toggle visible proxies for click points")
{
	static hsBool on = false;
	plProxyDrawMsg* msg = TRACKED_NEW plProxyDrawMsg(plProxyDrawMsg::kCamera | ((on = !on) ? plProxyDrawMsg::kCreate : plProxyDrawMsg::kDestroy));
	plgDispatch::MsgSend(msg);
	static UInt32 oldMask = plDrawableSpans::kNormal;
	if( on )
	{
		oldMask = pfConsole::GetPipeline()->GetDrawableTypeMask();
		pfConsole::GetPipeline()->SetDrawableTypeMask(plDrawableSpans::kCameraProxy);
	}
	else
	{
		pfConsole::GetPipeline()->SetDrawableTypeMask(oldMask);
	}
	char	str[ 256 ];
	sprintf( str, on ? "Now showing only camera proxies" : "Restoring previous render state" );
	PrintString( str );
}

PF_CONSOLE_CMD( Graphics, ForceSecondMonitor, "bool v", "Run the game on the second monitor" )
{
	plPipeline::fInitialPipeParams.ForceSecondMonitor = (bool) params[0];
}

#endif // LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_CMD( Graphics, Width, "int w", "Initializes width" )
{
	plPipeline::fInitialPipeParams.Width = (int) params[0];
}

PF_CONSOLE_CMD( Graphics, Height, "int h", "Initializes height" )
{
	plPipeline::fInitialPipeParams.Height = (int) params[0];
}
PF_CONSOLE_CMD(Graphics, ColorDepth, "int colordepth", "Initializes color depth")
{
	plPipeline::fInitialPipeParams.ColorDepth = (int) params[0];
}

PF_CONSOLE_CMD( Graphics, Windowed, "bool w", "Initialize Windowed Mode")
{
	plPipeline::fInitialPipeParams.Windowed = (bool) params[0];
}

PF_CONSOLE_CMD( Graphics, TextureQuality, "int quality", "Initialize texture quality")
{
	int texqual = (int)params[0];
	if (texqual < 0)
		texqual = 0;
	else if (texqual > 2)
		texqual = 2;

	plPipeline::fInitialPipeParams.TextureQuality = texqual;
}

PF_CONSOLE_CMD( Graphics, AntiAliasAmount, "int aa", "Init AA Level")
{
	plPipeline::fInitialPipeParams.AntiAliasingAmount = (int) params[0];
}

PF_CONSOLE_CMD( Graphics, AnisotropicLevel, "int l", "Init Aniso Level" )
{
	plPipeline::fInitialPipeParams.AnisotropicLevel = (int) params[0];
}

PF_CONSOLE_CMD( Graphics, EnableVSync, "bool b", "Init VerticalSync" )
{
	plPipeline::fInitialPipeParams.VSync = (bool) params[0];
}

PF_CONSOLE_CMD( Graphics, EnablePlanarReflections, "bool", "Enable the draw and update of planar reflections" )
{
	bool enable = (bool)params[0];
	plDynamicCamMap::SetEnabled(enable);
}

//////////////////////////////////////////////////////////////////////////////
//// App Group Commands //////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
PF_CONSOLE_GROUP( App )		// Defines a main command group

#ifndef LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_CMD( App,
				Event,
				"string obj, string evType, float s, int reps",
				"string obj, string evType, float s, int reps" )
{

	char str[256];
	plKey key = FindSceneObjectByName(params[0], nil, str);
	plSceneObject* obj = plSceneObject::ConvertNoRef(key->GetObjectPtr());
	if( !obj )
	{
		strcat(str, " - Not Found!");
		PrintString(str);
		return;
	}

	plKey receiver = nil;
	PrintString(str);

	int i;
	for( i = 0; i < obj->GetNumModifiers(); i++ )
	{
		if( plSimpleModifier::ConvertNoRef(obj->GetModifier(i)) )
		{
			receiver = obj->GetModifier(i)->GetKey();
			break;
		}
	}
	if( !receiver )
	{
		strcat(str, " - Modifier Not Found!");
		PrintString(str);
		return;
	}

	plAnimCmdMsg* cmd = TRACKED_NEW plAnimCmdMsg;
	cmd->SetSender(plClient::GetInstance()->GetKey());
	cmd->SetCmd(plAnimCmdMsg::kAddCallbacks);
#if 1
	cmd->AddReceiver(receiver);
#else
	cmd->AddReceiver(key);
	cmd->SetBCastFlag(plMessage::kPropagateToModifiers, true);
#endif

	float secs = 0;
	int reps = 1;

	char* eventStr = params[1];
	CallbackEvent event;
	if( !_stricmp(eventStr, "Start") )
	{
		event = kStart;
	}
	else
	if( !_stricmp(eventStr, "Stop") )
	{
		event = kStop;
	}
	else
	if( !_stricmp(eventStr, "Time") )
	{
		event = kTime;
		secs = params[2];
	}
	if( numParams > 3 )
	{
		reps = params[3];
	}
	reps--;

	plEventCallbackMsg* callback = TRACKED_NEW plEventCallbackMsg(plClient::GetInstance()->GetKey(), event, 0, secs, reps);
	cmd->AddCallback(callback);
	hsRefCnt_SafeUnRef(callback);
	plgDispatch::MsgSend(cmd);
}

PF_CONSOLE_CMD( App,
				Sound,
				"string obj, string evType, float s, int reps",
				"string obj, string evType, float s, int reps" )
{

	char str[256];
	plKey key = FindSceneObjectByName(params[0], nil, str);
	plSceneObject* obj = plSceneObject::ConvertNoRef(key->GetObjectPtr());
	if( !obj )
	{
		strcat(str, " - Not Found!");
		PrintString(str);
		return;
	}

	PrintString(str);

	plSoundMsg* cmd = TRACKED_NEW plSoundMsg;
	cmd->SetSender(plClient::GetInstance()->GetKey());
	cmd->SetCmd(plSoundMsg::kAddCallbacks);
	cmd->AddReceiver(key);

	float secs = 0;
	int reps = -1;

	char* eventStr = params[1];
	CallbackEvent event;
	if( !_stricmp(eventStr, "Start") )
	{
		event = kStart;
	}
	else
	if( !_stricmp(eventStr, "Stop") )
	{
		event = kStop;
	}
	else
	if( !_stricmp(eventStr, "Time") )
	{
		event = kTime;
		secs = params[2];
	}
	if( numParams > 3 )
	{
		reps = params[3];
	}
	reps--;

	plEventCallbackMsg* callback = TRACKED_NEW plEventCallbackMsg(plClient::GetInstance()->GetKey(), event, 0, secs, reps);
	cmd->AddCallback(callback);
	hsRefCnt_SafeUnRef(callback);
	plgDispatch::MsgSend(cmd);
}

PF_CONSOLE_CMD( App,
				Overlay,
				"string name, ...", // paramList
				"Enable/Disable/Toggle display of named CamView object" )
{
	char str[256];
	char* name = params[0];
	plKey key = FindSceneObjectByName(name, nil, str);
	if( !key )
	{
		sprintf(str, "%s - Not Found!", name);
		PrintString(str);
		return;
	}
	plSceneObject* obj = plSceneObject::ConvertNoRef(key->GetObjectPtr());
	if( !obj )
	{
		sprintf(str, "%s - Not Found!", name);
		PrintString(str);
		return;
	}

	int i;
	for( i = 0; i < obj->GetNumModifiers(); i++ )
	{
		if( plPostEffectMod::ConvertNoRef(obj->GetModifier(i)) )
			break;
	}
	if( i >= obj->GetNumModifiers() )
	{
		sprintf(str, "%s - No CamView Modifier found!", name);
		PrintString(str);
		return;
	}
	strcpy(str, name);

	plAnimCmdMsg* cmd = TRACKED_NEW plAnimCmdMsg(nil, obj->GetModifier(i)->GetKey(), nil);

	if( numParams > 1 )
	{
		bool on = bool(params[1]);
		if( on )
		{
			cmd->SetCmd(plAnimCmdMsg::kContinue);
			strcat(str, " - Enabled");
		}
		else
		{
			cmd->SetCmd(plAnimCmdMsg::kStop);
			strcat(str, " - Disabled");
		}
	}
	else
	{
		cmd->SetCmd(plAnimCmdMsg::kToggleState);
		strcat(str, " - Toggled");
	}
	plgDispatch::MsgSend(cmd);
	PrintString(str);
}

PF_CONSOLE_CMD( App,		// groupName
			   TimeClamp,		// fxnName
			   "float maxSecsPerFrame", // paramList
			   "Clamp elapsed game time per frame (b4 scale)" )	// helpString
{
	float s = params[0];
	hsTimer::SetTimeClamp( s );

	char str[256];
	sprintf(str, "Time clamped to %f secs", s);
	PrintString( str );
}

PF_CONSOLE_CMD( App,		// groupName
			   TimeSmoothingClamp,		// fxnName
			   "float maxSecsPerFrame", // paramList
			   "Clamp max elapsed time that we'll smooth frame deltas" )	// helpString
{
	float s = params[0];
	hsTimer::SetTimeSmoothingClamp( s );

	char str[256];
	sprintf(str, "Time smoothing clamped to %f secs", s);
	PrintString( str );
}

PF_CONSOLE_CMD( App,		// groupName
			   FrameInc,		// fxnName
			   "float msPerFrame", // paramList
			   "Advance exactly msPerFrame milliseconds each frame" )	// helpString
{
	float s = params[0];
	s *= 1.e-3f;
	hsTimer::SetFrameTimeInc( s );

	char str[256];
	sprintf(str, "Frame advancing %f per frame (in frame time)", s * 1.e3f );
	PrintString( str );
}

PF_CONSOLE_CMD( App,		// groupName
			   RealTime,		// fxnName
			   "", // paramList
			   "Run in realtime" )	// helpString
{
	hsTimer::SetRealTime( true );

	char str[256];
	sprintf(str, "Now running real time");
	PrintString( str );
}

PF_CONSOLE_CMD( App,		// groupName
			   FrameTime,		// fxnName
			   "", // paramList
			   "Run in frametime" )	// helpString
{
	hsTimer::SetRealTime( false );

	char str[256];
	sprintf(str, "Now running frame time");
	PrintString( str );
}

PF_CONSOLE_CMD( App,		// groupName
			   ScaleTime,		// fxnName
			   "float s", // paramList
			   "Scale factor for time (e.g. ScaleTime 2 doubles speed of game)" )	// helpString
{
	float s = params[0];
	hsTimer::SetTimeScale( s );

	char str[256];
	sprintf(str, "Time scaled to %4.4f percent", s * 100.f );
	PrintString( str );
}

#include "../plInputCore/plSceneInputInterface.h"

PF_CONSOLE_CMD( App,		// groupName
			   ShowLOS,		// fxnName
			   "", // paramList
			   "Show object LOS hits" )	// helpString
{
	char str[256];
	if (plSceneInputInterface::fShowLOS)
	{
		plSceneInputInterface::fShowLOS = false;
		sprintf(str, "Stop displaying LOS hits");
	}
	else
	{
		plSceneInputInterface::fShowLOS = true;
		sprintf(str, "Start showing LOS hits");
	}
	PrintString( str );
}

#endif // LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_CMD( App,			// groupName
			   Quit,			// fxnName
			   "",				// paramList
			   "Quit the client app" ) // helpString
{
	if( plClient::GetInstance() )
		PostMessage(plClient::GetInstance()->GetWindowHandle(), 
			WM_SYSCOMMAND,
			SC_CLOSE,
			0);
}

#ifndef LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_CMD(App,
			   AuxInitDir,
			   "string pathName",
			   "Set an auxiliary init directory to read")
{
	if( plClient::GetInstance() )	
		plClient::GetInstance()->SetAuxInitDir(params[0]);
}


PF_CONSOLE_CMD( App,		// groupName
			   GetBuildDate,		// fxnName
			   "", // paramList
			   "Prints the date and time this build was created" )	// helpString
{
	char str[256];
	sprintf(str, "This Plasma 2.0 client built at %s on %s.", pnBuildDates::fBuildTime, pnBuildDates::fBuildDate );
	PrintString( str );
}

PF_CONSOLE_CMD( App,		// groupName
			   GetBranchDate,		// fxnName
			   "", // paramList
			   "Prints the date of the branch this code was produced from, or \"Pre-release\" if it is from the main code" )	// helpString
{
	char str[256];
	sprintf(str, "The branch date for this Plasma 2.0 client is: %s.", pnBuildDates::fBranchDate );
	PrintString( str );
}

PF_CONSOLE_CMD(App,
			   LowPriority,
			   "",
			   "Set low priority for this process")
{
	SetPriorityClass( GetCurrentProcess(), IDLE_PRIORITY_CLASS );
	PrintString( "Set process priority to lowest setting" );
}


PF_CONSOLE_CMD(App,
			   VerifyUnloaded,
			   "string age",
			   "Verify the given age is really unloaded into logfile logs/<age>.log")
{
	hsAssert(0, "Fixme");
	char* age = params[0];
	char str[256];
	sprintf(str, "%s.log", age);
//	hsgResMgr::ResMgr()->VerifyAgeUnloaded(str, age);

	sprintf(str, "Verification of age %s complete", age);
	PrintString(str);
}

#endif // LIMIT_CONSOLE_COMMANDS

#ifdef LIMIT_CONSOLE_COMMANDS // for now, disable languages in external clients
PF_CONSOLE_CMD(App,
			   SetLanguage,
			   "string language",
			   "Set the language (English, French, German, Spanish, Italian, or Japanese)")
{
	if (pfConsole::GetPipeline())
	{
		PrintString("This command must be used in an .ini file");
		return;
	}

	if (stricmp(params[0], "english") == 0)
		plLocalization::SetLanguage(plLocalization::kEnglish);
	else if (stricmp(params[0], "french") == 0)
		plLocalization::SetLanguage(plLocalization::kFrench);
	else if (stricmp(params[0], "german") == 0)
		plLocalization::SetLanguage(plLocalization::kGerman);
	else if (stricmp(params[0], "spanish") == 0)
		plLocalization::SetLanguage(plLocalization::kSpanish);
	else if (stricmp(params[0], "italian") == 0)
		plLocalization::SetLanguage(plLocalization::kItalian);
	else if (stricmp(params[0], "japanese") == 0)
		plLocalization::SetLanguage(plLocalization::kJapanese);

}
#endif // LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_CMD(App,
			   DemoMode,
			   "",
			   "Set the app to demo mode")
{
	if (pfConsole::GetPipeline())
	{
		PrintString("This command must be used in an .ini file");
		return;
	}

	plNetClientApp::GetInstance()->SetFlagsBit(plNetClientApp::kDemoMode);
}

PF_CONSOLE_CMD(App,
			   BounceLogs,
			   "",
			   "Clear all log files.")
{
	plStatusLogMgr::GetInstance().BounceLogs();
}

//////////////////////////////////////////////////////////////////////////////
//// Dispatch Group Commands /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#ifndef LIMIT_CONSOLE_COMMANDS

#include "pfDispatchLog.h"

PF_CONSOLE_GROUP( Dispatch )		// Defines a main command group
PF_CONSOLE_SUBGROUP( Dispatch, Log )		// Creates a sub-group under a given group

PF_CONSOLE_CMD( Dispatch_Log,		// groupName
			   LongReceives,		// fxnName
			   "", // paramList
			   "Log long msg receives (over 50 ms)" )	// helpString
{
	plDispatchLog::InitInstance();
	plDispatchLog::GetInstance()->SetFlags(plDispatchLog::GetInstance()->GetFlags() | plDispatchLog::kLogLongReceives);
}

PF_CONSOLE_CMD( Dispatch_Log,		// groupName
			   AddFilterType,		// fxnName
			   "string className", // paramList
			   "Adds a type filter to the Dispatch Logger" )	// helpString
{
	plDispatchLog::InitInstance();
	plDispatchLog::GetInstance()->AddFilterType(plFactory::FindClassIndex(params[0]));
}

PF_CONSOLE_CMD( Dispatch_Log,		// groupName
			   AddFilterExactType,		// fxnName
			   "string className", // paramList
			   "Adds an exact type filter to the Dispatch Logger" )	// helpString
{
	plDispatchLog::InitInstance();
	plDispatchLog::GetInstance()->AddFilterExactType(plFactory::FindClassIndex(params[0]));
}

PF_CONSOLE_CMD( Dispatch_Log,		// groupName
			   RemoveFilterType,		// fxnName
			   "string className", // paramList
			   "Removes a type filter to the Dispatch Logger" )	// helpString
{
	plDispatchLog::InitInstance();
	plDispatchLog::GetInstance()->RemoveFilterType(plFactory::FindClassIndex(params[0]));
}

PF_CONSOLE_CMD( Dispatch_Log,		// groupName
			   RemoveFilterExactType,		// fxnName
			   "string className", // paramList
			   "Removes an exact type filter to the Dispatch Logger" )	// helpString
{
	plDispatchLog::InitInstance();
	plDispatchLog::GetInstance()->RemoveFilterExactType(plFactory::FindClassIndex(params[0]));
}

PF_CONSOLE_CMD( Dispatch_Log,		// groupName
			   Include,		// fxnName
			   "", // paramList
			   "Sets Dispatch Log filters to be treated as an include list" )	// helpString
{
	plDispatchLog::InitInstance();
	plDispatchLog::GetInstance()->SetFlags(plDispatchLog::GetInstance()->GetFlags() | plDispatchLog::kInclude);
}

PF_CONSOLE_CMD( Dispatch_Log,		// groupName
			   Exclude,		// fxnName
			   "", // paramList
			   "Sets Dispatch Log filters to be treated as an exclude list" )	// helpString
{
	plDispatchLog::InitInstance();
	plDispatchLog::GetInstance()->SetFlags(plDispatchLog::GetInstance()->GetFlags() & ~plDispatchLog::kInclude);
}

#endif // LIMIT_CONSOLE_COMMANDS

//////////////////////////////////////////////////////////////////////////////
//// ResManager/Registry Commands ////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#ifndef LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_GROUP( Registry )		// Defines a main command group

PF_CONSOLE_CMD( Registry, ToggleDebugStats, "", "Toggles the debug statistics screen for the registry" )
{
	plResManagerHelper *helper = plResManagerHelper::GetInstance();
	if( helper == nil )
	{
		PrintString( "ERROR: ResManager helper object not initialized." );
		return;
	}

	static bool on = false;
	if( on )
	{
		helper->EnableDebugScreen( false );
		PrintString( "ResManager debug stats disabled" );
	}
	else
	{
		helper->EnableDebugScreen( true );
		plStatusLogMgr::GetInstance().SetCurrStatusLog( "ResManager Status" );
		PrintString( "ResManager debug stats enabled" );
	}
}

PF_CONSOLE_CMD( Registry, SetLoggingLevel, "int level", "Sets the logging level for the registry. 0 is no logging, 3 is max detail." )
{
	int newLevel = params[ 0 ];

	if( newLevel < 0 || newLevel > 4 )
	{
		PrintString( "ERROR: Invalid level specified. Valid levels are 0-3." );
		return;
	}

	plResMgrSettings::Get().SetLoggingLevel( (UInt8)newLevel );
	{
		char msg[ 128 ];
		sprintf( msg, "Registry logging set to %s", ( newLevel == 0 ) ? "none" : ( newLevel == 1 ) ? "basic" : 
														( newLevel == 2 ) ? "detailed" : ( newLevel == 3 ) ? "object-level" 
															: "object-read-level" );
		PrintString( msg );
	}
}

class plActiveRefPeekerKey : public plKeyImp
{
	public:
		UInt16      PeekNumNotifies() { return GetNumNotifyCreated(); }
		plRefMsg*   PeekNotifyCreated(int i) { return GetNotifyCreated(i); }
		hsBool      PeekIsActiveRef(int i) const { return IsActiveRef(i); }
};

// Not static so others can call it - making it even handier
void	MyHandyPrintFunction( const plKey &obj, void (*PrintString)( const char * ) )
{
	plActiveRefPeekerKey *peeker = (plActiveRefPeekerKey *)(plKeyImp *)obj;

	if( peeker->GetUoid().IsClone() )
		PrintStringF( PrintString, "%d refs on %s, clone %d:%d: loaded=%d", 
		peeker->PeekNumNotifies(), obj->GetUoid().GetObjectName(), 
		peeker->GetUoid().GetCloneID(), peeker->GetUoid().GetClonePlayerID(),
		obj->ObjectIsLoaded() ? 1 : 0);
	else
		PrintStringF( PrintString, "%d refs on %s: loaded=%d", 
		peeker->PeekNumNotifies(), obj->GetUoid().GetObjectName(), obj->ObjectIsLoaded() ? 1 : 0 );

	if( peeker->PeekNumNotifies() == 0 )
		return;

	UInt32 a, i, j, limit = 30, count = 0;
	for( a = 0; a < 2; a++ )
	{
		PrintString( ( a == 0 ) ? "  Active:" : "  Passive:" );

		for( i = 0; i < peeker->PeekNumNotifies(); i++ )
		{
			if( ( a == 0 && peeker->PeekIsActiveRef( i ) ) || ( a == 1 && !peeker->PeekIsActiveRef( i ) ) )
			{
				plRefMsg *msg = peeker->PeekNotifyCreated( i );
				if( msg != nil )
				{
					for( j = 0; j < msg->GetNumReceivers(); j++ )
					{
						if( limit == 0 )
							count++;
						else
						{
							limit--;

							const plKey rcvr = msg->GetReceiver( j );
							PrintStringF( PrintString, "    %s:%s", plFactory::GetNameOfClass( rcvr->GetUoid().GetClassType() ), rcvr->GetUoid().GetObjectName() );
						}
					}
				}
			}
		}
	}

	if( count > 0 )
		PrintStringF( PrintString, "...and %d others", count );
}

PF_CONSOLE_CMD( Registry, ListRefs, "string keyType, string keyName", "For the given key (referenced by type and name), lists all of "
			   "the objects who currently have active refs on it." )
{
	char result[ 256 ];
	plKey obj = FindObjectByNameAndType( params[ 1 ], params[ 0 ], nil, result);
	if( obj == nil )
	{
		PrintString( result );
		return;
	}

	MyHandyPrintFunction( obj, PrintString );
	
	plActiveRefPeekerKey *peeker = (plActiveRefPeekerKey *)(plKeyImp *)obj;
	if( peeker->GetNumClones() > 0 )
	{
		UInt32 i;
		for( i = 0; i < peeker->GetNumClones(); i++ )
		{
			MyHandyPrintFunction( peeker->GetCloneByIdx( i ), PrintString );
		}
	}
}

PF_CONSOLE_CMD(Registry, LogReadTimes, "", "Dumps the time for each object read to a file")
{
	((plResManager*)hsgResMgr::ResMgr())->LogReadTimes(true);
}

#endif // LIMIT_CONSOLE_COMMANDS


//////////////////////////////////////////////////////////////////////////////
//// Camera Group Commands ///////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
PF_CONSOLE_GROUP( Camera )		// Defines a main command group

#ifndef LIMIT_CONSOLE_COMMANDS


PF_CONSOLE_CMD( Camera, AvatarVisible1stPerson, "bool b", "turn avatar visibility in 1st person on or off")
{
	bool b = params[0];
	plCameraBrain1_FirstPerson::fDontFade = b;
}

PF_CONSOLE_CMD( Camera, FallTimerDelay, "float b", "fall timer delay")
{
	hsScalar f = params[0];
	plVirtualCam1::fFallTimerDelay = f;
}


PF_CONSOLE_CMD( Camera, Force3rdPersonOneshots, "bool b", "force camera to 3rd person for oneshots on or off")
{
	bool b = params[0];
	plAvOneShotTask::fForce3rdPerson = b;
}

PF_CONSOLE_CMD( Camera, Force3rdPersonMultistage, "bool b", "force camera to 3rd person for multistage on or off")
{
	bool b = params[0];
	plAvBrainGeneric::fForce3rdPerson = b;
}


PF_CONSOLE_CMD( Camera,		// groupName
			   Next,		// fxnName
			   "", // paramList
			   "Set the virtual camera to go to the next camera in the scene" )	// helpString
{
	plUoid pU1( kVirtualCamera1_KEY );
	plKey fLOS1 = hsgResMgr::ResMgr()->FindKey( pU1 );
	if (fLOS1)
	{
		plVirtualCam1::ConvertNoRef(fLOS1->GetObjectPtr())->Next();
		return;
	}
	
}

PF_CONSOLE_CMD( Camera,		// groupName
			   IgnoreRegions,		// fxnName
			   "bool b", // paramList
			   "Switch on / off camera regions" )	// helpString
{
	bool b = params[0];
	
	plUoid pU1( kVirtualCamera1_KEY );
	plKey fLOS1 = hsgResMgr::ResMgr()->FindKey( pU1 );
	if (fLOS1)
	{
		plVirtualCam1::ConvertNoRef(fLOS1->GetObjectPtr())->CameraRegions(b);
		return;
	}
	
}

PF_CONSOLE_CMD( Camera,		// groupName
			   LogFOV,		// fxnName
			   "bool b", // paramList
			   "Switch on / off verbose camera FOV change logging" )	// helpString
{
	bool b = params[0];
	
	plUoid pU1( kVirtualCamera1_KEY );
	plKey fLOS1 = hsgResMgr::ResMgr()->FindKey( pU1 );
	if (fLOS1)
	{
		plVirtualCam1::ConvertNoRef(fLOS1->GetObjectPtr())->LogFOV(b);
		return;
	}
	
}

PF_CONSOLE_CMD( Camera,		// groupName
			   Prev,		// fxnName
			   "", // paramList
			   "Set the virtual camera to go to the prev camera in the scene" )	// helpString
{
	plUoid pU1( kVirtualCamera1_KEY );
	plKey fLOS1 = hsgResMgr::ResMgr()->FindKey( pU1 );
	if (fLOS1)
	{
		plVirtualCam1::ConvertNoRef(fLOS1->GetObjectPtr())->Prev();
		return;
	}

}

PF_CONSOLE_CMD( Camera,		// groupName
			   SetFOV,		// fxnName
			   "float x, float y", // paramList
			   "Set the field of view for all cameras" )	// helpString
{
	float x = params[0];
	float y = params[1];
	plUoid pU1( kVirtualCamera1_KEY );
	plKey fLOS1 = hsgResMgr::ResMgr()->FindKey( pU1 );
	if (fLOS1)
	{
		plVirtualCam1::SetFOV(x,y);
		return;
	}
}

PF_CONSOLE_CMD( Camera,		// groupName
			   Drive,		// fxnName
			   "", // paramList
			   "Toggle drive mode" )	// helpString
{
	plVirtualCam1::Instance()->Drive();
}



PF_CONSOLE_CMD( Camera,		// groupName
			   IncreaseDriveTurnRate,		// fxnName
			   "", // paramList
			   "increase drive turn rate" )	// helpString
{
	plCameraBrain1_Drive::fTurnRate += 20.0f;
	
}

PF_CONSOLE_CMD( Camera,		// groupName
			   DecreaseDriveTurnRate,		// fxnName
			   "", // paramList
			   "decrease drive turn rate" )	// helpString
{
	plCameraBrain1_Drive::fTurnRate -= 20.0f;
	if (plCameraBrain1_Drive::fTurnRate < 0.0)
		plCameraBrain1_Drive::fTurnRate = 20.0f;
	
}


PF_CONSOLE_CMD( Camera, SwitchTo, "string cameraName", "Switch to the named camera")
{
	char str[256];
	char foo[256];
	sprintf(foo, "%s_", (const char*)params[0]);
	plKey key = FindObjectByNameAndType(foo, "plCameraModifier1", nil, str, true);
	PrintString(str);

	if (key)
	{
		plCameraMsg* pMsg = TRACKED_NEW plCameraMsg;
		pMsg->SetCmd(plCameraMsg::kResponderTrigger);
		pMsg->SetCmd(plCameraMsg::kRegionPushCamera);
		pMsg->SetNewCam(key);
		pMsg->SetBCastFlag(plMessage::kBCastByExactType);
		plgDispatch::MsgSend(pMsg);
	}
}

#endif // LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_CMD( Camera,		// groupName
			   SetFallSpeeds,		// fxnName
			   "float accel, float vel, float decel", // paramList
			   "Set camera fall speeds" )	// helpString
{
	float a = params[0];
	float v = params[1];
	float d = params[2];
	plCameraBrain1::fFallAccel = a;
	plCameraBrain1::fFallVelocity = v;
	plCameraBrain1::fFallDecel = d;
}

PF_CONSOLE_CMD( Camera,		// groupName
			   SetFallPOASpeeds,		// fxnName
			   "float accel, float vel, float decel", // paramList
			   "Set camera fall speeds" )	// helpString
{
	float a = params[0];
	float v = params[1];
	float d = params[2];
	plCameraBrain1::fFallPOAAccel = a;
	plCameraBrain1::fFallPOAVelocity = v;
	plCameraBrain1::fFallPOADecel = d;
}


PF_CONSOLE_CMD( Camera,		// groupName
			   SetGlobalAccel,		// fxnName
			   "float x", // paramList
			   "Set global camera acceleration - must set Camera.UseSpeedOverrides to TRUE to see effect" )	// helpString
{
	float f = params[0];
	plVirtualCam1::Instance()->fAccel = f;
}

PF_CONSOLE_CMD( Camera,		// groupName
			   SetGlobalDecel,		// fxnName
			   "float x", // paramList
			   "Set global camera deceleration - must set Camera.UseSpeedOverrides to TRUE to see effect" )	// helpString
{
	float f = params[0];
	plVirtualCam1::Instance()->fDecel = f;
}

PF_CONSOLE_CMD( Camera,		// groupName
			   SetGlobalVelocity,		// fxnName
			   "float x", // paramList
			   "Set global camera velocity - must set Camera.UseSpeedOverrides to TRUE to see effect" )	// helpString
{
	float f = params[0];
	plVirtualCam1::Instance()->fVel = f;
}

PF_CONSOLE_CMD( Camera,		// groupName
			   UseSpeedOverrides,		// fxnName
			   "bool b", // paramList
			   "Use console overrides for accel / decel" )	// helpString
{
	bool b = params[0];
	plVirtualCam1::Instance()->fUseAccelOverride = b;
}

PF_CONSOLE_CMD( Camera, VerticalPanAlways, "bool b", "turn vertical panning on always when walking")
{
	bool b = params[0];
	plVirtualCam1::WalkPan3rdPerson = b;
}

PF_CONSOLE_CMD( Camera, FirstPersonAlways, "bool b", "always in first person")
{
	bool b = params[0];
	plVirtualCam1::StayInFirstPersonForever = b;
}


#ifndef LIMIT_CONSOLE_COMMANDS


PF_CONSOLE_CMD( Camera,		// groupName
			   Freeze,		// fxnName
			   "bool b", // paramList
			   "freeze the camera system" )	// helpString
{
	bool b = params[0];
	plVirtualCam1::Instance()->freeze = b;
}

PF_CONSOLE_CMD( Camera,		// groupName
			   AlwaysCut,		// fxnName
			   "bool b", // paramList
			   "Forces camera transitions to always cut" )	// helpString
{
	bool b = params[0];
	plVirtualCam1::Instance()->alwaysCutForColin = b;
}

#endif // LIMIT_CONSOLE_COMMANDS

////////////////////////////////////////////////////////////////////////
//// Logic Mod Group Commands ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////

#ifndef LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_GROUP( Logic )

static plLogicModBase *FindLogicMod(const char *name)
{
	char str[256];
	plKey key = FindObjectByNameAndType(name, "plLogicModifier", nil, str, true);
	pfConsole::AddLine(str);

	if (key)
		return plLogicModBase::ConvertNoRef(key->GetObjectPtr());

	return nil;
}

PF_CONSOLE_CMD( Logic, TriggerDetectorNum, "int detectorNum", "Triggers the detector with this number (from ListDetectors)")
{
	std::vector<std::string> activatorNames;
	plKeyFinder::Instance().GetActivatorNames(activatorNames);

	int activatorNum = params[0];
	if (activatorNum < 1 || activatorNum > activatorNames.size())
	{
		PrintString("Detector number out of range");
		return;
	}

	plLogicModBase *mod = FindLogicMod(activatorNames[activatorNum-1].c_str());
	if (mod)
		mod->ConsoleTrigger(plNetClientMgr::GetInstance()->GetLocalPlayerKey());
}

PF_CONSOLE_CMD( Logic, TriggerDetector, "string detectorComp", "Triggers the named detector component")
{
	plLogicModBase *mod = FindLogicMod((const char*)params[0]);
	if (mod)
		mod->ConsoleTrigger(plNetClientMgr::GetInstance()->GetLocalPlayerKey());
}

PF_CONSOLE_CMD(Logic, EnableDetector, "string detectorComp, bool enable", "Enables/disables the named detector component")
{
	plLogicModBase *mod = FindLogicMod((const char*)params[0]);
	if (mod)
	{
		plEnableMsg* enableMsg = TRACKED_NEW plEnableMsg;
		enableMsg->SetCmd(params[1] ? plEnableMsg::kEnable : plEnableMsg::kDisable);
		enableMsg->SetCmd(plEnableMsg::kAll);
		enableMsg->Send(mod->GetKey());
	}
}

static void ResponderSendTrigger(plKey responderKey, int responderState, bool fastForward = false)
{
	plNotifyMsg *msg = TRACKED_NEW plNotifyMsg;

	if (fastForward)
	{
		msg->fType = plNotifyMsg::kResponderFF;
	}
	else
	{
		msg->fType = plNotifyMsg::kActivator;
	}

	msg->fState = 1;	// Triggered

	// Setup the event data in case this is a OneShot responder that needs it
	plKey playerKey = plNetClientMgr::GetInstance()->GetLocalPlayerKey();
	msg->AddPickEvent(playerKey, nil, true, hsPoint3(0,0,0) );

	if (responderState != -1)
		msg->AddResponderStateEvent(responderState);
	
	// Send it to the responder modifier
	msg->AddReceiver(responderKey);
	plgDispatch::MsgSend(msg);
}

PF_CONSOLE_CMD( Logic, TriggerResponderNum, "int responderNum, ...", "Triggers the responder with this number (from ListResponders). (Optional: number of the state to switch to)")
{
	if (numParams > 2)
	{
		PrintString("Too many parameters");
		return;
	}

	std::vector<std::string> responderNames;
	plKeyFinder::Instance().GetResponderNames(responderNames);

	int responderNum = params[0];
	if (responderNum < 1 || responderNum > responderNames.size())
	{
		PrintString("Responder number out of range");
		return;
	}

	int responderState = -1;
	if (numParams == 2)
	{
		responderState = params[1];
	}

	char str[256];
	plKey key = FindObjectByNameAndType(responderNames[responderNum-1].c_str(), "plResponderModifier", nil, str, true);
	PrintString(str);

	if (key)
		ResponderSendTrigger(key, responderState);
}

PF_CONSOLE_CMD( Logic, TriggerResponder, "string responderComp, ...", "Triggers the named responder component. (Optional: number of the state to switch to)")
{
	if (numParams > 2)
	{
		PrintString("Too many parameters");
		return;
	}
	
	char str[256];
	plKey key = FindObjectByNameAndType(params[0], "plResponderModifier", nil, str, true);
	PrintString(str);

	int responderState = -1;
	if (numParams == 2)
	{
		responderState = params[1];
	}

	if (key)
		ResponderSendTrigger(key, responderState);
}

PF_CONSOLE_CMD( Logic, FastForwardResponder, "string responderComp, ...", "Fastforwards the named responder component. (Optional: number of the state to switch to)")
{
	if (numParams > 2)
	{
		PrintString("Too many parameters");
		return;
	}
	
	char str[256];
	plKey key = FindObjectByNameAndType(params[0], "plResponderModifier", nil, str, true);
	PrintString(str);

	int responderState = -1;
	if (numParams == 2)
	{
		responderState = params[1];
	}

	if (key)
		ResponderSendTrigger(key, responderState, true);
}

PF_CONSOLE_CMD(Logic, ListDetectors, "", "Prints the names of the loaded detectors to the console")
{
	std::vector<std::string> activatorNames;
	plKeyFinder::Instance().GetActivatorNames(activatorNames);

	for (int i = 0; i < activatorNames.size(); i++)
	{
		char buf[256];
		sprintf(buf, "%d. %s", i+1, activatorNames[i].c_str());
		PrintString(buf);
	}
}

PF_CONSOLE_CMD(Logic, ListResponders, "", "Prints the names of the loaded responders to the console")
{
	std::vector<std::string> responderNames;
	plKeyFinder::Instance().GetResponderNames(responderNames);

	for (int i = 0; i < responderNames.size(); i++)
	{
		char buf[256];
		sprintf(buf, "%d. %s", i+1, responderNames[i].c_str());
		PrintString(buf);
	}
}

#include "../plModifier/plResponderModifier.h"

PF_CONSOLE_CMD(Logic, ResponderAnimCue, "", "Toggle box being drawn on screen when a responder starts an anim")
{
	if (plResponderModifier::ToggleDebugAnimBox())
		PrintString("Responder Anim Cue On");
	else
		PrintString("Responder Anim Cue Off");
}

PF_CONSOLE_CMD(Logic, ResponderNoLog, "string prefix", "Don't log responders that begin with the specified string")
{
	plResponderModifier::NoLogString(params[0]);
}

#include "../plModifier/plDetectorLog.h"
PF_CONSOLE_CMD(Logic, WriteDetectorLog, "", "Write detector log to logfile")
{
	DetectorDoLogfile();
}

#endif // LIMIT_CONSOLE_COMMANDS

////////////////////////////////////////////////////////////////////////
//// Audio System Group Commands ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////
PF_CONSOLE_GROUP( Audio )

PF_CONSOLE_CMD( Audio, Enable, "bool on", "Switch DirectX Audio on or off at runtime")
{
	bool on = params[0];
	plgAudioSys::Activate( on );
}

PF_CONSOLE_CMD( Audio, UseHardware, "bool on", "Enable audio hardware acceleration")
{
	bool on = params[0];
	plgAudioSys::SetUseHardware( on );
}

PF_CONSOLE_CMD( Audio, UseEAX, "bool on", "Enable EAX sound acceleration (requires hardware acceleration)")
{
	bool on = params[0];
	plgAudioSys::EnableEAX( on );
}

PF_CONSOLE_CMD( Audio, Initialize, "bool on", "Set to false to completely disable audio playback in plasma")
{ 
	bool on = params[0];
	plgAudioSys::SetActive(on);
}

PF_CONSOLE_CMD( Audio, Restart, "", "Restarts the audio system" )
{ 
	plgAudioSys::Restart();
}

PF_CONSOLE_CMD( Audio, MuteAll, "bool on", "Mute or unmute all sounds")
{
	plgAudioSys::SetMuted( (bool)params[ 0 ] );
}

PF_CONSOLE_CMD( Audio, SetDistanceModel, "int type", "Sets the distance model for all 3d sounds")
{
	if(plgAudioSys::Sys())
	{
		plgAudioSys::Sys()->SetDistanceModel((int) params[0]);
	}
}

PF_CONSOLE_CMD( Audio, LogStreamingUpdates, "bool on", "Logs every buffer fill for streaming sounds")
{
	plgAudioSys::SetLogStreamingUpdates((bool) params[0]);
}

PF_CONSOLE_CMD( Audio, SetAllChannelVolumes, "float soundFX, float music, float ambience, float voice, float gui", "Sets the master volume of all the given audio channels.")
{
	plgAudioSys::ASChannel	chans[ 6 ] = { plgAudioSys::kSoundFX, plgAudioSys::kBgndMusic, plgAudioSys::kAmbience, plgAudioSys::kVoice, plgAudioSys::kGUI, plgAudioSys::kNPCVoice };


	int i;

	for( i = 0; i < 5; i++ )
	{
		hsScalar	vol = (hsScalar)(float)params[ i ];
		if( vol > 1.f )
			vol = 1.f;
		else if( vol < 0.f )
			vol = 0.f;

		plgAudioSys::SetChannelVolume( chans[ i ], vol );
	}
}

PF_CONSOLE_CMD( Audio, SetChannelVolume, "string channel, float percentage", "Sets the master volume of a given audio channel\n\
Valid channels are: SoundFX, BgndMusic, Voice, GUI, NPCVoice and Ambience.")
{
	plgAudioSys::ASChannel	chan;


	if( stricmp( params[ 0 ], "SoundFX" ) == 0 )
		chan = plgAudioSys::kSoundFX;
	else if( stricmp( params[ 0 ], "BgndMusic" ) == 0 )
		chan = plgAudioSys::kBgndMusic;
	else if( stricmp( params[ 0 ], "Voice" ) == 0 )
		chan = plgAudioSys::kVoice;
	else if( stricmp( params[ 0 ], "Ambience" ) == 0 )
		chan = plgAudioSys::kAmbience;
	else if( stricmp( params[ 0 ], "GUI" ) == 0 )
		chan = plgAudioSys::kGUI;
	else if( stricmp( params[ 0 ], "NPCVoice" ) == 0 )
		chan = plgAudioSys::kNPCVoice;
	else
	{
		PrintString( "Invalid channel specified. Use SoundFX, BgndMusic, Voice, Ambience or GUI." );
		return;
	}

	hsScalar	vol = (hsScalar)(float)params[ 1 ];
	if( vol > 1.f )
		vol = 1.f;
	else if( vol < 0.f )
		vol = 0.f;

	plgAudioSys::SetChannelVolume( chan, vol );

	char	msg[ 128 ];
	switch( chan )
	{
		case plgAudioSys::kSoundFX:		sprintf( msg, "Setting SoundFX master volume to %4.2f", vol ); break;
		case plgAudioSys::kBgndMusic:	sprintf( msg, "Setting BgndMusic master volume to %4.2f", vol ); break;
		case plgAudioSys::kVoice:		sprintf( msg, "Setting Voice master volume to %4.2f", vol ); break;
		case plgAudioSys::kAmbience:	sprintf( msg, "Setting Ambience master volume to %4.2f", vol ); break;
		case plgAudioSys::kGUI:			sprintf( msg, "Setting GUI master volume to %4.2f", vol ); break;
		case plgAudioSys::kNPCVoice:	sprintf( msg, "Setting NPC Voice master volume to %4.2f", vol ); break;
	}
	PrintString( msg );
}

PF_CONSOLE_CMD( Audio, Set2D3DBias, "float bias", "Sets the 2D/3D bias when not using hardware acceleration.")
{

	hsScalar	bias = (hsScalar)(float)params[ 0 ];
	plgAudioSys::Set2D3DBias( bias );

}

PF_CONSOLE_CMD( Audio, ShowNumActiveBuffers, "bool b", "Shows the number of Direct sounds buffers in use")
{
	plgAudioSys::ShowNumBuffers((bool)params[0]);
}

PF_CONSOLE_CMD( Audio, SetDeviceName, "string deviceName", "Meant for plClient init only")
{
	plgAudioSys::SetDeviceName(params[0]);		// this will set the name of the audio system device without actually reseting it
}

PF_CONSOLE_CMD( Audio,		// groupName
			   EnableVoiceCompression,		// fxnName
			   "bool b", // paramList
			   "turn voice compression on and off" )	// helpString
{
	bool b = params[0];
	plVoiceRecorder::EnableCompression(b);

}

PF_CONSOLE_CMD( Audio,		// groupName
			   ShowIcons,		// fxnName
			   "bool b", // paramList
			   "turn voice recording icons on and off" )	// helpString
{
	bool b = params[0];
	plVoiceRecorder::EnableIcons(b);

}

PF_CONSOLE_CMD( Audio,		// groupName
			   SquelchLevel,		// fxnName
			   "float f", // paramList
			   "Set the squelch level" )	// helpString
{
	float f = params[0];
	plVoiceRecorder::SetSquelch(f);

}


PF_CONSOLE_CMD( Audio,		// groupName
			   PushToTalk,		// fxnName
			   "bool b", // paramList
			   "turn push-to-talk on or off" )	// helpString
{
	bool b = params[0];
	plVoiceRecorder::EnablePushToTalk(b);

}

PF_CONSOLE_CMD( Audio,		// groupName
			   EnableVoiceNetBroadcast,		// fxnName
			   "bool b", // paramList
			   "turn voice-over-net on and off" )	// helpString
{
	bool b = params[0];
	plVoiceRecorder::EnableNetVoice(b);

}

PF_CONSOLE_CMD( Audio,								// groupName
			   SetVoiceQuality,						// fxnName
			   "int q",								// paramList
			   "Set quality of voice encoding" )	// helpString
{
	int q = params[0];
	plVoiceRecorder::SetQuality(q);
}


PF_CONSOLE_CMD( Audio,		// groupName
			   SetVBR,		// fxnName
			   "bool q",	// paramList
			   "Toggle variable bit rate" )	// helpString
{
	bool q = params[0];
	plVoiceRecorder::SetVBR(q);
}

PF_CONSOLE_CMD( Audio,		// groupName
			   EnableVoiceRecording,		// fxnName
			   "bool b", // paramList
			   "turn voice recording on or off" )	// helpString
{
	bool b = params[0];
	plVoiceRecorder::EnableRecording(b);

}

PF_CONSOLE_CMD( Audio,
				EnableVoiceChat,
				"bool b",
				"Enable Voice chat" )
{
	plVoicePlayer::Enable((bool) params[0]);
	plVoiceRecorder::EnableRecording((bool) params[0]);
}

#ifndef LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_CMD( Audio, NextDebugPlate, "", "Cycles through the volume displays for all registered sounds" )
{
	plgAudioSys::NextDebugSound();
}

#endif // LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_CMD( Audio, SetLoadOnDemand, "bool on", "Enable or disable load-on-demand for sounds")
{
	plSound::SetLoadOnDemand( (bool)params[ 0 ] );
}

PF_CONSOLE_CMD( Audio, SetTwoStageLOD, "bool on", "Enables or disables two-stage LOD, where sounds can be loaded into RAM but not into sound buffers. Less of a performance hit, harder on memory.")
{
	// For two-stage LOD, we want to disable LoadFromDiskOnDemand, so that we'll load into RAM at startup but not
	// into sound buffers until demanded to do so. Enabling LoadFromDiskOnDemand basically conserves as much memory
	// as possible
	plSound::SetLoadFromDiskOnDemand( !(bool)params[ 0 ] );
}

PF_CONSOLE_CMD( Audio, SetVolume,
				"string obj, float vol", "Sets the volume on a given object. 1 is max volume, 0 is silence" )
{
	char	str[ 256 ];
	plKey key = FindSceneObjectByName(params[ 0 ], nil, str);
	if( key == nil )
		return;

	plSceneObject* obj = plSceneObject::ConvertNoRef(key->GetObjectPtr());
	if( !obj )
		return;

	const plAudioInterface	*ai = obj->GetAudioInterface();
	plKey	aiKey = ai->GetKey();

	plSoundMsg* cmd = TRACKED_NEW plSoundMsg;
	cmd->SetSender( plClient::GetInstance()->GetKey() );
	cmd->SetCmd( plSoundMsg::kSetVolume );
	cmd->fVolume = params[ 1 ];
	cmd->AddReceiver( key );
	plgDispatch::MsgSend(cmd);
}


#ifndef LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_CMD( Audio, IsolateSound,
				"string soundComponentName", "Mutes all sounds except the given sound. Use Audio.MuteAll false to remove the isolation." )
{
	char			str[ 512 ];
	plKey			key;
	plAudioSysMsg	*asMsg;

	key = FindSceneObjectByName( params[ 0 ], nil, str );
	if( key == nil )
	{
		sprintf( str, "Cannot find sound %s", (char *)params[ 0 ] );
		PrintString( str );
		return;
	}

	plSceneObject *obj = plSceneObject::ConvertNoRef( key->GetObjectPtr() );
	if( !obj )
	{
		sprintf( str, "Cannot get sceneObject %s", (char *)params[ 0 ] );
		PrintString( str );
		return;
	}

	const plAudioInterface	*ai = obj->GetAudioInterface();
	if( ai == nil )
	{
		sprintf( str, "sceneObject %s has no audio interface", (char *)params[ 0 ] );
		PrintString( str );
		return;
	}

	asMsg = TRACKED_NEW plAudioSysMsg( plAudioSysMsg::kMuteAll );
	plgDispatch::MsgSend( asMsg );

	asMsg = TRACKED_NEW plAudioSysMsg( plAudioSysMsg::kUnmuteAll );
	asMsg->AddReceiver( ai->GetKey() );
	asMsg->SetBCastFlag( plMessage::kBCastByExactType, false );
	plgDispatch::MsgSend( asMsg );


	char str2[ 256 ];
	sprintf( str2, "Sounds on sceneObject %s isolated.", (char *)params[ 0 ] );
	PrintString( str2 );
}

#endif // LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_CMD( Audio, SetMicVolume, "float volume", "Sets the microphone volume, in the range of 0 to 1" )
{
	if( !plWinMicLevel::CanSetLevel() )
		PrintString( "Unable to set microphone level" );
	else
	{
		plWinMicLevel::SetLevel( (float)params[ 0 ] );
	}
}

#ifndef LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_CMD( Audio, MCNTest, "int which", "" )
{
	if( (int)params[ 0 ] == 0 )
		plgAudioSys::ClearDebugFlags();
	else if( (int)params[ 0 ] == 1 )
		plgAudioSys::SetDebugFlag( plgAudioSys::kDisableRightSelect );
	else if( (int)params[ 0 ] == 2 )
		plgAudioSys::SetDebugFlag( plgAudioSys::kDisableLeftSelect );
}

PF_CONSOLE_CMD( Audio, Mark, "", "" )
{
	static int markNum = 0;
	plStatusLog::AddLineS( "threadfun.log", "******* Mark #%d *******", markNum++ );
}

#endif // LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_CMD( Audio, SetStreamingBufferSize, "float sizeInSecs", "Sets the size of the streaming buffer for each streaming sound." )
{
	plgAudioSys::SetStreamingBufferSize( (float)params[ 0 ] );
	PrintString( "Changes won't take effect until you restart the audio system." );
}

PF_CONSOLE_CMD( Audio, SetStreamFromRAMCutoff, "float cutoffInSecs", "Sets the cutoff between streaming from RAM and streaming directly from disk." )
{
	plgAudioSys::SetStreamFromRAMCutoff( (float)params[ 0 ] );
	PrintString( "Changes won't take effect until you restart the audio system." );
}

PF_CONSOLE_CMD( Audio, SetPriorityCutoff, "int cutoff", "Stops sounds from loading whose priority is greater than this cutoff." )
{
	plgAudioSys::SetPriorityCutoff( (int)params[ 0 ] );
}

PF_CONSOLE_CMD( Audio, EnableExtendedLogs, "bool enable", "Enables or disables the extended audio logs." )
{
	plgAudioSys::EnableExtendedLogs( (bool)params[ 0 ] );
}

PF_CONSOLE_GROUP( Listener )

#ifndef LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_CMD( Listener, ShowDebugInfo, "bool show", "Shows or hides debugging info")
{
	plListener::ShowDebugInfo( (bool)params[ 0 ] );
}

#endif // LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_CMD( Listener, UseCameraOrientation, "", "Use the camera's orientation to orient the listener")
{
	plSetListenerMsg *set = TRACKED_NEW plSetListenerMsg( plSetListenerMsg::kVCam | plSetListenerMsg::kFacing, nil, true );
	set->Send();
}

PF_CONSOLE_CMD( Listener, UseCameraPosition, "", "Use the canera's position to position the listener")
{
	plSetListenerMsg *set = TRACKED_NEW plSetListenerMsg( plSetListenerMsg::kVCam | plSetListenerMsg::kPosition, nil, true );
	set->Send();
}

PF_CONSOLE_CMD( Listener, UseCameraVelocity, "", "Use the camera's velocity to set the listener velocity")
{
	plSetListenerMsg *set = TRACKED_NEW plSetListenerMsg( plSetListenerMsg::kVCam | plSetListenerMsg::kVelocity, nil, true );
	set->Send();
}

PF_CONSOLE_CMD( Listener, UsePlayerOrientation, "", "Use the player's orientation to orient the listener")
{
	plKey pKey = plNetClientMgr::GetInstance()->GetLocalPlayerKey();
	if( pKey )
	{
		plSetListenerMsg *set = TRACKED_NEW plSetListenerMsg( plSetListenerMsg::kFacing, pKey, true );
		set->Send();
	}
}
PF_CONSOLE_CMD( Listener, UsePlayerPosition, "", "Use the player's position to position the listener")
{
	plKey pKey = plNetClientMgr::GetInstance()->GetLocalPlayerKey();
	if (pKey)
	{
		plSetListenerMsg *set = TRACKED_NEW plSetListenerMsg( plSetListenerMsg::kPosition, pKey, true );
		set->Send();
	}
}

PF_CONSOLE_CMD( Listener, UsePlayerVelocity, "", "Use the player's velocity to set the listener velocity")
{
	plKey pKey = plNetClientMgr::GetInstance()->GetLocalPlayerKey();
	if (pKey)
	{
		plSetListenerMsg *set = TRACKED_NEW plSetListenerMsg( plSetListenerMsg::kVelocity, pKey, true );
		set->Send();
	}
}

PF_CONSOLE_CMD( Listener, XMode, "bool b", "Sets velocity and position to avatar, and orientation to camera")
{
	static UInt32 oldPosType = 0, oldFacingType = 0, oldVelType = 0;
	
	plSetListenerMsg *set = nil;
	plKey pKey = plNetClientMgr::GetInstance()->GetLocalPlayerKey();
	plListener* pListener;

	if( (bool)params[ 0 ] )
	{
		// Get the listener object
		plUoid lu(kListenerMod_KEY);
		plKey pLKey = hsgResMgr::ResMgr()->FindKey(lu);
		if (pLKey)
		{	
			pListener = plListener::ConvertNoRef(pLKey->GetObjectPtr());
		}

		if(pListener)
		{
			// Save old types
			oldPosType = pListener->GetAttachedPosType();
			oldFacingType = pListener->GetAttachedFacingType();
			oldVelType = pListener->GetAttachedVelType();
		}
		
		plStatusLog::AddLineS("audio.log", "XMode on");
		
		plSetListenerMsg *set = TRACKED_NEW plSetListenerMsg( plSetListenerMsg::kVCam | plSetListenerMsg::kFacing, nil, true );
		set->Send();
		if (pKey)
		{
			set = TRACKED_NEW plSetListenerMsg( plSetListenerMsg::kVelocity, pKey, true );
			set->Send();
			set = TRACKED_NEW plSetListenerMsg( plSetListenerMsg::kPosition, pKey, true );
			set->Send();
		}
	}
	else 
	{
		if(oldPosType == plListener::kCamera)
		{
			plSetListenerMsg *set = TRACKED_NEW plSetListenerMsg( plSetListenerMsg::kVCam | plSetListenerMsg::kPosition, nil, true );
			set->Send();
		}
		else
		{
			set = TRACKED_NEW plSetListenerMsg( plSetListenerMsg::kPosition, pKey, true );
			set->Send();
		}
		if(oldFacingType == plListener::kCamera)
		{
			set = TRACKED_NEW plSetListenerMsg( plSetListenerMsg::kVCam | plSetListenerMsg::kFacing, nil, true );
			set->Send();
		}
		else
		{
			set = TRACKED_NEW plSetListenerMsg( plSetListenerMsg::kFacing, pKey, true );
			set->Send();
		}
		if(oldVelType == plListener::kCamera)
		{
			set = TRACKED_NEW plSetListenerMsg( plSetListenerMsg::kVCam | plSetListenerMsg::kVelocity, nil, true );
			set->Send();
		}
		else
		{
			set = TRACKED_NEW plSetListenerMsg( plSetListenerMsg::kVelocity, pKey, true );
			set->Send();
		}
		plStatusLog::AddLineS("audio.log", "%s, %d, %d, %d", "XMode off", oldPosType, oldFacingType, oldVelType);
	}
}



////////////////////////////////////////////////////////////////////////
//// Input System Group Commands ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////

#ifndef LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_GROUP( DInput )

PF_CONSOLE_CMD( DInput, UseDInput, "bool on", "Turns off DirectInput")
{
	bool on = params[0];
	plInputManager::UseDInput(on);
}


PF_CONSOLE_CMD( DInput, Config, "", "Launch DInput configuration screen")
{
	plgAudioSys::Activate(false);
	plInputEventMsg* pMsg = TRACKED_NEW plInputEventMsg;
	pMsg->fEvent = plInputEventMsg::kConfigure;
	pMsg->AddReceiver( plInputManager::GetInstance()->GetKey() );
	pMsg->SetBCastFlag(plMessage::kBCastByType, false);
	plgDispatch::MsgSend(pMsg);
}

#endif // LIMIT_CONSOLE_COMMANDS


//////////////////////////////////////////////////////////////////////////////
//// Keyboard Remapping Group Commands ///////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

PF_CONSOLE_GROUP( Keyboard )		// Defines a main command group

PF_CONSOLE_CMD( Keyboard, ResetBindings, "", "Resets the keyboard bindings to their defaults" )
{
	if( plInputInterfaceMgr::GetInstance() != nil )
		plInputInterfaceMgr::GetInstance()->InitDefaultKeyMap();

	PrintString( "Keyboard bindings reset" );
}


PF_CONSOLE_CMD( Keyboard, ClearBindings, "", "Resets the keyboard bindings to empty" )
{
	if( plInputInterfaceMgr::GetInstance() != nil )
		plInputInterfaceMgr::GetInstance()->ClearAllKeyMaps();

	PrintString( "Keyboard bindings destroyed" );
}

static plKeyCombo	IBindKeyToVKey( const char *string )
{
	char	str[ 16 ];
	int		i;

	plKeyCombo	combo;


	strcpy( str, string );

	// Find modifiers to set flags with
	combo.fFlags = 0;
	if( strstr( str, "_S" ) || strstr( str, "_s" ) )
		combo.fFlags |= plKeyCombo::kShift;
	if( strstr( str, "_C" ) || strstr( str, "_c" ) )
		combo.fFlags |= plKeyCombo::kCtrl;
	
	// Get rid of modififers
	for( i = 0; str[ i ] != 0 && str[ i ] != '_'; i++ );
	str[ i ] = 0;

	// Convert raw key
	combo.fKey = plKeyMap::ConvertCharToVKey( str );
	if( combo.fKey == KEY_UNMAPPED )
		combo = plKeyCombo::kUnmapped;

	// And return!
	return combo;

}

static ControlEventCode	IBindStringToCmdCode( const char *string )
{
	return plKeyMap::ConvertCharToControlCode( string );
}

PF_CONSOLE_CMD( Keyboard,		// groupName
			   BindKey,		// fxnName
			   "string key1, string action", // paramList
			   "Binds the given single key combo to the given action" )	// helpString
{
	ControlEventCode code = IBindStringToCmdCode( params[ 1 ] );
	if( code == END_CONTROLS )
	{
		PrintString( "ERROR: Invalid command name" );
		return;
	}

	if( plInputInterfaceMgr::GetInstance() != nil )
	{
		plKeyCombo key1 = IBindKeyToVKey( params[ 0 ] );
		plInputInterfaceMgr::GetInstance()->BindAction( key1, code );
	}
}

PF_CONSOLE_CMD( Keyboard,		// groupName
			   BindAction,		// fxnName
			   "string key1, string key2, string action", // paramList
			   "Binds the given two keys to the given action (you can specify 'UNDEFINED' for either key if you wish)" )	// helpString
{
	ControlEventCode code = IBindStringToCmdCode( params[ 2 ] );
	if( code == END_CONTROLS )
	{
		PrintString( "ERROR: Invalid command name" );
		return;
	}

	if( plInputInterfaceMgr::GetInstance() != nil )
	{
		plKeyCombo key1 = IBindKeyToVKey( params[ 0 ] );
		plKeyCombo key2 = IBindKeyToVKey( params[ 1 ] );
		plInputInterfaceMgr::GetInstance()->BindAction( key1, key2, code );
	}
}

PF_CONSOLE_CMD( Keyboard,		// groupName
			   BindConsoleCmd,		// fxnName
			   "string key, string command", // paramList
			   "Bind console command to key" )	// helpString
{
	plKeyCombo key = IBindKeyToVKey( params[ 0 ] );

	if( plInputInterfaceMgr::GetInstance() != nil )
		plInputInterfaceMgr::GetInstance()->BindConsoleCmd( key, params[ 1 ], plKeyMap::kFirstAlways );
}


//////////////////////////////////////////////////////////////////////////////
//// Stat Gather Commands ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

PF_CONSOLE_GROUP( Nav )

PF_CONSOLE_CMD( Nav, PageInNode,	// Group name, Function name
				"string roomName",			// Params
				"Pages in a scene node." )	// Help string
{
	plSynchEnabler ps(false);	// disable dirty tracking while paging in
	plClientMsg* pMsg1 = TRACKED_NEW plClientMsg(plClientMsg::kLoadRoom);
	pMsg1->AddReceiver( plClient::GetInstance()->GetKey() );
	pMsg1->AddRoomLoc(plKeyFinder::Instance().FindLocation(nil, params[0]));
	plgDispatch::MsgSend(pMsg1);
}

PF_CONSOLE_CMD( Nav, PageInNodeList,	// Group name, Function name
               "string roomNameBase",			// Params
               "Pages in all scene nodes that start with name." )	// Help string
{
	plSynchEnabler ps(false);	// disable dirty tracking while paging in

	std::string pageInNodesStr;
	pageInNodesStr += "dat\\";
	pageInNodesStr += (char*)params[0];
	pageInNodesStr += "*.prx";
	hsFolderIterator pageInNodesIter(pageInNodesStr.data(), true);

	plClientMsg* pMsg1 = TRACKED_NEW plClientMsg(plClientMsg::kLoadRoom);
	while (pageInNodesIter.NextFile()) 
	{
		char nodeName[255];
		_splitpath(pageInNodesIter.GetFileName(), NULL, NULL, nodeName, NULL);
		pMsg1->AddRoomLoc(plKeyFinder::Instance().FindLocation(nil, nodeName));
	}
	pMsg1->AddReceiver( plClient::GetInstance()->GetKey() );
	plgDispatch::MsgSend(pMsg1);
}

#ifndef LIMIT_CONSOLE_COMMANDS


PF_CONSOLE_CMD( Nav, PageOutNode,	// Group name, Function name
				"string roomName",			// Params
				"pages out a scene node." )	// Help string
{
	plSynchEnabler ps(false);	// disable dirty tracking while paging out
	plClientMsg* pMsg1 = TRACKED_NEW plClientMsg(plClientMsg::kUnloadRoom);
	pMsg1->AddReceiver( plClient::GetInstance()->GetKey() );
	pMsg1->AddRoomLoc(plKeyFinder::Instance().FindLocation(nil, params[0]));
	plgDispatch::MsgSend(pMsg1);
}

PF_CONSOLE_CMD( Nav, UnloadPlayer,	// Group name, Function name
				"string objName",			// Params
				"unloads a named player" )	// Help string
{
	char str[256];
	plKey key = FindSceneObjectByName(params[0], nil, str);
	PrintString("UnloadPlayer (console version) is currently broken. Hassle Matt.");
	// plNetClientMgr::UnloadPlayer(key);
}

PF_CONSOLE_CMD( Nav, UnloadSceneObject,	// Group name, Function name
				"string objName",			// Params
				"unloads a named scene object" )	// Help string
{

	PrintString("OBSOLETE");
}

PF_CONSOLE_CMD( Nav, MovePlayer,	// Group name, Function name
				"string playerName, string destPage",			// Params
				"moves a player from one paging unit to another" )	// Help string
{
	char str[256];
	plKey playerKey = FindSceneObjectByName(params[0], nil, str);
	PrintString(str);
	if( !playerKey )
		return;

	plKey nodeKey = FindObjectByName(params[1], plSceneNode::Index(), nil, str);
	PrintString(str);
	if( !nodeKey )
		return;

	plNodeChangeMsg* msg = TRACKED_NEW plNodeChangeMsg(nil, playerKey, nodeKey);
	plgDispatch::MsgSend(msg);
	sprintf(str, "%s moved to %s", (char*)params[0], (char*)params[1]);
	PrintString(str);
}

PF_CONSOLE_CMD( Nav, ExcludePage, "string pageName", "Excludes the given page from ever being loaded. Useful for debugging." )
{
	if( plNetClientMgr::GetInstance() == nil )
		PrintString( "Unable to exclude page--NetClientMgr not loaded" );
	else
	{
		char	str[ 256 ];
		sprintf( str, "Page %s excluded from load", (char *)params[ 0 ] );
		plAgeLoader::GetInstance()->AddExcludedPage( params[ 0 ] );
		PrintString( str );
	}
}

PF_CONSOLE_CMD( Nav, ClearExcludeList, "", "Clears the list of pages to exclude from loading." )
{
	if( plAgeLoader::GetInstance() != nil )
		plAgeLoader::GetInstance()->ClearPageExcludeList();
}

#endif // LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_GROUP( Movie ) // Defines a main command group

PF_CONSOLE_CMD( Movie,
					Start,
				   "string filename",
				   "Start of movie with this filename" )
{
	char* filename = params[0];
	plMovieMsg* mov = TRACKED_NEW plMovieMsg(filename, plMovieMsg::kStart);

//#define MF_TEST_MOVIECALLBACKS
#ifdef MF_TEST_MOVIECALLBACKS
	plMovieMsg* cb = TRACKED_NEW plMovieMsg("avi/intro0.bik", plMovieMsg::kStart);
	mov->AddCallback(cb);
	mov->SetCmd(mov->GetCmd() | plMovieMsg::kAddCallbacks);
#endif // MF_TEST_MOVIECALLBACKS

	mov->Send();

	PrintStringF(PrintString, "%s now playing", filename);
}

PF_CONSOLE_CMD( Movie,
					Stop,
				   "string filename",
				   "Stop movie with this filename" )
{
	char* filename = params[0];
	plMovieMsg* mov = TRACKED_NEW plMovieMsg(filename, plMovieMsg::kStop);
	mov->Send();

	PrintStringF(PrintString, "%s now stopping", filename);
}

PF_CONSOLE_CMD( Movie,
					Pause,
				   "string filename",
				   "Pause movie with this filename" )
{
	char* filename = params[0];
	plMovieMsg* mov = TRACKED_NEW plMovieMsg(filename, plMovieMsg::kPause);
	mov->Send();

	PrintStringF(PrintString, "%s now pausing", filename);
}

PF_CONSOLE_CMD( Movie,
					Resume,
				   "string filename",
				   "Resume movie with this filename" )
{
	char* filename = params[0];
	plMovieMsg* mov = TRACKED_NEW plMovieMsg(filename, plMovieMsg::kResume);
	mov->Send();

	PrintStringF(PrintString, "%s now resuming", filename);
}

PF_CONSOLE_CMD( Movie,
					Move,
				   "string filename, float x, float y",
				   "Move center of movie with this filename to x,y" )
{
	char* filename = params[0];
	plMovieMsg* mov = TRACKED_NEW plMovieMsg(filename, plMovieMsg::kMove);
	float x = params[1];
	float y = params[2];
	mov->SetCenter(x, y);
	mov->Send();

	PrintStringF(PrintString, "%s now at %g,%g", filename, x, y);
}

PF_CONSOLE_CMD( Movie,
					Scale,
				   "string filename, float x, float y",
				   "Scale movie with this filename by x,y" )
{
	char* filename = params[0];
	plMovieMsg* mov = TRACKED_NEW plMovieMsg(filename, plMovieMsg::kScale);
	float x = params[1];
	float y = params[2];
	mov->SetScale(x, y);
	mov->Send();

	PrintStringF(PrintString, "%s now scaled to %g,%g", filename, x, y);
}

PF_CONSOLE_CMD( Movie,
					Alpha,
				   "string filename, float a",
				   "Set opacity of movie with this filename to a" )
{
	char* filename = params[0];
	plMovieMsg* mov = TRACKED_NEW plMovieMsg(filename, plMovieMsg::kOpacity);
	float a = params[1];
	mov->SetOpacity(a);
	mov->Send();

	PrintStringF(PrintString, "%s opacity now at %g", filename, a);
}

PF_CONSOLE_CMD( Movie,
					Color,
				   "string filename, float r, float g, float b",
				   "Color movie with this filename as r,g,b" )
{
	char* filename = params[0];
	plMovieMsg* mov = TRACKED_NEW plMovieMsg(filename, plMovieMsg::kColor);
	float r = params[1];
	float g = params[2];
	float b = params[3];
	mov->SetColor(r, g, b, 1.f);
	mov->Send();

	PrintStringF(PrintString, "%s now tinted to %g,%g,%g", filename, r, g, b);
}

PF_CONSOLE_CMD( Movie,
					Volume,
				   "string filename, float v",
				   "Set volume of movie with this filename to v" )
{
	char* filename = params[0];
	plMovieMsg* mov = TRACKED_NEW plMovieMsg(filename, plMovieMsg::kVolume);
	float v = params[1];
	mov->SetVolume(v);
	mov->Send();

	PrintStringF(PrintString, "%s volume now at %g", filename, v);
}

PF_CONSOLE_CMD( Movie,
					FadeIn,
				   "string filename, float secs, float r, float g, float b, float a",
				   "Fade in movie with this filename from r,g,b,a over secs seconds" )
{
	char* filename = params[0];
	plMovieMsg* mov = TRACKED_NEW plMovieMsg(filename, plMovieMsg::kFadeIn);
	float secs = params[1];
	float r = params[2];
	float g = params[3];
	float b = params[4];
	float a = params[5];
	mov->SetFadeInSecs(secs);
	mov->SetFadeInColor(r, g, b, a);
	mov->Send();

	PrintStringF(PrintString, "%s now fading from %g,%g,%g,%g over %g secs", filename, r, g, b, a, secs);
}

PF_CONSOLE_CMD( Movie,
					FadeOut,
				   "string filename, float secs, float r, float g, float b, float a",
				   "Fade out movie with this filename to r,g,b,a over secs seconds" )
{
	char* filename = params[0];
	plMovieMsg* mov = TRACKED_NEW plMovieMsg(filename, plMovieMsg::kFadeOut);
	float secs = params[1];
	float r = params[2];
	float g = params[3];
	float b = params[4];
	float a = params[5];
	mov->SetFadeOutSecs(secs);
	mov->SetFadeOutColor(r, g, b, a);
	mov->Send();

	PrintStringF(PrintString, "%s now fading to %g,%g,%g,%g over %g secs", filename, r, g, b, a, secs);
}



//////////////////////////////////////////////////////////////////////////////
// Test hacks for setting overall quality and clamping board capability
//////////////////////////////////////////////////////////////////////////////
PF_CONSOLE_GROUP( Quality )

PF_CONSOLE_CMD( Quality,
					Level,
				   "int quality",
				   "Fake quality slider from .ini file" )
{
	int q = params[0];
	if( q < 0 )
		q = 0;
	else if (q > 3)
		q = 3;
	plClient::GetInstance()->SetQuality(q);

	char str[256];
	sprintf(str, "Quality slider to %d", q);
	PrintString(str);
}

#include "../plSurface/plShaderTable.h"

PF_CONSOLE_CMD( Quality,
					Cap,
				   "int cap",
				   "Limit graphics capability from .ini file" )
{
	int c = params[0];
	if( c < 0 )
		c = 0;
	if( c == 666 )
		plShaderTable::SetLoadFromFile(true);
	else
		plClient::GetInstance()->SetClampCap(c);

	char str[256];
	sprintf(str, "Graphics capability clamped to %d", c);
	PrintString(str);
}


//////////////////////////////////////////////////////////////////////////////
//// Geometry Object Access Console Commands (strictly for testing) ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#ifndef LIMIT_CONSOLE_COMMANDS

#include "../plDrawable/plSharedMesh.h"
#include "../plDrawable/plAccessGeometry.h"
#include "../plDrawable/plMorphSequence.h"
#include "../plAvatar/plAvatarClothing.h"

PF_CONSOLE_GROUP( Access )


/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

static plMorphSequence* LocalMorphSequence()
{
	plKey playerKey = plNetClientMgr::GetInstance()->GetLocalPlayerKey();
	if( !playerKey )
		return nil;
	plSceneObject* playerObj = plSceneObject::ConvertNoRef(playerKey->ObjectIsLoaded());
	if( !playerObj )
		return nil;

	const plCoordinateInterface* pci = playerObj->GetCoordinateInterface();
	const plModifier* constSeq = nil;
	int i;
	for( i = 0; i < pci->GetNumChildren(); i++ )
	{
		const plSceneObject* child = pci->GetChild(i)->GetOwner();
		if( child )
		{
			int j;
			for( j = 0; j < child->GetNumModifiers(); j++ )
			{
				constSeq = child->GetModifier(j);
				if( constSeq && plMorphSequence::ConvertNoRef(constSeq) )
				{
					return (plMorphSequence*)constSeq; // safe cast, we've already checked type (plus we're const_cast'ing).
				}
			}
		}
	}
	return nil;
}

PF_CONSOLE_CMD( Access,
					Morph,
				   "string morphMod, int iLay, int iDel, float wgt",
				   "Set the weight for a morphMod" )
{
	char str[256];
	char name[256];
	char* preFix = params[0];
	sprintf(name, "%s_plMorphSequence_0", preFix);
	plKey key = FindObjectByName(name, plMorphSequence::Index(), nil, str);
	PrintString(str);
	if (!key)
		return;

	plMorphSequence* seq = plMorphSequence::ConvertNoRef(key->GetObjectPtr());

	int iLay = params[1];
	int iDel = params[2];
	float wgt = params[3];

	seq->SetWeight(iLay, iDel, wgt);

	sprintf(str, "Layer[%d][%d] = %g\n", iLay, iDel, wgt);
	PrintString(str);
}

PF_CONSOLE_CMD( Access,
					MoAct,
				   "string morphMod",
				   "Activate a morphMod" )
{
	char str[256];
	char name[256];
	char* preFix = params[0];
	sprintf(name, "%s_plMorphSequence_2", preFix);
	plKey key = FindObjectByName(name, plMorphSequence::Index(), nil, str);
	PrintString(str);
	if (!key)
		return;

	plMorphSequence* seq = plMorphSequence::ConvertNoRef(key->GetObjectPtr());

	seq->Activate();

	sprintf(str, "%s Active\n", name);
	PrintString(str);
}

PF_CONSOLE_CMD( Access,
					MoDeAct,
				   "string morphMod",
				   "Activate a morphMod" )
{
	char str[256];
	char name[256];
	char* preFix = params[0];
	sprintf(name, "%s_plMorphSequence_2", preFix);
	plKey key = FindObjectByName(name, plMorphSequence::Index(), nil, str);
	PrintString(str);
	if (!key)
		return;

	plMorphSequence* seq = plMorphSequence::ConvertNoRef(key->GetObjectPtr());

	seq->DeActivate();

	sprintf(str, "%s Unactive\n", name);
	PrintString(str);
}
//////////////////
PF_CONSOLE_CMD( Access,
					Weight,
				   "int iLay, int iDel, float wgt",
				   "Set the weight for a morphMod" )
{
	plMorphSequence* seq = LocalMorphSequence();
	if( !seq )
	{
		PrintString("Sequence not found\n");
		return;
	}

	int iLay = params[0];
	int iDel = params[1];
	float wgt = params[2];

	seq->SetWeight(iLay, iDel, wgt);
	
	char str[256];
	sprintf(str, "Layer[%d][%d] = %g\n", iLay, iDel, wgt);
	PrintString(str);
}


PF_CONSOLE_CMD( Access,
			   ZeroBrian,
			   "int BeginLay, int EndLay",
			   "Zero the morph layers from begin to end" )
{
	plMorphSequence* seq = LocalMorphSequence();
	if( !seq )
	{
		PrintString("Sequence not found\n");
		return;
	}
	
	int i;
	int s = params[0];
	int e = params[1];
	
	for(i = s; i <= e; i++)
	{
		seq->SetWeight(i, 0, 0.0);
		seq->SetWeight(i, 1, 0.0);
	}
	PrintString("Zeroed");
	
}

char *gCurrMorph = nil;

PF_CONSOLE_CMD( Access,
				SetMorphItem,
				"string itemName",
				"Set which clothing item we want to morph" )
{
	delete [] gCurrMorph;
	gCurrMorph = hsStrcpy(nil, params[0]);
}

PF_CONSOLE_CMD( Access,
			   IncBrian,
			   "int iLay, float inc",
			   "Inc the weight for a morphMod pair" )
{
	plMorphSequence* seq = LocalMorphSequence();
	if( !seq )
	{
		PrintString("Sequence not found\n");
		return;
	}

	plClothingItem *item = plClothingMgr::GetClothingMgr()->FindItemByName(gCurrMorph);
	if( !item )
	{
		PrintString("Item not found");
		return;
	}

	plKey meshKey = item->fMeshes[0]->GetKey();
	int iLay = params[0];
	float inc = params[1];

	if (iLay >= seq->GetNumLayers(meshKey))
	{
		PrintString("Layer index too high");
		return;
	}	

	float wgtPlus;
	float wgtMinus;

	wgtPlus = seq->GetWeight(iLay,0,meshKey);
	wgtMinus = seq->GetWeight(iLay,1,meshKey);

	float val = wgtPlus - wgtMinus;

	val += inc;

	if(val > 1.0) val = 1.0;
	if(val < -1.0) val = -1.0;

	if(val > 0)
	{
		wgtPlus = val;
		wgtMinus = 0;
	}
	else
	{
		wgtMinus = -val;
		wgtPlus = 0;
	}

	seq->SetWeight(iLay, 0, wgtPlus, meshKey);
	seq->SetWeight(iLay, 1, wgtMinus, meshKey);

	char str[256];
	sprintf(str, "Layer[%d][%d] = %g\n", iLay, 0, wgtPlus);
	PrintString(str);

	sprintf(str, "Layer[%d][%d] = %g\n", iLay, 1, wgtMinus);
	PrintString(str);
}




PF_CONSOLE_CMD( Access,
					FaAct,
				   "",
				   "Activate face morphMod" )
{
	plMorphSequence* seq = LocalMorphSequence();
	if( !seq )
	{
		PrintString("Sequence not found\n");
		return;
	}

	seq->Activate();

	char str[256];
	sprintf(str, "%s Active\n", seq->GetKey()->GetName());
	PrintString(str);
}

PF_CONSOLE_CMD( Access,
					FaDeAct,
				   "",
				   "Deactivate a morphMod" )
{
	plMorphSequence* seq = LocalMorphSequence();
	if( !seq )
	{
		PrintString("Sequence not found\n");
		return;
	}

	seq->DeActivate();

	char str[256];
	sprintf(str, "%s Unactive\n", seq->GetKey()->GetName());
	PrintString(str);
}

PF_CONSOLE_CMD( Access,
					Face,
				   "string clothItem",
				   "Set face morphMod to affect a clothing item" )
{
	char str[256];
	plMorphSequence* seq = LocalMorphSequence();
	if( !seq )
	{
		PrintString("Sequence not found\n");
		return;
	}

	plClothingItem *item = plClothingMgr::GetClothingMgr()->FindItemByName(params[0]);
	if( !item )
		return;

	seq->SetUseSharedMesh(true);
	seq->AddSharedMesh(item->fMeshes[plClothingItem::kLODHigh]);

	sprintf(str, "%s on item %s\n", seq->GetKey()->GetName(), (char *)params[0]);
	PrintString(str);
}

#include "../pfSurface/plFadeOpacityMod.h"

PF_CONSOLE_CMD( Access,
					Fade,
				   "",
				   "Test fading on visibility" )
{
	hsBool disabled = !plFadeOpacityMod::GetLOSCheckDisabled();

	plFadeOpacityMod::SetLOSCheckDisabled(disabled);

	char str[256];
	sprintf(str, "LOS check now %s", disabled ? "disabled" : "enabled");
	PrintString(str);
}

PF_CONSOLE_CMD( Access,
					ObjFade,
				   "string obj, float fadeUp, float fadeDown",
				   "Test fading on visibility" )
{
	char str[256];
	plKey key = FindSceneObjectByName(params[0], nil, str);
	PrintString(str);
	if( !key )
		return;

	plSceneObject* obj = plSceneObject::ConvertNoRef(key->GetObjectPtr());
	if( !obj )
		return;

	float fadeUp = params[1];
	float fadeDown = params[2];

	plFadeOpacityMod* mod = TRACKED_NEW plFadeOpacityMod;
	mod->SetFadeUp(fadeUp);
	mod->SetFadeDown(fadeDown);

	hsgResMgr::ResMgr()->NewKey(obj->GetKey()->GetName(), mod, obj->GetKey()->GetUoid().GetLocation());

	hsgResMgr::ResMgr()->AddViaNotify(mod->GetKey(), TRACKED_NEW plObjRefMsg(obj->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier), plRefFlags::kActiveRef);

}

#include "../plDrawable/plVisLOSMgr.h"

static plSceneObject* losObj = nil;

PF_CONSOLE_CMD( Access,
					Obj,
				   "string losObj",
				   "Set the los test marker" )
{
	char str[256];
	plKey key = FindSceneObjectByName(params[0], nil, str);
	PrintString(str);
	if( !key )
		return;

	losObj = plSceneObject::ConvertNoRef(key->GetObjectPtr());
}

PF_CONSOLE_CMD( Access,
					HackLOS,
				   "string marker",
				   "Set the Los hack marker" )
{
	char str[256];
	plKey key = FindSceneObjectByName(params[0], nil, str);
	PrintString(str);

	plSceneObject* so = nil;
	if( key )
	{
		so = plSceneObject::ConvertNoRef(key->GetObjectPtr());
	}

	extern void VisLOSHackBegin(plPipeline* p, plSceneObject* m);

	VisLOSHackBegin(pfConsole::GetPipeline(), so);
}

PF_CONSOLE_CMD( Access,
					HackEnd,
				   "",
				   "stop the hackage" )
{
	extern void VisLOSHackBegin(plPipeline* p, plSceneObject* m);

	VisLOSHackBegin(nil, nil);
}


PF_CONSOLE_CMD( Access,
					LOS,
					"...",
					"Fire LOS test check" )
{
	static float dist = 1.e5f;
	if( numParams > 0 )
		dist = params[0];

	hsPoint3 from = pfConsole::GetPipeline()->GetViewPositionWorld();

	Int32 sx = pfConsole::GetPipeline()->Width() / 2;
	Int32 sy = pfConsole::GetPipeline()->Height() / 2;
	hsPoint3 targ;
	pfConsole::GetPipeline()->ScreenToWorldPoint(1, 0, &sx, &sy, dist, 0, &targ);

	plVisHit hit;
	if( plVisLOSMgr::Instance()->Check(from, targ, hit) )
	{
		char buff[256];
		sprintf(buff, "(%g, %g, %g)", hit.fPos.fX, hit.fPos.fY, hit.fPos.fZ);
		PrintString(buff);

		if( losObj )
		{
			hsMatrix44 l2w = losObj->GetLocalToWorld();
			l2w.fMap[0][3] = hit.fPos.fX;
			l2w.fMap[1][3] = hit.fPos.fY;
			l2w.fMap[2][3] = hit.fPos.fZ;
			l2w.NotIdentity();
			hsMatrix44 w2l;
			l2w.GetInverse(&w2l);
			losObj->SetTransform(l2w, w2l);
		}
	}

}

#include "../plMessage/plBulletMsg.h"

plSceneObject* gunObj = nil;
hsScalar gunRadius = 1.f;
hsScalar gunRange = 5000.f;

PF_CONSOLE_CMD( Access,
					Gun,
				   "string gun, float radius, ...",
				   "Fire shot along gun's z-axis, creating decal of radius <radius>, with optional max-range (def 1000)" )
{
	char str[256];
	plKey key = FindSceneObjectByName(params[0], nil, str);
	PrintString(str);
	if( !key )
		return;

	plSceneObject* so = plSceneObject::ConvertNoRef(key->GetObjectPtr());
	if( !so )
		return;

	gunObj = so;

	gunRadius = params[1];

	if( numParams > 2 )
		gunRange = params[2];
}

PF_CONSOLE_CMD( Access,
					Shot,
				   "",
				   "Fire shot along gun's z-axis" )
{
	plSceneObject* so = gunObj;
	if( !so )
	{
		PrintString("Set gun object first");
		return;
	}

	hsMatrix44 l2w = so->GetLocalToWorld();
	hsVector3 dir(l2w.fMap[0][2], l2w.fMap[1][2], l2w.fMap[2][2]);
	dir.Normalize();
	hsPoint3 pos = l2w.GetTranslate();

	hsScalar radius = gunRadius;

	hsScalar range = gunRange;

	plBulletMsg* bull = TRACKED_NEW plBulletMsg(nil, nil, nil);
	bull->FireShot(pos, dir, radius, range);

	bull->Send();

	PrintString("Shot fired!");
}

PF_CONSOLE_CMD( Access,
					XShot,
				   "",
				   "Fire shot along gun's neg x-axis" )
{
	plSceneObject* so = gunObj;
	if( !so )
	{
		PrintString("Set gun object first");
		return;
	}

	hsMatrix44 l2w = so->GetLocalToWorld();
	hsVector3 dir(-l2w.fMap[0][0], -l2w.fMap[1][0], -l2w.fMap[2][0]);
	dir.Normalize();
	hsPoint3 pos = l2w.GetTranslate();

	hsScalar radius = gunRadius;

	hsScalar range = gunRange;

	plBulletMsg* bull = TRACKED_NEW plBulletMsg(nil, nil, nil);
	bull->FireShot(pos, dir, radius, range);

	bull->Send();

	PrintString("Shot fired!");
}

PF_CONSOLE_CMD( Access,
			   Party,
			   "string bull, string psys",
			   "Add particle system <psys> to bulletMgr <bull>")
{
	char str[256];
	plKey bullKey = FindObjectByName(params[0], plDynaBulletMgr::Index(), nil, str, false);
	PrintString(str);
	if( !(bullKey && bullKey->GetObjectPtr()) )
	{
		PrintString("bullet not found");
		return;
	}

	plKey sysKey = FindSceneObjectByName(params[1], nil, str);
	if( !(sysKey && sysKey->GetObjectPtr()) )
	{
		PrintString("Psys not found");
		return;
	}
	hsgResMgr::ResMgr()->AddViaNotify(sysKey, TRACKED_NEW plGenRefMsg(bullKey, plRefMsg::kOnCreate, 0, plDynaBulletMgr::kRefPartyObject), plRefFlags::kPassiveRef);

	PrintString("sys added");
}

#endif // LIMIT_CONSOLE_COMMANDS

//////////////////////////////////////////////////////////////////////////////
//// WaveSet Console Commands ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#ifndef LIMIT_CONSOLE_COMMANDS

#include "../plDrawable/plWaveSet7.h"
#include "../plDrawable/plFixedWaterState7.h"
PF_CONSOLE_GROUP( Wave )

PF_CONSOLE_SUBGROUP( Wave, Set)		// Creates a sub-group under a given group

namespace plWaveCmd {
	enum Cmd
	{
		kWindDir,

		kGeoLen,
		kGeoChop,
		kGeoAmp,
		kGeoAngle,

		kTexLen,
		kTexChop,
		kTexAmp,
		kTexAngle,

		kNoise,

		kSpecAtten,

		kWaterTint,
		kWaterOpacity,
		kSpecularTint,
		kSpecularMute,
		kRippleScale,
		kWaterHeight,
		kWaterOffsetOpac,
		kWaterOffsetRefl,
		kWaterOffsetWave,
		kDepthFalloffOpac,
		kDepthFalloffRefl,
		kDepthFalloffWave,
		kMaxAtten,
		kMinAtten,
		kEnvCenter,
		kEnvRadius,
	};
};

typedef void PrintFunk(const char* str);

static inline hsScalar FracToPercent(hsScalar f) { return (hsScalar)(1.e2 * f); }
static inline hsScalar PercentToFrac(hsScalar f) { return (hsScalar)(1.e-2 * f); }

static inline hsScalar RadToDeg(hsScalar r) { return r * 180.f / hsScalarPI; }
static inline hsScalar DegToRad(hsScalar d) { return d * hsScalarPI / 180.f; }

static void IDisplayWaveVal(PrintFunk PrintString, plWaveSet7* wave, plWaveCmd::Cmd cmd)
{
	if( !wave )
		return;

	using namespace plWaveCmd;

	char buff[256];

	hsPoint3 pos;
	hsVector3 vec;
	hsColorRGBA col;

	plFixedWaterState7 state = wave->State();
	switch( cmd )
	{
	case kGeoLen:
		sprintf(buff, "Min/Max Geo Wavelengths = %f/%f", wave->GetGeoMinLength(), wave->GetGeoMaxLength());
		break;
	case kGeoChop:
		sprintf(buff, "Geo Choppiness = %f", FracToPercent(wave->GetGeoChop()));
		break;
	case kGeoAmp:
		sprintf(buff, "Geo Wave Amplitude to Length Ratio (%%) = %f", FracToPercent(wave->GetGeoAmpOverLen()));
		break;
	case kGeoAngle:
		sprintf(buff, "Geo Spread of waves about Wind Dir = %f degrees", RadToDeg(wave->GetGeoAngleDev()));
		break;

	case kTexLen:
		sprintf(buff, "Min/Max Tex Wavelengths = %f/%f", wave->GetTexMinLength(), wave->GetTexMaxLength());
		break;
	case kTexChop:
		sprintf(buff, "Tex Choppiness = %f", FracToPercent(wave->GetTexChop()));
		break;
	case kTexAmp:
		sprintf(buff, "Tex Wave Amplitude to Length Ratio = %f%%", FracToPercent(wave->GetTexAmpOverLen()));
		break;
	case kTexAngle:
		sprintf(buff, "Tex Spread of waves about Wind Dir = %f degrees", RadToDeg(wave->GetTexAngleDev()));
		break;
	
	case kNoise:
		sprintf(buff, "Noising texture ripples at %f %%", FracToPercent(wave->GetSpecularNoise()));
		break;
	
	case kSpecAtten:
		sprintf(buff, "Ripples fade out from %f to %f feet", wave->GetSpecularStart(), wave->GetSpecularEnd());
		break;

	case kWaterOpacity:
		sprintf(buff, "WaterOpacity = %f", FracToPercent(wave->GetWaterTint().a));
		break;
	case kRippleScale:
		sprintf(buff, "RippleScale = %f", wave->GetRippleScale());
		break;
	case kWaterHeight:
		sprintf(buff, "WaterHeight = %f", wave->GetWaterHeight());
		break;
	case kEnvRadius:
		sprintf(buff, "EnvRadius = %f", wave->GetEnvRadius());
		break;
	case kWaterOffsetOpac:
		sprintf(buff, "OpacStart = %f", -wave->GetOpacOffset());
		break;
	case kWaterOffsetRefl:
		sprintf(buff, "ReflStart = %f", -wave->GetReflOffset());
		break;
	case kWaterOffsetWave:
		sprintf(buff, "WaveStart = %f", -wave->GetWaveOffset());
		break;

	case kDepthFalloffOpac:
		sprintf(buff, "OpacEnd = %f", wave->GetOpacFalloff());
		break;
	case kDepthFalloffRefl:
		sprintf(buff, "ReflEnd = %f", wave->GetReflFalloff());
		break;
	case kDepthFalloffWave:
		sprintf(buff, "WaveEnd = %f", wave->GetWaveFalloff());
		break;

	case kWindDir:
		vec = wave->GetWindDir();
		sprintf(buff, "WindDir (%f, %f)", vec.fX, vec.fY);
		break;
	case kMinAtten:
		vec = wave->GetMinAtten();
		sprintf(buff, "MinAtten (%f, %f, %f)", vec.fX, vec.fY, vec.fZ);
		break;
	case kMaxAtten:
		vec = wave->GetMaxAtten();
		sprintf(buff, "MaxAtten (%f, %f, %f)", vec.fX, vec.fY, vec.fZ);
		break;

	case kEnvCenter:
		pos = wave->GetEnvCenter();
		sprintf(buff, "EnvCenter (%f, %f, %f)", pos.fX, pos.fY, pos.fZ);
		break;

	case kWaterTint:
		col = wave->GetWaterTint();
		sprintf(buff, "Water tint (%d, %d, %d)", 
			int(col.r * 255.9f), 
			int(col.g * 255.9f), 
			int(col.b * 255.9f));
		break;
	case kSpecularTint:
		col = wave->GetSpecularTint();
		sprintf(buff, "Specular tint (%d, %d, %d)", 
			int(col.r * 255.9f), 
			int(col.g * 255.9f), 
			int(col.b * 255.9f));
		break;
	case kSpecularMute:
		col = wave->GetSpecularTint();
		sprintf(buff, "Specular mute %f", FracToPercent(col.a));
		break;

	default:
		sprintf(buff, "Unknown parameter");
		break;
	}
	PrintString(buff);
}

static plWaveSet7* IGetWaveSet(PrintFunk PrintString, const char* name)
{
	char str[256];
	plKey waveKey = FindObjectByName(name, plWaveSet7::Index(), nil, str, false);
	PrintString(str);
	if (!waveKey)
		return nil;

	plWaveSet7* waveSet = plWaveSet7::ConvertNoRef(waveKey->ObjectIsLoaded());
	if( !waveSet )
	{
		PrintString("Found object, but it's not a Water component. Ignoring");
	}
	return waveSet;
}

static plWaveSet7* ICheckWaveParams(PrintFunk PrintString, const char* name, int numParams, int n, plWaveCmd::Cmd cmd)
{
	if( !numParams )
	{
		PrintString("Missing name of water component");
		return nil;
	}
	plWaveSet7* waveSet = IGetWaveSet(PrintString, name);
	if( waveSet && (numParams < n) )
	{
		IDisplayWaveVal(PrintString, waveSet, cmd);
		return nil;
	}
	return waveSet;
}

static hsScalar LimitVal(hsScalar val, hsScalar lo, hsScalar hi, PrintFunk PrintString)
{
	if( val < lo )
	{
		char buff[256];
		sprintf(buff, "%f too low, clamped to %f", val, lo);
		PrintString(buff);
		val = lo;
	}
	else if( val > hi )
	{
		char buff[256];
		sprintf(buff, "%f too high, clamped to %f", val, hi);
		PrintString(buff);
		val = hi;
	}
	return val;
}

static bool ISendWaveCmd1f(PrintFunk PrintString, pfConsoleCmdParam* params, int numParams, plWaveCmd::Cmd cmd)
{
	plWaveSet7* wave = ICheckWaveParams(PrintString, params[0], numParams, 2, cmd);
	if( !wave )
		return false;

	using namespace plWaveCmd;

	float val = params[1];

	hsScalar secs = ( numParams > 2 ) ? params[2] : 0.f;

	switch( cmd )
	{
	case kGeoChop:
		wave->SetGeoChop(PercentToFrac(val), secs);
		break;
	case kGeoAmp:
		wave->SetGeoAmpOverLen(PercentToFrac(val), secs);
		break;
	case kGeoAngle:
		wave->SetGeoAngleDev(DegToRad(val), secs);
		break;

	case kTexChop:
		wave->SetTexChop(PercentToFrac(val), secs);
		break;
	case kTexAmp:
		wave->SetTexAmpOverLen(PercentToFrac(val), secs);
		break;
	case kTexAngle:
		wave->SetTexAngleDev(DegToRad(val), secs);
		break;

	case kNoise:
		wave->SetSpecularNoise(PercentToFrac(val), secs);
		break;

	case kWaterOpacity:
		wave->SetWaterOpacity(PercentToFrac(val), secs);
		break;
	case kSpecularMute:
		wave->SetSpecularMute(PercentToFrac(val), secs);
		break;
	case kRippleScale:
		wave->SetRippleScale(val, secs);
		break;
	case kWaterHeight:
		wave->SetWaterHeight(val, secs);
		break;
	case kEnvRadius:
		wave->SetEnvRadius(val, secs);
		break;
	case kWaterOffsetOpac:
		wave->SetOpacOffset(-val, secs);
		break;
	case kWaterOffsetRefl:
		wave->SetReflOffset(-val, secs);
		break;
	case kWaterOffsetWave:
		wave->SetWaveOffset(-val, secs);
		break;

	case kDepthFalloffOpac:
		wave->SetOpacFalloff(val, secs);
		break;
	case kDepthFalloffRefl:
		wave->SetReflFalloff(val, secs);
		break;
	case kDepthFalloffWave:
		wave->SetWaveFalloff(val, secs);
		break;

	default:
		return false;
	}
	return true;
}

static bool ISendWaveCmd2f(PrintFunk PrintString, pfConsoleCmdParam* params, int numParams, plWaveCmd::Cmd cmd)
{
	plWaveSet7* wave = ICheckWaveParams(PrintString, params[0], numParams, 3, cmd);
	if( !wave )
		return false;

	using namespace plWaveCmd;

	hsScalar secs = ( numParams > 3 ) ? params[3] : 0.f;

	hsVector3 vec;
	plFixedWaterState7 state = wave->State();
	switch( cmd )
	{
	case kWindDir:
		vec = wave->GetWindDir();
		vec.fX = params[1];
		vec.fY = params[2];
		wave->SetWindDir(vec, secs);
		break;

	case kGeoLen:
		wave->SetGeoMinLength(params[1], secs);
		wave->SetGeoMaxLength(params[2], secs);
		break;

	case kTexLen:
		wave->SetTexMinLength(params[1], secs);
		wave->SetTexMaxLength(params[2], secs);
		break;

	case kSpecAtten:
		wave->SetSpecularStart(params[1], secs);
		wave->SetSpecularEnd(params[2], secs);
		break;


	default:
		return false;
	}
	return true;
}

static bool ISendWaveCmd3f(PrintFunk PrintString, pfConsoleCmdParam* params, int numParams, plWaveCmd::Cmd cmd)
{
	plWaveSet7* wave = ICheckWaveParams(PrintString, params[0], numParams, 4, cmd);
	if( !wave )
		return false;

	using namespace plWaveCmd;

	float x = params[1];
	float y = params[2];
	float z = params[3];
	hsVector3 vec(x, y, z);
	hsPoint3 pos(x, y, z);

	hsScalar secs = ( numParams > 4 ) ? params[4] : 0.f;

	switch( cmd )
	{
	case kWindDir:
		wave->SetWindDir(vec, secs);
		break;
	case kMinAtten:
		wave->SetMinAtten(vec, secs);
		break;
	case kMaxAtten:
		wave->SetMaxAtten(vec, secs);
		break;

	case kEnvCenter:
		wave->SetEnvCenter(pos, secs);
		break;

	default:
		return false;
	}
	return true;
}

static bool ISendWaveCmd4c(PrintFunk PrintString, pfConsoleCmdParam* params, int numParams, plWaveCmd::Cmd cmd)
{
	plWaveSet7* wave = ICheckWaveParams(PrintString, params[0], numParams, 4, cmd);
	if( !wave )
		return false;

	using namespace plWaveCmd;

	float r = params[1];
	float g = params[2];
	float b = params[3];

	hsScalar secs = ( numParams > 4 ) ? params[4] : 0.f;

	hsColorRGBA col;
	col.Set(r / 255.f, g / 255.f, b / 255.f, 1.f);

	switch( cmd )
	{
	case kWaterTint:
		col.a = wave->GetWaterOpacity();
		wave->SetWaterTint(col, secs);
		break;
	case kSpecularTint:
		col.a = wave->GetSpecularMute();
		wave->SetSpecularTint(col, secs);
		break;

	default:
		return false;
	}
	return true;
}

PF_CONSOLE_CMD( Wave, Log,	// Group name, Function name
				"string waveSet",			// Params none
				"Toggle logging for waves" )	// Help string
{
	const char* name = params[0];
	plWaveSet7* waveSet = IGetWaveSet(PrintString, name);
	if( waveSet )
	{
		hsBool logging = !waveSet->Logging();
		if( logging )
			waveSet->StartLog();
		else
			waveSet->StopLog();

		char buff[256];
		sprintf(buff, "Logging for %s now %s", name, logging ? "on" : "off");
		PrintString(buff);
	}
}

PF_CONSOLE_CMD( Wave, Graph,	// Group name, Function name
				"string waveSet",			// Params none
				"Toggle graphing lens for waves" )	// Help string
{
	const char* name = params[0];
	plWaveSet7* waveSet = IGetWaveSet(PrintString, name);
	if( waveSet )
	{
		hsBool graphing = !waveSet->Graphing();
		if( graphing )
			waveSet->StartGraph();
		else
			waveSet->StopGraph();

		char buff[256];
		sprintf(buff, "Graphing for %s now %s", name, graphing ? "on" : "off");
		PrintString(buff);
	}
}

// Geometric wave param block 
PF_CONSOLE_CMD( Wave_Set, GeoLen,	// Group name, Function name
				"string waveSet, ...",			// Params none
				"Set <min> and <max> geometric wavelengths in feet" )	// Help string
{
	ISendWaveCmd2f(PrintString, params, numParams, plWaveCmd::kGeoLen);
}

PF_CONSOLE_CMD( Wave_Set, GeoAmp,	// Group name, Function name
				"string waveSet, ...",			// Params none
				"Set geometric wave ratio of Amplitude to Wavelengths (as percentage)" )	// Help string
{
	ISendWaveCmd1f(PrintString, params, numParams, plWaveCmd::kGeoAmp);
}

PF_CONSOLE_CMD( Wave_Set, GeoChop,	// Group name, Function name
				"string waveSet, ...",			// Params none
				"Set current geometric wave choppiness" )	// Help string
{
	ISendWaveCmd1f(PrintString, params, numParams, plWaveCmd::kGeoChop);
}

PF_CONSOLE_CMD( Wave_Set, GeoAngle,	// Group name, Function name
				"string waveSet, ...",			// Params none
				"Set geometric wave angular Spread about wind direction (in degrees)" )	// Help string
{
	ISendWaveCmd1f(PrintString, params, numParams, plWaveCmd::kGeoAngle);
}

PF_CONSOLE_CMD( Wave_Set, ReflTint,	// Group name, Function name
				"string waveSet, ...",			// Params none
				"Set current reflection tint (r g b)" )	// Help string
{
	ISendWaveCmd4c(PrintString, params, numParams, plWaveCmd::kSpecularTint);
}

PF_CONSOLE_CMD( Wave_Set, ReflMute,	// Group name, Function name
				"string waveSet, ...",			// Params none
				"Set current reflection muting f (100 % no muting, 0% all gone)" )	// Help string
{
	ISendWaveCmd1f(PrintString, params, numParams, plWaveCmd::kSpecularMute);
}

// Texture wave param block

PF_CONSOLE_CMD( Wave_Set, TexLen,	// Group name, Function name
				"string waveSet, ...",			// Params none
				"Set <min> and <max> texture wavelengths in feet" )	// Help string
{
	ISendWaveCmd2f(PrintString, params, numParams, plWaveCmd::kTexLen);
}

PF_CONSOLE_CMD( Wave_Set, TexAmp,	// Group name, Function name
				"string waveSet, ...",			// Params none
				"Set texture wave ratio of Amplitude to Wavelengths (as percentage)" )	// Help string
{
	ISendWaveCmd1f(PrintString, params, numParams, plWaveCmd::kTexAmp);
}

PF_CONSOLE_CMD( Wave_Set, TexChop,	// Group name, Function name
				"string waveSet, ...",			// Params none
				"Set current texture wave choppiness" )	// Help string
{
	ISendWaveCmd1f(PrintString, params, numParams, plWaveCmd::kTexChop);
}

PF_CONSOLE_CMD( Wave_Set, Noise,	// Group name, Function name
				"string waveSet, ...",			// Params none
				"Set current noising of texture waves (as percentage)" )	// Help string
{
	ISendWaveCmd1f(PrintString, params, numParams, plWaveCmd::kNoise);
}

PF_CONSOLE_CMD( Wave_Set, Scale,	// Group name, Function name
				"string waveSet, ...",			// Params none
				"Set current water ripple scale f" )	// Help string
{
	ISendWaveCmd1f(PrintString, params, numParams, plWaveCmd::kRippleScale);
}

PF_CONSOLE_CMD( Wave_Set, TexAngle,	// Group name, Function name
				"string waveSet, ...",			// Params none
				"Set texture wave spread about wind dir (in degrees)" )	// Help string
{
	ISendWaveCmd1f(PrintString, params, numParams, plWaveCmd::kTexAngle);
}

PF_CONSOLE_CMD( Wave_Set, SpecAtten,	// Group name, Function name
				"string waveSet, ...",			// Params none
				"Set falloff of ripples from <start> to <end> in feet" )	// Help string
{
	ISendWaveCmd2f(PrintString, params, numParams, plWaveCmd::kSpecAtten);
}

// Minor water param block

PF_CONSOLE_CMD( Wave_Set, OpacStart,	// Group name, Function name
				"string waveSet, ...",			// Params none
				"Set current water opacity start f" )	// Help string
{
	ISendWaveCmd1f(PrintString, params, numParams, plWaveCmd::kWaterOffsetOpac);
}

PF_CONSOLE_CMD( Wave_Set, OpacEnd,	// Group name, Function name
				"string waveSet, ...",			// Params none
				"Set current water opacity end f" )	// Help string
{
	ISendWaveCmd1f(PrintString, params, numParams, plWaveCmd::kDepthFalloffOpac);
}

PF_CONSOLE_CMD( Wave_Set, ReflStart,	// Group name, Function name
				"string waveSet, ...",			// Params none
				"Set current water reflection start f" )	// Help string
{
	ISendWaveCmd1f(PrintString, params, numParams, plWaveCmd::kWaterOffsetRefl);
}

PF_CONSOLE_CMD( Wave_Set, ReflEnd,	// Group name, Function name
				"string waveSet, ...",			// Params none
				"Set current water refleciton end f" )	// Help string
{
	ISendWaveCmd1f(PrintString, params, numParams, plWaveCmd::kDepthFalloffRefl);
}

PF_CONSOLE_CMD( Wave_Set, WaveStart,	// Group name, Function name
				"string waveSet, ...",			// Params none
				"Set current water wave start f" )	// Help string
{
	ISendWaveCmd1f(PrintString, params, numParams, plWaveCmd::kWaterOffsetWave);
}

PF_CONSOLE_CMD( Wave_Set, WaveEnd,	// Group name, Function name
				"string waveSet, ...",			// Params none
				"Set current water wave end f" )	// Help string
{
	ISendWaveCmd1f(PrintString, params, numParams, plWaveCmd::kDepthFalloffWave);
}

// Reflection param block

PF_CONSOLE_CMD( Wave_Set, EnvCenter,	// Group name, Function name
				"string waveSet, ...",			// Params none
				"Set current EnvMap Center (x y z)" )	// Help string
{
	ISendWaveCmd3f(PrintString, params, numParams, plWaveCmd::kEnvCenter);
}

PF_CONSOLE_CMD( Wave_Set, Radius,	// Group name, Function name
				"string waveSet, ...",			// Params none
				"Set current envmap radius f" )	// Help string
{
	ISendWaveCmd1f(PrintString, params, numParams, plWaveCmd::kEnvRadius);
}

// Misc. These are normally implicit by associated data (ref object or material).
PF_CONSOLE_CMD( Wave_Set, Direction,	// Group name, Function name
				"string waveSet, ...",			// Params none
				"Set current wind direction (x y)" )	// Help string
{
	ISendWaveCmd2f(PrintString, params, numParams, plWaveCmd::kWindDir);
}


PF_CONSOLE_CMD( Wave_Set, WaterTint,	// Group name, Function name
				"string waveSet, ...",			// Params none
				"Set current water tint (r g b)" )	// Help string
{
	ISendWaveCmd4c(PrintString, params, numParams, plWaveCmd::kWaterTint);
}


PF_CONSOLE_CMD( Wave_Set, Opacity,	// Group name, Function name
				"string waveSet, ...",			// Params none
				"Set current water max opacity f" )	// Help string
{
	ISendWaveCmd1f(PrintString, params, numParams, plWaveCmd::kWaterOpacity);
}

#endif // LIMIT_CONSOLE_COMMANDS



//////////////////////////////////////////////////////////////////////////////
//// Object Console Commands ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#ifndef LIMIT_CONSOLE_COMMANDS


PF_CONSOLE_GROUP( SceneObject )

PF_CONSOLE_SUBGROUP( SceneObject, SetEnable)		// Creates a sub-group under a given group


PF_CONSOLE_CMD( SceneObject_SetEnable, Drawable,	// Group name, Function name
				"string objName, bool on",			// Params none
				"Enable or disable drawing of a sceneobject" )	// Help string
{
	char str[256];
	plKey key = FindSceneObjectByName(params[0], nil, str);
	PrintString(str);
	if (!key)
		return;

	bool enable = params[1];
	
	plEnableMsg* pEMsg = TRACKED_NEW plEnableMsg;
	pEMsg->SetCmd( enable ? plEnableMsg::kEnable : plEnableMsg::kDisable );
	pEMsg->SetCmd( plEnableMsg::kDrawable );
	pEMsg->AddReceiver( key );
	plgDispatch::MsgSend( pEMsg );
}

PF_CONSOLE_CMD( SceneObject_SetEnable, Physical,	// Group name, Function name
				"string objName, bool on",			// Params none
				"Enable or disable the physical of a sceneobject" )	// Help string
{
	char str[256];
	plKey key = FindSceneObjectByName(params[0], nil, str);
	PrintString(str);
	if (!key)
		return;

	bool enable = params[1];
	
	plEnableMsg* pEMsg = TRACKED_NEW plEnableMsg;
	pEMsg->SetCmd( enable ? plEnableMsg::kEnable : plEnableMsg::kDisable );
	pEMsg->SetCmd( plEnableMsg::kPhysical );
	pEMsg->AddReceiver( key );
	plgDispatch::MsgSend( pEMsg );
}
/*
PF_CONSOLE_CMD( SceneObject_SetEnable, PhysicalT,	// Group name, Function name
				"string objName, bool on",			// Params none
				"Enable or disable the physical of a sceneobject" )	// Help string
{
	char str[256];
	plKey key = FindSceneObjectByName(params[0], nil, str);
	PrintString(str);
	if (!key)
		return;

	bool enable = params[1];
	
	plEventGroupEnableMsg* pMsg = TRACKED_NEW plEventGroupEnableMsg;
	if( enable )
		pMsg->SetFlags(plEventGroupEnableMsg::kCollideOn | plEventGroupEnableMsg::kReportOn);
	else
		pMsg->SetFlags(plEventGroupEnableMsg::kCollideOff | plEventGroupEnableMsg::kReportOff);

	pMsg->AddReceiver(key);
	pMsg->Send();

}
*/
PF_CONSOLE_CMD( SceneObject_SetEnable, Audible,		// Group name, Function name
				"string objName, bool on",			// Params none
				"Enable or disable the audible of a sceneobject" )	// Help string
{
	char str[256];
	plKey key = FindSceneObjectByName(params[0], nil, str);
	PrintString(str);
	if (!key)
		return;

	bool enable = params[1];
	
	plEnableMsg* pEMsg = TRACKED_NEW plEnableMsg;
	pEMsg->SetCmd( enable ? plEnableMsg::kEnable : plEnableMsg::kDisable );
	pEMsg->SetCmd( plEnableMsg::kAudible );
	pEMsg->AddReceiver( key );
	plgDispatch::MsgSend( pEMsg );
}

PF_CONSOLE_CMD( SceneObject_SetEnable, All,			// Group name, Function name
				"string objName, bool on",			// Params none
				"Enable or disable all fxns of a sceneobject" )	// Help string
{
	char str[256];
	plKey key = FindSceneObjectByName(params[0], nil, str);
	PrintString(str);
	if (!key)
		return;

	bool enable = params[1];
	
	plEnableMsg* pEMsg = TRACKED_NEW plEnableMsg;
	pEMsg->SetCmd( enable ? plEnableMsg::kEnable : plEnableMsg::kDisable );
	pEMsg->SetCmd( plEnableMsg::kAll );
	pEMsg->AddReceiver( key );
	plgDispatch::MsgSend( pEMsg );
}

PF_CONSOLE_CMD( SceneObject, Attach,			// Group name, Function name
				"string childName, string parentName",			// Params none
				"Attach child to parent" )	// Help string
{
	char str[256];

	const char* childName = params[0];
	const char* parentName = params[1];

	plKey childKey = FindSceneObjectByName(childName, nil, str);
	if( !childKey )
	{
		PrintString(str);
		return;
	}
	plSceneObject* child = plSceneObject::ConvertNoRef(childKey->GetObjectPtr());
	if( !child )
	{
		sprintf( str, "Child SceneObject not found");
		PrintString(str);
		return;
	}

	plKey parentKey = FindSceneObjectByName(parentName, nil, str);
	if( !parentKey )
	{
		PrintString(str);
		return;
	}

	plAttachMsg* attMsg = TRACKED_NEW plAttachMsg(parentKey, child, plRefMsg::kOnRequest, nil);
	plgDispatch::MsgSend(attMsg);

	sprintf(str, "%s now child of %s", childName, parentName);
	PrintString(str);

}

PF_CONSOLE_CMD( SceneObject, Detach,			// Group name, Function name
				"string childName",			// Params none
				"Detach a child from parent (if any)" )	// Help string
{
	char str[256];

	const char* childName = params[0];

	plKey childKey = FindSceneObjectByName(childName, nil, str);
	if( !childKey )
	{
		PrintString(str);
		return;
	}
	plSceneObject* child = plSceneObject::ConvertNoRef(childKey->GetObjectPtr());
	if( !child )
	{
		sprintf( str, "Child SceneObject not found");
		PrintString(str);
		return;
	}

	if( child 
		&& child->GetCoordinateInterface() 
		&& child->GetCoordinateInterface()->GetParent() 
		&& child->GetCoordinateInterface()->GetParent()->GetOwner() )
	{
		plKey parentKey = child->GetCoordinateInterface()->GetParent()->GetOwner()->GetKey();
		plAttachMsg* attMsg = TRACKED_NEW plAttachMsg(parentKey, child, plRefMsg::kOnRemove, nil);
		plgDispatch::MsgSend(attMsg);

		sprintf(str, "%s detached from %s", childName, parentKey->GetName());
		PrintString(str);
		return;
	}
	else
	{
		sprintf(str, "%s not attached to anything", childName);
		PrintString(str);
		return;
	}
}

#endif // LIMIT_CONSOLE_COMMANDS

//////////////////////////////////////////////////////////////
// PHYSICS (The Havok Flavour)
//////////////////////////////////////////////////////////////

#ifndef LIMIT_CONSOLE_COMMANDS

#include "../plPhysX/plPXPhysicalControllerCore.h"

PF_CONSOLE_GROUP( Physics )

PF_CONSOLE_CMD( Physics, Rebuild, "", "Rebuilds the avatars collision cache")
{
	plPXPhysicalControllerCore::RebuildCache();
}

PF_CONSOLE_CMD(Physics, MaxPhysicalAvatars, "int max", "Set the maximum number of avatar physicals allowed. Default = 0 (meaning no limit)")
{
	int max = params[0];
	plPXPhysicalControllerCore::SetMaxNumberOfControllers(max);
}

/*
PF_CONSOLE_CMD( Physics, SetStepsPerSecond, "int steps", "Sets the number of physics substeps per second, regardless of rendering framerate.")
{
	int newSteps = params[0];
	plSimulationMgr::GetInstance()->SetStepsPerSecond(newSteps);
}

PF_CONSOLE_CMD( Physics, GetStepsPerSecond, "", "Prints the number of physics substeps per second.")
{
	int steps = plSimulationMgr::GetInstance()->GetStepsPerSecond();

	char buffy[256];
	sprintf(buffy, "Current physics resolution is %d frames per second.", steps);
	PrintString(buffy);
}

PF_CONSOLE_CMD(Physics, SetMaxDelta, "float maxDelta", "Sets the largest frame-to-frame delta that physics will try to resolve before giving up and freezing.")
{
	float newMaxDelta = params[0];
	plSimulationMgr::GetInstance()->SetMaxDelta(newMaxDelta);
}

PF_CONSOLE_CMD(Physics, GetMaxDelta, "", "Prints the largest frame-to-frame delta that physics will try to resolve before giving up and freezing.")
{
	float oldMaxDelta = plSimulationMgr::GetInstance()->GetMaxDelta();

	char buffy[256];
	sprintf(buffy, "When (delta > %f), physics is suspended for that frame.", oldMaxDelta);
	PrintString(buffy);
}

PF_CONSOLE_CMD(Physics, SetDeactivateFreq, "float freq", "")
{
	float freq = params[0];
	plSimulationMgr::GetInstance()->SetDeactivateFreq(freq);
}

PF_CONSOLE_CMD(Physics, SetCollisionTolerance, "float tol", "Minimum distance objects must be from each other to collide.  Set from an ini file.")
{
	float tol = params[0];
	plHKPhysicsContext::SetCollisionTolerance(tol);
}

PF_CONSOLE_CMD( Physics, Suspend, "", "Toggle suspend/resume physics.")
{
	if(plSimulationMgr::GetInstance()->IsSuspended())
		plSimulationMgr::GetInstance()->Resume();
	else
		plSimulationMgr::GetInstance()->Suspend();
}

PF_CONSOLE_CMD( Physics, ShowExternal, "", "Display a snapshot of the world as Havok sees it. Requires separate debug app." )
{
	plSimulationMgr::GetInstance()->ExternalDebugDisplay();
}

//PF_CONSOLE_CMD( Physics, SetGravity, "float fpsps", "Set gravity in feet per second per second.")
//{
//	plSimulationMgr::GetInstance()->SetGravity(0,0,params[0]);
//}

PF_CONSOLE_CMD( Physics, ApplyForce, "string Object, float x, float y, float z", "Apply a force to a scene object at its center of mass.")
{
	plKey key = FindSceneObjectByName(params[0], nil, nil);

	if(key) 
	{
		hsVector3 force(params[1], params[2], params[3]);
		plForceMsg *m = TRACKED_NEW plForceMsg(nil, key, force);
		plgDispatch::MsgSend(m);
	}
}

PF_CONSOLE_CMD( Physics, ApplyForceAtPoint, "string Object, float forceX, float forceY, float forceZ, float pointX, float pointY, float pointZ", "Apply a force to a scene object at a particular point in world space.")
{
	plKey key = FindSceneObjectByName(params[0], nil, nil, nil);

	if(key) 
	{
		hsVector3 force(params[1], params[2], params[3]);
		hsPoint3 point(params[4], params[5], params[6]);
		plOffsetForceMsg *m = TRACKED_NEW plOffsetForceMsg(nil, key, force, point);
		plgDispatch::MsgSend(m);
	}
}

PF_CONSOLE_CMD( Physics, ApplyTorque, "string Object, float axisX, float axisY, float axisZ", "Apply a torque to a scene object about given axis. Magnitude is size of force.")
{
	plKey key = FindSceneObjectByName(params[0], nil, nil);

	if(key)
	{
		hsVector3 torque(params[1], params[2], params[3]);
		plTorqueMsg *m = TRACKED_NEW plTorqueMsg(nil, key, torque);
		plgDispatch::MsgSend(m);
	}
}

PF_CONSOLE_CMD( Physics, ApplyImpulse, "string Object, float x, float y, float z", "Apply an impulse to a scene object.")
{
	plKey key = FindSceneObjectByName(params[0], nil, nil);

	if(key)
	{
		hsVector3 impulse(params[1], params[2], params[3]);
		plImpulseMsg *m = TRACKED_NEW plImpulseMsg(nil, key, impulse);
		plgDispatch::MsgSend(m);
	}
}

PF_CONSOLE_CMD( Physics, ApplyImpulseAtPoint, "string Object, float impulseX, float impulseY, float impulseZ, float pointX, float pointY, float pointZ", "Apply an impulse to a scene object at a particular point in world space.")
{
	plKey key = FindSceneObjectByName(params[0], nil, nil);

	if(key)
	{
		hsVector3 impulse(params[1], params[2], params[3]);
		hsPoint3 point(params[4], params[5], params[6]);
		plOffsetImpulseMsg *m = TRACKED_NEW plOffsetImpulseMsg(nil, key, impulse, point);
		plgDispatch::MsgSend(m);
	}
}

PF_CONSOLE_CMD( Physics, ApplyAngularImpulse, "string Object, float x, float y, float z", "Apply a rotational impulse about the given axis a scene object. Magnitude is strength.")
{
	plKey key = FindSceneObjectByName(params[0], nil, nil);

	if(key)
	{
		hsVector3 impulse(params[1], params[2], params[3]);
		plAngularImpulseMsg *m = TRACKED_NEW plAngularImpulseMsg(nil, key, impulse);
		plgDispatch::MsgSend(m);
	}
}

PF_CONSOLE_CMD( Physics, ApplyDamping, "string Object, float dampFactor", "Reduce the velocity and spin of the object to the given percentage.")
{
	plKey key = FindSceneObjectByName(params[0], nil, nil);

	if(key)
	{
		float dampFactor = params[1];
		plDampMsg *m = TRACKED_NEW plDampMsg(nil, key, dampFactor);
		plgDispatch::MsgSend(m);
	}
}

PF_CONSOLE_CMD( Physics, ShiftMass, "string Object, float x, float y, float z", "Shift the object's center of mass.")
{
	plKey key = FindSceneObjectByName(params[0], nil, nil);

	if(key)
	{
		hsVector3 offset(params[1], params[2], params[3]);
		plShiftMassMsg *m = TRACKED_NEW plShiftMassMsg(nil, key, offset);
		plgDispatch::MsgSend(m);
	}
}

PF_CONSOLE_CMD( Physics, Suppress, "string Object, int doSuppress", "Remove(true) or re-add the named physical from/to the simulation.")
{
	plKey key = FindSceneObjectByName(params[0], nil, nil);
	if(key)
	{
		int iDoSuppress = params[1];
		
		bool doSuppress = iDoSuppress ? true : false;
		plSimSuppressMsg *msg = TRACKED_NEW plSimSuppressMsg(nil, key, doSuppress);
		msg->Send();
	}
}

PF_CONSOLE_CMD( Physics, SetEventGroup, "string Object, int group, int status, int clearOthers", "Add to or remove from physics event group.")
{
	plKey key = FindSceneObjectByName(params[0], nil, nil);

	if(key)
	{
		int group = params[1], status = params[2], clearOthers = params[3];
		plEventGroupMsg *m = TRACKED_NEW plEventGroupMsg(nil, key, group, status, clearOthers);
		plgDispatch::MsgSend(m);
	}
}

PF_CONSOLE_CMD( Physics, Freeze, "string Object, int status", "Immobilize the given simulated object.")
{
	plKey key = FindSceneObjectByName(params[0], nil, nil);

	if(key)
	{
		int status = params[1];

		plFreezeMsg *m = TRACKED_NEW plFreezeMsg(nil, key, nil, status);

		plgDispatch::MsgSend(m);
	}
}

#include "../plHavok1/plHKCollision.h"

PF_CONSOLE_CMD( Physics, ToggleShowImpacts, "", "Shows the names of impacting physicals on screen.")
{
	plHKCollision::ToggleDisplayImpacts();
}

PF_CONSOLE_CMD( Physics, DumpRejectedBroadphase, "", "")
{
	plSimulationMgr::GetInstance()->DumpRejectedBroadPhase(true);
}

extern int gPhysicsAnimatedOptimize;
PF_CONSOLE_CMD( Physics, OptimizeAnimatedPhysicals, "int enable", "if true then dont eval non moving physical animations")
{
	gPhysicsAnimatedOptimize = params[0];
}

PF_CONSOLE_CMD( Physics, ClearLog, "", "Clear the physics log.")
{
	plSimulationMgr::ClearLog();
}
*/
#include "../plPhysical/plPhysicalSDLModifier.h"

PF_CONSOLE_CMD(Physics, LogSDL, "int level", "Turn logging of physics SDL state on or off. 0=off 1=send/receive only 2=any attempt")
{
	int level = params[0];
	plPhysicalSDLModifier::SetLogLevel(level);
}

#include "../plPhysX/plSimulationMgr.h"
PF_CONSOLE_CMD(Physics, ExtraProfile, "", "Toggle extra simulation profiling")
{
	char str[256];
	if (plSimulationMgr::fExtraProfile)
	{
		plSimulationMgr::fExtraProfile = false;
		sprintf(str, "Stop extra profiling");
	}
	else
	{
		plSimulationMgr::fExtraProfile = true;
		sprintf(str, "Start extra profiling");
	}
	PrintString( str );
}
PF_CONSOLE_CMD(Physics, SubworldOptimization, "", "Toggle subworld optimization")
{
	char str[256];
	if (plSimulationMgr::fSubworldOptimization)
	{
		plSimulationMgr::fSubworldOptimization = false;
		sprintf(str, "Stop subworld optimization");
	}
	else
	{
		plSimulationMgr::fSubworldOptimization = true;
		sprintf(str, "Start subworld optimization");
	}
	PrintString( str );
}
PF_CONSOLE_CMD(Physics, ClampingOnStep, "", "Toggle whether to clamp the step size on advance")
{
	char str[256];
	if (plSimulationMgr::fDoClampingOnStep)
	{
		plSimulationMgr::fDoClampingOnStep = false;
		sprintf(str, "Stop clamping the step size");
	}
	else
	{
		plSimulationMgr::fDoClampingOnStep = true;
		sprintf(str, "Start clamping the step size");
	}
	PrintString( str );
}

PF_CONSOLE_CMD(Physics, 
			   ShowControllerDebugDisplay,
			   "", 
			   "Toggle the physics controller debug display")
{
	plPXPhysicalControllerCore::fDebugDisplay = !plPXPhysicalControllerCore::fDebugDisplay;
}
PF_CONSOLE_CMD(Physics, 
			   ListAwakeActors,
			   "", 
			   "Toggles displaying the list of awake actors")
{
	plSimulationMgr::fDisplayAwakeActors= !plSimulationMgr::fDisplayAwakeActors;
}

/*
PF_CONSOLE_CMD( Physics, PlayPhysicsSounds, "bool b", "Turn physics sounds on/off.")
{
	bool b = params[0];
	plHKCollision::TogglePhysicsSounds(b);
}
*/
#endif // LIMIT_CONSOLE_COMMANDS


//////////////////////////////////////////////////////////////
// Mouse controls
//////////////////////////////////////////////////////////////


PF_CONSOLE_GROUP( Mouse )

PF_CONSOLE_CMD( Mouse, Invert, nil, "invert the mouse")
{
	plMouseDevice::SetInverted(true);
}

PF_CONSOLE_CMD( Mouse, UnInvert, nil, "un-invert the mouse")
{
	plMouseDevice::SetInverted(false);
}

#ifndef LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_CMD( Mouse, SetDeadZone, "float zone", "Sets the dead zone for the mouse - range is from 0.0 to 1.0")
{
	hsScalar f = params[0];
}

PF_CONSOLE_CMD( Mouse, Enable, nil, "Enable mouse input")
{
//	plCommandInterfaceModifier::GetInstance()->EnableMouseInput();
}
PF_CONSOLE_CMD( Mouse, Disable, nil, "Disable mouse input")
{
//	plCommandInterfaceModifier::GetInstance()->DisableMouseInput();
}

PF_CONSOLE_CMD( Mouse, SetFadeDelay, "float delayInSecs", "Set how long the cursor has to not move before it fades out" )
{
	if( plAvatarInputInterface::GetInstance() != nil )
		plAvatarInputInterface::GetInstance()->SetCursorFadeDelay( params[ 0 ] );
}

PF_CONSOLE_CMD( Mouse, Hide, nil, "hide mouse cursor")
{
	plMouseDevice::HideCursor(true);
}

PF_CONSOLE_CMD( Mouse, Show, nil, "hide mouse cursor")
{
	plMouseDevice::ShowCursor(true);
}

/*PF_CONSOLE_CMD( Mouse, SetScale, "float scale", "Sets the mouse scaling factor (sensitivity)" )
{
	plInputManager::GetInstance()->SetMouseScale( params[ 0 ] );
	PrintStringF( PrintString, "Mouse scale factor set to %4.2f", params[ 0 ] );
}
*/

PF_CONSOLE_CMD( Mouse, ForceHide, "bool force", "Forces the mouse to be hidden (or doesn't)" )
{
	plInputInterfaceMgr::GetInstance()->ForceCursorHidden( (bool)params[ 0 ] );
	PrintStringF( PrintString, "Mouse cursor %s", params[ 0 ] ? "forced to be hidden" : "back to normal" );
}

#endif // LIMIT_CONSOLE_COMMANDS

//////////////////////////////////////////////////////////////////////////////
//// Age Group Commands //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#ifndef LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_GROUP( Age )

PF_CONSOLE_CMD(Age, ShowSDL, "", "Prints the age SDL values")
{
	plStateDataRecord * rec = NEWZERO(plStateDataRecord);
	if (!VaultAgeGetAgeSDL(rec)) {
		PrintString("Age SDL not found");
		DEL(rec);
		return;
	}
	
	char line[2048];

	plStatusLog::AddLineS("ShowSDL.log", "-----------------------------------");
	for (unsigned i = 0; i < rec->GetNumVars(); ++i) {
		plStateVariable * var = rec->GetVar(i);
		if (plSimpleStateVariable * simple = var->GetAsSimpleStateVar()) {
			const char * name = var->GetName();
			StrPrintf(line, arrsize(line), "%s=", name);
			for (unsigned j = 0; j < simple->GetCount(); ++j) {
				char * str = simple->GetAsString(j);
				StrPack(line, str, arrsize(line));
				StrPack(line, ",", arrsize(line));
				FREE(str);
			}
			PrintString(line);
			plStatusLog::AddLineS("ShowSDL.log", "%s", line);
		}
	}	
	
	DEL(rec);
}

PF_CONSOLE_CMD( Age, GetElapsedDays, "string agedefnfile", "Gets the elapsed days and fractions" )
{
	hsUNIXStream s;
	if (!s.Open(params[0]))
	{
		PrintString("Couldn't open age defn file!");
		return;
	}

	plAgeDescription age;
	age.Read(&s);
	
	char str[256];
	plUnifiedTime current;
	current.SetToUTC();
	sprintf(str,"ElapsedTime: %f Days",age.GetAgeElapsedDays(current));

	PrintString(str);
	s.Close();
}

PF_CONSOLE_CMD( Age, GetTimeOfDay, "string agedefnfile", "Gets the elapsed days and fractions" )
{
	hsUNIXStream s;
	if (!s.Open(params[0]))
	{
		PrintString("Couldn't open age defn file!");
		return;
	}

	plAgeDescription age;
	age.Read(&s);
	
	char str[256];
	plUnifiedTime current;
	current.SetToUTC();
	sprintf(str,"TimeOfDay: %f percent",age.GetAgeTimeOfDayPercent(current));

	PrintString(str);
	s.Close();
}

PF_CONSOLE_CMD( Age, SetSDLFloat, "string varName, float value, int index", "Set the value of an age global variable" )
{
	int index = (int)params[2];

	extern const plPythonSDLModifier *ExternFindAgePySDL();
	const plPythonSDLModifier *sdlMod = ExternFindAgePySDL();
	if (!sdlMod)
		return;

	plSimpleStateVariable *var = sdlMod->GetStateCache()->FindVar(params[0]);
	if (!var)
		return;

	float v;
	var->Get(&v, index);
	var->Set((float)params[1], index);
	// set the variable in the pythonSDL also
	((plPythonSDLModifier*)sdlMod)->SetItemFromSDLVar(var);
	// set it back to original so that its different
	plSynchedObject* p = plSynchedObject::ConvertNoRef(((plSDLModifier*)sdlMod)->GetStateOwnerKey()->GetObjectPtr());
	if (p)
		p->DirtySynchState(sdlMod->GetSDLName(),plSynchedObject::kSendImmediately|plSynchedObject::kSkipLocalOwnershipCheck|plSynchedObject::kForceFullSend);
}

PF_CONSOLE_CMD( Age, SetSDLInt, "string varName, int value, int index", "Set the value of an age global variable" )
{
	int index = (int)params[2];

	extern const plPythonSDLModifier *ExternFindAgePySDL();
	const plPythonSDLModifier *sdlMod = ExternFindAgePySDL();
	if (!sdlMod)
		return;

	plSimpleStateVariable *var = sdlMod->GetStateCache()->FindVar(params[0]);
	if (!var)
		return;

	int	v;
	var->Get(&v, index);
	var->Set((int)params[1], index);
	// set the variable in the pythonSDL also
	((plPythonSDLModifier*)sdlMod)->SetItemFromSDLVar(var);
	plSynchedObject* p = plSynchedObject::ConvertNoRef(((plSDLModifier*)sdlMod)->GetStateOwnerKey()->GetObjectPtr());
	if (p)
		p->DirtySynchState(sdlMod->GetSDLName(),plSynchedObject::kSendImmediately|plSynchedObject::kSkipLocalOwnershipCheck|plSynchedObject::kForceFullSend);
}

PF_CONSOLE_CMD( Age, SetSDLBool, "string varName, bool value, int index", "Set the value of an age global variable" )
{
	int index = (int)params[2];
	
	extern const plPythonSDLModifier *ExternFindAgePySDL();
	const plPythonSDLModifier *sdlMod = ExternFindAgePySDL();
	if (!sdlMod)
		return;

	plSimpleStateVariable *var = sdlMod->GetStateCache()->FindVar(params[0]);
	if (!var)
		return;

	bool v;
	var->Get(&v, index);
	var->Set((bool)params[1], index);
	// set the variable in the pythonSDL also
	((plPythonSDLModifier*)sdlMod)->SetItemFromSDLVar(var);
	plSynchedObject* p = plSynchedObject::ConvertNoRef(((plSDLModifier*)sdlMod)->GetStateOwnerKey()->GetObjectPtr());
	if (p)
		p->DirtySynchState(sdlMod->GetSDLName(),plSynchedObject::kSendImmediately|plSynchedObject::kSkipLocalOwnershipCheck|plSynchedObject::kForceFullSend);
}

#endif // LIMIT_CONSOLE_COMMANDS

//////////////////////////////////////////////////////////////////////////////
//// Particle System Group Commands /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#ifndef LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_GROUP( ParticleSystem ) // Defines a main command group

void UpdateParticleParam(char *objName, Int32 paramID, hsScalar value, void (*PrintString)(const char *))
{
	char str[256];
	plKey key = FindSceneObjectByName(objName, nil, str);
	PrintString(str);
	if (key == nil) return;

	plSceneObject* so = plSceneObject::ConvertNoRef(key->GetObjectPtr());
	if (so == nil) return;

	int i;
	for (i = so->GetNumModifiers() - 1; i >= 0; i--)
	{
		const plParticleSystem *sys = plParticleSystem::ConvertNoRef(so->GetModifier(i));
		if (sys != nil)
		{
			plgDispatch::MsgSend(TRACKED_NEW plParticleUpdateMsg(nil, sys->GetKey(), nil, paramID, value));
			PrintString("Particle system successfully updated.");
			return;
		}
	}
	PrintString("The scene object specified has no particle system.");
}

PF_CONSOLE_CMD( ParticleSystem,					// Group name
				SetPPS,							// Function name
				"string objName, float value",	// Params
				"Set the particles-per-second generated" )	// Help string
{
	UpdateParticleParam(params[0], plParticleUpdateMsg::kParamParticlesPerSecond, params[1], PrintString);
}

PF_CONSOLE_CMD( ParticleSystem,					// Group name
				SetInitialPitchRange,			// Function name
				"string objName, float value",	// Params
				"Set the initial range of pitch of generated particles" )	// Help string
{
	UpdateParticleParam(params[0], plParticleUpdateMsg::kParamInitPitchRange, params[1], PrintString);
}

PF_CONSOLE_CMD( ParticleSystem,					// Group name
				SetInitialYawRange,				// Function name
				"string objName, float value",	// Params
				"Set the initial range of yaw of generated particles" )	// Help string
{
	UpdateParticleParam(params[0], plParticleUpdateMsg::kParamInitYawRange, params[1], PrintString);
}

PF_CONSOLE_CMD( ParticleSystem,					// Group name
				SetInitialVelocityMin,			// Function name
				"string objName, float value",	// Params
				"Set the minimum initial velocity of generated particles" )	// Help string
{
	UpdateParticleParam(params[0], plParticleUpdateMsg::kParamVelMin, params[1], PrintString);
}

PF_CONSOLE_CMD( ParticleSystem,					// Group name
				SetInitialVelocityMax,		// Function name
				"string objName, float value",	// Params
				"Set the maximum initial velocity of generated particles" )	// Help string
{
	UpdateParticleParam(params[0], plParticleUpdateMsg::kParamVelMax, params[1], PrintString);
}

PF_CONSOLE_CMD( ParticleSystem,					// Group name
				SetWidth,						// Function name
				"string objName, float value",	// Params
				"Set the width of generated particles" )	// Help string
{
	UpdateParticleParam(params[0], plParticleUpdateMsg::kParamXSize, params[1], PrintString);
}

PF_CONSOLE_CMD( ParticleSystem,					// Group name
				SetHeight,						// Function name
				"string objName, float value",	// Params
				"Set the height of generated particles" )	// Help string
{
	UpdateParticleParam(params[0], plParticleUpdateMsg::kParamYSize, params[1], PrintString);
}

PF_CONSOLE_CMD( ParticleSystem,					// Group name
				SetScaleMin,					// Function name
				"string objName, float value",	// Params
				"Set the minimum width/height scaling of generated particles" )	// Help string
{
	UpdateParticleParam(params[0], plParticleUpdateMsg::kParamScaleMin, params[1], PrintString);
}

PF_CONSOLE_CMD( ParticleSystem,					// Group name
				SetScaleMax,					// Function name
				"string objName, float value",	// Params
				"Set the maximum width/height scaling of generated particles" )	// Help string
{
	UpdateParticleParam(params[0], plParticleUpdateMsg::kParamScaleMax, params[1], PrintString);
}

PF_CONSOLE_CMD( ParticleSystem,					// Group name
				SetGeneratorLife,				// Function name
				"string objName, float value",	// Params
				"Set the remaining life of the particle generator" )	// Help string
{
	UpdateParticleParam(params[0], plParticleUpdateMsg::kParamGenLife, params[1], PrintString);
}

PF_CONSOLE_CMD( ParticleSystem,					// Group name
				SetParticleLifeMin,				// Function name
				"string objName, float value",	// Params
				"Set the minimum lifespan of generated particles (negative values make them immortal)" )	// Help string
{
	UpdateParticleParam(params[0], plParticleUpdateMsg::kParamPartLifeMin, params[1], PrintString);
}

PF_CONSOLE_CMD( ParticleSystem,					// Group name
				SetParticleLifeMax,				// Function name
				"string objName, float value",	// Params
				"Set the max lifespan of generated particles" )	// Help string
{
	UpdateParticleParam(params[0], plParticleUpdateMsg::kParamPartLifeMax, params[1], PrintString);
}

PF_CONSOLE_CMD( ParticleSystem,
			   TransferParticlesToAvatar,
			   "string objName, int numParticles",
			   "Creates a system (if necessary) on the avatar, and transfers particles" )
{
	char str[256];
	plKey key = FindSceneObjectByName(params[0], nil, str);
	if (key == nil) 
		return;
	
	plSceneObject* so = plSceneObject::ConvertNoRef(key->GetObjectPtr());
	if (so == nil) 
		return;
	
	plArmatureMod *avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();	
	if (avMod)
		(TRACKED_NEW plParticleTransferMsg(nil, avMod->GetKey(), 0, so->GetKey(), (int)params[1]))->Send();
}

PF_CONSOLE_CMD( ParticleSystem,
			   KillParticles,
			   "string objName, float timeLeft, float num, bool useAsPercentage",
			   "Flag some particles for death." )
{
	char str[256];
	plKey key = FindSceneObjectByName(params[0], nil, str);
	if (key == nil) 
		return;
	
	plSceneObject* so = plSceneObject::ConvertNoRef(key->GetObjectPtr());
	if (so == nil) 
		return;
	
	const plParticleSystem *sys = plParticleSystem::ConvertNoRef(so->GetModifierByType(plParticleSystem::Index()));
	if (sys != nil)
	{
		UInt8 flags = (params[3] ? plParticleKillMsg::kParticleKillPercentage : 0);
		(TRACKED_NEW plParticleKillMsg(nil, sys->GetKey(), 0, params[2], params[1], flags))->Send();
	}
}


PF_CONSOLE_SUBGROUP( ParticleSystem, Flock )

static plParticleFlockEffect *FindFlock(char *objName)
{
	char str[256];
	plKey key = FindSceneObjectByName(objName, nil, str);
	
	if (key == nil)
		return nil;
	
	plSceneObject *so = plSceneObject::ConvertNoRef(key->GetObjectPtr());
	if (so == nil) 
		return nil;
	
	const plParticleSystem *sys = plParticleSystem::ConvertNoRef(so->GetModifierByType(plParticleSystem::Index()));
	if (sys == nil)
		return nil;
	
	plParticleFlockEffect *flock = plParticleFlockEffect::ConvertNoRef(sys->GetEffect(plParticleFlockEffect::Index()));
	if (flock == nil)
		return nil;
	
	return flock;
}

PF_CONSOLE_CMD( ParticleSystem_Flock,
			   SetFlockOffset,
			   "string objName, float x, float y, float z",
			   "Set the flock's goal to be an offset from its sceneObject")
{
	plParticleEffect *flock = FindFlock(params[0]);
	if (flock)
	{
		(TRACKED_NEW plParticleFlockMsg(nil, flock->GetKey(), 0, plParticleFlockMsg::kFlockCmdSetOffset, params[1], params[2], params[3]))->Send();
	}
}	

PF_CONSOLE_CMD( ParticleSystem_Flock,
			   SetDissentTarget,
			   "string objName, float x, float y, float z",
			   "Set the goal for particles that leave the flock")
{
	plParticleEffect *flock = FindFlock(params[0]);
	if (flock)
	{
		(TRACKED_NEW plParticleFlockMsg(nil, flock->GetKey(), 0, plParticleFlockMsg::kFlockCmdSetDissentPoint, params[1], params[2], params[3]))->Send();
	}
}

PF_CONSOLE_CMD( ParticleSystem_Flock,
			   SetConformDistance,
			   "string objName, float value",
			   "")
{
	plParticleFlockEffect *flock = FindFlock(params[0]);
	if (flock)
		flock->SetInfluenceAvgRadius(params[1]);
	else
		PrintString("Can't find flock effect");
}	

PF_CONSOLE_CMD( ParticleSystem_Flock,
			   SetRepelDistance,
			   "string objName, float value",
			   "")
{
	plParticleFlockEffect *flock = FindFlock(params[0]);
	if (flock)
		flock->SetInfluenceRepelRadius(params[1]);
	else
		PrintString("Can't find flock effect");
}	

PF_CONSOLE_CMD( ParticleSystem_Flock,
			   SetGoalDistance,
			   "string objName, float value",
			   "")
{
	plParticleFlockEffect *flock = FindFlock(params[0]);
	if (flock)
		flock->SetGoalRadius(params[1]);
	else
		PrintString("Can't find flock effect");
}	

PF_CONSOLE_CMD( ParticleSystem_Flock,
			   SetFullChaseDistance,
			   "string objName, float value",
			   "")
{
	plParticleFlockEffect *flock = FindFlock(params[0]);
	if (flock)
		flock->SetFullChaseRadius(params[1]);
	else
		PrintString("Can't find flock effect");
}	

PF_CONSOLE_CMD( ParticleSystem_Flock,
			   SetConformStr,
			   "string objName, float value",
			   "")
{
	plParticleFlockEffect *flock = FindFlock(params[0]);
	if (flock)
		flock->SetConformStr(params[1]);
	else
		PrintString("Can't find flock effect");
}	

PF_CONSOLE_CMD( ParticleSystem_Flock,
			   SetRepelStr,
			   "string objName, float value",
			   "")
{
	plParticleFlockEffect *flock = FindFlock(params[0]);
	if (flock)
		flock->SetRepelStr(params[1]);
	else
		PrintString("Can't find flock effect");
}	

PF_CONSOLE_CMD( ParticleSystem_Flock,
			   SetGoalOrbitStr,
			   "string objName, float value",
			   "")
{
	plParticleFlockEffect *flock = FindFlock(params[0]);
	if (flock)
		flock->SetGoalOrbitStr(params[1]);
	else
		PrintString("Can't find flock effect");
}	

PF_CONSOLE_CMD( ParticleSystem_Flock,
			   SetGoalChaseStr,
			   "string objName, float value",
			   "")
{
	plParticleFlockEffect *flock = FindFlock(params[0]);
	if (flock)
		flock->SetGoalChaseStr(params[1]);
	else
		PrintString("Can't find flock effect");
}	

PF_CONSOLE_CMD( ParticleSystem_Flock,
			   SetMaxOrbitSpeed,
			   "string objName, float value",
			   "")
{
	plParticleFlockEffect *flock = FindFlock(params[0]);
	if (flock)
		flock->SetMaxOrbitSpeed(params[1]);
	else
		PrintString("Can't find flock effect");
}	

PF_CONSOLE_CMD( ParticleSystem_Flock,
			   SetMaxChaseSpeed,
			   "string objName, float value",
			   "")
{
	plParticleFlockEffect *flock = FindFlock(params[0]);
	if (flock)
		flock->SetMaxChaseSpeed(params[1]);
	else
		PrintString("Can't find flock effect");
}	

#endif // LIMIT_CONSOLE_COMMANDS


//////////////////////////////////////////////////////////////////////////////
//// Animation Commands		//////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#ifndef LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_GROUP( Animation ) // Defines a main command group

void SendAnimCmdMsg(char *objName, plMessage *msg)
{
	char str[256];
	plKey key = FindSceneObjectByName(objName, nil, str);
	if (key != nil)
	{
		msg->AddReceiver(key);
		plgDispatch::MsgSend(msg);
	}
	else // message wasn't sent
		delete msg;
}


PF_CONSOLE_CMD( Animation,							// Group name
				Start,								// Function name
				"string objName, string animName",	// Params
				"Start the animation" )				// Help string
{
	plAnimCmdMsg *msg = TRACKED_NEW plAnimCmdMsg();
	msg->SetCmd(plAnimCmdMsg::kContinue);
	msg->SetAnimName(nil);
	msg->SetBCastFlag(plMessage::kPropagateToModifiers);
	SendAnimCmdMsg(params[0], msg);
}

PF_CONSOLE_CMD( Animation,							// Group name
				Stop,								// Function name
				"string objName, string animName",	// Params
				"Stop the animation" )				// Help string
{
	plAnimCmdMsg *msg = TRACKED_NEW plAnimCmdMsg();
	msg->SetCmd(plAnimCmdMsg::kStop);
	msg->SetAnimName(nil);
	msg->SetBCastFlag(plMessage::kPropagateToModifiers);
	SendAnimCmdMsg(params[0], msg);
}

PF_CONSOLE_CMD( Animation,							// Group name
				SetBlend,							// Function name
				"string objName, string animName, float blend, float rate",	// Params
				"Set the animation's blend value and rate to change" )		// Help string
{
	plAGCmdMsg *msg = TRACKED_NEW plAGCmdMsg();
	msg->SetCmd(plAGCmdMsg::kSetBlend);
	msg->fBlend = params[2];
	msg->fBlendRate = params[3];
	msg->SetAnimName(params[1]);
	msg->SetBCastFlag(plMessage::kPropagateToModifiers);
	SendAnimCmdMsg(params[0], msg);
}

PF_CONSOLE_CMD( Animation,							// Group name
				SetAmp,								// Function name
				"string objName, string animName, float amp, float rate",	// Params
				"Set the amplitude of this animation and rate to change" )	// Help string
{
	plAGCmdMsg *msg = TRACKED_NEW plAGCmdMsg();
	msg->SetCmd(plAGCmdMsg::kSetAmp);
	msg->fAmp = params[2];
	msg->fAmpRate = params[3];
	msg->SetAnimName(params[1]);
	msg->SetBCastFlag(plMessage::kPropagateToModifiers);
	SendAnimCmdMsg(params[0], msg);
}

PF_CONSOLE_CMD( Animation,							// Group name
				SetSpeed,							// Function name
				"string objName, string animName, float speed, float rate",	// Params
				"Set the speed of this animation and rate to change" )		// Help string
{
	plAnimCmdMsg *msg = TRACKED_NEW plAnimCmdMsg();
	msg->SetCmd(plAnimCmdMsg::kSetSpeed);
	msg->fSpeed = params[2];
	msg->fSpeedChangeRate = params[3];
	msg->SetAnimName(params[1]);
	msg->SetBCastFlag(plMessage::kPropagateToModifiers);
	SendAnimCmdMsg(params[0], msg);
}

PF_CONSOLE_CMD( Animation,
			   AddDebugItems,
			   "string name",
			   "Add keys with the given name (substrings ok) to our report list" )
{
	plAnimDebugList *adl = plClient::GetInstance()->fAnimDebugList;
	if (adl) 
		adl->AddObjects(params[0]);
}

PF_CONSOLE_CMD( Animation,
			   RemoveDebugItems,
			   "string name",
			   "Remove keys with the given name (substrings ok) to our report list" )
{
	plAnimDebugList *adl = plClient::GetInstance()->fAnimDebugList;
	if (adl)
		adl->RemoveObjects(params[0]);
}

PF_CONSOLE_CMD( Animation,
			   ShowDebugTimes,
			   "",
			   "Toggle the view of our debug list." )
{
	plAnimDebugList *adl = plClient::GetInstance()->fAnimDebugList;
	if (adl)
		adl->fEnabled = !adl->fEnabled;
}

PF_CONSOLE_CMD( Animation,
			   ToggleDelayedTransforms,
			   "",
			   "Toggle the possibility of delayed transform evaluation." )
{
	hsBool enabled = !plCoordinateInterface::GetDelayedTransformsEnabled();
	plCoordinateInterface::SetDelayedTransformsEnabled(enabled);

	char buff[256];
	sprintf(buff, "Potential delay of transform eval is now %s", (enabled ? "ENABLED" : "DISABLED"));
	PrintString(buff);
}

#endif // LIMIT_CONSOLE_COMMANDS

////////////////////////////////////////////////////////////////////////
// Clothing Commands
////////////////////////////////////////////////////////////////////////
#ifndef LIMIT_CONSOLE_COMMANDS

#include "../plAvatar/plArmatureMod.h"
#include "../plAvatar/plAvatarClothing.h"
#include "../plAvatar/plClothingLayout.h"
#include "../pfMessage/plClothingMsg.h"

PF_CONSOLE_GROUP( Clothing ) // Defines a main command group

PF_CONSOLE_CMD( Clothing,							// Group name
				AddItemToCloset,					// Function name
				"string itemName, float r, float g, float b, float r2, float g2, float b2",	// Params
				"Add a clothing item to your closet" )		// Help string
{
	hsTArray<plClosetItem> items;
	items.SetCount(1);
	items[0].fItem = plClothingMgr::GetClothingMgr()->FindItemByName(params[0]);
	items[0].fOptions.fTint1.Set(params[1], params[2], params[3], 1.f);
	items[0].fOptions.fTint2.Set(params[4], params[5], params[6], 1.f);

	plClothingMgr::GetClothingMgr()->AddItemsToCloset(items);
}

PF_CONSOLE_CMD( Clothing,							// Group name
				WearItem,							// Function name
				"string itemName",					// Params
				"Has your avatar wear the item of clothing specified" )		// Help string
{
	plArmatureMod *avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();	
	plClothingItem *item = plClothingMgr::GetClothingMgr()->FindItemByName(params[0]);

	if (avMod && item)
	{
		avMod->GetClothingOutfit()->AddItem(item);
	}
}

PF_CONSOLE_CMD( Clothing,							// Group name
				RemoveItem,							// Function name
				"string itemName",					// Params
				"Has your avatar remove the item of clothing specified" )		// Help string
{
	plArmatureMod *avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();	
	plClothingItem *item = plClothingMgr::GetClothingMgr()->FindItemByName(params[0]);
	
	if (avMod && item)
	{
		avMod->GetClothingOutfit()->RemoveItem(item);
	}
}

PF_CONSOLE_CMD( Clothing,							// Group name
				TintItem,							// Function name
				"string itemName, float red, float green, float blue, int layer",	// Params
				"Change the color of an item of clothing you're wearing" )		// Help string
{
	plArmatureMod *avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();	
	plClothingItem *item = plClothingMgr::GetClothingMgr()->FindItemByName(params[0]);
	UInt8 layer;
	if ((int)params[4] == 2)
		layer = plClothingElement::kLayerTint2;
	else
		layer = plClothingElement::kLayerTint1;
	
	if (avMod && item)
	{
		avMod->GetClothingOutfit()->TintItem(item, params[1], params[2], params[3], true, true, true, true, layer);
	}
}

PF_CONSOLE_CMD( Clothing,							// Group name
				TintSkin,							// Function name
				"float red, float green, float blue",					// Params
				"Change your avatar's skin color" )		// Help string
{
	plArmatureMod *avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();	
	if (avMod)
	{
		avMod->GetClothingOutfit()->TintSkin(params[0], params[1], params[2]);
	}
}

PF_CONSOLE_CMD( Clothing,							// Group name
				AgeSkin,							// Function name
				"float age",					// Params
				"Blend (0 to 1) between young and old skin." )		// Help string
{
	plArmatureMod *avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();	
	if (avMod)
	{
		avMod->GetClothingOutfit()->SetAge(params[0]);
	}
}

PF_CONSOLE_CMD( Clothing,							// Group name
			   BlendSkin,							// Function name
			   "float blend, int layer",					// Params
			   "Set the blend (0 to 1) for a specific skin layer (1 to 6)." )		// Help string
{
	plArmatureMod *avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();	
	if (avMod)
	{
		avMod->GetClothingOutfit()->SetSkinBlend(params[0], (int)params[1] + plClothingElement::kLayerSkinBlend1 - 1);
	}
}

PF_CONSOLE_CMD( Clothing,
			   ChangeAvatar,
			   "string name",
			   "Switch your avatar to a different gender ('Male' / 'Female')" )
{
	plClothingMgr::ChangeAvatar(params[0]);
}

PF_CONSOLE_CMD( Clothing,							// Group name
			   SaveCustomizations,					// Function name
			   "",					// Params
			   "Save your customizations to the vault." )		// Help string
{
	plArmatureMod *avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();	
	if (avMod)
	{
		avMod->GetClothingOutfit()->WriteToVault();
	}
}

static plPlate *avatarTargetTexturePlate = nil;

PF_CONSOLE_CMD( Clothing,
			   ShowTargetTexture,
			   "...",
			   "Show/hide the texture we use for the local avatar on a square (for debugging)." )
{
	if (avatarTargetTexturePlate == nil)
	{
		plArmatureMod *avMod = nil;
		if (numParams > 0)
			avMod = plAvatarMgr::GetInstance()->FindAvatarByPlayerID((int)params[0]);
		else
			avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();			
		if (avMod)
		{
			plPlateManager::Instance().CreatePlate( &avatarTargetTexturePlate );
			avatarTargetTexturePlate->SetMaterial(avMod->GetClothingOutfit()->fMaterial);
			avatarTargetTexturePlate->SetPosition(0,0);
			avatarTargetTexturePlate->SetSize(1.9, 1.9);
			avatarTargetTexturePlate->SetVisible(true);
		}
	}
	else
	{
		plPlateManager::Instance().DestroyPlate(avatarTargetTexturePlate);
	}
}

PF_CONSOLE_CMD( Clothing,
			   WearMaintainerOutfit,
			   "",
			   "Wear the Maintainer outfit" )
{
	plArmatureMod *avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();	
	if (avMod)
	{
		avMod->GetClothingOutfit()->WearMaintainerOutfit();
	}
}

PF_CONSOLE_CMD( Clothing,
			   RemoveMaintainerOutfit,
			   "",
			   "Return to your normal outfit" )
{
	plArmatureMod *avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();	
	if (avMod)
	{
		avMod->GetClothingOutfit()->RemoveMaintainerOutfit();
	}
}
	
#endif // LIMIT_CONSOLE_COMMANDS

//////////////////////////////////////////////////////////////////////////////
//// KI Commands /////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

PF_CONSOLE_GROUP( KI ) // Defines a main command group


#ifndef LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_CMD( KI,							// Group name
				UpgradeLevel,					// Function name
				"int level",				// Params
				"Upgrade KI to new level." )	// Help string
{
	// create the mesage to send
	pfKIMsg *msg = TRACKED_NEW pfKIMsg( pfKIMsg::kUpgradeKILevel );

	msg->SetIntValue((int) params[0]);

	// send it off
	plgDispatch::MsgSend( msg );
}

PF_CONSOLE_CMD( KI,							// Group name
				DowngradeLevel,				// Function name
				"int level",				// Params
				"Downgrade KI level to next lower level." )	// Help string
{
	// create the mesage to send
	pfKIMsg *msg = TRACKED_NEW pfKIMsg( pfKIMsg::kDowngradeKILevel );

	msg->SetIntValue((int) params[0]);

	// send it off
	plgDispatch::MsgSend( msg );
}

PF_CONSOLE_CMD( KI,								// Group name
				AllOfThePower,					// Function name
				"",								// Params
				"All the phased functionality turned on." )	// Help string
{
	// create the mesage to send
	pfKIMsg *msg = TRACKED_NEW pfKIMsg( pfKIMsg::kKIPhasedAllOn );

	// send it off
	plgDispatch::MsgSend( msg );
}

PF_CONSOLE_CMD( KI,								// Group name
				NoneOfThePower,					// Function name
				"",								// Params
				"All the phased functionality turned off." )	// Help string
{
	// create the mesage to send
	pfKIMsg *msg = TRACKED_NEW pfKIMsg( pfKIMsg::kKIPhasedAllOff );

	// send it off
	plgDispatch::MsgSend( msg );
}

PF_CONSOLE_CMD( KI,								// Group name
				AddJournal,						// Function name
				"",								// Params
				"Add the journal to the Blackbar." ) // Help string
{
	// create the message to send
	pfKIMsg *msg = TRACKED_NEW pfKIMsg( pfKIMsg::kAddJournalBook );

	// send if off
	plgDispatch::MsgSend( msg );
}

PF_CONSOLE_CMD( KI,								// Group name
				RemoveJournal,					// Function name
				"",								// Params
				"Removes the journal from the Blackbar." ) // Help string
{
	// create the message to send
	pfKIMsg *msg = TRACKED_NEW pfKIMsg( pfKIMsg::kRemoveJournalBook );

	// send if off
	plgDispatch::MsgSend( msg );
}

#endif // LIMIT_CONSOLE_COMMANDS

////////////////////////////////////////////////////////////////////////
// Execute a Python file command
////////////////////////////////////////////////////////////////////////

PF_CONSOLE_GROUP( Python ) // Defines a main command group

PF_CONSOLE_CMD( Python,							// Group name
				RunFile,							// Function name
				"string filename",					// Params
				"Run the specified Python file program" )		// Help string
{
	// now evaluate this mess they made
	PyObject* mymod = PythonInterface::FindModule("__main__");
	// make sure the filename doesn't have the .py extension (import doesn't need it)
	char importname[200];
	int i;
	for (i=0; i<199; i++ )
	{
		char ch = ((const char*)params[0])[i];
		// if we are at the end of the string or at a dot, truncate here
		if ( ch == '.' || ch == 0 )
			break;
		else
			importname[i] = ((const char*)params[0])[i];
	}
	importname[i] = 0;

	// create the line to execute the file
	char runline[256];
	sprintf(runline,"import %s", importname);
	PythonInterface::RunString(runline,mymod);
	std::string output;
	// get the messages
	PythonInterface::getOutputAndReset(&output);
	PrintString(output.c_str());
}


#include "../pfPython/cyMisc.h"

PF_CONSOLE_CMD( Python,							// Group name
				SetLoggingLevel,				// Function name
				"int level",					// Params
				"Set the python logging print level (1-4)" )		// Help string
{
	cyMisc::SetPythonLoggingLevel((int) params[0]);
}

#ifndef LIMIT_CONSOLE_COMMANDS
PF_CONSOLE_CMD( Python,
			    UsePythonDebugger,
				"",
				"Enables the python debugger (only works in an .ini file)" )
{
	PythonInterface::UsePythonDebugger(true);
}


#include "../pfMessage/pfBackdoorMsg.h"
PF_CONSOLE_CMD( Python,
			    Backdoor,
				"string target, ...",	// Params
				"Send a debug trigger to a python file modifier" )
{
	const char* extraParms = "";
	if (numParams > 1)
		extraParms = params[1];
	pfBackdoorMsg *msg = TRACKED_NEW pfBackdoorMsg( params[0],extraParms );
	// send it off
	plgDispatch::MsgSend( msg );
}

PF_CONSOLE_CMD( Python,
			    Cheat,
				"string functions, ...",	// Params
				"Run a cheat command" )
{
	const char* extraParms = "";
	if (numParams > 1)
		extraParms = params[1];
	// now evaluate this mess they made
	PyObject* mymod = PythonInterface::FindModule("__main__");

	// create the line to execute the file
	char runline[256];
	sprintf(runline,"import xCheat;xCheat.%s('%s')", (const char*)params[0],extraParms);
	PythonInterface::RunString(runline,mymod);
	std::string output;
	// get the messages
	PythonInterface::getOutputAndReset(&output);
	PrintString(output.c_str());
}

PF_CONSOLE_CMD( Python,
			    ListCheats,
				"",					// Params - None
				"Show a list of python commands" )
{
	// now evaluate this mess they made
	PyObject* mymod = PythonInterface::FindModule("__main__");

	PythonInterface::RunString("import xCheat;xc=[x for x in dir(xCheat) if not x.startswith('_')]\nfor i in range((len(xc)/4)+1): print xc[i*4:(i*4)+4]\n",mymod);
	std::string output;
	// get the messages
	PythonInterface::getOutputAndReset(&output);
	PrintString(output.c_str());
}

#endif // LIMIT_CONSOLE_COMMANDS



#ifndef LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_GROUP(Demo)

PF_CONSOLE_CMD(Demo, RecordNet, "string recType, string recName", "Records a network demo (must be set in an ini file)")
{
	if (plNetClientMgr::GetInstance()->RecordMsgs(params[0],params[1]))
		PrintString("Recording Started");
	else
		PrintString("Recording Failed");
}

PF_CONSOLE_CMD(Demo, PlayNet, "string recName", "Plays back a network demo")
{
	if (plNetClientMgr::GetInstance()->PlaybackMsgs(params[0]))
		PrintString("Playback Started");
	else
		PrintString("Playback Failed");
}
/*
#include "../plHavok1/plVehicleModifier.h"

PF_CONSOLE_GROUP(Vehicle)

PF_CONSOLE_CMD(Vehicle, ShowStats, "", "")
{
	plVehicleModifier::ShowStats();
}

PF_CONSOLE_CMD(Vehicle, SetSuspensionStrength, "float val", "10-100")
{
	float val = params[0];
	plVehicleModifier::SetSuspensionStrength(val);
}

PF_CONSOLE_CMD(Vehicle, SetSuspensionDamping, "float val", "1-8")
{
	float val = params[0];
	plVehicleModifier::SetSuspensionDamping(val);
}

PF_CONSOLE_CMD(Vehicle, SetMass, "float val", "")
{
	float val = params[0];
	plVehicleModifier::SetMass(val);
}

PF_CONSOLE_CMD(Vehicle, LoadSettings, "", "")
{
	plVehicleModifier::ReadKeyValues();
	PrintString("Loaded vehicle settings");
}

PF_CONSOLE_CMD(Vehicle, SaveSettings, "", "")
{
	plVehicleModifier::WriteKeyValues();
	PrintString("Saved vehicle settings");
}
*/
#endif // LIMIT_CONSOLE_COMMANDS


#ifndef LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_GROUP(Vault)

PF_CONSOLE_CMD(Vault, Dump, "", "Prints the vault structure of current player and age to the nearest log file")
{
	VaultDump(L"Player", NetCommGetPlayer()->playerInt);
	VaultDump(L"Age", NetCommGetAge()->ageVaultId);
}

#endif

//////////////////////////////////////////////////////////////////////////////
// End.
