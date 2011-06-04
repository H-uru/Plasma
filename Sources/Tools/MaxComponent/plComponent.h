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
#ifndef PL_COMPONENT_H
#define PL_COMPONENT_H

/************************************************************************************************
**
**	Information regarding the development of Components for Plasma 2.0
**	
**	Date:		3/2/2001
**	Version:	Plasma 2.00	
**	Needs:		A working knowledge of max sdk (Specifically the ParamBlocks...)
**				A working knowledge of C++				
**
**
**	As the components are heavily reliant on the use of the MAX paramblock, the following methodology
**	should be used in their implementation:
**
**	In order for the paramblock to not be thrown away during linking, we have started the use of a
**	dummy function such as:
**
**		void DummyCodeIncludeStartPointFunc()
**		{
**		}
**
**	This is called within the code for the GlobalUtility function, currently.
**
**	Next create a derived class from the plComponent class with three public functions,
**	being its constunctor, its custom destructor and the converter.  Currently, we have the
**	converter returning an hsBool to let you know how successful the conversion process has been
**	during export to the .prd format.
**
**  After the class above has been declared, the param block stuff follows:
**
**		(Note all the imformation specifics regarding how to develop the paramblocks can be found
**		in the help included in the maxsdk.)
**
**	CLASS_DESC is a macro that Max has created that helps stuff your data into a param block.  It
**	takes in the following formal parameters:
**
**	CLASS_DESC(plStartingPointComponent, gStartPtDesc, "StartingPoint",  "StartingPointComp", Class_ID(0x2a127b68, 0xdc7367a))
**
**		1st parameter : The C++ class that is going to be accessing this function (assumed to be passed by reference)
**		2nd parameter : The Instance of the class descriptor that will be used specifically for 
**							the following param block.  It is type int.
**		3rd parameter : The Short name description string for this paramblock.
**		4th parameter : The long name description string for this paramblock.
**		5th parameter : The unique CLASS_ID created via the included GEN_ID.exe that comes with maxsdk.
**
**	Following this macro, an enum array is useful to store the relatively unique ID constants for the paramblock.
**
**  The format of the definitions in this shown below, but basically is in the form of a sequence of fixed specs followed by a variable number of tagged optional specs for each parameter. 
**
**
**	Next, the actual param block is declared such as the following:
**
**	ParamBlockDesc2 gStartPtBk
**		(
**		1, _T(""), 0, &gStartPtDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, 0,
**	
**		//rollout
**		IDD_COMP_STARTINGPOINT, IDS_COMP_STARTINGPOINTS, 0, 0, NULL,
**	
**		// params
**		kPlayerStartingPoint,		_T("Start Point:"),		TYPE_STRING, 		0, 0,	
**		p_ui,			TYPE_EDITBOX, IDC_COMP_STARTPOINT,
**		end,
**	
**		end
**		);
**
**	First is the name of the descriptor method that is to be used, in this case the ParamBlockDesc2
**	type.  Second is the unique name of this this paramblock.  Following, a collection of comma delimited
**	parameters ensue.  The exact details as well as other flags to use herein can be found in the MaxR4 sdk
**	Help file.  We shall explain the above parameters to help give you an idea of the layout.
**	All paramblocks have the following format:	
**
**       1. Required block specs & per-block flags, followed by,
**       2. Optional owning object reference number for the block if auto-construct, followed by,
**       3. Optional parameter map specs if auto-ui, followed by,
**		 4. Zero or more parameter specs, comprising:
**            a. Required parameter specs, followed by,
**            b. Zero or more optional parameter specs, each with is own leading tag, 
**               the list terminated by an 'end' tag, followed by,
**       5. an 'end' tag
**
**	THE BLOCK SPECS:
**
**		BlockParameter 1:  
**					"1" is the permanent block ID for the parameter block2.  It must be unique
**					to the block.  Considering that you can have several blocks stored together in the same window,
**					you may want to enum this as well.  THIS IS A REQUIRED FIELD.
**		
**		BlockParameter 2: 
**					The internal name string.  This name is not localized. Internal names are meant
**					to be parsable as identifiers.  As such they should begin with an alpha character, have only 
**					alphanumerics, and have no spaces, punctuations, etc.  The convention for multi-word names 
**					is to use studly-caps, eg, paintRadius.  THIS IS A REQUIRED FIELD.
**
**		BlockParameter 3:  
**					The resource ID of the localized (sub-anim) name string.  It is of type int.  THIS IS
**					A REQUIRED FIELD.
**
**		BlockParameter 4: 
**					Points to the class descriptor2 of the owning class.  This is used to add this 
**					descriptor to the ClassDesc2's table of block descriptors for the class. Note: This value may 
**					be passed as NULL for situations where the blocks owning ClassDesc2 is not available for static 
**					initializations (such as in a separate file).  Before using the descriptor for any block 
**					construction, the ClassDesc2* must be initialized with the method:
**
**					void ParamBlockDesc2::SetClassDesc(ClassDesc2* cd);
**
**					You can only call this method once on a descriptor and then only if it has been 
**					constructed initially with a NULL cd.
**
**		BlockParameter 5:  One or more BYTE flags.  In this instance, we are only using:
**					
**					P_AUTO_CONSTRUCT
**							Indicates the parameter block2 will be constructed and referenced automatically 
**							to its owner in the call to ClassDesc2::MakeAutoParamBlocks().  If this flag is 
**							set, the parameter block's reference number in the owning object should be given 
**							immediately following the flag word in the descriptor constructor.  See 
**							<auto_construct_block_refno>.
**					  
**					P_AUTO_UI
**							Indicates this block supports automatic UI rollout management in calls to 
**							ClassDesc2::BeginEditParams(), ClassDesc2::EndEditParams(),ClassDesc2::CreateParamDlg(),
**							ClassDesc2::CreateParamDialog(), etc. 
**							
*8							The <auto_ui_parammap_specs> must be supplied in the descriptor constructor. 
**
**		BlockParameter 6:  Because we used the AUTO_UI the following parameters are REQUIRED.
**		
**						int dialog_template_ID, 
**								(in this case IDD_COMP_something from an .rc file)
**						int dialog_title_res_ID, 
**								(in this case IDS_COMP_something from the String Table)
**						int flag_mask, 
**								(This is used by ClassDesc2::BeginEditParams() and ClassDesc2::EndEditParams() 
**								to determine whether the ParamMap2 shold be created/deleted on this call.  
**								All the bits in the supplied mask must be on in the Begin/EndEditParams 
**								flag longword for the action to take place.  For this example we have 0.)
**						int rollup_flags 
**								(This flag is used to control rollup creation.  You may pass
**								APPENDROLL_CLOSED to have the rollup added in the closed (rolled up) state.  
**								Otherwise pass 0.)
**						ParamMap2UserDlgProc* proc
**								(If there are controls in the dialog that require special processing this user 
**								If not used then NULL should be passed.)
**								
**  THE PARAM SPECS:
**
**		Param Parameter 1:	ParamID id
**				(The permanent, position-independent ID for the parameter.  In this case, it is
**				the constant kStartingPoint.)
**		Param Parameter 2:	TCHAR* internal_name
**				(The internal name for the parameter.  In our case "Start Point:".  This is what is written on the
**				button, spinner, whizbang, foozle, etc.)
**		Param Parameter 3:	ParamType type
**				(The type of parameter.  See List of ParamType Choices.   In this case it is TYPE_STRING.  It could be
**				TYPE_INT, TYPE_FLOAT, etc, etc, etc.)
**		Param Parameter 4:	[int table_size]
**				(If the type is one of the Tab<> types, you need to supply an initial table size 
**				which can be 0.  Which it is in our case above.)
**		Param Parameter 5:	int flags
**				(These flags are ORed together.   We don't use any, hence 0.  They include useful things such as
**				p_SubAnim which allow track view acess...)
**		Param Parameter 6: int local_name_res_ID 
**				(Here lies that relatively unique enum mentioned above.  Hence, the kStartingPoint.)
**
**	OPTIONAL STUFF:
**		
**		  As param blocks don't necessarily need a visual GUI, any that you include are optional.
**
**		  p_ui, 
**			(This declares that a GUI will be instantiated.  The parameter hereafter will declare the type.)
**		  GUI_Type
**			(This is the type of GUI that you are going to use.  In our case it is a CUSTOM editbox.
**		
**				NOTE:  MAX prefers that you create CUSTOM instances of all GUI resources (such as spinners, 
**							editboxes, etc)
**			)
**
**		  ID
**			(This is the implementor defined name that this GUI is using.  It is the name of the object that 
**			you created a resource of.  We created an IDC_COMP_STARTINGPOINT which was of a CustEdit type.)
**
**		  end
**			(This lets the constructor that makes uses to know that you have finished one of the p_ui param 
**			blocks.)
**			
**
**	FINAL REQUISITE STUFF:
**
**	end
**	);	
**		(This is the conclusion of the Block descriptor.  Sort of intuitive...)
**
*************************************************************************************************/

#include "plComponentBase.h"
#include "../MaxExport/plErrorMsg.h"


class plMaxNode;
class plErrorMsg;

class plComponent : public plComponentBase
{
public:

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg)	{ return true; }

	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)		{ return true; }
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg) = 0;

	// DeInit pass--free up any temp memory you might have allocated here
	virtual hsBool DeInit(plMaxNode *node, plErrorMsg *pErrMsg)		{ return true; }
};

#endif
