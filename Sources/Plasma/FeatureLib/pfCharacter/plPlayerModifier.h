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
//
//#ifndef plPlayerModifier_inc
//#define plPlayerModifier_inc
//
//#include "../pnModifier/plSingleModifier.h"
//#include "../pnSceneObject/plSimulationInterface.h"
//#include "hsMatrix44.h"
//
//class plControlEventMsg;
//
//namespace Havok {
//  class Vector3;
//}
//
//class plPlayerModifier : public plSingleModifier
//{
//protected:
//  
//  enum
//  {
//      kWantsToSpawn = 0,
//      kTimerSet,
//      kHasSpawned,
//      kNeedsLocalSetup
//  };
//
//  struct spawnPt
//  {
//      hsPoint3 pt;
//      float dist;
//  };
//
//  static float     fTurnRate;
//
//  static float     fAcceleration;
//  static float     fDeceleration;
//  static float     fMaxVelocity;
//  float            fCurSpeed;
//
//
//  double              fLastTime;
//  hsMatrix44          fDesiredMatrix;
//
//  hsPoint3            fDesiredPosition;
//  hsPoint3            fFacingTarget;
//  bool                bUseDesiredFacing;
//  bool                bUseDesiredMatrix;
//  bool                bIgnoreDesiredMatrix;
//
//  float            fRotationScalar;
//  hsTArray<spawnPt*>  fSpawnPoints;
//  
//  void IAdjustVelocity(float adjAccelRate, 
//                       float adjDecelRate, 
//                       hsVector3* dir, 
//                       hsVector3* vel, 
//                       float maxSpeed, 
//                       float distToGoal,
//                       double elapsedTime);
//
//  float IClampVelocity(hsVector3* vel, float maxSpeed, double elapsedTime);
//  hsBool32 IShouldDecelerate(float decelSpeed, float curSpeed, float distToGoal);
//
//  hsBool  HasMovementFlag(int f) const { return fMoveFlags.IsBitSet(f); }
//  void    SetMovementFlag(int f) { fMoveFlags.SetBit(f); }
//  void    ClearMovementFlag(int which) { fMoveFlags.ClearBit( which ); }
//
//  hsBitVector     fMoveFlags;
//  hsBitVector     fFlags;
//
//  void WarpToSpawnPoint() { SetFlag( kWantsToSpawn ); }
//
//  hsBool  bMoving;
//
//  void IApplyForce(plSimulationInterface::plSimpleForce type, const Havok::Vector3 &vec);
//  void IDoLocalSetup(plSceneObject*);
//  void    IMakeUsListener( plSceneObject *so );
//
//public:
//  plPlayerModifier();
//  virtual ~plPlayerModifier();
//
//  CLASSNAME_REGISTER( plPlayerModifier );
//  GETINTERFACE_ANY( plPlayerModifier, plSingleModifier );
//
//  virtual hsBool  MsgReceive(plMessage* msg);
//  virtual void    AddTarget(plSceneObject* so);
//  virtual void    RemoveTarget(plSceneObject* so);
//
//  hsBool HandleControlInput(plControlEventMsg* pMsg);
//  virtual hsBool IEval(double secs, float del, uint32_t dirty);
//
//  void SetMoving(hsBool b);
//  hsBool IsMoving() { return bMoving; }
//
//  hsBool  HasFlag(int f) const { return fFlags.IsBitSet(f); }
//  void    SetFlag(int f) { fFlags.SetBit(f); }
//  void    ClearFlag(int which) { fFlags.ClearBit( which ); }
//
//  static void SetTurnRate(float f) {fTurnRate = f;}
//  static void SetAcceleration(float f) {fAcceleration = f;}
//  static void SetDeceleration(float f) {fDeceleration = f;}
//  static void SetVelocity(float f)     {fMaxVelocity = f;}
//
//  
//};
//
//#endif plPlayerModifier_inc
