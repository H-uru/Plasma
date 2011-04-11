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
#ifndef plControlEventCodes_inc
#define plControlEventCodes_inc


// list of all controls that could potentially
// be mapped to a key, a mouse button, etc 

// NOTE:
// NOTE:
// NOTE: PLEASE READ BEFORE MODIFYING THIS FILE!!!!!!!!!!!!!!!!!!!
// NOTE:
// NOTE:
//   Please put new control key events at the end of this enum (just before END_CONTROLS).
//   This way the corresponding Python constants will match more often
//     and cause much less stress on game play programmers who's telescopes stop working
//        Thanks!

enum ControlEventCode
{
	// button controls
	B_CONTROL_ACTION			= 0,
	B_CONTROL_ACTION_MOUSE,
	B_CONTROL_JUMP,
	B_CONTROL_MOVE_FORWARD,
	B_CONTROL_MOVE_BACKWARD,
	B_CONTROL_STRAFE_LEFT,
	B_CONTROL_STRAFE_RIGHT,
	B_CONTROL_MOVE_UP,
	B_CONTROL_MOVE_DOWN,
	B_CONTROL_ROTATE_LEFT,
	B_CONTROL_ROTATE_RIGHT,
	B_CONTROL_ROTATE_UP,
	B_CONTROL_ROTATE_DOWN,
	B_CONTROL_MODIFIER_FAST,
	B_CONTROL_ALWAYS_RUN,	// This is no longer a bindable key. It is hard-coded to caps-lock
	B_CONTROL_EQUIP,
	B_CONTROL_DROP,
	B_CONTROL_TURN_TO,
	B_TOGGLE_DRIVE_MODE,
	B_CAMERA_MOVE_FORWARD,
	B_CAMERA_MOVE_BACKWARD,
	B_CAMERA_MOVE_UP,
	B_CAMERA_MOVE_DOWN,
	B_CAMERA_MOVE_LEFT,
	B_CAMERA_MOVE_RIGHT,
	B_CAMERA_PAN_UP,
	B_CAMERA_PAN_DOWN,
	B_CAMERA_PAN_LEFT,
	B_CAMERA_PAN_RIGHT,
	B_CAMERA_MOVE_FAST,
	B_CAMERA_ROTATE_RIGHT,
	B_CAMERA_ROTATE_LEFT,
	B_CAMERA_ROTATE_UP,
	B_CAMERA_ROTATE_DOWN,
	B_CAMERA_RECENTER,
	B_CAMERA_DRIVE_SPEED_UP,
	B_CAMERA_DRIVE_SPEED_DOWN,
	B_CAMERA_ZOOM_IN,
	B_CAMERA_ZOOM_OUT,
	B_SET_CONSOLE_MODE,
	B_CONTROL_CONSOLE_COMMAND,
	B_CONTROL_TOGGLE_PHYSICAL,
	B_CONTROL_PICK,
	// axis controls
	A_CONTROL_MOVE,
	A_CONTROL_TURN,
	A_CONTROL_MOUSE_X,
	A_CONTROL_MOUSE_Y,
	// special controls
	S_SET_CURSOR_UP,
	S_SET_CURSOR_DOWN,
	S_SET_CURSOR_RIGHT,
	S_SET_CURSOR_LEFT,
	S_SET_CURSOR_POISED,
	S_SET_CURSOR_HIDDEN,
	S_SET_CURSOR_UNHIDDEN,
	S_SET_CURSOR_ARROW,
	S_SEARCH_FOR_PICKABLE,
	S_INCREASE_MIC_VOL,
	S_DECREASE_MIC_VOL,
	S_PUSH_TO_TALK,
	S_SET_THIRD_PERSON_MODE,
	S_SET_FIRST_PERSON_MODE,
	S_SET_WALK_MODE,
	S_SET_FREELOOK,
	S_SET_CONSOLE_SINGLE,
	S_SET_CONSOLE_HIDDEN,
	// Inventory controls
	dead__B_CONTROL_SET_EQUIPED_STATE,
	dead__B_CONTROL_SCROLL_UP_LIST,
	dead__B_CONTROL_SCROLL_DOWN_LIST,
	dead__B_CONTROL_SET_INVENTORY_ACTIVE,
	dead__B_CONTROL_SET_INVENTORY_DISACTIVE,
	dead__B_CONTROL_REMOVE_INV_OBJECT,
	dead__B_CONTROL_ENABLE_OBJECT,
	// Avatar emote controls
	B_CONTROL_EMOTE,
	B_CONTROL_EXIT_MODE,
	// new controls key events
	B_CONTROL_DIVE,
	B_CAMERA_PAN_TO_CURSOR,
	B_CONTROL_OPEN_KI,
	B_CONTROL_OPEN_BOOK,
	B_CONTROL_EXIT_GUI_MODE,
	B_CONTROL_MODIFIER_STRAFE,
	B_CONTROL_CAMERA_WALK_PAN,
	S_SET_BASIC_MODE, // Opposite of walk mode
	B_CONTROL_IGNORE_AVATARS, //anti-griefing thing
	B_CONTROL_LADDER_INVERTED, // used by ladders to invert forward/backward
	B_CONTROL_CONSUMABLE_JUMP, // used to limit the avatar to one jump per keypress
	S_SET_WALK_BACK_MODE,
	S_SET_WALK_BACK_LB_MODE,
	S_SET_CURSOR_UPWARD,
	S_SET_LADDER_CONTROL,
	S_CLEAR_LADDER_CONTROL,
	END_CONTROLS
};



#endif // plControlEventCodes_inc 
