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
#ifndef plVersion_h_inc
#define plVersion_h_inc

#include "hsTypes.h"

// RULES:
// Log your change
// Set Minor Version to ZERO when you Bump Major Version
// If you change the minor, see GetChangedCreateables in plVersion.cpp

#define PLASMA2_MAJOR_VERSION	70	// Major Version...every file will need to be reexported
#define PLASMA2_MINOR_VERSION	2	// Minor Version...only files with the specified creatables
									// will need to be reexported

// Don't modify this, it's automatically updated when branches are made
#define PLASMA2_BRANCH_NAME "Main Branch"

class plVersion
{
public:
	static UInt16 GetMajorVersion();
	static UInt16 GetMinorVersion();

	// Pass in a creatable index to get its current version.  Zero means it
	// hasn't changed since the last major version change, anything else is the
	// last minor version it changed at.  This takes into account the version of
	// parent classes.
	static int GetCreatableVersion(UInt16 creatableIndex);
};

/*	Major Log							---Death to those who do not log changes---
	#	Date	Who		comment
	1	5/8/01	Paulg	Added version Number
	2	5/11/01	mf		Changed sortable mesh format by 2 bytes.
	3	5/14/01 Paulg	Index files Location is used for ReadConfig at Client Startup, RoomLoc added to Index
	4   5/21/01 matt	Fixed errant bounding types for physicals (old bad codes will choke)
	5	5/24/01 mf		Added Occluders to the data format.
	6	5/29/01 mf		Purvis changed the sound data format and didn't change the version.
	7	5/3/01	thamer	Cleaned up some object read/write routines
	8	6/12/01	mcn		Obliterated hsGTexture on down. Now we use plBitmaps and plMipmaps (class IDs changed)
	9	6/21/01 mcn		Updated drawable disk format. Forgot a field that made particle systems break when it wasn't there
   10	6/25/01	mcn		Updated plMipmap format. Helps resolve the huge memory issues we were having
   11	6/28/01 mf		Slight changes to material and geometry formats to support projective texturing.
   12	7/02/01 mcn		Changed file format to fix disappearing objects in SceneViewer
   13   7/09/01 bob		Changed file format of plSimpleParticleGenerator to allow mesh generated systems.
   14	7/13/01 thamer	Changed uoid read/write to include clone number
   15   7/20/01 thamer	Changed uoid sequence to support up to 4 player rooms
   16	7/20/01	mcn		Added a second drawable + key to drawInterfaces
   17   7/22/01 bob		Changed file format to have particle systems support material diffuse/opacity animations
   18	7/27/01 mcn		Added stuff to drawableSpans for SceneViewer. Are we ever going to have another non-zero minor version? :)
   19	7/31/01 mf		Did a get and the format had changed but the version hadn't been updated (again). Is this version thing dead?
   20	8/13/01	mcn		Not that I know of. Change to various object formats to support object instancing.
   21	8/20/01 mf		Like you'd know. Revised inter-object dependency connections throughout the five scene graphs.
   22	8/21/01 mf		CoordinateInterfaces keep refs to child SceneObjects instead of child CoordinateInterfaces.
   22	8/21/01	mcn		You hurt my feelings. I bump major version in retaliation. (Also upped max UV count) 
   23	8/24/01 mf		Made runtime lights respect Max's OTM.
   24	8/26/01 mcn		Changed plSound format to support stereo (background music) sounds
   25	9/05/01 thamer  Changed uoid to support 32bit clones
   26	9/14/01	mcn		Added all 12 parameters to the Audio Region component.
   27	9/25/01	mf		Added soft volumes to run-time lights. Breaks any scene with RT lights.
   28  10/03/01 matt    Changed physics format just enough to make everyone reexport everything.
   29	10/9/01 ee		Changed Uoid member types to a new format that can accomodate strings.
   30  10/10/01	mcn		Split diffuse layer colors to static and runtime diffuse colors. Breaks the entire universe and parts of New Jersey.
   31  10/11/01	ee		Activated string usage in uoids -- this version thing is sure not dead now.
   32  10/11/01	mf		Changed format of plSpan. Only breaks scenes with visible geometry. Should that be Minor Version?
   33  10/12/01 cp		changed read/write functions in plAnimTimeConvert.  Breaks all previously exported animations
   34  10/22/01 Colin	Changed format of plBitmap
   35  10/30/01	mf		Change to span format, breaks anything with visible geometry.
   36	11/5/01	mcn		Changed structure of sound classes to be far more intuitive and bug-free.
   37	11/9/01	ee		Just for fun.
   38  11/13/01 bob		Changed file format for plAnimTimeConvert / plAnimCmdMsg / plAGAnim (sense a theme?) 
   39  11/25/01 mf		More sorting info for drawable geometry.
   40  11/29/01 mcn		Fixed sound fade param read/write. Anything with sounds must be re-exported.
   41  12/04/01	bob		More animation file format changes.
   42  12/05/01 mcn		Changes to sound format. Now properties are in a general props field. Also added disable-LOD prop to sounds
   43  12/18/01	cjp		changes to physics system
   44  12/18/01 bob		anim file format.
   45  12/19/01 Colin	anim file format, again.
   46  12/28/01 mf		DrawInterface format change to purge the last of the Opaque/Blending dualism.
   47	 1/2/01	mf		Decoupled the hardware and software skinning vertex formats.
   48 	 1/6/01	mf		Added grey area to Soft Regions (formerly Soft Volumes).
   49   1/17/02 cp		new camera system checked into client and plugins	
   50   1/21/02 Colin	Physics changes
   51   1/24/02 Colin	Animation format change
   52   2/14/02 bob		plAnimTimeConvert format change
   53   2/14/02 matt    made all brains non-keyed. reordering all over the place
   54   3/15/02 cjp		new LOS query types, format change to plHKPhysical	
   55   4/09/02 cjp		more new LOS query types, format change to plHKPhysical, camera type changes	
   56   4/23/02 bob		nuked plSimpleModifier from plAGMasterMod. Changes file format for animation.
   57   5/17/02	Colin	Changed Uoid format for cloning
   58	7/02/02	thamer	Changed synchedOject R/W format to support more LocalOnly options
   59	8/14/02	bob		Anim file format
   60	2/08/03 mf		Changed formats for everything drawable and/or physical. That leaves behaviors, but it's about time for a major version change anyway.
   61	2/18/03 mf		Between my changes and Bob's, we're just not sure what hasn't changed anymore. At least drawables and avatars.
   62	3/30/03 mf		Added LoadMask to plUoid
   63	5/30/03	thamer	optimized Uoid size
   64  10/28/05 jeff	changed plLocation to handle more pages and plUoid for optimization reasons
   65	2/22/06 bob		animation key rewrite to save space
   66   2/27/06 bob		64-bit quaternion anim keys
   67	2/28/06 bob		Anims store UInt16 frame numbers, not 32-bit float times.
   68	3/03/06 bob		constant anim channels (plMatrixConstant, plScalarConstant had no R/W methods)
   69	5/08/06 bob		changed plVertCoder and hsMatrix44::Read/Write
   70	2/12/07 bob		Merged in several registry/resMangaer fixes
*/

/*	Minor Log							---Death to those who do not log changes---
	#	Date	Who		comment
	1	5/8/01	Paulg	Added version Number
	2	5/9/01	mcn		Changed color components of drawableSpans
	0	5/11/01	mf		Dropped back to zero on Major Version change
	1	8/06/01 thamer	Upped the version for mf's anim callback changes.
	2   8/06/01 bob		Changes to particle system read/write. 
	0	8/13/01	mcn		Bumped back to 0 due to major version change.
	1   8/23/01 bob		Added animation controller to particle systems, changing their format.
	0	8/24/01 mcn		Bumped back to 0 yet again.
	1	8/27/01	mcn		Changed how emissive flags are handled on materials.
	2	8/29/01 bob		Changed plAvatarMod file format
	0   9/05/01	thamer	Bumped back to 0 yet again.
	1	9/17/01 matt    Avatar only.
	2	9/24/01 bob		Avatar and sound file formats changed. (For age linking effects)
	0	9/25/01 mf		Reset to zero for major change.
   1	10/7/01 Colin	Format of plResponderModifier and plEventCallbackMsg changed.
   0	10/9/01 ee		Reset to zero for major change.
   1	10/11/01 mcn	Changed sound format. Anything with sounds (including avatar b/c of linking sounds) breaks. Is this a record for # of version changes in one week?
   1	10/15/01 mcn	Added soft volumes to sounds. Breaks anything with sounds. Again.
   2	10/17/01 Colin	Changed format of plAGAnim
   0	10/22/01 Colin	Reset to zero for major change.
   1	11/01/01 bob	Changed format for plAGAnim and plAGMasterMod. Will break avatars without re-export.
   0	11/02/01 mf		Reset to zero for major change.
   1	11/28/01 mcn	Changed meaning of sound volume property. Will work if not re-exported, but will sound wrong.
   0	11/29/01 mcn	Reset on major version change.
   1	12/10/01 bob	Changes to animTimeConvert. Animated materials will need a re-export.
   0	12/18/01 bob	Reset to zero for major change.
   1	1/07/02	 bob	File format change for particle systems.
   0	1/21/02  Colin	Reset to zero for major change.
   1	1/24/02  Colin	File format for responders
   2	1/24/02  mcn	Changed postEffectMod format. Not many people use it though, so only minor version change.
   0	1/24/02  Colin	Reset to zero for major change.
   1	1/28/02	 bob	File format change to avatars for clothing customization.
   2	1/30/02	 Colin	File format change to sit component
   3	1/31/02	 Colin	File format change to logic modifier (all detector components)
   4	2/13/02  Colin	File format change to ladder modifier
   0	2/14/02  bob	Reset to zero for major change.
   1(5)	2/14/02	 mcn	GUI control format changes to support dynamic text layers (the RIGHT way) 
   2(6)	2/19/02	 mcn	Added version field to dialogs, to allow synching with C++ and Python code (just in case)
   7	3/01/02	 mcn	Moved GUI controls to pfGUIColorScheme methods of handling colors/fonts
   8	3/01/02  cjp	changed format of new camera brains, only affects the few using them right now
   10	3/08/02	 mcn	Changed internal format of sounds to facilitate packing them in files later
   11	3/08/02	 mcn	Added a shadow option to GUI color schemes. Also added mouse-over animations to buttons.
   12	3/12/02  bob	Change format of clothing
   13   3/15/02  matt   Changed base class for sit modifier
   0	3/15/02  cjp	Only chris forgot to log it - mf
   1	3/29/02  mf		Added light group support for particle systems.
   2	4/02/02  Colin	Format change for responder modifiers
   0	4/15/02  cjp	Only chris forgot to log it - mf
   1	4/15/02	 mf		Added a parm to partycle systems just for fun.
   2	4/17/02	 cjp	Added new modifier for interface information, changed plMaxNode.cpp.
   0	4/23/02	 bob	Reset to zero.
   1	4/25/02	 mcn	GUI objects no longer fog. Must reexport GUI components.
   2	5/05/02	 mf		Particle system enhancements
   3	5/06/02  matt	Changes to one shot modifier and multistage modifier
   4	5/06/02	 bob	Changes to plArmatureMod and Clothing stuff. For swappable meshes and footsteps.
   5	5/07/02  bob	Changed clothing item file format (again) for text descriptions and thumbnails
   6	5/09/02  mcn	Added sound event capabilities to GUI controls
   7	5/13/02  mcn	Added some new options to GUI knobs
   8	5/14/02  matt	Fix net propagation for animation stages (fading) and generic brains (exit flags)
   0	5/17/02  Colin	Reset for major version
   1	5/21/02	 mcn	Added a channel select option for plWin32Sounds
   2	5/23/02  bob	Added multiple texture layers per element in plClothingItem
   3	5/23/02	 mcn	Added some options to plSoundBuffer.
   4	6/18/02	 mcn	Added some more options to plSound, incl. localOnly flags
   6	6/21/02  mcn	Updated plDynamicTextMap to include an initial image buffer
   7	6/26/02  mcn	Major revision to sound system to support hardware acceleration
   0	7/02/02	thamer	Reset for major version
   1    7/08/02  matt   Format change for animation stages -- added next/prevStage override
   2	7/10/02  bob	Format changes for avatar footstep sounds
   3	7/12/02  mcn	Format change to sounds for EAX effects
   4	7/23/02	 bob	Format change to footstep sounds for more surface options
   5	7/29/02	 mcn	More EAX format changes to sounds
   6	7/29/02	 mcn	Added cutoff attenuation to spot/omni lights
   7	8/01/02	 bob	Format change to clothing items
   0	8/14/02	 bob	Reset for major version
   1	8/19/02	 bob	plClothingItem file format
   2	9/02/02	 bob	plArmatureLODMod file format change for bone LOD
   3	9/12/02	 mf		Making ripples and waves play nice with each other.
   4	9/18/02	 bob	plClothingItem file format... again
   5	9/19/02	 mcn	New GUI control proxy stuff
   6	9/23/02	 mcn	Removed dead sound stuff
   7	9/24/02	 mcn	Sound priority stuff
   8	9/25/02	 mcn	Support for new grouped random sound objects
   9	10/15/02 mcn	Material anim support in GUI controls
   10	10/21/02 mcn	Variable volume support to group sounds. No format break.
   11	10/29/02 cjp	proper python notification of avatar page out & last avatar out plus elevator persistance fix - breaks message format for exclude regions 
   12	10/29/02 mcn	Fixing chris's booboo 
   13	10/29/02 cjp	Changed camera component data format slightly.
   16	11/18.02 cjp	changed camera speed components & objects, don't know where v 14 & 15 went	
   17   12/04/20 matt    New line-of-sight categories; format change for physicals.
   18	12/05/02 mf		Enhanced linefollowmod to play nice with stereizer, and added field to occluder's cullpoly.
   19	12/17/02 matt	Bumped armaturemod, based on strong circumstantial evidenced that I missed something.
   20	12/31/02 matt	New format for animation stages, written by the multistage mod.
   21   01/03/03 matt   Change to sitmodifier.
   22	01/16/03 matt	More simplification for animation stages and generic brains.
   23	01/20/03 bob	Added layer to clothing items for aged skin blending
   24	01/21/03 mf		plLayers now read and write out their shaders.
   25	01/29/03 bob	plMorphSequence read/writes the plSharedMeshes it uses
   27	02/05/03 mcn	Updates to pfGUIButtonMod
   0	02/18/03 mf		Reset for major version change.
   1	02/19/03 bob	Added layers to plClothingItem. Took the opp to cleanup the R/W functions
   2	02/21/03 mf		Added features to dynamic water. No one should notice.
   3	02/24/03 cjp	changed animated cameras - added new commands for controlling them
   4	02/24/03 mcn	Updates to GUI stuff to support, er, new GUI stuff.
   5	03/11/03 bob	Clothing and linkSound format change
   0	03/30/03 mf		Reset for major version change.
   9	04/14/03 Colin	Havok change.  At 9 since mf forgot to bump the version (and Matt did a bunch of undocumented bumps)
   10	04/15/03 bob	Added a footstep surface type
   11	4/18/03	 mcn	Added list of the layers grabbed by a GUI DynDisplay
   12	4/23/03	 bob	File format for plMultistageBehMod, needed for a ladder fix
   13	05/01/03 Colin	Changed how Havok vectors and quats are written
   14	03/30/03 mf		Particle effect enhancement
	0	05/30/03 thamer	Reset for major version change.
	1	06/01/03 bob	Added a flags variable to plSharedMesh
	2   06/06/03 markd  Added NotifyType for GUIButtons
	3	06/07/03 mcn	Added sound groups to physicals
	4	06/24/03 bob	More params for Flocking particles
    5	07/05/03 mf		Particle and footprints working together
	6	07/13/03 bob	Avatar file format
	7	07/31/03 bob	LinkToAgeMsg (which affects responders) and panic link regions
	8	08/21/03 Colin	Removed some stuff from plSoundBuffer
	9	08/22/03 bob	Added info to plClothingItem
   10	09/16/03 bob	Removed stuff in plAnimCmdMsg, which affects responders.
   11	09/18/03 mf		Changed plLoadMask, of which plWaveSet7 is the biggest user.
   12	01/03/04 jeff	pfGUIDynDisplayCtrl now stores material as well as layer information
!! 11	02/10/04 mf		Dropped back a version, to reduce patch size for Expansion 1.
   12	03/12/04 bob	New stuff in Swim Regions
    0	10/28/05 jeff	Reset for major version change
	1	11/28/05 jeff	GUI text boxes can now pull strings from the localization mgr
	2	12/01/05 jeff	pfKIMsg can now handle unicode strings
	3	02/02/06 adam	modified plDynamicEnvMap as a part of back-porting planar reflections from P21
	0	02/22/06 bob	Reset for major version change
	1	04/12/06 Colin	Changed physical format
	0	05/08/06 bob	Reset for major version change
	1   05/16/06 markd  Changed physics format to include boxes
	2	06/26/06 jeff	Changed coop brain format so book sharing works again
	3	12/05/06 bob	Avatar uses a render target instead of a plMipmap.
	4	01/16/07 bob	Still does, it's just not created at export.
	5	01/24/07 adam	Changed plAGMasterMod so one can be set as a group master in grouped anims
	0	02/12/07 bob	Reset for major version change
	1	03/29/07 jeff	Changed plLoadAvatarMsg and plArmatureMod to be more flexible
	2	06/28/07 jeff	Changed plAvBrainHuman format to store whether it's an actor or not
*/

#endif // plVersion_h_inc
